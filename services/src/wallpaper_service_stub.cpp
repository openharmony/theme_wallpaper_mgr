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
#include "memory_guard.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace OHOS::HiviewDFX;
using namespace OHOS::Media;

WallpaperServiceStub::WallpaperServiceStub()
{
    memberFuncMap_[SET_WALLPAPER] = &WallpaperServiceStub::OnSetWallpaper;
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
    MemoryGuard cacheGuard;
    HILOG_INFO("start##ret = %{public}u", code);
    std::u16string myDescriptor = WallpaperServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        HILOG_ERROR("end##descriptor checked fail");
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
    int32_t ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    HILOG_INFO("end##ret = %{public}d", ret);
    return ret;
}

int32_t WallpaperServiceStub::OnSetWallpaper(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::SetWallpaper start.");

    int32_t fd = data.ReadFileDescriptor();
    int32_t wallpaperType = data.ReadInt32();
    int32_t length = data.ReadInt32();
    ErrorCode wallpaperErrorCode = SetWallpaper(fd, wallpaperType, length);
    close(fd);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetPixelMap(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::GetPixelMap start.");

    int32_t wallpaperType = data.ReadInt32();
    IWallpaperService::FdInfo fdInfo;
    ErrorCode wallpaperErrorCode = GetPixelMap(wallpaperType, fdInfo);
    HILOG_INFO(" OnGetPixelMap wallpaperErrorCode = %{public}d", wallpaperErrorCode);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperErrorCode == E_OK) {
        if (!reply.WriteInt32(fdInfo.size)) {
            HILOG_ERROR("WriteInt32 fail");
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        if (!reply.WriteFileDescriptor(fdInfo.fd)) {
            HILOG_ERROR("WriteFileDescriptor fail");
            close(fdInfo.fd);
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        close(fdInfo.fd);
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetColors(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetColors start.");
    int32_t wallpaperType = data.ReadInt32();
    std::vector<uint64_t> vecWallpaperColors = GetColors(wallpaperType);
    auto size = vecWallpaperColors.size();
    if (!reply.WriteUInt64Vector(vecWallpaperColors)) {
        HILOG_ERROR("WallpaperServiceStub::OnGetColors WriteUInt64Vector error.");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return (size == 0) ? IPC_STUB_INVALID_DATA_ERR : ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetFile(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetFile start.");

    int32_t wallpaperType = data.ReadInt32();
    int32_t wallpaperFd = INVALID_FD;
    ErrorCode wallpaperErrorCode = GetFile(wallpaperType, wallpaperFd);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail");
        if (wallpaperFd > INVALID_FD) {
            close(wallpaperFd);
        }
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperErrorCode == E_OK && !reply.WriteFileDescriptor(wallpaperFd)) {
        HILOG_ERROR("WriteFileDescriptor fail");
        close(wallpaperFd);
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperFd > INVALID_FD) {
        close(wallpaperFd);
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetWallpaperId(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetWallpaperId start.");

    int32_t wallpaperType = data.ReadInt32();
    int32_t wallpaperId = GetWallpaperId(wallpaperType);
    if (!reply.WriteInt32(wallpaperId)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    HILOG_INFO("End. Id[%{public}d]", wallpaperId);
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetWallpaperMinHeight(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetWallpaperMinHeight start.");
    int32_t wallpaperMinHeight = GetWallpaperMinHeight();
    if (!reply.WriteInt32(wallpaperMinHeight)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    HILOG_INFO("End. height[%{public}d]", wallpaperMinHeight);
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetWallpaperMinWidth(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnGetWallpaperMinWidth start.");

    int32_t wallpaperMinWidth = GetWallpaperMinWidth();
    if (!reply.WriteInt32(wallpaperMinWidth)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    HILOG_INFO("End. width[%{public}d]", wallpaperMinWidth);
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnIsChangePermitted(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnIsChangePermitted start.");
    auto bResult = IsChangePermitted();
    if (!reply.WriteBool(bResult)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnIsOperationAllowed(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnIsOperationAllowed start.");
    auto bResult = IsOperationAllowed();
    if (!reply.WriteBool(bResult)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnResetWallpaper(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnResetWallpaper start.");
    int32_t wallpaperType = data.ReadInt32();
    ErrorCode wallpaperErrorCode = ResetWallpaper(wallpaperType);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnWallpaperOn(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOn in");
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOG_ERROR("OnWallpaperOn nullptr after ipc");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    sptr<IWallpaperColorChangeListener> WallpaperListenerProxy = iface_cast<IWallpaperColorChangeListener>(remote);

    bool status = On(std::move(WallpaperListenerProxy));
    int32_t ret = status ? static_cast<int32_t>(E_OK) : static_cast<int32_t>(E_DEAL_FAILED);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOn out");
    return ERR_NONE;
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
    int32_t ret = status ? static_cast<int32_t>(E_OK) : static_cast<int32_t>(E_DEAL_FAILED);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOff out");
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnRegisterWallpaperCallback(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("WallpaperServiceStub::OnRegisterWallpaperCallback start");
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    bool status = false;
    if (object == nullptr) {
        HILOG_ERROR("RegisterWallpaperCallback failed");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    sptr<IWallpaperCallback> callbackProxy = iface_cast<IWallpaperCallback>(object);

    status = RegisterWallpaperCallback(callbackProxy);
    int32_t ret = status ? static_cast<int32_t>(E_OK) : static_cast<int32_t>(E_DEAL_FAILED);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}
} // namespace WallpaperMgrService
} // namespace OHOS