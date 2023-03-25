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
#ifndef WALLPAPER_JS_ERROR_H
#define WALLPAPER_JS_ERROR_H
#include <string>

#include "hilog_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "wallpaper_common.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS::WallpaperNAPI {
constexpr const char *PARAMETERERRORMESSAGE = "BusinessError 401: Parameter error.";
constexpr const char *PERMISSIONDENIEDMESSAGE = "BusinessError 201: Permission Denied.";
constexpr const char *EQUIPMENTERRORMESSAGE = "BusinessError 801: Equipment error.";
constexpr const char *PERMISSIONFAILEDMESSAGE = "BusinessError 202: Permission verification failed,"
                                                "application which is not a system application uses system API.";
enum ErrorThrowType : int32_t {
    PERMISSION_ERROR = 201,
    SYSTEM_APP_PERMISSION_ERROR = 202,
    PARAMETER_ERROR = 401,
    EQUIPMENT_ERROR = 801,
    OPENFILE_FAILED = 13100001,
    IMAGE_FORMAT_INCORRECT
};
struct JsErrorInfo {
    int32_t code = 0;
    std::string message;
};

class JsError {
public:
    static void ThrowError(napi_env env, int32_t errorCode, const std::string &errorMessage);
    static JsErrorInfo ConvertErrorCode(int32_t wallpaperErrorCode);
};
} // namespace OHOS::WallpaperNAPI

#endif // WALLPAPER_JS_ERROR_H