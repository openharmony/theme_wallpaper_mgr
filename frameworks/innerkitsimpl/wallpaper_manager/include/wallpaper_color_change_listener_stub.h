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

#ifndef WALLPAPER_COLOR_CHANGE_LISTENER_STUB_H
#define WALLPAPER_COLOR_CHANGE_LISTENER_STUB_H

#include <cstdint>
#include <vector>

#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "iwallpaper_color_change_listener.h"
#include "refbase.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {

class WallpaperColorChangeListenerStub : public IRemoteStub<IWallpaperColorChangeListener> {
public:
    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;
};

} // namespace WallpaperMgrService
} // namespace OHOS

#endif // WALLPAPER_COLOR_CHANGE_LISTENER_H