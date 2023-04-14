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
#include "wallpaper_manager.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

#include "dfx_types.h"
#include "file_deal.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "i_wallpaper_service.h"
#include "if_system_ability_manager.h"
#include "image_packer.h"
#include "image_source.h"
#include "image_type.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "wallpaper_service_cb_stub.h"
#include "wallpaper_service_proxy.h"

namespace OHOS {
using namespace MiscServices;
namespace WallpaperMgrService {
constexpr int OPTION_QUALITY = 100;
WallpaperManager::WallpaperManager() {}
WallpaperManager::~WallpaperManager()
{
    std::map<int32_t, int32_t>::iterator iter = wallpaperFdMap_.begin();
    while (iter != wallpaperFdMap_.end()) {
        close(iter->second);
        ++iter;
    }
}

void WallpaperManager::ResetService(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("Remote is dead, reset service instance");
    std::lock_guard<std::mutex> lock(wpProxyLock_);
    if (wpProxy_ != nullptr) {
        sptr<IRemoteObject> object = wpProxy_->AsObject();
        if ((object != nullptr) && (remote == object)) {
            object->RemoveDeathRecipient(deathRecipient_);
            wpProxy_ = nullptr;
        }
    }
}

sptr<IWallpaperService> WallpaperManager::GetService()
{
    std::lock_guard<std::mutex> lock(wpProxyLock_);
    if (wpProxy_ != nullptr) {
        return wpProxy_;
    }

    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOG_ERROR("Get samgr failed");
        return nullptr;
    }
    sptr<IRemoteObject> object = samgr->GetSystemAbility(WALLPAPER_MANAGER_SERVICE_ID);
    if (object == nullptr) {
        HILOG_ERROR("Get wallpaper object from samgr failed");
        return nullptr;
    }

    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new DeathRecipient();
    }

    if ((object->IsProxyObject()) && (!object->AddDeathRecipient(deathRecipient_))) {
        HILOG_ERROR("Failed to add death recipient");
    }

    HILOG_INFO("get remote object ok");
    wpProxy_ = iface_cast<WallpaperServiceProxy>(object);
    if (wpProxy_ == nullptr) {
        HILOG_ERROR("iface_cast failed");
    }
    return wpProxy_;
}

void WallpaperManager::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DelayedRefSingleton<WallpaperManager>::GetInstance().ResetService(remote);
}

template<typename F, typename... Args> ErrCode WallpaperManager::CallService(F func, Args &&...args)
{
    auto service = GetService();
    if (service == nullptr) {
        HILOG_ERROR("get service failed");
        return ERR_DEAD_OBJECT;
    }

    ErrCode result = (service->*func)(std::forward<Args>(args)...);
    if (SUCCEEDED(result)) {
        return ERR_OK;
    }

    // Reset service instance if 'ERR_DEAD_OBJECT' happened.
    if (result == ERR_DEAD_OBJECT) {
        ResetService(service);
    }

    HILOG_ERROR("Callservice failed with: %{public}d", result);
    return result;
}

int32_t WallpaperManager::GetColors(int32_t wallpaperType, const ApiInfo &apiInfo, std::vector<uint64_t> &colors)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    if (apiInfo.isSystemApi) {
        return wpServerProxy->GetColorsV9(wallpaperType, colors);
    }
    return wpServerProxy->GetColors(wallpaperType, colors);
}

int32_t WallpaperManager::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return -1;
    }
    std::lock_guard<std::mutex> lock(wpFdLock_);
    std::map<int32_t, int32_t>::iterator iter = wallpaperFdMap_.find(wallpaperType);
    if (iter != wallpaperFdMap_.end() && fcntl(iter->second, F_GETFL) != -1) {
        close(iter->second);
        wallpaperFdMap_.erase(iter);
    }
    int32_t wallpaperErrorCode = wpServerProxy->GetFile(wallpaperType, wallpaperFd);
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK) && wallpaperFd != -1) {
        wallpaperFdMap_.insert(std::pair<int32_t, int32_t>(wallpaperType, wallpaperFd));
    }
    return wallpaperErrorCode;
}

int32_t WallpaperManager::SetWallpaper(std::string url, int32_t wallpaperType, const ApiInfo &apiInfo)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    std::string fileRealPath;
    if (!GetRealPath(url, fileRealPath)) {
        HILOG_ERROR("get real path file failed, len = %{public}zu", url.size());
        return static_cast<int32_t>(E_FILE_ERROR);
    }
    FILE *pixMap = std::fopen(fileRealPath.c_str(), "rb");
    if (pixMap == nullptr) {
        HILOG_ERROR("fopen faild, %{public}s, %{public}s", fileRealPath.c_str(), strerror(errno));
        return static_cast<int32_t>(E_FILE_ERROR);
    }
    int32_t fend = fseek(pixMap, 0, SEEK_END);
    int32_t length = ftell(pixMap);
    int32_t fset = fseek(pixMap, 0, SEEK_SET);
    if (length <= 0 || fend != 0 || fset != 0) {
        HILOG_ERROR("ftell file failed or fseek file failed, errno %{public}d", errno);
        fclose(pixMap);
        return static_cast<int32_t>(E_FILE_ERROR);
    }
    fclose(pixMap);
    int fd = open(fileRealPath.c_str(), O_RDONLY, 0660);
    if (fd < 0) {
        HILOG_ERROR("open file failed, errno %{public}d", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return static_cast<int32_t>(E_FILE_ERROR);
    }
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    int32_t wallpaperErrorCode = 0;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wpServerProxy->SetWallpaperByFDV9(fd, wallpaperType, length);
    } else {
        wallpaperErrorCode = wpServerProxy->SetWallpaperByFD(fd, wallpaperType, length);
    }
    close(fd);
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        CloseWallpaperFd(wallpaperType);
    }
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    return wallpaperErrorCode;
}

int32_t WallpaperManager::SetWallpaper(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType,
    const ApiInfo &apiInfo)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return static_cast<int32_t>(E_DEAL_FAILED);
    }

    std::stringbuf stringBuf;
    std::ostream ostream(&stringBuf);
    int mapSize = WritePixelMapToStream(ostream, pixelMap);
    if (mapSize <= 0) {
        HILOG_ERROR("WritePixelMapToStream faild");
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    char *buffer = new (std::nothrow) char[mapSize]();
    if (buffer == nullptr) {
        return static_cast<int32_t>(E_NO_MEMORY);
    }
    stringBuf.sgetn(buffer, mapSize);

    int fd[2];
    pipe(fd);
    fcntl(fd[1], F_SETPIPE_SZ, mapSize);
    fcntl(fd[0], F_SETPIPE_SZ, mapSize);
    int32_t writeSize = write(fd[1], buffer, mapSize);
    if (writeSize != mapSize) {
        HILOG_ERROR("Write file failed, errno %{public}d", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        delete[] buffer;
        return static_cast<int32_t>(E_WRITE_PARCEL_ERROR);
    }
    close(fd[1]);
    int32_t wallpaperErrorCode = 0;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wpServerProxy->SetWallpaperByMapV9(fd[0], wallpaperType, mapSize);
    } else {
        wallpaperErrorCode = wpServerProxy->SetWallpaperByMap(fd[0], wallpaperType, mapSize);
    }
    close(fd[0]);
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        CloseWallpaperFd(wallpaperType);
    }
    delete[] buffer;
    return wallpaperErrorCode;
}

int64_t WallpaperManager::WritePixelMapToStream(std::ostream &outputStream,
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap)
{
    OHOS::Media::ImagePacker imagePacker;
    OHOS::Media::PackOption option;
    option.format = "image/jpeg";
    option.quality = OPTION_QUALITY;
    option.numberHint = 1;
    std::set<std::string> formats;
    uint32_t ret = imagePacker.GetSupportedFormats(formats);
    if (ret != 0) {
        HILOG_ERROR("image packer get supported format failed, ret=%{public}u.", ret);
    }

    imagePacker.StartPacking(outputStream, option);
    imagePacker.AddImage(*pixelMap);
    int64_t packedSize = 0;
    imagePacker.FinalizePacking(packedSize);
    HILOG_INFO("FrameWork WritePixelMapToStream End! packedSize=%{public}lld.", static_cast<long long>(packedSize));
    return packedSize;
}

int32_t WallpaperManager::GetPixelMap(int32_t wallpaperType, const ApiInfo &apiInfo,
    std::shared_ptr<OHOS::Media::PixelMap> &pixelMap)
{
    HILOG_INFO("FrameWork GetPixelMap Start by FD");
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return static_cast<int32_t>(E_SA_DIED);
    }
    IWallpaperService::FdInfo fdInfo;
    int32_t wallpaperErrorCode = 0;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wpServerProxy->GetPixelMapV9(wallpaperType, fdInfo);
    } else {
        wallpaperErrorCode = wpServerProxy->GetPixelMap(wallpaperType, fdInfo);
    }
    if (wallpaperErrorCode != static_cast<int32_t>(E_OK)) {
        return wallpaperErrorCode;
    }
    uint32_t errorCode = 0;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";
    HILOG_INFO(" CreateImageSource by FD");
    std::unique_ptr<OHOS::Media::ImageSource> imageSource =
        OHOS::Media::ImageSource::CreateImageSource(fdInfo.fd, opts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("ImageSource::CreateImageSource failed,errcode= %{public}d", errorCode);
        return static_cast<int32_t>(E_IMAGE_ERRCODE);
    }
    OHOS::Media::DecodeOptions decodeOpts;
    HILOG_INFO(" CreatePixelMap");
    pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);

    if (errorCode != 0) {
        HILOG_ERROR("ImageSource::CreatePixelMap failed,errcode= %{public}d", errorCode);
        return static_cast<int32_t>(E_IMAGE_ERRCODE);
    }
    close(fdInfo.fd);
    return wallpaperErrorCode;
}

int WallpaperManager::GetWallpaperId(int wallpaperType)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return -1;
    }
    return wpServerProxy->GetWallpaperId(wallpaperType);
}

int32_t WallpaperManager::GetWallpaperMinHeight(const ApiInfo &apiInfo, int32_t &minHeight)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    if (apiInfo.isSystemApi) {
        return wpServerProxy->GetWallpaperMinHeightV9(minHeight);
    }
    return wpServerProxy->GetWallpaperMinHeight(minHeight);
}

int32_t WallpaperManager::GetWallpaperMinWidth(const ApiInfo &apiInfo, int32_t &minWidth)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    if (apiInfo.isSystemApi) {
        return wpServerProxy->GetWallpaperMinWidthV9(minWidth);
    }
    return wpServerProxy->GetWallpaperMinWidth(minWidth);
}

bool WallpaperManager::IsChangePermitted()
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }
    return wpServerProxy->IsChangePermitted();
}

bool WallpaperManager::IsOperationAllowed()
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }
    return wpServerProxy->IsOperationAllowed();
}

int32_t WallpaperManager::ResetWallpaper(std::int32_t wallpaperType, const ApiInfo &apiInfo)
{
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return static_cast<int32_t>(E_SA_DIED);
    }
    int32_t wallpaperErrorCode = 0;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wpServerProxy->ResetWallpaperV9(wallpaperType);
    } else {
        wallpaperErrorCode = wpServerProxy->ResetWallpaper(wallpaperType);
    }
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        CloseWallpaperFd(wallpaperType);
    }
    return wallpaperErrorCode;
}

bool WallpaperManager::On(const std::string &type, std::shared_ptr<WallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperManager::On in");
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }
    if (listener == nullptr) {
        HILOG_ERROR("listener is nullptr.");
        return false;
    }
    std::lock_guard<std::mutex> lck(listenerMapMutex_);

    sptr<WallpaperColorChangeListenerClient> ipcListener = new (std::nothrow)
        WallpaperColorChangeListenerClient(listener);
    if (ipcListener == nullptr) {
        HILOG_ERROR("new WallpaperColorChangeListenerClient failed");
        return false;
    }
    HILOG_DEBUG("WallpaperManager::On out");
    return wpServerProxy->On(ipcListener);
}

bool WallpaperManager::Off(const std::string &type, std::shared_ptr<WallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperManager::Off in");
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }
    std::lock_guard<std::mutex> lck(listenerMapMutex_);
    bool status = false;
    if (listener != nullptr) {
        sptr<WallpaperColorChangeListenerClient> ipcListener = new (std::nothrow)
            WallpaperColorChangeListenerClient(listener);
        if (ipcListener == nullptr) {
            HILOG_ERROR("new WallpaperColorChangeListenerClient failed");
            return false;
        }
        status = wpServerProxy->Off(ipcListener);
    } else {
        status = wpServerProxy->Off(nullptr);
    }
    if (status == false) {
        HILOG_ERROR("off failed");
        return false;
    }
    HILOG_DEBUG("WallpaperManager::Off out");
    return true;
}

JScallback WallpaperManager::GetCallback()
{
    return callback;
}

void WallpaperManager::SetCallback(bool (*cb)(int))
{
    callback = cb;
}

bool WallpaperManager::RegisterWallpaperCallback(bool (*callback)(int))
{
    HILOG_ERROR("  WallpaperManager::RegisterWallpaperCallback statrt");
    SetCallback(callback);
    auto wpServerProxy = GetService();
    if (wpServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }

    if (callback == nullptr) {
        HILOG_ERROR("callback is NULL.");
        return false;
    }
    HILOG_INFO("  WallpaperManager::RegisterWallpaperCallback");

    bool status = wpServerProxy->RegisterWallpaperCallback(new WallpaperServiceCbStub());
    if (status == false) {
        HILOG_ERROR("off failed code=%d.", ERR_NONE);
        return false;
    }

    return 0;
}

void WallpaperManager::ReporterFault(FaultType faultType, FaultCode faultCode)
{
    MiscServices::FaultMsg msg;
    msg.faultType = faultType;
    msg.errorCode = faultCode;
    FaultReporter::ReportRuntimeFault(msg);
}

void WallpaperManager::CloseWallpaperFd(int32_t wallpaperType)
{
    std::lock_guard<std::mutex> lock(wpFdLock_);
    std::map<int32_t, int32_t>::iterator iter = wallpaperFdMap_.find(wallpaperType);
    if (iter != wallpaperFdMap_.end() && fcntl(iter->second, F_GETFL) != -1) {
        close(iter->second);
        wallpaperFdMap_.erase(iter);
    }
}

bool WallpaperManager::GetRealPath(const std::string &inOriPath, std::string &outRealPath)
{
    char realPath[PATH_MAX + 1] = { 0x00 };
    if (inOriPath.size() > PATH_MAX || realpath(inOriPath.c_str(), realPath) == nullptr) {
        HILOG_ERROR("get real path fail");
        return false;
    }
    outRealPath = std::string(realPath);
    if (!OHOS::FileExists(outRealPath)) {
        HILOG_ERROR("real path file is not exist! %{public}s", outRealPath.c_str());
        return false;
    }
    return true;
}
} // namespace WallpaperMgrService
} // namespace OHOS