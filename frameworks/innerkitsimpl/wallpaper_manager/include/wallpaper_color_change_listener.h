/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef WALLPAPER_COLOR_CHANGE_LISTENER_H
#define WALLPAPER_COLOR_CHANGE_LISTENER_H

#include <memory>
#include <vector>

#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperColorChangeListener {
public:
    WallpaperColorChangeListener() = default;

    virtual ~WallpaperColorChangeListener()
    {
    }

    virtual void OnColorsChange(const std::vector<uint32_t> &color, int wallpaperType)
    {
    }
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // WALLPAPER_COLOR_CHANGE_LISTENER_H
