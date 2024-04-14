/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef WALLPAPER_SERVICE_IPC_INTERFACE_CODE_H
#define WALLPAPER_SERVICE_IPC_INTERFACE_CODE_H

/* SAID: 3705*/
namespace OHOS {
namespace WallpaperMgrService {
enum class WallpaperServiceIpcInterfaceCode {
    SET_WALLPAPER,
    GET_PIXELMAP,
    GET_COLORS,
    GET_WALLPAPER_ID,
    GET_FILE,
    RESET_WALLPAPER,
    ON,
    OFF,
    IS_CHANGE_PERMITTED,
    IS_OPERATION_ALLOWED,
    REGISTER_CALLBACK,
    SET_WALLPAPER_V9,
    GET_PIXELMAP_V9,
    GET_COLORS_V9,
    RESET_WALLPAPER_V9,
    SET_VIDEO,
    SET_CUSTOM,
    SEND_EVENT,
    SET_WALLPAPER_PIXELMAP,
    SET_WALLPAPER_PIXELMAP_V9,
};
} // namespace WallpaperMgrService
} // namespace OHOS

#endif // WALLPAPER_SERVICE_IPC_INTERFACE_CODE_H