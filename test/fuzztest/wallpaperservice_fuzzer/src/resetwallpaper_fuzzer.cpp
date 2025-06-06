/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>

#include "fuzztest_utils.h"
#include "message_parcel.h"
#include "resetwallpaper_fuzzer.h"
#include "wallpaper_service.h"

using namespace OHOS::WallpaperMgrService;
constexpr uint32_t CODE = static_cast<uint32_t>(IWallpaperServiceIpcCode::COMMAND_RESET_WALLPAPER);

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    OHOS::FuzzTestUtils::FuzzTestRemoteRequest(provider, CODE);
    return 0;
}