/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "wallpaper_service.h"
#include "updatewallpapermap_fuzzer.h"

using namespace OHOS::WallpaperMgrService;
namespace OHOS {
WallpaperMgrService WallpaperMgrService;
constexpr size_t THRESHOLD = 4;
int32_t ConverToInt32(const uint8_t *ptr, size_t size)
{
    if (ptr == nullptr || (size < sizeof(int32_t))) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    if (bigVar < 0) {
        return 0;
    }
    return bigVar;
}

void UpdataWallpaperMapFuzzTest(const uint8_t *data, size_t size)
{
    int32_t userId = ConverToInt32(data, size);
    int32_t wallpaperType = 1;
    WallpaperMgrService.UpdataWallpaperMap(userId, wallpaperType);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < THRESHOLD) {
        return 0;
    }
    OHOS::UpdataWallpaperMapFuzzTest(data, size);
    return 0;
}
} // namespace OHOS
