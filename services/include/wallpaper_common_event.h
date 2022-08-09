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
#ifndef WALLPAPER_SERVICES_WALLPAPER_COMMON_EVENT_H
#define WALLPAPER_SERVICES_WALLPAPER_COMMON_EVENT_H

#include <string>
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "common_event_manager.h"

namespace OHOS {
namespace WallpaperMgrService {
using CommonEventSubscriber = OHOS::EventFwk::CommonEventSubscriber;
using CommonEventData = OHOS::EventFwk::CommonEventData;
using CommonEventSubscribeInfo = OHOS::EventFwk::CommonEventSubscribeInfo;
static const std::string WALLPAPER_LOCK_SETTING_SUCCESS_EVENT = "com.ohos.wallpaperlocksettingsuccess";
static const std::string WALLPAPER_SYSTEM_SETTING_SUCCESS_EVENT = "com.ohos.wallpapersystemsettingsuccess";
static const int WALLPAPER_LOCK_SETTING_SUCCESS_CODE = 11000;
static const int WALLPAPER_SYSTEM_SETTING_SUCCESS_CODE = 21000;

class WallpaperCommonEvent : public OHOS::EventFwk::CommonEventSubscriber {
public:
    WallpaperCommonEvent(const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo)
    {}
    ~WallpaperCommonEvent() = default;
    static std::shared_ptr<WallpaperCommonEvent> subscriber;
    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
    static bool PublishEvent(const OHOS::AAFwk::Want &want, int eventCode, const std::string &eventData);
    static void UnregisterSubscriber(std::shared_ptr<OHOS::EventFwk::CommonEventSubscriber> subscriber);
    static bool RegisterSubscriber();
    static void SendWallpaperLockSettingMessage();
    static void SendWallpaperSystemSettingMessage();
private:
    std::function<void(const EventFwk::CommonEventData &)> callback_;
};
}
}

#endif