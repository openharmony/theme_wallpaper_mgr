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

#include "wallpaper_extension_ability.h"

#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "js_wallpaper_extension_ability.h"
#include "runtime.h"
#include "wallpaper_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
WallpaperExtensionAbility *WallpaperExtensionAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    HILOG_INFO("jws WallpaperExtensionAbility::Create runtime");
    if (!runtime) {
        return new WallpaperExtensionAbility();
    }
    HILOG_INFO("jws WallpaperExtensionAbility::Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsWallpaperExtensionAbility::Create(runtime);

        default:
            return new WallpaperExtensionAbility();
    }
}

void WallpaperExtensionAbility::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("jws WallpaperExtensionAbility begin init context");
    ExtensionBase<WallpaperExtensionContext>::Init(record, application, handler, token);
}

std::shared_ptr<WallpaperExtensionContext> WallpaperExtensionAbility::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_INFO("jws WallpaperExtensionAbility begin init context");
    std::shared_ptr<WallpaperExtensionContext> context =
        ExtensionBase<WallpaperExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        HILOG_ERROR("WallpaperExtensionAbility::CreateAndInitContext context is nullptr");
        return context;
    }
    return context;
}
} // namespace AbilityRuntime
} // namespace OHOS