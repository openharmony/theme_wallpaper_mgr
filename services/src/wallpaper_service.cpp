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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <display_type.h>
#include <fcntl.h>
#include <iostream>
#include <sys/prctl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <window_manager.h>

#include "ability_manager_client.h"
#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
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
#include "image_packer.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "iservice_registry.h"
#include "memory_guard.h"
#include "pixel_map.h"
#include "surface.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "wallpaper_common.h"
#include "wallpaper_common_event_manager.h"
#include "wallpaper_extension_ability_connection.h"
#include "wallpaper_extension_ability_death_recipient.h"
#include "wallpaper_service_cb_proxy.h"
#include "window.h"

namespace OHOS {
namespace WallpaperMgrService {
REGISTER_SYSTEM_ABILITY_BY_ID(WallpaperService, WALLPAPER_MANAGER_SERVICE_ID, true);

using namespace OHOS::Media;
using namespace OHOS::MiscServices;
using namespace OHOS::Security::AccessToken;

constexpr const char *WALLPAPER = "wallpaper_system_orig";
constexpr const char *WALLPAPER_CROP = "wallpaper_system";
constexpr const char *WALLPAPER_LOCK_ORIG = "wallpaper_lock_orig";
constexpr const char *WALLPAPER_LOCK_CROP = "wallpaper_lock";
constexpr const char *OHOS_WALLPAPER_BUNDLE_NAME = "com.ohos.launcher";

constexpr int64_t INIT_INTERVAL = 10000L;
constexpr int64_t DELAY_TIME = 1000L;
constexpr int64_t QUERY_USER_ID_INTERVAL = 300L;
constexpr int32_t HALF = 2;
constexpr int32_t CONNECT_EXTENSION_INTERVAL = 500000;
constexpr int32_t CONNECT_EXTENSION_MAX_RETRY_TIMES = 360;
constexpr int32_t HUNDRED = 100;
constexpr int32_t FOO_MAX_LEN = 52428800;
constexpr int32_t MAX_RETRY_TIMES = 20;
constexpr int32_t QUERY_USER_MAX_RETRY_TIMES = 100;
constexpr int32_t DEFAULT_WALLPAPER_ID = -1;
constexpr int32_t DEFAULT_USER_ID = 0;
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

int32_t WallpaperService::Init()
{
    InitQueryUserId(QUERY_USER_MAX_RETRY_TIMES);
    bool ret = Publish(this);
    if (!ret) {
        HILOG_ERROR("Publish failed.");
        ReporterFault(FaultType::SERVICE_FAULT, FaultCode::SF_SERVICE_UNAVAILABLE);
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
    auto cmd = std::make_shared<Command>(std::vector<std::string>({ "-all" }), "Show all",
        [this](const std::vector<std::string> &input, std::string &output) -> bool {
            int32_t height = 0;
            int32_t width = 0;
            GetWallpaperMinHeight(height);
            GetWallpaperMinWidth(width);
            std::string bundleName(OHOS_WALLPAPER_BUNDLE_NAME);
            output.append("height\t\t\t: " + std::to_string(height) + "\n")
                .append("width\t\t\t: " + std::to_string(width) + "\n")
                .append("WallpaperExtension\t: ExtensionInfo{" + bundleName + "}\n");
            return true;
        });
    DumpHelper::GetInstance().RegisterCommand(cmd);
    StatisticReporter::StartTimerThread();
    if (Init() != E_OK) {
        auto callback = [=]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        HILOG_ERROR("Init failed. Try again 10s later");
    }
    return;
}

void WallpaperService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    HILOG_INFO("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    if (systemAbilityId == COMMON_EVENT_SERVICE_ID) {
        int32_t times = 0;
        RegisterSubscriber(times);
    }
}

void WallpaperService::RegisterSubscriber(int32_t times)
{
    MemoryGuard cacheGuard;
    times++;
    subscriber_ = std::make_shared<WallpaperCommonEventSubscriber>(*this);
    bool subRes = OHOS::EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
    if (!subRes && times <= MAX_RETRY_TIMES) {
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
    connection_ = nullptr;
    recipient_ = nullptr;
    if (subscriber_ != nullptr) {
        bool unSubscribeResult = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
        HILOG_INFO("UnregisterSubscriber end, unSubscribeResult = %{public}d", unSubscribeResult);
    }
    state_ = ServiceRunningState::STATE_NOT_START;
    HILOG_INFO("OnStop end.");
}

void WallpaperService::InitData()
{
    HILOG_INFO("WallpaperService::initData --> start ");
    wallpaperId_ = DEFAULT_WALLPAPER_ID;
    int32_t userId = DEFAULT_USER_ID;
    systemWallpaperMap_.Clear();
    lockWallpaperMap_.Clear();
    wallpaperTmpFullPath_ = WALLPAPER_USERID_PATH + WALLPAPER_TMP_DIRNAME;
    wallpaperCropPath_ = WALLPAPER_USERID_PATH + WALLPAPER_CROP_PICTURE;
    systemWallpaperColor_ = 0;
    lockWallpaperColor_ = 0;
    colorChangeListenerMap_.clear();
    InitUserDir(userId);
    LoadSettingsLocked(userId, true);
    InitResources(userId, WALLPAPER_SYSTEM);
    InitResources(userId, WALLPAPER_LOCKSCREEN);
    SaveColor(userId, WALLPAPER_SYSTEM);
    SaveColor(userId, WALLPAPER_LOCKSCREEN);
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
    prctl(PR_SET_NAME, "WallpaperExtensionThread");
    int32_t time = 0;
    ErrCode ret = 0;
    AAFwk::Want want;
    want.SetElementName(OHOS_WALLPAPER_BUNDLE_NAME, "WallpaperExtAbility");
    AAFwk::AbilityManagerClient::GetInstance()->Connect();
    HILOG_INFO("WallpaperService::Startwhile");
    while (1) {
        HILOG_INFO("WallpaperService::StartAbility");
        time++;
        ret = ConnectExtensionAbility(want);
        if (ret == 0 || time >= CONNECT_EXTENSION_MAX_RETRY_TIMES) {
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

void WallpaperService::InitQueryUserId(int32_t times)
{
    times--;
    bool ret = InitUsersOnBoot();
    if (!ret && times > 0) {
        HILOG_INFO("InitQueryUserId failed");
        auto callback = [this, times]() { InitQueryUserId(times); };
        serviceHandler_->PostTask(callback, QUERY_USER_ID_INTERVAL);
    }
}

bool WallpaperService::InitUsersOnBoot()
{
    std::vector<int32_t> userIds;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds) != ERR_OK || userIds.empty()) {
        HILOG_INFO("WallpaperService: failed to get current userIds");
        return false;
    }
    HILOG_INFO("WallpaperService::get current userIds success, Current userId: %{public}d", userIds[0]);
    for (auto userId : userIds) {
        InitUserDir(userId);
        LoadSettingsLocked(userId, true);
        InitResources(userId, WALLPAPER_SYSTEM);
        InitResources(userId, WALLPAPER_LOCKSCREEN);
    }
    return true;
}

void WallpaperService::OnInitUser(int32_t userId)
{
    if (userId < 0) {
        HILOG_ERROR("userId error, userId = %{public}d", userId);
        return;
    }
    std::string userDir = WALLPAPER_USERID_PATH + std::to_string(userId);
    if (FileDeal::IsFileExist(userDir)) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!OHOS::ForceRemoveDirectory(userDir)) {
            HILOG_ERROR("Force remove user directory path failed, errno %{public}d, userId:%{public}d", errno, userId);
            return;
        }
    }
    if (!InitUserDir(userId)) {
        return;
    }
    LoadSettingsLocked(userId, true);
    InitResources(userId, WALLPAPER_SYSTEM);
    InitResources(userId, WALLPAPER_LOCKSCREEN);
    HILOG_INFO("OnInitUser success, userId = %{public}d", userId);
}

void WallpaperService::InitResources(int32_t userId, WallpaperType wallpaperType)
{
    std::string pathName;
    if (!GetFileNameFromMap(userId, wallpaperType, FileType::CROP_FILE, pathName)) {
        HILOG_ERROR("Get user file name from map failed, userId = %{public}d", userId);
        return;
    }
    if (!FileDeal::IsFileExist(pathName)) {
        WallpaperData wallpaperData;
        if (!GetWallpaperSafeLocked(userId, wallpaperType, wallpaperData)) {
            HILOG_ERROR("Get wallpaper data failed, userId = %{public}d", userId);
            return;
        }
        if (!RestoreUserResources(wallpaperData, wallpaperType)) {
            HILOG_ERROR("RestoreUserResources error");
            return;
        }
    }
}

bool WallpaperService::InitUserDir(int32_t userId)
{
    std::string userDir = WALLPAPER_USERID_PATH + std::to_string(userId);
    if (!FileDeal::Mkdir(userDir)) {
        HILOG_ERROR("Failed to create destination path, userId:%{public}d ", userId);
        return false;
    }
    std::string wallpaperSystemFilePath = userDir + "/" + WALLPAPER_SYSTEM_DIRNAME;
    if (!FileDeal::Mkdir(wallpaperSystemFilePath)) {
        HILOG_ERROR("Failed to create destination wallpaper system path, userId:%{public}d, type:%{public}s", userId,
            WALLPAPER_SYSTEM_DIRNAME.c_str());
        return false;
    }
    std::string wallpaperLockScreenFilePath = userDir + "/" + WALLPAPER_LOCKSCREEN_DIRNAME;
    if (!FileDeal::Mkdir(wallpaperLockScreenFilePath)) {
        HILOG_ERROR("Failed to create destination wallpaper lockscreen path, userId:%{public}d, type:%{public}s",
            userId, WALLPAPER_LOCKSCREEN_DIRNAME.c_str());
        return false;
    }
    return true;
}

bool WallpaperService::RestoreUserResources(const WallpaperData &wallpaperData, WallpaperType wallpaperType)
{
    const std::string &wallpaperDefaultPath = wallpaperType == WallpaperType::WALLPAPER_SYSTEM
                                                  ? WALLPAPER_DEFAULT_FILEFULLPATH
                                                  : WALLPAPER_DEFAULT_LOCK_FILEFULLPATH;
    if (!FileDeal::IsFileExist(wallpaperDefaultPath) ||
        !FileDeal::IsFileExist(GetWallpaperDir(wallpaperData.userId, wallpaperType))) {
        HILOG_INFO("Copy file path is not exist");
        return false;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    if (!FileDeal::CopyFile(wallpaperDefaultPath, wallpaperData.wallpaperFile)) {
        HILOG_INFO("CopyFile WALLPAPER_DEFAULT_FILEFULLPATH failed");
        return false;
    }
    if (!FileDeal::CopyFile(wallpaperDefaultPath, wallpaperData.cropFile)) {
        HILOG_INFO("CopyFile WALLPAPER_DEFAULT_FILEFULLPATH failed");
        return false;
    }
    HILOG_INFO("Restore user resources end ");
    return true;
}

void WallpaperService::OnRemovedUser(int32_t userId)
{
    if (userId < 0) {
        HILOG_ERROR("userId error, userId = %{public}d", userId);
        return;
    }
    ClearWallpaperLocked(userId, WALLPAPER_SYSTEM);
    ClearWallpaperLocked(userId, WALLPAPER_LOCKSCREEN);
    std::string userDir = WALLPAPER_USERID_PATH + std::to_string(userId);
    std::lock_guard<std::mutex> lock(mtx_);
    if (!OHOS::ForceRemoveDirectory(userDir)) {
        HILOG_ERROR("Force remove user directory path failed, errno %{public}d.", errno);
    }
    HILOG_INFO("OnRemovedUser end, userId = %{public}d", userId);
}

void WallpaperService::OnSwitchedUser(int32_t userId)
{
    if (userId < 0) {
        HILOG_ERROR("userId error, userId = %{public}d", userId);
        return;
    }
    std::string userDir = WALLPAPER_USERID_PATH + std::to_string(userId);
    LoadSettingsLocked(userId, true);
    if (!FileDeal::IsFileExist(userDir)) {
        HILOG_INFO("User file is not exist, userId = %{public}d", userId);
        InitUserDir(userId);
        InitResources(userId, WALLPAPER_SYSTEM);
        InitResources(userId, WALLPAPER_LOCKSCREEN);
    }
    SaveColor(userId, WALLPAPER_SYSTEM);
    SaveColor(userId, WALLPAPER_LOCKSCREEN);
    shared_ptr<WallpaperCommonEventManager> wallpaperCommonEventManager = make_shared<WallpaperCommonEventManager>();
    HILOG_INFO("Send wallpaper setting message");
    wallpaperCommonEventManager->SendWallpaperSystemSettingMessage();
    wallpaperCommonEventManager->SendWallpaperLockSettingMessage();
    HILOG_INFO("OnSwitchedUser end, newUserId = %{public}d", userId);
}

void WallpaperService::OnBootPhase()
{
    HILOG_INFO("WallpaperService OnBootPhase");
}

int32_t WallpaperService::GetDisplayId()
{
    int32_t displayid = 0;
    return displayid;
}

std::string WallpaperService::GetWallpaperDir(int32_t userId, WallpaperType wallpaperType)
{
    std::string userIdPath = WALLPAPER_USERID_PATH + std::to_string(userId);
    std::string wallpaperFilePath;
    if (wallpaperType == WALLPAPER_SYSTEM) {
        wallpaperFilePath = userIdPath + "/" + WALLPAPER_SYSTEM_DIRNAME;
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        wallpaperFilePath = userIdPath + "/" + WALLPAPER_LOCKSCREEN_DIRNAME;
    }
    return wallpaperFilePath;
}

bool WallpaperService::GetFileNameFromMap(int32_t userId, WallpaperType wallpaperType, FileType fileType,
    std::string &filePathName)
{
    auto iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                      : lockWallpaperMap_.Find(userId);
    if (!iterator.first) {
        HILOG_INFO("system wallpaper already cleared");
        return false;
    }
    filePathName = fileType == FileType::WALLPAPER_FILE ? iterator.second.wallpaperFile : iterator.second.cropFile;
    return true;
}

int32_t WallpaperService::MakeWallpaperIdLocked()
{
    HILOG_INFO("MakeWallpaperIdLocked start");
    if (wallpaperId_ == INT32_MAX) {
        wallpaperId_ = DEFAULT_WALLPAPER_ID;
    }
    return ++wallpaperId_;
}

void WallpaperService::LoadSettingsLocked(int32_t userId, bool keepDimensionHints)
{
    HILOG_INFO("load Setting locked start.");
    if (!systemWallpaperMap_.Contains(userId)) {
        HILOG_INFO("systemWallpaperMap_ does not contains userId");
        std::string wallpaperSystemFilePath = GetWallpaperDir(userId, WALLPAPER_SYSTEM);
        WallpaperData wallpaperSystem(userId, wallpaperSystemFilePath + "/" + WALLPAPER,
            wallpaperSystemFilePath + "/" + WALLPAPER_CROP);
        wallpaperSystem.allowBackup = true;
        wallpaperSystem.wallpaperId = DEFAULT_WALLPAPER_ID;
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperSystem);
    }
    if (!lockWallpaperMap_.Contains(userId)) {
        HILOG_INFO("lockWallpaperMap_ does not Contains userId");
        std::string wallpaperLockScreenFilePath = GetWallpaperDir(userId, WALLPAPER_LOCKSCREEN);
        WallpaperData wallpaperLock(userId, wallpaperLockScreenFilePath + "/" + WALLPAPER_LOCK_ORIG,
            wallpaperLockScreenFilePath + "/" + WALLPAPER_LOCK_CROP);
        wallpaperLock.allowBackup = true;
        wallpaperLock.wallpaperId = DEFAULT_WALLPAPER_ID;
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperLock);
    }
    HILOG_INFO("load Setting locked end.");
}

ErrorCode WallpaperService::GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    if (wallpaperType == WALLPAPER_SYSTEM) {
        colors.emplace_back(systemWallpaperColor_);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        colors.emplace_back(lockWallpaperColor_);
    }
    HILOG_INFO(" Service End!");
    return E_OK;
}

ErrorCode WallpaperService::GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return GetColors(wallpaperType, colors);
}

ErrorCode WallpaperService::GetFile(int32_t wallpaperType, int32_t &wallpaperFd)
{
    if (!WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER)) {
        HILOG_ERROR("GetPixelMap no get permission!");
        return E_NO_PERMISSION;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN) &&
        wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }
    auto type = static_cast<WallpaperType>(wallpaperType);
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    ErrorCode ret = GetImageFd(userId, type, wallpaperFd);
    HILOG_INFO("GetImageFd fd:%{public}d, ret:%{public}d", wallpaperFd, ret);
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

bool WallpaperService::SaveColor(int32_t userId, WallpaperType wallpaperType)
{
    uint32_t errorCode = 0;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::string pathName;
    if (!GetFileNameFromMap(userId, wallpaperType, FileType::CROP_FILE, pathName)) {
        return false;
    }
    std::unique_ptr<OHOS::Media::ImageSource> imageSource =
        OHOS::Media::ImageSource::CreateImageSource(pathName, opts, errorCode);
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
        for (const auto &listener : colorChangeListenerMap_) {
            listener.second->OnColorsChange(colors, WALLPAPER_SYSTEM);
        }
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN && !CompareColor(lockWallpaperColor_, color)) {
        lockWallpaperColor_ = color.PackValue();
        colors.emplace_back(lockWallpaperColor_);
        std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
        for (const auto &listener : colorChangeListenerMap_) {
            listener.second->OnColorsChange(colors, WALLPAPER_LOCKSCREEN);
        }
    }
    return true;
}

bool WallpaperService::MakeCropWallpaper(int32_t userId, WallpaperType wallpaperType)
{
    uint32_t errorCode = 0;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/jpeg";
    std::string pathName;
    if (!GetFileNameFromMap(userId, wallpaperType, FileType::WALLPAPER_FILE, pathName)) {
        return false;
    }
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource(pathName, opts, errorCode);
    if (imageSource == nullptr || errorCode != 0) {
        return false;
    }
    OHOS::Media::DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> wallpaperPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (wallpaperPixelMap == nullptr || errorCode != 0) {
        return false;
    }
    SetPixelMapCropParameters(std::move(wallpaperPixelMap), decodeOpts);
    wallpaperPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        return false;
    }
    std::string &tmpPath = wallpaperCropPath_;
    int64_t packedSize = WritePixelMapToFile(tmpPath, std::move(wallpaperPixelMap));
    std::string cropFile;
    if (packedSize <= 0 || !GetFileNameFromMap(userId, wallpaperType, FileType::CROP_FILE, cropFile)) {
        return false;
    }
    if (!FileDeal::CopyFile(tmpPath, cropFile) || !FileDeal::DeleteFile(tmpPath)) {
        return false;
    }
    return true;
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

ErrorCode WallpaperService::SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    if (!WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER)) {
        HILOG_INFO("SetWallpaper no set permission!");
        return E_NO_PERMISSION;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN) &&
        wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }
    if (length <= 0 || length > FOO_MAX_LEN) {
        return E_PARAMETERS_INVALID;
    }
    std::string uri = wallpaperTmpFullPath_;
    char *paperBuf = new (std::nothrow) char[length]();
    if (paperBuf == nullptr) {
        return E_NO_MEMORY;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    int32_t readSize = read(fd, paperBuf, length);
    if (readSize <= 0) {
        HILOG_ERROR("read fd failed");
        delete[] paperBuf;
        return E_DEAL_FAILED;
    }
    int32_t fdw = open(uri.c_str(), O_WRONLY | O_CREAT, 0660);
    if (fdw < 0) {
        HILOG_ERROR("Open wallpaper tmpFullPath failed, errno %{public}d", errno);
        delete[] paperBuf;
        return E_DEAL_FAILED;
    }
    int32_t writeSize = write(fdw, paperBuf, length);
    if (writeSize <= 0) {
        HILOG_ERROR("Write to fdw failed, errno %{public}d", errno);
        ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_DROP_FAILED);
        delete[] paperBuf;
        close(fdw);
        return E_DEAL_FAILED;
    }
    delete[] paperBuf;
    close(fdw);
    WallpaperType type = static_cast<WallpaperType>(wallpaperType);
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    ErrorCode wallpaperErrorCode = SetWallpaperBackupData(userId, uri, type);
    SaveColor(userId, type);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::SetWallpaperV9(int32_t fd, int32_t wallpaperType, int32_t length)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return SetWallpaper(fd, wallpaperType, length);
}

ErrorCode WallpaperService::SetWallpaperBackupData(int32_t userId, const std::string &uriOrPixelMap,
    WallpaperType wallpaperType)
{
    HILOG_INFO("set wallpaper and backup data Start.");
    if (!OHOS::FileExists(uriOrPixelMap)) {
        return E_DEAL_FAILED;
    }
    WallpaperData wallpaperData;
    bool ret = GetWallpaperSafeLocked(userId, wallpaperType, wallpaperData);
    if (!ret) {
        HILOG_ERROR("GetWallpaperSafeLocked failed !");
        return E_DEAL_FAILED;
    }
    wallpaperData.wallpaperId = MakeWallpaperIdLocked();
    if (!FileDeal::CopyFile(uriOrPixelMap, wallpaperData.wallpaperFile)) {
        HILOG_ERROR("CopyFile failed !");
        return E_DEAL_FAILED;
    }
    if (!FileDeal::DeleteFile(uriOrPixelMap)) {
        return E_DEAL_FAILED;
    }
    if (!MakeCropWallpaper(userId, wallpaperType)) {
        HILOG_ERROR("Make crop wallpaper failed !");
        return E_DEAL_FAILED;
    }
    shared_ptr<WallpaperCommonEventManager> wallpaperCommonEventManager = make_shared<WallpaperCommonEventManager>();
    if (wallpaperType == WALLPAPER_SYSTEM) {
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperData);
        HILOG_INFO("Send wallpaper system setting message");
        wallpaperCommonEventManager->SendWallpaperSystemSettingMessage();
        ReporterUsageTimeStatistic();
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
        HILOG_INFO("Send wallpaper lock setting message");
        wallpaperCommonEventManager->SendWallpaperLockSettingMessage();
        ReporterUsageTimeStatistic();
    }
    HILOG_INFO("SetWallpaperBackupData callbackProxy_->OnCall start");
    if (callbackProxy_ != nullptr) {
        callbackProxy_->OnCall(wallpaperType);
    }
    return E_OK;
}

void WallpaperService::ReporterUsageTimeStatistic()
{
    int32_t uid = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    std::string bundleName;
    bool bRet = WPGetBundleNameByUid(uid, bundleName);
    if (!bRet) {
        bundleName = OHOS_WALLPAPER_BUNDLE_NAME;
    }
    UsageTimeStat timeStat;
    timeStat.packagesName = bundleName;
    timeStat.startTime = time(nullptr);
    StatisticReporter::ReportUsageTimeStatistic(uid, timeStat);
}

ErrorCode WallpaperService::GetPixelMap(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    HILOG_INFO("WallpaperService::getPixelMap start ");
    if (!WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER)) {
        HILOG_INFO("GetPixelMap no get permission!");
        return E_NO_PERMISSION;
    }
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN) &&
        wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }
    auto type = static_cast<WallpaperType>(wallpaperType);
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    ErrorCode ret = GetImageSize(userId, type, fdInfo.size);
    if (ret != E_OK) {
        HILOG_ERROR("GetImageSize failed");
        return ret;
    }
    ret = GetImageFd(userId, type, fdInfo.fd);
    if (ret != E_OK) {
        HILOG_ERROR("GetImageFd failed");
        return ret;
    }
    return E_OK;
}

ErrorCode WallpaperService::GetPixelMapV9(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    return GetPixelMap(wallpaperType, fdInfo);
}

int32_t WallpaperService::GetWallpaperId(int32_t wallpaperType)
{
    HILOG_INFO("WallpaperService::GetWallpaperId --> start ");
    int32_t iWallpaperId = -1;
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        auto iterator = lockWallpaperMap_.Find(userId);
        if (iterator.first) {
            iWallpaperId = iterator.second.wallpaperId;
        }
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        auto iterator = systemWallpaperMap_.Find(userId);
        if (iterator.first) {
            iWallpaperId = iterator.second.wallpaperId;
        }
    }
    HILOG_INFO("WallpaperService::GetWallpaperId --> end ID[%{public}d]", iWallpaperId);
    return iWallpaperId;
}

ErrorCode WallpaperService::GetWallpaperMinHeight(int32_t &minHeight)
{
    HILOG_INFO("WallpaperService::GetWallpaperMinHeight --> start ");
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr");
        return E_DEAL_FAILED;
    }
    minHeight = display->GetHeight();
    return E_OK;
}

ErrorCode WallpaperService::GetWallpaperMinHeightV9(int32_t &minHeight)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return GetWallpaperMinHeight(minHeight);
}

ErrorCode WallpaperService::GetWallpaperMinWidth(int32_t &minWidth)
{
    HILOG_INFO("WallpaperService::GetWallpaperMinWidth --> start ");
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        HILOG_ERROR("GetDefaultDisplay is nullptr");
        return E_DEAL_FAILED;
    }
    minWidth = display->GetWidth();
    return E_OK;
}

ErrorCode WallpaperService::GetWallpaperMinWidthV9(int32_t &minWidth)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return GetWallpaperMinWidth(minWidth);
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

ErrorCode WallpaperService::ResetWallpaper(int32_t wallpaperType)
{
    HILOG_INFO("reset wallpaper Start!");
    bool permissionSet = WPCheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    if (!permissionSet) {
        HILOG_INFO("reset wallpaper no set permission!");
        return E_NO_PERMISSION;
    }

    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN) &&
        wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        HILOG_INFO("wallpaperType = %{public}d type not support ", wallpaperType);
        return E_PARAMETERS_INVALID;
    }
    WallpaperType type = static_cast<WallpaperType>(wallpaperType);
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    ErrorCode wallpaperErrorCode = SetDefaultDataForWallpaper(userId, type);
    HILOG_INFO(" Set default data result[%{public}d]", wallpaperErrorCode);
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::ResetWallpaperV9(int32_t wallpaperType)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return ResetWallpaper(wallpaperType);
}

ErrorCode WallpaperService::SetDefaultDataForWallpaper(int32_t userId, WallpaperType wallpaperType)
{
    WallpaperData wallpaperData;
    if (!GetWallpaperSafeLocked(userId, wallpaperType, wallpaperData)) {
        return E_DEAL_FAILED;
    }

    if (!RestoreUserResources(wallpaperData, wallpaperType)) {
        HILOG_ERROR("RestoreUserResources error");
        return E_DEAL_FAILED;
    }
    wallpaperData.wallpaperId = DEFAULT_WALLPAPER_ID;
    wallpaperData.allowBackup = true;
    shared_ptr<WallpaperCommonEventManager> wallpaperCommonEventManager = make_shared<WallpaperCommonEventManager>();
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
        HILOG_INFO("Send wallpaper lock setting message");
        wallpaperCommonEventManager->SendWallpaperLockSettingMessage();
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperData);
        HILOG_INFO("Send wallpaper system setting message");
        wallpaperCommonEventManager->SendWallpaperSystemSettingMessage();
    }
    if (callbackProxy_ != nullptr) {
        HILOG_INFO("CopyScreenLockWallpaper callbackProxy OnCall start");
        callbackProxy_->OnCall(wallpaperType);
    }
    SaveColor(userId, wallpaperType);
    return E_OK;
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
    callbackProxy_ = callback;
    return true;
}

bool WallpaperService::GetWallpaperSafeLocked(int32_t userId, WallpaperType wallpaperType, WallpaperData &wallpaperData)
{
    HILOG_INFO("function start.");
    auto iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                      : lockWallpaperMap_.Find(userId);
    if (!iterator.first) {
        HILOG_INFO(" No Lock wallpaper?  Not tracking for lock-only ");
        LoadSettingsLocked(userId, true);
        iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                     : lockWallpaperMap_.Find(userId);
        if (!iterator.first) {
            HILOG_ERROR("Fail to get wallpaper data");
            return false;
        }
    }
    wallpaperData = iterator.second;
    return true;
}

void WallpaperService::ClearWallpaperLocked(int32_t userId, WallpaperType wallpaperType)
{
    HILOG_INFO("Clear wallpaper Start!");
    auto iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                      : lockWallpaperMap_.Find(userId);
    if (!iterator.first) {
        HILOG_INFO("Lock wallpaper already cleared");
        return;
    }
    if (!iterator.second.wallpaperFile.empty()) {
        if (wallpaperType == WALLPAPER_LOCKSCREEN) {
            lockWallpaperMap_.Erase(userId);
        } else if (wallpaperType == WALLPAPER_SYSTEM) {
            systemWallpaperMap_.Erase(userId);
        }
    }
    HILOG_INFO("Clear wallpaper End!");
}

bool WallpaperService::WPCheckCallingPermission(const std::string &permissionName)
{
    bool bflag = false;
    int32_t result;
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    auto tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
    if (tokenType == TOKEN_NATIVE || tokenType == TOKEN_SHELL || tokenType == TOKEN_HAP) {
        result = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    } else {
        HILOG_INFO("Check permission tokenId illegal");
        return false;
    }
    if (result == TypePermissionState::PERMISSION_GRANTED) {
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

    ErrCode errCode = bundleMgrProxy->GetNameForUid(uid, bname);
    if (errCode != ERR_OK) {
        HILOG_ERROR("Get bundle name failed errcode:%{public}d", errCode);
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

int32_t WallpaperService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    int32_t uid = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    const int32_t maxUid = 10000;
    if (uid > maxUid) {
        AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
        auto tokenType = AccessTokenKit::GetTokenTypeFlag(callerToken);
        if (tokenType != TOKEN_NATIVE && tokenType != TOKEN_SHELL) {
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
    std::vector<int32_t> ids;
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        HILOG_ERROR("query active user failed errCode=%{public}d", ret);
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (connection_ == nullptr) {
        connection_ = new WallpaperExtensionAbilityConnection(*this);
    }
    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectExtensionAbility(want, connection_, ids[0]);
    HILOG_INFO("ConnectExtensionAbility errCode=%{public}d", ret);
    return ret;
}

bool WallpaperService::IsSystemApp()
{
    HILOG_INFO("IsSystemApp start");
    uint64_t tokenId = IPCSkeleton::GetCallingFullTokenID();
    return TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

ErrorCode WallpaperService::GetImageFd(int32_t userId, WallpaperType wallpaperType, int32_t &fd)
{
    HILOG_INFO("WallpaperService::GetImageFd start ");
    std::string filePathName;
    if (!GetFileNameFromMap(userId, wallpaperType, FileType::CROP_FILE, filePathName)) {
        return E_DEAL_FAILED;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    fd = open(filePathName.c_str(), O_RDONLY, S_IREAD);
    if (fd < 0) {
        HILOG_ERROR("Open file failed, errno %{public}d", errno);
        ReporterFault(FaultType::LOAD_WALLPAPER_FAULT, FaultCode::RF_FD_INPUT_FAILED);
        return E_DEAL_FAILED;
    }
    HILOG_INFO("fd = %{public}d", fd);
    return E_OK;
}

ErrorCode WallpaperService::GetImageSize(int32_t userId, WallpaperType wallpaperType, int32_t &size)
{
    HILOG_INFO("WallpaperService::GetImageSize start ");
    std::string filePathName;
    HILOG_INFO("userId = %{public}d", userId);
    if (!GetFileNameFromMap(userId, wallpaperType, FileType::CROP_FILE, filePathName)) {
        return E_DEAL_FAILED;
    }

    if (!OHOS::FileExists(filePathName)) {
        HILOG_ERROR("file is not exist!");
        return E_NOT_FOUND;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    FILE *fd = fopen(filePathName.c_str(), "rb");
    if (fd == nullptr) {
        HILOG_ERROR("fopen file failed, errno %{public}d", errno);
        return E_FILE_ERROR;
    }
    int32_t fend = fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    int32_t fset = fseek(fd, 0, SEEK_SET);
    if (size <= 0 || fend != 0 || fset != 0) {
        HILOG_ERROR("ftell file failed or fseek file failed, errno %{public}d", errno);
        fclose(fd);
        return E_FILE_ERROR;
    }
    fclose(fd);
    return E_OK;
}

bool WallpaperService::BlockRetry(int64_t interval, uint32_t maxRetryTimes, std::function<bool()> function)
{
    uint32_t times = 0;
    bool ret = false;
    do {
        times++;
        ret = function();
        if (ret) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        HILOG_INFO("function() is: %d", ret);
    } while (times < maxRetryTimes);
    HILOG_INFO("retry times: %d", times);
    return ret;
}

int32_t WallpaperService::QueryActiveUserId()
{
    std::vector<int32_t> ids;
    ErrCode errCode = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (errCode != ERR_OK || ids.empty()) {
        HILOG_INFO("Query active userid failed, errCode: %{public}d, ", errCode);
        return DEFAULT_USER_ID;
    }
    return ids[0];
}
} // namespace WallpaperMgrService
} // namespace OHOS