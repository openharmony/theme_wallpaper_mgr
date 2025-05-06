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

#include <fuzzer/FuzzedDataProvider.h>

#include <cstdint>
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

using namespace OHOS::Security::AccessToken;
using namespace OHOS::Media;

namespace OHOS {
constexpr const char *WALLPAPER_DEFAULT_FILEFULLPATH = "/system/etc/wallpaperdefault.jpeg";
constexpr int32_t PIXELMAP_SIZE = 100;
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

void GetColorsFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    WallpaperMgrService::ApiInfo apiInfo{ provider.ConsumeBool(), provider.ConsumeBool() };
    std::vector<uint64_t> colors;
    WallpaperMgrService::WallpaperManager::GetInstance().GetColors(wallpaperType, apiInfo, colors);
}

void GetWallpaperIdFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    WallpaperMgrService::WallpaperManager::GetInstance().GetWallpaperId(wallpaperType);
}

void ResetWallpaperFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    WallpaperMgrService::ApiInfo apiInfo{ provider.ConsumeBool(), provider.ConsumeBool() };
    WallpaperMgrService::WallpaperManager::GetInstance().ResetWallpaper(wallpaperType, apiInfo);
}

void SetWallpaperByUriFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    std::string uri = provider.ConsumeRandomLengthString();
    WallpaperMgrService::ApiInfo apiInfo{ provider.ConsumeBool(), provider.ConsumeBool() };
    auto listener = std::make_shared<WallpaperEventListenerFuzzTestImpl>();
    WallpaperMgrService::WallpaperManager::GetInstance().On("colorChange", listener);
    WallpaperMgrService::WallpaperManager::GetInstance().SetWallpaper(uri, wallpaperType, apiInfo);
    WallpaperMgrService::WallpaperManager::GetInstance().Off("colorChange", listener);
    WallpaperMgrService::WallpaperManager::GetInstance().SetWallpaper(
        WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperType, apiInfo);
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

void GetFileFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    int32_t wallpaperFd = -1;
    WallpaperMgrService::WallpaperManager::GetInstance().GetFile(wallpaperType, wallpaperFd);
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

void GetPixelMapFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<int32_t>();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    WallpaperMgrService::ApiInfo apiInfo{ provider.ConsumeBool(), provider.ConsumeBool() };
    WallpaperMgrService::WallpaperManager::GetInstance().GetPixelMap(wallpaperType, apiInfo, pixelMap);
}

void SetVideoFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    std::string uri = provider.ConsumeRandomLengthString();
    WallpaperMgrService::WallpaperManager::GetInstance().SetVideo(uri, wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().SetVideo(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperType);
}

void SendEventFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    std::string eventType = provider.ConsumeRandomLengthString();
    WallpaperMgrService::WallpaperManager::GetInstance().SendEvent(eventType);
}

void SetCustomWallpaperFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    std::string uri = provider.ConsumeRandomLengthString();
    WallpaperMgrService::WallpaperManager::GetInstance().SetCustomWallpaper(uri, wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().SetCustomWallpaper(
        WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperType);
}

void SetAllWallpapersFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    std::vector<WallpaperInfo> allWallpaperInfos;
    WallpaperInfo wallpaperInfo;
    wallpaperInfo.foldState = FoldState::NORMAL;
    wallpaperInfo.rotateState = RotateState::PORT;
    wallpaperInfo.source = provider.ConsumeRandomLengthString();
    allWallpaperInfos.push_back(wallpaperInfo);
    WallpaperMgrService::WallpaperManager::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
    wallpaperInfo.source = WALLPAPER_DEFAULT_FILEFULLPATH;
    allWallpaperInfos.push_back(wallpaperInfo);
    WallpaperMgrService::WallpaperManager::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
}

void GetCorrespondWallpaperFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    WallpaperMgrService::WallpaperManager::GetInstance().GetCorrespondWallpaper(
        wallpaperType, FoldState::NORMAL, RotateState::PORT, pixelMap);
}

void IsDefaultWallpaperResourceFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t userId = provider.ConsumeIntegral<uint32_t>();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    WallpaperMgrService::WallpaperManagerClient::GetInstance().IsDefaultWallpaperResource(userId, wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().IsDefaultWallpaperResource(userId, wallpaperType);
}

void SetAllWallpapersClientFuzzTest(FuzzedDataProvider &provider)
{
    GrantNativePermission();
    uint32_t wallpaperType = provider.ConsumeIntegral<uint32_t>();
    std::vector<WallpaperInfo> allWallpaperInfos;
    WallpaperInfo wallpaperInfo;
    wallpaperInfo.foldState = FoldState::NORMAL;
    wallpaperInfo.rotateState = RotateState::PORT;
    wallpaperInfo.source = provider.ConsumeRandomLengthString();
    allWallpaperInfos.push_back(wallpaperInfo);
    WallpaperMgrService::WallpaperManagerClient::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
    wallpaperType = 0;
    WallpaperMgrService::WallpaperManagerClient::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
    WallpaperMgrService::WallpaperManager::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
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
    OHOS::GetColorsFuzzTest(provider);
    OHOS::GetWallpaperIdFuzzTest(provider);
    OHOS::ResetWallpaperFuzzTest(provider);
    OHOS::SetWallpaperByUriFuzzTest(provider);
    OHOS::SetWallpaperByMapFuzzTest(provider);
    OHOS::GetFileFuzzTest(provider);
    OHOS::WallpaperManagerFuzzTest(provider);
    OHOS::GetPixelMapFuzzTest(provider);
    OHOS::SendEventFuzzTest(provider);
    OHOS::SetVideoFuzzTest(provider);
    OHOS::SetCustomWallpaperFuzzTest(provider);
    OHOS::SetAllWallpapersFuzzTest(provider);
    OHOS::GetCorrespondWallpaperFuzzTest(provider);
    OHOS::IsDefaultWallpaperResourceFuzzTest(provider);
    OHOS::WallpaperManagerProxyFuzzTest(provider);
    OHOS::IsDefaultWallpaperResourceFuzzTest(provider);
    return 0;
}