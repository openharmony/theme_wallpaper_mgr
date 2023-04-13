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
#include "wallpaper_service.h"

#include <display_type.h>
#include <fcntl.h>
#include <rs_surface_node.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <window_manager.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>

#include "ability_manager_client.h"
#include "accesstoken_adapter.h"
#include "bundle_mgr_proxy.h"
#include "canvas.h"
#include "color_picker.h"
#include "command.h"
#include "directory_ex.h"
#include "dump_helper.h"
#include "effect_errors.h"
#include "export/color.h"
#include "file_deal.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "image/bitmap.h"
#include "image_packer.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "iservice_registry.h"
#include "pen.h"
#include "pixel_map.h"
#include "surface.h"
#include "system_ability_definition.h"
#include "wallpaper_common.h"
#include "wallpaper_common_event.h"
#include "wallpaper_service_cb_proxy.h"
#include "wallpaper_extension_ability_death_recipient.h"
#include "window.h"
#include "memory_guard.h"

namespace OHOS {
namespace WallpaperMgrService {
REGISTER_SYSTEM_ABILITY_BY_ID(WallpaperService, WALLPAPER_MANAGER_SERVICE_ID, true);

using namespace OHOS::Media;
using namespace OHOS::MiscServices;

const std::string WallpaperService::WALLPAPER = "wallpaper_orig";
const std::string WallpaperService::WALLPAPER_CROP = "wallpaper";
const std::string WallpaperService::WALLPAPER_LOCK_ORIG = "wallpaper_lock_orig";
const std::string WallpaperService::WALLPAPER_LOCK_CROP = "wallpaper_lock";
const std::string WallpaperService::WALLPAPER_BUNDLE_NAME = "com.ohos.launcher";

const std::int64_t INIT_INTERVAL = 10000L;
const std::int64_t DELAY_TIME = 1000L;
constexpr int HALF = 2;
constexpr int32_t CONNECT_EXTENSION_INTERVAL = 500000;
constexpr int32_t CONNECT_EXTENSION_MAX_RETRY_TIMES = 360;
constexpr int HUNDRED = 100;
constexpr int FOO_MAX_LEN = 52428800;
constexpr int MAX_RETRY_TIMES = 20;
constexpr int32_t DEFAULT_WALLPAPER_ID = -1;
constexpr int32_t FILE_PERMISSION = 0440;
std::mutex WallpaperService::instanceLock_;

sptr<WallpaperService> WallpaperService::instance_;

std::shared_ptr<AppExecFwk::EventHandler> WallpaperService::serviceHandler_;

WallpaperService::WallpaperService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), state_(ServiceRunningState::STATE_NOT_START)
{
}

WallpaperService::WallpaperService() : state_(ServiceRunningState::STATE_NOT_START)
{
}

WallpaperService::~WallpaperService()
{
}

sptr<WallpaperService> WallpaperService::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new WallpaperService();
        }
    }
    return instance_;
}

int32_t WallpaperService::Init()
{
    bool ret = Publish(this);
    if (!ret) {
        HILOG_ERROR("Publish failed.");
        ReporterFault(FaultType::SERVICE_FAULT, FaultCode::SF_SERVICE_UNAVAIABLE);
        return -1;
    }
    HILOG_INFO("Publish success.");
    state_ = ServiceRunningState::STATE_RUNNING;
    return E_OK;
}

void WallpaperService::OnStart()
{
    HILOG_INFO("Enter OnStart.");
    MemoryGuard cacheGuard;
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOG_ERROR("WallpaperService is already running.");
        return;
    }
    InitData();
    InitServiceHandler();
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    std::thread(&WallpaperService::StartWallpaperExtension, this).detach();
    int uid = static_cast<int>(IPCSkeleton::GetCallingUid());
    auto cmd = std::make_shared<Command>(std::vector<std::string>({ "-all" }), "Show all",
        [this, uid](const std::vector<std::string> &input, std::string &output) -> bool {
            int32_t height = 0;
            int32_t width = 0;
            GetWallpaperMinHeight(height);
            GetWallpaperMinWidth(width);
            std::string bundleName(WALLPAPER_BUNDLE_NAME);
            WPGetBundleNameByUid(uid, bundleName);
            output.append("height\t\t\t: " + std::to_string(height) + "\n")
                .append("width\t\t\t: " + std::to_string(width) + "\n")
                .append("WallpaperExtension\t: ExtensionInfo{" + bundleName + "}\n");
            return true;
        });
    DumpHelper::GetInstance().RegisterCommand(cmd);
    StatisticReporter::StartTimerThread();
    if (Init() != 0) {
        auto callback = [=]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        HILOG_ERROR("Init failed. Try again 10s later");
        return;
    }
    return;
}

void WallpaperService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    HILOG_INFO("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        int times = 0;
        RegisterSubscriber(times);
    }
}

void WallpaperService::RegisterSubscriber(int times)
{
    MemoryGuard cacheGuard;
    times++;
    bool subRes = WallpaperCommonEvent::RegisterSubscriber();
    if (subRes == false && times <= MAX_RETRY_TIMES) {
        HILOG_INFO("RegisterSubscriber failed");
        auto callback = [this, times]() { RegisterSubscriber(times); };
        serviceHandler_->PostTask(callback, DELAY_TIME);
    }
}

void WallpaperService::InitServiceHandler()
{
    HILOG_INFO("InitServiceHandler started.");
    if (serviceHandler_ != nullptr) {
        HILOG_ERROR("InitServiceHandler already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create("WallpaperService");
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);

    HILOG_INFO("InitServiceHandler succeeded.");
}

void WallpaperService::OnStop()
{
    HILOG_INFO("OnStop started.");
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return;
    }
    serviceHandler_ = nullptr;
    recipient_ = nullptr;
    state_ = ServiceRunningState::STATE_NOT_START;
    HILOG_INFO("OnStop end.");
}

void WallpaperService::InitData()
{
    HILOG_INFO("WallpaperService::initData --> start ");
    userId_ = 0;
    wallpaperId_ = DEFAULT_WALLPAPER_ID;
    wallpaperMap_.Clear();
    lockWallpaperMap_.Clear();
    userId_ = GetUserId();
    std::string userIdPath = GetWallpaperDir();
    this->wallpaperLockScreenFilePath_ = userIdPath + "/" + WALLPAPER_LOCKSCREEN_DIRNAME;
    this->wallpaperSystemFilePath_ = userIdPath + "/" + WALLPAPER_SYSTEM_DIRNAME;
    wallpaperLockScreenFileFullPath_ = wallpaperLockScreenFilePath_ + "/" + WALLPAPER_LOCK_ORIG;
    wallpaperLockScreenCropFileFullPath_ = wallpaperLockScreenFilePath_ + "/" + WALLPAPER_LOCK_CROP;
    wallpaperSystemCropFileFullPath_ = wallpaperSystemFilePath_ + "/" + WALLPAPER_CROP;
    wallpaperSystemFileFullPath_ = wallpaperSystemFilePath_ + "/" + WALLPAPER;
    wallpaperTmpFullPath_ = wallpaperSystemFilePath_ + "/" + WALLPAPER_TMP_DIRNAME;
    wallpaperCropPath = wallpaperSystemFilePath_ + "/" + WALLPAPER_CROP_PICTURE;
    LoadSettingsLocked(userId_, true);
    SaveColor(WALLPAPER_SYSTEM);
    SaveColor(WALLPAPER_LOCKSCREEN);
    systemWallpaperColor_ = 0;
    lockWallpaperColor_ = 0;
    colorChangeListenerMap_.clear();
    HILOG_INFO("WallpaperService::initData --> end ");
}

void WallpaperService::AddWallpaperExtensionDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject != nullptr) {
        IPCObjectProxy *proxy = reinterpret_cast<IPCObjectProxy *>(remoteObject.GetRefPtr());
        if (recipient_ == nullptr) {
            recipient_ = sptr<IRemoteObject::DeathRecipient>(new WallpaperExtensionAbilityDeathRecipient(*this));
        }
        if (proxy != nullptr && !proxy->IsObjectDead()) {
            HILOG_INFO("get remoteObject succeed");
            proxy->AddDeathRecipient(recipient_);
        }
    }
}

void WallpaperService::StartWallpaperExtension()
{
    MemoryGuard cacheGuard;
    HILOG_INFO("WallpaperService StartWallpaperExtension");
    int time = 0;
    ErrCode ret = 0;
    AAFwk::Want want;
    want.SetElementName(WALLPAPER_BUNDLE_NAME, "WallpaperExtAbility");
    AAFwk::AbilityManagerClient::GetInstance()->Connect();
    HILOG_INFO("WallpaperService::Startwhile");
    while (1) {
        HILOG_INFO("WallpaperService::StartAbility");
        time++;
        ret = ConnectExtensionAbility(want);
        if (ret == 0 || time == CONNECT_EXTENSION_MAX_RETRY_TIMES) {
            break;
        }
        usleep(CONNECT_EXTENSION_INTERVAL);
        HILOG_INFO("WallpaperService::StartAbility %{public}d", time);
    }
    if (ret != 0) {
        HILOG_ERROR("WallpaperService::StartAbility --> failed ");
        ReporterFault(FaultType::SERVICE_FAULT, FaultCode::SF_STARTABILITY_FAILED);
    }
}
void WallpaperService::OnBootPhase()
{
    HILOG_INFO("WallpaperService OnBootPhase");
    AAFwk::Want want;
    want.SetElementName(WALLPAPER_BUNDLE_NAME, "WallpaperExtAbility");
    AAFwk::AbilityManagerClient::GetInstance()->Connect();
    AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
}

int WallpaperService::GetUserId()
{
    userId_ = 0;
    return userId_;
}

int WallpaperService::GetDisplayId()
{
    int displayid = 0;
    return displayid;
}

std::string WallpaperService::GetWallpaperDir()
{
    std::string sWallpaperPath = WALLPAPER_USERID_PATH + std::to_string(userId_);
    return sWallpaperPath;
}

int WallpaperService::MakeWallpaperIdLocked()
{
    HILOG_INFO("MakeWallpaperIdLocked start");
    if (wallpaperId_ == INT32_MAX) {
        wallpaperId_ = DEFAULT_WALLPAPER_ID;
    }
    return ++wallpaperId_;
}

void WallpaperService::LoadSettingsLocked(int userId, bool keepDimensionHints)
{
    HILOG_INFO("load Setting locked start.");
    if (!wallpaperMap_.Contains(userId)) {
        HILOG_INFO("wallpaperMap_ does not contains userId");
        MigrateFromOld();
        WallpaperData wallpaper(userId, wallpaperSystemFileFullPath_, wallpaperSystemCropFileFullPath_);
        wallpaper.allowBackup = true;
        wallpaper.wallpaperId_ = DEFAULT_WALLPAPER_ID;
        wallpaperMap_.InsertOrAssign(userId, wallpaper);
    }

    if (!lockWallpaperMap_.Contains(userId)) {
        HILOG_INFO("lockWallpaperMap_ does not Contains userId");
        WallpaperData wallpaperLock(userId, wallpaperLockScreenFileFullPath_, wallpaperLockScreenCropFileFullPath_);
        wallpaperLock.allowBackup = true;
        wallpaperLock.wallpaperId_ = DEFAULT_WALLPAPER_ID;
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperLock);
    }
    HILOG_INFO("load Setting locked end.");
}

void WallpaperService::MigrateFromOld()
{
    if (!OHOS::FileExists(wallpaperLockScreenFilePath_)) {
        if (!OHOS::ForceCreateDirectory(wallpaperLockScreenFilePath_)) {
            return;
        }
    }
    if (!OHOS::FileExists(wallpaperSystemFilePath_)) {
        if (!OHOS::ForceCreateDirectory(wallpaperSystemFilePath_)) {
            return;
        }
    }
    if (OHOS::FileExists(wallpaperSystemCropFileFullPath_)) {
        if (!OHOS::FileExists(wallpaperSystemFileFullPath_)) {
            int ret = FileDeal::CopyFile(wallpaperSystemCropFileFullPath_, wallpaperSystemFileFullPath_);
            if (ret < 0) {
                return;
            }
        }
    } else if (OHOS::FileExists(WALLPAPER_DEFAULT_FILEFULLPATH)) {
        int ret = FileDeal::CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperSystemCropFileFullPath_);
        if (ret < 0) {
            return;
        }
        ret = FileDeal::CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperSystemFileFullPath_);
        if (ret < 0) {
            return;
        }
    }
    if (OHOS::FileExists(wallpaperLockScreenCropFileFullPath_)) {
        if (!OHOS::FileExists(wallpaperLockScreenFileFullPath_)) {
            int ret = FileDeal::CopyFile(wallpaperLockScreenCropFileFullPath_, wallpaperLockScreenFileFullPath_);
            if (ret < 0) {
                return;
            }
        }
    } else if (OHOS::FileExists(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH)) {
        int ret = FileDeal::CopyFile(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH, wallpaperLockScreenCropFileFullPath_);
        if (ret < 0) {
            return;
        }
        ret = FileDeal::CopyFile(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH, wallpaperLockScreenFileFullPath_);
        if (ret < 0) {
            return;
        }
    }
}

int32_t WallpaperService::GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    if (wallpaperType == WALLPAPER_SYSTEM) {
        colors.emplace_back(systemWallpaperColor_);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        colors.emplace_back(lockWallpaperColor_);
    }
    HILOG_INFO(" Service End!");
    return E_OK;
}

int32_t WallpaperService::GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return static_cast<int32_t>(E_NOT_SYSTEM_APP);
    }
    return GetColors(wallpaperType, colors);
}

int32_t WallpaperService::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    if (!WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER)) {
        HILOG_INFO("GetFile no get permission!");
        return static_cast<int32_t>(E_NO_PERMISSION);
    }
    int32_t ret = GetImageFd(wallpaperType, wallpaperFd);
    HILOG_INFO("Get image fd ret is : %{public}d", ret);
    return ret;
}

int64_t WallpaperService::WritePixelMapToFile(const std::string &filePath, std::unique_ptr<PixelMap> pixelMap)
{
    ImagePacker imagePacker;
    PackOption option;
    option.format = "image/jpeg";
    option.quality = HUNDRED;
    option.numberHint = 1;
    std::set<std::string> formats;
    uint32_t ret = imagePacker.GetSupportedFormats(formats);
    if (ret != 0) {
        return 0;
    }
    imagePacker.StartPacking(filePath, option);
    HILOG_INFO("AddImage start");
    imagePacker.AddImage(*pixelMap);
    int64_t packedSize = 0;
    HILOG_INFO("FinalizePacking start");
    imagePacker.FinalizePacking(packedSize);
    return packedSize;
}

bool WallpaperService::CompareColor(const uint64_t &localColor, const ColorManager::Color &color)
{
    return localColor == color.PackValue();
}

bool WallpaperService::SaveColor(int wallpaperType)
{
    uint32_t errorCode = 0;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<OHOS::Media::ImageSource> imageSource =
        OHOS::Media::ImageSource::CreateImageSource((wallpaperType == WALLPAPER_SYSTEM ?
            wallpaperSystemCropFileFullPath_: wallpaperLockScreenCropFileFullPath_), opts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("CreateImageSource failed");
        return false;
    }
    OHOS::Media::DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> wallpaperPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("CreatePixelMap failed");
        return false;
    }

    auto colorPicker = Rosen::ColorPicker::CreateColorPicker(std::move(wallpaperPixelMap), errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("CreateColorPicker failed");
        return false;
    }
    auto color = ColorManager::Color();
    uint32_t ret = colorPicker->GetMainColor(color);
    if (ret != Rosen::SUCCESS) {
        HILOG_ERROR("GetMainColor failed ret is : %{public}d", ret);
        return false;
    }
    std::vector<uint64_t> colors;
    if (wallpaperType == WALLPAPER_SYSTEM && !CompareColor(systemWallpaperColor_, color)) {
        systemWallpaperColor_ = color.PackValue();
        colors.emplace_back(systemWallpaperColor_);
        std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
        for (const auto listener : colorChangeListenerMap_) {
            listener.second->OnColorsChange(colors, WALLPAPER_SYSTEM);
        }
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN && !CompareColor(lockWallpaperColor_, color)) {
        lockWallpaperColor_ = color.PackValue();
        colors.emplace_back(lockWallpaperColor_);
        std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
        for (const auto listener : colorChangeListenerMap_) {
            listener.second->OnColorsChange(colors, WALLPAPER_LOCKSCREEN);
        }
    }
    return true;
}

bool WallpaperService::MakeCropWallpaper(int wallpaperType)
{
    uint32_t errorCode = 0;
    bool ret = false;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::unique_ptr<OHOS::Media::ImageSource> imageSource = OHOS::Media::ImageSource::CreateImageSource(
        (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemFileFullPath_ : wallpaperLockScreenFileFullPath_), opts,
        errorCode);
    if (imageSource == nullptr || errorCode != 0) {
        return ret;
    }
    OHOS::Media::DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> wallpaperPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (wallpaperPixelMap == nullptr || errorCode != 0) {
        return ret;
    }
    SetPixelMapCropParameters(std::move(wallpaperPixelMap), decodeOpts);
    wallpaperPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        return false;
    }
    std::string tmpPath = wallpaperCropPath;
    int64_t packedSize = WritePixelMapToFile(tmpPath, std::move(wallpaperPixelMap));
    if (packedSize != 0) {
        ret = FileDeal::CopyFile(tmpPath, (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemCropFileFullPath_
                                                                             : wallpaperLockScreenCropFileFullPath_));
        if (remove(tmpPath.c_str()) < 0) {
            return false;
        }
    }
    HILOG_INFO("End Crop wallpaper: ret= %{public}d", ret);
    return ret;
}

void WallpaperService::SetPixelMapCropParameters(std::unique_ptr<PixelMap> wallpaperPixelMap, DecodeOptions &decodeOpts)
{
    int32_t pictureHeight = wallpaperPixelMap->GetHeight();
    int32_t pictureWidth = wallpaperPixelMap->GetWidth();
    int32_t pyScrWidth = 0;
    int32_t pyScrHeight = 0;
    GetWallpaperMinWidth(pyScrWidth);
    GetWallpaperMinHeight(pyScrHeight);
    bool bHeightFlag = false;
    bool bWidthFlag = false;
    if (pictureHeight > pyScrHeight) {
        decodeOpts.CropRect.top = (pictureHeight - pyScrHeight) / HALF;
        bHeightFlag = true;
    }
    if (pictureWidth > pyScrWidth) {
        decodeOpts.CropRect.left = (pictureWidth - pyScrWidth) / HALF;
        bWidthFlag = true;
    }
    decodeOpts.CropRect.height = bHeightFlag ? pyScrHeight : pictureHeight;
    decodeOpts.desiredSize.height = decodeOpts.CropRect.height;
    decodeOpts.CropRect.width = bWidthFlag ? pyScrWidth : pictureWidth;
    decodeOpts.desiredSize.width = decodeOpts.CropRect.width;
}

int32_t WallpaperService::SetWallpaperByMapV9(int32_t fd, int32_t wallpaperType, int32_t length)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return static_cast<int32_t>(E_NOT_SYSTEM_APP);
    }
    return SetWallpaperByMap(fd, wallpaperType, length);
}

int32_t WallpaperService::SetWallpaperByMap(int fd, int wallpaperType, int length)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaperByMap", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER_BY_MAP));
    HILOG_INFO("SetWallpaperByMap");
    if (!WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER)) {
        HILOG_INFO("SetWallpaperByMap no set permission!");
        return static_cast<int32_t>(E_NO_PERMISSION);
    }
    if (length <= 0 || length > FOO_MAX_LEN) {
        return static_cast<int32_t>(E_PARAMETERS_INVALID);
    }
    std::string url = wallpaperTmpFullPath_;
    char *paperBuf = new (std::nothrow) char[length]();
    if (paperBuf == nullptr) {
        return static_cast<int32_t>(E_NO_MEMORY);
    }
    mtx.lock();
    int32_t bufsize = read(fd, paperBuf, length);
    if (bufsize <= 0) {
        HILOG_ERROR("read fd failed");
        delete[] paperBuf;
        mtx.unlock();
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int fdw = open(url.c_str(), O_WRONLY | O_CREAT, 0660);
    if (fdw < 0) {
        HILOG_ERROR("Open wallpaper tmpFullPath failed, errno %{public}d", errno);
        delete[] paperBuf;
        mtx.unlock();
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int writeSize = write(fdw, paperBuf, length);
    mtx.unlock();
    if (writeSize <= 0) {
        HILOG_ERROR("WritefdToFile failed");
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_DROP_FAILED);
        delete[] paperBuf;
        close(fdw);
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    delete[] paperBuf;
    close(fdw);
    int32_t wallpaperErrorCode = SetWallpaperBackupData(url, wallpaperType);
    SaveColor(wallpaperType);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetWallpaperByMap", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER_BY_MAP));
    return wallpaperErrorCode;
}

int32_t WallpaperService::SetWallpaperByFDV9(int32_t fd, int32_t wallpaperType, int32_t length)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return static_cast<int32_t>(E_NOT_SYSTEM_APP);
    }
    return SetWallpaperByFD(fd, wallpaperType, length);
}

int32_t WallpaperService::SetWallpaperByFD(int fd, int wallpaperType, int length)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaperByFD", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER_BY_FD));
    HILOG_INFO("SetWallpaperByFD");
    if (!WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER)) {
        return static_cast<int32_t>(E_NO_PERMISSION);
    }
    std::string url = wallpaperTmpFullPath_;
    if (length <= 0 || length > FOO_MAX_LEN) {
        return static_cast<int32_t>(E_PARAMETERS_INVALID);
    }
    char *paperBuf = new (std::nothrow) char[length];
    if (paperBuf == nullptr) {
        return E_NO_MEMORY;
    }
    mtx.lock();
    int readSize = read(fd, paperBuf, length);
    if (readSize <= 0) {
        HILOG_ERROR("read from fd fail");
        delete[] paperBuf;
        mtx.unlock();
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int fdw = open(url.c_str(), O_WRONLY | O_CREAT, 0660);
    if (fdw < 0) {
        HILOG_ERROR("Open wallpaper tmpFullPath failed, errno %{public}d", errno);
        delete[] paperBuf;
        mtx.unlock();
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    int writeSize = write(fdw, paperBuf, length);
    mtx.unlock();
    if (writeSize <= 0) {
        HILOG_ERROR("Write to fdw fail, errno %{public}d", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_DROP_FAILED);
        close(fdw);
        delete[] paperBuf;
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    close(fdw);
    delete[] paperBuf;
    int32_t wallpaperErrorCode = SetWallpaperBackupData(url, wallpaperType);
    SaveColor(wallpaperType);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetWallpaperByFD", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER_BY_FD));
    return wallpaperErrorCode;
}

int32_t WallpaperService::SetWallpaperBackupData(std::string uriOrPixelMap, int wallpaperType)
{
    HILOG_INFO("set wallpaper and backup data Start.");
    if (wallpaperType != WALLPAPER_LOCKSCREEN && wallpaperType != WALLPAPER_SYSTEM) {
        return static_cast<int32_t>(E_PARAMETERS_INVALID);
    }

    if (!OHOS::FileExists(uriOrPixelMap)) {
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    WallpaperData wallpaperData(userId_,
        (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemFileFullPath_ : wallpaperLockScreenFileFullPath_),
        (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemCropFileFullPath_ : wallpaperLockScreenCropFileFullPath_));

    mtx.lock();
    bool ret = GetWallpaperSafeLocked(userId_, wallpaperType, wallpaperData);
    if (!ret) {
        HILOG_ERROR("GetWallpaperSafeLocked failed !");
        mtx.unlock();
        return static_cast<int32_t>(E_DEAL_FAILED);
    }

    wallpaperData.wallpaperId_ = MakeWallpaperIdLocked();
    bool retFileCp = FileDeal::CopyFile(uriOrPixelMap,
        (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemFileFullPath_ : wallpaperLockScreenFileFullPath_));
    bool retCropFileCp = MakeCropWallpaper(wallpaperType);
    mtx.unlock();

    if (wallpaperType == WALLPAPER_SYSTEM) {
        wallpaperMap_.InsertOrAssign(userId_, wallpaperData);
        WallpaperCommonEvent::SendWallpaperSystemSettingMessage();
        ReporterUsageTimeStatisic();
        HILOG_INFO("  SetWallpaperBackupData callbackProxy->OnCall start");
        if (callbackProxy != nullptr) {
            callbackProxy->OnCall(wallpaperType);
        }
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId_, wallpaperData);
        WallpaperCommonEvent::SendWallpaperLockSettingMessage();
        ReporterUsageTimeStatisic();
        HILOG_INFO("  SetWallpaperBackupData callbackProxy->OnCall start");
        if (callbackProxy != nullptr) {
            callbackProxy->OnCall(wallpaperType);
        }
    }
    if (remove(uriOrPixelMap.c_str()) < 0) {
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    return (retFileCp && retCropFileCp) ? static_cast<int32_t>(E_OK) : static_cast<int32_t>(E_DEAL_FAILED);
}

void WallpaperService::ReporterUsageTimeStatisic()
{
    int userId = static_cast<int>(IPCSkeleton::GetCallingUid());
    std::string bundleName;
    bool bRet = WPGetBundleNameByUid(userId, bundleName);
    if (!bRet) {
        bundleName = WALLPAPER_BUNDLE_NAME;
    }
    UsageTimeStat timeStat;
    timeStat.packagesName = bundleName;
    timeStat.startTime = time(nullptr);
    StatisticReporter::ReportUsageTimeStatistic(userId, timeStat);
}

int32_t WallpaperService::GetPixelMapV9(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    return GetPixelMap(wallpaperType, fdInfo);
}

int32_t WallpaperService::GetPixelMap(int wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    HILOG_INFO("WallpaperService::getPixelMap start ");
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return static_cast<int32_t>(E_NOT_SYSTEM_APP);
    }
    if (!WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER)) {
        HILOG_INFO("GetPixelMap no get permission!");
        return static_cast<int32_t>(E_NO_PERMISSION);
    }
    int32_t ret = GetImageSize(wallpaperType, fdInfo.size);
    if (ret != static_cast<int32_t>(E_OK)) {
        HILOG_ERROR("GetImageSize failed");
        return ret;
    }
    ret = GetImageFd(wallpaperType, fdInfo.fd);
    if (ret != static_cast<int32_t>(E_OK)) {
        HILOG_ERROR("GetImageFd failed");
        return ret;
    }
    return static_cast<int32_t>(E_OK);
}

int WallpaperService::GetWallpaperId(int32_t wallpaperType)
{
    HILOG_INFO("WallpaperService::GetWallpaperId --> start ");
    int iWallpaperId = 1;
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        auto wallpaperData = lockWallpaperMap_.Find(userId_);
        if (wallpaperData.first) {
            iWallpaperId = wallpaperData.second.wallpaperId_;
        }
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        auto wallpaperData = wallpaperMap_.Find(userId_);
        if (wallpaperData.first) {
            iWallpaperId = wallpaperData.second.wallpaperId_;
        }
    }
    HILOG_INFO("WallpaperService::GetWallpaperId --> end ID[%{public}d]", iWallpaperId);
    return iWallpaperId;
}

int32_t WallpaperService::GetWallpaperMinHeightV9(int32_t &minHeight)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return static_cast<int32_t>(E_NOT_SYSTEM_APP);
    }
    return GetWallpaperMinHeight(minHeight);
}

int32_t WallpaperService::GetWallpaperMinHeight(int32_t &minHeight)
{
    HILOG_INFO("WallpaperService::GetWallpaperMinHeight --> start ");
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr");
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    minHeight = display->GetHeight();
    return static_cast<int32_t>(E_OK);
}

int32_t WallpaperService::GetWallpaperMinWidthV9(int32_t &minWidth)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return static_cast<int32_t>(E_NOT_SYSTEM_APP);
    }
    return GetWallpaperMinWidth(minWidth);
}

int32_t WallpaperService::GetWallpaperMinWidth(int32_t &minWidth)
{
    HILOG_INFO("WallpaperService::GetWallpaperMinWidth --> start ");
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr");
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    minWidth = display->GetWidth();
    return static_cast<int32_t>(E_OK);
}

bool WallpaperService::IsChangePermitted()
{
    HILOG_INFO("IsChangePermitted wallpaper Start!");
    bool bFlag = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    return bFlag;
}

bool WallpaperService::IsOperationAllowed()
{
    HILOG_INFO("IsOperationAllowed wallpaper Start!");
    bool bFlag = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    return bFlag;
}

int32_t WallpaperService::ResetWallpaperV9(int32_t wallpaperType)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return static_cast<int32_t>(E_NOT_SYSTEM_APP);
    }
    return ResetWallpaper(wallpaperType);
}

int32_t WallpaperService::ResetWallpaper(int wallpaperType)
{
    HILOG_INFO("reset wallpaper Start!");
    bool permissionSet = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    if (!permissionSet) {
        HILOG_INFO("reset wallpaper no set permission!");
        return static_cast<int32_t>(E_NO_PERMISSION);
    }

    if (wallpaperType != WALLPAPER_LOCKSCREEN && wallpaperType != WALLPAPER_SYSTEM) {
        HILOG_INFO("wallpaperType = %{public}d type not support ", wallpaperType);
        return static_cast<int32_t>(E_PARAMETERS_INVALID);
    }
    int32_t wallpaperErrorCode;
    ClearWallpaperLocked(userId_, wallpaperType);
    wallpaperErrorCode = SetDefaultDateForWallpaper(userId_, wallpaperType);
    HILOG_INFO(" Set default data result[%{public}d]", wallpaperErrorCode);

    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        if (lockWallpaperMap_.Contains(userId_)) {
            wallpaperErrorCode = static_cast<int32_t>(E_OK);
        }
    } else {
        if (wallpaperMap_.Contains(userId_)) {
            wallpaperErrorCode = static_cast<int32_t>(E_OK);
        }
    }
    HILOG_INFO("reset wallpaper End!");
    return wallpaperErrorCode;
}

bool WallpaperService::CopySystemWallpaper()
{
    if (!OHOS::FileExists(wallpaperSystemFilePath_)) {
        if (!OHOS::ForceCreateDirectory(wallpaperSystemFilePath_)) {
            HILOG_ERROR("CopySystemWallpaper ForceCreateDirectory error");
            return false;
        }
    }
    if (OHOS::FileExists(WALLPAPER_DEFAULT_FILEFULLPATH)) {
        if (!FileDeal::CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperSystemCropFileFullPath_)) {
            HILOG_ERROR("CopyScreenLockWallpaper copy Crop file error");
            return false;
        }
        if (!FileDeal::CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperSystemFileFullPath_)) {
            HILOG_ERROR("CopyScreenLockWallpaper copy Original file error");
            return false;
        }
        WallpaperCommonEvent::SendWallpaperSystemSettingMessage();
        if (callbackProxy != nullptr) {
            HILOG_INFO("CopySystemWallpaper callbackProxy OnCall start");
            callbackProxy->OnCall(WALLPAPER_SYSTEM);
        }
        SaveColor(WALLPAPER_SYSTEM);
    } else {
        HILOG_ERROR("FileExists error");
        return false;
    }
    return true;
}
bool WallpaperService::CopyScreenLockWallpaper()
{
    if (!OHOS::FileExists(wallpaperLockScreenFilePath_)) {
        if (!OHOS::ForceCreateDirectory(wallpaperLockScreenFilePath_)) {
            HILOG_ERROR("CopyScreenLockWallpaper ForceCreateDirectory error");
            return false;
        }
    }
    if (OHOS::FileExists(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH)) {
        if (!FileDeal::CopyFile(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH, wallpaperLockScreenCropFileFullPath_)) {
            HILOG_ERROR("CopyScreenLockWallpaper copy Crop file error");
            return false;
        }
        if (!FileDeal::CopyFile(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH, wallpaperLockScreenFileFullPath_)) {
            HILOG_ERROR("CopyScreenLockWallpaper copy Original file error");
            return false;
        }
        WallpaperCommonEvent::SendWallpaperLockSettingMessage();
        if (callbackProxy != nullptr) {
            HILOG_INFO("CopyScreenLockWallpaper callbackProxy OnCall start");
            callbackProxy->OnCall(WALLPAPER_LOCKSCREEN);
        }
        SaveColor(WALLPAPER_LOCKSCREEN);
    } else {
        HILOG_ERROR("FileExists error");
        return false;
    }
    return true;
}

int32_t WallpaperService::SetDefaultDateForWallpaper(int32_t userId, int32_t wpType)
{
    std::string tmpPath = "";
    std::string tmpCropPath = "";
    if (wpType == WALLPAPER_LOCKSCREEN) {
        if (!CopyScreenLockWallpaper()) {
            HILOG_ERROR("CopyScreenLockWallpaper error");
            return static_cast<int32_t>(E_DEAL_FAILED);
        }
        tmpPath = wallpaperLockScreenFileFullPath_;
        tmpCropPath = wallpaperLockScreenCropFileFullPath_;
    } else {
        if (!CopySystemWallpaper()) {
            HILOG_ERROR("CopySystemWallpaper error");
            return static_cast<int32_t>(E_DEAL_FAILED);
        }
        tmpPath = wallpaperSystemFileFullPath_;
        tmpCropPath = wallpaperSystemCropFileFullPath_;
    }
    WallpaperData wallpaperData(userId, tmpPath, tmpCropPath);
    wallpaperData.wallpaperId_ = DEFAULT_WALLPAPER_ID;
    wallpaperData.allowBackup = true;
    if (wpType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    } else {
        wallpaperMap_.InsertOrAssign(userId, wallpaperData);
    }
    return static_cast<int32_t>(E_OK);
}

bool WallpaperService::On(sptr<IWallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperService::On in");
    if (listener == nullptr) {
        HILOG_ERROR("WallpaperService::On listener is null");
        return false;
    }
    std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
    colorChangeListenerMap_.insert_or_assign(IPCSkeleton::GetCallingTokenID(), listener);
    HILOG_DEBUG("WallpaperService::On out");
    return true;
}

bool WallpaperService::Off(sptr<IWallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperService::Off in");
    (void)listener;
    std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
    auto iter = colorChangeListenerMap_.find(IPCSkeleton::GetCallingTokenID());
    if (iter != colorChangeListenerMap_.end()) {
        iter->second = nullptr;
        colorChangeListenerMap_.erase(iter);
    }
    HILOG_DEBUG("WallpaperService::Off out");
    return true;
}

bool WallpaperService::RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback)
{
    HILOG_INFO("  WallpaperService::RegisterWallpaperCallback");
    callbackProxy = callback;
    return true;
}

bool WallpaperService::GetWallpaperSafeLocked(int userId, int wpType, WallpaperData paperdata)
{
    HILOG_INFO("function start.");
    bool ret = true;
    if (wpType == WALLPAPER_LOCKSCREEN) {
        auto wallpaperData = lockWallpaperMap_.Find(userId);
        if (!wallpaperData.first) {
            HILOG_INFO(" No Lock wallpaper?  Not tracking for lock-only ");
            LoadSettingsLocked(userId, true);
            wallpaperData = lockWallpaperMap_.Find(userId);
            if (!wallpaperData.first) {
                ret = false;
                HILOG_INFO("default data is saved into mLockWallpaperMap failed.");
            }
        }
        if (ret) {
            paperdata.wallpaperId_ = wallpaperData.second.wallpaperId_;
            paperdata.allowBackup = wallpaperData.second.allowBackup;
        }
    } else {
        auto wallpaperData = wallpaperMap_.Find(userId);
        if (!wallpaperData.first) {
            HILOG_INFO(" No system wallpaper?  Not tracking for lock-only ");
            LoadSettingsLocked(userId, true);
            wallpaperData = wallpaperMap_.Find(userId);
            if (!wallpaperData.first) {
                ret = false;
                HILOG_INFO("default data is saved into mWallpaperMap failed.");
            }
        }
        if (ret) {
            paperdata.wallpaperId_ = wallpaperData.second.wallpaperId_;
            paperdata.allowBackup = wallpaperData.second.allowBackup;
        }
    }
    return ret;
}

void WallpaperService::ClearWallpaperLocked(int userId, int wpType)
{
    HILOG_INFO("Clear wallpaper Start!");
    if (wpType == WALLPAPER_LOCKSCREEN) {
        auto wallpaperData = lockWallpaperMap_.Find(userId);
        if (!wallpaperData.first) {
            HILOG_INFO("Lock wallpaper already cleared");
            return;
        }
        if (!wallpaperData.second.wallpaperFile_.empty()) {
            lockWallpaperMap_.Erase(userId);
        }
    } else {
        auto wallpaperData = wallpaperMap_.Find(userId);
        if (!wallpaperData.first) {
            HILOG_INFO("system wallpaper already cleared");
            LoadSettingsLocked(userId, true);
            wallpaperData = wallpaperMap_.Find(userId);
            if (!wallpaperData.first) {
                HILOG_INFO("system wallpaper already cleared too");
                return;
            }
        }
        if (!wallpaperData.second.wallpaperFile_.empty()) {
            wallpaperMap_.Erase(userId);
        }
    }
    HILOG_INFO("Clear wallpaper End!");
}

bool WallpaperService::WPCheckCallingPermission(const std::string &permissionName)
{
    bool bflag = false;
    int result;
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == Security::AccessToken::TOKEN_NATIVE || tokenType == Security::AccessToken::TOKEN_SHELL
        || tokenType == Security::AccessToken::TOKEN_HAP) {
        result = AccessTokenProxy::VerifyAccessToken(callerToken, permissionName);
    } else {
        HILOG_INFO("Check permission tokenId ilegal");
        return false;
    }
    if (result == Security::AccessToken::TypePermissionState::PERMISSION_GRANTED) {
        bflag = true;
    } else {
        bflag = false;
    }
    HILOG_INFO("Check permission result %{public}d", result);
    return bflag;
}

bool WallpaperService::WPGetBundleNameByUid(std::int32_t uid, std::string &bname)
{
    sptr<ISystemAbilityManager> systemMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemMgr == nullptr) {
        HILOG_ERROR("Fail to get system ability mgr");
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("Fail to get bundle manager proxy");
        return false;
    }

    sptr<OHOS::AppExecFwk::IBundleMgr> bundleMgrProxy = iface_cast<OHOS::AppExecFwk::IBundleMgr>(remoteObject);
    if (bundleMgrProxy == nullptr) {
        HILOG_ERROR("Bundle mgr proxy is nullptr");
        return false;
    }

    if (!bundleMgrProxy->GetBundleNameForUid(uid, bname)) {
        HILOG_ERROR("Get bundle name failed");
        return false;
    }
    HILOG_INFO("Get bundle name is %{public}s", bname.c_str());
    return true;
}

void WallpaperService::ReporterFault(FaultType faultType, FaultCode faultCode)
{
    FaultMsg msg;
    msg.faultType = faultType;
    msg.errorCode = faultCode;
    ReportStatus nRet;
    if (faultType == FaultType::SERVICE_FAULT) {
        msg.moduleName = "WallpaperService";
        nRet = FaultReporter::ReportServiceFault(msg);
    } else {
        nRet = FaultReporter::ReportRuntimeFault(msg);
    }

    if (nRet == ReportStatus::SUCCESS) {
        HILOG_INFO("ReporterFault success");
    } else {
        HILOG_ERROR("ReporterFault failed");
    }
}

int WallpaperService::Dump(int fd, const std::vector<std::u16string> &args)
{
    int uid = static_cast<int>(IPCSkeleton::GetCallingUid());
    const int maxUid = 10000;
    if (uid > maxUid) {
        Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
        auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);
        if (tokenType != Security::AccessToken::TOKEN_NATIVE && tokenType != Security::AccessToken::TOKEN_SHELL) {
            return 1;
        }
    }

    std::vector<std::string> argsStr;
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    if (DumpHelper::GetInstance().Dispatch(fd, argsStr)) {
        HILOG_ERROR("DumpHelper Dispatch failed");
        return 0;
    }
    return 1;
}

int32_t WallpaperService::ConnectExtensionAbility(const AAFwk::Want &want)
{
    HILOG_DEBUG("ConnectAdapter");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOG_ERROR("connect ability server failed errCode=%{public}d", errCode);
        return errCode;
    }
    std::vector<int> ids;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        HILOG_ERROR("query active user failed errCode=%{public}d", ret);
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    const sptr<AAFwk::IAbilityConnection> connection = new WallpaperExtensionAbilityConnection();
    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectExtensionAbility(want, connection, ids[0]);
    HILOG_INFO("ConnectExtensionAbility errCode=%{public}d", ret);
    return ret;
}

int32_t WallpaperService::GetFilePath(int wallpaperType, std::string &filePath)
{
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        auto wallpaperData = lockWallpaperMap_.Find(userId_);
        if (wallpaperData.first) {
            filePath = wallpaperData.second.cropFile_;
            return static_cast<int32_t>(E_OK);
        }
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        auto wallpaperData = wallpaperMap_.Find(userId_);
        if (wallpaperData.first) {
            filePath = wallpaperData.second.cropFile_;
            return static_cast<int32_t>(E_OK);
        }
    }
    return static_cast<int32_t>(E_PARAMETERS_INVALID);
}

bool WallpaperService::IsSystemApp()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = GetBundleMgr();
    bool isSystemApplication = false;
    if (bundleMgr != nullptr) {
        isSystemApplication = bundleMgr->CheckIsSystemAppByUid(uid);
    }
    return isSystemApplication;
}

int32_t WallpaperService::GetImageFd(int32_t wallpaperType, int32_t &fd)
{
    HILOG_INFO("WallpaperService::GetImageFd start ");
    std::string filePath = "";
    if (GetFilePath(wallpaperType, filePath) != static_cast<int32_t>(E_OK)) {
        return static_cast<int32_t>(E_PARAMETERS_INVALID);
    }
    mtx.lock();
    fd = open(filePath.c_str(), O_RDONLY, FILE_PERMISSION);
    if (fd < 0) {
        HILOG_ERROR("Open file Path failed, errno %{public}d.", errno);
        ReporterFault(FaultType::LOAD_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        mtx.unlock();
        return static_cast<int32_t>(E_DEAL_FAILED);
    }
    mtx.unlock();
    return static_cast<int32_t>(E_OK);
}

int32_t WallpaperService::GetImageSize(int32_t wallpaperType, int32_t &size)
{
    HILOG_INFO("WallpaperService::GetImageSize start ");
    std::string filePath = "";
    if (GetFilePath(wallpaperType, filePath) != static_cast<int32_t>(E_OK)) {
        return static_cast<int32_t>(E_PARAMETERS_INVALID);
    }

    if (!OHOS::FileExists(filePath)) {
        HILOG_ERROR("file is not exist!");
        return static_cast<int32_t>(E_NOT_FOUND);
    }
    mtx.lock();
    FILE *fd = fopen(filePath.c_str(), "rb");
    if (fd == nullptr) {
        HILOG_ERROR("fopen file Path failed, errno %{public}d.", errno);
        mtx.unlock();
        return static_cast<int32_t>(E_FILE_ERROR);
    }
    int32_t fend = fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    int32_t fset = fseek(fd, 0, SEEK_SET);
    if (size <= 0 || fend != 0 || fset != 0) {
        HILOG_ERROR("ftell file failed or fseek file failed, errno %{public}d", errno);
        fclose(fd);
        mtx.unlock();
        return static_cast<int32_t>(E_FILE_ERROR);
    }
    fclose(fd);
    mtx.unlock();
    return static_cast<int32_t>(E_OK);
}

OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> WallpaperService::GetBundleMgr()
{
    auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        return nullptr;
    }
    auto bundleMgrSa = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleMgrSa == nullptr) {
        return nullptr;
    }
    auto bundleMgr = OHOS::iface_cast<AppExecFwk::IBundleMgr>(bundleMgrSa);
    if (bundleMgr == nullptr) {
        HILOG_INFO("GetBundleMgr is null");
    }
    return bundleMgr;
}
} // namespace WallpaperMgrService
} // namespace OHOS