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

#ifndef SERVICES_INCLUDE_WALLPAPER_SERVICE_PROXY_H
#define SERVICES_INCLUDE_WALLPAPER_SERVICE_PROXY_H

#include <mutex>

#include "i_wallpaper_service.h"
#include "iremote_proxy.h"
#include "wallpaper_service_ipc_interface_code.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperServiceProxy : public IRemoteProxy<IWallpaperService> {
public:
    explicit WallpaperServiceProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IWallpaperService>(object)
    {
    }
    ~WallpaperServiceProxy()
    {
    }
    DISALLOW_COPY_AND_MOVE(WallpaperServiceProxy);
    ErrorCode SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SetWallpaperByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType) override;
    ErrorCode GetPixelMap(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo) override;
    ErrorCode GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    ErrorCode GetFile(int32_t wallpaperType, int32_t &wallpaperFd) override;
    int32_t GetWallpaperId(int32_t wallpaperType) override;
    bool IsChangePermitted() override;
    bool IsOperationAllowed() override;
    ErrorCode ResetWallpaper(int wallpaperType) override;
    ErrorCode On(const std::string &type, sptr<IWallpaperEventListener> listener) override;
    ErrorCode Off(const std::string &type, sptr<IWallpaperEventListener> listener) override;
    bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) override;
    ErrorCode SetVideo(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SetCustomWallpaper(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SendEvent(const std::string &eventType) override;
    ErrorCode ConvertIntToErrorCode(int32_t errorCode);

    ErrorCode SetWallpaperV9(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SetWallpaperV9ByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType) override;
    ErrorCode GetPixelMapV9(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo) override;
    ErrorCode GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    ErrorCode ResetWallpaperV9(int wallpaperType) override;

private:
    ErrorCode SetWallpaperInner(
        int32_t fd, int32_t wallpaperType, int32_t length, WallpaperServiceIpcInterfaceCode code);
    ErrorCode SetWallpaperInnerByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap,
       int32_t wallpaperType, WallpaperServiceIpcInterfaceCode code);
    ErrorCode GetPixelMapInner(
        int32_t wallpaperType, WallpaperServiceIpcInterfaceCode code, IWallpaperService::FdInfo &fdInfo);
    ErrorCode GetColorsInner(
        int32_t wallpaperType, WallpaperServiceIpcInterfaceCode code, std::vector<uint64_t> &colors);
    ErrorCode ResetWallpaperInner(int wallpaperType, WallpaperServiceIpcInterfaceCode code);
    bool StartSocket(std::string uri, std::string name);
    std::vector<std::uint8_t> PixelMapToVector(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
    static inline BrokerDelegator<WallpaperServiceProxy> delegator_;
    sptr<OHOS::Media::PixelMap> pixelMapData_;
    std::string serviceReadUri = "";
    int32_t mapSize = 0;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_PROXY_H