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

#include <pthread.h>
#include <unistd.h>
#include <uv.h>

#include <map>
#include <string>
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
const int32_t EVENTTYPESIZE = 64;
const int32_t BUFFERSIZE = 100;
const int32_t PARAMETER_ERROR_CODE = 401;
constexpr const char *COLOR_CHANGE_EVENT = "colorChange";
constexpr const char *WALLPAPER_CHANGE_EVENT = "wallpaperChange";

napi_value NAPI_GetColors(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetColors in");
    auto context = std::make_shared<GetContextInfo>();
    ApiInfo apiInfo{ false, false };
    NapiWallpaperAbility::GetColorsInner(context, apiInfo);
    Call call(env, info, std::dynamic_pointer_cast<Call::Context>(context), 1, apiInfo.needException);
    return call.AsyncCall(env, "getColors");
}

napi_value NAPI_GetColorsSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetColorsSync in");
    auto context = std::make_shared<GetContextInfo>();
    ApiInfo apiInfo{ true, true };
    NapiWallpaperAbility::GetColorsInner(context, apiInfo);
    Call call(env, info, context, 1, apiInfo.needException);
    return call.SyncCall(env);
}

void NapiWallpaperAbility::GetColorsInner(std::shared_ptr<GetContextInfo> context, const ApiInfo &apiInfo)
{
    HILOG_DEBUG("GetColorsInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::CheckValidArgWallpaperType(env, argc, argv[0], context)) {
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value data = WallpaperJSUtil::Convert2JSRgbaArray(env, context->colors);
        HILOG_DEBUG("output Convert2JSRgbaArray data != nullptr[%{public}d]", data != nullptr);
        *result = data;
        return napi_ok;
    };
    auto exec = [context, apiInfo](Call::Context *ctx) {
        HILOG_DEBUG("exec GetColors");
        ErrorCode wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(context->wallpaperType, apiInfo,
                context->colors);
        if (wallpaperErrorCode == E_OK && !context->colors.empty()) {
            context->status = napi_ok;
            return;
        }
        if (apiInfo.needException) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code != 0) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("exec GetColors colors size : %{public}zu", context->colors.size());
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetId(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::GetIdInner(context);
    Call call(env, info, context, 1, false);
    return call.AsyncCall(env, "getId");
}

void NapiWallpaperAbility::GetIdInner(std::shared_ptr<GetContextInfo> context)
{
    HILOG_DEBUG("GetIdInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::CheckValidArgWallpaperType(env, argc, argv[0], context)) {
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->wallpaperId, result);
        HILOG_DEBUG("output napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context](Call::Context *ctx) {
        HILOG_DEBUG("exec GetWallpaperId");
        context->wallpaperId =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(context->wallpaperType);
        HILOG_DEBUG("exec GetWallpaperId wallpaperId : %{public}d", context->wallpaperId);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetFile(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetFile in");
    auto context = std::make_shared<GetFileContextInfo>();
    ApiInfo apiInfo{ false, false };
    NapiWallpaperAbility::GetFileInner(context, apiInfo);
    Call call(env, info, context, 1, apiInfo.needException);
    return call.AsyncCall(env, "getFile");
}

void NapiWallpaperAbility::GetFileInner(std::shared_ptr<GetFileContextInfo> context, const ApiInfo &apiInfo)
{
    HILOG_DEBUG("GetFileInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::CheckValidArgWallpaperType(env, argc, argv[0], context)) {
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value data = nullptr;
        napi_create_int32(env, context->wallpaperFd, &data);
        HILOG_DEBUG("output [%{public}d]", data != nullptr);
        *result = data;
        return napi_ok;
    };
    auto exec = [context, apiInfo](Call::Context *ctx) {
        HILOG_DEBUG("exec GetFile");
        ErrorCode wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(context->wallpaperType,
                context->wallpaperFd);
        if (wallpaperErrorCode == E_OK && context->wallpaperFd >= 0) {
            context->status = napi_ok;
            return;
        }
        if (apiInfo.needException) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code == PARAMETER_ERROR_CODE) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message + WALLPAPERTYPE_PARAMETER_TYPE);
            } else {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }

        HILOG_DEBUG("exec GetFile fd: %{public}d", context->wallpaperFd);
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetMinHeight(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinHeight in");
    auto context = std::make_shared<GetMinContextInfo>();
    ApiInfo apiInfo{ false, false };
    NapiWallpaperAbility::GetMinHeightInner(context, apiInfo);
    Call call(env, info, context, 0, apiInfo.needException);
    return call.AsyncCall(env, "getMinHeight");
}

napi_value NAPI_GetMinHeightSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinHeightSync in");
    auto context = std::make_shared<GetMinContextInfo>();
    ApiInfo apiInfo{ true, true };
    NapiWallpaperAbility::GetMinHeightInner(context, apiInfo);
    Call call(env, info, context, 0, apiInfo.needException);
    return call.SyncCall(env);
}

void NapiWallpaperAbility::GetMinHeightInner(std::shared_ptr<GetMinContextInfo> context, const ApiInfo &apiInfo)
{
    HILOG_DEBUG("GetMinHeightInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->minHeight, result);
        HILOG_DEBUG("output  napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context, apiInfo](Call::Context *ctx) {
        HILOG_DEBUG("exec GetWallpaperMinHeight");
        ErrorCode wallpaperErrorCode = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinHeight(
            apiInfo, context->minHeight);
        if (wallpaperErrorCode == E_OK && context->minHeight >= 0) {
            context->status = napi_ok;
            return;
        }
        if (apiInfo.needException) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code != 0) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
    };
    context->SetAction(nullptr, std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetMinWidth(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinWidth in");
    auto context = std::make_shared<GetMinContextInfo>();
    ApiInfo apiInfo{ false, false };
    NapiWallpaperAbility::GetMinWidthInner(context, apiInfo);
    Call call(env, info, context, 0, apiInfo.needException);
    return call.AsyncCall(env, "getMinWidth");
}

napi_value NAPI_GetMinWidthSync(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinWidthSync in");
    auto context = std::make_shared<GetMinContextInfo>();
    ApiInfo apiInfo{ true, true };
    NapiWallpaperAbility::GetMinWidthInner(context, apiInfo);
    Call call(env, info, context, 0, apiInfo.needException);
    return call.SyncCall(env);
}

void NapiWallpaperAbility::GetMinWidthInner(std::shared_ptr<GetMinContextInfo> context, const ApiInfo &apiInfo)
{
    HILOG_DEBUG("GetMinWidthInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->minWidth, result);
        HILOG_DEBUG("output  napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context, apiInfo](Call::Context *ctx) {
        HILOG_DEBUG("exec GetWallpaperMinWidth");
        ErrorCode wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinWidth(apiInfo, context->minWidth);
        if (wallpaperErrorCode == E_OK && context->minWidth >= 0) {
            context->status = napi_ok;
        }
        if (apiInfo.needException) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code != 0) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
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
    Call call(env, info, context, 0, false);
    return call.AsyncCall(env, "isChangePermitted");
}

void NapiWallpaperAbility::IsChangeAllowedInner(std::shared_ptr<PermissionContextInfo> context)
{
    HILOG_DEBUG("IsChangeAllowedInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->isChangePermitted, result);
        HILOG_DEBUG("output napi_get_boolean[%{public}d]", status);
        return status;
    };
    auto exec = [context](Call::Context *ctx) {
        HILOG_DEBUG("exec IsChangePermitted");
        context->isChangePermitted = WallpaperMgrService::WallpaperManagerkits::GetInstance().IsChangePermitted();
        HILOG_DEBUG("exec IsChangePermitted : %{public}d", context->isChangePermitted);
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
    Call call(env, info, context, 0, false);
    return call.AsyncCall(env, "isOperationAllowed");
}

void NapiWallpaperAbility::IsUserChangeAllowedInner(std::shared_ptr<PermissionContextInfo> context)
{
    HILOG_DEBUG("IsUserChangeAllowedInner in");
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->isOperationAllowed, result);
        HILOG_DEBUG("output napi_get_boolean[%{public}d]", status);
        return status;
    };
    auto exec = [context](Call::Context *ctx) {
        HILOG_DEBUG("exec IsOperationAllowed");
        context->isOperationAllowed = WallpaperMgrService::WallpaperManagerkits::GetInstance().IsOperationAllowed();
        HILOG_DEBUG("exec IsOperationAllowed[%{public}d]", context->isOperationAllowed);
        context->status = napi_ok;
    };
    context->SetAction(nullptr, std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_Reset(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_Reset in");
    auto context = std::make_shared<SetContextInfo>();
    ApiInfo apiInfo{ false, false };
    NapiWallpaperAbility::RestoreInner(context, apiInfo);
    Call call(env, info, context, 1, apiInfo.needException);
    return call.AsyncCall(env, "reset");
}

napi_value NAPI_Restore(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_Restore in");
    auto context = std::make_shared<SetContextInfo>();
    ApiInfo apiInfo{ true, true };
    NapiWallpaperAbility::RestoreInner(context, apiInfo);
    Call call(env, info, context, 1, apiInfo.needException);
    return call.AsyncCall(env, "restore");
}

void NapiWallpaperAbility::RestoreInner(std::shared_ptr<SetContextInfo> context, const ApiInfo &apiInfo)
{
    HILOG_DEBUG("RestoreInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, 1)) {
            HILOG_DEBUG("input  argc : %{public}zu", argc);
            context->SetErrInfo(
                ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE) + PARAMETER_COUNT);
            return napi_invalid_arg;
        }
        if (!NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_number)) {
            HILOG_DEBUG("input  argc : %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE)
            + WALLPAPERTYPE_PARAMETER_TYPE);
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input wallpaperType : %{public}d", context->wallpaperType);
        return napi_pending_exception;
    };
    auto exec = [context, apiInfo](Call::Context *ctx) {
        HILOG_DEBUG("exec ResetWallpaper");
        ErrorCode wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(context->wallpaperType, apiInfo);
        HILOG_DEBUG("exec ResetWallpaper[%{public}d]", wallpaperErrorCode);
        if (wallpaperErrorCode == E_OK) {
            context->status = napi_ok;
        }
        if (apiInfo.needException) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code == PARAMETER_ERROR_CODE) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message + WALLPAPERTYPE_PARAMETER_TYPE);
            } else {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("exec status[%{public}d], context->status[%{public}d]", wallpaperErrorCode, context->status);
    };
    context->SetAction(std::move(input), nullptr);
    context->SetExecution(std::move(exec));
}

napi_value NAPI_SetWallpaper(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<SetContextInfo>();
    ApiInfo apiInfo{ false, false };
    NapiWallpaperAbility::SetImageInput(context);
    NapiWallpaperAbility::SetImageExec(context, apiInfo);
    Call call(env, info, context, TWO, apiInfo.needException);
    return call.AsyncCall(env, "setWallpaper");
}

napi_value NAPI_SetImage(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<SetContextInfo>();
    ApiInfo apiInfo{ true, true };
    NapiWallpaperAbility::SetImageInput(context);
    NapiWallpaperAbility::SetImageExec(context, apiInfo);
    Call call(env, info, context, TWO, apiInfo.needException);
    return call.AsyncCall(env, "setImage");
}

napi_value NAPI_SendEvent(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<GetContextInfo>();
    NapiWallpaperAbility::SendEventInner(context);
    Call call(env, info, context, TWO, true);
    return call.AsyncCall(env);
}

void NapiWallpaperAbility::SendEventInner(std::shared_ptr<GetContextInfo> context)
{
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, TWO)) {
            HILOG_ERROR("Input argc: %{public}zu", argc);
            context->SetErrInfo(
                ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE) + PARAMETER_COUNT);
            return napi_invalid_arg;
        }
        if (!NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)
            || !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_string)) {
            HILOG_ERROR("Input argc: %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR,
                std::string(PARAMETER_ERROR_MESSAGE) + "The type must be string.");
            return napi_invalid_arg;
        }
        char eventType[EVENTTYPESIZE] = { 0 };
        size_t bufSize = BUFFERSIZE;
        size_t len = 0;
        napi_get_value_string_utf8(env, argv[0], eventType, bufSize, &len);
        context->eventType = eventType;
        HILOG_DEBUG("Input event type: %{public}s", context->eventType.c_str());
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->result, result);
        HILOG_DEBUG("Output status: %{public}d", status);
        return status;
    };

    auto exec = [context](Call::Context *ctx) {
        ErrorCode wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().SendEvent(context->eventType);
        if (wallpaperErrorCode == E_OK) {
            context->status = napi_ok;
            context->result = true;
        } else {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code == PARAMETER_ERROR_CODE) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message
                + "The eventType must be SHOW_SYSTEM_SCREEN or SHOW_LOCK_SCREEN.");
            } else {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("Exec context status: [%{public}d]", context->status);
    };
    context->SetAction(std::move(input), std::move(output));
    context->SetExecution(std::move(exec));
}

napi_value NAPI_SetVideo(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<SetContextInfo>();
    NapiWallpaperAbility::SetVideoInner(context);
    Call call(env, info, context, TWO, true);
    return call.AsyncCall(env);
}

void NapiWallpaperAbility::SetVideoInner(std::shared_ptr<SetContextInfo> context)
{
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, TWO)) {
            HILOG_ERROR("Input argc: %{public}zu", argc);
            context->SetErrInfo(
                ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE) + PARAMETER_COUNT);
            return napi_invalid_arg;
        }
        if (!NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)
            || !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_number)
            || !NapiWallpaperAbility::IsValidArgRange(env, argv[1])) {
            HILOG_ERROR("Input argc: %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE)
            + "The first parameter type must be string, the second type must be WallpaperType"
            + "and parameter range must be WALLPAPER_LOCKSCREEN or WALLPAPER_SYSTEM.");

            return napi_invalid_arg;
        }
        context->uri = WallpaperJSUtil::Convert2String(env, argv[0]);
        napi_get_value_int32(env, argv[1], &context->wallpaperType);
        HILOG_DEBUG("Input wallpaperType: %{public}d", context->wallpaperType);
        return napi_ok;
    };

    auto exec = [context](Call::Context *ctx) {
        ErrorCode wallpaperErrorCode =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().SetVideo(context->uri, context->wallpaperType);
        if (wallpaperErrorCode == E_OK) {
            context->status = napi_ok;
        } else {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code == PARAMETER_ERROR_CODE) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message + DYNAMIC_WALLPAPERTYPE_PARAMETER_TYPE);
            } else {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("Exec context status:[%{public}d]", context->status);
    };
    context->SetAction(std::move(input), nullptr);
    context->SetExecution(std::move(exec));
}

void NapiWallpaperAbility::SetImageInput(std::shared_ptr<SetContextInfo> context)
{
    HILOG_DEBUG("SetImageInput in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, TWO)) {
            HILOG_DEBUG("input argc : %{public}zu", argc);
            context->SetErrInfo(
                ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE) + PARAMETER_COUNT);
            return napi_invalid_arg;
        }
        if ((!NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)
                && !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_object))
            || !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_number)) {
            HILOG_DEBUG("input argc : %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE)
            + "The first parameter type must be string or image.PixelMap, the second type must be WallpaperType"
            + "and parameter range must be WALLPAPER_LOCKSCREEN or WALLPAPER_SYSTEM.");
            return napi_invalid_arg;
        }
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        if (valueType == napi_string) {
            context->uri = WallpaperJSUtil::Convert2String(env, argv[0]);
        } else {
            std::shared_ptr<PixelMap> pixelMap = PixelMapNapi::GetPixelMap(env, argv[0]);
            if (pixelMap == nullptr) {
                HILOG_ERROR("PixelMapNapi::GetPixelMap error");
                context->isPixelEmp = true;
                return napi_generic_failure;
            } else {
                context->isPixelEmp = false;
            }
            context->pixelMap = pixelMap;
        }
        napi_get_value_int32(env, argv[1], &context->wallpaperType);
        HILOG_DEBUG("input wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    context->SetAction(std::move(input), nullptr);
}

void NapiWallpaperAbility::SetImageExec(std::shared_ptr<SetContextInfo> context, const ApiInfo &apiInfo)
{
    HILOG_DEBUG("SetImageExec in");
    auto exec = [context, apiInfo](Call::Context *ctx) {
        ErrorCode wallpaperErrorCode = E_UNKNOWN;
        if (context->uri.length() == 0) {
            HILOG_DEBUG("exec setWallpaper by pixelMap");
            if (!context->isPixelEmp) {
                wallpaperErrorCode =
                    WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(context->pixelMap,
                        context->wallpaperType, apiInfo);
            }
        } else {
            HILOG_DEBUG("exec setWallpaper by uri");
            wallpaperErrorCode = WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(context->uri,
                context->wallpaperType, apiInfo);
        }
        if (wallpaperErrorCode == E_OK) {
            context->status = napi_ok;
        }
        if (apiInfo.needException) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code == PARAMETER_ERROR_CODE) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message + DYNAMIC_WALLPAPERTYPE_PARAMETER_TYPE);
            } else {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("exec context->status[%{public}d]", context->status);
    };
    context->SetExecution(std::move(exec));
}

napi_value NAPI_GetPixelMap(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetPixelMap in");
    auto context = std::make_shared<GetContextInfo>();
    ApiInfo apiInfo{ false, false };
    NapiWallpaperAbility::GetImageInner(context, apiInfo);
    Call call(env, info, context, 1, apiInfo.needException);
    return call.AsyncCall(env, "getPixelMap");
}

napi_value NAPI_GetImage(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetImage in");
    auto context = std::make_shared<GetContextInfo>();
    ApiInfo apiInfo{ true, true };
    NapiWallpaperAbility::GetImageInner(context, apiInfo);
    Call call(env, info, context, 1, apiInfo.needException);
    return call.AsyncCall(env, "getImage");
}

void NapiWallpaperAbility::GetImageInner(std::shared_ptr<GetContextInfo> context, const ApiInfo &apiInfo)
{
    HILOG_DEBUG("GetImageInner in");
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!NapiWallpaperAbility::IsValidArgCount(argc, 1)) {
            HILOG_DEBUG("input argc : %{public}zu", argc);
            context->SetErrInfo(
                ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE) + PARAMETER_COUNT);
            return napi_invalid_arg;
        }
        if (!NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_number)
            || !NapiWallpaperAbility::IsValidArgRange(env, argv[1])) {
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE)
            + WALLPAPERTYPE_PARAMETER_TYPE);
            return napi_invalid_arg;
        }
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value pixelVal =
            (context->pixelMap != nullptr ? PixelMapNapi::CreatePixelMap(env, std::move(context->pixelMap)) : nullptr);
        HILOG_DEBUG("output PixelMapNapi::CreatePixelMap != nullptr[%{public}d]", pixelVal != nullptr);
        *result = pixelVal;
        return napi_ok;
    };
    auto exec = [context, apiInfo](Call::Context *ctx) {
        HILOG_DEBUG("exec GetImageInner");
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
        ErrorCode wallpaperErrorCode = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(
            context->wallpaperType, apiInfo, pixelMap);
        HILOG_DEBUG("exec wallpaperErrorCode[%{public}d]", wallpaperErrorCode);
        if (wallpaperErrorCode == E_OK) {
            context->status = napi_ok;
            context->pixelMap = (pixelMap != nullptr ? std::move(pixelMap) : nullptr);
            return;
        }
        if (apiInfo.needException) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code == PARAMETER_ERROR_CODE) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message + WALLPAPERTYPE_PARAMETER_TYPE);
            } else {
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
    if (!NapiWallpaperAbility::IsValidArgCount(argc, TWO)
        || !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)
        || !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_function)) {
        HILOG_DEBUG("input argc : %{public}zu", argc);
        if (NapiWallpaperAbility::IsValidArgCount(argc, 1) && // 1: argument count
            NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)
            && WallpaperJSUtil::Convert2String(env, argv[0]) == COLOR_CHANGE_EVENT) {
            return nullptr;
        }
        JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(E_PARAMETERS_INVALID);
        JsError::ThrowError(env, jsErrorInfo.code, jsErrorInfo.message
        + "The parameters must be two, the first parameters must be string, second parameter must be function.");
        return nullptr;
    }
    std::string type = WallpaperJSUtil::Convert2String(env, argv[0]);
    HILOG_DEBUG("type : %{public}s", type.c_str());
    if (type != COLOR_CHANGE_EVENT && type != WALLPAPER_CHANGE_EVENT) {
        HILOG_ERROR("do not support event type: %{public}s", type.c_str());
        JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(E_PARAMETERS_INVALID);
        JsError::ThrowError(env, jsErrorInfo.code, jsErrorInfo.message
        + "The first parameter type must be COLOR_CHANGE_EVENT and WALLPAPER_CHANGE_EVENT.");
        return nullptr;
    }
    std::shared_ptr<WallpaperMgrService::WallpaperEventListener> listener =
        std::make_shared<NapiWallpaperAbility>(env, argv[1]);
    ErrorCode errorCode = WallpaperMgrService::WallpaperManagerkits::GetInstance().On(type, listener);
    if (errorCode != E_OK) {
        HILOG_ERROR("WallpaperMgrService::WallpaperManagerkits::GetInstance().On failed!");
        if (type == COLOR_CHANGE_EVENT) {
            return nullptr;
        }
        JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(errorCode);
        JsError::ThrowError(env, jsErrorInfo.code, jsErrorInfo.message);
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
    if (!NapiWallpaperAbility::IsValidArgCount(argc, 1) || // 1: argument count
        !NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)) {
        HILOG_DEBUG("input argc : %{public}zu", argc);
        JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(E_PARAMETERS_INVALID);
        JsError::ThrowError(env, jsErrorInfo.code, jsErrorInfo.message
        + "Only one parameter and the type must be string.");
        return nullptr;
    }
    std::string type = WallpaperJSUtil::Convert2String(env, argv[0]);
    HILOG_DEBUG("type : %{public}s", type.c_str());
    if (type != COLOR_CHANGE_EVENT && type != WALLPAPER_CHANGE_EVENT) {
        HILOG_ERROR("do not support event type: %{public}s", type.c_str());
        JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(E_PARAMETERS_INVALID);
        JsError::ThrowError(env, jsErrorInfo.code, jsErrorInfo.message
        + "The first parameter type must be COLOR_CHANGE_EVENT and WALLPAPER_CHANGE_EVENT.");
        return nullptr;
    }
    std::shared_ptr<WallpaperMgrService::WallpaperEventListener> listener = nullptr;
    if (argc >= TWO) {
        if (NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_function)) {
            listener = std::make_shared<NapiWallpaperAbility>(env, argv[1]);
        } else if (!NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_undefined)
                   && !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_null)) {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(E_PARAMETERS_INVALID);
            JsError::ThrowError(env, jsErrorInfo.code, jsErrorInfo.message
            + "The second parameter is neither a valid function type nor undefined or null.");
            return nullptr;
        }
    }
    ErrorCode errorCode = WallpaperMgrService::WallpaperManagerkits::GetInstance().Off(type, listener);
    if (errorCode != E_OK) {
        HILOG_ERROR("WallpaperMgrService::WallpaperManagerkits::GetInstance().Off failed!");
        if (type == COLOR_CHANGE_EVENT) {
            return nullptr;
        }
        JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(errorCode);
        JsError::ThrowError(env, jsErrorInfo.code, jsErrorInfo.message);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NAPI_SetCustomWallpaper(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<SetContextInfo>();
    NapiWallpaperAbility::SetCustomWallpaper(context);
    Call call(env, info, context, TWO, true);
    return call.AsyncCall(env);
}

void NapiWallpaperAbility::SetCustomWallpaper(std::shared_ptr<SetContextInfo> context)
{
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (argc < TWO) {
            HILOG_ERROR("Input argc: %{public}zu", argc);
            context->SetErrInfo(
                ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE) + PARAMETER_COUNT);
            return napi_invalid_arg;
        }
        if (!NapiWallpaperAbility::IsValidArgType(env, argv[0], napi_string)
            || !NapiWallpaperAbility::IsValidArgType(env, argv[1], napi_number)) {
            HILOG_ERROR("Input argc: %{public}zu", argc);
            context->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE)
            + "The first parameter type must be string. the second type must be WallpaperType and"
            + "parameter range must be WALLPAPER_LOCKSCREEN or WALLPAPER_SYSTEM.");
            return napi_invalid_arg;
        }
        context->uri = WallpaperJSUtil::Convert2String(env, argv[0]);
        napi_get_value_int32(env, argv[1], &context->wallpaperType);
        HILOG_DEBUG("Input wallpaperType: %{public}d", context->wallpaperType);
        return napi_ok;
    };

    auto exec = [context](Call::Context *ctx) {
        ErrorCode wallpaperErrorCode = WallpaperMgrService::WallpaperManagerkits::GetInstance().SetCustomWallpaper(
            context->uri, context->wallpaperType);
        if (wallpaperErrorCode == E_OK) {
            context->status = napi_ok;
        } else {
            JsErrorInfo jsErrorInfo = JsError::ConvertErrorCode(wallpaperErrorCode);
            if (jsErrorInfo.code == PARAMETER_ERROR_CODE) {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message + WALLPAPERTYPE_PARAMETER_TYPE);
            } else {
                context->SetErrInfo(jsErrorInfo.code, jsErrorInfo.message);
            }
        }
        HILOG_DEBUG("Exec context status:[%{public}d]", context->status);
    };
    context->SetAction(std::move(input), nullptr);
    context->SetExecution(std::move(exec));
}

NapiWallpaperAbility::NapiWallpaperAbility(napi_env env, napi_value callback) : env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

NapiWallpaperAbility::~NapiWallpaperAbility()
{
    HILOG_INFO("~NapiWallpaperAbility start!");
    WorkData *workData = new (std::nothrow) WorkData(env_, callback_);
    if (workData != nullptr) {
        uv_after_work_cb afterCallback = [](uv_work_t *work, int status) {
            WorkData *workData = reinterpret_cast<WorkData *>(work->data);
            napi_delete_reference(workData->env_, workData->callback_);
            delete workData;
            delete work;
        };
        MiscServices::UvQueue::Call(env_, workData, afterCallback);
    }
}

void NapiWallpaperAbility::OnColorsChange(const std::vector<uint64_t> &color, int wallpaperType)
{
    HILOG_INFO("OnColorsChange start! wallpaperType:%{public}d", wallpaperType);
    WallpaperMgrService::WallpaperEventListener::OnColorsChange(color, wallpaperType);
    EventDataWorker *eventDataWorker = new (std::nothrow)
        EventDataWorker(this->shared_from_this(), color, wallpaperType);
    if (eventDataWorker == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        delete eventDataWorker;
        return;
    }
    work->data = eventDataWorker;
    uv_queue_work_with_qos(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            EventDataWorker *eventDataInner = reinterpret_cast<EventDataWorker *>(work->data);
            if (eventDataInner == nullptr || eventDataInner->listener == nullptr) {
                delete work;
                return;
            }
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(eventDataInner->listener->env_, &scope);
            if (scope == nullptr) {
                delete eventDataInner;
                delete work;
                return;
            }
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
            napi_status callStatus = napi_call_function(
                eventDataInner->listener->env_, global, callback, sizeof(args) / sizeof(args[0]), args, &result);
            if (callStatus != napi_ok) {
                HILOG_ERROR("notify data change failed callStatus:%{public}d", callStatus);
            }
            napi_close_handle_scope(eventDataInner->listener->env_, scope);
            delete eventDataInner;
            delete work;
        },
        uv_qos_user_initiated);
}

void NapiWallpaperAbility::OnWallpaperChange(
    WallpaperType wallpaperType, WallpaperResourceType resourceType, const std::string &uri)
{
    WallpaperChangedData *data = new (std::nothrow)
        WallpaperChangedData(this->shared_from_this(), wallpaperType, resourceType, uri);
    if (data == nullptr) {
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        delete data;
        return;
    }
    work->data = data;
    uv_queue_work_with_qos(
        loop_, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            WallpaperChangedData *dataInner = reinterpret_cast<WallpaperChangedData *>(work->data);
            if (dataInner == nullptr || dataInner->listener == nullptr) {
                delete work;
                return;
            }
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(dataInner->listener->env_, &scope);
            if (scope == nullptr) {
                delete dataInner;
                delete work;
                return;
            }
            napi_value jsWallpaperType = nullptr;
            napi_value jsResourceType = nullptr;
            napi_value jsResourceUri = nullptr;
            napi_create_int32(dataInner->listener->env_, dataInner->wallpaperType, &jsWallpaperType);
            napi_create_int32(dataInner->listener->env_, dataInner->resourceType, &jsResourceType);
            napi_create_string_utf8(
                dataInner->listener->env_, dataInner->uri.c_str(), dataInner->uri.length(), &jsResourceUri);
            napi_value callback = nullptr;
            napi_value args[3] = { jsWallpaperType, jsResourceType, jsResourceUri };
            napi_get_reference_value(dataInner->listener->env_, dataInner->listener->callback_, &callback);
            napi_value global = nullptr;
            napi_get_global(dataInner->listener->env_, &global);
            napi_value result;
            napi_status callStatus = napi_call_function(
                dataInner->listener->env_, global, callback, sizeof(args) / sizeof(args[0]), args, &result);
            if (callStatus != napi_ok) {
                HILOG_ERROR("notify data change failed callStatus:%{public}d", callStatus);
            }
            napi_close_handle_scope(dataInner->listener->env_, scope);
            delete dataInner;
            delete work;
        }, uv_qos_user_initiated);
}

bool NapiWallpaperAbility::IsValidArgCount(size_t argc, size_t expectationSize)
{
    return argc >= expectationSize;
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

bool NapiWallpaperAbility::CheckValidArgWallpaperType(
    napi_env env, size_t argc, napi_value argValue, std::shared_ptr<Call::Context> ctx)
{
    if (!NapiWallpaperAbility::IsValidArgCount(argc, ONE)) {
        HILOG_DEBUG("input argc : %{public}zu", argc);
        ctx->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE) + PARAMETER_COUNT);
        return false;
    }
    if (!NapiWallpaperAbility::IsValidArgType(env, argValue, napi_number)
        || !NapiWallpaperAbility::IsValidArgRange(env, argValue)) {
        HILOG_DEBUG("input argc : %{public}zu", argc);
        ctx->SetErrInfo(ErrorThrowType::PARAMETER_ERROR, std::string(PARAMETER_ERROR_MESSAGE)
        + WALLPAPERTYPE_PARAMETER_TYPE);
        return false;
    }
    return true;
}
} // namespace WallpaperNAPI
} // namespace OHOS
