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
#include "wallpapermanager_fuzzer.h"

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

void WallpaperManagerCallbackFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> uniquePixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMap = std::move(uniquePixelMap);
    int32_t size = provider.ConsumeIntegral<int32_t>();
    int32_t fd = provider.ConsumeIntegral<int32_t>();
    WallpaperMgrService::WallpaperManager::GetInstance().CreatePixelMapByFd(fd, size, pixelMap);
    std::string wallpaperPath = "/data/service/el1/public/wallpaper/100/system/wallpaper_home";
    int32_t wallpaperFd = open(wallpaperPath.c_str(), O_RDONLY, S_IREAD);
    FILE *file = fopen(wallpaperPath.c_str(), "rb");
    if (file == nullptr) {
        close(wallpaperFd);
        return;
    }
    int32_t fend = fseek(file, 0, SEEK_END);
    size = ftell(file);
    int32_t fset = fseek(file, 0, SEEK_SET);
    if (size <= 0 || fend != 0 || fset != 0) {
        fclose(file);
        close(wallpaperFd);
        return;
    }
    if (fclose(file) < 0) {
        close(wallpaperFd);
        return;
    }
    WallpaperMgrService::WallpaperManager::GetInstance().CreatePixelMapByFd(wallpaperFd, size, pixelMap);
    close(wallpaperFd);

    auto wallpaperProxy = WallpaperMgrService::WallpaperManager::GetInstance().GetService();
    JScallback callback = nullptr;
    WallpaperMgrService::WallpaperManager::GetInstance().RegisterWallpaperCallback(callback);
    bool registerWallpaperCallback;
    sptr<IWallpaperCallback> wallpaperCallback;
    wallpaperProxy->RegisterWallpaperCallback(wallpaperCallback, registerWallpaperCallback);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    /* Run your code on data */
    OHOS::WallpaperManagerCallbackFuzzTest(provider);
    return 0;
}