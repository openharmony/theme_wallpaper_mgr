/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "clearredundatfile_fuzzer.h"
#include "nativetoken_kit.h"
#include "pixel_map.h"
#include "token_setproc.h"
#include "wallpaper_service.h"

using namespace OHOS::Security::AccessToken;
using namespace OHOS::Media;

using namespace OHOS::WallpaperMgrService;
namespace OHOS {
WallpaperMgrService WallpaperMgrService;
constexpr size_t LENGTH = 2;
constexpr size_t THRESHOLD = 4;

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

int32_t ConverToInt32(const uint8_t *ptr, size_t size)
{
    if (ptr == nullptr || (size < sizeof(int32_t))) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    if (bigVar < 0) {
        return 0;
    }
    return bigVar;
}

void ClearRedundatFileFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    WallpaperMgrService.ClearRedundantFile(userId, wallpaperType, path);
}

void GetPictureFileNameFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    WallpaperMgrService.GetPictureFileName(userId, wallpaperType, path);
}

void GetFileNameFromMapFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    WallpaperMgrService.GetFileNameFromMap(userId, wallpaperType, path);
}

void GetFileFuzzTest(const uint8_t *data, size_t size)
{
    int32_t fd = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.GetFile(wallpaperType, fd);
}

void GetWallpaperDirFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.GetWallpaperDir(userId, wallpaperType);
}

void InitUserDirFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    WallpaperMgrService.InitUserDir(userId);
}

void OnInitUserFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    WallpaperMgrService.OnInitUser(userId);
}

void OnRemovedUserFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    WallpaperMgrService.OnRemovedUser(userId);
}

void OnSwitchedUserFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    WallpaperMgrService.OnSwitchedUser(userId);
}

void UpdataWallpaperMapFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.UpdataWallpaperMap(userId, wallpaperType);
}

void GetColorsFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    std::vector<uint64_t> colors;
    WallpaperMgrService.GetColors(wallpaperType, colors);
}

void GetColorsV9FuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    std::vector<uint64_t> colors;
    WallpaperMgrService.GetColorsV9(wallpaperType, colors);
}

void GetFileFuzzTest(const uint8_t *data, size_t size)
{
    int32_t wallpaperType = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperFd = ConverToInt32(data, size);
    WallpaperMgrService.GetFile(wallpaperType, wallpaperFd);
}

void SaveColorFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    WallpaperMgrService.SaveColor(userId, wallpaperType);
}

void SetWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    int32_t fd = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    WallpaperMgrService.SetWallpaper(userId, wallpaperType, length);
}

void SetWallpaperByPixelMapFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.SetWallpaperByPixelMap(pixelMap, wallpaperType);
}

void SetWallpaperV9FuzzTest(const uint8_t *data, size_t size)
{
    int32_t fd = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    WallpaperMgrService.SetWallpaperV9(userId, wallpaperType, length);
}

void SetWallpaperByPixelMapFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.SetWallpaperByPixelMap(pixelMap, wallpaperType);
}

void SetWallpaperBackupDataFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t resourceTypeInt = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(resourceTypeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string uriOrPixelMap = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.SetWallpaperBackupData(userId, resourceType, uriOrPixelMap, wallpaperType);
}

void GetResTypeFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.GetResType(userId, wallpaperType);
}

void SendEventFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    std::string eventType = std::string((const char *)data, size);
    WallpaperMgrService.SendEvent(eventType);
}

void SendWallpaperChangeEventFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.SendWallpaperChangeEvent(userId, wallpaperType);
}

void SetVideoFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t fd = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    WallpaperMgrService.SetVideo(fd, wallpaperType, length);
}

void SetCustomWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t fd = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t type = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    WallpaperMgrService.SetCustomWallpaper(fd, type, length);
}

void GetWallpaperIdFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.GetWallpaperId(wallpaperType);
}

void ResetWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.ResetWallpaper(wallpaperType);
}

void ResetWallpaperV9FuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.ResetWallpaperV9(wallpaperType);
}

void SetDefaultDataForWallpaperFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    WallpaperMgrService.SetDefaultDataForWallpaper(userId, wallpaperType);
}

void ClearWallpaperLockedFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    WallpaperMgrService.ClearWallpaperLocked(userId, wallpaperType);
}

void GetImageFdFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t fd = ConverToInt32(data, size);
    WallpaperMgrService.GetImageFd(userId, wallpaperType, fd);
}

void GetImageSizeFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t size = ConverToInt32(data, size);
    WallpaperMgrService.GetImageSize(userId, wallpaperType, size);
}

void CheckUserPermissionByIdFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    WallpaperMgrService.CheckUserPermissionById(userId);
}

void ClearRedundatFileServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaper = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(wallpaper);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string uriOrPixelMap = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.ClearRedundantFile(userId, wallpaperType, path);
}

void GetPictureFileNameServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    int32_t wallpaper = ConverToInt32(data, size);
    WallpaperResourceType wallpaperTypes = static_cast<WallpaperResourceType>(wallpaper);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.GetPictureFileName(userId, wallpaperType, path);
}

void GetFileNameFromMapServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    int32_t wallpaperManger = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(wallpaperManger);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.GetFileNameFromMap(userId, wallpaperType, path);
}

void GetFileFuzzServiceTest(const uint8_t *data, size_t size)
{
    int32_t fd = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    int32_t wallpaper = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(wallpaper);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.GetFile(wallpaperType, fd);
}

void GetWallpaperDirServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicepath = std::string((const char *)data, size);
    int32_t wallpaperManger = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(wallpaperManger);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt32 = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt32);
    WallpaperMgrService.GetWallpaperDir(userId, wallpaperType);
}

void InitUserDirFuzzServiceTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    int32_t wallpaperTypes = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    int32_t resourceTypes = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(resourceTypes);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt32 = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt32);
    WallpaperMgrService.InitUserDir(userId);
}

void OnInitUserFuzzServiceTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    int32_t wallpaperTypesService = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    int32_t resourceTypes = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(resourceTypes);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt32 = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt32);
    WallpaperMgrService.OnInitUser(userId);
}

void OnRemovedUserFuzzServiceTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    int32_t wallpaperTypesService = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt32 = ConverToInt32(data, size);
    WallpaperType wallpaperTypes = static_cast<WallpaperType>(wallpaperTypeInt32);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    int32_t resourceTypes = ConverToInt32(data, size);
    WallpaperResourceType resourceTypeManager = static_cast<WallpaperResourceType>(resourceTypes);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt32 = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt32);
    WallpaperMgrService.OnRemovedUser(userId);
}

void OnSwitchedUserServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypesService = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperTypes = static_cast<WallpaperType>(wallpaperTypeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    int32_t resourceTypes = ConverToInt32(data, size);
    WallpaperResourceType resourcesType = static_cast<WallpaperResourceType>(resourceTypes);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInts = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInts);
    WallpaperMgrService.OnSwitchedUser(userId);
}

void UpdataWallpaperMapServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperMangerPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t resourceTypes = ConverToInt32(data, size);
    WallpaperResourceType resourcesType = static_cast<WallpaperResourceType>(resourceTypes);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInts = ConverToInt32(data, size);
    WallpaperType wallpaperTypes = static_cast<WallpaperType>(wallpaperTypeInts);
    WallpaperMgrService.UpdataWallpaperMap(userId, wallpaperType);
}

void GetColorsServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    std::vector<uint64_t> colors;
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperMangerPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t resourceTypes = ConverToInt32(data, size);
    WallpaperResourceType resourcesType = static_cast<WallpaperResourceType>(resourceTypes);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInts = ConverToInt32(data, size);
    WallpaperType wallpaperTypes = static_cast<WallpaperType>(wallpaperTypeInts);
    WallpaperMgrService.GetColors(wallpaperType, colors);
}

void GetColorsV9ServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    std::vector<uint64_t> colors;
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperManagerPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t resourceType = ConverToInt32(data, size);
    WallpaperResourceType resourcesType = static_cast<WallpaperResourceType>(resourceType);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInts = ConverToInt32(data, size);
    WallpaperType wallpaperTypes = static_cast<WallpaperType>(wallpaperTypeInts);
    WallpaperMgrService.GetColorsV9(wallpaperType, colors);
}

void GetFileServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t wallpaperType = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperFd = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t resourceType = ConverToInt32(data, size);
    WallpaperResourceType resourcesType = static_cast<WallpaperResourceType>(resourceType);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperServicePath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInts = ConverToInt32(data, size);
    WallpaperType wallpaperTypes = static_cast<WallpaperType>(wallpaperTypeInts);
    WallpaperMgrService.GetFile(wallpaperType, wallpaperFd);
}

void SaveColorServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypes = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperManagerPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInts = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    WallpaperMgrService.SaveColor(userId, wallpaperType);
}

void SetWallpaperServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t fd = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypes = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string paths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    WallpaperMgrService.SetWallpaper(userId, wallpaperType, length);
}

void SetWallpaperByPixelMapServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string paths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    WallpaperMgrService.SetWallpaperByPixelMap(pixelMap, wallpaperType);
}

void SetWallpaperV9ServiceFuzzTest(const uint8_t *data, size_t size)
{
    int32_t fd = ConverToInt32(data, size);
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string paths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    WallpaperMgrService.SetWallpaperV9(userId, wallpaperType, length);
}

void SetWallpaperByPixelMapServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string paths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.SetWallpaperByPixelMap(pixelMap, wallpaperType);
}

void SetWallpaperBackupDataServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string paths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t resourceTypeInt = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(resourceTypeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string uriOrPixelMap = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.SetWallpaperBackupData(userId, resourceType, uriOrPixelMap, wallpaperType);
}

void GetResTypeFuzzServiceTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string paths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPath = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    WallpaperMgrService.GetResType(userId, wallpaperType);
}

void SendEventFuzzServiceTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t resourceTypeInt = ConverToInt32(data, size);
    WallpaperResourceType resourceType = static_cast<WallpaperResourceType>(resourceTypeInt);
    std::string eventType = std::string((const char *)data, size);
    WallpaperMgrService.SendEvent(eventType);
}

void SendWallpaperChangeEventServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTypeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTypeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    WallpaperMgrService.SendWallpaperChangeEvent(userId, wallpaperType);
}

void SetVideoServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t fd = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    WallpaperMgrService.SetVideo(fd, wallpaperType, length);
}

void SetCustomWallpaperServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t fd = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    int32_t type = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t length = ConverToInt32(data, size);
    WallpaperMgrService.SetCustomWallpaper(fd, type, length);
}

void GetWallpaperIdServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    int32_t wallpaperType = ConverToInt32(data, size);
    WallpaperMgrService.GetWallpaperId(wallpaperType);
}

void ResetWallpaperServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    WallpaperMgrService.ResetWallpaper(wallpaperType);
}

void ResetWallpaperV9ServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t wallpaperType = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    WallpaperMgrService.ResetWallpaperV9(wallpaperType);
}

void SetDefaultDataForWallpaperServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    WallpaperMgrService.SetDefaultDataForWallpaper(userId, wallpaperType);
}

void ClearWallpaperLockedServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    WallpaperMgrService.ClearWallpaperLocked(userId, wallpaperType);
}

void GetImageFdServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t fd = ConverToInt32(data, size);
    WallpaperMgrService.GetImageFd(userId, wallpaperType, fd);
}

void GetImageSizeServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t wallpaperTpyeInt = ConverToInt32(data, size);
    WallpaperType wallpaperType = static_cast<WallpaperType>(wallpaperTpyeInt);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    int32_t size = ConverToInt32(data, size);
    WallpaperMgrService.GetImageSize(userId, wallpaperType, size);
}

void CheckUserPermissionByIdServiceFuzzTest(const uint8_t *data, size_t size)
{
    GrantNativePermission();
    int32_t userId = ConverToInt32(data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string wallpaperPaths = std::string((const char *)data, size);
    data = data + sizeof(int32_t);
    size = size - sizeof(int32_t);
    std::string Path = std::string((const char *)data, size);
    WallpaperMgrService.CheckUserPermissionById(userId);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < THRESHOLD) {
        return 0;
    }
    OHOS::ClearRedundatFileFuzzTest(data, size);
    return 0;
}
} // namespace OHOS