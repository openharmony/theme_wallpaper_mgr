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

#ifndef SERVICES_INCLUDE_WALLPAPER_SERVICE_STUB_H
#define SERVICES_INCLUDE_WALLPAPER_SERVICE_STUB_H

#include <map>

#include "i_wallpaper_service.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"
#include "wallpaper_service_ipc_interface_code.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperServiceStub : public IRemoteStub<IWallpaperService> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    WallpaperServiceStub(bool serialInvokeFlag = true);
    ~WallpaperServiceStub();

private:
    int32_t OnSetWallpaper(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetWallpaperByPixelMap(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetPixelMap(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetColors(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetFile(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallpaperId(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsChangePermitted(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsOperationAllowed(MessageParcel &data, MessageParcel &reply);
    int32_t OnResetWallpaper(MessageParcel &data, MessageParcel &reply);
    int32_t OnWallpaperOn(MessageParcel &data, MessageParcel &reply);
    int32_t OnWallpaperOff(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterWallpaperCallback(MessageParcel &data, MessageParcel &reply);

    int32_t OnSetWallpaperV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetWallpaperV9ByPixelMap(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetPixelMapV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetColorsV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnResetWallpaperV9(MessageParcel &data, MessageParcel &reply);

    int32_t OnSetWallpaperInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnSetWallpaperInnerByPixelMap(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnGetPixelMapInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnGetColorsInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnResetWallpaperInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnSetVideo(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetCustomWallpaper(MessageParcel &data, MessageParcel &reply);
    int32_t OnSendEvent(MessageParcel &data, MessageParcel &reply);

    using WallpaperServiceFunc = int32_t (WallpaperServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    std::shared_ptr<OHOS::Media::PixelMap> VectorToPixelMap(std::vector<std::uint8_t> value);
    std::map<WallpaperServiceIpcInterfaceCode, WallpaperServiceFunc> memberFuncMap_;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_STUB_H