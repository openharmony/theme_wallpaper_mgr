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
#include "pixel_map.h"
#include "token_setproc.h"
#include "wallpaper_manager.h"
#include "wallpaper_manager_client.h"
#include "wallpaper_manager_common_info.h"

using namespace OHOS::Security::AccessToken;
using namespace OHOS::Media;

namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr size_t LENGTH = 2;
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
    if (size < LENGTH) {
        return;
    }
    WallpaperMgrService::ApiInfo apiInfo{ static_cast<bool>(data[0] % 2), static_cast<bool>(data[1] % 2) };
    std::vector<uint64_t> colors;
    WallpaperMgrService::WallpaperManager::GetInstance().GetColors(wallpaperType, apiInfo, colors);
}

void GetWallpaperIdFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    WallpaperMgrService::WallpaperManager::GetInstance().GetWallpaperId(wallpaperType);
}

void ResetWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    if (size < LENGTH) {
        return;
    }
    WallpaperMgrService::ApiInfo apiInfo{ static_cast<bool>(data[0] % 2), static_cast<bool>(data[1] % 2) };
    WallpaperMgrService::WallpaperManager::GetInstance().ResetWallpaper(wallpaperType, apiInfo);
}

void SetWallpaperByUriFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    data = data + OFFSET;
    size = size - OFFSET;
    GrantNativePermission();
    std::string uri(reinterpret_cast<const char *>(data), size);
    if (size < LENGTH) {
        return;
    }
    WallpaperMgrService::ApiInfo apiInfo{ static_cast<bool>(data[0] % 2), static_cast<bool>(data[1] % 2) };
    auto listener = std::make_shared<WallpaperEventListenerFuzzTestImpl>();
    WallpaperMgrService::WallpaperManager::GetInstance().On("colorChange", listener);
    WallpaperMgrService::WallpaperManager::GetInstance().SetWallpaper(uri, wallpaperType, apiInfo);
    WallpaperMgrService::WallpaperManager::GetInstance().Off("colorChange", listener);
}

void SetWallpaperByMapFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> uniquePixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMap = std::move(uniquePixelMap);
    auto listener = std::make_shared<WallpaperEventListenerFuzzTestImpl>();
    if (size < LENGTH) {
        return;
    }
    WallpaperMgrService::ApiInfo apiInfo{ static_cast<bool>(data[0] % 2), static_cast<bool>(data[1] % 2) };
    WallpaperMgrService::WallpaperManager::GetInstance().On("colorChange", listener);
    WallpaperMgrService::WallpaperManager::GetInstance().SetWallpaper(pixelMap, wallpaperType, apiInfo);
    WallpaperMgrService::WallpaperManager::GetInstance().Off("colorChange", listener);
}

void GetFileFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    int32_t wallpaperFd = 0;
    WallpaperMgrService::WallpaperManager::GetInstance().GetFile(wallpaperType, wallpaperFd);
}

void WallpaperManagerFuzzTest(const uint8_t *data, size_t size)
{
    auto minHeight = static_cast<int32_t>(size);
    auto minWidth = static_cast<int32_t>(size);
    if (size < LENGTH) {
        return;
    }
    WallpaperMgrService::ApiInfo apiInfo{ static_cast<bool>(data[0] % 2), static_cast<bool>(data[1] % 2) };
    GrantNativePermission();
    WallpaperMgrService::WallpaperManager::GetInstance().GetWallpaperMinHeight(apiInfo, minHeight);
    WallpaperMgrService::WallpaperManager::GetInstance().GetWallpaperMinWidth(apiInfo, minWidth);
    WallpaperMgrService::WallpaperManager::GetInstance().IsChangePermitted();
    WallpaperMgrService::WallpaperManager::GetInstance().IsOperationAllowed();
}

void GetPixelMapFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    if (size < LENGTH) {
        return;
    }
    WallpaperMgrService::ApiInfo apiInfo{ static_cast<bool>(data[0] % 2), static_cast<bool>(data[1] % 2) };
    WallpaperMgrService::WallpaperManager::GetInstance().GetPixelMap(wallpaperType, apiInfo, pixelMap);
}

void SetVideoFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    data = data + OFFSET;
    size = size - OFFSET;
    GrantNativePermission();
    std::string uri(reinterpret_cast<const char *>(data), size);
    WallpaperMgrService::WallpaperManager::GetInstance().SetVideo(uri, wallpaperType);
}

void SendEventFuzzTest(const uint8_t *data, size_t size)
{
    data = data + OFFSET;
    size = size - OFFSET;
    GrantNativePermission();
    std::string eventType(reinterpret_cast<const char *>(data), size);
    WallpaperMgrService::WallpaperManager::GetInstance().SendEvent(eventType);
}

void SetCustomWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    data = data + OFFSET;
    size = size - OFFSET;
    GrantNativePermission();
    std::string uri(reinterpret_cast<const char *>(data), size);
    if (size < LENGTH) {
        return;
    }
    WallpaperMgrService::WallpaperManager::GetInstance().SetCustomWallpaper(uri, wallpaperType);
}

void SetAllWallpapersFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    data = data + OFFSET;
    size = size - OFFSET;
    GrantNativePermission();
    std::vector<WallpaperInfo> allWallpaperInfos;
    WallpaperInfo wallpaperInfo;
    wallpaperInfo.foldState = FoldState::NORMAL;
    wallpaperInfo.rotateState = RotateState::PORT;
    wallpaperInfo.source = std::string(reinterpret_cast<const char *>(data), size);
    allWallpaperInfos.push_back(wallpaperInfo);
    WallpaperMgrService::WallpaperManager::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
    wallpaperType = 0;
    WallpaperMgrService::WallpaperManager::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
}

void GetCorrespondWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    WallpaperMgrService::WallpaperManager::GetInstance().GetCorrespondWallpaper(
        wallpaperType, FoldState::NORMAL, RotateState::PORT, pixelMap);
}

void IsDefaultWallpaperResourceFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t userId = ConvertToUint32(data);
    data = data + OFFSET;
    size = size - OFFSET;
    uint32_t wallpaperType = ConvertToUint32(data);
    GrantNativePermission();
    WallpaperMgrService::WallpaperManagerClient::GetInstance().IsDefaultWallpaperResource(userId, wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().IsDefaultWallpaperResource(userId, wallpaperType);
}

void SetAllWallpapersClientFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t wallpaperType = ConvertToUint32(data);
    data = data + OFFSET;
    size = size - OFFSET;
    GrantNativePermission();
    std::vector<WallpaperInfo> allWallpaperInfos;
    WallpaperInfo wallpaperInfo;
    wallpaperInfo.foldState = FoldState::NORMAL;
    wallpaperInfo.rotateState = RotateState::PORT;
    wallpaperInfo.source = std::string(reinterpret_cast<const char *>(data), size);
    allWallpaperInfos.push_back(wallpaperInfo);
    WallpaperMgrService::WallpaperManagerClient::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
    wallpaperType = 0;
    WallpaperMgrService::WallpaperManagerClient::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
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
    OHOS::SetWallpaperByUriFuzzTest(data, size);
    OHOS::SetWallpaperByMapFuzzTest(data, size);
    OHOS::GetFileFuzzTest(data, size);
    OHOS::WallpaperManagerFuzzTest(data, size);
    OHOS::GetPixelMapFuzzTest(data, size);
    OHOS::SendEventFuzzTest(data, size);
    OHOS::SetVideoFuzzTest(data, size);
    OHOS::SetCustomWallpaperFuzzTest(data, size);
    OHOS::SetAllWallpapersFuzzTest(data, size);
    OHOS::GetCorrespondWallpaperFuzzTest(data, size);
    OHOS::IsDefaultWallpaperResourceFuzzTest(data, size);
    OHOS::SetAllWallpapersClientFuzzTest(data, size);
    return 0;
}