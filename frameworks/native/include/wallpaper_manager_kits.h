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

#ifndef INNERKITS_WALLPAPER_MANAGER_KITS_H
#define INNERKITS_WALLPAPER_MANAGER_KITS_H
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "pixel_map.h"
#include "visibility.h"
#include "wallpaper_common.h"
#include "wallpaper_event_listener.h"
#include "wallpaper_manager_common_info.h"

/**
 * Interfaces for ohos account subsystem.
 */
namespace OHOS {
namespace WallpaperMgrService {
class WallpaperManagerkits {
public:
    virtual ~WallpaperManagerkits() = default;
    DISALLOW_COPY_AND_MOVE(WallpaperManagerkits);

    /**
     * Get instance of ohos Wallpaper manager.
     *
     * @return Instance of ohos Wallpaper manager.
     */
    WALLPAPER_API static WallpaperManagerkits &GetInstance();

    /**
    * Wallpaper set.
    * @param  uriOrPixelMap Wallpaper picture; wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or
    * WALLPAPER_LOCKSCREEN
    * @return  ErrorCode
    */
    virtual ErrorCode SetWallpaper(std::string uri, int32_t wallpaperType, const ApiInfo &apiInfo) = 0;

    /**
    * Wallpaper set.
    * @param  pixelMap:picture pixelMap struct; wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or
    * WALLPAPER_LOCKSCREEN
    * @return  ErrorCode
    */
    virtual ErrorCode SetWallpaper(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType,
        const ApiInfo &apiInfo) = 0;

    /**
        *Obtains the default pixel map of a wallpaper of the specified type.
        * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN ;
        * Obtains image.PixelMap png type The bitmap file of wallpaper
        * @return ErrorCode
        * @permission ohos.permission.GET_WALLPAPER
        * @systemapi Hide this for inner system use.
    */
    virtual ErrorCode GetPixelMap(int32_t wallpaperType, const ApiInfo &apiInfo,
        std::shared_ptr<OHOS::Media::PixelMap> &pixelMap) = 0;

    /**
     * Obtains the WallpaperColorsCollection instance for the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of array callback function
     */
    virtual ErrorCode GetColors(int32_t wallpaperType, const ApiInfo &apiInfo, std::vector<uint64_t> &colors) = 0;

    /**
     * Obtains the ID of the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of callback function
     */
    virtual int32_t GetWallpaperId(int32_t wallpaperType) = 0;

    virtual ErrorCode GetFile(int32_t wallpaperType, int32_t &wallpaperFd) = 0;

    /**
     * Obtains the minimum height of the wallpaper.
     * @return number type of callback function
     */
    virtual ErrorCode GetWallpaperMinHeight(const ApiInfo &apiInfo, int32_t &minHeight) = 0;

    /**
     * Obtains the minimum width of the wallpaper.
     * @return number type of callback function
     */
    virtual ErrorCode GetWallpaperMinWidth(const ApiInfo &apiInfo, int32_t &minWidth) = 0;

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
    virtual ErrorCode ResetWallpaper(std::int32_t wallpaperType, const ApiInfo &apiInfo) = 0;

    /**
     * Registers a listener for wallpaper event to receive notifications about the changes.
     * @param type event type
     * @param listener event listener
     * @return error code
     */
    virtual ErrorCode On(const std::string &type, std::shared_ptr<WallpaperEventListener> listener) = 0;

    /**
     * Unregisters a listener for wallpaper event to receive notifications about the changes.
     * @param type event type
     * @param listener event listener
     * @return error code
     */
    virtual ErrorCode Off(const std::string &type, std::shared_ptr<WallpaperEventListener> listener) = 0;

    /**
     * Sets live wallpaper of the specified type based on the uri path of the MP4 file.
     * @param uri Indicates the uri path of the MP4 file.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return ErrorCode
     * @permission ohos.permission.SET_WALLPAPER
     */
    virtual ErrorCode SetVideo(const std::string &uri, const int32_t wallpaperType) = 0;

    /**
     * Sets custom wallpaper of the specified type based on the uri path.
     * @param uri Indicates the uri path.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return ErrorCode
     * @permission ohos.permission.SET_WALLPAPER
     */
    virtual ErrorCode SetCustomWallpaper(const std::string &uri, int32_t wallpaperType) = 0;

    /**
     * The application sends the event to the wallpaper service.
     * @param eventType Event type, values for SHOW_SYSTEMSCREEN or SHOW_LOCKSCREEN
     * @return ErrorCode
     * @permission ohos.permission.SET_WALLPAPER
     */
    virtual ErrorCode SendEvent(const std::string &eventType) = 0;
    using JScallback = bool (*)(int32_t);

    virtual bool RegisterWallpaperCallback(JScallback callback) = 0;

    virtual JScallback GetCallback() = 0;

    virtual void SetCallback(JScallback cb) = 0;

    virtual void CloseWallpaperFd(int32_t wallpaperType) = 0;

protected:
    WallpaperManagerkits() = default;
};
} // namespace WallpaperMgrService
} // namespace OHOS

#endif
