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

#include<map>
#include "iremote_stub.h"
#include "i_wallpaper_service.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperServiceStub : public IRemoteStub<IWallpaperService> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    WallpaperServiceStub();
    ~WallpaperServiceStub();
private:
    int32_t OnSetWallpaperUriByFD(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetWallpaperByMap(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetPixelMapFile(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetPixelMap(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetColors(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetFile(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallpaperId(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallpaperMinHeight(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallpaperMinWidth(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsChangePermitted(MessageParcel &data, MessageParcel &reply);
    int32_t OnIsOperationAllowed(MessageParcel &data, MessageParcel &reply);
    int32_t OnResetWallpaper(MessageParcel &data, MessageParcel &reply);
    int32_t OnWallpaperOn(MessageParcel &data, MessageParcel &reply);
    int32_t OnWallpaperOff(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterWallpaperCallback(MessageParcel &data, MessageParcel &reply);

    int32_t OnSetWallpaperUriByFDV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetWallpaperByMapV9(MessageParcel &data, MessageParcel &reply);

    int32_t OnGetPixelMapV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetColorsV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallpaperMinHeightV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetWallpaperMinWidthV9(MessageParcel &data, MessageParcel &reply);
    int32_t OnResetWallpaperV9(MessageParcel &data, MessageParcel &reply);

    int32_t OnSetWallpaperUriByFDInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnSetWallpaperByMapInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnGetPixelMapInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnGetColorsInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnGetWallpaperMinHeightInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnGetWallpaperMinWidthInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);
    int32_t OnResetWallpaperInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi);

    using WallpaperServiceFunc = int32_t (WallpaperServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, WallpaperServiceFunc> memberFuncMap_;
};
}
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICE_STUB_H