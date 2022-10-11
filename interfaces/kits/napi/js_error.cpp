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
#include "js_error.h"

namespace OHOS::WallpaperNAPI {

using namespace OHOS::WallpaperMgrService;

void JsError::ThrowError(napi_env env, int32_t errorCode, const std::string &errorMessage)
{
    HILOG_DEBUG("ThrowError in");
    napi_value message;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, errorMessage.c_str(), NAPI_AUTO_LENGTH, &message));
    napi_value code;
    int32_t errorCodeValue = static_cast<int32_t>(errorCode);
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, std::to_string(errorCodeValue).c_str(), NAPI_AUTO_LENGTH, &code));
    napi_value error;
    NAPI_CALL_RETURN_VOID(env, napi_create_error(env, code, message, &error));
    NAPI_CALL_RETURN_VOID(env, napi_throw(env, error));
}

JsErrorInfo JsError::ConvertErrorCode(int32_t wallpaperErrorCode)
{
    HILOG_DEBUG("ConvertErrorCode in");
    JsErrorInfo errorObject;
    switch (static_cast<ErrorCode>(wallpaperErrorCode)) {
        case E_PARAMETERS_INVALID:
            errorObject.code = static_cast<int32_t>(ErrorThrowType::PARAMETER_ERROR);
            errorObject.message = PARAMETERERRORMESSAGE;
            break;
        case E_NO_PERMISSION:
            errorObject.code = static_cast<int32_t>(ErrorThrowType::PERMISSION_ERROR);
            errorObject.message = PERMISSIONDENIEDMESSAGE;
            break;
        default:
            HILOG_DEBUG("Non-existent error type!");
            break;
    }
    return errorObject;
}
} // namespace OHOS::WallpaperNAPI