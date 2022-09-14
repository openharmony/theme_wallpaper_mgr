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
#include <string>
#include <vector>
#include <memory>
#include "pixel_map.h"
#include "wallpaper_color_change_listener.h"
#include "wallpaper_manager_common_info.h"
#include "i_wallpaper_service.h"

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
    static WallpaperManagerkits& GetInstance();

    /**
    * Wallpaper set.
    * @param  uriOrPixelMap Wallpaper picture; wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or
    * WALLPAPER_LOCKSCREEN
    * @return  true or false
    */
    virtual bool SetWallpaper(std::string url, int wallpaperType) = 0;

     /**
    * Wallpaper set.
    * @param  pixelMap:picture pixelMap struct; wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or
    * WALLPAPER_LOCKSCREEN
    * @return  true or false
    */
    virtual bool SetWallpaper(std::unique_ptr<OHOS::Media::PixelMap> &pixelMap, int wallpaperType) = 0;

    /**
        *Obtains the default pixel map of a wallpaper of the specified type.
        * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
        * @return image.PixelMap png type The bitmap file of wallpaper
        * @permission ohos.permission.GET_WALLPAPER
        * @systemapi Hide this for inner system use.
    */
    virtual std::shared_ptr<OHOS::Media::PixelMap> GetPixelMap(int wallpaperType) = 0;

    /**
     * Obtains the WallpaperColorsCollection instance for the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return RgbaColor type of array callback function
     */
    virtual std::vector<RgbaColor> GetColors(int wallpaperType) = 0;

    /**
     * Obtains the ID of the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of callback function
     */
    virtual int  GetWallpaperId(int wallpaperType) = 0;

    virtual int32_t GetFile(int wallpaperType) = 0;

    /**
     * Obtains the minimum height of the wallpaper.
     * @return number type of callback function
     */
    virtual int  GetWallpaperMinHeight() = 0;

    /**
     * Obtains the minimum width of the wallpaper.
     * @return number type of callback function
     */
    virtual int GetWallpaperMinWidth() = 0;

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
    virtual bool ResetWallpaper(std::int32_t wallpaperType) = 0;

    /**
     * Screen shot live wallpaper
     * @param scale
     * @param pixelFormat
     * @return image.PixelMap png type The bitmap file of wallpaper
     * @permission ohos.permission.CAPTURE_SCREEN
     * @systemapi Hide this for inner system use.
     * @return  true or false
     */
    virtual bool ScreenshotLiveWallpaper(int wallpaperType, OHOS::Media::PixelMap pixelMap) = 0;

    /**
     * Registers a listener for wallpaper color changes to receive notifications about the changes.
     * @param type The incoming colorChange table open receiver pick a color change wallpaper wallpaper color changes
     * @param callback Provides dominant colors of the wallpaper.
     * @return  true or false
     */
    virtual bool On(std::shared_ptr<WallpaperColorChangeListener> listener) = 0;

    /**
     * Registers a listener for wallpaper color changes to receive notifications about the changes.
     * @param type Incoming 'colorChange' table delete receiver to pick up a color change wallpaper wallpaper color
     * changes
     * @param callback Provides dominant colors of the wallpaper.
     */
    virtual bool Off(std::shared_ptr<WallpaperColorChangeListener> listener) = 0;

    virtual bool RegisterWallpaperCallback(bool (*callback) (int)) = 0;
    using JScallback = bool (*) (int);

    virtual JScallback GetCallback() = 0;

    virtual void SetCallback(bool (*cb) (int)) = 0;

    virtual void SetWpProxy(sptr<IWallpaperService> wpProxyMock) = 0;

    virtual void ResetWpProxy() = 0;

    virtual void CloseWallpaperFd(int32_t wallpaperType) = 0;
protected:
    WallpaperManagerkits() = default;
};
}
}


#endif
