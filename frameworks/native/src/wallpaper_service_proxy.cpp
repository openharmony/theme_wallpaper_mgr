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

#include "wallpaper_service_proxy.h"

#include "file_deal.h"
#include "hilog_wrapper.h"
#include "i_wallpaper_service.h"
#include "iremote_broker.h"
#include "wallpaper_common.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace OHOS::HiviewDFX;
ErrorCode WallpaperServiceProxy::GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    return GetColorsInner(wallpaperType, WallpaperServiceIpcInterfaceCode::GET_COLORS, colors);
}

ErrorCode WallpaperServiceProxy::GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    return GetColorsInner(wallpaperType, WallpaperServiceIpcInterfaceCode::GET_COLORS_V9, colors);
}

ErrorCode WallpaperServiceProxy::GetColorsInner(
    int32_t wallpaperType, WallpaperServiceIpcInterfaceCode code, std::vector<uint64_t> &colors)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return E_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR("Get color failed, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }
    ErrorCode wallpaperErrorCode = ConvertIntToErrorCode(reply.ReadInt32());
    if (wallpaperErrorCode == E_OK) {
        if (!reply.ReadUInt64Vector(&colors)) {
            HILOG_ERROR(" Failed to ReadUInt64Vector ");
        }
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperServiceProxy::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return E_WRITE_PARCEL_ERROR;
    }

    int32_t result =
        Remote()->SendRequest(static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_FILE), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" get file result = %{public}d ", result);
        return E_DEAL_FAILED;
    }
    ErrorCode wallpaperErrorCode = ConvertIntToErrorCode(reply.ReadInt32());
    if (wallpaperErrorCode == E_OK) {
        wallpaperFd = reply.ReadFileDescriptor();
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperServiceProxy::SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length)
{
    return SetWallpaperInner(fd, wallpaperType, length, WallpaperServiceIpcInterfaceCode::SET_WALLPAPER);
}

ErrorCode WallpaperServiceProxy::SetWallpaperV9(int32_t fd, int32_t wallpaperType, int32_t length)
{
    return SetWallpaperInner(fd, wallpaperType, length, WallpaperServiceIpcInterfaceCode::SET_WALLPAPER_V9);
}

ErrorCode WallpaperServiceProxy::SetWallpaperInner(
    int32_t fd, int32_t wallpaperType, int32_t length, WallpaperServiceIpcInterfaceCode code)
{
    HILOG_INFO(" WallpaperServiceProxy::SetWallpaper --> start ");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteFileDescriptor(fd)) {
        HILOG_ERROR(" Failed to WriteFileDescriptor ");
        return E_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteInt32(length)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return E_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperCallbackProxy::SetWallpaper fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }

    return ConvertIntToErrorCode(reply.ReadInt32());
}

ErrorCode WallpaperServiceProxy::GetPixelMap(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    return GetPixelMapInner(wallpaperType, WallpaperServiceIpcInterfaceCode::GET_PIXELMAP, fdInfo);
}

ErrorCode WallpaperServiceProxy::GetPixelMapV9(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    return GetPixelMapInner(wallpaperType, WallpaperServiceIpcInterfaceCode::GET_PIXELMAP_V9, fdInfo);
}

ErrorCode WallpaperServiceProxy::SetVideo(int32_t fd, int32_t wallpaperType, int32_t length)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR("Failed to write parcelable");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteFileDescriptor(fd)) {
        HILOG_ERROR("Failed to WriteFileDescriptor");
        return E_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR("Failed to WriteInt32");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteInt32(length)) {
        HILOG_ERROR("Failed to WriteInt32");
        return E_WRITE_PARCEL_ERROR;
    }

    int32_t result =
        Remote()->SendRequest(static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_VIDEO), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR("Set video failed, result: %{public}d", result);
        return E_DEAL_FAILED;
    }

    return ConvertIntToErrorCode(reply.ReadInt32());
}

ErrorCode WallpaperServiceProxy::SetCustomWallpaper(const std::string &uri, int32_t wallpaperType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR("Failed to write parcelable");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteString(uri)) {
        HILOG_ERROR("Failed to WriteString");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR("Failed to WriteInt32");
        return E_WRITE_PARCEL_ERROR;
    }
    int32_t result = Remote()->SendRequest(
        static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SET_CUSTOM), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR("Set video failed, result: %{public}d", result);
        return E_DEAL_FAILED;
    }
    return ConvertIntToErrorCode(reply.ReadInt32());
}

ErrorCode WallpaperServiceProxy::GetPixelMapInner(
    int32_t wallpaperType, WallpaperServiceIpcInterfaceCode code, IWallpaperService::FdInfo &fdInfo)
{
    HILOG_INFO(" WallpaperServiceProxy::getPixelMap --> start ");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return E_DEAL_FAILED;
    }
    int32_t result = Remote()->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetPixelMap fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }
    ErrorCode wallpaperErrorCode = ConvertIntToErrorCode(reply.ReadInt32());
    if (wallpaperErrorCode == E_OK) {
        fdInfo.size = reply.ReadInt32();
        fdInfo.fd = reply.ReadFileDescriptor();
    }
    return wallpaperErrorCode;
}

int32_t WallpaperServiceProxy::GetWallpaperId(int32_t wallpaperType)
{
    int32_t iWallpaperId = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return false;
    }

    data.WriteInt32(wallpaperType);
    int32_t result = Remote()->SendRequest(
        static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_ID), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperId fail, result = %{public}d ", result);
        return -1;
    }

    iWallpaperId = reply.ReadInt32();
    HILOG_INFO(" End => iWallpaperId[%{public}d]", iWallpaperId);
    return iWallpaperId;
}

ErrorCode WallpaperServiceProxy::GetWallpaperMinHeight(int32_t &minHeight)
{
    return GetWallpaperMinHeightInner(WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_MIN_HEIGHT, minHeight);
}

ErrorCode WallpaperServiceProxy::GetWallpaperMinHeightV9(int32_t &minHeight)
{
    return GetWallpaperMinHeightInner(WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_MIN_HEIGHT_V9, minHeight);
}

ErrorCode WallpaperServiceProxy::GetWallpaperMinHeightInner(WallpaperServiceIpcInterfaceCode code, int32_t &minHeight)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinHeight fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }

    ErrorCode wallpaperErrorCode = ConvertIntToErrorCode(reply.ReadInt32());
    if (wallpaperErrorCode == E_OK) {
        minHeight = reply.ReadInt32();
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperServiceProxy::GetWallpaperMinWidth(int32_t &minWidth)
{
    return GetWallpaperMinWidthInner(WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_MIN_WIDTH, minWidth);
}

ErrorCode WallpaperServiceProxy::GetWallpaperMinWidthV9(int32_t &minWidth)
{
    return GetWallpaperMinWidthInner(WallpaperServiceIpcInterfaceCode::GET_WALLPAPER_MIN_WIDTH_V9, minWidth);
}

ErrorCode WallpaperServiceProxy::GetWallpaperMinWidthInner(WallpaperServiceIpcInterfaceCode code, int32_t &minWidth)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinWidth fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }
    ErrorCode wallpaperErrorCode = ConvertIntToErrorCode(reply.ReadInt32());
    if (wallpaperErrorCode == E_OK) {
        minWidth = reply.ReadInt32();
    }
    return wallpaperErrorCode;
}

bool WallpaperServiceProxy::IsChangePermitted()
{
    bool bFlag = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return false;
    }

    int32_t result = Remote()->SendRequest(
        static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::IS_CHANGE_PERMITTED), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::IsChangePermitted fail, result = %{public}d ", result);
        return false;
    }

    bFlag = reply.ReadBool();
    return bFlag;
}

bool WallpaperServiceProxy::IsOperationAllowed()
{
    bool bFlag = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return false;
    }

    int32_t result = Remote()->SendRequest(
        static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::IS_OPERATION_ALLOWED), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxyIsOperationAllowed fail, result = %{public}d ", result);
        return false;
    }

    bFlag = reply.ReadBool();
    return bFlag;
}

ErrorCode WallpaperServiceProxy::ResetWallpaper(int32_t wallpaperType)
{
    return ResetWallpaperInner(wallpaperType, WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER);
}

ErrorCode WallpaperServiceProxy::ResetWallpaperV9(int32_t wallpaperType)
{
    return ResetWallpaperInner(wallpaperType, WallpaperServiceIpcInterfaceCode::RESET_WALLPAPER_V9);
}

ErrorCode WallpaperServiceProxy::ResetWallpaperInner(int32_t wallpaperType, WallpaperServiceIpcInterfaceCode code)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }

    data.WriteInt32(wallpaperType);
    int32_t result = Remote()->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::ResetWallpaper fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }
    return ConvertIntToErrorCode(reply.ReadInt32());
}

ErrorCode WallpaperServiceProxy::SendEvent(const std::string &eventType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR("Failed to write parcelable");
        return E_WRITE_PARCEL_ERROR;
    }

    if (!data.WriteString(eventType)) {
        HILOG_ERROR("write eventType failed.");
        return E_WRITE_PARCEL_ERROR;
    }

    int32_t result = Remote()->SendRequest(
        static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::SEND_EVENT), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR("Failed to send event, result: %{public}d", result);
        return E_DEAL_FAILED;
    }
    return ConvertIntToErrorCode(reply.ReadInt32());
}

ErrorCode WallpaperServiceProxy::On(const std::string &type, sptr<IWallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperServiceProxy::On in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }
    if (listener == nullptr) {
        HILOG_ERROR("listener is nullptr");
        return E_DEAL_FAILED;
    }

    if (!data.WriteString(type)) {
        HILOG_ERROR("write type failed.");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("write subscribe type or parcel failed.");
        return E_WRITE_PARCEL_ERROR;
    }
    int32_t result =
        Remote()->SendRequest(static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::ON), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::On fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }

    HILOG_DEBUG("WallpaperServiceProxy::On out");
    return ConvertIntToErrorCode(reply.ReadInt32());
}

ErrorCode WallpaperServiceProxy::Off(const std::string &type, sptr<IWallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperServiceProxy::Off in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }
    if (!data.WriteString(type)) {
        HILOG_ERROR("write type failed.");
        return E_WRITE_PARCEL_ERROR;
    }
    if (listener != nullptr) {
        if (!data.WriteRemoteObject(listener->AsObject())) {
            HILOG_ERROR("write subscribe type or parcel failed.");
            return E_WRITE_PARCEL_ERROR;
        }
    }

    int32_t result =
        Remote()->SendRequest(static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::OFF), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::Off fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }

    HILOG_DEBUG("WallpaperServiceProxy::Off out");
    return ConvertIntToErrorCode(reply.ReadInt32());
}

bool WallpaperServiceProxy::RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback)
{
    HILOG_DEBUG("WallpaperServiceProxy::RegisterWallpaperCallback in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return false;
    }
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr");
        return false;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("write subscribe type or parcel failed.");
        return false;
    }
    HILOG_INFO("  Remote()->SendRequest");
    int32_t result = Remote()->SendRequest(
        static_cast<uint32_t>(WallpaperServiceIpcInterfaceCode::REGISTER_CALLBACK), data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::REGISTER_CALLBACK fail, result = %{public}d ", result);
        return false;
    }
    int32_t status = reply.ReadInt32();
    bool ret = status == static_cast<int32_t>(E_OK);
    HILOG_DEBUG("WallpaperServiceProxy::REGISTER_CALLBACK out");
    return ret;
}

ErrorCode WallpaperServiceProxy::ConvertIntToErrorCode(int32_t errorCode)
{
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    switch (errorCode) {
        case static_cast<int32_t>(E_OK):
        case static_cast<int32_t>(E_SA_DIED):
        case static_cast<int32_t>(E_READ_PARCEL_ERROR):
        case static_cast<int32_t>(E_WRITE_PARCEL_ERROR):
        case static_cast<int32_t>(E_PUBLISH_FAIL):
        case static_cast<int32_t>(E_TRANSACT_ERROR):
        case static_cast<int32_t>(E_DEAL_FAILED):
        case static_cast<int32_t>(E_PARAMETERS_INVALID):
        case static_cast<int32_t>(E_SET_RTC_FAILED):
        case static_cast<int32_t>(E_NOT_FOUND):
        case static_cast<int32_t>(E_NO_PERMISSION):
        case static_cast<int32_t>(E_FILE_ERROR):
        case static_cast<int32_t>(E_IMAGE_ERRCODE):
        case static_cast<int32_t>(E_NO_MEMORY):
        case static_cast<int32_t>(E_NOT_SYSTEM_APP):
        case static_cast<int32_t>(E_USER_IDENTITY_ERROR):
            wallpaperErrorCode = static_cast<ErrorCode>(errorCode);
            break;
        default:
            break;
    }
    return wallpaperErrorCode;
}
} // namespace WallpaperMgrService
} // namespace OHOS
