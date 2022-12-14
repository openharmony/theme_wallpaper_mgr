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
constexpr const int32_t INVALID_FD = -1;
std::vector<uint64_t> WallpaperServiceProxy::GetColors(int wallpaperType)
{
    std::vector<uint64_t> colors;
    MessageParcel data, reply;
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

int32_t WallpaperServiceProxy::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return INVALID_FD;
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return INVALID_FD;
    }

    int32_t result = Remote()->SendRequest(GET_FILE, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" get file result = %{public}d ", result);
        return INVALID_FD;
    }
    int32_t wallpaperErrorCode = reply.ReadInt32();
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        wallpaperFd = reply.ReadFileDescriptor();
    }
    return wallpaperErrorCode;
}

std::string WallpaperServiceProxy::getUrl()
{
    HILOG_INFO("return FWReadUrl= %{public}s ", FWReadUrl.c_str());
    return FWReadUrl;
}
int32_t WallpaperServiceProxy::SetWallpaperByMap(int fd, int wallpaperType, int length)
{
    HILOG_INFO(" SetWallpaperByMap ");
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    if (!data.WriteFileDescriptor(fd)) {
        HILOG_ERROR(" Failed to WriteFileDescriptor ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    if (!data.WriteInt32(length)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    int32_t result = Remote()->SendRequest(SET_WALLPAPER_MAP, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperCallbackProxy::SetWallpaper fail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }

    return reply.ReadInt32();
}
int32_t WallpaperServiceProxy::SetWallpaperByFD(int fd, int wallpaperType, int length)
{
    HILOG_INFO(" SetWallpaperByFD ");
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    if (!data.WriteFileDescriptor(fd)) {
        HILOG_ERROR(" Failed to WriteFileDescriptor ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    if (!data.WriteInt32(length)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    int32_t result = Remote()->SendRequest(SET_WALLPAPER_URI_FD, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperCallbackProxy::SetWallpaperfail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    return reply.ReadInt32();
}

int32_t WallpaperServiceProxy::GetPixelMap(int wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    HILOG_INFO(" WallpaperServiceProxy::getPixelMap --> start ");
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int32_t result = Remote()->SendRequest(GET_PIXELMAP, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetPixelMap fail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int32_t wallpaperErrorCode = reply.ReadInt32();
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        fdInfo.size = reply.ReadInt32();
        fdInfo.fd = reply.ReadFileDescriptor();
    }
    return wallpaperErrorCode;
}

int WallpaperServiceProxy::GetWallpaperId(int wallpaperType)
{
    int iWallpaperId = 1;
    MessageParcel data, reply;
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
int WallpaperServiceProxy::GetWallpaperMinHeight()
{
    int iWallpaperMinHeight = 0;
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return 0;
    }

    int32_t result = Remote()->SendRequest(GET_WALLPAPER_MIN_HEIGHT, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinHeight fail, result = %{public}d ", result);
        return 0;
    }

    iWallpaperMinHeight = reply.ReadInt32();
    HILOG_INFO(" End => iWallpaperMinHeight[%{public}d]", iWallpaperMinHeight);
    return iWallpaperMinHeight;
}

int WallpaperServiceProxy::GetWallpaperMinWidth()
{
    int iWallpaperMinWidth = 0;
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return 0;
    }

    int32_t result = Remote()->SendRequest(GET_WALLPAPER_MIN_WIDTH, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinWidth fail, result = %{public}d ", result);
        return 0;
    }

    iWallpaperMinWidth = reply.ReadInt32();
    HILOG_INFO(" End => iWallpaperMinWidth[%{public}d]", iWallpaperMinWidth);
    return iWallpaperMinWidth;
}

bool WallpaperServiceProxy::IsChangePermitted()
{
    bool bFlag = false;
    MessageParcel data, reply;
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
    MessageParcel data, reply;
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

int32_t WallpaperServiceProxy::ResetWallpaper(int wallpaperType)
{
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    data.WriteInt32(wallpaperType);
    int32_t result = Remote()->SendRequest(RESET_WALLPAPER, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::ResetWallpaper fail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    return reply.ReadInt32();
}

bool WallpaperServiceProxy::On(sptr<IWallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperServiceProxy::On in");
    MessageParcel data, reply;
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

    int32_t status = reply.ReadInt32();
    bool ret = (status == static_cast<int32_t>(E_OK)) ? true : false;
    HILOG_DEBUG("WallpaperServiceProxy::On out");
    return ret;
}

bool WallpaperServiceProxy::Off(sptr<IWallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperServiceProxy::Off in");
    MessageParcel data, reply;
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

    int32_t status = reply.ReadInt32();
    bool ret = (status == static_cast<int32_t>(E_OK)) ? true : false;
    HILOG_DEBUG("WallpaperServiceProxy::Off out");
    return ret;
}

bool WallpaperServiceProxy::RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback)
{
    HILOG_DEBUG("WallpaperServiceProxy::RegisterWallpaperCallback in");
    MessageParcel data, reply;
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

    int32_t status = reply.ReadInt32();
    bool ret = (status == static_cast<int32_t>(E_OK)) ? true : false;
    HILOG_DEBUG("WallpaperServiceProxy::REGISTER_CALLBACK out");
    return ret;
}
} // namespace WallpaperMgrService
} // namespace OHOS
