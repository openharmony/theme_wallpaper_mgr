/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "wallpaper_common_event.h"

#include "hilog_wrapper.h"
namespace OHOS {
namespace WallpaperMgrService {
constexpr const char *WALLPAPER_LOCK_SETTING_SUCCESS_EVENT = "com.ohos.wallpaperlocksettingsuccess";
constexpr const char *WALLPAPER_SYSTEM_SETTING_SUCCESS_EVENT = "com.ohos.wallpapersystemsettingsuccess";
constexpr int32_t WALLPAPER_LOCK_SETTING_SUCCESS_CODE = 11000;
constexpr int32_t WALLPAPER_SYSTEM_SETTING_SUCCESS_CODE = 21000;

void WallpaperCommonEvent::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    HILOG_INFO("WallpaperCommonEvent::OnReceiveEvent");
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED) {
        WallpaperService::OnBootPhase();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED) {
        HILOG_INFO("OnInitUser userId = %{public}d", data.GetCode());
        wallpaperService_.OnInitUser(data.GetCode());
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        HILOG_INFO("OnRemovedUser userId = %{public}d", data.GetCode());
        wallpaperService_.OnRemovedUser(data.GetCode());
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        HILOG_INFO("OnSwitchedUser userId = %{public}d", data.GetCode());
        wallpaperService_.OnSwitchedUser(data.GetCode());
    }
}

bool WallpaperCommonEvent::PublishEvent(const OHOS::AAFwk::Want &want, int32_t eventCode, const std::string &eventData)
{
    OHOS::EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(eventData);
    OHOS::EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = OHOS::EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    HILOG_INFO("PublishEvent end publishResult = %{public}d", publishResult);
    return publishResult;
}

void WallpaperCommonEvent::UnregisterSubscriber(std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> subscriber)
{
    if (subscriber != nullptr) {
        bool subscribeResult = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber);
        subscriber = nullptr;
        HILOG_INFO("WallpaperCommonEvent::UnregisterSubscriber end###subscribeResult = %{public}d", subscribeResult);
    }
}

bool WallpaperCommonEvent::RegisterSubscriber()
{
    HILOG_INFO("WallpaperCommonEvent::RegisterSubscriber");
    return OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(shared_from_this());
}

void WallpaperCommonEvent::SendWallpaperLockSettingMessage()
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = WALLPAPER_LOCK_SETTING_SUCCESS_CODE;
    want.SetParam("WallpaperLockSettingMessage", true);
    want.SetAction(WALLPAPER_LOCK_SETTING_SUCCESS_EVENT);
    std::string eventData("WallpaperLockSettingMessage");
    PublishEvent(want, eventCode, eventData);
}

void WallpaperCommonEvent::SendWallpaperSystemSettingMessage()
{
    OHOS::AAFwk::Want want;
    int32_t eventCode = WALLPAPER_SYSTEM_SETTING_SUCCESS_CODE;
    want.SetParam("WallpaperSystemSettingMessage", true);
    want.SetAction(WALLPAPER_SYSTEM_SETTING_SUCCESS_EVENT);
    std::string eventData("WallpaperSystemSettingMessage");
    PublishEvent(want, eventCode, eventData);
}

CommonEventSubscribeInfo WallpaperCommonEvent::CreateSubscriberInfo() {
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    return CommonEventSubscribeInfo(matchingSkills);
}
} // namespace WallpaperMgrService
} // namespace OHOS