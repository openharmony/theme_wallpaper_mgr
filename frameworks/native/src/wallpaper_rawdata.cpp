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

#include <securec.h>

#include <iostream>

#include "hilog_wrapper.h"
#include "wallpaper_rawdata.h"

namespace OHOS::WallpaperMgrService {
WallpaperRawData::WallpaperRawData()

{
}
int32_t WallpaperRawData::RawDataCpy(const void *readData)
{
    if (readData == nullptr || size == 0) {
        return -1;
    }
    void *tempData = malloc(size);
    if (tempData == nullptr) {
        HILOG_ERROR("tempData is nullptr");
        return -1;
    }
    if (memcpy_s(tempData, size, readData, size) != EOK) {
        free(tempData);
        HILOG_ERROR("memcpy_s tempData is fail");
        return -1;
    }
    if (data != nullptr) {
        free(const_cast<void *>(data));
    }
    data = tempData;
    free(tempData);
    return 0;
}
} // namespace OHOS::WallpaperMgrService