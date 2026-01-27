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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

#include "dfx_types.h"
#include "display_manager.h"
#include "file_deal.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "image_packer.h"
#include "image_source.h"
#include "image_type.h"
#include "iservice_registry.h"
#include "iwallpaper_service.h"
#include "system_ability_definition.h"
#include "wallpaper_manager.h"
#include "wallpaper_picture_info_by_parcel.h"
#include "wallpaper_rawdata.h"
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
constexpr int32_t BASE_NUMBER = 10;
constexpr int32_t LOAD_TIME = 4;
constexpr mode_t MODE = 0660;

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
WallpaperManager &WallpaperManager::GetInstance()
{
    static WallpaperManager wallpaperManager;
    return wallpaperManager;
}

WallpaperManager::DeathRecipient::DeathRecipient()
{
}
WallpaperManager::DeathRecipient::~DeathRecipient()
{
}
void WallpaperManager::ResetService(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("Remote is dead, reset service instance.");
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
        HILOG_ERROR("Get samgr failed!");
        return nullptr;
    }
    sptr<IRemoteObject> object = samgr->CheckSystemAbility(WALLPAPER_MANAGER_SERVICE_ID);
    if (object == nullptr) {
        HILOG_ERROR("Check wallpaper object from samgr failed!");
        object = samgr->LoadSystemAbility(WALLPAPER_MANAGER_SERVICE_ID, LOAD_TIME);
        if (object == nullptr) {
            HILOG_ERROR("Load samgr failed!");
            return nullptr;
        }
    }
    
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new DeathRecipient();
    }

    if ((object->IsProxyObject()) && (!object->AddDeathRecipient(deathRecipient_))) {
        HILOG_ERROR("Failed to add death recipient!");
    }

    wallpaperProxy_ = iface_cast<IWallpaperService>(object);
    if (wallpaperProxy_ == nullptr) {
        HILOG_ERROR("iface_cast failed!");
    }
    return wallpaperProxy_;
}

void WallpaperManager::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    WallpaperManager::GetInstance().ResetService(remote);
    int32_t times = 0;
    bool result = false;
    do {
        times++;
        result = WallpaperManager::GetInstance().RegisterWallpaperListener();
        if (result != true) {
            usleep(TIME_INTERVAL);
        }
    } while (result != true && times < MAX_RETRY_TIMES);
    HILOG_INFO("Register WallpaperListener result:%{public}d.", result);
}

template<typename F, typename... Args> ErrCode WallpaperManager::CallService(F func, Args &&...args)
{
    auto service = GetService();
    if (service == nullptr) {
        HILOG_ERROR("get service failed!");
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

    HILOG_ERROR("Callservice failed with: %{public}d.", result);
    return result;
}

ErrorCode WallpaperManager::GetColors(int32_t wallpaperType, const ApiInfo &apiInfo, std::vector<uint64_t> &colors)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }
    if (apiInfo.isSystemApi) {
        return ConvertIntToErrorCode(wallpaperServerProxy->GetColorsV9(wallpaperType, colors));
    }
    return ConvertIntToErrorCode(wallpaperServerProxy->GetColors(wallpaperType, colors));
}

ErrorCode WallpaperManager::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }
    std::lock_guard<std::mutex> lock(wallpaperFdLock_);
    std::map<int32_t, int32_t>::iterator iter = wallpaperFdMap_.find(wallpaperType);
    if (iter != wallpaperFdMap_.end() && fcntl(iter->second, F_GETFL) != -1) {
        close(iter->second);
        wallpaperFdMap_.erase(iter);
    }
    ErrorCode wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->GetFile(wallpaperType, wallpaperFd));
    if (wallpaperErrorCode == E_OK && wallpaperFd != -1) {
        wallpaperFdMap_.insert(std::pair<int32_t, int32_t>(wallpaperType, wallpaperFd));
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::SetWallpaper(std::string uri, int32_t wallpaperType, const ApiInfo &apiInfo)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }
    std::string fileRealPath;
    if (!FileDeal::GetRealPath(uri, fileRealPath)) {
        HILOG_ERROR("get real path file failed, len = %{public}zu.", uri.size());
        return E_PARAMETERS_INVALID;
    }

    long length = 0;
    ErrorCode wallpaperErrorCode = CheckWallpaperFormat(fileRealPath, false, length);
    if (wallpaperErrorCode != E_OK) {
        HILOG_ERROR("Check wallpaper format failed!");
        return wallpaperErrorCode;
    }

    int32_t fd = open(fileRealPath.c_str(), O_RDONLY, MODE);
    if (fd < 0) {
        HILOG_ERROR("open file failed, errno %{public}d!", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_FILE_ERROR;
    }
    fdsan_exchange_owner_tag(fd, 0, WP_DOMAIN);
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->SetWallpaperV9(fd, wallpaperType, length));
    } else {
        wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->SetWallpaper(fd, wallpaperType, length));
    }
    fdsan_close_with_tag(fd, WP_DOMAIN);
    if (wallpaperErrorCode == E_OK) {
        CloseWallpaperFd(wallpaperType);
    }
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::SetWallpaper(
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType, const ApiInfo &apiInfo)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }

    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (pixelMap == nullptr) {
        HILOG_ERROR("pixelMap is nullptr!");
        return E_DEAL_FAILED;
    }
    std::vector<std::uint8_t> value;
    if (!pixelMap->EncodeTlv(value)) {
        HILOG_ERROR("pixelMap encode failed!");
        return E_DEAL_FAILED;
    }
    WallpaperRawData wallpaperRawData;
    wallpaperRawData.size = value.size();
    wallpaperRawData.data = value.data();
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode =
            ConvertIntToErrorCode(wallpaperServerProxy->SetWallpaperV9ByPixelMap(wallpaperRawData, wallpaperType));
    } else {
        wallpaperErrorCode =
            ConvertIntToErrorCode(wallpaperServerProxy->SetWallpaperByPixelMap(wallpaperRawData, wallpaperType));
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
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }
    std::string fileRealPath;
    if (!FileDeal::GetRealPath(uri, fileRealPath)) {
        HILOG_ERROR("Get real path failed, uri: %{public}s!", uri.c_str());
        return E_PARAMETERS_INVALID;
    }

    long length = 0;
    ErrorCode wallpaperErrorCode = CheckWallpaperFormat(fileRealPath, true, length);
    if (wallpaperErrorCode != E_OK) {
        HILOG_ERROR("Check wallpaper format failed!");
        return wallpaperErrorCode;
    }
    int32_t fd = open(fileRealPath.c_str(), O_RDONLY, MODE);
    if (fd < 0) {
        HILOG_ERROR("Open file failed, errno %{public}d!", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_FILE_ERROR;
    }
    fdsan_exchange_owner_tag(fd, 0, WP_DOMAIN);
    StartAsyncTrace(HITRACE_TAG_MISC, "SetVideo", static_cast<int32_t>(TraceTaskId::SET_VIDEO));
    wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->SetVideo(fd, wallpaperType, length));
    fdsan_close_with_tag(fd, WP_DOMAIN);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetVideo", static_cast<int32_t>(TraceTaskId::SET_VIDEO));
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::SetCustomWallpaper(const std::string &uri, const int32_t wallpaperType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }
    std::string fileRealPath;
    if (!FileDeal::GetRealPath(uri, fileRealPath)) {
        HILOG_ERROR("Get real path failed, uri: %{public}s!", uri.c_str());
        return E_PARAMETERS_INVALID;
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
    int32_t fd = open(fileRealPath.c_str(), O_RDONLY, MODE);
    if (fd < 0) {
        HILOG_ERROR("Open file failed, errno %{public}d!", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_FILE_ERROR;
    }
    fdsan_exchange_owner_tag(fd, 0, WP_DOMAIN);
    StartAsyncTrace(HITRACE_TAG_MISC, "SetCustomWallpaper", static_cast<int32_t>(TraceTaskId::SET_CUSTOM_WALLPAPER));
    wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->SetCustomWallpaper(fd, wallpaperType, length));
    fdsan_close_with_tag(fd, WP_DOMAIN);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetCustomWallpaper", static_cast<int32_t>(TraceTaskId::SET_CUSTOM_WALLPAPER));
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::GetPixelMap(
    int32_t wallpaperType, const ApiInfo &apiInfo, std::shared_ptr<OHOS::Media::PixelMap> &pixelMap)
{
    HILOG_INFO("FrameWork GetPixelMap Start by FD.");
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_SA_DIED;
    }
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    int32_t size = 0;
    int32_t fd = -1;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->GetPixelMapV9(wallpaperType, size, fd));
    } else {
        wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->GetPixelMap(wallpaperType, size, fd));
    }
    if (wallpaperErrorCode != E_OK) {
        return wallpaperErrorCode;
    }
    // current wallpaper is live video, not image
    if (size == 0 && fd == -1) { // 0: empty file size; -1: invalid file description
        pixelMap = nullptr;
        return E_OK;
    }
    wallpaperErrorCode = CreatePixelMapByFd(fd, size, pixelMap);
    if (wallpaperErrorCode != E_OK) {
        pixelMap = nullptr;
        return wallpaperErrorCode;
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::CreatePixelMapByFd(
    int32_t fd, int32_t size, std::shared_ptr<OHOS::Media::PixelMap> &pixelMap)
{
    if (size <= 0 || size > MAX_VIDEO_SIZE || fd < 0) {
        HILOG_ERROR("Size or fd error!");
        return E_IMAGE_ERRCODE;
    }
    uint8_t *buffer = new uint8_t[size];
    ssize_t bytesRead = read(fd, buffer, size);
    if (bytesRead < 0) {
        HILOG_ERROR("Read fd to buffer fail!");
        delete[] buffer;
        close(fd);
        return E_IMAGE_ERRCODE;
    }
    uint32_t errorCode = 0;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<OHOS::Media::ImageSource> imageSource =
        OHOS::Media::ImageSource::CreateImageSource(buffer, size, opts, errorCode);
    if (errorCode != 0 || imageSource == nullptr) {
        HILOG_ERROR("ImageSource::CreateImageSource failed, errcode= %{public}d!", errorCode);
        delete[] buffer;
        close(fd);
        return E_IMAGE_ERRCODE;
    }
    OHOS::Media::DecodeOptions decodeOpts;
    pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("ImageSource::CreatePixelMap failed, errcode= %{public}d!", errorCode);
        delete[] buffer;
        close(fd);
        return E_IMAGE_ERRCODE;
    }
    delete[] buffer;
    close(fd);
    return E_OK;
}

int32_t WallpaperManager::GetWallpaperId(int32_t wallpaperType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return -1;
    }
    return wallpaperServerProxy->GetWallpaperId(wallpaperType);
}

ErrorCode WallpaperManager::GetWallpaperMinHeight(const ApiInfo &apiInfo, int32_t &minHeight)
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr.");
        return E_DEAL_FAILED;
    }
    minHeight = display->GetHeight();
    return E_OK;
}

ErrorCode WallpaperManager::GetWallpaperMinWidth(const ApiInfo &apiInfo, int32_t &minWidth)
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr.");
        return E_DEAL_FAILED;
    }
    minWidth = display->GetWidth();
    return E_OK;
}

bool WallpaperManager::IsChangePermitted()
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return false;
    }
    bool isChangePermitted = false;
    wallpaperServerProxy->IsChangePermitted(isChangePermitted);
    return isChangePermitted;
}

bool WallpaperManager::IsOperationAllowed()
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return false;
    }
    bool isOperationAllowed = false;
    wallpaperServerProxy->IsOperationAllowed(isOperationAllowed);
    return isOperationAllowed;
}
ErrorCode WallpaperManager::ResetWallpaper(std::int32_t wallpaperType, const ApiInfo &apiInfo)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_SA_DIED;
    }
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (apiInfo.isSystemApi) {
        wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->ResetWallpaperV9(wallpaperType));
    } else {
        wallpaperErrorCode = ConvertIntToErrorCode(wallpaperServerProxy->ResetWallpaper(wallpaperType));
    }
    if (wallpaperErrorCode == E_OK) {
        CloseWallpaperFd(wallpaperType);
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::On(const std::string &type, std::shared_ptr<WallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperManager::On in.");
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_SA_DIED;
    }
    if (listener == nullptr) {
        HILOG_ERROR("listener is nullptr.");
        return E_DEAL_FAILED;
    }
    sptr<WallpaperEventListenerClient> ipcListener = new (std::nothrow) WallpaperEventListenerClient(listener);
    if (ipcListener == nullptr) {
        HILOG_ERROR("new WallpaperEventListenerClient failed!");
        return E_NO_MEMORY;
    }
    {
        std::lock_guard<std::mutex> lock(listenerMapLock_);
        listenerMap_.insert_or_assign(type, ipcListener);
    }
    return ConvertIntToErrorCode(wallpaperServerProxy->On(type, ipcListener));
}

ErrorCode WallpaperManager::Off(const std::string &type, std::shared_ptr<WallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperManager::Off in.");
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_SA_DIED;
    }
    sptr<WallpaperEventListenerClient> ipcListener = nullptr;
    ipcListener = new (std::nothrow) WallpaperEventListenerClient(listener);
    if (ipcListener == nullptr) {
        HILOG_ERROR("new WallpaperEventListenerClient failed!");
        return E_NO_MEMORY;
    }
    return ConvertIntToErrorCode(wallpaperServerProxy->Off(type, ipcListener));
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
    HILOG_INFO("  WallpaperManager::RegisterWallpaperCallback statrt.");
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
    bool registerWallpaperCallback = false;
    wallpaperServerProxy->RegisterWallpaperCallback(new WallpaperServiceCbStub(), registerWallpaperCallback);
    if (!registerWallpaperCallback) {
        HILOG_ERROR("off failed code=%d.", ERR_NONE);
        return false;
    }
    return registerWallpaperCallback;
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
        HILOG_ERROR("Get proxy failed!");
        return false;
    }

    std::lock_guard<std::mutex> lock(listenerMapLock_);
    for (const auto &iter : listenerMap_) {
        auto ret = ConvertIntToErrorCode(service->On(iter.first, iter.second));
        if (ret != E_OK) {
            HILOG_ERROR(
                "Register WallpaperListener failed type:%{public}s, errcode:%{public}d", iter.first.c_str(), ret);
            return false;
        }
    }
    return true;
}
ErrorCode WallpaperManager::SendEvent(const std::string &eventType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }
    return ConvertIntToErrorCode(wallpaperServerProxy->SendEvent(eventType));
}

bool WallpaperManager::CheckVideoFormat(const std::string &fileName)
{
    int32_t fd = -1;
    int64_t length = 0;
    FILE *wallpaperFile = OpenFile(fileName, fd, length);
    if (wallpaperFile == nullptr) {
        HILOG_ERROR("Open file: %{public}s failed!", fileName.c_str());
        return false;
    }
    std::shared_ptr<Media::AVMetadataHelper> helper = Media::AVMetadataHelperFactory::CreateAVMetadataHelper();
    if (helper == nullptr) {
        HILOG_ERROR("Create metadata helper failed!");
        fclose(wallpaperFile);
        return false;
    }
    int32_t offset = 0;
    int32_t errorCode = helper->SetSource(fd, offset, length);
    if (errorCode != 0) {
        HILOG_ERROR("Set helper source failed!");
        fclose(wallpaperFile);
        return false;
    }
    auto metaData = helper->ResolveMetadata();
    if (metaData.find(Media::AV_KEY_MIME_TYPE) != metaData.end()) {
        if (metaData[Media::AV_KEY_MIME_TYPE] != "video/mp4") {
            HILOG_ERROR("Video mime type is not video/mp4!");
            fclose(wallpaperFile);
            return false;
        }
    } else {
        HILOG_ERROR("Cannot get video mime type!");
        fclose(wallpaperFile);
        return false;
    }

    if (metaData.find(Media::AV_KEY_DURATION) != metaData.end()) {
        int32_t videoDuration = ConverString2Int(metaData[Media::AV_KEY_DURATION]);
        if (videoDuration < MIN_TIME || videoDuration > MAX_TIME) {
            HILOG_ERROR("The durations of this vodeo is not between 0s ~ 5s!");
            fclose(wallpaperFile);
            return false;
        }
    } else {
        HILOG_ERROR("Cannot get the duration of this video!");
        fclose(wallpaperFile);
        return false;
    }
    fclose(wallpaperFile);
    return true;
}

FILE *WallpaperManager::OpenFile(const std::string &fileName, int &fd, int64_t &fileSize)
{
    if (!OHOS::FileExists(fileName)) {
        HILOG_ERROR("File is not exist, file: %{public}s.", fileName.c_str());
        return nullptr;
    }

    FILE *file = fopen(fileName.c_str(), "r");
    if (file == nullptr) {
        HILOG_ERROR("Get video file failed!");
        return nullptr;
    }
    fd = fileno(file);
    if (fd < 0) {
        HILOG_ERROR("Get video videoFd failed!");
        fclose(file);
        return nullptr;
    }
    struct stat64 st;
    if (fstat64(fd, &st) != 0) {
        HILOG_ERROR("Failed to fstat64!");
        fclose(file);
        return nullptr;
    }
    fileSize = static_cast<int64_t>(st.st_size);
    return file;
}

ErrorCode WallpaperManager::CheckWallpaperFormat(const std::string &realPath, bool isLive, long &length)
{
    if (isLive && (FileDeal::GetExtension(realPath) != ".mp4" || !CheckVideoFormat(realPath))) {
        HILOG_ERROR("Check live wallpaper file failed!");
        return E_PARAMETERS_INVALID;
    }

    FILE *file = std::fopen(realPath.c_str(), "rb");
    if (file == nullptr) {
        HILOG_ERROR("Fopen failed, %{public}s, %{public}s!", realPath.c_str(), strerror(errno));
        return E_FILE_ERROR;
    }

    int32_t fend = fseek(file, 0, SEEK_END);
    length = ftell(file);
    int32_t fset = fseek(file, 0, SEEK_SET);
    if (length <= 0 || (isLive && length > MAX_VIDEO_SIZE) || fend != 0 || fset != 0) {
        HILOG_ERROR("ftell file failed or fseek file failed, errno %{public}d!", errno);
        fclose(file);
        return E_FILE_ERROR;
    }
    fclose(file);
    return E_OK;
}

ErrorCode WallpaperManager::SetAllWallpapers(std::vector<WallpaperInfo> allWallpaperInfos, int32_t wallpaperType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_DEAL_FAILED;
    }
    WallpaperPictureInfoByParcel wallpaperPictureInfoByParcel;
    WallpaperPictureInfo wallpaperPictureInfo;
    std::vector<int32_t> fdVector;
    ErrorCode wallpaperCode;
    for (const auto &wallpaperInfo : allWallpaperInfos) {
        std::string fileRealPath;
        if (!FileDeal::GetRealPath(wallpaperInfo.source, fileRealPath)) {
            HILOG_ERROR("get real path file failed, len = %{public}zu.", wallpaperInfo.source.size());
            return E_PARAMETERS_INVALID;
        }
        wallpaperCode = GetFdByPath(wallpaperInfo, wallpaperPictureInfo, fileRealPath);
        if (wallpaperCode != E_OK) {
            CloseWallpaperInfoFd(wallpaperPictureInfoByParcel.wallpaperPictureInfo_);
            HILOG_ERROR("PathConvertFd failed");
            return wallpaperCode;
        }
        wallpaperPictureInfoByParcel.wallpaperPictureInfo_.push_back(wallpaperPictureInfo);
        fdVector.push_back(wallpaperPictureInfo.fd);
    }

    StartAsyncTrace(HITRACE_TAG_MISC, "SetAllWallpapers", static_cast<int32_t>(TraceTaskId::SET_ALL_WALLPAPERS));
    ErrorCode wallpaperErrorCode = ConvertIntToErrorCode(
        wallpaperServerProxy->SetAllWallpapers(wallpaperPictureInfoByParcel, wallpaperType, fdVector));
    if (wallpaperErrorCode == E_OK) {
        CloseWallpaperFd(wallpaperType);
    }
    CloseWallpaperInfoFd(wallpaperPictureInfoByParcel.wallpaperPictureInfo_);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetAllWallpapers", static_cast<int32_t>(TraceTaskId::SET_ALL_WALLPAPERS));
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::GetFdByPath(
    const WallpaperInfo &wallpaperInfo, WallpaperPictureInfo &wallpaperPictureInfo, std::string fileRealPath)
{
    wallpaperPictureInfo.foldState = wallpaperInfo.foldState;
    wallpaperPictureInfo.rotateState = wallpaperInfo.rotateState;
    wallpaperPictureInfo.fd = open(fileRealPath.c_str(), O_RDONLY, MODE);
    if (wallpaperPictureInfo.fd < 0) {
        HILOG_ERROR("open file failed, errno %{public}d!", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_FILE_ERROR;
    }
    fdsan_exchange_owner_tag(wallpaperPictureInfo.fd, 0, WP_DOMAIN);
    ErrorCode wallpaperErrorCode = CheckWallpaperFormat(fileRealPath, false, wallpaperPictureInfo.length);
    if (wallpaperErrorCode != E_OK) {
        HILOG_ERROR("Check wallpaper format failed!");
        return wallpaperErrorCode;
    }
    uint32_t errorCode = 0;
    OHOS::Media::SourceOptions opts;
    std::unique_ptr<OHOS::Media::ImageSource> imageSource =
        OHOS::Media::ImageSource::CreateImageSource(fileRealPath, opts, errorCode);
    if (errorCode != 0 || imageSource == nullptr) {
        HILOG_ERROR("CreateImageSource failed!");
        return E_PARAMETERS_INVALID;
    }
    ImageInfo imageInfo;
    if (imageSource->GetImageInfo(imageInfo) != 0) {
        HILOG_ERROR("GetImageInfo failed!");
        return E_PARAMETERS_INVALID;
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperManager::GetCorrespondWallpaper(
    int32_t wallpaperType, int32_t foldState, int32_t rotateState, std::shared_ptr<OHOS::Media::PixelMap> &pixelMap)
{
    HILOG_INFO("GetCorrespondWallpaper start.");
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return E_SA_DIED;
    }
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    int32_t size = 0;
    int32_t fd = -1;
    wallpaperErrorCode = ConvertIntToErrorCode(
        wallpaperServerProxy->GetCorrespondWallpaper(wallpaperType, foldState, rotateState, size, fd));
    if (wallpaperErrorCode != E_OK) {
        return wallpaperErrorCode;
    }
    // current wallpaper is live video, not image
    if (size == 0 && fd == -1) { // 0: empty file size; -1: invalid file description
        pixelMap = nullptr;
        return E_OK;
    }
    wallpaperErrorCode = CreatePixelMapByFd(fd, size, pixelMap);
    if (wallpaperErrorCode != E_OK) {
        pixelMap = nullptr;
        return wallpaperErrorCode;
    }
    return wallpaperErrorCode;
}

void WallpaperManager::CloseWallpaperInfoFd(std::vector<WallpaperPictureInfo> wallpaperPictureInfos)
{
    for (auto &wallpaperInfo : wallpaperPictureInfos) {
        if (wallpaperInfo.fd >= 0) {
            fdsan_close_with_tag(wallpaperInfo.fd, WP_DOMAIN);
        }
    }
}

bool WallpaperManager::IsDefaultWallpaperResource(int32_t userId, int32_t wallpaperType)
{
    auto wallpaperServerProxy = GetService();
    if (wallpaperServerProxy == nullptr) {
        HILOG_ERROR("Get proxy failed!");
        return false;
    }
    bool isDefaultWallpaperResource = false;
    wallpaperServerProxy->IsDefaultWallpaperResource(userId, wallpaperType, isDefaultWallpaperResource);
    return isDefaultWallpaperResource;
}

int32_t WallpaperManager::ConverString2Int(const std::string &value)
{
    int32_t result = -1;
    if (!value.empty() && std::all_of(value.begin(), value.end(), ::isdigit)) {
        char *endPtr = nullptr;
        result = strtol(value.c_str(), &endPtr, BASE_NUMBER);
        if (*endPtr != '\0') {
            return -1;
        }
    }
    return result;
}

std::unordered_map<int32_t, ErrorCode> errorCodeMap = {
    { static_cast<int32_t>(NO_ERROR), E_OK },
    { static_cast<int32_t>(E_OK), E_OK },
    { static_cast<int32_t>(E_SA_DIED), E_SA_DIED },
    { static_cast<int32_t>(E_READ_PARCEL_ERROR), E_READ_PARCEL_ERROR },
    { static_cast<int32_t>(E_WRITE_PARCEL_ERROR), E_WRITE_PARCEL_ERROR },
    { static_cast<int32_t>(E_PUBLISH_FAIL), E_PUBLISH_FAIL },
    { static_cast<int32_t>(E_TRANSACT_ERROR), E_TRANSACT_ERROR },
    { static_cast<int32_t>(E_DEAL_FAILED), E_DEAL_FAILED },
    { static_cast<int32_t>(E_PARAMETERS_INVALID), E_PARAMETERS_INVALID },
    { static_cast<int32_t>(E_SET_RTC_FAILED), E_SET_RTC_FAILED },
    { static_cast<int32_t>(E_NOT_FOUND), E_NOT_FOUND },
    { static_cast<int32_t>(E_NO_PERMISSION), E_NO_PERMISSION },
    { static_cast<int32_t>(E_FILE_ERROR), E_FILE_ERROR },
    { static_cast<int32_t>(E_IMAGE_ERRCODE), E_IMAGE_ERRCODE },
    { static_cast<int32_t>(E_NO_MEMORY), E_NO_MEMORY },
    { static_cast<int32_t>(E_NOT_SYSTEM_APP), E_NOT_SYSTEM_APP },
    { static_cast<int32_t>(E_USER_IDENTITY_ERROR), E_USER_IDENTITY_ERROR },
    { static_cast<int32_t>(E_CHECK_DESCRIPTOR_ERROR), E_CHECK_DESCRIPTOR_ERROR },
};

ErrorCode WallpaperManager::ConvertIntToErrorCode(int32_t errorCode)
{
    auto it = errorCodeMap.find(errorCode);
    if (it == errorCodeMap.end()) {
        return E_UNKNOWN;
    }
    return it->second;
}

} // namespace WallpaperMgrService
} // namespace OHOS