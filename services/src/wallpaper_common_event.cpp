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
#include "wallpaper_service.h"
namespace OHOS {
namespace WallpaperMgrService {
std::shared_ptr<WallpaperCommonEvent> WallpaperCommonEvent::subscriber = nullptr;

void WallpaperCommonEvent::OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data)
{
    HILOG_INFO("WallpaperCommonEvent::OnReceiveEvent");
    WallpaperService::OnBootPhase();
}

bool WallpaperCommonEvent::PublishEvent(const OHOS::AAFwk::Want &want, int eventCode, const std::string &eventData)
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
    OHOS::EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber = std::make_shared<WallpaperCommonEvent>(subscriberInfo);
    return OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
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
}
}