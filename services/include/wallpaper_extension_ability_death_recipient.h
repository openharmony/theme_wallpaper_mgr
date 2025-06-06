/*
* Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef WALLPAPER_EXTENSION_ABILITY_DEATH_RECIPIENT_H
#define WALLPAPER_EXTENSION_ABILITY_DEATH_RECIPIENT_H
#include "iremote_object.h"
#include "wallpaper_service.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperExtensionAbilityDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit WallpaperExtensionAbilityDeathRecipient(WallpaperService &wallpaperService)
        : wallpaperService_(wallpaperService)
    {
    }
    virtual ~WallpaperExtensionAbilityDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    WallpaperService &wallpaperService_;
};

} // namespace WallpaperMgrService
} // namespace OHOS

#endif // WALLPAPER_EXTENSION_ABILITY_DEATH_RECIPIENT_H
