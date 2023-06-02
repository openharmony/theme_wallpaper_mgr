/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef SERVICES_INCLUDE_WALLPAPER_DATA_H
#define SERVICES_INCLUDE_WALLPAPER_DATA_H
#include <string>
#include <vector>

#include "component_name.h"
#include "i_wallpaper_manager_callback.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {
struct WallpaperData {
    int32_t userId;
    std::string wallpaperFile;     // source image
    std::string liveWallpaperFile; // source video
    std::string customPackageUri;
    std::string cropFile; // eventual destination
    std::string name;
    int32_t wallpaperId;
    bool allowBackup;
    WallpaperResourceType resourceType;
    /**
     * The component name of the currently set live wallpaper.
     */
    ComponentName wallpaperComponent;
    WallpaperData(int32_t userId, std::string fileName);
    WallpaperData();
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_DATA_H
