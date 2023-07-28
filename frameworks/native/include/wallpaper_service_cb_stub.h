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
#ifndef SERVICES_INCLUDE_WALLPAPER_SERVICE_CB_STUB_H
#define SERVICES_INCLUDE_WALLPAPER_SERVICE_CB_STUB_H

#include <map>

#include "i_wallpaper_callback.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperManager;
class WallpaperServiceCbStub : public IRemoteStub<IWallpaperCallback> {
public:
    WallpaperServiceCbStub();
    ~WallpaperServiceCbStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t OnCall(const int32_t num) override;

private:
    int32_t HandleOnCall(MessageParcel &data, MessageParcel &reply);
    using WallpaperCallbackFunc = int32_t (WallpaperServiceCbStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, WallpaperCallbackFunc> memberFuncMap_;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif