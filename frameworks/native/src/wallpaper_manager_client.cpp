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

#include "hilog_wrapper.h"
#include "wallpaper_manager.h"
#include "wallpaper_manager_client.h"

namespace OHOS {
namespace WallpaperMgrService {
WallpaperManagerClient::WallpaperManagerClient()
{
}
WallpaperManagerClient::~WallpaperManagerClient()
{
}
WallpaperManagerClient &WallpaperManagerClient::GetInstance()
{
    static WallpaperManagerClient client;
    return client;
}
bool WallpaperManagerClient::IsDefaultWallpaperResource(int32_t userId, int32_t wallpaperType)
{
    HILOG_DEBUG("WallpaperManagerClient IsDefaultResource.");
    return WallpaperManager::GetInstance().IsDefaultWallpaperResource(userId, wallpaperType);
}

int32_t WallpaperManagerClient::SetAllWallpapers(std::vector<WallpaperInfo> allWallpaperInfos, int32_t wallpaperType)
{
    HILOG_DEBUG("WallpaperManagerClient SetAllWallpapers.");
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().SetAllWallpapers(allWallpaperInfos, wallpaperType);
    return static_cast<int32_t>(wallpaperErrorCode);
}

} // namespace WallpaperMgrService
} // namespace OHOS