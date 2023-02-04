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

#include "wallpaper_data.h"

#include "file_ex.h"

namespace OHOS {
namespace WallpaperMgrService {
WallpaperData::WallpaperData(int32_t userId, std::string wallpaperFileName, std::string cropFileName)
    : userId_(userId), wallpaperFile_(wallpaperFileName), cropFile_(cropFileName), wallpaperId_(-1), allowBackup(false)
{
}
WallpaperData::WallpaperData() : userId_(0), wallpaperId_(-1), allowBackup(false)
{
}
// Called during initialization of a given user's wallpaper bookkeeping
bool WallpaperData::cropExists()
{
    return OHOS::FileExists(cropFile_);
}

bool WallpaperData::sourceExists()
{
    return OHOS::FileExists(wallpaperFile_);
}
} // namespace WallpaperMgrService
} // namespace OHOS
