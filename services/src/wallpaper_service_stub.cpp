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

#include "wallpaper_service_stub.h"

#include <unistd.h>

#include "ashmem.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "parcel.h"
#include "pixel_map.h"
#include "wallpaper_common.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace OHOS::HiviewDFX;
using namespace OHOS::Media;

WallpaperServiceStub::WallpaperServiceStub()
{
    memberFuncMap_[SET_WALLPAPER_URI_FD] = &WallpaperServiceStub::OnSetWallpaperUriByFD;
    memberFuncMap_[SET_WALLPAPER_MAP] = &WallpaperServiceStub::OnSetWallpaperByMap;
    memberFuncMap_[GET_PIXELMAP] = &WallpaperServiceStub::OnGetPixelMap;
    memberFuncMap_[GET_COLORS] = &WallpaperServiceStub::OnGetColors;
    memberFuncMap_[GET_WALLPAPER_ID] = &WallpaperServiceStub::OnGetWallpaperId;
    memberFuncMap_[GET_FILE] = &WallpaperServiceStub::OnGetFile;
    memberFuncMap_[GET_WALLPAPER_MIN_HEIGHT] = &WallpaperServiceStub::OnGetWallpaperMinHeight;
    memberFuncMap_[GET_WALLPAPER_MIN_WIDTH] = &WallpaperServiceStub::OnGetWallpaperMinWidth;
    memberFuncMap_[ON] = &WallpaperServiceStub::OnWallpaperOn;
    memberFuncMap_[OFF] = &WallpaperServiceStub::OnWallpaperOff;
    memberFuncMap_[IS_CHANGE_PERMITTED] = &WallpaperServiceStub::OnIsChangePermitted;
    memberFuncMap_[IS_OPERATION_ALLOWED] = &WallpaperServiceStub::OnIsOperationAllowed;
    memberFuncMap_[RESET_WALLPAPER] = &WallpaperServiceStub::OnResetWallpaper;
    memberFuncMap_[REGISTER_CALLBACK] = &WallpaperServiceStub::OnRegisterWallpaperCallback;
}

WallpaperServiceStub::~WallpaperServiceStub()
{
    memberFuncMap_.clear();
}

int32_t WallpaperServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    HILOG_INFO(" start##ret = %{public}u", code);
    std::u16string myDescripter = WallpaperServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        HILOG_ERROR(" end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    pid_t p = IPCSkeleton::GetCallingPid();
    pid_t p1 = IPCSkeleton::GetCallingUid();
    HILOG_INFO("CallingPid = %{public}d, CallingUid = %{public}d, code = %{public}u", p, p1, code);
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    HILOG_INFO(" end##ret = %{public}d", ret);
    return ret;
}
int32_t WallpaperServiceStub::OnSetWallpaperByMap(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::SetWallpaperMap start.");

    int fd = data.ReadFileDescriptor();
    int wallpaperType = data.ReadInt32();
    int length = data.ReadInt32();
    int32_t wallpaperErrorCode = SetWallpaperByMap(fd, wallpaperType, length);
    close(fd);
    reply.WriteInt32(wallpaperErrorCode);
    return static_cast<int32_t>(E_OK) ? 0 : -1;
}
int32_t WallpaperServiceStub::OnSetWallpaperUriByFD(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::SetWallpaperUri start.");

    int fd = data.ReadFileDescriptor();
    int wallpaperType = data.ReadInt32();
    HILOG_INFO("wallpaperType= %{public}d", wallpaperType);
    int length = data.ReadInt32();
    HILOG_INFO("SetWallpaperByFD start");
    int32_t wallpaperErrorCode = SetWallpaperByFD(fd, wallpaperType, length);
    close(fd);
    reply.WriteInt32(wallpaperErrorCode);
    return wallpaperErrorCode == static_cast<int32_t>(E_OK) ? 0 : -1;
}

int32_t WallpaperServiceStub::OnGetPixelMap(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = 0;
    HILOG_INFO("WallpaperServiceStub::GetPixelMap start.");

    int wallpaperType = data.ReadInt32();
    IWallpaperService::FdInfo fdInfo;
    int wallpaperErrorCode = GetPixelMap(wallpaperType, fdInfo);
    HILOG_INFO(" OnGetPixelMap wallpaperErrorCode = %{public}d", wallpaperErrorCode);
    reply.WriteInt32(wallpaperErrorCode);
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        if (!reply.WriteInt32(fdInfo.size)) {
            HILOG_ERROR("WriteInt32 fail");
            ret = -1;
        }
        if (!reply.WriteFileDescriptor(fdInfo.fd)) {
            HILOG_ERROR("WriteFileDescriptor fail");
            ret = -1;
        }
    }
    HILOG_INFO(" OnGetPixelMap ret = %{public}d", ret);
    return ret;
}

int32_t WallpaperServiceStub::OnGetColors(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetColors start.");
    int wallpaperType = data.ReadInt32();
    std::vector<uint64_t> vecWallpaperColors = GetColors(wallpaperType);
    auto size = vecWallpaperColors.size();
    if (!reply.WriteUInt64Vector(vecWallpaperColors)) {
        HILOG_ERROR("WallpaperServiceStub::OnGetColors WriteUInt64Vector error.");
        return -1;
    }
    return (size == 0) ? -1 : 0;
}

int32_t WallpaperServiceStub::OnGetFile(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetFile start.");

    int32_t wallpaperType = data.ReadInt32();
    int wallpaperFd;
    int32_t wallpaperErrorCode = GetFile(wallpaperType, wallpaperFd);
    reply.WriteFileDescriptor(wallpaperFd);
    reply.WriteInt32(wallpaperErrorCode);
    return (wallpaperFd >= 0) ? 0 : -1;
}

int32_t WallpaperServiceStub::OnGetWallpaperId(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetWallpaperId start.");

    int wallpaperType = data.ReadInt32();
    int wallpaerid = GetWallpaperId(wallpaperType);
    if (!reply.WriteInt32(wallpaerid)) {
        HILOG_ERROR("Write result data failed");
    }
    HILOG_INFO("End. Id[%{public}d]", wallpaerid);
    return wallpaerid;
}

int32_t WallpaperServiceStub::OnGetWallpaperMinHeight(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetWallpaperMinHeight start.");
    int wallpaerMinHeight = GetWallpaperMinHeight();
    if (!reply.WriteInt32(wallpaerMinHeight)) {
        HILOG_ERROR("Write result data failed");
    }
    HILOG_INFO("End. height[%{public}d]", wallpaerMinHeight);
    return wallpaerMinHeight;
}

int32_t WallpaperServiceStub::OnGetWallpaperMinWidth(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetWallpaperMinWidth start.");

    int wallpaperMinWidth = GetWallpaperMinWidth();
    if (!reply.WriteInt32(wallpaperMinWidth)) {
        HILOG_ERROR("Write result data failed");
    }
    HILOG_INFO("End. width[%{public}d]", wallpaperMinWidth);
    return wallpaperMinWidth;
}

int32_t WallpaperServiceStub::OnIsChangePermitted(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnIsChangePermitted start.");
    auto bResult = IsChangePermitted();
    if (!reply.WriteBool(bResult)) {
        HILOG_ERROR("Write result data failed");
    }
    return (bResult) ? 0 : -1;
}

int32_t WallpaperServiceStub::OnIsOperationAllowed(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnIsOperationAllowed start.");
    auto bResult = IsOperationAllowed();
    if (!reply.WriteBool(bResult)) {
        HILOG_ERROR("Write result data failed");
    }
    return (bResult) ? 0 : -1;
}

int32_t WallpaperServiceStub::OnResetWallpaper(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnResetWallpaper start.");
    int wallpaperType = data.ReadInt32();
    auto wallpaperErrorCode = ResetWallpaper(wallpaperType);
    if (!reply.WriteInt32(wallpaperErrorCode)) {
        HILOG_ERROR("Write result data failed");
    }
    return wallpaperErrorCode == static_cast<int32_t>(E_OK) ? 0 : -1;
}

int32_t WallpaperServiceStub::OnWallpaperOn(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOn in");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOG_ERROR("OnWallpaperOn nullptr after ipc");
        if (!reply.WriteInt32(static_cast<int32_t>(E_READ_PARCEL_ERROR))) {
            return -1;
        }
        return 0;
    }
    sptr<IWallpaperColorChangeListener> WallpaperListenerProxy = iface_cast<IWallpaperColorChangeListener>(remote);

    bool status = On(std::move(WallpaperListenerProxy));
    int32_t ret = status ? 0 : -1;
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed");
        return -1;
    }
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOn out");
    return 0;
}

int32_t WallpaperServiceStub::OnWallpaperOff(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOff in");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    bool status = false;
    if (remote == nullptr) {
        status = Off(nullptr);
    } else {
        sptr<IWallpaperColorChangeListener> WallpaperListenerProxy = iface_cast<IWallpaperColorChangeListener>(remote);
        status = Off(std::move(WallpaperListenerProxy));
    }
    int32_t ret = status ? 0 : -1;
    if (!reply.WriteInt32(ret)) {
        return -1;
    }
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOff out");
    return 0;
}

int32_t WallpaperServiceStub::OnRegisterWallpaperCallback(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("  WallpaperServiceStub::OnRegisterWallpaperCallback start");
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    if (object == nullptr) {
        HILOG_ERROR("RegisterWallpaperCallback failed");
        reply.WriteInt32(-1);
        return -1;
    }
    sptr<IWallpaperCallback> callbackProxy = iface_cast<IWallpaperCallback>(object);

    RegisterWallpaperCallback(callbackProxy);
    reply.WriteInt32(0);
    return 0;
}
} // namespace WallpaperMgrService
} // namespace OHOS