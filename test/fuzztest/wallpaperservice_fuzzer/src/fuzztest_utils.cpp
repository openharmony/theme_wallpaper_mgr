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

#include "fuzztest_utils.h"

#include "message_parcel.h"
#include "wallpaper_service.h"
using namespace OHOS::WallpaperMgrService;
namespace OHOS {
const std::u16string WALLPAPERSERVICES_INTERFACE_TOKEN = u"ohos.Wallpaper.IWallpaperService";
void FuzzTestUtils::FuzzTestRemoteRequest(const uint8_t *rawData, size_t size, uint32_t code)
{
    MessageParcel data;
    data.WriteInterfaceToken(WALLPAPERSERVICES_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    wallpaperService->OnRemoteRequest(code, data, reply, option);
}
} // namespace OHOS
