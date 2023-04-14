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
#include "iremote_proxy.h"
#include "i_wallpaper_service.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperServiceProxy : public IRemoteProxy<IWallpaperService> {
public:
    explicit WallpaperServiceProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IWallpaperService>(object) {}
    ~WallpaperServiceProxy() {}
    DISALLOW_COPY_AND_MOVE(WallpaperServiceProxy);
    int32_t SetWallpaperByFD(int32_t fd, int32_t wallpaperType, int32_t length) override;
    int32_t SetWallpaperByMap(int32_t fd, int32_t wallpaperType, int32_t length) override;
    int32_t GetPixelMap(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo) override;
    int32_t GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    int32_t GetFile(int32_t wallpaperType, int32_t &wallpaperFd) override;
    int32_t GetWallpaperId(int32_t wallpaperType) override;
    int32_t GetWallpaperMinHeight(int32_t &minHeight) override;
    int32_t GetWallpaperMinWidth(int32_t &minWidth) override;
    bool IsChangePermitted() override;
    bool IsOperationAllowed() override;
    int32_t ResetWallpaper(int wallpaperType) override;
    bool On(sptr<IWallpaperColorChangeListener> listener) override;
    bool Off(sptr<IWallpaperColorChangeListener> listener) override;
    bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) override;
    std::string getUrl();

    int32_t SetWallpaperByFDV9(int32_t fd, int32_t wallpaperType, int32_t length) override;
    int32_t SetWallpaperByMapV9(int32_t fd, int32_t wallpaperType, int32_t length) override;
    int32_t GetPixelMapV9(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo) override;
    int32_t GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    int32_t GetWallpaperMinHeightV9(int32_t &minHeight) override;
    int32_t GetWallpaperMinWidthV9(int32_t &minWidth) override;
    int32_t ResetWallpaperV9(int wallpaperType) override;

private:
    bool StartSocket(std::string url, std::string name);
    int32_t SetWallpaperByFDInner(int32_t fd, int32_t wallpaperType, int32_t length, uint32_t code);
    int32_t SetWallpaperByMapInner(int32_t fd, int32_t wallpaperType, int32_t length, uint32_t code);
    int32_t GetPixelMapInner(int32_t wallpaperType, uint32_t code, IWallpaperService::FdInfo &fdInfo);
    int32_t GetColorsInner(int32_t wallpaperType, uint32_t code, std::vector<uint64_t> &colors);
    int32_t GetWallpaperMinHeightInner(uint32_t code, int32_t &minHeight);
    int32_t GetWallpaperMinWidthInner(uint32_t code, int32_t &minWidth);
    int32_t ResetWallpaperInner(int wallpaperType, uint32_t code);
    static inline BrokerDelegator<WallpaperServiceProxy> delegator_;
    sptr<OHOS::Media::PixelMap> Pmdata_;
    std::string FWReadUrl = "";
    int32_t mapSize = 0;
};
}
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_PROXY_H