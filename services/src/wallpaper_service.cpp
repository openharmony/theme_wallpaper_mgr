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
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "pixel_map.h"
#include "scene_board_judgement.h"
#include "surface.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "uri.h"
#include "uri_permission_manager_client.h"
#include "wallpaper_common.h"
#include "wallpaper_common_event_manager.h"
#include "wallpaper_extension_ability_connection.h"
#include "wallpaper_extension_ability_death_recipient.h"
#include "wallpaper_service_cb_proxy.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace WallpaperMgrService {
REGISTER_SYSTEM_ABILITY_BY_ID(WallpaperService, WALLPAPER_MANAGER_SERVICE_ID, true);

using namespace OHOS::Media;
using namespace OHOS::MiscServices;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AccountSA;

constexpr const char *WALLPAPER_SYSTEM_ORIG = "wallpaper_system_orig";
constexpr const char *WALLPAPER_LOCK_ORIG = "wallpaper_lock_orig";
constexpr const char *LIVE_WALLPAPER_SYSTEM_ORIG = "live_wallpaper_system_orig";
constexpr const char *LIVE_WALLPAPER_LOCK_ORIG = "live_wallpaper_lock_orig";
constexpr const char *CUSTOM_WALLPAPER_LOCK = "custom_wallpaper_lock";
constexpr const char *CUSTOM_WALLPAPER_SYSTEM = "custom_wallpaper_system";
constexpr const char *OHOS_WALLPAPER_BUNDLE_NAME = "com.ohos.launcher";
constexpr const char *SHOW_SYSTEM_SCREEN = "SHOW_SYSTEMSCREEN";
constexpr const char *SHOW_LOCK_SCREEN = "SHOW_LOCKSCREEN";
constexpr const char *SYSTEM_RES_TYPE = "SystemResType";
constexpr const char *LOCKSCREEN_RES_TYPE = "LockScreenResType";
constexpr const char *WALLPAPER_CHANGE = "wallpaperChange";
constexpr const char *COLOR_CHANGE = "colorChange";
constexpr const char *BUNDLE_NAME_KEY = "persist.wallpaper_mgr.bundleName";
constexpr const char *SCENEBOARD_BUNDLE_NAME = "com.ohos.sceneboard";

constexpr int64_t INIT_INTERVAL = 10000L;
constexpr int64_t DELAY_TIME = 1000L;
constexpr int64_t QUERY_USER_ID_INTERVAL = 300L;
constexpr int32_t CONNECT_EXTENSION_INTERVAL = 500000;
constexpr int32_t CONNECT_EXTENSION_MAX_RETRY_TIMES = 360;
constexpr int32_t FOO_MAX_LEN = 52428800;
constexpr int32_t MAX_RETRY_TIMES = 20;
constexpr int32_t QUERY_USER_MAX_RETRY_TIMES = 100;
constexpr int32_t DEFAULT_WALLPAPER_ID = -1;
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t DEFAULT_VALUE = -1;
constexpr int32_t MAX_VIDEO_SIZE = 104857600;
constexpr int32_t MIN_OFFSET = -50;
constexpr int32_t MAX_OFFSET = 50;
constexpr double OFFSET_UNIT = 0.01;
const int CONFIG_LEN = 30;

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
    //InitQueryUserId(QUERY_USER_MAX_RETRY_TIMES);
    std::function<bool()> function = [this]() -> bool { return InitUsersOnBoot(); };
    BlockRetry(QUERY_USER_ID_INTERVAL, QUERY_USER_MAX_RETRY_TIMES, function);
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
    wallpaperEventMap_.clear();
    appBundleName_ = SCENEBOARD_BUNDLE_NAME;
    InitBundleNameParameter();
    InitUserDir(userId);
    LoadSettingsLocked(userId, true);
    InitResources(userId, WALLPAPER_SYSTEM);
    InitResources(userId, WALLPAPER_LOCKSCREEN);
    LoadWallpaperState();
    SaveColor(userId, WALLPAPER_SYSTEM);
    SaveColor(userId, WALLPAPER_LOCKSCREEN);
    HILOG_INFO("WallpaperService::initData --> end ");
}

void WallpaperService::InitBundleNameParameter()
{
    char value[CONFIG_LEN] = "";
    if (GetParameter(BUNDLE_NAME_KEY, "", value, CONFIG_LEN) < 0 || strlen(value) == 0) {
        HILOG_ERROR("No found bundle name from system parameter.");
        return;
    }
    appBundleName_ = value;
    HILOG_INFO("get appBundleName_ :%{public}s", appBundleName_.c_str());
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
    if (!GetFileNameFromMap(userId, wallpaperType, pathName)) {
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
    AAFwk::Want want;
    want.SetElementName(OHOS_WALLPAPER_BUNDLE_NAME, "WallpaperExtAbility");
    ConnectExtensionAbility(want);
    std::string userDir = WALLPAPER_USERID_PATH + std::to_string(userId);
    LoadSettingsLocked(userId, true);
    if (!FileDeal::IsFileExist(userDir)) {
        HILOG_INFO("User file is not exist, userId = %{public}d", userId);
        InitUserDir(userId);
        InitResources(userId, WALLPAPER_SYSTEM);
        InitResources(userId, WALLPAPER_LOCKSCREEN);
    }
    LoadWallpaperState();
    GrantUriPermissionByUserId(userId);
    SendWallpaperChangeEvent(userId, WALLPAPER_SYSTEM);
    SendWallpaperChangeEvent(userId, WALLPAPER_LOCKSCREEN);
    SaveColor(userId, WALLPAPER_SYSTEM);
    SaveColor(userId, WALLPAPER_LOCKSCREEN);
    HILOG_INFO("OnSwitchedUser end, newUserId = %{public}d", userId);
}

void WallpaperService::GrantUriPermissionByUserId(int32_t userId)
{
    WallpaperData systemData;
    WallpaperData lockScreenData;
    if (!GetWallpaperSafeLocked(userId, WALLPAPER_SYSTEM, systemData) ||
        !GetWallpaperSafeLocked(userId, WALLPAPER_LOCKSCREEN, lockScreenData)) {
        return;
    }
    if (systemData.resourceType == PACKAGE) {
        GrantUriPermission(systemData.customPackageUri, appBundleName_);
    }
    if (lockScreenData.resourceType == PACKAGE) {
        GrantUriPermission(lockScreenData.customPackageUri, appBundleName_);
    }
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

bool WallpaperService::GetFileNameFromMap(int32_t userId, WallpaperType wallpaperType, std::string &filePathName)
{
    auto iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                      : lockWallpaperMap_.Find(userId);
    if (!iterator.first) {
        HILOG_ERROR("system wallpaper already cleared");
        return false;
    }
    HILOG_INFO("GetFileNameFromMap resourceType : %{public}d", static_cast<int32_t>(iterator.second.resourceType));
    switch (iterator.second.resourceType) {
        case PICTURE:
            filePathName = iterator.second.wallpaperFile;
            break;
        case VIDEO:
            filePathName = iterator.second.liveWallpaperFile;
            break;
        case DEFAULT:
            filePathName = iterator.second.wallpaperFile;
            break;
        case PACKAGE:
            filePathName = iterator.second.customPackageUri;
            break;
        default:
            filePathName = "";
            break;
    }
    return filePathName != "";
}

bool WallpaperService::GetPictureFileName(int32_t userId, WallpaperType wallpaperType, std::string &filePathName)
{
    auto iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                      : lockWallpaperMap_.Find(userId);
    if (!iterator.first) {
        HILOG_ERROR("system wallpaper already cleared");
        return false;
    }
    filePathName = iterator.second.wallpaperFile;
    HILOG_INFO("GetPictureFileName filePathName : %{public}s", filePathName.c_str());
    return filePathName != "";
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
        WallpaperData wallpaperSystem(userId, wallpaperSystemFilePath + "/" + WALLPAPER_SYSTEM_ORIG);
        wallpaperSystem.liveWallpaperFile = wallpaperSystemFilePath + "/" + LIVE_WALLPAPER_SYSTEM_ORIG;
        wallpaperSystem.customPackageUri = wallpaperSystemFilePath + "/" + CUSTOM_WALLPAPER_SYSTEM;
        wallpaperSystem.allowBackup = true;
        wallpaperSystem.resourceType = PICTURE;
        wallpaperSystem.wallpaperId = DEFAULT_WALLPAPER_ID;
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperSystem);
    }
    if (!lockWallpaperMap_.Contains(userId)) {
        HILOG_INFO("lockWallpaperMap_ does not Contains userId");
        std::string wallpaperLockScreenFilePath = GetWallpaperDir(userId, WALLPAPER_LOCKSCREEN);
        WallpaperData wallpaperLock(userId, wallpaperLockScreenFilePath + "/" + WALLPAPER_LOCK_ORIG);
        wallpaperLock.liveWallpaperFile = wallpaperLockScreenFilePath + "/" + LIVE_WALLPAPER_LOCK_ORIG;
        wallpaperLock.customPackageUri = wallpaperLockScreenFilePath + "/" + CUSTOM_WALLPAPER_LOCK;
        wallpaperLock.allowBackup = true;
        wallpaperLock.resourceType = PICTURE;
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
    if (!CheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER)) {
        HILOG_ERROR("GetPixelMap no get permission!");
        return E_NO_PERMISSION;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN) &&
        wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }
    auto type = static_cast<WallpaperType>(wallpaperType);
    int32_t userId = QueryActiveUserId();
    if (GetResType(userId, type) == WallpaperResourceType::PACKAGE) {
        HILOG_ERROR("Current user's wallpaper is custom package");
        wallpaperFd = -1; // -1: invalid file description
        return E_OK;
    }
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    ErrorCode ret = GetImageFd(userId, type, wallpaperFd);
    HILOG_INFO("GetImageFd fd:%{public}d, ret:%{public}d", wallpaperFd, ret);
    return ret;
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
    if (!GetPictureFileName(userId, wallpaperType, pathName)) {
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

    pictureHeight_ = wallpaperPixelMap->GetHeight();
    pictureWidth_ = wallpaperPixelMap->GetWidth();

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
    OnColorsChange(wallpaperType, color);
    return true;
}

ErrorCode WallpaperService::SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    ErrorCode wallpaperErrorCode = SetWallpaper(fd, wallpaperType, length, PICTURE);
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

ErrorCode WallpaperService::SetWallpaperBackupData(int32_t userId, WallpaperResourceType resourceType,
    const std::string &uriOrPixelMap, WallpaperType wallpaperType)
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
    wallpaperData.resourceType = resourceType;
    wallpaperData.wallpaperId = MakeWallpaperIdLocked();
    std::string wallpaperFile = resourceType == VIDEO ? wallpaperData.liveWallpaperFile : wallpaperData.wallpaperFile;
    if (!FileDeal::CopyFile(uriOrPixelMap, wallpaperFile)) {
        HILOG_ERROR("CopyFile failed !");
        return E_DEAL_FAILED;
    }
    if (!FileDeal::DeleteFile(uriOrPixelMap)) {
        return E_DEAL_FAILED;
    }
    if (!SaveWallpaperState(userId, wallpaperType)) {
        HILOG_ERROR("Save wallpaper state failed!");
    }
    if (wallpaperType == WALLPAPER_SYSTEM) {
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperData);
        ReporterUsageTimeStatistic();
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
        ReporterUsageTimeStatistic();
    }
    if (!SendWallpaperChangeEvent(userId, wallpaperType)) {
        HILOG_ERROR("Send wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    return E_OK;
}

WallpaperResourceType WallpaperService::GetResType(int32_t userId, WallpaperType wallpaperType)
{
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        auto iterator = lockWallpaperMap_.Find(userId);
        if (iterator.first) {
            return iterator.second.resourceType;
        }
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        auto iterator = systemWallpaperMap_.Find(userId);
        if (iterator.first) {
            return iterator.second.resourceType;
        }
    }
    return WallpaperResourceType::DEFAULT;
}

ErrorCode WallpaperService::SendEvent(const std::string &eventType)
{
    HILOG_DEBUG("Send event start.");
    if (!CheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER)) {
        HILOG_ERROR("Send event not set permission!");
        return E_NO_PERMISSION;
    }

    int32_t userId = QueryActiveUserId();
    WallpaperType wallpaperType;
    WallpaperData data;
    if (eventType == SHOW_SYSTEM_SCREEN) {
        wallpaperType = WALLPAPER_SYSTEM;
    } else if (eventType == SHOW_LOCK_SCREEN) {
        wallpaperType = WALLPAPER_LOCKSCREEN;
    } else {
        HILOG_ERROR("Event type error!");
        return E_PARAMETERS_INVALID;
    }

    if (!GetWallpaperSafeLocked(userId, wallpaperType, data)) {
        HILOG_ERROR("Get wallpaper safe locked failed!");
        return E_PARAMETERS_INVALID;
    }
    ReporterUsageTimeStatistic();
    std::string uri;
    GetFileNameFromMap(userId, WALLPAPER_SYSTEM, uri);
    WallpaperChanged(wallpaperType, data.resourceType, uri);
    return E_OK;
}

bool WallpaperService::SendWallpaperChangeEvent(int32_t userId, WallpaperType wallpaperType)
{
    WallpaperData wallpaperData;
    if (!GetWallpaperSafeLocked(userId, wallpaperType, wallpaperData)) {
        HILOG_ERROR("GetWallpaperSafeLocked failed !");
        return false;
    }
    shared_ptr<WallpaperCommonEventManager> wallpaperCommonEventManager = make_shared<WallpaperCommonEventManager>();
    if (wallpaperType == WALLPAPER_SYSTEM) {
        HILOG_INFO("Send wallpaper system setting message");
        wallpaperCommonEventManager->SendWallpaperSystemSettingMessage(wallpaperData.resourceType);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        HILOG_INFO("Send wallpaper lock setting message");
        wallpaperCommonEventManager->SendWallpaperLockSettingMessage(wallpaperData.resourceType);
    }
    HILOG_INFO("SetWallpaperBackupData callbackProxy_->OnCall start");
    if (callbackProxy_ != nullptr && wallpaperData.resourceType == PICTURE) {
        callbackProxy_->OnCall(wallpaperType);
    }
    std::string uri;
    WallpaperChanged(wallpaperType, wallpaperData.resourceType, uri);
    return true;
}

ErrorCode WallpaperService::SetVideo(int32_t fd, int32_t wallpaperType, int32_t length)
{
    if (!IsSystemApp()) {
        HILOG_ERROR("current app is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    StartAsyncTrace(HITRACE_TAG_MISC, "SetVideo", static_cast<int32_t>(TraceTaskId::SET_VIDEO));
    ErrorCode wallpaperErrorCode = SetWallpaper(fd, wallpaperType, length, VIDEO);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetVideo", static_cast<int32_t>(TraceTaskId::SET_VIDEO));
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::SetCustomWallpaper(const std::string &uri, int32_t type)
{
    if (!IsSystemApp()) {
        HILOG_ERROR("current app is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    if (!CheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER)) {
        HILOG_INFO("SetWallpaper no set permission!");
        return E_NO_PERMISSION;
    }
    if (type != static_cast<int32_t>(WALLPAPER_LOCKSCREEN) && type != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }
    StartAsyncTrace(HITRACE_TAG_MISC, "SetCustomWallpaper", static_cast<int32_t>(TraceTaskId::SET_CUSTOM_WALLPAPER));
    int32_t userId = QueryActiveUserId();
    WallpaperType wallpaperType = static_cast<WallpaperType>(type);
    WallpaperData wallpaperData;
    if (!GetWallpaperSafeLocked(userId, wallpaperType, wallpaperData)) {
        HILOG_ERROR("GetWallpaper data failed!");
        return E_DEAL_FAILED;
    }
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        HILOG_ERROR("SceneBoard is not Enabled");
        return E_NO_PERMISSION;
    }
    wallpaperData.resourceType = PACKAGE;
    wallpaperData.wallpaperId = MakeWallpaperIdLocked();
    if (wallpaperType == WALLPAPER_SYSTEM) {
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    }
    if (!SaveWallpaperState(userId, wallpaperType)) {
        HILOG_ERROR("Save wallpaper state failed!");
    }
    if (!SendWallpaperChangeEvent(userId, wallpaperType)) {
        HILOG_ERROR("Send wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetCustomWallpaper", static_cast<int32_t>(TraceTaskId::SET_CUSTOM_WALLPAPER));
    return E_OK;
}

void WallpaperService::ReporterUsageTimeStatistic()
{
    int32_t uid = static_cast<int32_t>(IPCSkeleton::GetCallingUid());
    std::string bundleName;
    bool bRet = GetBundleNameByUid(uid, bundleName);
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
    if (!CheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER)) {
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
    // current user's wallpaper is live video, not image
    WallpaperResourceType resType = GetResType(userId, type);
    if (resType != PICTURE) {
        HILOG_ERROR("Current user's wallpaper is live video, not image");
        fdInfo.size = 0; // 0: empty file size
        fdInfo.fd = -1;  // -1: invalid file description
        return E_OK;
    }
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
    bool bFlag = CheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    return bFlag;
}

bool WallpaperService::IsOperationAllowed()
{
    HILOG_INFO("IsOperationAllowed wallpaper Start!");
    bool bFlag = CheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
    return bFlag;
}

ErrorCode WallpaperService::ResetWallpaper(int32_t wallpaperType)
{
    HILOG_INFO("reset wallpaper Start!");
    bool permissionSet = CheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER);
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
    if (!CheckUserPermissionById(userId)) {
        return E_USER_IDENTITY_ERROR;
    }
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
    if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    } else if (wallpaperType == WALLPAPER_SYSTEM) {
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    }
    if (!SendWallpaperChangeEvent(userId, wallpaperType)) {
        HILOG_ERROR("Send wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    SaveColor(userId, wallpaperType);
    return E_OK;
}

ErrorCode WallpaperService::On(const std::string &type, sptr<IWallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperService::On in");
    if (listener == nullptr) {
        HILOG_ERROR("WallpaperService::On listener is null");
        return E_DEAL_FAILED;
    }
    if (type == WALLPAPER_CHANGE && !IsSystemApp()) {
        HILOG_ERROR("current app is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
    wallpaperEventMap_[type].insert_or_assign(IPCSkeleton::GetCallingTokenID(), listener);
    HILOG_DEBUG("WallpaperService::On out");
    return E_OK;
}

ErrorCode WallpaperService::Off(const std::string &type, sptr<IWallpaperEventListener> listener)
{
    HILOG_DEBUG("WallpaperService::Off in");
    (void)listener;
    if (type == WALLPAPER_CHANGE && !IsSystemApp()) {
        HILOG_ERROR("current app is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
    auto iter = wallpaperEventMap_.find(type);
    if (iter != wallpaperEventMap_.end()) {
        auto it = iter->second.find(IPCSkeleton::GetCallingTokenID());
        if (it != iter->second.end()) {
            it->second = nullptr;
            iter->second.erase(it);
        }
    }
    HILOG_DEBUG("WallpaperService::Off out");
    return E_OK;
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
    if (!iterator.second.wallpaperFile.empty() || !iterator.second.liveWallpaperFile.empty()) {
        if (wallpaperType == WALLPAPER_LOCKSCREEN) {
            lockWallpaperMap_.Erase(userId);
        } else if (wallpaperType == WALLPAPER_SYSTEM) {
            systemWallpaperMap_.Erase(userId);
        }
    }
    HILOG_INFO("Clear wallpaper End!");
}

bool WallpaperService::CheckCallingPermission(const std::string &permissionName)
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

bool WallpaperService::GetBundleNameByUid(std::int32_t uid, std::string &bname)
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
    if (connection_ == nullptr) {
        connection_ = new WallpaperExtensionAbilityConnection(*this);
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectExtensionAbility(want, connection_, DEFAULT_VALUE);
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
    if (!GetFileNameFromMap(userId, wallpaperType, filePathName)) {
        return E_DEAL_FAILED;
    }
    if (GetResType(userId, wallpaperType) == WallpaperResourceType::PACKAGE) {
        HILOG_INFO("The current wallpaper is a custom wallpaper");
        return E_OK;
    }
    {
        std::lock_guard<std::mutex> lock(mtx_);
        fd = open(filePathName.c_str(), O_RDONLY, S_IREAD);
    }
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
    if (!GetPictureFileName(userId, wallpaperType, filePathName)) {
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
        HILOG_ERROR("Query active userid failed, errCode: %{public}d, ", errCode);
        return DEFAULT_USER_ID;
    }
    return ids[0];
}

bool WallpaperService::CheckUserPermissionById(int32_t userId)
{
    OsAccountInfo osAccountInfo;
    ErrCode errCode = OsAccountManager::QueryOsAccountById(userId, osAccountInfo);
    if (errCode != ERR_OK) {
        HILOG_ERROR("Query os account info failed, errCode: %{public}d, ", errCode);
        return false;
    }
    HILOG_INFO("osAccountInfo GetType: %{public}d", static_cast<int32_t>(osAccountInfo.GetType()));
    if (osAccountInfo.GetType() == OsAccountType::GUEST) {
        HILOG_ERROR("The guest does not have permissions");
        return false;
    }
    return true;
}

ErrorCode WallpaperService::SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length,
    WallpaperResourceType resourceType)
{
    ErrorCode errCode = CheckValid(wallpaperType, length, resourceType);
    if (errCode != E_OK) {
        return errCode;
    }
    std::string uri = wallpaperTmpFullPath_;
    char *paperBuf = new (std::nothrow) char[length]();
    if (paperBuf == nullptr) {
        return E_NO_MEMORY;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    if (read(fd, paperBuf, length) <= 0) {
        HILOG_ERROR("read fd failed");
        delete[] paperBuf;
        return E_DEAL_FAILED;
    }
    mode_t mode = S_IRUSR | S_IWUSR;
    int32_t fdw = open(uri.c_str(), O_WRONLY | O_CREAT, mode);
    if (fdw < 0) {
        HILOG_ERROR("Open wallpaper tmpFullPath failed, errno %{public}d", errno);
        delete[] paperBuf;
        return E_DEAL_FAILED;
    }
    if (write(fdw, paperBuf, length) <= 0) {
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
    if (!CheckUserPermissionById(userId)) {
        return E_USER_IDENTITY_ERROR;
    }
    ErrorCode wallpaperErrorCode = SetWallpaperBackupData(userId, resourceType, uri, type);
    if (resourceType == PICTURE) {
        SaveColor(userId, type);
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::checkSetOffsetPermission()
{
    if (!IsSystemApp()) {
        HILOG_ERROR("current app is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return E_OK;
}

ErrorCode WallpaperService::SetOffset(int32_t xOffset, int32_t yOffset)
{
    HILOG_DEBUG("Current xOffset: %{public}d, yOffset: %{public}d", xOffset, yOffset);
    if (!IsSystemApp()) {
        HILOG_ERROR("current app is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }

    int32_t userId = QueryActiveUserId();
    WallpaperData data;
    if (!GetWallpaperSafeLocked(userId, WALLPAPER_SYSTEM, data)) {
        HILOG_ERROR("Get wallpaper safe locked failed!");
        return E_NOT_FOUND;
    }

    if (data.resourceType != PICTURE) {
        HILOG_ERROR("Live wallpaper not support offset.");
        return E_NOT_FOUND;
    }

    if ((xOffset < MIN_OFFSET || xOffset > MAX_OFFSET) || (yOffset < MIN_OFFSET || yOffset > MAX_OFFSET)) {
        HILOG_ERROR("Current xOffset %{public}d or yOffset %{public}d is invalid value, must be between -50 and 50",
            xOffset, yOffset);
        return E_PARAMETERS_INVALID;
    }
    int32_t pyScrWidth = 0;
    GetWallpaperMinWidth(pyScrWidth);
    int32_t pyScrHeight = 0;
    GetWallpaperMinHeight(pyScrHeight);
    int remainWidthSize = pictureWidth_ - pyScrWidth;
    int remainHeightSize = pictureHeight_ - pyScrHeight;
    HILOG_DEBUG("pictureWidth: %{public}d, pyScrWidth: %{public}d, remainWidthSize: %{public}d, remainHeightSize: "
                "%{public}d",
        pictureWidth_, pyScrWidth, remainWidthSize, remainHeightSize);
    int32_t xOffsetPixel = static_cast<int32_t>(remainWidthSize * xOffset * OFFSET_UNIT);
    int32_t yOffsetPixel = static_cast<int32_t>(remainHeightSize * yOffset * OFFSET_UNIT);

    if (callbackProxy_ != nullptr) {
        HILOG_DEBUG("Set callbackProxy OnOffsetCall start %{public}d, %{public}d", xOffsetPixel, yOffsetPixel);
        callbackProxy_->OnOffsetCall(xOffsetPixel, yOffsetPixel);
    }
    return E_OK;
}

void WallpaperService::OnColorsChange(WallpaperType wallpaperType, const ColorManager::Color &color)
{
    std::vector<uint64_t> colors;
    if (wallpaperType == WALLPAPER_SYSTEM && !CompareColor(systemWallpaperColor_, color)) {
        systemWallpaperColor_ = color.PackValue();
        colors.emplace_back(systemWallpaperColor_);
        NotifyColorChange(colors, WALLPAPER_SYSTEM);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN && !CompareColor(lockWallpaperColor_, color)) {
        lockWallpaperColor_ = color.PackValue();
        colors.emplace_back(lockWallpaperColor_);
        NotifyColorChange(colors, WALLPAPER_LOCKSCREEN);
    }
}

ErrorCode WallpaperService::CheckValid(int32_t wallpaperType, int32_t length, WallpaperResourceType resourceType)
{
    if (!CheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER)) {
        HILOG_INFO("SetWallpaper no set permission!");
        return E_NO_PERMISSION;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN) &&
        wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }

    int maxLength = resourceType == VIDEO ? MAX_VIDEO_SIZE : FOO_MAX_LEN;
    if (length <= 0 || length > maxLength) {
        return E_PARAMETERS_INVALID;
    }
    return E_OK;
}

bool WallpaperService::WallpaperChanged(WallpaperType wallpaperType, WallpaperResourceType resType,
    const std::string &uri)
{
    std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
    auto it = wallpaperEventMap_.find(WALLPAPER_CHANGE);
    if (it != wallpaperEventMap_.end()) {
        for (auto iter = it->second.begin(); iter != it->second.end(); iter++) {
            if (iter->second == nullptr) {
                continue;
            }
            iter->second->OnWallpaperChange(wallpaperType, resType, uri);
        }
        return true;
    }
    return false;
}

void WallpaperService::NotifyColorChange(const std::vector<uint64_t> &colors, const WallpaperType &wallpaperType)
{
    std::lock_guard<std::mutex> autoLock(listenerMapMutex_);
    auto it = wallpaperEventMap_.find(COLOR_CHANGE);
    if (it != wallpaperEventMap_.end()) {
        for (auto iter = it->second.begin(); iter != it->second.end(); iter++) {
            if (iter->second == nullptr) {
                continue;
            }
            iter->second->OnColorsChange(colors, wallpaperType);
        }
    }
}

bool WallpaperService::SaveWallpaperState(int32_t userId, WallpaperType wallpaperType)
{
    WallpaperData systemData;
    WallpaperData lockScreenData;
    if (!GetWallpaperSafeLocked(userId, WALLPAPER_SYSTEM, systemData) ||
        !GetWallpaperSafeLocked(userId, WALLPAPER_LOCKSCREEN, lockScreenData)) {
        return false;
    }
    nlohmann::json root;
    root[SYSTEM_RES_TYPE] = static_cast<int32_t>(systemData.resourceType);
    root[LOCKSCREEN_RES_TYPE] = static_cast<int32_t>(lockScreenData.resourceType);
    std::string json = root.dump();
    if (json.empty()) {
        HILOG_ERROR("write user config file failed. because json content is empty.");
        return false;
    }

    std::string userPath = WALLPAPER_USERID_PATH + std::to_string(userId) + "/wallpapercfg";
    mode_t mode = S_IRUSR | S_IWUSR;
    int fd = open(userPath.c_str(), O_CREAT | O_WRONLY | O_SYNC, mode);
    if (fd <= 0) {
        HILOG_ERROR("open user config file failed.");
        return false;
    }
    ssize_t size = write(fd, json.c_str(), json.size());
    if (size <= 0) {
        HILOG_ERROR("write user config file failed.");
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

void WallpaperService::LoadWallpaperState()
{
    int32_t userId = QueryActiveUserId();
    std::string userPath = WALLPAPER_USERID_PATH + std::to_string(userId) + "/wallpapercfg";
    int fd = open(userPath.c_str(), O_RDONLY, S_IREAD);
    if (fd <= 0) {
        HILOG_ERROR("open user config file failed.");
        return;
    }
    const size_t len = 255;
    char buf[len] = { 0 };
    ssize_t size = read(fd, buf, len);
    if (size <= 0) {
        HILOG_ERROR("read user config file failed.");
        close(fd);
        return;
    }
    close(fd);

    if (buf[0] == '\0') {
        return;
    }
    WallpaperData systemData;
    WallpaperData lockScreenData;
    if (!GetWallpaperSafeLocked(userId, WALLPAPER_SYSTEM, systemData) ||
        !GetWallpaperSafeLocked(userId, WALLPAPER_LOCKSCREEN, lockScreenData)) {
        return;
    }
    auto root = nlohmann::json::parse(buf);
    if (root.contains(SYSTEM_RES_TYPE)) {
        systemData.resourceType = static_cast<WallpaperResourceType>(root[SYSTEM_RES_TYPE].get<int>());
    }
    if (root.contains(LOCKSCREEN_RES_TYPE)) {
        lockScreenData.resourceType = static_cast<WallpaperResourceType>(root[LOCKSCREEN_RES_TYPE].get<int>());
    }
}

int32_t WallpaperService::GrantUriPermission(const std::string &path, const std::string &bundleName)
{
    Uri uri(path);
    return AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermission(uri,
        AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION, bundleName, 0);
}
} // namespace WallpaperMgrService
} // namespace OHOS