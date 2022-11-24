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
#include "wallpaper_color_change_listener_stub.h"

#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace std::chrono;
int32_t WallpaperColorChangeListenerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    HILOG_DEBUG("WallpaperColorChangeListenerStub::OnRemoteRequest Start");
    std::u16string descriptor = WallpaperColorChangeListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote");
        return -1;
    }
    switch (code) {
        case ONCOLORSCHANGE: {
            std::vector<uint64_t> color;
            if (!data.ReadUInt64Vector(&color)) {
                HILOG_ERROR("ONCOLORSCHANGE ReadUInt64Vector error");
                return -1;
            }
            int wallpaperType = data.ReadInt32();
            OnColorsChange(color, wallpaperType);
            HILOG_DEBUG("WallpaperColorChangeListenerStub::OnRemoteRequest End");
            return 0;
        }
        default: {
            HILOG_ERROR("code error, WallpaperColorChangeListenerStub::OnRemoteRequest End");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}
} // namespace WallpaperMgrService
} // namespace OHOS
