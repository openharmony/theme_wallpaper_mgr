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
#include "memory_guard.h"
#include "parcel.h"
#include "pixel_map.h"
#include "wallpaper_common.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace OHOS::HiviewDFX;
using namespace OHOS::Media;

WallpaperServiceStub::WallpaperServiceStub(bool serialInvokeFlag) : IRemoteStub(serialInvokeFlag)
{
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::SET_WALLPAPER] = &WallpaperServiceStub::OnSetWallpaper;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::GET_PIXELMAP] = &WallpaperServiceStub::OnGetPixelMap;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::GET_COLORS] = &WallpaperServiceStub::OnGetColors;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_ID] = &WallpaperServiceStub::OnGetWallpaperId;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::GET_FILE] = &WallpaperServiceStub::OnGetFile;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::ON] = &WallpaperServiceStub::OnWallpaperOn;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::OFF] = &WallpaperServiceStub::OnWallpaperOff;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::IS_CHANGE_PERMITTED] = &WallpaperServiceStub::OnIsChangePermitted;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::IS_OPERATION_ALLOWED] =
        &WallpaperServiceStub::OnIsOperationAllowed;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER] = &WallpaperServiceStub::OnResetWallpaper;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::REGISTER_CALLBACK] =
        &WallpaperServiceStub::OnRegisterWallpaperCallback;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_V9] = &WallpaperServiceStub::OnSetWallpaperV9;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::GET_PIXELMAP_V9] = &WallpaperServiceStub::OnGetPixelMapV9;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::GET_COLORS_V9] = &WallpaperServiceStub::OnGetColorsV9;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER_V9] = &WallpaperServiceStub::OnResetWallpaperV9;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::SET_VIDEO] = &WallpaperServiceStub::OnSetVideo;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::SET_CUSTOM] = &WallpaperServiceStub::OnSetCustomWallpaper;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::SEND_EVENT] = &WallpaperServiceStub::OnSendEvent;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_PIXELMAP] =
        &WallpaperServiceStub::OnSetWallpaperByPixelMap;
    memberFuncMap_[WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_PIXELMAP_V9] =
        &WallpaperServiceStub::OnSetWallpaperV9ByPixelMap;
}

WallpaperServiceStub::~WallpaperServiceStub()
{
    memberFuncMap_.clear();
}

int32_t WallpaperServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
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
    if (code >= static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER)
        && code <= static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_PIXELMAP_V9)) {
        auto itFunc = memberFuncMap_.find(static_cast<WallpaperServiceIpcInterfaceCode>(code));
        if (itFunc != memberFuncMap_.end()) {
            auto memberFunc = itFunc->second;
            if (memberFunc != nullptr) {
                return (this->*memberFunc)(data, reply);
            }
        }
    }
    int32_t ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    HILOG_INFO("end##ret = %{public}d", ret);
    return ret;
}

int32_t WallpaperServiceStub::OnSetWallpaper(MessageParcel &data, MessageParcel &reply)
{
    return OnSetWallpaperInner(data, reply, false);
}

int32_t WallpaperServiceStub::OnSetWallpaperV9(MessageParcel &data, MessageParcel &reply)
{
    return OnSetWallpaperInner(data, reply, true);
}

int32_t WallpaperServiceStub::OnSetWallpaperInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi)
{
    HILOG_DEBUG("WallpaperServiceStub::SetWallpaper start.");
    int32_t fd = data.ReadFileDescriptor();
    if (fd < 0) {
        HILOG_ERROR("ReadFileDescriptor fail. fd[%{public}d]", fd);
        return IPC_STUB_INVALID_DATA_ERR;
    }
    int32_t wallpaperType = data.ReadInt32();
    int32_t length = data.ReadInt32();
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (isSystemApi) {
        wallpaperErrorCode = SetWallpaperV9(fd, wallpaperType, length);
    } else {
        wallpaperErrorCode = SetWallpaper(fd, wallpaperType, length);
    }
    close(fd);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnSetWallpaperByPixelMap(MessageParcel &data, MessageParcel &reply)
{
    return OnSetWallpaperInnerByPixelMap(data, reply, false);
}

int32_t WallpaperServiceStub::OnSetWallpaperV9ByPixelMap(MessageParcel &data, MessageParcel &reply)
{
    return OnSetWallpaperInnerByPixelMap(data, reply, true);
}

int32_t WallpaperServiceStub::OnSetWallpaperInnerByPixelMap(MessageParcel &data, MessageParcel &reply, bool isSystemApi)
{
    HILOG_DEBUG("WallpaperServiceStub::SetWallpaper start.");
    int32_t vectorPixelMapSize = data.ReadInt32();
    if (vectorPixelMapSize <= 0) {
        HILOG_ERROR("ReadInt32 fail ");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    auto *rawData = (uint8_t *)data.ReadRawData(vectorPixelMapSize);
    if (rawData == nullptr) {
        HILOG_ERROR("ReadRawData fail");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    std::vector<uint8_t> VectorPixelMap(rawData, rawData + vectorPixelMapSize);
    int32_t wallpaperType = data.ReadInt32();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = VectorToPixelMap(VectorPixelMap);
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (isSystemApi) {
        wallpaperErrorCode = SetWallpaperV9ByPixelMap(pixelMap, wallpaperType);
    } else {
        wallpaperErrorCode = SetWallpaperByPixelMap(pixelMap, wallpaperType);
    }
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetPixelMap(MessageParcel &data, MessageParcel &reply)
{
    return OnGetPixelMapInner(data, reply, false);
}

int32_t WallpaperServiceStub::OnSetVideo(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnSetVideo start.");
    int32_t fd = data.ReadFileDescriptor();
    int32_t wallpaperType = data.ReadInt32();
    int32_t length = data.ReadInt32();
    ErrorCode wallpaperErrorCode = SetVideo(fd, wallpaperType, length);
    close(fd);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("Write int is failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnSetCustomWallpaper(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::SetCustomWallpaper start.");
    auto fd = data.ReadFileDescriptor();
    int32_t wallpaperType = data.ReadInt32();
    int32_t length = data.ReadInt32();
    ErrorCode wallpaperErrorCode = SetCustomWallpaper(fd, wallpaperType, length);
    close(fd);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("Write int is failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetPixelMapV9(MessageParcel &data, MessageParcel &reply)
{
    return OnGetPixelMapInner(data, reply, true);
}

int32_t WallpaperServiceStub::OnGetPixelMapInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi)
{
    HILOG_DEBUG("WallpaperServiceStub::GetPixelMap start.");
    int32_t wallpaperType = data.ReadInt32();
    IWallpaperService::FdInfo fdInfo;
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (isSystemApi) {
        wallpaperErrorCode = GetPixelMapV9(wallpaperType, fdInfo);
    } else {
        wallpaperErrorCode = GetPixelMap(wallpaperType, fdInfo);
    }
    HILOG_INFO(" OnGetPixelMap wallpaperErrorCode = %{public}d", wallpaperErrorCode);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail");
        close(fdInfo.fd);
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperErrorCode == E_OK) {
        if (!reply.WriteInt32(fdInfo.size)) {
            HILOG_ERROR("WriteInt32 fail");
            close(fdInfo.fd);
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        if (!reply.WriteFileDescriptor(fdInfo.fd)) {
            HILOG_ERROR("WriteFileDescriptor fail");
            close(fdInfo.fd);
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
    }
    close(fdInfo.fd);
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetColors(MessageParcel &data, MessageParcel &reply)
{
    return OnGetColorsInner(data, reply, false);
}

int32_t WallpaperServiceStub::OnGetColorsV9(MessageParcel &data, MessageParcel &reply)
{
    return OnGetColorsInner(data, reply, true);
}

int32_t WallpaperServiceStub::OnGetColorsInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi)
{
    HILOG_DEBUG("WallpaperServiceStub::OnGetColors start.");
    int32_t wallpaperType = data.ReadInt32();
    std::vector<uint64_t> vecWallpaperColors;
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (isSystemApi) {
        wallpaperErrorCode = GetColorsV9(wallpaperType, vecWallpaperColors);
    } else {
        wallpaperErrorCode = GetColors(wallpaperType, vecWallpaperColors);
    }
    auto size = vecWallpaperColors.size();
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperErrorCode == E_OK) {
        if (!reply.WriteUInt64Vector(vecWallpaperColors)) {
            HILOG_ERROR("WallpaperServiceStub::OnGetColors WriteUInt64Vector error.");
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
    }
    return (size == 0) ? IPC_STUB_INVALID_DATA_ERR : ERR_NONE;
}

int32_t WallpaperServiceStub::OnGetFile(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnGetFile start.");
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
    HILOG_DEBUG("WallpaperServiceStub::OnGetWallpaperId start.");
    int32_t wallpaperType = data.ReadInt32();
    int32_t wallpaperId = GetWallpaperId(wallpaperType);
    if (!reply.WriteInt32(wallpaperId)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    HILOG_INFO("End. Id[%{public}d]", wallpaperId);
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnIsChangePermitted(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnIsChangePermitted start.");
    auto bResult = IsChangePermitted();
    if (!reply.WriteBool(bResult)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnIsOperationAllowed(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnIsOperationAllowed start.");
    auto bResult = IsOperationAllowed();
    if (!reply.WriteBool(bResult)) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnResetWallpaper(MessageParcel &data, MessageParcel &reply)
{
    return OnResetWallpaperInner(data, reply, false);
}

int32_t WallpaperServiceStub::OnResetWallpaperV9(MessageParcel &data, MessageParcel &reply)
{
    return OnResetWallpaperInner(data, reply, true);
}

int32_t WallpaperServiceStub::OnResetWallpaperInner(MessageParcel &data, MessageParcel &reply, bool isSystemApi)
{
    HILOG_DEBUG("WallpaperServiceStub::OnResetWallpaper start.");
    int32_t wallpaperType = data.ReadInt32();
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (isSystemApi) {
        wallpaperErrorCode = ResetWallpaperV9(wallpaperType);
    } else {
        wallpaperErrorCode = ResetWallpaper(wallpaperType);
    }
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("Write result data failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnWallpaperOn(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOn in");
    std::string type = data.ReadString();
    if (type.empty()) {
        HILOG_ERROR("OnWallpaperOn type is empty after ipc");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOG_ERROR("OnWallpaperOn nullptr after ipc");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    sptr<IWallpaperEventListener> wallpaperListenerProxy = iface_cast<IWallpaperEventListener>(remote);
    ErrorCode errorCode = On(type, std::move(wallpaperListenerProxy));
    int32_t ret = static_cast<int32_t>(errorCode);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnWallpaperOff(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOff in");
    std::string type = data.ReadString();
    if (type.empty()) {
        HILOG_ERROR("OnWallpaperOff type is empty after ipc");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    ErrorCode errorCode = E_UNKNOWN;
    if (remote == nullptr) {
        errorCode = Off(type, nullptr);
    } else {
        sptr<IWallpaperEventListener> wallpaperListenerProxy = iface_cast<IWallpaperEventListener>(remote);
        errorCode = Off(type, std::move(wallpaperListenerProxy));
    }
    int32_t ret = static_cast<int32_t>(errorCode);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnSendEvent(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnSendEvent start.");
    std::string eventType = data.ReadString();
    auto result = SendEvent(eventType);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("Write int is failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnRegisterWallpaperCallback(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnRegisterWallpaperCallback start");
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

std::shared_ptr<OHOS::Media::PixelMap> WallpaperServiceStub::VectorToPixelMap(std::vector<std::uint8_t> value)
{
    HILOG_DEBUG("VectorToPixelMap start");
    if (value.size() == 0) {
        return nullptr;
    }
    return std::shared_ptr<PixelMap> (PixelMap::DecodeTlv(value));
}
} // namespace WallpaperMgrService
} // namespace OHOS