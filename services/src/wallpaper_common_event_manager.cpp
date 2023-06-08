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

#include "wallpaper_common_event_manager.h"

#include "hilog_wrapper.h"
namespace OHOS {
namespace WallpaperMgrService {
constexpr const char *LOCKSCREEN_WALLPAPER_SETTING_SUCCESS_EVENT = "com.ohos.wallpaperlocksettingsuccess";
constexpr const char *SYSTEM_WALLPAPER_SETTING_SUCCESS_EVENT = "com.ohos.wallpapersystemsettingsuccess";
constexpr int32_t LOCKSCREEN_WALLPAPER_SETTING_SUCCESS_CODE = 11000;
constexpr int32_t SYSTEM_WALLPAPER_SETTING_SUCCESS_CODE = 21000;

bool WallpaperCommonEventManager::PublishEvent(const OHOS::AAFwk::Want &want, int32_t eventCode,
    const std::string &eventData)
{
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(eventData);
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    HILOG_INFO("PublishEvent end publishResult = %{public}d", publishResult);
    return publishResult;
}

void WallpaperCommonEventManager::SendWallpaperLockSettingMessage(WallpaperResourceType resType)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = LOCKSCREEN_WALLPAPER_SETTING_SUCCESS_CODE;
    want.SetParam("WallpaperLockSettingMessage", true);
    want.SetParam("WallpaperLockScreenResourceType", static_cast<int>(resType));
    want.SetAction(LOCKSCREEN_WALLPAPER_SETTING_SUCCESS_EVENT);
    std::string eventData("WallpaperLockSettingMessage");
    PublishEvent(want, eventCode, eventData);
}

void WallpaperCommonEventManager::SendWallpaperSystemSettingMessage(WallpaperResourceType resType)
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = SYSTEM_WALLPAPER_SETTING_SUCCESS_CODE;
    want.SetParam("WallpaperSystemSettingMessage", true);
    want.SetParam("WallpaperSystemResourceType", static_cast<int>(resType));
    want.SetAction(SYSTEM_WALLPAPER_SETTING_SUCCESS_EVENT);
    std::string eventData("WallpaperSystemSettingMessage");
    PublishEvent(want, eventCode, eventData);
}
} // namespace WallpaperMgrService
} // namespace OHOS