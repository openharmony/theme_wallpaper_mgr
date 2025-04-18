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

#ifndef WALLPAPER_WALLPAPEREVENTMANAGER_H
#define WALLPAPER_WALLPAPEREVENTMANAGER_H
#include <string>

#include "common_event_manager.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {

class WallpaperCommonEventManager {
public:
    void SendWallpaperLockSettingMessage(WallpaperResourceType resType);
    void SendWallpaperSystemSettingMessage(WallpaperResourceType resType);

private:
    bool PublishEvent(const OHOS::AAFwk::Want &want, int32_t eventCode, const std::string &eventData);
    ;
};
} // namespace WallpaperMgrService
} // namespace OHOS

#endif
