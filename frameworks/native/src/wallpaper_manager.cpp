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

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dfx_types.h"
#include "display_manager.h"
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
constexpr int32_t MIN_TIME = 0;
constexpr int32_t MAX_TIME = 5000;
constexpr int32_t MAX_VIDEO_SIZE = 104857600;
constexpr int32_t MAX_RETRY_TIMES = 10;
constexpr int32_t TIME_INTERVAL = 500000;

using namespace OHOS::Media;

WallpaperManager::WallpaperManager()
{
}
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
    std::lock_guard<std::mutex> lock(wallpaperProxyLock_);
    if (wallpaperProxy_ != nullptr) {
        sptr<IRemoteObject> object = wallpaperProxy_->AsObject();
        if ((object != nullptr) && (remote == object)) {
            object->RemoveDeathRecipient(deathRecipient_);
            wallpaperProxy_ = nullptr;
        }
    }
}

sptr<IWallpaperService> WallpaperManager::GetService()
{
    std::lock_guard<std::mutex> lock(wallpaperProxyLock_);
    if (wallpaperProxy_ != nullptr) {
        return wallpaperProxy_;
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

    wallpaperProxy_ = iface_cast<WallpaperServiceProxy>(object);
    if (wallpaperProxy_ == nullptr) {
        HILOG_ERROR("iface_cast failed");
    }
    return wallpaperProxy_;
}

void WallpaperManager::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DelayedRefSingleton<WallpaperManager>::GetInstance().ResetService(remote);
    int32_t times = 0;
    bool result = false;
    do {
        times++;
        result = DelayedRefSingleton<WallpaperManager>::GetInstance().RegisterWallpaperListener();
        if (result != true) {
            usleep(TIME_INTERVAL);
        }
    } while (result != true && times < MAX_RETRY_TIMES);
    HILOG_INFO("Register WallpaperListener result:%{public}d.", result);
}

template<typename F, typename... Args>
ErrCode WallpaperManager::CallService(F func, Args &&...args)
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

ErrorCode WallpaperManager::GetColors(int32_t wallpaperType, const ApiInfo &apiInfo, std::vector<uint64_t> &colors)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    if (apiInfo.isSystemApi) {
        return wallpaperServerProxy->GetColorsV9(wallpaperType, colors);
    }
    return wallpaperServerProxy->GetColors(wallpaperType, colors);
}

ErrorCode WallpaperManager::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    std::lock_guard<std::mutex> lock(wallpaperFdLock_);
    std::map<int32_t, int32_t>::iterator iter = wallpaperFdMap_.find(wallpaperType);
    if (iter != wallpaperFdMap_.end() && fcntl(iter->second, F_GETFL) != -1) {
        close(iter->second);
        wallpaperFdMap_.erase(iter);
    }
    ErrorCode wallpaperErrorCode = wallpaperServerProxy->GetFile(wallpaperType, wallpaperFd);
    if (wallpaperErrorCode == E_OK && wallpaperFd != -1) {
        wallpaperFdMap_.insert(std::pair<int32_t, int32_t>(wallpaperType, wallpaperFd));
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::SetWallpaper(std::string uri, int32_t wallpaperType, const ApiInfo &apiInfo)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    std::string fileRealPath;
    if (!FileDeal::GetRealPath(uri, fileRealPath)) {
        HILOG_ERROR("get real path file failed, len = %{public}zu", uri.size());
        return E_FILE_ERROR;
    }

    long length = 0;
    ErrorCode wallpaperErrorCode = CheckWallpaperFormat(fileRealPath, false, length);
    if (wallpaperErrorCode != E_OK) {
        HILOG_ERROR("Check wallpaper format failed!");
        return wallpaperErrorCode;
    }

    int32_t fd = open(fileRealPath.c_str(), O_RDONLY, 0660);
    if (fd < 0) {
        HILOG_ERROR("open file failed, errno %{public}d", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_FILE_ERROR;
    }
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wallpaperServerProxy->SetWallpaperV9(fd, wallpaperType, length);
    } else {
        wallpaperErrorCode = wallpaperServerProxy->SetWallpaper(fd, wallpaperType, length);
    }
    close(fd);
    if (wallpaperErrorCode == E_OK) {
        CloseWallpaperFd(wallpaperType);
    }
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::SetWallpaper(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType,
    const ApiInfo &apiInfo)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }

    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wallpaperServerProxy->SetWallpaperV9ByPixelMap(pixelMap, wallpaperType);
    } else {
        wallpaperErrorCode = wallpaperServerProxy->SetWallpaperByPixelMap(pixelMap, wallpaperType);
    }
    if (wallpaperErrorCode == static_cast<int32_t>(E_OK)) {
        CloseWallpaperFd(wallpaperType);
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::SetVideo(const std::string &uri, const int32_t wallpaperType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    std::string fileRealPath;
    if (!FileDeal::GetRealPath(uri, fileRealPath)) {
        HILOG_ERROR("Get real path failed, uri: %{public}s", uri.c_str());
        return E_FILE_ERROR;
    }

    long length = 0;
    ErrorCode wallpaperErrorCode = CheckWallpaperFormat(fileRealPath, true, length);
    if (wallpaperErrorCode != E_OK) {
        HILOG_ERROR("Check wallpaper format failed!");
        return wallpaperErrorCode;
    }
    int32_t fd = open(fileRealPath.c_str(), O_RDONLY, 0660);
    if (fd < 0) {
        HILOG_ERROR("Open file failed, errno %{public}d", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_FILE_ERROR;
    }
    StartAsyncTrace(HITRACE_TAG_MISC, "SetVideo", static_cast<int32_t>(TraceTaskId::SET_VIDEO));
    wallpaperErrorCode = wallpaperServerProxy->SetVideo(fd, wallpaperType, length);
    close(fd);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetVideo", static_cast<int32_t>(TraceTaskId::SET_VIDEO));
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::SetCustomWallpaper(const std::string &uri, const int32_t wallpaperType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    std::string fileRealPath;
    if (!FileDeal::GetRealPath(uri, fileRealPath)) {
        HILOG_ERROR("Get real path failed, uri: %{public}s", uri.c_str());
        return E_FILE_ERROR;
    }
    if (!FileDeal::IsZipFile(uri)) {
        return E_FILE_ERROR;
    }
    long length = 0;
    ErrorCode wallpaperErrorCode = CheckWallpaperFormat(fileRealPath, false, length);
    if (wallpaperErrorCode != E_OK) {
        HILOG_ERROR("Check wallpaper format failed!");
        return wallpaperErrorCode;
    }
    int32_t fd = open(fileRealPath.c_str(), O_RDONLY, 0660);
    if (fd < 0) {
        HILOG_ERROR("Open file failed, errno %{public}d", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_FILE_ERROR;
    }
    StartAsyncTrace(HITRACE_TAG_MISC, "SetCustomWallpaper", static_cast<int32_t>(TraceTaskId::SET_CUSTOM_WALLPAPER));
    wallpaperErrorCode = wallpaperServerProxy->SetCustomWallpaper(fd, wallpaperType, length);
    close(fd);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetCustomWallpaper", static_cast<int32_t>(TraceTaskId::SET_CUSTOM_WALLPAPER));
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::GetPixelMap(int32_t wallpaperType, const ApiInfo &apiInfo,
    std::shared_ptr<OHOS::Media::PixelMap> &pixelMap)
{
    HILOG_INFO("FrameWork GetPixelMap Start by FD");
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_SA_DIED;
    }
    IWallpaperService::FdInfo fdInfo;
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wallpaperServerProxy->GetPixelMapV9(wallpaperType, fdInfo);
    } else {
        wallpaperErrorCode = wallpaperServerProxy->GetPixelMap(wallpaperType, fdInfo);
    }
    if (wallpaperErrorCode != E_OK) {
        return wallpaperErrorCode;
    }
    // current wallpaper is live video, not image
    if (fdInfo.size == 0 && fdInfo.fd == -1) { // 0: empty file size; -1: invalid file description
        pixelMap = nullptr;
        return E_OK;
    }
    uint32_t errorCode = 0;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<OHOS::Media::ImageSource> imageSource =
        OHOS::Media::ImageSource::CreateImageSource(fdInfo.fd, opts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("ImageSource::CreateImageSource failed,errcode= %{public}d", errorCode);
        close(fdInfo.fd);
        return E_IMAGE_ERRCODE;
    }
    close(fdInfo.fd);
    OHOS::Media::DecodeOptions decodeOpts;
    pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);

    if (errorCode != 0) {
        HILOG_ERROR("ImageSource::CreatePixelMap failed,errcode= %{public}d", errorCode);
        return E_IMAGE_ERRCODE;
    }
    return wallpaperErrorCode;
}

int32_t WallpaperManager::GetWallpaperId(int32_t wallpaperType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return -1;
    }
    return wallpaperServerProxy->GetWallpaperId(wallpaperType);
}

ErrorCode WallpaperManager::GetWallpaperMinHeight(const ApiInfo &apiInfo, int32_t &minHeight)
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr");
        return E_DEAL_FAILED;
    }
    minHeight = display->GetHeight();
    return E_OK;
}

ErrorCode WallpaperManager::GetWallpaperMinWidth(const ApiInfo &apiInfo, int32_t &minWidth)
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr");
        return E_DEAL_FAILED;
    }
    minWidth = display->GetWidth();
    return E_OK;
}

bool WallpaperManager::IsChangePermitted()
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }
    return wallpaperServerProxy->IsChangePermitted();
}

bool WallpaperManager::IsOperationAllowed()
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }
    return wallpaperServerProxy->IsOperationAllowed();
}
ErrorCode WallpaperManager::ResetWallpaper(std::int32_t wallpaperType, const ApiInfo &apiInfo)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_SA_DIED;
    }
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = wallpaperServerProxy->ResetWallpaperV9(wallpaperType);
    } else {
        wallpaperErrorCode = wallpaperServerProxy->ResetWallpaper(wallpaperType);
    }
    if (wallpaperErrorCode == E_OK) {
        CloseWallpaperFd(wallpaperType);
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::On(const std::string &type, std::shared_ptr<WallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperManager::On in");
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_SA_DIED;
    }
    if (listener == nullptr) {
        HILOG_ERROR("listener is nullptr.");
        return E_DEAL_FAILED;
    }
    sptr<WallpaperEventListenerClient> ipcListener = new (std::nothrow) WallpaperEventListenerClient(listener);
    if (ipcListener == nullptr) {
        HILOG_ERROR("new WallpaperEventListenerClient failed");
        return E_NO_MEMORY;
    }
    {
        std::lock_guard<std::mutex> lock(listenerMapLock_);
        listenerMap_.insert_or_assign(type, ipcListener);
    }
    return wallpaperServerProxy->On(type, ipcListener);
}

ErrorCode WallpaperManager::Off(const std::string &type, std::shared_ptr<WallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperManager::Off in");
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_SA_DIED;
    }
    sptr<WallpaperEventListenerClient> ipcListener = nullptr;
    if (listener != nullptr) {
        ipcListener = new (std::nothrow) WallpaperEventListenerClient(listener);
        if (ipcListener == nullptr) {
            HILOG_ERROR("new WallpaperEventListenerClient failed");
            return E_NO_MEMORY;
        }
    }
    return wallpaperServerProxy->Off(type, ipcListener);
}

JScallback WallpaperManager::GetCallback()
{
    return callback;
}

void WallpaperManager::SetCallback(JScallback cb)
{
    callback = cb;
}

bool WallpaperManager::RegisterWallpaperCallback(JScallback callback)
{
    HILOG_INFO("  WallpaperManager::RegisterWallpaperCallback statrt");
    SetCallback(callback);
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }

    if (callback == nullptr) {
        HILOG_ERROR("callback is NULL.");
        return false;
    }

    bool status = wallpaperServerProxy->RegisterWallpaperCallback(new WallpaperServiceCbStub());
    if (!status) {
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
    std::lock_guard<std::mutex> lock(wallpaperFdLock_);
    std::map<int32_t, int32_t>::iterator iter = wallpaperFdMap_.find(wallpaperType);
    if (iter != wallpaperFdMap_.end() && fcntl(iter->second, F_GETFL) != -1) {
        close(iter->second);
        wallpaperFdMap_.erase(iter);
    }
}

bool WallpaperManager::RegisterWallpaperListener()
{
    auto service = GetService();
    if (service == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return false;
    }

    std::lock_guard<std::mutex> lock(listenerMapLock_);
    for (const auto &iter : listenerMap_) {
        auto ret = service->On(iter.first, iter.second);
        if (ret != E_OK) {
            HILOG_ERROR("Register WallpaperListener failed type:%{public}s,errcode:%{public}d", iter.first.c_str(),
                ret);
            return false;
        }
    }
    return true;
}
ErrorCode WallpaperManager::SendEvent(const std::string &eventType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed");
        return E_DEAL_FAILED;
    }
    return wallpaperServerProxy->SendEvent(eventType);
}

bool WallpaperManager::CheckVideoFormat(const std::string &fileName)
{
    int32_t videoFd = -1;
    int64_t length = 0;
    if (!OpenFile(fileName, videoFd, length)) {
        HILOG_ERROR("Open file: %{public}s failed.", fileName.c_str());
        return false;
    }
    std::shared_ptr<Media::AVMetadataHelper> helper = Media::AVMetadataHelperFactory::CreateAVMetadataHelper();
    if (helper == nullptr) {
        HILOG_ERROR("Create metadata helper failed!");
        close(videoFd);
        return false;
    }
    int32_t offset = 0;
    int32_t errorCode = helper->SetSource(videoFd, offset, length);
    if (errorCode != 0) {
        HILOG_ERROR("Set helper source failed");
        close(videoFd);
        return false;
    }
    auto metaData = helper->ResolveMetadata();
    if (metaData.find(Media::AV_KEY_MIME_TYPE) != metaData.end()) {
        if (metaData[Media::AV_KEY_MIME_TYPE] != "video/mp4") {
            HILOG_ERROR("Video mime type is not video/mp4!");
            close(videoFd);
            return false;
        }
    } else {
        HILOG_ERROR("Cannot get video mime type!");
        close(videoFd);
        return false;
    }

    if (metaData.find(Media::AV_KEY_DURATION) != metaData.end()) {
        int32_t videoDuration = std::stoi(metaData[Media::AV_KEY_DURATION]);
        if (videoDuration < MIN_TIME || videoDuration > MAX_TIME) {
            HILOG_ERROR("The durations of this vodeo is not between 0s ~ 5s!");
            close(videoFd);
            return false;
        }
    } else {
        HILOG_ERROR("Cannot get the duration of this video!");
        close(videoFd);
        return false;
    }
    close(videoFd);
    return true;
}

bool WallpaperManager::OpenFile(const std::string &fileName, int32_t &fd, int64_t &fileSize)
{
    if (!OHOS::FileExists(fileName)) {
        HILOG_ERROR("File is not exist, file: %{public}s", fileName.c_str());
        return false;
    }

    fd = open(fileName.c_str(), O_RDONLY);
    if (fd <= 0) {
        HILOG_ERROR("Get video fd failed!");
        return false;
    }
    struct stat64 st;
    if (fstat64(fd, &st) != 0) {
        HILOG_ERROR("Failed to fstat64");
        close(fd);
        return false;
    }
    fileSize = static_cast<int64_t>(st.st_size);
    return true;
}

ErrorCode WallpaperManager::CheckWallpaperFormat(const std::string &realPath, bool isLive, long &length)
{
    if (isLive && (FileDeal::GetExtension(realPath) != ".mp4" || !CheckVideoFormat(realPath))) {
        HILOG_ERROR("Check live wallpaper file failed!");
        return E_PARAMETERS_INVALID;
    }

    FILE *file = std::fopen(realPath.c_str(), "rb");
    if (file == nullptr) {
        HILOG_ERROR("Fopen failed, %{public}s, %{public}s", realPath.c_str(), strerror(errno));
        return E_FILE_ERROR;
    }

    int32_t fend = fseek(file, 0, SEEK_END);
    length = ftell(file);
    int32_t fset = fseek(file, 0, SEEK_SET);
    if (length <= 0 || (isLive && length > MAX_VIDEO_SIZE) || fend != 0 || fset != 0) {
        HILOG_ERROR("ftell file failed or fseek file failed, errno %{public}d", errno);
        fclose(file);
        return E_FILE_ERROR;
    }
    fclose(file);
    return E_OK;
}

} // namespace WallpaperMgrService
} // namespace OHOS