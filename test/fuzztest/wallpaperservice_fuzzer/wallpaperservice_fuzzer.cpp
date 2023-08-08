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

#include "wallpaperservice_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "message_parcel.h"
#include "wallpaper_service.h"

using namespace OHOS::WallpaperMgrService;

namespace OHOS {
constexpr size_t THRESHOLD = 4;
const std::u16string WALLPAPERSERVICES_INTERFACE_TOKEN = u"ohos.Wallpaper.IWallpaperService";

using namespace OHOS::Security::AccessToken;

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

bool FuzzWallpaperService(const uint8_t *rawData, size_t size, uint32_t code)
{
    MessageParcel data;
    data.WriteInterfaceToken(WALLPAPERSERVICES_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    wallpaperService->OnRemoteRequest(code, data, reply, option);
    return true;
}

bool FuzzSetWallpaperServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzGetPixelMapServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_PIXELMAP);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzGetColorsServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_COLORS);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzGetWallpaperIdServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_ID);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzGetFileServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_FILE);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzResetWallpaperServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzOnServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::ON);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzOffServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::OFF);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzIsChangePermittedServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::IS_CHANGE_PERMITTED);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzIsOperationAllowedServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::IS_OPERATION_ALLOWED);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzRegisterCallbackServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::REGISTER_CALLBACK);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzSetWallpaperV9ServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_V9);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzGetPixelMapV9ServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_PIXELMAP_V9);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzGetColorsV9ServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_COLORS_V9);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzResetWallpaperV9ServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER_V9);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzSetVideoServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_VIDEO);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzSetCustomServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_CUSTOM);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzSendEventServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SEND_EVENT);
    return FuzzWallpaperService(rawData, size, code);
}

bool FuzzBoundaryCodeServiceTest(const uint8_t *rawData, size_t size){
    uint32_t code = static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SEND_EVENT) + 1;
    return FuzzWallpaperService(rawData, size, code);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }

    /* Run your code on data */
    OHOS::FuzzSetWallpaperServiceTest(data, size);
    OHOS::FuzzGetPixelMapServiceTest(data, size);
    OHOS::FuzzGetColorsServiceTest(data, size);
    OHOS::FuzzGetWallpaperIdServiceTest(data, size);
    OHOS::FuzzGetFileServiceTest(data, size);
    OHOS::FuzzResetWallpaperServiceTest(data, size);
    OHOS::FuzzOnServiceTest(data, size);
    OHOS::FuzzOffServiceTest(data, size);
    OHOS::FuzzIsChangePermittedServiceTest(data, size);
    OHOS::FuzzIsOperationAllowedServiceTest(data, size);
    OHOS::FuzzRegisterCallbackServiceTest(data, size);
    OHOS::FuzzSetWallpaperV9ServiceTest(data, size);
    OHOS::FuzzGetPixelMapV9ServiceTest(data, size);
    OHOS::FuzzGetColorsV9ServiceTest(data, size);
    OHOS::FuzzResetWallpaperV9ServiceTest(data, size);
    OHOS::FuzzSetVideoServiceTest(data, size);
    OHOS::FuzzSetCustomServiceTest(data, size);
    OHOS::FuzzSendEventServiceTest(data, size);
    OHOS::FuzzBoundaryCodeServiceTest(data, size);
    return 0;
}