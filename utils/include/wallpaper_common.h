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
#ifndef WALLPAPER_SERVICES_WALLPAPER_COMMON_H
#define WALLPAPER_SERVICES_WALLPAPER_COMMON_H

#include <string>

#include "errors.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {
#define WALLPAPER_SERVICE_NAME "WallpaperMgrService"
static const std::string WALLPAPER_BUNDLE_NAME = "ohos";
static const std::string WALLPAPER_PERMISSION_NAME_GET_WALLPAPER = "ohos.permission.GET_WALLPAPER";
static const std::string WALLPAPER_PERMISSION_NAME_SET_WALLPAPER = "ohos.permission.SET_WALLPAPER";
static const std::string WALLPAPER_PERMISSION_NAME_CAPTURE_SCREEN = "ohos.permission.CAPTURE_SCREEN";

static const std::string TEST_LABEL = "test label";
static const std::string TEST_DESCRIPTION = "test description";

struct ApiInfo {
    bool needException;
    bool isSystemApi;
};

enum WallpaperModule {
    WALLPAPER_MODULE_SERVICE_ID = 0x07,
};
// time error offset, used only in this file.
constexpr ErrCode WALLPAPER_ERR_OFFSET = ErrCodeOffset(SUBSYS_SMALLSERVICES, WALLPAPER_MODULE_SERVICE_ID);

enum ErrorCode : int32_t {
    NO_ERROR = 0,
    E_OK = WALLPAPER_ERR_OFFSET,
    E_SA_DIED,
    E_READ_PARCEL_ERROR,
    E_WRITE_PARCEL_ERROR,
    E_PUBLISH_FAIL,
    E_TRANSACT_ERROR,
    E_DEAL_FAILED,
    E_PARAMETERS_INVALID,
    E_SET_RTC_FAILED,
    E_NOT_FOUND,
    E_NO_PERMISSION,
    E_FILE_ERROR,
    E_IMAGE_ERRCODE,
    E_NO_MEMORY,
    E_NOT_SYSTEM_APP,
    E_USER_IDENTITY_ERROR,
    E_CHECK_DESCRIPTOR_ERROR,
    E_PICTURE_OVERSIZED,
    E_UNKNOWN,
};
using JsCallbackOffset = bool (*)(int32_t, int32_t);

struct WallpaperPictureInfo {
    FoldState foldState;
    RotateState rotateState;
    int32_t fd;
    int32_t length;
    std::string tempPath;
};

} // namespace WallpaperMgrService
} // namespace OHOS

#endif // WALLPAPER_SERVICES_WALLPAPER_COMMON_H