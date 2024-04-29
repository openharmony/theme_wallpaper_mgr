/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "js_wallpaper_extension_ability.h"

#include "ability_info.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_wallpaper_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "uv_queue.h"
#include "wallpaper_extension_ability_stub.h"
#include "wallpaper_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
} // namespace

JsWallpaperExtensionAbility *JsWallpaperExtensionAbility::jsWallpaperExtensionAbility = NULL;
std::mutex JsWallpaperExtensionAbility::mtx;
using namespace OHOS::AppExecFwk;
using namespace OHOS::MiscServices;
JsWallpaperExtensionAbility *JsWallpaperExtensionAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility begin Create");
    std::lock_guard<std::mutex> lock(mtx);
    jsWallpaperExtensionAbility = new JsWallpaperExtensionAbility(static_cast<JsRuntime &>(*runtime));
    return jsWallpaperExtensionAbility;
}

JsWallpaperExtensionAbility::JsWallpaperExtensionAbility(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{
}
JsWallpaperExtensionAbility::~JsWallpaperExtensionAbility()
{
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    std::lock_guard<std::mutex> lock(mtx);
    jsWallpaperExtensionAbility = nullptr;
}

void JsWallpaperExtensionAbility::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility begin Init");
    WallpaperExtensionAbility::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        HILOG_ERROR("Failed to get srcPath");
        return;
    }
    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        Extension::abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsWallpaperExtensionAbility object");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    napi_value contextObj = CreateJsWallpaperExtensionContext(env, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("WallpaperExtensionContext", &contextObj, ARGC_ONE);
    contextObj = shellContextRef->GetNapiValue();
    context->Bind(jsRuntime_, shellContextRef.release());
    napi_set_named_property(env, obj, "context", contextObj);
    if (contextObj == nullptr) {
        HILOG_ERROR("Failed to get wallpaper extension native object");
        return;
    }
    auto workContext = new (std::nothrow) WallpaperExtensionContext();
    if (workContext == nullptr) {
        HILOG_ERROR("Failed to new workContext");
        return;
    }
    auto ret = napi_wrap(env, contextObj, workContext, [](napi_env, void *data, void *) {
            delete static_cast<AbilityRuntime::Context *>(data);
        }, nullptr, nullptr);
    if (ret != napi_ok) {
        delete workContext;
    }
}

void JsWallpaperExtensionAbility::OnStart(const AAFwk::Want &want)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "OnStart", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
    StartAsyncTrace(HITRACE_TAG_MISC, "Extension::OnStart",
        static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    Extension::OnStart(want);
    FinishAsyncTrace(HITRACE_TAG_MISC, "Extension::OnStart",
        static_cast<int32_t>(TraceTaskId::ONSTART_MIDDLE_EXTENSION));
    HILOG_INFO("jws JsWallpaperExtensionAbility OnStart begin..");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    StartAsyncTrace(HITRACE_TAG_MISC, "onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    FinishAsyncTrace(HITRACE_TAG_MISC, "onCreate", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    CallObjectMethod("createWallpaperWin");
    RegisterWallpaperCallback();
    HILOG_INFO("%{public}s end.", __func__);
    FinishAsyncTrace(HITRACE_TAG_MISC, "onCreate", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
}

void JsWallpaperExtensionAbility::OnStop()
{
    WallpaperExtensionAbility::OnStop();
    HILOG_INFO("jws JsWallpaperExtensionAbility OnStop begin.");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        HILOG_INFO("The wallpaper extension connection is not disconnected.");
    }
}

sptr<IRemoteObject> JsWallpaperExtensionAbility::OnConnect(const AAFwk::Want &want)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility OnConnect begin.");
    Extension::OnConnect(want);
    auto remoteObj = new (std::nothrow) WallpaperMgrService::WallpaperExtensionAbilityStub();
    if (remoteObj == nullptr) {
        HILOG_ERROR("failed to create IWallpaperExtensionAbility");
        return nullptr;
    }
    return remoteObj;
}

void JsWallpaperExtensionAbility::OnDisconnect(const AAFwk::Want &want)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility OnDisconnect begin.");
    Extension::OnDisconnect(want);
}

void JsWallpaperExtensionAbility::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility OnCommand begin.");
    Extension::OnCommand(want, restart, startId);
    HILOG_INFO("%{public}s begin restart=%{public}s,startId=%{public}d.", __func__, restart ? "true" : "false",
        startId);
}

napi_value JsWallpaperExtensionAbility::CallObjectMethod(const std::string &name, napi_value const *argv, size_t argc)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility::CallObjectMethod(%{public}s), begin", name.c_str());

    if (!jsObj_) {
        HILOG_WARN("Not found WallpaperExtensionAbility.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get WallpaperExtensionAbility object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name.c_str(), &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from WallpaperExtensionAbility object", name.c_str());
        return nullptr;
    }

    HILOG_INFO("JsWallpaperExtensionAbility::CallFunction(%{public}s), success", name.c_str());
    napi_value remoteNapi = nullptr;
    napi_status status = napi_call_function(env, obj, method, argc, argv, &remoteNapi);
    if (status != napi_ok) {
        HILOG_ERROR("JsWallpaperExtensionAbility::CallFunction(%{public}s), failed", name.c_str());
        return nullptr;
    }
    return remoteNapi;
}

void JsWallpaperExtensionAbility::GetSrcPath(std::string &srcPath)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility GetSrcPath begin.");
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

void JsWallpaperExtensionAbility::RegisterWallpaperCallback()
{
    WallpaperMgrService::WallpaperManagerkits::GetInstance().RegisterWallpaperCallback(
        [](int32_t wallpaperType) -> bool {
            HILOG_INFO("jsWallpaperExtensionAbility->CallObjectMethod");
            std::lock_guard<std::mutex> lock(mtx);
            if (JsWallpaperExtensionAbility::jsWallpaperExtensionAbility == nullptr) {
                return false;
            }
            napi_env napiEnv = (JsWallpaperExtensionAbility::jsWallpaperExtensionAbility->jsRuntime_).GetNapiEnv();
            WorkData *workData = new (std::nothrow) WorkData(napiEnv, nullptr, wallpaperType);
            if (workData == nullptr) {
                return false;
            }
            uv_after_work_cb afterCallback = [](uv_work_t *work, int32_t status) {
                WorkData *workData = reinterpret_cast<WorkData *>(work->data);
                if (workData == nullptr) {
                    delete work;
                    return;
                }
                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(workData->env_, &scope);
                if (scope == nullptr) {
                    delete workData;
                    delete work;
                    return;
                }
                napi_value type = OHOS::AppExecFwk::WrapInt32ToJS(workData->env_, workData->wallpaperType);

                napi_value arg[] = { type };
                std::lock_guard<std::mutex> lock(mtx);
                if (JsWallpaperExtensionAbility::jsWallpaperExtensionAbility != nullptr) {
                    JsWallpaperExtensionAbility::jsWallpaperExtensionAbility->CallObjectMethod(
                        "onWallpaperChange", arg, ARGC_ONE);
                }
                napi_close_handle_scope(workData->env_, scope);
                delete workData;
                delete work;
            };
            UvQueue::Call(napiEnv, workData, afterCallback);
            return true;
        });
}

} // namespace AbilityRuntime
} // namespace OHOS
