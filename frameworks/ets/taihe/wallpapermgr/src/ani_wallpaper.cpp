/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#define LOG_TAG "AniObserverUtils"
#include "ani_wallpaper.h"
#include <algorithm>
#include <endian.h>

#include "hilog_wrapper.h"

namespace ani_wallpaper {

std::shared_ptr<OHOS::AppExecFwk::EventHandler> JsBaseObserver::mainHandler_;

JsBaseObserver::JsBaseObserver(VarCallbackType cb, ani_ref jsCallbackRef)
    : jsCallback_(cb), jsCallbackRef_(jsCallbackRef)
{
}

bool JsBaseObserver::SendEventToMainThread(const std::function<void()> func)
{
    if (func == nullptr) {
        return false;
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            HILOG_ERROR("GetMainEventRunner failed");
            return false;
        }
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    mainHandler_->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
    return true;
}

void JsBaseObserver::Release()
{
    HILOG_INFO("WallpaperObserver::Release");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    taihe::env_guard guard;
    if (auto *env = guard.get_env()) {
        env->GlobalReference_Delete(jsCallbackRef_);
        jsCallbackRef_ = nullptr;
    }
}

WallpaperObserver::WallpaperObserver(VarCallbackType cb, ani_ref jsCallbackRef)
    : JsBaseObserver(cb, jsCallbackRef)
{
    HILOG_INFO("WallpaperObserver");
}

WallpaperObserver::~WallpaperObserver()
{
    HILOG_INFO("~WallpaperObserver");
}

void WallpaperObserver::OnWallpaperChange(WallpaperType wallpaperType,
    WallpaperResourceType resourceType, const std::string &uri)
{
    auto sharedThis = shared_from_this();
    SendEventToMainThread([wallpaperType, resourceType, uri, sharedThis] {
        sharedThis->OnWallpaperChangeInMainThread(wallpaperType, resourceType, uri);
    });
}

void WallpaperObserver::OnWallpaperChangeInMainThread(WallpaperType wallpaperType,
    WallpaperResourceType resourceType, const std::string &uri)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (jsCallbackRef_ == nullptr) {
        return;
    }
    auto &jsFunc = std::get<ani_wallpaper::JsWallpaperCallbackType>(jsCallback_);
    ::ohos::wallpaper::WallpaperType type = ::ohos::wallpaper::WallpaperType::from_value(wallpaperType);
    ::ohos::wallpaper::WallpaperResourceType ResourceType =
        ::ohos::wallpaper::WallpaperResourceType::from_value(resourceType);
    auto uris = taihe::optional<::taihe::string>::make(uri);
    jsFunc(type, ResourceType, uris);
}
} // } // ani_wallpaper