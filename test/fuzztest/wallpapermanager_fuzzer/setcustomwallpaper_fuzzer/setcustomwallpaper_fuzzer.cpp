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
#include "setcustomwallpaper_fuzzer.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
constexpr const char *WALLPAPER_DEFAULT_FILEFULLPATH = "/system/etc/wallpaperdefault.jpeg";
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

void SetCustomWallpaperFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    std::string uri = provider.ConsumeRandomLengthString();
    WallpaperMgrService::WallpaperManager::GetInstance().SetCustomWallpaper(uri, wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().SetCustomWallpaper(
        std::string(WALLPAPER_DEFAULT_FILEFULLPATH), wallpaperType);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    /* Run your code on data */
    OHOS::SetCustomWallpaperFuzzTest(provider);
    return 0;
}