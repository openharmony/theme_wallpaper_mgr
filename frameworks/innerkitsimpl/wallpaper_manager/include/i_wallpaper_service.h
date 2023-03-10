/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_WALLPAPER_SERVICE_INTERFACE_H
#define SERVICES_INCLUDE_WALLPAPER_SERVICE_INTERFACE_H

#include <vector>
#include <string>
#include "iremote_broker.h"
#include "pixel_map.h"
#include "pixel_map_parcel.h"
#include "wallpaper_manager_common_info.h"
#include "wallpaper_color_change_listener.h"
#include "iwallpaper_color_change_listener.h"
#include "wallpaper_color_change_listener_client.h"
#include "i_wallpaper_callback.h"

namespace OHOS {
namespace WallpaperMgrService {
class IWallpaperService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.Wallpaper.IWallpaperService");
    enum {
        SET_WALLPAPER_URI_FD,
        SET_WALLPAPER_MAP,
        GET_PIXELMAP,
        GET_PIXELMAPFILE,
        GET_COLORS,
        GET_WALLPAPER_ID,
        GET_FILE,
        GET_WALLPAPER_MIN_HEIGHT,
        GET_WALLPAPER_MIN_WIDTH,
        RESET_WALLPAPER,
        ON,
        OFF,
        IS_CHANGE_PERMITTED,
        IS_OPERATION_ALLOWED,
        REGISTER_CALLBACK,
        SET_WALLPAPER_URI_FD_V9,
        SET_WALLPAPER_MAP_V9,
        GET_PIXELMAP_V9,
        GET_COLORS_V9,
        GET_WALLPAPER_MIN_HEIGHT_V9,
        GET_WALLPAPER_MIN_WIDTH_V9,
        RESET_WALLPAPER_V9,
    };
    struct getPixelMap {
        std::string result;
        int fileLen;
    };

    struct FdInfo {
        int fd = -1;
        int size = 0;
    };

    /**
    * Wallpaper set.
    * @param  uriOrPixelMap Wallpaper picture; wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or
    * WALLPAPER_LOCKSCREEN
    * @return  true or false
    */
    virtual int32_t SetWallpaperByFD(int fd, int wallpaperType, int length) = 0;
    virtual int32_t SetWallpaperByMap(int fd, int wallpaperType, int length) = 0;
    virtual int32_t GetPixelMap(int wallpaperType, FdInfo &fdInfo) = 0;
    /**
     * Obtains the WallpaperColorsCollection instance for the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of array callback function
     */
    virtual int32_t GetColors(int wallpaperType, std::vector<uint64_t> &colors) = 0;

    virtual int32_t GetFile(int wallpaperType, int32_t &wallpaperFd) = 0;

    /**
     * Obtains the ID of the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of callback function
     */
    virtual int GetWallpaperId(int wallpaperType) = 0;

    /**
     * Obtains the minimum height of the wallpaper.
     * @return number type of callback function
     */
    virtual int32_t GetWallpaperMinHeight(int32_t &minHeight) = 0;

    /**
     * Obtains the minimum width of the wallpaper.
     * @return number type of callback function
     */
    virtual int32_t GetWallpaperMinWidth(int32_t &minWidth) = 0;

    /**
     * Checks whether to allow the application to change the wallpaper for the current user.
     * @return boolean type of callback function
     */
    virtual bool IsChangePermitted() = 0;

    /**
     * Checks whether a user is allowed to set wallpapers.
     * @return boolean type of callback function
     */
    virtual bool IsOperationAllowed() = 0;

    /**
     * Removes a wallpaper of the specified type and restores the default one.
     * @param wallpaperType  Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @permission ohos.permission.SET_WALLPAPER
     */
    virtual int32_t ResetWallpaper(int wallpaperType) = 0;

    /**
     * Registers a listener for wallpaper color changes to receive notifications about the changes.
     * @param type The incoming colorChange table open receiver pick a color change wallpaper wallpaper color changes
     * @param callback Provides dominant colors of the wallpaper.
     * @return  true or false
     */
    virtual bool On(sptr<IWallpaperColorChangeListener> listener) = 0;

    /**
     * Registers a listener for wallpaper color changes to receive notifications about the changes.
     * @param type Incoming 'colorChange' table delete receiver to pick up a color change wallpaper wallpaper color
     * changes
     * @param callback Provides dominant colors of the wallpaper.
     */
    virtual bool Off(sptr<IWallpaperColorChangeListener> listener) = 0;

    virtual bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) = 0;

    virtual int32_t SetWallpaperByFDV9(int32_t fd, int32_t wallpaperType, int32_t length) = 0;
    virtual int32_t SetWallpaperByMapV9(int32_t fd, int32_t wallpaperType, int32_t length) = 0;
    virtual int32_t GetPixelMapV9(int32_t wallpaperType, FdInfo &fdInfo) = 0;
    virtual int32_t GetColorsV9(int wallpaperType, std::vector<uint64_t> &colors) = 0;
    virtual int32_t GetWallpaperMinHeightV9(int32_t &minHeight) = 0;
    virtual int32_t GetWallpaperMinWidthV9(int32_t &minWidth) = 0;
    virtual int32_t ResetWallpaperV9(int32_t wallpaperType) = 0;
};
}
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_INTERFACE_H