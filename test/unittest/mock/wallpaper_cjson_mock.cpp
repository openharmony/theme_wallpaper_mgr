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

#include "wallpaper_cjson_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
WallpaperMgrCjsonMock::WallpaperMgrCjsonMock()
{
}

WallpaperMgrCjsonMock::~WallpaperMgrCjsonMock()
{
}

extern "C" {
cJSON *cJSON_AddNumberToObject(cJSON *const object, const char *const name, const double number)
{
    if (WallpaperMgrCjson::wallpaperMgrCjson == nullptr) {
        return nullptr;
    }
    return WallpaperMgrCjson::wallpaperMgrCjson->cJSON_AddNumberToObject(object, name, number);
}

char *cJSON_Print(const cJSON *item)
{
    if (WallpaperMgrCjson::wallpaperMgrCjson == nullptr) {
        return nullptr;
    }
    return WallpaperMgrCjson::wallpaperMgrCjson->cJSON_Print(item);
}

cJSON *cJSON_Parse(const char *item)
{
    if (WallpaperMgrCjson::wallpaperMgrCjson == nullptr) {
        return nullptr;
    }
    return WallpaperMgrCjson::wallpaperMgrCjson->cJSON_Parse(item);
}

cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *const object, const char *const string)
{
    if (WallpaperMgrCjson::wallpaperMgrCjson == nullptr) {
        return nullptr;
    }
    return WallpaperMgrCjson::wallpaperMgrCjson->cJSON_GetObjectItemCaseSensitive(object, string);
}
}
} // namespace OHOS