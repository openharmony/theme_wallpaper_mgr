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
#ifndef SERVICES_INCLUDE_WALLPAPER_CALLBACK_INTERFACE_H
#define SERVICES_INCLUDE_WALLPAPER_CALLBACK_INTERFACE_H

#include "iremote_broker.h"

namespace OHOS {
namespace WallpaperMgrService {
class IWallpaperCallback : public IRemoteBroker {
public:
    enum Message { ONCALL = 0 };
    virtual int32_t OnCall(const int32_t num) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.Wallpaper.IWallpaperCallback");
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif