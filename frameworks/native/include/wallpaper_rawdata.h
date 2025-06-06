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

#ifndef SERVICES_INCLUDE_WALLPAPER_SERVICE_WALLPAPER_RAWDATA_H
#define SERVICES_INCLUDE_WALLPAPER_SERVICE_WALLPAPER_RAWDATA_H
#include <cstdint>
#include <string>
namespace OHOS::WallpaperMgrService {
class WallpaperRawData {
public:
    WallpaperRawData();
    ~WallpaperRawData() = default;

    uint32_t size;
    const void *data;
    int32_t RawDataCpy(const void *readData);
};
} // namespace OHOS::WallpaperMgrService

#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_WALLPAPER_RAWDATA_H