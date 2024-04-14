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

#include "js_wallpaper_extension_context.h"

#include <cstdint>
#include <mutex>

#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;

static std::map<ConnecttionKey, sptr<JSWallpaperExtensionConnection>, key_compare> connects_;
static std::mutex g_connectMapMtx;
static int64_t g_serialNumber = 0;
static std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;

class JsWallpaperExtensionContext final {
public:
    explicit JsWallpaperExtensionContext(const std::shared_ptr<WallpaperExtensionContext> &context) : context_(context)
    {
    }
    ~JsWallpaperExtensionContext() = default;
    JsWallpaperExtensionContext() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        HILOG_INFO("JsAbilityContext::Finalizer is called");
        std::unique_ptr<JsWallpaperExtensionContext>(static_cast<JsWallpaperExtensionContext *>(data));
    }

    static napi_value StartAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsWallpaperExtensionContext, OnStartAbility);
    }

    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsWallpaperExtensionContext, OnStartAbilityWithAccount);
    }

    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsWallpaperExtensionContext, OnConnectAbilityWithAccount);
    }

    static napi_value TerminateAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsWallpaperExtensionContext, OnTerminateAbility);
    }

    static napi_value ConnectAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsWallpaperExtensionContext, OnConnectAbility);
    }

    static napi_value DisconnectAbility(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsWallpaperExtensionContext, OnDisconnectAbility);
    }

private:
    std::weak_ptr<WallpaperExtensionContext> context_;

    napi_value OnStartAbility(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_INFO("OnStartAbility is called");
        // only support one or two or three params
        if (argc != ARGC_ONE && argc != ARGC_TWO && argc != ARGC_THREE) {
            HILOG_ERROR("Not enough params");
            return CreateJsUndefined(env);
        }

        decltype(argc) unwrapArgc = 0;
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s", __func__, want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;

        AAFwk::StartOptions startOptions;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[INDEX_ONE], &valueType);
        if (argc > ARGC_ONE && valueType == napi_object) {
            HILOG_INFO("OnStartAbility start options is used.");
            AppExecFwk::UnwrapStartOptions(env, argv[INDEX_ONE], startOptions);
            unwrapArgc++;
        }

        NapiAsyncTask::CompleteCallback complete = [weak = context_, want, startOptions, unwrapArgc](napi_env env,
                                                       NapiAsyncTask &task, int32_t status) {
            HILOG_INFO("startAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                return;
            }

            ErrCode errcode = ERR_OK;
            (unwrapArgc == 1) ? errcode = context->StartAbility(want)
                              : errcode = context->StartAbility(want, startOptions);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsError(env, errcode, "Start Ability failed."));
            }
        };

        napi_value lastParam = (argc > unwrapArgc) ? argv[unwrapArgc] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("WallpaperExtensionContext::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnStartAbilityWithAccount(napi_env env, size_t argc, napi_value *argv)
    {
        // only support two or three or four params
        if (argc != ARGC_TWO && argc != ARGC_THREE && argc != ARGC_FOUR) {
            HILOG_ERROR("Not enough params");
            return CreateJsUndefined(env);
        }

        decltype(argc) unwrapArgc = 0;
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s", __func__, want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        unwrapArgc++;

        int32_t accountId = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, argv[INDEX_ONE], accountId)) {
            HILOG_ERROR("%{public}s called, the second parameter is invalid.", __func__);
            return CreateJsUndefined(env);
        }
        HILOG_INFO("%{public}d accountId:", accountId);
        unwrapArgc++;

        AAFwk::StartOptions startOptions;
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[INDEX_ONE], &valueType);
        if (argc > ARGC_TWO && valueType == napi_object) {
            HILOG_INFO("OnStartAbilityWithAccount start options is used.");
            AppExecFwk::UnwrapStartOptions(env, argv[INDEX_TWO], startOptions);
            unwrapArgc++;
        }

        NapiAsyncTask::CompleteCallback complete = [weak = context_, want, accountId, startOptions,
                                                       unwrapArgc](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                return;
            }

            ErrCode errcode = ERR_OK;
            (unwrapArgc == ARGC_TWO) ? errcode = context->StartAbilityWithAccount(want, accountId)
                                     : errcode = context->StartAbilityWithAccount(want, accountId, startOptions);
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsError(env, errcode, "Start Ability failed."));
            }
        };

        napi_value lastParam = (argc == unwrapArgc) ? nullptr : argv[unwrapArgc];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("WallpaperExtensionContext::OnStartAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnTerminateAbility(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_INFO("OnTerminateAbility is called");
        // only support one or zero params
        if (argc != ARGC_ZERO && argc != ARGC_ONE) {
            HILOG_ERROR("Not enough params");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete = [weak = context_](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HILOG_INFO("TerminateAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                return;
            }

            auto errcode = context->TerminateAbility();
            if (errcode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsError(env, errcode, "Terminate Ability failed."));
            }
        };

        napi_value lastParam = (argc == ARGC_ZERO) ? nullptr : argv[INDEX_ZERO];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("WallpaperExtensionContext::OnTerminateAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnConnectAbility(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_INFO("OnConnectAbility is called");
        // only support two params
        if (argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return CreateJsUndefined(env);
        }

        // unwrap want
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s", __func__, want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        // unwarp connection
        sptr<JSWallpaperExtensionConnection> connection = new JSWallpaperExtensionConnection(env);
        int64_t connectId = GetConnectId(argv, want, connection);
        NapiAsyncTask::CompleteCallback complete = [weak = context_, want, connection, connectId](napi_env env,
                                                       NapiAsyncTask &task, int32_t status) {
            HILOG_INFO("OnConnectAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                return;
            }
            HILOG_INFO("context->ConnectAbility connection:%{public}d", (int32_t)connectId);
            if (!context->ConnectAbility(want, connection)) {
                connection->CallJsFailed(ERROR_CODE_ONE);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("WallpaperExtensionContext::OnConnectAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
        napi_value connectResult = nullptr;
        napi_create_int64(env, connectId, &connectResult);
        return connectResult;
    }

    napi_value OnConnectAbilityWithAccount(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_INFO("OnConnectAbilityWithAccount is called");
        if (argc != ARGC_THREE) {
            HILOG_ERROR("Not enough params");
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        OHOS::AppExecFwk::UnwrapWant(env, argv[INDEX_ZERO], want);
        HILOG_INFO("%{public}s bundlename:%{public}s abilityname:%{public}s", __func__, want.GetBundle().c_str(),
            want.GetElement().GetAbilityName().c_str());
        int32_t accountId = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, argv[INDEX_ONE], accountId)) {
            HILOG_ERROR("%{public}s called, the second parameter is invalid.", __func__);
            return CreateJsUndefined(env);
        }
        sptr<JSWallpaperExtensionConnection> connection = new JSWallpaperExtensionConnection(env);
        int64_t connectId = GetConnectId(argv, want, connection);
        NapiAsyncTask::CompleteCallback complete = [weak = context_, want, accountId, connection,
                                                       connectId](napi_env env, NapiAsyncTask &task, int32_t status) {
            HILOG_INFO("OnConnectAbilityWithAccount begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                return;
            }
            HILOG_INFO("context->ConnectAbilityWithAccount connection:%{public}d", (int32_t)connectId);
            if (!context->ConnectAbilityWithAccount(want, accountId, connection)) {
                connection->CallJsFailed(ERROR_CODE_ONE);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("WallpaperExtensionContext::OnConnectAbilityWithAccount", env,
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
        napi_value connectResult = nullptr;
        napi_create_int64(env, connectId, &connectResult);
        return connectResult;
    }

    static int64_t GetConnectId(const napi_value *argv, const AAFwk::Want &want,
        const sptr<JSWallpaperExtensionConnection> &connection)
    {
        connection->SetJsConnectionObject(argv[1]);
        int64_t connectId = g_serialNumber;
        ConnecttionKey key;
        key.id = g_serialNumber;
        key.want = want;
        {
            std::lock_guard<std::mutex> lock(g_connectMapMtx);
            connects_.emplace(key, connection);
        }
        if (g_serialNumber < INT64_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
        HILOG_INFO("%{public}s not find connection, make new one.", __func__);
        return connectId;
    }

    napi_value OnDisconnectAbility(napi_env env, size_t argc, napi_value *argv)
    {
        HILOG_INFO("OnDisconnectAbility is called");
        if (argc != ARGC_ONE && argc != ARGC_TWO) {
            HILOG_ERROR("Not enough params");
            return CreateJsUndefined(env);
        }

        AAFwk::Want want;
        int64_t connectId = -1;
        sptr<JSWallpaperExtensionConnection> connection = nullptr;
        napi_get_value_int64(env, reinterpret_cast<napi_value>(argv[INDEX_ZERO]), &connectId);
        HILOG_INFO("OnDisconnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
        {
            std::lock_guard<std::mutex> lock(g_connectMapMtx);
            auto item = std::find_if(connects_.begin(), connects_.end(),
                [&connectId](const std::map<ConnecttionKey, sptr<JSWallpaperExtensionConnection>>::value_type &obj) {
                    return connectId == obj.first.id;
                });
            if (item != connects_.end()) {
                want = item->first.want;
                connection = item->second;
                HILOG_INFO("%{public}s find conn ability exist", __func__);
            } else {
                HILOG_INFO("%{public}s not find conn exist.", __func__);
            }
        }
        NapiAsyncTask::CompleteCallback complete = [weak = context_, want, connection](napi_env env,
                                                       NapiAsyncTask &task, int32_t status) {
            HILOG_INFO("OnDisconnectAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "Context is released"));
                return;
            }
            if (connection == nullptr) {
                HILOG_WARN("connection nullptr");
                task.Reject(env, CreateJsError(env, ERROR_CODE_TWO, "not found connection"));
                return;
            }
            auto errcode = context->DisconnectAbility(want, connection);
            errcode == 0 ? task.Resolve(env, CreateJsUndefined(env))
                         : task.Reject(env, CreateJsError(env, errcode, "Disconnect Ability failed."));
        };

        napi_value lastParam = (argc == ARGC_ONE) ? nullptr : argv[INDEX_ONE];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("WallpaperExtensionContext::OnDisconnectAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

napi_value CreateJsMetadata(napi_env env, const AppExecFwk::Metadata &info)
{
    HILOG_INFO("CreateJsMetadata");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "name", CreateJsValue(env, info.name));
    napi_set_named_property(env, objValue, "value", CreateJsValue(env, info.value));
    napi_set_named_property(env, objValue, "resource", CreateJsValue(env, info.resource));
    return objValue;
}

napi_value CreateJsMetadataArray(napi_env env, const std::vector<AppExecFwk::Metadata> &info)
{
    HILOG_INFO("CreateJsMetadataArray");
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, info.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &item : info) {
        napi_set_element(env, arrayValue, index++, CreateJsMetadata(env, item));
    }
    return arrayValue;
}

napi_value CreateJsExtensionAbilityInfo(napi_env env, const AppExecFwk::ExtensionAbilityInfo &info)
{
    HILOG_INFO("CreateJsExtensionAbilityInfo");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, info.bundleName));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, info.moduleName));
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, info.name));
    napi_set_named_property(env, objValue, "labelId", CreateJsValue(env, info.labelId));
    napi_set_named_property(env, objValue, "descriptionId", CreateJsValue(env, info.descriptionId));
    napi_set_named_property(env, objValue, "iconId", CreateJsValue(env, info.iconId));
    napi_set_named_property(env, objValue, "isVisible", CreateJsValue(env, info.visible));
    napi_set_named_property(env, objValue, "extensionAbilityType", CreateJsValue(env, info.type));

    napi_value permissionArrayValue = nullptr;
    napi_create_array_with_length(env, info.permissions.size(), &permissionArrayValue);

    if (permissionArrayValue != nullptr) {
        int32_t index = 0;
        for (auto permission : info.permissions) {
            napi_set_element(env, permissionArrayValue, index++, CreateJsValue(env, permission));
        }
    }
    napi_set_named_property(env, objValue, "permissions", permissionArrayValue);
    napi_set_named_property(env, objValue, "applicationInfo", CreateJsApplicationInfo(env, info.applicationInfo));
    napi_set_named_property(env, objValue, "metadata", CreateJsMetadataArray(env, info.metadata));
    napi_set_named_property(env, objValue, "enabled", CreateJsValue(env, info.enabled));
    napi_set_named_property(env, objValue, "readPermission", CreateJsValue(env, info.readPermission));
    napi_set_named_property(env, objValue, "writePermission", CreateJsValue(env, info.writePermission));
    return objValue;
}

napi_value CreateJsWallpaperExtensionContext(napi_env env, std::shared_ptr<WallpaperExtensionContext> context)
{
    HILOG_INFO("CreateJsWallpaperExtensionContext begin");
    napi_value objValue = CreateJsExtensionContext(env, context);

    std::unique_ptr<JsWallpaperExtensionContext> jsContext = std::make_unique<JsWallpaperExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), JsWallpaperExtensionContext::Finalizer, nullptr, nullptr);

    // make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    const char *moduleName = "JsWallpaperExtensionContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, JsWallpaperExtensionContext::StartAbility);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, JsWallpaperExtensionContext::TerminateAbility);
    BindNativeFunction(env, objValue, "connectAbility", moduleName, JsWallpaperExtensionContext::ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectAbility", moduleName, JsWallpaperExtensionContext::DisconnectAbility);
    BindNativeFunction(env, objValue, "startAbilityWithAccount", moduleName,
        JsWallpaperExtensionContext::StartAbilityWithAccount);
    BindNativeFunction(env, objValue, "connectAbilityWithAccount", moduleName,
        JsWallpaperExtensionContext::ConnectAbilityWithAccount);

    if (context) {
        HILOG_INFO("Set ExtensionAbilityInfo Property");
        auto abilityInfo = context->GetAbilityInfo();
        auto hapModuleInfo = context->GetHapModuleInfo();
        if (abilityInfo && hapModuleInfo) {
            auto isExist = [&abilityInfo](const AppExecFwk::ExtensionAbilityInfo &info) {
                HILOG_INFO("%{public}s, %{public}s", info.bundleName.c_str(), info.name.c_str());
                return info.bundleName == abilityInfo->bundleName && info.name == abilityInfo->name;
            };
            auto infoIter =
                std::find_if(hapModuleInfo->extensionInfos.begin(), hapModuleInfo->extensionInfos.end(), isExist);
            if (infoIter == hapModuleInfo->extensionInfos.end()) {
                HILOG_ERROR("Get target fail.");
                return objValue;
            }
            napi_set_named_property(env, objValue, "extensionAbilityInfo",
                CreateJsExtensionAbilityInfo(env, *infoIter));
        }
    }

    return objValue;
}

JSWallpaperExtensionConnection::JSWallpaperExtensionConnection(napi_env env) : env_(env)
{
}

JSWallpaperExtensionConnection::~JSWallpaperExtensionConnection() = default;

void JSWallpaperExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    HILOG_INFO("OnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ nullptr");
        return;
    }
    wptr<JSWallpaperExtensionConnection> connection = this;
    auto task = [connection, element, remoteObject, resultCode]() {
        sptr<JSWallpaperExtensionConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_ERROR("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
    };
    handler_->PostTask(task, "OnAbilityConnectDone");
}

void JSWallpaperExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    HILOG_INFO("HandleOnAbilityConnectDone begin, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    HILOG_INFO("OnAbilityConnectDone begin NAPI_ohos_rpc_CreateJsRemoteObject");
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = { napiElementName, napiRemoteObject };

    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        HILOG_ERROR("Failed to get onConnect from object");
        return;
    }
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, &callResult);
}

void JSWallpaperExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    HILOG_INFO("OnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    if (handler_ == nullptr) {
        HILOG_ERROR("handler_ nullptr");
        return;
    }
    wptr<JSWallpaperExtensionConnection> connection = this;
    auto task = [connection, element, resultCode]() {
        sptr<JSWallpaperExtensionConnection> connectionSptr = connection.promote();
        if (!connectionSptr) {
            HILOG_ERROR("connectionSptr nullptr");
            return;
        }
        connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
    };
    handler_->PostTask(task, "OnAbilityDisconnectDone");
}

void JSWallpaperExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int32_t resultCode)
{
    HILOG_INFO("HandleOnAbilityDisconnectDone begin, resultCode:%{public}d", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = { napiElementName };
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from object");
        return;
    }

    // release connect
    HILOG_INFO("OnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    {
        std::lock_guard<std::mutex> lock(g_connectMapMtx);
        auto item = std::find_if(connects_.begin(), connects_.end(),
            [bundleName, abilityName](
                const std::map<ConnecttionKey, sptr<JSWallpaperExtensionConnection>>::value_type &obj) {
                return (bundleName == obj.first.want.GetBundle()) &&
                       (abilityName == obj.first.want.GetElement().GetAbilityName());
            });
        if (item != connects_.end()) {
            // match bundlename && abilityname
            connects_.erase(item);
            HILOG_INFO("OnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
        }
    }
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ARGC_TWO, argv, &callResult);
}

void JSWallpaperExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref value = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &value);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(value));
}

void JSWallpaperExtensionConnection::CallJsFailed(int32_t errorCode)
{
    HILOG_INFO("CallJsFailed begin");
    if (jsConnectionObject_ == nullptr) {
        HILOG_ERROR("jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onFailed from object");
        return;
    }
    napi_value result = nullptr;
    napi_create_int32(env_, errorCode, &result);
    napi_value argv[] = { result };

    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ARGC_ONE, argv, &callResult);
}
} // namespace AbilityRuntime
} // namespace OHOS