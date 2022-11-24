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
    int32_t SetWallpaperByFD(int fd, int wallpaperType, int length) override;
    int32_t SetWallpaperByMap(int fd, int wallpaperType, int length) override;
    int32_t GetPixelMap(int wallpaperType, IWallpaperService::FdInfo &fdInfo) override;
    std::vector<uint64_t> GetColors(int wallpaperType) override;
    int32_t GetFile(int wallpaperType, int32_t &wallpaperFd) override;
    int GetWallpaperId(int wallpaperType) override;
    int GetWallpaperMinHeight() override;
    int GetWallpaperMinWidth() override;
    bool IsChangePermitted() override;
    bool IsOperationAllowed() override;
    int32_t ResetWallpaper(int wallpaperType) override;
    bool ScreenshotLiveWallpaper(int sacleNumber, OHOS::Media::PixelMap pixelMap) override;
    bool On(sptr<IWallpaperColorChangeListener> listener) override;
    bool Off(sptr<IWallpaperColorChangeListener> listener) override;
    bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) override;
    std::string getUrl();

private:
    bool StartSocket(std::string url, std::string name);
    static inline BrokerDelegator<WallpaperServiceProxy> delegator_;
    sptr<OHOS::Media::PixelMap> Pmdata_;
    std::string FWReadUrl = "";
    int32_t mapSize = 0;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_PROXY_H