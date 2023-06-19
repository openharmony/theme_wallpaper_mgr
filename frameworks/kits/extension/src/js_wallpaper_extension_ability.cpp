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
constexpr size_t ARGC_TWO = 2;
} // namespace
struct WorkData {
    NativeEngine *nativeEng;
    int32_t wallpaperType;
    WorkData(NativeEngine *nativeEng, int32_t wallpaperType) : nativeEng(nativeEng), wallpaperType(wallpaperType)
    {
    }
};

struct OffsetWorkData {
    NativeEngine *nativeEng;
    int32_t xOffset;
    int32_t yOffset;
    OffsetWorkData(NativeEngine *nativeEng, int32_t xOffset, int32_t yOffset)
        : nativeEng(nativeEng), xOffset(xOffset), yOffset(yOffset)
    {
    }
};

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
    HILOG_INFO("JsWallpaperExtension::Init module:%{public}s,srcPath:%{public}s.", moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto &engine = jsRuntime_.GetNativeEngine();
    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        Extension::abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }
    HILOG_INFO("JsWallpaperExtensionAbility::Init ConvertNativeValueTo.");
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsWallpaperExtensionAbility object");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("JsWallpaperExtensionAbility::Init CreateJsWallpaperExtensionContext.");
    NativeValue *contextObj = CreateJsWallpaperExtensionContext(engine, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("WallpaperExtensionContext", &contextObj, ARGC_ONE);
    contextObj = shellContextRef->Get();
    context->Bind(jsRuntime_, shellContextRef.release());
    obj->SetProperty("context", contextObj);
    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get wallpaper extension native object");
        return;
    }
    nativeObj->SetNativePointer(
        new std::weak_ptr<AbilityRuntime::Context>(context),
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr wallpaper extension context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        }, nullptr);
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
    NativeEngine *nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue *nativeWant = reinterpret_cast<NativeValue *>(napiWant);
    NativeValue *argv[] = { nativeWant };
    StartAsyncTrace(HITRACE_TAG_MISC, "onCreated", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    CallObjectMethod("onCreated", argv, ARGC_ONE);
    FinishAsyncTrace(HITRACE_TAG_MISC, "onCreated", static_cast<int32_t>(TraceTaskId::ONCREATE_EXTENSION));
    CallObjectMethod("createWallpaperWin");
    RegisterWallpaperCallback();
    RegisterOffsetCallback();
    HILOG_INFO("%{public}s end.", __func__);
    FinishAsyncTrace(HITRACE_TAG_MISC, "onCreated", static_cast<int32_t>(TraceTaskId::ONSTART_EXTENSION));
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
    HILOG_INFO("%{public}s end.", __func__);
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
    HILOG_INFO("%{public}s end.", __func__);
}

NativeValue *JsWallpaperExtensionAbility::CallObjectMethod(
    const std::string &name, NativeValue *const *argv, size_t argc)
{
    HILOG_INFO("jws JsWallpaperExtensionAbility::CallObjectMethod(%{public}s), begin", name.c_str());

    if (!jsObj_) {
        HILOG_WARN("Not found WallpaperExtensionAbility.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get WallpaperExtensionAbility object");
        return nullptr;
    }

    NativeValue *method = obj->GetProperty(name.c_str());
    if (method == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from WallpaperExtensionAbility object", name.c_str());
        return nullptr;
    }
    HILOG_INFO("JsWallpaperExtensionAbility::CallFunction(%{public}s), success", name.c_str());
    return nativeEngine.CallFunction(value, method, argv, argc);
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
            NativeEngine *nativeEng =
                &(JsWallpaperExtensionAbility::jsWallpaperExtensionAbility->jsRuntime_).GetNativeEngine();
            WorkData *workData = new (std::nothrow) WorkData(nativeEng, wallpaperType);
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
                napi_open_handle_scope(reinterpret_cast<napi_env>(workData->nativeEng), &scope);
                if (scope == nullptr) {
                    delete workData;
                    delete work;
                    return;
                }
                napi_value type = OHOS::AppExecFwk::WrapInt32ToJS(reinterpret_cast<napi_env>(workData->nativeEng),
                    workData->wallpaperType);

                NativeValue *nativeType = reinterpret_cast<NativeValue *>(type);
                NativeValue *arg[] = { nativeType };
                std::lock_guard<std::mutex> lock(mtx);
                if (JsWallpaperExtensionAbility::jsWallpaperExtensionAbility != nullptr) {
                    JsWallpaperExtensionAbility::jsWallpaperExtensionAbility->CallObjectMethod(
                        "onWallpaperChanged", arg, ARGC_ONE);
                }
                napi_close_handle_scope(reinterpret_cast<napi_env>(workData->nativeEng), scope);
                delete workData;
                delete work;
            };
            UvQueue::Call(reinterpret_cast<napi_env>(nativeEng), workData, afterCallback);
            return true;
        });
}

void JsWallpaperExtensionAbility::RegisterOffsetCallback()
{
    WallpaperMgrService::WallpaperManagerkits::GetInstance().RegisterOffsetCallback([](int32_t xOffset,
                                                                                        int32_t yOffset) -> bool {
        HILOG_DEBUG("RegisterOffset start");
        std::lock_guard<std::mutex> lock(mtx);
        if (JsWallpaperExtensionAbility::jsWallpaperExtensionAbility == nullptr) {
            return false;
        }
        NativeEngine *nativeEng =
            &(JsWallpaperExtensionAbility::jsWallpaperExtensionAbility->jsRuntime_).GetNativeEngine();
        OffsetWorkData *workData = new (std::nothrow) OffsetWorkData(nativeEng, xOffset, yOffset);
        if (workData == nullptr) {
            return false;
        }
        uv_after_work_cb afterCallback = [](uv_work_t *work, int32_t status) {
            OffsetWorkData *workData = reinterpret_cast<OffsetWorkData *>(work->data);
            if (workData == nullptr) {
                delete work;
                return;
            }
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(reinterpret_cast<napi_env>(workData->nativeEng), &scope);
            if (scope == nullptr) {
                delete workData;
                delete work;
                return;
            }
            napi_value xOffset =
                OHOS::AppExecFwk::WrapInt32ToJS(reinterpret_cast<napi_env>(workData->nativeEng), workData->xOffset);
            napi_value yOffset =
                OHOS::AppExecFwk::WrapInt32ToJS(reinterpret_cast<napi_env>(workData->nativeEng), workData->yOffset);
            NativeValue *nativeX = reinterpret_cast<NativeValue *>(xOffset);
            NativeValue *nativeY = reinterpret_cast<NativeValue *>(yOffset);
            NativeValue *arg[] = { nativeX, nativeY };
            std::lock_guard<std::mutex> lock(mtx);
            if (JsWallpaperExtensionAbility::jsWallpaperExtensionAbility != nullptr) {
                JsWallpaperExtensionAbility::jsWallpaperExtensionAbility->CallObjectMethod("onOffset", arg, ARGC_TWO);
            }
            napi_close_handle_scope(reinterpret_cast<napi_env>(workData->nativeEng), scope);
            delete workData;
            delete work;
        };
        UvQueue::Call(reinterpret_cast<napi_env>(nativeEng), workData, afterCallback);
        return true;
    });
}

} // namespace AbilityRuntime
} // namespace OHOS
