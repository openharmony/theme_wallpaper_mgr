/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef INNERKITS_WALLPAPER_MANAGER_CLIENT_H
#define INNERKITS_WALLPAPER_MANAGER_CLIENT_H

#include <cstdint>
#include <vector>

#include "nocopyable.h"
#include "wallpaper_common.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperManagerClient {
    DISALLOW_COPY_AND_MOVE(WallpaperManagerClient);
    WallpaperManagerClient();
    ~WallpaperManagerClient();

public:
    static WallpaperManagerClient &GetInstance();
    bool IsDefaultWallpaperResource(int32_t userId, int32_t wallpaperType);
    int32_t SetAllWallpapers(std::vector<WallpaperInfo> allWallpaperInfos, int32_t wallpaperType);
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif