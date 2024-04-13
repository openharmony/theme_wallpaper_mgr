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
#include "wallpaper_service_cb_stub.h"

#include "hilog_wrapper.h"
#include "wallpaper_manager.h"

namespace OHOS {
namespace WallpaperMgrService {
WallpaperServiceCbStub::WallpaperServiceCbStub()
{
    memberFuncMap_[ONCALL] = &WallpaperServiceCbStub::HandleOnCall;
}

int32_t WallpaperServiceCbStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    HILOG_INFO("  WallpaperServiceCbStub::OnRemoteRequest start##ret = %{public}u", code);
    std::u16string myDescriptor = WallpaperServiceCbStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        HILOG_ERROR(" end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int32_t ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    HILOG_INFO("  WallpaperServiceCbStub::OnRemoteRequest end##ret = %{public}d", ret);
    return ret;
}

int32_t WallpaperServiceCbStub::HandleOnCall(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("  WallpaperServiceCbStub::HandleOnCall");
    int32_t wallpaperType = data.ReadInt32();
    OnCall(wallpaperType);
    HILOG_INFO("wallpaperType = %{public}d", wallpaperType);
    return 0;
}

int32_t WallpaperServiceCbStub::OnCall(const int32_t num)
{
    HILOG_INFO("  WallpaperServiceCbStub::OnCall");
    WallpaperMgrService::WallpaperManagerkits::GetInstance().GetCallback()(num);
    return 0;
}

} // namespace WallpaperMgrService
} // namespace OHOS