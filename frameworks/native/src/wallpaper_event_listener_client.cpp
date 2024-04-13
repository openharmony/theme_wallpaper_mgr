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

#define LOG_TAG "WallpaperEventListenerClient"

#include "wallpaper_event_listener_client.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace WallpaperMgrService {
WallpaperEventListenerClient::WallpaperEventListenerClient(
    std::shared_ptr<WallpaperMgrService::WallpaperEventListener> wallpaperEventListener)
    : wallpaperEventListener_(wallpaperEventListener)
{
}

WallpaperEventListenerClient::~WallpaperEventListenerClient()
{
}

void WallpaperEventListenerClient::OnColorsChange(const std::vector<uint64_t> &color, int32_t wallpaperType)
{
    HILOG_INFO("OnColorsChange start");
    if (wallpaperEventListener_ != nullptr) {
        wallpaperEventListener_->OnColorsChange(color, wallpaperType);
    }
}

void WallpaperEventListenerClient::OnWallpaperChange(WallpaperType wallpaperType, WallpaperResourceType resourceType,
    const std::string &uri)
{
    if (wallpaperEventListener_ != nullptr) {
        wallpaperEventListener_->OnWallpaperChange(wallpaperType, resourceType, uri);
    }
}

const std::shared_ptr<WallpaperEventListener> WallpaperEventListenerClient::GetEventListener() const
{
    return wallpaperEventListener_;
}
} // namespace WallpaperMgrService
} // namespace OHOS
