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
#include "wallpapermanagerproxy_fuzzer.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
constexpr int32_t PIXELMAP_SIZE = 100;
static const char *g_perms[2] = { "ohos.permission.GET_WALLPAPER", "ohos.permission.SET_WALLPAPER" };
void GrantNativePermission()
{
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
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

void WallpaperManagerProxyFuzzTest(FuzzedDataProvider &provider)
{
    auto wallpaperProxy = WallpaperMgrService::WallpaperManager::GetInstance().GetService();
    int32_t fd = provider.ConsumeIntegral<int32_t>();
    int32_t wallpaperType = provider.ConsumeIntegral<int32_t>();
    long length = provider.ConsumeIntegral<long>();
    wallpaperProxy->SetWallpaperV9(fd, wallpaperType, length);
    wallpaperProxy->SetWallpaper(fd, wallpaperType, length);
    wallpaperProxy->SetVideo(fd, wallpaperType, length);
    wallpaperProxy->SetCustomWallpaper(fd, wallpaperType, length);

    std::vector<int8_t> value = provider.ConsumeBytes<int8_t>(PIXELMAP_SIZE);
    WallpaperMgrService::WallpaperRawData wallpaperRawData;
    wallpaperRawData.size = value.size();
    wallpaperRawData.data = value.data();
    wallpaperProxy->SetWallpaperByPixelMap(wallpaperRawData, wallpaperType);
    wallpaperProxy->SetWallpaperV9ByPixelMap(wallpaperRawData, wallpaperType);

    int32_t pixelmapSize;
    int32_t pixelmapFd;
    wallpaperProxy->GetPixelMapV9(wallpaperType, pixelmapSize, pixelmapFd);
    wallpaperProxy->GetPixelMap(wallpaperType, pixelmapSize, pixelmapFd);
    WallpaperMgrService::WallpaperPictureInfoByParcel wallpaperPictureInfoByParcel;

    WallpaperMgrService::WallpaperPictureInfo wallpaperPictureInfo;
    wallpaperPictureInfo.foldState = FoldState::NORMAL;
    wallpaperPictureInfo.rotateState = RotateState::PORT;
    wallpaperPictureInfo.fd = fd;
    wallpaperPictureInfo.length = length;
    wallpaperPictureInfoByParcel.wallpaperPictureInfo_.push_back(wallpaperPictureInfo);
    std::vector<int32_t> fdVector;
    fdVector.push_back(fd);
    wallpaperProxy->SetAllWallpapers(wallpaperPictureInfoByParcel, wallpaperType, fdVector);

    int32_t foldState = 0;
    int32_t rotateState = 0;
    wallpaperProxy->GetCorrespondWallpaper(wallpaperType, foldState, rotateState, pixelmapSize, pixelmapFd);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    /* Run your code on data */
    OHOS::WallpaperManagerProxyFuzzTest(provider);
    return 0;
}