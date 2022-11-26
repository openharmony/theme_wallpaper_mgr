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

#ifndef I_WALLPAPER_COLOR_CHANGE_LISTENER_H
#define I_WALLPAPER_COLOR_CHANGE_LISTENER_H

#include <cstdint>
#include <vector>
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "refbase.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {
class IWallpaperColorChangeListener : public IRemoteBroker {
public:
    enum Message { ONCOLORSCHANGE = 0 };
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.WallpaperMgrService.IWallpaperColorChangeListener");
    virtual void OnColorsChange(const std::vector<uint64_t> &color, int wallpaperType) = 0;
};
} // namespace WallpaperMgrService
} // namespace OHOS

#endif // I_WALLPAPER_COLOR_CHANGE_LISTENER_H
