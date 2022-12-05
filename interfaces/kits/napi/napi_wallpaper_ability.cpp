/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "napi_wallpaper_ability.h"

#include <map>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <uv.h>
#include <vector>

#include "hilog_wrapper.h"
#include "js_error.h"
#include "uv_queue.h"
#include "wallpaper_manager.h"
#include "wallpaper_manager_common_info.h"

using namespace OHOS::Media;
namespace OHOS {
namespace WallpaperNAPI {
const int32_t ONE = 1;
const int32_t TWO = 2;

struct WorkData {
    napi_env env_;
    napi_ref callback_;
    WorkData(napi_env env, napi_ref callback) : env_(env), callback_(callback)
    {
    }
};

napi_value NAPI_GetColors(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetColors in");
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::GetColorsInner(context);
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 1, false);
    return asyncCall.Call(env);
}

napi_value NAPI_GetColorsSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetColorsSync in");
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::GetColorsInner(context);
    AsyncCall asyncCall(env, info, context, 1, true);
    return asyncCall.SyncCall(env);
}

void NapiWallpaperAbility::GetColorsInner(std::shared_ptr<GetContextInfo> context)
{
    HILOG_DEBUG("GetColorsInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::CheckValidArgWallpaperType(env, argc, argv[0], context)) {
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input  wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value data = WallpaperJSUtil::Convert2JSRgbaArray(env, context->colors);
        HILOG_DEBUG("output  Convert2JSRgbaArray data != nullptr[%{public}d]", data != nullptr);
        *result = data;
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  GetColors");
        context->colors = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(context->wallpaperType);
        HILOG_DEBUG("exec  GetColors colors size : %{public}zu", context->colors.size());
        if (!context->colors.empty()) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetId(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::GetIdInner(context);
    AsyncCall asyncCall(env, info, context, 1, false);
    return asyncCall.Call(env);
}

napi_value NAPI_GetIdSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetIdSync in");
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::GetIdInner(context);
    AsyncCall asyncCall(env, info, context, 1, true);
    return asyncCall.SyncCall(env);
}

void NapiWallpaperAbility::GetIdInner(std::shared_ptr<GetContextInfo> context)
{
    HILOG_DEBUG("GetIdInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::CheckValidArgWallpaperType(env, argc, argv[0], context)) {
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input  wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->wallpaperId, result);
        HILOG_DEBUG("output  napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  GetWallpaperId");
        context->wallpaperId =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(context->wallpaperType);
        HILOG_DEBUG("exec  GetWallpaperId wallpaperId : %{public}d", context->wallpaperId);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetFile(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetFile in");
    auto context = std::make_shared<GetFileContextInfo>();
    NapiWallpaperAbility::GetFileInner(context);
    AsyncCall asyncCall(env, info, context, 1, false);
    return asyncCall.Call(env);
}

napi_value NAPI_GetFileSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetFileSync in");
    auto context = std::make_shared<GetFileContextInfo>();
    NapiWallpaperAbility::GetFileInner(context);
    AsyncCall asyncCall(env, info, context, 1, true);
    return asyncCall.SyncCall(env);
}

void NapiWallpaperAbility::GetFileInner(std::shared_ptr<GetFileContextInfo> context)
{
    HILOG_DEBUG("GetFileInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::CheckValidArgWallpaperType(env, argc, argv[0], context)) {
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input  wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value data = nullptr;
        napi_create_int32(env, context->wallpaperFd, &data);
        HILOG_DEBUG("output [%{public}d]", data != nullptr);
        *result = data;
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  GetFile");
        int32_t wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(context->wallpaperType,
                context->wallpaperFd);
        if (wallpaperErrorCode == static_cast<int32_t>(WallpaperMgrService::E_OK)) {
            context->status = napi_ok;
        } else {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code != 0) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("exec  GetFile fd: %{public}d", context->wallpaperFd);
        if (context->wallpaperFd >= 0) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetMinHeight(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinHeight in");
    auto context = std::make_shared<GetMinContextInfo>();
    NapiWallpaperAbility::GetMinHeightInner(context);
    AsyncCall asyncCall(env, info, context, 0, false);
    return asyncCall.Call(env);
}

napi_value NAPI_GetMinHeightSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinHeightSync in");
    auto context = std::make_shared<GetMinContextInfo>();
    NapiWallpaperAbility::GetMinHeightInner(context);
    AsyncCall asyncCall(env, info, context, 0, true);
    return asyncCall.SyncCall(env);
}

void NapiWallpaperAbility::GetMinHeightInner(std::shared_ptr<GetMinContextInfo> context)
{
    HILOG_DEBUG("GetMinHeightInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->minHeight, result);
        HILOG_DEBUG("output  napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  GetWallpaperMinHeight");
        context->minHeight = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinHeight();
        HILOG_DEBUG("exec  GetWallpaperMinHeight minHeight : %{public}d", context->minHeight);
        if (context->minHeight) {
            context->status = napi_ok;
        }
    };
    context->SetAction(nullptr, std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetMinWidth(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinWidth in");
    auto context = std::make_shared<GetMinContextInfo>();
    NapiWallpaperAbility::GetMinWidthInner(context);
    AsyncCall asyncCall(env, info, context, 0, false);
    return asyncCall.Call(env);
}

napi_value NAPI_GetMinWidthSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinWidthSync in");
    auto context = std::make_shared<GetMinContextInfo>();
    NapiWallpaperAbility::GetMinWidthInner(context);
    AsyncCall asyncCall(env, info, context, 0, true);
    return asyncCall.SyncCall(env);
}

void NapiWallpaperAbility::GetMinWidthInner(std::shared_ptr<GetMinContextInfo> context)
{
    HILOG_DEBUG("GetMinWidthInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->minHeight, result);
        HILOG_DEBUG("output  napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  GetWallpaperMinWidth");
        context->minWidth = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinWidth();
        HILOG_DEBUG("exec  GetWallpaperMinWidth minWidth : %{public}d", context->minWidth);
        if (context->minWidth) {
            context->status = napi_ok;
        }
    };
    context->SetAction(nullptr, std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_IsChangePermitted(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_IsChangePermitted in");
    auto context = std::make_shared<PermissionContextInfo>();
    NapiWallpaperAbility::IsChangeAllowedInner(context);
    AsyncCall asyncCall(env, info, context, 0, false);
    return asyncCall.Call(env);
}

napi_value NAPI_IsChangeAllowed(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_IsChangeAllowed in");
    auto context = std::make_shared<PermissionContextInfo>();
    NapiWallpaperAbility::IsChangeAllowedInner(context);
    AsyncCall asyncCall(env, info, context, 0, true);
    return asyncCall.SyncCall(env);
}

void NapiWallpaperAbility::IsChangeAllowedInner(std::shared_ptr<PermissionContextInfo> context)
{
    HILOG_DEBUG("IsChangeAllowedInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->isChangePermitted, result);
        HILOG_DEBUG("output  napi_get_boolean[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  IsChangePermitted");
        context->isChangePermitted = WallpaperMgrService::WallpaperManagerkits::GetInstance().IsChangePermitted();
        HILOG_DEBUG("exec  IsChangePermitted : %{public}d", context->isChangePermitted);
        context->status = napi_ok;
    };
    context->SetAction(nullptr, std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_IsOperationAllowed(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_IsOperationAllowed in");
    auto context = std::make_shared<PermissionContextInfo>();
    NapiWallpaperAbility::IsUserChangeAllowedInner(context);
    AsyncCall asyncCall(env, info, context, 0, false);
    return asyncCall.Call(env);
}

napi_value NAPI_IsUserChangeAllowed(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_IsUserChangeAllowed in");
    auto context = std::make_shared<PermissionContextInfo>();
    NapiWallpaperAbility::IsUserChangeAllowedInner(context);
    AsyncCall asyncCall(env, info, context, 0, true);
    return asyncCall.SyncCall(env);
}

void NapiWallpaperAbility::IsUserChangeAllowedInner(std::shared_ptr<PermissionContextInfo> context)
{
    HILOG_DEBUG("IsUserChangeAllowedInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->isOperationAllowed, result);
        HILOG_DEBUG("output  napi_get_boolean[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  IsOperationAllowed");
        context->isOperationAllowed = WallpaperMgrService::WallpaperManagerkits::GetInstance().IsOperationAllowed();
        HILOG_DEBUG("exec  IsOperationAllowed[%{public}d]", context->isOperationAllowed);
        context->status = napi_ok;
    };
    context->SetAction(nullptr, std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_Reset(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_Reset in");
    auto context = std::make_shared<SetContextInfo>();
    NapiWallpaperAbility::RestoreInner(context);
    AsyncCall asyncCall(env, info, context, 1, false);
    return asyncCall.Call(env);
}

napi_value NAPI_Restore(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_Restore in");
    auto context = std::make_shared<SetContextInfo>();
    NapiWallpaperAbility::RestoreInner(context);
    AsyncCall asyncCall(env, info, context, 1, true);
    return asyncCall.Call(env);
}

void NapiWallpaperAbility::RestoreInner(std::shared_ptr<SetContextInfo> context)
{
    HILOG_DEBUG("RestoreInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, 1) ||
            !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_number)) {
            HILOG_DEBUG("input  argc : %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, PARAMETERERRORMESSAGE);
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input  wallpaperType : %{public}d", context->wallpaperType);
        return napi_pending_exception;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  ResetWallpaper");
        int32_t wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(context->wallpaperType);
        HILOG_DEBUG("exec  ResetWallpaper[%{public}d]", wallpaperErrorCode);
        if (wallpaperErrorCode == static_cast<int32_t>(WallpaperMgrService::E_OK)) {
            context->status = napi_ok;
        } else {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code != 0) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("exec  status[%{public}d], context->status[%{public}d]", wallpaperErrorCode, context->status);
    };
    context->SetAction(std::move(input), nullptr);
    context->SetExecution(std::move(exec));
}

napi_value NAPI_SetWallpaper(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<SetContextInfo>();
    NapiWallpaperAbility::SetImageInput(context);
    NapiWallpaperAbility::SetImageExec(context);
    AsyncCall asyncCall(env, info, context, TWO, false);
    return asyncCall.Call(env);
}

napi_value NAPI_SetImage(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<SetContextInfo>();
    NapiWallpaperAbility::SetImageInput(context);
    NapiWallpaperAbility::SetImageExec(context);
    AsyncCall asyncCall(env, info, context, TWO, true);
    return asyncCall.Call(env);
}

void NapiWallpaperAbility::SetImageInput(std::shared_ptr<SetContextInfo> context)
{
    HILOG_DEBUG("SetImageInput in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, TWO) ||
            (!NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string) &&
                !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_object)) ||
            !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_number)) {
            HILOG_DEBUG("input  argc : %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, PARAMETERERRORMESSAGE);
            return napi_invalid_arg;
        }
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType == napi_string) {
            context->url = WallpaperJSUtil::Convert2String(env, argv[0]);
        } else {
            std::shared_ptr<PixelMap> tmpPixelMap = PixelMapNapi::GetPixelMap(env, argv[0]);
            if (tmpPixelMap == nullptr) {
                HILOG_ERROR("PixelMapNapi::GetPixelMap error");
                context->isPixelEmp = true;
                return napi_generic_failure;
            } else {
                context->isPixelEmp = false;
            }
            context->pixelMap = std::make_unique<PixelMap>(*tmpPixelMap);
        }
        napi_get_value_int32(env, argv[1], &context->wallpaperType);
        HILOG_DEBUG("input  wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    context->SetAction(std::move(input), nullptr);
}

void NapiWallpaperAbility::SetImageExec(std::shared_ptr<SetContextInfo> context)
{
    HILOG_DEBUG("SetImageExec in");
    auto exec = [context](AsyncCall::Context *ctx) {
        int32_t wallpaperErrorCode = 0;
        if (context->url.length() == 0) {
            HILOG_DEBUG("exec  setWallpaper by pixelMap");
            if (!context->isPixelEmp) {
                wallpaperErrorCode =
                    WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(context->pixelMap,
                        context->wallpaperType);
            }
        } else {
            HILOG_DEBUG("exec  setWallpaper by URL");
            wallpaperErrorCode = WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(context->url,
                context->wallpaperType);
        }
        if (wallpaperErrorCode == static_cast<int32_t>(WallpaperMgrService::E_OK)) {
            context->status = napi_ok;
        } else {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code != 0) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("exec  context->status[%{public}d]", context->status);
    };
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetPixelMap(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetPixelMap in");
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::GetImageInner(context);
    AsyncCall asyncCall(env, info, context, 1, false);
    return asyncCall.Call(env);
}

napi_value NAPI_GetImage(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetImage in");
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::GetImageInner(context);
    AsyncCall asyncCall(env, info, context, 1, true);
    return asyncCall.Call(env);
}

void NapiWallpaperAbility::GetImageInner(std::shared_ptr<GetContextInfo> context)
{
    HILOG_DEBUG("GetImageInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, 1) ||
            !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_number)) {
            HILOG_DEBUG("input  argc : %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, PARAMETERERRORMESSAGE);
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input  wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value pixelVal = PixelMapNapi::CreatePixelMap(env, std::move(context->pixelMap));
        HILOG_DEBUG("output  PixelMapNapi::CreatePixelMap != nullptr[%{public}d]", pixelVal != nullptr);
        *result = pixelVal;
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec  GetImageInner");
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
        int32_t wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(context->wallpaperType, pixelMap);
        HILOG_DEBUG("exec  wallpaperErrorCode[%{public}d]", wallpaperErrorCode);
        if (wallpaperErrorCode == static_cast<int32_t>(WallpaperMgrService::E_OK) && pixelMap != nullptr) {
            context->status = napi_ok;
            context->pixelMap = std::move(pixelMap);
        } else {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code != 0) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_On(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_On in");
    size_t argc = TWO;
    napi_value argv[TWO] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (!NapiWallpaperAbility::IsValidArgCount(argc, TWO) ||
        !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string) ||
        !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_function)) {
        HILOG_DEBUG("input  argc : %{public}zu", argc);
        JsError::ThrowError(env, ErrorThrowType::PARAMETER_ERROR, PARAMETERERRORMESSAGE);
        return nullptr;
    }
    std::string type = WallpaperJSUtil::Convert2String(env, argv[0]);
    HILOG_DEBUG("type : %{public}s", type.c_str());

    std::shared_ptr<WallpaperMgrService::WallpaperColorChangeListener> listener =
        std::make_shared<NapiWallpaperAbility>(env, argv[1]);

    bool status = WallpaperMgrService::WallpaperManagerkits::GetInstance().On(type, listener);
    if (!status) {
        HILOG_ERROR("WallpaperMgrService::WallpaperManagerkits::GetInstance().On failed!");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NAPI_Off(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_Off in");
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (!NapiWallpaperAbility::IsValidArgCount(argc, ONE) ||
        !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)) {
        HILOG_DEBUG("input  argc : %{public}zu", argc);
        JsError::ThrowError(env, ErrorThrowType::PARAMETER_ERROR, PARAMETERERRORMESSAGE);
        return nullptr;
    }
    std::string type = WallpaperJSUtil::Convert2String(env, argv[0]);
    HILOG_DEBUG("type : %{public}s", type.c_str());

    std::shared_ptr<WallpaperMgrService::WallpaperColorChangeListener> listener = nullptr;
    if (argc == TWO) {
        if (!NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_function)) {
            JsError::ThrowError(env, ErrorThrowType::PARAMETER_ERROR, PARAMETERERRORMESSAGE);
            return nullptr;
        }
        listener = std::make_shared<NapiWallpaperAbility>(env, argv[1]);
    }

    bool status = WallpaperMgrService::WallpaperManagerkits::GetInstance().Off(type, listener);
    if (!status) {
        HILOG_ERROR("WallpaperMgrService::WallpaperManagerkits::GetInstance().Off failed!");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

NapiWallpaperAbility::NapiWallpaperAbility(napi_env env, napi_value callback) : env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

NapiWallpaperAbility::~NapiWallpaperAbility()
{
    WorkData *workData = new WorkData(env_, callback_);
    uv_after_work_cb afterCallback = [](uv_work_t *work, int status) {
        WorkData *workData = reinterpret_cast<WorkData *>(work->data);
        napi_delete_reference(workData->env_, workData->callback_);
        delete work;
        work = nullptr;
    };
    MiscServices::UvQueue::Call(env_, workData, afterCallback);
}

void NapiWallpaperAbility::OnColorsChange(const std::vector<uint64_t> &color, int wallpaperType)
{
    WallpaperMgrService::WallpaperColorChangeListener::OnColorsChange(color, wallpaperType);
    EventDataWorker *eventDataWorker = new EventDataWorker(this, color, wallpaperType);
    uv_work_t *work = new uv_work_t;
    work->data = eventDataWorker;
    uv_queue_work(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            EventDataWorker *eventDataInner = reinterpret_cast<EventDataWorker *>(work->data);
            napi_value jsWallpaperType = nullptr;
            napi_create_int32(eventDataInner->listener->env_, eventDataInner->wallpaperType, &jsWallpaperType);
            napi_value jsRgbaArray =
                WallpaperJSUtil::Convert2JSRgbaArray(eventDataInner->listener->env_, eventDataInner->color);
            napi_value callback = nullptr;
            napi_value args[2] = { jsRgbaArray, jsWallpaperType };
            napi_get_reference_value(eventDataInner->listener->env_, eventDataInner->listener->callback_, &callback);
            napi_value global = nullptr;
            napi_get_global(eventDataInner->listener->env_, &global);
            napi_value result;
            napi_status callStatus =
                napi_call_function(eventDataInner->listener->env_, global, callback, 2, args, &result);
            if (callStatus != napi_ok) {
                HILOG_ERROR("notify data change failed callStatus:%{public}d callback:%{public}p", callStatus,
                    callback);
            }
            delete eventDataInner;
            eventDataInner = nullptr;
            delete work;
            work = nullptr;
        });
}

bool NapiWallpaperAbility::IsValidArgCount(size_t argc, size_t expectationSize)
{
    return (argc < expectationSize) ? false : true;
}

bool NapiWallpaperAbility::IsValidArgType(napi_env env, napi_value argValue, napi_valuetype expectationType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argValue, &valueType);
    return (valueType != expectationType) ? false : true;
}

bool NapiWallpaperAbility::IsValidArgRange(napi_env env, napi_value argValue)
{
    int wallpaperType;
    napi_get_value_int32(env, argValue, &wallpaperType);
    return (wallpaperType != WALLPAPER_LOCKSCREEN && wallpaperType != WALLPAPER_SYSTEM) ? false : true;
}

bool NapiWallpaperAbility::CheckValidArgWallpaperType(napi_env env, size_t argc, napi_value argValue,
    std::shared_ptr<AsyncCall::Context> ctx)
{
    if (!NapiWallpaperAbility::IsValidArgCount(argc, ONE) ||
        !NapiWallpaperAbility::IsValidArgType(env, argValue, napi_number) ||
        !NapiWallpaperAbility::IsValidArgRange(env, argValue)) {
        HILOG_DEBUG("input  argc : %{public}zu", argc);
        ctx->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, PARAMETERERRORMESSAGE);
        return false;
    }
    return true;
}
} // namespace WallpaperNAPI
} // namespace OHOS
