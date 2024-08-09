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

#include <unistd.h>

#include "ashmem.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "memory_guard.h"
#include "parcel.h"
#include "pixel_map.h"
#include "wallpaper_common.h"
#include "wallpaper_service_ipc_interface_code.h"
#include "wallpaper_service_stub.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace OHOS::HiviewDFX;
using namespace OHOS::Media;

WallpaperServiceStub::WallpaperServiceStub(bool serialInvokeFlag) : IRemoteStub(serialInvokeFlag)
{
    requestHandlers = {
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER),
            [this](MessageParcel &data, MessageParcel &reply) { return OnSetWallpaper(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_PIXELMAP),
            [this](MessageParcel &data, MessageParcel &reply) { return OnGetPixelMap(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_COLORS),
            [this](MessageParcel &data, MessageParcel &reply) { return OnGetColors(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_ID),
            [this](MessageParcel &data, MessageParcel &reply) { return OnGetWallpaperId(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_FILE),
            [this](MessageParcel &data, MessageParcel &reply) { return OnGetFile(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::ON),
            [this](MessageParcel &data, MessageParcel &reply) { return OnWallpaperOn(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::OFF),
            [this](MessageParcel &data, MessageParcel &reply) { return OnWallpaperOff(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::IS_CHANGE_PERMITTED),
            [this](MessageParcel &data, MessageParcel &reply) { return OnIsChangePermitted(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::IS_OPERATION_ALLOWED),
            [this](MessageParcel &data, MessageParcel &reply) { return OnIsOperationAllowed(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER),
            [this](MessageParcel &data, MessageParcel &reply) { return OnResetWallpaper(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::REGISTER_CALLBACK),
            [this](MessageParcel &data, MessageParcel &reply) { return OnRegisterWallpaperCallback(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_V9),
            [this](MessageParcel &data, MessageParcel &reply) { return OnSetWallpaperV9(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_PIXELMAP_V9),
            [this](MessageParcel &data, MessageParcel &reply) { return OnGetPixelMapV9(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_COLORS_V9),
            [this](MessageParcel &data, MessageParcel &reply) { return OnGetColorsV9(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER_V9),
            [this](MessageParcel &data, MessageParcel &reply) { return OnResetWallpaperV9(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_VIDEO),
            [this](MessageParcel &data, MessageParcel &reply) { return OnSetVideo(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_CUSTOM),
            [this](MessageParcel &data, MessageParcel &reply) { return OnSetCustomWallpaper(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SEND_EVENT),
            [this](MessageParcel &data, MessageParcel &reply) { return OnSendEvent(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_PIXELMAP),
            [this](MessageParcel &data, MessageParcel &reply) { return OnSetWallpaperByPixelMap(data, reply); } },
        { static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_PIXELMAP_V9),
            [this](MessageParcel &data, MessageParcel &reply) { return OnSetWallpaperV9ByPixelMap(data, reply); } },
    };
}

WallpaperServiceStub::~WallpaperServiceStub()
{
}

int32_t WallpaperServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    MemoryGuard cacheGuard;
    std::u16string myDescriptor = WallpaperServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        HILOG_ERROR("Remote descriptor not the same as local descriptor.");
        return E_CHECK_DESCRIPTOR_ERROR;
    }
    return HandleWallpaperRequest(code, data, reply, option);
}

WallpaperRequestHandler WallpaperServiceStub::GetWallpaperRequestHandler(uint32_t code)
{
    auto it = requestHandlers.find(code);
    if (it != requestHandlers.end()) {
        return it->second;
    }
    return nullptr;
}

int32_t WallpaperServiceStub::HandleWallpaperRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    WallpaperRequestHandler handler = GetWallpaperRequestHandler(code);
    if (handler) {
        return handler(data, reply);
    }
    HILOG_ERROR("remote request unhandled: %{public}d", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
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
        HILOG_ERROR("WriteInt32 fail!");
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
        HILOG_ERROR("ReadInt32 fail!");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    auto *rawData = (uint8_t *)data.ReadRawData(vectorPixelMapSize);
    if (rawData == nullptr) {
        HILOG_ERROR("ReadRawData fail!");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    std::vector<uint8_t> VectorPixelMap(rawData, rawData + vectorPixelMapSize);
    int32_t wallpaperType = data.ReadInt32();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = VectorToPixelMap(VectorPixelMap);
    if (pixelMap == nullptr) {
        HILOG_ERROR("VectorToPixelMap fail!");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (isSystemApi) {
        wallpaperErrorCode = SetWallpaperV9ByPixelMap(pixelMap, wallpaperType);
    } else {
        wallpaperErrorCode = SetWallpaperByPixelMap(pixelMap, wallpaperType);
    }
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail!");
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
        HILOG_ERROR("Write int is failed!");
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
        HILOG_ERROR("Write int is failed!");
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
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (isSystemApi) {
        wallpaperErrorCode = GetPixelMapV9(wallpaperType, pixelMap);
    } else {
        wallpaperErrorCode = GetPixelMap(wallpaperType, pixelMap);
    }
    HILOG_INFO(" OnGetPixelMap wallpaperErrorCode = %{public}d", wallpaperErrorCode);
    if (!reply.WriteInt32(static_cast<int32_t>(wallpaperErrorCode))) {
        HILOG_ERROR("WriteInt32 fail!");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperErrorCode == E_OK) {
        if (pixelMap == nullptr) {
            HILOG_ERROR("&pixelMap nullptr!");
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        std::vector<std::uint8_t> value = PixelMapToVector(pixelMap);
        if (!reply.WriteInt32(value.size())) {
            HILOG_ERROR("Failed to WriteInt32!");
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
        if (!reply.WriteRawData(value.data(), value.size())) {
            HILOG_ERROR("Failed to WriteUIntoVector!");
            return IPC_STUB_WRITE_PARCEL_ERR;
        }
    }
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
        HILOG_ERROR("WriteInt32 fail!");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperErrorCode == E_OK) {
        if (!reply.WriteUInt64Vector(vecWallpaperColors)) {
            HILOG_ERROR("WallpaperServiceStub::OnGetColors WriteUInt64Vector error!");
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
        HILOG_ERROR("WriteInt32 fail!");
        if (wallpaperFd > INVALID_FD) {
            close(wallpaperFd);
        }
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    if (wallpaperErrorCode == E_OK && !reply.WriteFileDescriptor(wallpaperFd)) {
        HILOG_ERROR("WriteFileDescriptor fail!");
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
        HILOG_ERROR("Write result data failed!");
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
        HILOG_ERROR("Write result data failed!");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnIsOperationAllowed(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnIsOperationAllowed start.");
    auto bResult = IsOperationAllowed();
    if (!reply.WriteBool(bResult)) {
        HILOG_ERROR("Write result data failed!");
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
        HILOG_ERROR("Write result data failed!");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnWallpaperOn(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOn in.");
    std::string type = data.ReadString();
    if (type.empty()) {
        HILOG_ERROR("OnWallpaperOn type is empty after ipc.");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOG_ERROR("OnWallpaperOn nullptr after ipc.");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    sptr<IWallpaperEventListener> wallpaperListenerProxy = iface_cast<IWallpaperEventListener>(remote);
    ErrorCode errorCode = On(type, std::move(wallpaperListenerProxy));
    int32_t ret = static_cast<int32_t>(errorCode);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed!");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnWallpaperOff(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnWallpaperOff in.");
    std::string type = data.ReadString();
    if (type.empty()) {
        HILOG_ERROR("OnWallpaperOff type is empty after ipc.");
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
        HILOG_ERROR("WriteInt32 failed!");
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
        HILOG_ERROR("Write int is failed!");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

int32_t WallpaperServiceStub::OnRegisterWallpaperCallback(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("WallpaperServiceStub::OnRegisterWallpaperCallback start.");
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    bool status = false;
    if (object == nullptr) {
        HILOG_ERROR("RegisterWallpaperCallback failed!");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    sptr<IWallpaperCallback> callbackProxy = iface_cast<IWallpaperCallback>(object);

    status = RegisterWallpaperCallback(callbackProxy);
    int32_t ret = status ? static_cast<int32_t>(E_OK) : static_cast<int32_t>(E_DEAL_FAILED);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("WriteInt32 failed!");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }
    return ERR_NONE;
}

std::shared_ptr<OHOS::Media::PixelMap> WallpaperServiceStub::VectorToPixelMap(std::vector<std::uint8_t> value)
{
    HILOG_DEBUG("VectorToPixelMap start.!");
    if (value.size() == 0) {
        return nullptr;
    }
    return std::shared_ptr<PixelMap>(PixelMap::DecodeTlv(value));
}

std::vector<std::uint8_t> WallpaperServiceStub::PixelMapToVector(std::shared_ptr<OHOS::Media::PixelMap> pixelMap)
{
    HILOG_DEBUG("PixelMapToVector start.");
    std::vector<std::uint8_t> value;
    if (pixelMap == nullptr || !pixelMap->EncodeTlv(value)) {
        HILOG_ERROR("pixelMap encode failed!");
        value.clear();
        return value;
    }
    return value;
}

} // namespace WallpaperMgrService
} // namespace OHOS