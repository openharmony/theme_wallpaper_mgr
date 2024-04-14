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

#include <string>
#include <vector>

#include "i_wallpaper_callback.h"
#include "iremote_broker.h"
#include "iwallpaper_event_listener.h"
#include "pixel_map.h"
#include "pixel_map_parcel.h"
#include "wallpaper_common.h"
#include "wallpaper_event_listener.h"
#include "wallpaper_event_listener_client.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {
class IWallpaperService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.Wallpaper.IWallpaperService");
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
    virtual ErrorCode SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length) = 0;
    virtual ErrorCode SetWallpaperByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap,
        int32_t wallpaperType) = 0;
    virtual ErrorCode GetPixelMap(int32_t wallpaperType, FdInfo &fdInfo) = 0;
    /**
     * Obtains the WallpaperColorsCollection instance for the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of array callback function
     */
    virtual ErrorCode GetColors(int wallpaperType, std::vector<uint64_t> &colors) = 0;

    virtual ErrorCode GetFile(int32_t wallpaperType, int32_t &wallpaperFd) = 0;

    /**
     * Obtains the ID of the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of callback function
     */
    virtual int GetWallpaperId(int32_t wallpaperType) = 0;

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
    virtual ErrorCode ResetWallpaper(int32_t wallpaperType) = 0;

    /**
     * Registers a listener for wallpaper event changes to receive notifications about the changes.
     * @param type event type
     * @param listener event observer.
     * @return error code
     */
    virtual ErrorCode On(const std::string &type, sptr<IWallpaperEventListener> listener) = 0;

    /**
     * Unregisters a listener for wallpaper event changes.
     * @param type event type
     * @param listener event observer.
     * @return error code
     */
    virtual ErrorCode Off(const std::string &type, sptr<IWallpaperEventListener> listener) = 0;

    virtual bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) = 0;

    virtual ErrorCode SetWallpaperV9(int32_t fd, int32_t wallpaperType, int32_t length) = 0;
    virtual ErrorCode SetWallpaperV9ByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap,
        int32_t wallpaperType) = 0;
    virtual ErrorCode GetPixelMapV9(int32_t wallpaperType, FdInfo &fdInfo) = 0;
    virtual ErrorCode GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors) = 0;
    virtual ErrorCode ResetWallpaperV9(int32_t wallpaperType) = 0;

    /**
     * @brief Sets live wallpaper of the specified type based on the uri path
     *        of the MP4 file.
     *
     * Need @permission ohos.permission.SET_WALLPAPER
     *
     * @param fd Indicates the handle of the MP4 file.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM
     *        or WALLPAPER_LOCKSCREEN
     * @param length file size of the MP4 file.
     * @return ErrorCode
     */
    virtual ErrorCode SetVideo(int32_t fd, int32_t wallpaperType, int32_t length) = 0;

    /**
     * Sets the custom wallpaper.
     */
    virtual ErrorCode SetCustomWallpaper(int32_t fd, int32_t wallpaperType, int32_t length) = 0;

    /**
     * @brief The application sends the event to the wallpaper service.
     *
     * Need @permission ohos.permission.SET_WALLPAPER
     *
     * @param eventType Event type, values for SHOW_SYSTEMSCREEN or SHOW_LOCKSCREEN
     * @return ErrorCode
     */
    virtual ErrorCode SendEvent(const std::string &eventType) = 0;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_INTERFACE_H