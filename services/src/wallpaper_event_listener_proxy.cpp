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

#define LOG_TAG "WallpaperEventListenerProxy"

#include "wallpaper_event_listener_proxy.h"

#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace std::chrono;

void WallpaperEventListenerProxy::OnColorsChange(const std::vector<uint64_t> &color, int32_t wallpaperType)
{
    HILOG_DEBUG("WallpaperEventListenerProxy::OnColorsChange Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WallpaperEventListenerProxy::GetDescriptor())) {
        HILOG_ERROR("write descriptor failed");
        return;
    }

    if (!data.WriteUInt64Vector(color)) {
        HILOG_ERROR("write color failed");
        return;
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR("write wallpaperType failed");
        return;
    }

    int32_t error = Remote()->SendRequest(ON_COLORS_CHANGE, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("SendRequest failed, error %{public}d", error);
    }
}

void WallpaperEventListenerProxy::OnWallpaperChange(WallpaperType wallpaperType, WallpaperResourceType resourceType,
    const std::string &uri)
{
    HILOG_DEBUG("WallpaperEventListenerProxy::OnWallpaperChange Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WallpaperEventListenerProxy::GetDescriptor())) {
        HILOG_ERROR("write descriptor failed");
        return;
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR("write wallpaperType failed");
        return;
    }
    if (!data.WriteInt32(resourceType)) {
        HILOG_ERROR("write resourceType failed");
        return;
    }
    if (!data.WriteString(uri)) {
        HILOG_ERROR("write uri failed");
        return;
    }

    int32_t error = Remote()->SendRequest(ON_WALLPAPER_CHANGE, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("SendRequest failed, error %{public}d", error);
    }
}

} // namespace WallpaperMgrService
} // namespace OHOS
