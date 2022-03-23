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
#include <thread>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <sys/sendfile.h>
#include <window_manager.h>
#include <display_type.h>
#include <iostream>
#include <cstdio>
#include <rs_surface_node.h>
#include "hilog_wrapper.h"
#include "system_ability_definition.h"
#include "wallpaper_common_event.h"
#include "image_utils.h"
#include "file_ex.h"
#include "directory_ex.h"
#include "file_util.h"
#include "time_util.h"
#include "image_packer.h"
#include "bundle_mgr_interface.h"
#include "permission/permission_kit.h"
#include "iservice_registry.h"
#include "image/bitmap.h"
#include "pixel_map.h"
#include "image_type.h"
#include "image_utils.h"
#include "image_source.h"
#include "canvas.h"
#include "pen.h"
#include "wallpaper_service.h"
#include "surface.h"
#include "window.h"
#include "wallpaper_service_cb_proxy.h"
#include "ability_manager_client.h"

namespace OHOS {
namespace WallpaperMgrService {
REGISTER_SYSTEM_ABILITY_BY_ID(WallpaperService, WALLPAPER_MANAGER_SERVICE_ID, true);

using namespace OHOS::Media;
using namespace OHOS::HiviewDFX::FileUtil;
using namespace OHOS::HiviewDFX::TimeUtil;

const std::string WallpaperService::WALLPAPER = "wallpaper_orig";
const std::string WallpaperService::WALLPAPER_CROP = "wallpaper";
const std::string WallpaperService::WALLPAPER_LOCK_ORIG = "wallpaper_lock_orig";
const std::string WallpaperService::WALLPAPER_LOCK_CROP = "wallpaper_lock";

const std::int64_t INIT_INTERVAL = 10000L;
constexpr int SCREEN_WIDTH = 480;
constexpr int SCREEN_HEIGHT = 960;

constexpr int HALF = 2;
constexpr int DOUBLE = 2;
constexpr int THREE = 3;
constexpr int FORE = 4;
constexpr int FIVE = 5;
constexpr int SIX = 6;
constexpr int TEN = 10;
constexpr int FIFTY = 50;
constexpr int HUNDRED = 100;
constexpr int HUNDRED_FIFTY = 150;
constexpr int FOO_MAX_LEN = 52428800;
std::mutex WallpaperService::instanceLock_;

sptr<WallpaperService> WallpaperService::instance_;

std::shared_ptr<AppExecFwk::EventHandler> WallpaperService::serviceHandler_;

WallpaperService::WallpaperService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), state_(ServiceRunningState::STATE_NOT_START)
{
}

WallpaperService::WallpaperService() : state_(ServiceRunningState::STATE_NOT_START)
{
    InitData();
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
    bool ret = Publish(WallpaperService::GetInstance());
    if (!ret) {
        HILOG_ERROR("Publish failed.");
        return -1;
    }
    HILOG_INFO("Publish success.");
    state_ = ServiceRunningState::STATE_RUNNING;
    return 0;
}

void WallpaperService::OnStart()
{
    HILOG_INFO("Enter OnStart.");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOG_ERROR("WallpaperService is already running.");
        return;
    }

    InitServiceHandler();
    if (Init() != 0) {
        auto callback = [=]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        HILOG_ERROR("Init failed. Try again 10s later");
        return;
    }
    HILOG_INFO("RegisterSubscriber");
    OHOS::EventFwk::MatchingSkills matchingSkills;
    HILOG_INFO("WallpaperCommonEvent::AddEvent");
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    HILOG_INFO("WallpaperCommonEvent::subscriberInfo");
    OHOS::EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<EventFwk::CommonEventSubscriber> subscriberPtr_ = std::make_shared<WallpaperCommonEvent>(subscriberInfo);
    bool subRes = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriberPtr_);
    if (subRes == false) {
        HILOG_INFO("RegisterSubscriber failed");
    } else {
        HILOG_INFO("RegisterSubscriber success");
    }
    HILOG_INFO("RegisterSubscriber end");

    std::thread(&WallpaperService::StartExt, this).detach();
    return;
}

void WallpaperService::InitServiceHandler()
{
    HILOG_INFO("InitServiceHandler started.");
    if (serviceHandler_ != nullptr) {
        HILOG_ERROR("InitServiceHandler already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner =
        AppExecFwk::EventRunner::Create("WallpaperService");
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
    state_ = ServiceRunningState::STATE_NOT_START;
    HILOG_INFO("OnStop end.");
}


void WallpaperService::InitData()
{
    HILOG_INFO("WallpaperService::initData --> start ");
    userId_ = 0;
    wallpaperId_ = 0;
    wallpaperMap_.clear();
    lockWallpaperMap_.clear();
    userId_ = GetUserId();
    std::string userIdPath = GetWallpaperDir(userId_);
    this->wallpaperLockScreenFilePath_ = userIdPath +
        "/" + WALLPAPER_DIRNAME + "/" + WALLPAPER_LOCKSCREEN_DIRNAME;
    this->wallpaperSystemFilePath_ = userIdPath +
        "/" + WALLPAPER_DIRNAME + "/" + WALLPAPER_SYSTEM_DIRNAME;
    wallpaperLockScreenFileFullPath_ = wallpaperLockScreenFilePath_ + "/" + WALLPAPER_LOCK_ORIG;
    wallpaperLockScreenCropFileFullPath_ = wallpaperLockScreenFilePath_ + "/" + WALLPAPER_LOCK_CROP;
    wallpaperSystemCropFileFullPath_ = wallpaperSystemFilePath_ + "/" + WALLPAPER_CROP;
    wallpaperSystemFileFullPath_ = wallpaperSystemFilePath_ + "/" + WALLPAPER;
    wallpaperTmpFullPath_ = wallpaperSystemFilePath_ + "/" + WALLPAPER_TMP_DIRNAME;
    wallpaperCropPath = wallpaperSystemFilePath_ + "/" + WALLPAPER_CROP_PICTURE;
    LoadSettingsLocked(userId_, true);
    HILOG_INFO("WallpaperService::initData --> end ");
}
void WallpaperService::StartExt()
{
    HILOG_INFO("WallpaperService StartExt");
    int time = 0;
    ErrCode ret = 0;
    AAFwk::Want want;
    want.SetElementName("com.example.ohosproject.hmservice", "WallpaperExtAbility");
    AAFwk::AbilityManagerClient::GetInstance()->Connect();
    HILOG_INFO("WallpaperService::Startwhile");
    while (1)
    {
        HILOG_INFO("WallpaperService::StartAbility");
        time++;
        ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
        if (ret == 0 || time == TEN) {
            break;
        }
        sleep(SIX);
        HILOG_INFO("WallpaperService::StartAbility %{public}d", time);
    }
    if (ret != 0) {
            HILOG_ERROR("WallpaperService::StartAbility --> failed ");
    }
}
void WallpaperService::OnBootPhase()
{
    HILOG_INFO("WallpaperService OnBootPhase");
    AAFwk::Want want;
    want.SetElementName("com.example.ohosproject.hmservice", "WallpaperExtAbility");
    AAFwk::AbilityManagerClient::GetInstance()->Connect();
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want);
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

std::string WallpaperService::GetWallpaperDir(int userId)
{
    std::string sWallpaperPath = WALLPAPER_USERID_PATH + std::to_string(userId_);
    return sWallpaperPath;
}

int WallpaperService::MakeWallpaperIdLocked()
{
    do {
        ++wallpaperId_;
    } while (wallpaperId_ == 0);
    return wallpaperId_;
}

void WallpaperService::LoadSettingsLocked(int userId, bool keepDimensionHints)
{
    HILOG_INFO("load Setting locked start.");
    std::map<int, WallpaperData>::iterator itr;
    std::map<int, WallpaperData>::iterator itrLock;
    itr = wallpaperMap_.find(userId);
    itrLock = lockWallpaperMap_.find(userId);

    if (itr == wallpaperMap_.end()) {
        MigrateFromOld();
        WallpaperData wallpaper(userId, wallpaperSystemFileFullPath_, wallpaperSystemCropFileFullPath_);
        wallpaper.allowBackup = true;
        wallpaper.wallpaperId_ =  MakeWallpaperIdLocked();
        wallpaperMap_.insert(std::pair<int, WallpaperData>(userId, wallpaper));
    }

    if (itrLock == lockWallpaperMap_.end()) {
        WallpaperData wallpaperLock(userId, wallpaperLockScreenFileFullPath_, wallpaperLockScreenCropFileFullPath_);
        wallpaperLock.allowBackup = true;
        wallpaperLock.wallpaperId_ =  MakeWallpaperIdLocked();
        lockWallpaperMap_.insert(std::pair<int, WallpaperData>(userId, wallpaperLock));
    }
    HILOG_INFO("load Setting locked end.");
}

bool  WallpaperService::ChangingToSame(ComponentName componentName, WallpaperData wallpaper)
{
    if (wallpaper.wallpaperComponent.equals(componentName)) {
        return true;
    }

    return false;
}
bool WallpaperService::BindWallpaperComponentLocked(ComponentName& componentName,
    bool force, bool fromUser, WallpaperData wallpaper)
{
    if (!force && ChangingToSame(componentName, wallpaper)) {
        return true;
    }
    return true;
}

bool WallpaperService::SetLockWallpaperCallback(IWallpaperManagerCallback* cb)
{
    keyguardListener_ = cb;

    return true;
}

void WallpaperService::MigrateFromOld()
{
    int ret = 0;
    if (!OHOS::FileExists(wallpaperLockScreenFilePath_)) {
        if (!OHOS::ForceCreateDirectory(wallpaperLockScreenFilePath_)) {
            return ;
        }
    }
    if (!OHOS::FileExists(wallpaperSystemFilePath_)) {
        if (!OHOS::ForceCreateDirectory(wallpaperSystemFilePath_)) {
            return ;
        }
    }
    if (OHOS::FileExists(wallpaperSystemCropFileFullPath_)) {
        if (!OHOS::FileExists(wallpaperSystemFileFullPath_)) {
            ret = OHOS::HiviewDFX::FileUtil::CopyFile(wallpaperSystemCropFileFullPath_,
                wallpaperSystemFileFullPath_);
            if (ret < 0) {
                return;
            }
        }
    } else if (OHOS::FileExists(WALLPAPER_DEFAULT_FILEFULLPATH)) {
        ret = OHOS::HiviewDFX::FileUtil::CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperSystemCropFileFullPath_);
        if (ret < 0) {
                return;
            }
        ret = OHOS::HiviewDFX::FileUtil::CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH,
            wallpaperSystemFileFullPath_);
        if (ret < 0) {
                return;
            }
    }
    if (OHOS::FileExists(wallpaperLockScreenCropFileFullPath_)) {
        if (!OHOS::FileExists(wallpaperLockScreenFileFullPath_)) {
            ret = OHOS::HiviewDFX::FileUtil::CopyFile(wallpaperLockScreenCropFileFullPath_,
                wallpaperLockScreenFileFullPath_);
                if (ret < 0) {
                return;
            }
        }
    } else if (OHOS::FileExists(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH)) {
        ret = OHOS::HiviewDFX::FileUtil::CopyFile(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH,
            wallpaperLockScreenCropFileFullPath_);
        if (ret < 0) {
                return;
            }
        ret = OHOS::HiviewDFX::FileUtil::CopyFile(WALLPAPER_DEFAULT_LOCK_FILEFULLPATH,
            wallpaperLockScreenFileFullPath_);
            if (ret < 0) {
                return;
            }
    }
}

std::vector<RgbaColor> WallpaperService::GetColors(int wallpaperType)
{
    std::vector<RgbaColor> Colors;
    RgbaColor tmpColor;
    for (int32_t indx = 0; indx < FIVE; indx++) {
        tmpColor.red = HUNDRED + DOUBLE * indx;
        tmpColor.blue = FIFTY + THREE * indx;
        tmpColor.green = FIFTY + FORE * indx;
        tmpColor.alpha = HUNDRED_FIFTY + DOUBLE * indx;
        Colors.emplace_back(tmpColor);
    }
    HILOG_INFO(" Service End!");
    return Colors;
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

bool WallpaperService::MakeCropWallpaper(int wallpaperType)
{
    uint32_t errorCode = 0;
    bool ret = false;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";

    std::unique_ptr<OHOS::Media::ImageSource> imageSource =
        OHOS::Media::ImageSource::CreateImageSource((wallpaperType == WALLPAPER_SYSTEM ?
            wallpaperSystemFileFullPath_: wallpaperLockScreenFileFullPath_), opts, errorCode);
    if (errorCode != 0) {
        return ret;
    }
    OHOS::Media::DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> wallpaperPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        return ret;
    }
    int32_t pictrueHeight = wallpaperPixelMap->GetHeight();
    int32_t pictrueWidth = wallpaperPixelMap->GetWidth();
    int pyScrWidth = GetWallpaperMinWidth();
    int pyScrHeight = GetWallpaperMinHeight();
    if (pictrueHeight > pyScrHeight || pictrueWidth > pyScrWidth) {
        decodeOpts.CropRect.top = (pictrueHeight - pyScrHeight)/HALF;
        decodeOpts.CropRect.width = pyScrWidth;
        decodeOpts.CropRect.left = (pictrueWidth - pyScrWidth)/HALF;
        decodeOpts.CropRect.height = pyScrHeight;
        decodeOpts.desiredSize.width = pyScrWidth;
        decodeOpts.desiredSize.height = pyScrHeight;
    }
    wallpaperPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        ret = false;
    } else {
        std::string tmpPath = wallpaperCropPath;
        int64_t packedSize = WritePixelMapToFile(tmpPath, std::move(wallpaperPixelMap));
        if (packedSize != 0) {
            ret = CopyFile(tmpPath, (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemCropFileFullPath_:
                wallpaperLockScreenCropFileFullPath_));
            if (remove(tmpPath.c_str()) < 0) {
                return false;
            }
        }
    }
    HILOG_INFO("End Crop wallpaper: ret= %{public}d", ret);
    return ret;
}

bool WallpaperService::SetWallpaperByMap(int fd, int wallpaperType, int length)
{
    
    HILOG_INFO("SetWallpaperByMap");
    bool permissionSet = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    if (!permissionSet) {
        HILOG_INFO("SetWallpaperByMap no set permission!");
        return false;
    }
    if (length == 0 || length > FOO_MAX_LEN) {
        return false;
    }
    std::string url = wallpaperTmpFullPath_;
    char* paperBuf = new char[length];
    mtx.lock();
    int32_t bufsize = read(fd, paperBuf, length);
    if (bufsize <= 0) {
        HILOG_ERROR("read fd failed");
        delete[] paperBuf;
        close(fd);
        mtx.unlock();
        return false;
    }
    int fdw = open(url.c_str(), O_WRONLY | O_CREAT, 0660);
    if (fdw == -1) {
        HILOG_ERROR("WallpaperService:: fdw fail");
        delete[] paperBuf;
        close(fd);
        mtx.unlock();
        return false;
    }
    int writeSize = write(fdw, paperBuf, length);
    mtx.unlock();
    if (writeSize <= 0) {
        HILOG_ERROR("WritefdToFile failed");
        delete[] paperBuf;
        close(fd);
        close(fdw);
        return false;
    }
    delete[] paperBuf;
    close(fd);
    close(fdw);
    return SetWallpaperBackupData(url, wallpaperType);
}

bool WallpaperService::SetWallpaperByFD(int fd, int wallpaperType, int length)
{
    HILOG_INFO("SetWallpaperByFD");
    bool permissionSet = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    if (!permissionSet) {
        HILOG_INFO("SetWallpaperByFD no set permission!");
        return false;
    }
    std::string url = wallpaperTmpFullPath_;
    if (length == 0 || length > FOO_MAX_LEN) {
        close(fd);
        return false;
    }
    char* paperBuf = new char[length];
    mtx.lock();
    int readSize = read(fd, paperBuf, length);
    if (readSize <= 0) {
        HILOG_ERROR("read from fd fail");
        delete[] paperBuf;
        close(fd);
        mtx.unlock();
        return false;
    }

    int fdw = open(url.c_str(), O_WRONLY | O_CREAT, 0660);
    if (fdw == -1) {
        HILOG_ERROR("WallpaperService:: fdw fail");
        delete[] paperBuf;
        close(fd);
        mtx.unlock();
        return false;
    }
    int writeSize = write(fdw, paperBuf, length);
    mtx.unlock();
    if (writeSize <= 0) {
        HILOG_ERROR("write to fdw fail");
        close(fd);
        close(fdw);
        delete[] paperBuf;
        return false;
    }
    close(fd);
    close(fdw);
    delete[] paperBuf;
    return SetWallpaperBackupData(url, wallpaperType);
}


bool WallpaperService::SetWallpaperBackupData(std::string uriOrPixelMap, int wallpaperType)
{
    HILOG_INFO("set wallpaper and backup data Start.");
    if (wallpaperType != WALLPAPER_LOCKSCREEN && wallpaperType != WALLPAPER_SYSTEM) {
        return false;
    }

    if (!OHOS::FileExists(uriOrPixelMap)) {
        return false;
    }
    WallpaperData tmpWP(userId_,
    (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemFileFullPath_:
        wallpaperLockScreenFileFullPath_),
    (wallpaperType == WALLPAPER_SYSTEM ? wallpaperSystemCropFileFullPath_:
        wallpaperLockScreenCropFileFullPath_));

    mtx.lock();
    bool ret = GetWallpaperSafeLocked(userId_, wallpaperType, tmpWP);
    if (!ret) {
        HILOG_ERROR("GetWallpaperSafeLocked failed !");
        mtx.unlock();
        return false;
    }

    tmpWP.wallpaperId_ = MakeWallpaperIdLocked();

    bool retFileCp = CopyFile(uriOrPixelMap, (wallpaperType ==
        WALLPAPER_SYSTEM ? wallpaperSystemFileFullPath_:wallpaperLockScreenFileFullPath_));
    bool retCropFileCp = MakeCropWallpaper(wallpaperType);
    mtx.unlock();
    if (wallpaperType == WALLPAPER_SYSTEM) {
        wallpaperMap_.insert(std::pair<int, WallpaperData>(userId_, tmpWP));
        WallpaperCommonEvent::SendWallpaperSystemSettingMessage();
        HILOG_INFO("  SetWallpaperBackupData callbackProxy->OnCall start");
        if (callbackProxy != nullptr) {
            callbackProxy->OnCall(wallpaperType);
        }
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.insert(std::pair<int, WallpaperData>(userId_, tmpWP));
        WallpaperCommonEvent::SendWallpaperLockSettingMessage();
        HILOG_INFO("  SetWallpaperBackupData callbackProxy->OnCall start");
        if (callbackProxy != nullptr) {
            callbackProxy->OnCall(wallpaperType);
        }
    } else {
        return false;
    }
    if (remove(uriOrPixelMap.c_str()) < 0) {
        return false;
    }
    return !retFileCp && !retCropFileCp;
}
IWallpaperService::mapFD  WallpaperService::GetPixelMap(int wallpaperType)
{
    mapFD mapFd;
    HILOG_INFO("WallpaperService::getPixelMap --> start ");
    bool permissionGet = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER);
    if (!permissionGet) {
        HILOG_INFO("GetPixelMap no get permission!");
        return mapFd;
    }

    std::string filePath = "";

    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        filePath = lockWallpaperMap_.find(userId_)->second.cropFile_;
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        filePath = wallpaperMap_.find(userId_)->second.cropFile_;
    }

    if (!OHOS::FileExists(filePath)) {
        HILOG_ERROR("file is not exist!");
        return mapFd;
    }
    mtx.lock();
    FILE *pixmap = fopen(filePath.c_str(), "rb");
    if (pixmap == nullptr) {
        HILOG_ERROR("fopen failed");
        mtx.unlock();
        return mapFd;
    }
    int fend = fseek(pixmap, 0, SEEK_END);
    int length = ftell(pixmap);
    int fset = fseek(pixmap, 0, SEEK_SET);
    if (length <= 0 || fend != 0 || fset != 0) {
        HILOG_ERROR("ftell failed or fseek failed");
        fclose(pixmap);
        mtx.unlock();
        return mapFd;
    }

    mapFd.size = length;
    int closeRes = fclose(pixmap);
    int fd = open(filePath.c_str(), O_RDONLY, 0440);
    mtx.unlock();
    if (closeRes != 0 || fd < 0) {
        HILOG_ERROR("open failed");
        return mapFd;
    }
    mapFd.fd = fd;
    HILOG_INFO("mapFd.fd = %{public}d",mapFd.fd);
    return mapFd;
}

int  WallpaperService::GetWallpaperId(int wallpaperType)
{
    HILOG_INFO("WallpaperService::GetWallpaperId --> start ");
    int iWallpaperId = 1;
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        iWallpaperId = lockWallpaperMap_.find(userId_)->second.wallpaperId_;
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        iWallpaperId = wallpaperMap_.find(userId_)->second.wallpaperId_;
    }
    HILOG_INFO("WallpaperService::GetWallpaperId --> end ID[%{public}d]", iWallpaperId);
    return iWallpaperId;
}
int  WallpaperService::GetWallpaperMinHeight()
{
    HILOG_INFO("WallpaperService::GetWallpaperMinHeight --> start ");
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    int iWallpaperMinHeight = display->GetHeight();
    HILOG_INFO("WallpaperService height: %{public}d", iWallpaperMinHeight);
    return iWallpaperMinHeight;
}

int  WallpaperService::GetWallpaperMinWidth()
{
    HILOG_INFO("WallpaperService::GetWallpaperMinWidth --> start ");
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    int iWallpaperMinWidth = display->GetWidth();
    HILOG_INFO("WallpaperService width: %{public}d", iWallpaperMinWidth);
    return iWallpaperMinWidth;
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

bool WallpaperService::ResetWallpaper(int wallpaperType)
{
    HILOG_INFO("reset wallpaper Start!");
    bool permissionSet = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    if (!permissionSet) {
        HILOG_INFO("reset wallpaper no set permission!");
        return false;
    }
    bool bFlag = false;
    if (wallpaperType != WALLPAPER_LOCKSCREEN && wallpaperType != WALLPAPER_SYSTEM) {
        HILOG_INFO("wallpaperType = %{public}d type not support ", wallpaperType);
        return bFlag;
    }

    ClearWallpaperLocked(userId_, wallpaperType);
    bFlag = SetDefaultDateForWallpaper(userId_, wallpaperType);
    HILOG_INFO(" Set default data result[%{public}d]", bFlag);

    std::map<int, WallpaperData>::iterator itr;
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        itr = lockWallpaperMap_.find(userId_);
    } else {
        itr = wallpaperMap_.find(userId_);
    }

    if (itr != lockWallpaperMap_.end()) {
        bFlag = true;
    }
    HILOG_INFO("reset wallpaper End!");
    return bFlag;
}

bool WallpaperService::SetDefaultDateForWallpaper(int userId, int wpType)
{
    std::string tmpPath = "";
    std::string tmpCropPath = "";
    bool ret = false;
    if (wpType == WALLPAPER_LOCKSCREEN) {
        if (!OHOS::FileExists(wallpaperLockScreenFilePath_)) {
            if (!OHOS::ForceCreateDirectory(wallpaperLockScreenFilePath_)) {
                return false;
            }
        }
        if (OHOS::FileExists(WALLPAPER_DEFAULT_FILEFULLPATH)) {
            if (CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperLockScreenCropFileFullPath_)) {
                return false;
            }
            ret = CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperLockScreenFileFullPath_);
            if (ret == true) {
                return false;
            }
            WallpaperCommonEvent::SendWallpaperLockSettingMessage();
            HILOG_INFO("SetDefaultDateForWallpaper callbackProxy->OnCall start");
            if (callbackProxy != nullptr) {
                callbackProxy->OnCall(wpType);
            }
        }
        tmpPath = wallpaperLockScreenFileFullPath_;
        tmpCropPath = wallpaperLockScreenCropFileFullPath_;
    } else {
        if (!OHOS::FileExists(wallpaperSystemFilePath_)) {
            if (!OHOS::ForceCreateDirectory(wallpaperSystemFilePath_)) {
                return false;
            }
        }
        if (OHOS::FileExists(WALLPAPER_DEFAULT_FILEFULLPATH)) {
            ret = CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperSystemCropFileFullPath_);
            if (ret == true) {
                return false;
            }
            ret = CopyFile(WALLPAPER_DEFAULT_FILEFULLPATH, wallpaperSystemFileFullPath_);
            if (ret == true) {
                return false;
            }
            WallpaperCommonEvent::SendWallpaperSystemSettingMessage();
            HILOG_INFO("SetDefaultDateForWallpaper callbackProxy->OnCall start");
            if (callbackProxy != nullptr) {
            callbackProxy->OnCall(wpType);
            }
        }
        tmpPath = wallpaperSystemFileFullPath_;
        tmpCropPath = wallpaperSystemCropFileFullPath_;
    }
    WallpaperData sdwpdata(userId, tmpPath, tmpCropPath);
    sdwpdata.wallpaperId_ = 0;
    sdwpdata.allowBackup = true;
    if (wpType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.insert(std::pair<int, WallpaperData>(userId, sdwpdata));
    } else {
        wallpaperMap_.insert(std::pair<int, WallpaperData>(userId, sdwpdata));
    }
    return true;
}
bool WallpaperService::ScreenshotLiveWallpaper(int scaleNumber, OHOS::Media::PixelMap pixelMap)
{
    bool bFlag = false;
    return bFlag;
}

bool WallpaperService::On(sptr<IWallpaperColorChangeListener> listener)
{
    std::vector<RgbaColor> Colors;
    RgbaColor tmpColor;
    for (int32_t indx = 0; indx < FIVE; indx++) {
        tmpColor.red = HUNDRED + DOUBLE * indx;
        tmpColor.blue = FIFTY + THREE * indx;
        tmpColor.green = FIFTY + FORE * indx;
        tmpColor.alpha = HUNDRED_FIFTY + DOUBLE * indx;
        Colors.emplace_back(tmpColor);
    }
    listener->onColorsChange(Colors, 0);
    return true;
}

bool WallpaperService::Off(sptr<IWallpaperColorChangeListener> listener)
{
    HILOG_DEBUG("WallpaperService::Off in");
    std::vector<RgbaColor> Colors;
    HILOG_INFO(" Service Start!");
    RgbaColor tmpColor;
    for (int32_t indx = 0; indx < FIVE; indx++) {
        tmpColor.red = HUNDRED + DOUBLE * indx;
        tmpColor.blue = FIFTY + THREE * indx;
        tmpColor.green = FIFTY + FORE * indx;
        tmpColor.alpha = HUNDRED_FIFTY + DOUBLE * indx;
        Colors.emplace_back(tmpColor);
    }
    listener->onColorsChange(Colors, 0);
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
    std::map<int, WallpaperData>::iterator itr;
    if (wpType == WALLPAPER_LOCKSCREEN) {
        itr = lockWallpaperMap_.find(userId);
        if (itr == lockWallpaperMap_.end()) {
            HILOG_INFO(" No Lock wallpaper?  Not tracking for lock-only ");
            LoadSettingsLocked(userId, true);
            itr = lockWallpaperMap_.find(userId);
            if (itr == lockWallpaperMap_.end()) {
                ret = false;
                HILOG_INFO("default data is saved into mLockWallpaperMap failed.");
            }
        }
        if (ret) {
            paperdata.wallpaperId_ = itr->second.wallpaperId_;
            paperdata.allowBackup = itr->second.allowBackup;
        }
    } else {
        itr = wallpaperMap_.find(userId);
        if (itr == wallpaperMap_.end()) {
            HILOG_INFO(" No system wallpaper?  Not tracking for lock-only ");
            LoadSettingsLocked(userId, true);
            itr = wallpaperMap_.find(userId);
            if (itr == wallpaperMap_.end()) {
                ret = false;
                HILOG_INFO("default data is saved into mWallpaperMap failed.");
            }
        }
        if (ret) {
            paperdata.wallpaperId_ = itr->second.wallpaperId_;
            paperdata.allowBackup = itr->second.allowBackup;
        }
    }
    return ret;
}

void WallpaperService::ClearWallpaperLocked(int userId, int wpType)
{
    HILOG_INFO("Clear wallpaper Start!");
    std::map<int, WallpaperData>::iterator itr;
    if (wpType == WALLPAPER_LOCKSCREEN) {
        itr = lockWallpaperMap_.find(userId);
        if (itr == lockWallpaperMap_.end()) {
            HILOG_INFO("Lock wallpaper already cleared");
            return;
        }
    } else {
        itr = wallpaperMap_.find(userId);
        if (itr == wallpaperMap_.end()) {
            HILOG_INFO("system wallpaper already cleared");
            LoadSettingsLocked(userId, true);
            itr = wallpaperMap_.find(userId);
        }
        if (itr == wallpaperMap_.end()) {
            HILOG_INFO("system wallpaper already cleared too");
            return;
        }
    }
    if (itr->second.wallpaperFile_.size() != 0) {
        if (wpType == WALLPAPER_LOCKSCREEN) {
            lockWallpaperMap_.erase(itr);
        } else {
            wallpaperMap_.erase(itr);
        }
    }
    HILOG_INFO("Clear wallpaper End!");
}

bool WallpaperService::WPCheckCallingPermission(const std::string &permissionName)
{
    bool bflag = false;
    int result;
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    if (Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) == Security::AccessToken::TOKEN_NATIVE) {
        result =  Security::AccessToken::AccessTokenKit::VerifyNativeToken(callerToken,
        permissionName);
    } else if (Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) ==
        Security::AccessToken::TOKEN_HAP) {
        result =  Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        permissionName);
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

    return true;
}
void WallpaperService::WallpaperDump(int fd)
{
    dprintf(fd, "\n - Wallpaper System State :\n");
    dprintf(fd, " * UserId = %d\n", userId_);
    dprintf(fd, " * WallpaperId = %d\n", wallpaperId_);
    int32_t height = GetWallpaperMinHeight();
    dprintf(fd, " * heigh = %d\n", height);
    int32_t width = GetWallpaperMinWidth();
    dprintf(fd, " * heigh = %d\n", width);
}
}
}