/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <fuzzer/FuzzedDataProvider.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <iostream>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "pixel_map.h"
#include "token_setproc.h"
#include "wallpaper_common.h"
#include "wallpaper_manager.h"
#include "wallpaper_manager_client.h"
#include "wallpaper_manager_common_info.h"
#include "wallpaper_picture_info_by_parcel.h"
#include "wallpaper_rawdata.h"
#include "setwallpaperbypixelmap_fuzzer.h"

using namespace OHOS::Media;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
static const char *g_perms[1] = { "ohos.permission.SET_WALLPAPER" };
void GrantNativePermission()
{
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = g_perms,
        .acls = nullptr,
        .processName = "wallpaper_service",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    AccessTokenKit::ReloadNativeTokenInfo();
}

class WallpaperEventListenerFuzzTestImpl : public OHOS::WallpaperMgrService::WallpaperEventListener {
public:
    std::vector<uint64_t> color_;
    int32_t wallpaperType_;
    WallpaperEventListenerFuzzTestImpl();
    ~WallpaperEventListenerFuzzTestImpl()
    {
    }

    WallpaperEventListenerFuzzTestImpl(const WallpaperEventListenerFuzzTestImpl &) = delete;
    WallpaperEventListenerFuzzTestImpl &operator=(const WallpaperEventListenerFuzzTestImpl &) = delete;
    WallpaperEventListenerFuzzTestImpl(WallpaperEventListenerFuzzTestImpl &&) = delete;
    WallpaperEventListenerFuzzTestImpl &operator=(WallpaperEventListenerFuzzTestImpl &&) = delete;

    // callback function will be called when the db data is changed.
    void OnColorsChange(const std::vector<uint64_t> &color, int32_t wallpaperType);

private:
    unsigned long callCount_;
};

void WallpaperEventListenerFuzzTestImpl::OnColorsChange(const std::vector<uint64_t> &color, int32_t wallpaperType)
{
    callCount_++;
    std::copy(color.begin(), color.end(), std::back_inserter(color_));
    wallpaperType_ = wallpaperType;
}

WallpaperEventListenerFuzzTestImpl::WallpaperEventListenerFuzzTestImpl() : wallpaperType_(-1), callCount_(0)
{
}

void SetWallpaperByMapFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> uniquePixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMap = std::move(uniquePixelMap);
    auto listener = std::make_shared<WallpaperEventListenerFuzzTestImpl>();
    WallpaperMgrService::ApiInfo apiInfo{ provider.ConsumeBool(), provider.ConsumeBool() };
    WallpaperMgrService::WallpaperManager::GetInstance().On("colorChange", listener);
    WallpaperMgrService::WallpaperManager::GetInstance().SetWallpaper(pixelMap, wallpaperType, apiInfo);
    WallpaperMgrService::WallpaperManager::GetInstance().Off("colorChange", listener);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    /* Run your code on data */
    OHOS::SetWallpaperByMapFuzzTest(provider);
    return 0;
}