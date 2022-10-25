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

#include "wallpaper_manager_common_info.h"
#include "wallpaper_manager_kits.h"


namespace OHOS {
void WallpaperManagerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        std::cout << "Invalid data" << std::endl;
        return;
    }

    WallpaperType wallpaperType = *reinterpret_cast<const WallpaperType *>(data);
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(wallpaperType);
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(wallpaperType);
    WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType);


    int32_t wallpaperFd = 0;
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(wallpaperType,wallpaperFd);
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(wallpaperType, pixelMap);

    std::string url(reinterpret_cast<const char*>(data), size);
    WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(url, wallpaperType);
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::WallpaperManagerFuzzTest(data, size);
    return 0;
}