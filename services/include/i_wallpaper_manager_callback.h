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

#ifndef SERVICES_INCLUDE_WALLPAPER_SERVICE_CALLBACK_INTERFACE_H
#define SERVICES_INCLUDE_WALLPAPER_SERVICE_CALLBACK_INTERFACE_H
#include <vector>
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperMgrService {
    using namespace std;
    class IWallpaperManagerCallback {
    public:
        virtual void OnWallpaperChanged();
        /**
       * Called when wallpaper colors change
      */
    virtual void OnWallpaperColorsChanged(vector<uint64_t> colors, int which, int userId);
    };
}
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_CALLBACK_INTERFACE_H