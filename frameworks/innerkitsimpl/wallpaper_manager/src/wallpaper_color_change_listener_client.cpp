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

#define LOG_TAG "WallpaperColorChangeListenerClient"

#include "hilog_wrapper.h"
#include "wallpaper_color_change_listener_client.h"

namespace OHOS {
namespace WallpaperMgrService {
WallpaperColorChangeListenerClient::WallpaperColorChangeListenerClient(
    std::shared_ptr<WallpaperMgrService::WallpaperColorChangeListener> wallpaperColorChangerListener)
    : wallpaperColorChangerListener_(wallpaperColorChangerListener)
{
    HILOG_INFO("start");
}

WallpaperColorChangeListenerClient::~WallpaperColorChangeListenerClient()
{
    HILOG_INFO("start");
}

void WallpaperColorChangeListenerClient::OnColorsChange(const std::vector<uint64_t> &color, int wallpaperType)
{
    HILOG_INFO("start");
    if (wallpaperColorChangerListener_ != nullptr) {
        wallpaperColorChangerListener_->OnColorsChange(color, wallpaperType);
    }
}

const std::shared_ptr<WallpaperColorChangeListener> WallpaperColorChangeListenerClient::GetColorChangeListener() const
{
    return wallpaperColorChangerListener_;
}
}  // namespace WallpaperMgrService
}  // namespace OHOS
