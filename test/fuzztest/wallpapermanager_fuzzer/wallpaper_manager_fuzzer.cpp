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

#include <cstdint>
#include <iostream>

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "wallpaper_manager_common_info.h"
#include "wallpaper_manager_kits.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t OFFSET = 4;

void GrantNativePermission()
{
    const char **perms = new const char *[2];
    perms[0] = "ohos.permission.GET_WALLPAPER";
    perms[1] = "ohos.permission.SET_WALLPAPER";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "wallpaper_service",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

void GetColorsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(wallpaperType);
}

void GetWallpaperIdFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(wallpaperType);
}

void ResetWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType);
}

void SetWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    data = data + OFFSET;
    size = size - OFFSET;
    GrantNativePermission();
    std::string url(reinterpret_cast<const char *>(data), size);
    WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(url, wallpaperType);
}

void GetFileFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    int32_t wallpaperFd = 0;
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(wallpaperType, wallpaperFd);
}

void GetPixelMapFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(wallpaperType, pixelMap);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::GetColorsFuzzTest(data, size);
    OHOS::GetWallpaperIdFuzzTest(data, size);
    OHOS::ResetWallpaperFuzzTest(data, size);
    OHOS::SetWallpaperFuzzTest(data, size);
    OHOS::GetFileFuzzTest(data, size);
    OHOS::GetPixelMapFuzzTest(data, size);
    return 0;
}