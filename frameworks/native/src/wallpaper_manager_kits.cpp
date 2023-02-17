/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wallpaper_manager_kits.h"

#include "wallpaper_manager.h"
namespace OHOS {
namespace WallpaperMgrService {
WallpaperManagerkits &WallpaperManagerkits::GetInstance()
{
    return DelayedRefSingleton<WallpaperManager>::GetInstance();
}
} // namespace WallpaperMgrService
} // namespace OHOS