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
#ifndef WALLPAPER_MGR_CJSON_MOCK_H
#define WALLPAPER_MGR_CJSON_MOCK_H

#include <gmock/gmock.h>

#include "cJSON.h"

namespace OHOS {
class WallpaperMgrCjson {
public:
    WallpaperMgrCjson(){};
    virtual ~WallpaperMgrCjson(){};
    virtual cJSON *cJSON_AddNumberToObject(cJSON *const object, const char *const name, const double number) = 0;
    virtual char *cJSON_Print(const cJSON *item) = 0;
    virtual cJSON *cJSON_Parse(const char *item) = 0;
    virtual cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *const object, const char *const string) = 0;
    static inline std::shared_ptr<WallpaperMgrCjson> wallpaperMgrCjson = nullptr;
};

class WallpaperMgrCjsonMock : public WallpaperMgrCjson {
public:
    WallpaperMgrCjsonMock();
    ~WallpaperMgrCjsonMock() override;

    MOCK_METHOD(cJSON *, cJSON_AddNumberToObject, (cJSON *, const char *, const double), (override));
    MOCK_METHOD(char *, cJSON_Print, (const cJSON *), (override));
    MOCK_METHOD(cJSON *, cJSON_Parse, (const char *), (override));
    MOCK_METHOD(cJSON *, cJSON_GetObjectItemCaseSensitive, (const cJSON *, const char *), (override));
};
} // namespace OHOS

#endif // WALLPAPER_MGR_CJSON_MOCK_H
