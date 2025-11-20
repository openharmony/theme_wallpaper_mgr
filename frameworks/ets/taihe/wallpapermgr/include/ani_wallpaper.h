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

#ifndef OHOS_ANI_OBSERVER_UTILS_H
#define OHOS_ANI_OBSERVER_UTILS_H
#include "taihe/runtime.hpp"
#include "ohos.wallpaper.proj.hpp"
#include "ohos.wallpaper.impl.hpp"
#include <string>
#include <optional>
#include <variant>

#include "event_handler.h"
#include "event_runner.h"
#include "wallpaper_event_listener.h"

namespace ani_wallpaper {

using namespace OHOS;

class GlobalRefGuard {
    ani_env *env_ = nullptr;
    ani_ref ref_ = nullptr;

public:
    GlobalRefGuard(ani_env *env, ani_object obj) : env_(env)
    {
        if (!env_ || !obj)
            return;
        if (ANI_OK != env_->GlobalReference_Create(obj, &ref_)) {
            ref_ = nullptr;
        }
    }
    explicit operator bool() const
    {
        return ref_ != nullptr;
    }
    ani_ref get() const
    {
        return ref_;
    }
    ~GlobalRefGuard()
    {
        if (env_ && ref_) {
            env_->GlobalReference_Delete(ref_);
        }
    }

    GlobalRefGuard(const GlobalRefGuard &) = delete;
    GlobalRefGuard &operator=(const GlobalRefGuard &) = delete;
};

using JsWallpaperCallbackType = ::taihe::callback<void(::ohos::wallpaper::WallpaperType allpaperType,
    ::ohos::wallpaper::WallpaperResourceType resourceType, ::taihe::optional_view<::taihe::string> uri)>;

using VarCallbackType = std::variant<JsWallpaperCallbackType>;

class JsBaseObserver {
public:
    JsBaseObserver(VarCallbackType cb, ani_ref jsCallbackRef);
    virtual ~JsBaseObserver() {}
    bool SendEventToMainThread(const std::function<void()> func);
    void Release();

    std::recursive_mutex mutex_;
    VarCallbackType jsCallback_;
    ani_ref jsCallbackRef_ = nullptr;
    static std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_;
};

template<class DerivedObserverClass>
ani_ref CreateCallbackRefIfNotDuplicate(std::vector<std::shared_ptr<DerivedObserverClass>> const& cbVec,
    ani_object callbackObj)
{
    ani_ref callbackRef = nullptr;
    ani_env *env = taihe::get_env();
    if (env == nullptr || ANI_OK != env->GlobalReference_Create(callbackObj, &callbackRef)) {
        return nullptr;
    }
    bool isDuplicate = std::any_of(cbVec.begin(), cbVec.end(),
        [env, callbackRef](std::shared_ptr<DerivedObserverClass> const& obj) {
            ani_boolean isEqual = false;
            return (ANI_OK == env->Reference_StrictEquals(callbackRef, obj->jsCallbackRef_, &isEqual)) && isEqual;
        });
    if (isDuplicate) {
        env->GlobalReference_Delete(callbackRef);
        return nullptr;
    }
    return callbackRef;
}

class WallpaperObserver : public JsBaseObserver,
    public WallpaperMgrService::WallpaperEventListener,
    public std::enable_shared_from_this<WallpaperObserver> {
public:
    WallpaperObserver(VarCallbackType cb, ani_ref jsCallbackRef);
    ~WallpaperObserver();
    void OnWallpaperChange(
        WallpaperType wallpaperType, WallpaperResourceType resourceType, const std::string &uri) override;
    void OnWallpaperChangeInMainThread(WallpaperType wallpaperType,
        WallpaperResourceType resourceType, const std::string &uri);
};
} // ani_wallpaper
#endif