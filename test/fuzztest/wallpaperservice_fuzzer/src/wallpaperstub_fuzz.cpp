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

#include "message_parcel.h"
#include "wallpaper_service.h"
#include "wallpaperstub_fuzzer.h"

using namespace OHOS::WallpaperMgrService;
constexpr size_t THRESHOLD = 4;
namespace OHOS {
const std::u16string WALLPAPERSERVICES_INTERFACE_TOKEN = u"ohos.Wallpaper.IWallpaperService";

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

void FuzzTestRemoteRequest(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(WALLPAPERSERVICES_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    uint32_t code = ConvertToUint32(rawData);
    wallpaperService->OnRemoteRequest(code, data, reply, option);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < THRESHOLD) {
        return 0;
    }
    OHOS::FuzzTestRemoteRequest(data, size);
    return 0;
}