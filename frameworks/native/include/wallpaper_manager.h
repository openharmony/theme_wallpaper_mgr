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

#ifndef INNERKITS_WALLPAPER_MANAGER_H
#define INNERKITS_WALLPAPER_MANAGER_H

#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "avmetadatahelper.h"
#include "fault_reporter.h"
#include "ipc_skeleton.h"
#include "iwallpaper_callback.h"
#include "iwallpaper_event_listener.h"
#include "iwallpaper_service.h"
#include "singleton.h"
#include "wallpaper_common.h"
#include "wallpaper_event_listener.h"
#include "wallpaper_event_listener_client.h"

using JScallback = bool (*)(int32_t);
namespace OHOS {
using namespace MiscServices;
namespace WallpaperMgrService {
class WallpaperManager {
    WallpaperManager();
    ~WallpaperManager();

public:
    DISALLOW_COPY_AND_MOVE(WallpaperManager);
    static WallpaperManager &GetInstance();

    /**
    * Wallpaper set.
    * @param  uriOrPixelMap Wallpaper picture; wallpaperType Wallpaper type,
    * values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
    * @return  ErrorCode
    */
    ErrorCode SetWallpaper(std::string uri, int32_t wallpaperType, const ApiInfo &apiInfo);

    /**
    * Wallpaper set.
    * @param  pixelMap:picture pixelMap struct; wallpaperType Wallpaper type,
    * values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
    * @return  ErrorCode
    */
    ErrorCode SetWallpaper(
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType, const ApiInfo &apiInfo);

    /**
        *Obtains the default pixel map of a wallpaper of the specified type.
        * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN;
        * Obtains image.PixelMap png type The bitmap file of wallpaper
        * @return ErrorCode
        * @permission ohos.permission.GET_WALLPAPER
        * @systemapi Hide this for inner system use.
    */
    ErrorCode GetPixelMap(
        int32_t wallpaperType, const ApiInfo &apiInfo, std::shared_ptr<OHOS::Media::PixelMap> &PixelMap);

    /**
     * Obtains the WallpaperColorsCollection instance for the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of array callback function
     */
    ErrorCode GetColors(int32_t wallpaperType, const ApiInfo &apiInfo, std::vector<uint64_t> &colors);

    /**
     * Obtains the ID of the wallpaper of the specified type.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return number type of callback function
     */
    int32_t GetWallpaperId(int32_t wallpaperType);

    ErrorCode GetFile(int32_t wallpaperType, int32_t &wallpaperFd);

    /**
     * Obtains the minimum height of the wallpaper.
     * @return number type of callback function
     */
    ErrorCode GetWallpaperMinHeight(const ApiInfo &apiInfo, int32_t &minHeight);

    /**
     * Obtains the minimum width of the wallpaper.
     * @return number type of callback function
     */
    ErrorCode GetWallpaperMinWidth(const ApiInfo &apiInfo, int32_t &minWidth);

    /**
     * Checks whether to allow the application to change the wallpaper for the current user.
     * @return boolean type of callback function
     */
    bool IsChangePermitted();

    /**
     * Checks whether a user is allowed to set wallpapers.
     * @return boolean type of callback function
     */
    bool IsOperationAllowed();

    /**
     * Removes a wallpaper of the specified type and restores the default one.
     * @param wallpaperType  Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @permission ohos.permission.SET_WALLPAPER
     */
    ErrorCode ResetWallpaper(std::int32_t wallpaperType, const ApiInfo &apiInfo);

    /**
     * Registers a listener for wallpaper event to receive notifications about the changes.
     * @param type event type
     * @param listener event listener
     * @return error code
     */
    ErrorCode On(const std::string &type, std::shared_ptr<WallpaperEventListener> listener);

    /**
     * Unregisters a listener for wallpaper event to receive notifications about the changes.
     * @param type event type
     * @param listener event listener
     * @return error code
     */
    ErrorCode Off(const std::string &type, std::shared_ptr<WallpaperEventListener> listener);

    /**
     * Sets live wallpaper of the specified type based on the uri path of the MP4 file.
     * @param uri Indicates the uri path of the MP4 file.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return ErrorCode
     * @permission ohos.permission.SET_WALLPAPER
     */
    ErrorCode SetVideo(const std::string &uri, const int32_t wallpaperType);

    /**
     * Sets custom wallpaper of the specified type based on the uri path.
     * @param uri Indicates the uri path.
     * @param wallpaperType Wallpaper type, values for WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN
     * @return ErrorCode
     * @permission ohos.permission.SET_WALLPAPER
     */
    ErrorCode SetCustomWallpaper(const std::string &uri, int32_t wallpaperType);

    /**
     * The application sends the event to the wallpaper service.
     * @param eventType Event type, values for SHOW_SYSTEMSCREEN or SHOW_LOCKSCREEN
     * @return ErrorCode
     * @permission ohos.permission.SET_WALLPAPER
     */
    ErrorCode SendEvent(const std::string &eventType);
    ErrorCode SetAllWallpapers(std::vector<WallpaperInfo> wallpaperInfo, int32_t wallpaperType);
    bool RegisterWallpaperCallback(JScallback callback);
    ErrorCode GetCorrespondWallpaper(int32_t wallpaperType, int32_t foldState, int32_t rotateState,
        std::shared_ptr<OHOS::Media::PixelMap> &pixelMap);

    JScallback GetCallback();

    void SetCallback(JScallback cb);

    void ReporterFault(FaultType faultType, FaultCode faultCode);

    void CloseWallpaperFd(int32_t wallpaperType);

    bool RegisterWallpaperListener();
    bool IsDefaultWallpaperResource(int32_t userId, int32_t wallpaperType);

private:
    class DeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        explicit DeathRecipient();
        ~DeathRecipient() override;
        DISALLOW_COPY_AND_MOVE(DeathRecipient);

        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };

    template<typename F, typename... Args> ErrCode CallService(F func, Args &&...args);
    bool CheckVideoFormat(const std::string &fileName);
    void ResetService(const wptr<IRemoteObject> &remote);
    sptr<IWallpaperService> GetService();
    int64_t WritePixelMapToStream(std::ostream &outputStream, std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
    FILE *OpenFile(const std::string &fileName, int &fd, int64_t &fileSize);
    ErrorCode CheckWallpaperFormat(const std::string &realPath, bool isLive);
    ErrorCode GetWallpaperSize(const std::string &realPath, bool isLive, int32_t &leng);
    ErrorCode CreatePixelMapByFd(int32_t fd, int32_t size, std::shared_ptr<OHOS::Media::PixelMap> &pixelMap);
    ErrorCode GetFdByPath(
        const WallpaperInfo &wallpaperInfo, WallpaperPictureInfo &wallpaperPictureInfo, std::string fileRealPath);
    void CloseWallpaperInfoFd(std::vector<WallpaperPictureInfo> wallpaperPictureInfos);
    int32_t ConverString2Int(const std::string &value);
    ErrorCode ConvertIntToErrorCode(int32_t errorCode);
    sptr<IWallpaperService> wallpaperProxy_{};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_{};
    std::mutex wallpaperFdLock_;
    std::map<int32_t, int32_t> wallpaperFdMap_;
    std::mutex wallpaperProxyLock_;
    std::mutex listenerMapLock_;
    std::map<std::string, sptr<WallpaperEventListenerClient>> listenerMap_;
    bool (*callback)(int32_t);
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif
