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

#define LOG_TAG "WallpaperColorChangeListenerProxy"

#include "iwallpaper_color_change_listener.h"

#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace WallpaperMgrService {
constexpr int ONCOLORSCHANGE = 0;
using namespace std::chrono;

WallpaperColorChangeListenerProxy::WallpaperColorChangeListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IWallpaperColorChangeListener>(impl)
{
}

void WallpaperColorChangeListenerProxy::onColorsChange(std::vector<RgbaColor> color, int wallpaperType)
{
    HILOG_DEBUG("WallpaperColorChangeListenerProxy::onColorsChange Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(WallpaperColorChangeListenerProxy::GetDescriptor())) {
        HILOG_ERROR("write descriptor failed");
        return;
    }

    unsigned int size = color.size();
    if (!data.WriteInt32(size)) {
        HILOG_ERROR("write color size failed");
        return;
    }
    for (unsigned int i = 0; i < size; ++i) {
        if (!(data.WriteInt32(color[i].red) && data.WriteInt32(color[i].green) && data.WriteInt32(color[i].blue) &&
                data.WriteInt32(color[i].alpha))) {
            HILOG_ERROR("write color failed");
            return;
        }
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR("write wallpaperType failed");
        return;
    }

    int error = Remote()->SendRequest(ONCOLORSCHANGE, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("SendRequest failed, error %d", error);
    }
    HILOG_DEBUG("WallpaperColorChangeListenerProxy::onColorsChange End");
}

int32_t WallpaperColorChangeListenerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    HILOG_DEBUG("WallpaperColorChangeListenerStub::OnRemoteRequest Start");
    HILOG_DEBUG("%d", code);
    std::u16string descriptor = WallpaperColorChangeListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote");
        return -1;
    }
    switch (code) {
        case ONCOLORSCHANGE: {
            std::vector<RgbaColor> color;
            unsigned int size = data.ReadInt32();
            for (unsigned int i = 0; i < size; ++i) {
                RgbaColor colorInfo;
                colorInfo.red = data.ReadInt32();
                colorInfo.blue = data.ReadInt32();
                colorInfo.green = data.ReadInt32();
                colorInfo.alpha = data.ReadInt32();
                color.emplace_back(colorInfo);
            }
            int wallpaperType = data.ReadInt32();
            onColorsChange(color, wallpaperType);
            HILOG_DEBUG("WallpaperColorChangeListenerStub::OnRemoteRequest End");
            return 0;
        }
        default:
            HILOG_DEBUG("code error, WallpaperColorChangeListenerStub::OnRemoteRequest End");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}
} // namespace WallpaperMgrService
} // namespace OHOS
