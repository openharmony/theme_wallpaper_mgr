/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ABILITY_RUNTIME_JS_WALLPAPER_EXTENSION_CONTEXT_H
#define ABILITY_RUNTIME_JS_WALLPAPER_EXTENSION_CONTEXT_H

#include <memory>
#include <node_api.h>

#include "ability_connect_callback.h"
#include "event_handler.h"
#include "napi/native_api.h"
#include "wallpaper_extension_context.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsWallpaperExtensionContext(napi_env env, std::shared_ptr<WallpaperExtensionContext> context);

class JSWallpaperExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSWallpaperExtensionConnection(napi_env env);
    ~JSWallpaperExtensionConnection();
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;
    void HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode);
    void SetJsConnectionObject(napi_value jsConnectionObject);
    void CallJsFailed(int32_t errorCode);

private:
    napi_env env_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
};

struct ConnecttionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnecttionKey &key1, const ConnecttionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_RUNTIME_JS_WALLPAPER_EXTENSION_CONTEXT_H
