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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_JS_WALLPAPER_EXTENSION_ABILITY_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_JS_WALLPAPER_EXTENSION_ABILITY_H

#include <mutex>

#include "wallpaper_extension_ability.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class WallpaperExtensionAbility;
class JsRuntime;

/**
 * @brief Basic wallpaper components.
 */
class JsWallpaperExtensionAbility : public WallpaperExtensionAbility,
                                    public std::enable_shared_from_this<JsWallpaperExtensionAbility> {
public:
    JsWallpaperExtensionAbility(JsRuntime &jsRuntime);
    virtual ~JsWallpaperExtensionAbility() override;
    static JsWallpaperExtensionAbility *jsWallpaperExtensionAbility;
    static std::mutex mtx;

    /**
     * @brief Create JsWallpaperExtensionAbility.
     *
     * @param runtime The runtime.
     * @return The JsWallpaperExtensionAbility instance.
     */
    static JsWallpaperExtensionAbility *Create(const std::unique_ptr<Runtime> &runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Wallpaper extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Wallpaper extension.
     * @return Returns a pointer to the <b>sid</b> of the connected Wallpaper extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this Wallpaper extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called back when Wallpaper is started.
     * This method can be called only by Wallpaper. You can use the StartAbility(ohos.aafwk.content.Want) method
     * to start Wallpaper. Then the system calls back the current method to use the transferred want parameter
     * to execute its own logic.
     *
     * @param want Indicates the want of Wallpaper to start.
     * @param restart Indicates the startup mode. The value true indicates that Wallpaper is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Wallpaper extension has been started. The startId is incremented
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int32_t startId) override;

    /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop() override;

private:
    void RegisterWallpaperCallback();

private:
    napi_value CallObjectMethod(const std::string &name, napi_value const *argv = nullptr, size_t argc = 0);

    void GetSrcPath(std::string &srcPath);

    JsRuntime &jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITYRUNTIME_OHOS_JS_WALLPAPER_EXTENSION_ABILITY_H