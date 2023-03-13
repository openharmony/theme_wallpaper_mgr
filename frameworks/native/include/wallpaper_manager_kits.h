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
#include "wallpaper_color_change_listener.h"
#include "wallpaper_common.h"
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
    virtual ErrorCode SetWallpaper(std::string uri, int32_t wallpaperType) = 0;

    /**
    * Wallpaper set.
    * @param  pixelMap:picture pixelMap struct; wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or
    * WALLPAPER_LOCKSCREEN
    * @return  ErrorCode
    */
    virtual ErrorCode SetWallpaper(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType) = 0;

    /**
        *Obtains the default pixel map of a wallpaper of the specified type.
        * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN ;
        * Obtains image.PixelMap png type The bitmap file of wallpaper
        * @return ErrorCode
        * @permission ohos.permission.GET_WALLPAPER
        * @systemapi Hide this for inner system use.
    */
    virtual ErrorCode GetPixelMap(int32_t wallpaperType, std::shared_ptr<OHOS::Media::PixelMap> &pixelMap) = 0;

    /**
     * Obtains the WallpaperColorsCollection instance for the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of array callback function
     */
    virtual std::vector<uint64_t> GetColors(int32_t wallpaperType) = 0;

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
    virtual int32_t GetWallpaperMinHeight() = 0;

    /**
     * Obtains the minimum width of the wallpaper.
     * @return number type of callback function
     */
    virtual int32_t GetWallpaperMinWidth() = 0;

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
    virtual ErrorCode ResetWallpaper(std::int32_t wallpaperType) = 0;

    /**
     * Registers a listener for wallpaper color changes to receive notifications about the changes.
     * @param type The incoming colorChange table open receiver pick a color change wallpaper wallpaper color changes
     * @param callback Provides dominant colors of the wallpaper.
     * @return  true or false
     */
    virtual bool On(const std::string &type, std::shared_ptr<WallpaperColorChangeListener> listener) = 0;

    /**
     * Registers a listener for wallpaper color changes to receive notifications about the changes.
     * @param type Incoming 'colorChange' table delete receiver to pick up a color change wallpaper wallpaper color
     * changes
     * @param callback Provides dominant colors of the wallpaper.
     */
    virtual bool Off(const std::string &type, std::shared_ptr<WallpaperColorChangeListener> listener) = 0;

    virtual bool RegisterWallpaperCallback(bool (*callback)(int32_t)) = 0;
    using JScallback = bool (*)(int32_t);

    virtual JScallback GetCallback() = 0;

    virtual void SetCallback(bool (*cb)(int32_t)) = 0;

    virtual void CloseWallpaperFd(int32_t wallpaperType) = 0;

protected:
    WallpaperManagerkits() = default;
};
} // namespace WallpaperMgrService
} // namespace OHOS

#endif
