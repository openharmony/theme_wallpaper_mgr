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

#include "wallpaper_extension_module_loader.h"
#include "wallpaper_extension.h"

namespace OHOS::AbilityRuntime {
WallpaperExtensionModuleLoader::WallpaperExtensionModuleLoader() = default;
WallpaperExtensionModuleLoader::~WallpaperExtensionModuleLoader() = default;

Extension *WallpaperExtensionModuleLoader::Create(const std::unique_ptr<Runtime>& runtime) const
{
    HILOG_INFO("jws WallpaperExtension::Create runtime");
    return WallpaperExtension::Create(runtime);
}

std::map<std::string, std::string> WallpaperExtensionModuleLoader::GetParams()
{
    std::map<std::string, std::string> params;
    params.insert(std::pair<std::string, std::string>("type", "8"));
    params.insert(std::pair<std::string, std::string>("name", "WallpaperExtension"));
    return params;
}

extern "C" __attribute__((visibility("default"))) void* OHOS_EXTENSION_GetExtensionModule()
{
    return &WallpaperExtensionModuleLoader::GetInstance();
}
} // namespace OHOS::AbilityRuntime