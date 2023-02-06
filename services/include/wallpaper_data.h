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
#include "wallpaper_manager_common_info.h"
#include "i_wallpaper_manager_callback.h"
#include "component_name.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperData {
public:
    int userId_;
    std::string wallpaperFile_; // source image
    std::string cropFile_;      // eventual destination
    std::string name_;
    int wallpaperId_;
    bool allowBackup;
    /**
     * The component name of the currently set live wallpaper.
     */
    ComponentName wallpaperComponent;
    
public:
    WallpaperData(int userId,  std::string wallpaperFileName, std::string cropFileName);
    WallpaperData();
    // Called during initialization of a given user's wallpaper bookkeeping
    bool cropExists();
    bool sourceExists();

private:
};
}
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_DATA_H
