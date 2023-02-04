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
std::vector<uint64_t> WallpaperServiceProxy::GetColors(int32_t wallpaperType)
{
    std::vector<uint64_t> colors;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return colors;
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return colors;
    }

    int32_t result = Remote()->SendRequest(GET_COLORS, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" get colors result = %{public}d ", result);
        return colors;
    }

    if (!reply.ReadUInt64Vector(&colors)) {
        HILOG_ERROR(" Failed to ReadUInt64Vector ");
    }
    return colors;
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

    int32_t result = Remote()->SendRequest(GET_FILE, data, reply, option);
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

std::string WallpaperServiceProxy::GetUri()
{
    HILOG_INFO("return serviceReadUri = %{public}s ", serviceReadUri.c_str());
    return serviceReadUri;
}
ErrorCode WallpaperServiceProxy::SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length)
{
    HILOG_INFO(" SetWallpaperByMap ");
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

    int32_t result = Remote()->SendRequest(SET_WALLPAPER, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperCallbackProxy::SetWallpaper fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }

    return ConvertIntToErrorCode(reply.ReadInt32());
}

ErrorCode WallpaperServiceProxy::GetPixelMap(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
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
    int32_t result = Remote()->SendRequest(GET_PIXELMAP, data, reply, option);
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
    int32_t result = Remote()->SendRequest(GET_WALLPAPER_ID, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperId fail, result = %{public}d ", result);
        return -1;
    }

    iWallpaperId = reply.ReadInt32();
    HILOG_INFO(" End => iWallpaperId[%{public}d]", iWallpaperId);
    return iWallpaperId;
}
int32_t WallpaperServiceProxy::GetWallpaperMinHeight()
{
    int32_t iWallpaperMinHeight = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return -1;
    }

    int32_t result = Remote()->SendRequest(GET_WALLPAPER_MIN_HEIGHT, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinHeight fail, result = %{public}d ", result);
        return -1;
    }

    iWallpaperMinHeight = reply.ReadInt32();
    HILOG_INFO(" End => iWallpaperMinHeight[%{public}d]", iWallpaperMinHeight);
    return iWallpaperMinHeight;
}

int32_t WallpaperServiceProxy::GetWallpaperMinWidth()
{
    int32_t iWallpaperMinWidth = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return -1;
    }

    int32_t result = Remote()->SendRequest(GET_WALLPAPER_MIN_WIDTH, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinWidth fail, result = %{public}d ", result);
        return -1;
    }

    iWallpaperMinWidth = reply.ReadInt32();
    HILOG_INFO(" End => iWallpaperMinWidth[%{public}d]", iWallpaperMinWidth);
    return iWallpaperMinWidth;
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

    int32_t result = Remote()->SendRequest(IS_CHANGE_PERMITTED, data, reply, option);
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

    int32_t result = Remote()->SendRequest(IS_OPERATION_ALLOWED, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxyIsOperationAllowed fail, result = %{public}d ", result);
        return false;
    }

    bFlag = reply.ReadBool();
    return bFlag;
}

ErrorCode WallpaperServiceProxy::ResetWallpaper(int32_t wallpaperType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return E_WRITE_PARCEL_ERROR;
    }

    data.WriteInt32(wallpaperType);
    int32_t result = Remote()->SendRequest(RESET_WALLPAPER, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::ResetWallpaper fail, result = %{public}d ", result);
        return E_DEAL_FAILED;
    }
    return ConvertIntToErrorCode(reply.ReadInt32());
}

bool WallpaperServiceProxy::On(sptr<IWallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperServiceProxy::On in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return false;
    }
    if (listener == nullptr) {
        HILOG_ERROR("listener is nullptr");
        return false;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("write subscribe type or parcel failed.");
        return false;
    }
    int32_t result = Remote()->SendRequest(ON, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::On fail, result = %{public}d ", result);
        return false;
    }

    HILOG_DEBUG("WallpaperServiceProxy::On out");
    return reply.ReadBool();
}

bool WallpaperServiceProxy::Off(sptr<IWallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperServiceProxy::Off in");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return false;
    }
    if (listener != nullptr) {
        if (!data.WriteRemoteObject(listener->AsObject())) {
            HILOG_ERROR("write subscribe type or parcel failed.");
            return false;
        }
    }

    int32_t result = Remote()->SendRequest(OFF, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::Off fail, result = %{public}d ", result);
        return false;
    }

    HILOG_DEBUG("WallpaperServiceProxy::Off out");
    return reply.ReadBool();
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
    int32_t result = Remote()->SendRequest(REGISTER_CALLBACK, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::REGISTER_CALLBACK fail, result = %{public}d ", result);
        return false;
    }

    HILOG_DEBUG("WallpaperServiceProxy::REGISTER_CALLBACK out");
    return reply.ReadBool();
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
            wallpaperErrorCode = static_cast<ErrorCode>(errorCode);
            break;
        default:
            break;
    }
    return wallpaperErrorCode;
}
} // namespace WallpaperMgrService
} // namespace OHOS
