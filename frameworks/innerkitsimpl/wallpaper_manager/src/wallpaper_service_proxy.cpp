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
int32_t WallpaperServiceProxy::GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    return GetColorsInner(wallpaperType, GET_COLORS, colors);
}

int32_t WallpaperServiceProxy::GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    return GetColorsInner(wallpaperType, GET_COLORS_V9, colors);
}

int32_t WallpaperServiceProxy::GetColorsInner(int32_t wallpaperType, uint32_t code, std::vector<uint64_t> &colors)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    int32_t result = Remote()->SendRequest(code, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR("Get color failed, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int32_t wallpaperErrorCode = reply.ReadInt32();
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        if (!reply.ReadUInt64Vector(&colors)) {
            HILOG_ERROR(" Failed to ReadUInt64Vector ");
        }
    }
    return wallpaperErrorCode;
}

int32_t WallpaperServiceProxy::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    MessageParcel data;
    MessageParcel reply;
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
    return SetWallpaperByMapInner(fd, wallpaperType, length, SET_WALLPAPER_MAP);
}

int32_t WallpaperServiceProxy::SetWallpaperByMapV9(int32_t fd, int32_t wallpaperType, int32_t length)
{
    return SetWallpaperByMapInner(fd, wallpaperType, length, SET_WALLPAPER_MAP_V9);
}

int32_t WallpaperServiceProxy::SetWallpaperByMapInner(int32_t fd, int32_t wallpaperType, int32_t length, uint32_t code)
{
    HILOG_INFO(" WallpaperServiceProxy::SetWallpaper --> start ");
    MessageParcel data;
    MessageParcel reply;
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

    int32_t result = Remote()->SendRequest(code, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperCalbackProxy::SetWallpaperfail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }

    return reply.ReadInt32();
}

int32_t WallpaperServiceProxy::SetWallpaperByFD(int fd, int wallpaperType, int length)
{
    return SetWallpaperByFDInner(fd, wallpaperType, length, SET_WALLPAPER_URI_FD);
}

int32_t WallpaperServiceProxy::SetWallpaperByFDV9(int32_t fd, int32_t wallpaperType, int32_t length)
{
    return SetWallpaperByFDInner(fd, wallpaperType, length, SET_WALLPAPER_URI_FD_V9);
}

int32_t WallpaperServiceProxy::SetWallpaperByFDInner(int32_t fd, int32_t wallpaperType, int32_t length, uint32_t code)
{
    HILOG_INFO(" SetWallpaperByFD ");
    MessageParcel data;
    MessageParcel reply;
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

    int32_t result = Remote()->SendRequest(code, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperCalbackProxy::SetWallpaperfail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    return reply.ReadInt32();
}

int32_t WallpaperServiceProxy::GetPixelMap(int wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    return GetPixelMapInner(wallpaperType, GET_PIXELMAP, fdInfo);
}

int32_t WallpaperServiceProxy::GetPixelMapV9(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    return GetPixelMapInner(wallpaperType, GET_PIXELMAP_V9, fdInfo);
}

int32_t WallpaperServiceProxy::GetPixelMapInner(int32_t wallpaperType, uint32_t code, IWallpaperService::FdInfo &fdInfo)
{
    HILOG_INFO(" WallpaperServiceProxy::getPixelMap --> start ");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    if (!data.WriteInt32(wallpaperType)) {
        HILOG_ERROR(" Failed to WriteInt32 ");
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int32_t result = Remote()->SendRequest(code, data, reply, option);
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

int32_t WallpaperServiceProxy::GetWallpaperMinHeight(int32_t &minHeight)
{
    return GetWallpaperMinHeightInner(GET_WALLPAPER_MIN_HEIGHT, minHeight);
}

int32_t WallpaperServiceProxy::GetWallpaperMinHeightV9(int32_t &minHeight)
{
    return GetWallpaperMinHeightInner(GET_WALLPAPER_MIN_HEIGHT_V9, minHeight);
}

int32_t WallpaperServiceProxy::GetWallpaperMinHeightInner(uint32_t code, int32_t &minHeight)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    int32_t result = Remote()->SendRequest(code, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinHeight fail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }

    int32_t wallpaperErrorCode = reply.ReadInt32();
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        minHeight = reply.ReadInt32();
    }
    return wallpaperErrorCode;
}

int32_t WallpaperServiceProxy::GetWallpaperMinWidth(int32_t &minWidth)
{
    return GetWallpaperMinWidthInner(GET_WALLPAPER_MIN_WIDTH, minWidth);
}

int32_t WallpaperServiceProxy::GetWallpaperMinWidthV9(int32_t &minWidth)
{
    return GetWallpaperMinWidthInner(GET_WALLPAPER_MIN_WIDTH_V9, minWidth);
}

int32_t WallpaperServiceProxy::GetWallpaperMinWidthInner(uint32_t code, int32_t &minWidth)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    int32_t result = Remote()->SendRequest(code, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::GetWallpaperMinWidth fail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int32_t wallpaperErrorCode = reply.ReadInt32();
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
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

int32_t WallpaperServiceProxy::ResetWallpaper(int wallpaperType)
{
    return ResetWallpaperInner(wallpaperType, RESET_WALLPAPER);
}

int32_t WallpaperServiceProxy::ResetWallpaperV9(int32_t wallpaperType)
{
    return ResetWallpaperInner(wallpaperType, RESET_WALLPAPER_V9);
}

int32_t WallpaperServiceProxy::ResetWallpaperInner(int32_t wallpaperType, uint32_t code)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        HILOG_ERROR(" Failed to write parcelable ");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }

    data.WriteInt32(wallpaperType);
    int32_t result = Remote()->SendRequest(code, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR(" WallpaperServiceProxy::ResetWallpaper fail, result = %{public}d ", result);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    return reply.ReadInt32();
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

    int32_t status = reply.ReadInt32();
    bool ret = (status == static_cast<int32_t>(E_OK)) ? true : false;
    HILOG_DEBUG("WallpaperServiceProxy::On out");
    return ret;
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

    int32_t status = reply.ReadInt32();
    bool ret = (status == static_cast<int32_t>(E_OK)) ? true : false;
    HILOG_DEBUG("WallpaperServiceProxy::Off out");
    return ret;
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

    int32_t status = reply.ReadInt32();
    bool ret = (status == static_cast<int32_t>(E_OK)) ? true : false;
    HILOG_DEBUG("WallpaperServiceProxy::REGISTER_CALLBACK out");
    return ret;
}
} // namespace WallpaperMgrService
} // namespace OHOS
