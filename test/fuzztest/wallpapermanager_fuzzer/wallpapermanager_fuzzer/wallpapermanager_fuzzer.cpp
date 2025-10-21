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

void WallpaperManagerFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    auto minHeight = provider.ConsumeIntegral<int32_t>();
    auto minWidth = provider.ConsumeIntegral<int32_t>();
    WallpaperMgrService::ApiInfo apiInfo{ provider.ConsumeBool(), provider.ConsumeBool() };
    WallpaperMgrService::WallpaperManager::GetInstance().GetWallpaperMinHeight(apiInfo, minHeight);
    WallpaperMgrService::WallpaperManager::GetInstance().GetWallpaperMinWidth(apiInfo, minWidth);
    WallpaperMgrService::WallpaperManager::GetInstance().IsChangePermitted();
    WallpaperMgrService::WallpaperManager::GetInstance().IsOperationAllowed();
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> uniquePixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMap = std::move(uniquePixelMap);
    int32_t size = provider.ConsumeIntegral<int32_t>();
    int32_t fd = provider.ConsumeIntegral<int32_t>();
    WallpaperMgrService::WallpaperManager::GetInstance().CreatePixelMapByFd(fd, size, pixelMap);
    WallpaperMgrService::WallpaperManager::GetInstance().GetCallback();
    FaultType faultType = FaultType::SERVICE_FAULT;
    FaultCode FaultCode = FaultCode::SF_SERVICE_UNAVAILABLE;
    WallpaperMgrService::WallpaperManager::GetInstance().ReporterFault(faultType, FaultCode);
    int32_t wallpaperType = provider.ConsumeIntegral<int32_t>();
    WallpaperMgrService::WallpaperManager::GetInstance().CloseWallpaperFd(wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().RegisterWallpaperListener();
    std::string fileName = provider.ConsumeRandomLengthString();
    WallpaperMgrService::WallpaperManager::GetInstance().CheckVideoFormat(fileName);
    int64_t fileSize;
    int32_t fdVedio;
    WallpaperMgrService::WallpaperManager::GetInstance().OpenFile(fileName, fdVedio, fileSize);
    long fdLength;
    bool idLive = provider.ConsumeBool();
    WallpaperMgrService::WallpaperManager::GetInstance().CheckWallpaperFormat(fileName, idLive, fdLength);
    WallpaperInfo wallpaperInfo;
    wallpaperInfo.foldState = FoldState::NORMAL;
    wallpaperInfo.rotateState = RotateState::PORT;
    wallpaperInfo.source = provider.ConsumeRandomLengthString();
    WallpaperMgrService::WallpaperPictureInfo wallpaperPictureInfo;
    wallpaperPictureInfo.foldState = FoldState::NORMAL;
    wallpaperPictureInfo.rotateState = RotateState::PORT;
    wallpaperPictureInfo.fd = fd;
    wallpaperPictureInfo.length = provider.ConsumeIntegral<long>();
    WallpaperMgrService::WallpaperManager::GetInstance().GetFdByPath(wallpaperInfo, wallpaperPictureInfo, fileName);
    WallpaperMgrService::WallpaperManager::GetInstance().ConverString2Int(fileName);
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    /* Run your code on data */
    OHOS::WallpaperManagerFuzzTest(provider);
    return 0;
}