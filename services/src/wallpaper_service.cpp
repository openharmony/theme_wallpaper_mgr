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

#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <sstream>

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
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "uri.h"
#include "wallpaper_common.h"
#include "wallpaper_common_event_manager.h"
#include "wallpaper_manager_common_info.h"
#include "wallpaper_service_cb_proxy.h"
#include "want.h"
#include "window.h"

#ifndef THEME_SERVICE
#include "ability_manager_client.h"
#include "wallpaper_extension_ability_death_recipient.h"
#endif

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
constexpr const char *CUSTOM_WALLPAPER_LOCK = "custom_lock.zip";
constexpr const char *CUSTOM_WALLPAPER_SYSTEM = "custom_system.zip";
constexpr const char *OHOS_WALLPAPER_BUNDLE_NAME = "com.ohos.launcher";
constexpr const char *SHOW_SYSTEM_SCREEN = "SHOW_SYSTEMSCREEN";
constexpr const char *SHOW_LOCK_SCREEN = "SHOW_LOCKSCREEN";
constexpr const char *SYSTEM_RES_TYPE = "SystemResType";
constexpr const char *LOCKSCREEN_RES_TYPE = "LockScreenResType";
constexpr const char *WALLPAPER_CHANGE = "wallpaperChange";
constexpr const char *COLOR_CHANGE = "colorChange";
constexpr const char *BUNDLE_NAME_KEY = "persist.wallpaper_mgr.bundleName";
constexpr const char *SCENEBOARD_BUNDLE_NAME = "com.ohos.sceneboard";

constexpr const char *WALLPAPER_USERID_PATH = "/data/service/el1/public/wallpaper/";
constexpr const char *WALLPAPER_SYSTEM_DIRNAME = "system";
constexpr const char *WALLPAPER_TMP_DIRNAME = "fwsettmp";
constexpr const char *WALLPAPER_LOCKSCREEN_DIRNAME = "lockscreen";
constexpr const char *WALLPAPER_DEFAULT_FILEFULLPATH = "/system/etc/wallpaperdefault.jpeg";
constexpr const char *WALLPAPER_DEFAULT_LOCK_FILEFULLPATH = "/system/etc/wallpaperlockdefault.jpeg";
constexpr const char *WALLPAPER_PREFABRICATE_FILEFULLPATH = "/sys_prod/media/themes/wallpaperdefault.jpeg";
constexpr const char *WALLPAPER_PREFABRICATE_LOCK_FILEFULLPATH = "/sys_prod/media/themes/wallpaperlockdefault.jpeg";
constexpr const char *WALLPAPER_CROP_PICTURE = "crop_file";

constexpr int64_t INIT_INTERVAL = 10000L;
constexpr int64_t DELAY_TIME = 1000L;
constexpr int64_t QUERY_USER_ID_INTERVAL = 300L;
constexpr int32_t FOO_MAX_LEN = 52428800;
constexpr int32_t MAX_RETRY_TIMES = 20;
constexpr int32_t QUERY_USER_MAX_RETRY_TIMES = 100;
constexpr int32_t DEFAULT_WALLPAPER_ID = -1;
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t MAX_VIDEO_SIZE = 104857600;
constexpr int32_t OPTION_QUALITY = 100;
const int CONFIG_LEN = 30;

#ifndef THEME_SERVICE
constexpr int32_t CONNECT_EXTENSION_INTERVAL = 100;
constexpr int32_t DEFAULT_VALUE = -1;
constexpr int32_t CONNECT_EXTENSION_MAX_RETRY_TIMES = 50;
#endif

std::mutex WallpaperService::instanceLock_;

sptr<WallpaperService> WallpaperService::instance_;

std::shared_ptr<AppExecFwk::EventHandler> WallpaperService::serviceHandler_;

WallpaperService::WallpaperService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate), WallpaperServiceStub(true),
      state_(ServiceRunningState::STATE_NOT_START)
{
}

WallpaperService::WallpaperService() : WallpaperServiceStub(true), state_(ServiceRunningState::STATE_NOT_START)
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
#ifndef THEME_SERVICE
    StartExtensionAbility(CONNECT_EXTENSION_MAX_RETRY_TIMES);
#endif
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
    auto cmd = std::make_shared<Command>(std::vector<std::string>({ "-all" }), "Show all",
        [this](const std::vector<std::string> &input, std::string &output) -> bool {
            output.append(
                "WallpaperExtensionAbility\t: ExtensionInfo{" + std::string(OHOS_WALLPAPER_BUNDLE_NAME) + "}\n");
            return true;
        });
    DumpHelper::GetInstance().RegisterCommand(cmd);
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
}

void WallpaperService::OnStop()
{
    HILOG_INFO("OnStop started.");
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return;
    }
    serviceHandler_ = nullptr;
#ifndef THEME_SERVICE
    connection_ = nullptr;
#endif
    recipient_ = nullptr;
    extensionRemoteObject_ = nullptr;
    if (subscriber_ != nullptr) {
        bool unSubscribeResult = OHOS::EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
        HILOG_INFO("UnregisterSubscriber end, unSubscribeResult = %{public}d", unSubscribeResult);
    }
    state_ = ServiceRunningState::STATE_NOT_START;
}

void WallpaperService::InitData()
{
    HILOG_INFO("WallpaperService::initData --> start ");
    wallpaperId_ = DEFAULT_WALLPAPER_ID;
    int32_t userId = DEFAULT_USER_ID;
    systemWallpaperMap_.Clear();
    lockWallpaperMap_.Clear();
    wallpaperTmpFullPath_ = std::string(WALLPAPER_USERID_PATH) + std::string(WALLPAPER_TMP_DIRNAME);
    wallpaperCropPath_ = std::string(WALLPAPER_USERID_PATH) + std::string(WALLPAPER_CROP_PICTURE);
    systemWallpaperColor_ = 0;
    lockWallpaperColor_ = 0;
    currentUserId_ = userId;
    wallpaperEventMap_.clear();
    appBundleName_ = SCENEBOARD_BUNDLE_NAME;
    InitBundleNameParameter();
    InitUserDir(userId);
    LoadSettingsLocked(userId);
    InitResources(userId, WALLPAPER_SYSTEM);
    InitResources(userId, WALLPAPER_LOCKSCREEN);
    LoadWallpaperState();
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

#ifndef THEME_SERVICE
void WallpaperService::AddWallpaperExtensionDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject != nullptr) {
        std::lock_guard<std::mutex> lock(remoteObjectMutex_);
        IPCObjectProxy *proxy = reinterpret_cast<IPCObjectProxy *>(remoteObject.GetRefPtr());
        if (recipient_ == nullptr) {
            recipient_ = sptr<IRemoteObject::DeathRecipient>(new WallpaperExtensionAbilityDeathRecipient(*this));
        }
        if (proxy != nullptr && !proxy->IsObjectDead()) {
            HILOG_INFO("get remoteObject succeed");
            proxy->AddDeathRecipient(recipient_);
            extensionRemoteObject_ = remoteObject;
        }
    }
}
#endif

void WallpaperService::RemoveExtensionDeathRecipient()
{
    if (extensionRemoteObject_ != nullptr && recipient_ != nullptr) {
        HILOG_INFO("Remove Extension DeathRecipient");
        std::lock_guard<std::mutex> lock(remoteObjectMutex_);
        extensionRemoteObject_->RemoveDeathRecipient(recipient_);
        recipient_ = nullptr;
        extensionRemoteObject_ = nullptr;
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

#ifndef THEME_SERVICE
void WallpaperService::StartExtensionAbility(int32_t times)
{
    times--;
    bool ret = ConnectExtensionAbility();
    if (!ret && times > 0 && serviceHandler_ != nullptr) {
        HILOG_ERROR("StartExtensionAbilty failed, remainder of the times: %{public}d", times);
        auto callback = [this, times]() { StartExtensionAbility(times); };
        serviceHandler_->PostTask(callback, CONNECT_EXTENSION_INTERVAL);
    }
}
#endif

bool WallpaperService::InitUsersOnBoot()
{
    std::vector<int32_t> userIds;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds) != ERR_OK || userIds.empty()) {
        HILOG_DEBUG("WallpaperService: failed to get current userIds");
        return false;
    }
    HILOG_INFO("WallpaperService::get current userIds success, Current userId: %{public}d", userIds[0]);
    for (auto userId : userIds) {
        InitUserDir(userId);
        LoadSettingsLocked(userId);
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
    LoadSettingsLocked(userId);
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
            WALLPAPER_SYSTEM_DIRNAME);
        return false;
    }
    std::string wallpaperLockScreenFilePath = userDir + "/" + WALLPAPER_LOCKSCREEN_DIRNAME;
    if (!FileDeal::Mkdir(wallpaperLockScreenFilePath)) {
        HILOG_ERROR("Failed to create destination wallpaper lockscreen path, userId:%{public}d, type:%{public}s",
            userId, WALLPAPER_LOCKSCREEN_DIRNAME);
        return false;
    }
    return true;
}

bool WallpaperService::RestoreUserResources(const WallpaperData &wallpaperData, WallpaperType wallpaperType)
{
    std::string wallpaperDefaultPath = wallpaperType == WallpaperType::WALLPAPER_SYSTEM
                                           ? WALLPAPER_DEFAULT_FILEFULLPATH
                                           : WALLPAPER_DEFAULT_LOCK_FILEFULLPATH;
    if (wallpaperType == WallpaperType::WALLPAPER_SYSTEM) {
        if (FileDeal::IsFileExist(WALLPAPER_PREFABRICATE_FILEFULLPATH)) {
            wallpaperDefaultPath = WALLPAPER_PREFABRICATE_FILEFULLPATH;
        }
    } else {
        if (FileDeal::IsFileExist(WALLPAPER_PREFABRICATE_LOCK_FILEFULLPATH)) {
            wallpaperDefaultPath = WALLPAPER_PREFABRICATE_LOCK_FILEFULLPATH;
        }
    }
    if (!FileDeal::IsFileExist(wallpaperDefaultPath)
        || !FileDeal::IsFileExist(GetWallpaperDir(wallpaperData.userId, wallpaperType))) {
        HILOG_ERROR("Copy file path is not exist");
        return false;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    if (!FileDeal::CopyFile(wallpaperDefaultPath, wallpaperData.wallpaperFile)) {
        HILOG_ERROR("CopyFile WALLPAPER_DEFAULT_FILEFULLPATH failed");
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
    if (userId == currentUserId_) {
        HILOG_ERROR("userId not switch, userId = %{public}d", userId);
        return;
    }
    currentUserId_ = userId;
    RemoveExtensionDeathRecipient();
#ifndef THEME_SERVICE
    ConnectExtensionAbility();
#endif
    std::string userDir = WALLPAPER_USERID_PATH + std::to_string(userId);
    std::string systemFile =
        WALLPAPER_USERID_PATH + std::to_string(userId) + "/" + WALLPAPER_SYSTEM_DIRNAME + "/" + WALLPAPER_SYSTEM_ORIG;
    std::string lockFile = WALLPAPER_USERID_PATH + std::to_string(userId) + "/" + WALLPAPER_LOCKSCREEN_DIRNAME + "/"
                           + WALLPAPER_LOCK_ORIG;
    LoadSettingsLocked(userId);
    if (!FileDeal::IsFileExist(userDir)) {
        HILOG_INFO("User file is not exist, userId = %{public}d", userId);
        InitUserDir(userId);
        InitResources(userId, WALLPAPER_SYSTEM);
        InitResources(userId, WALLPAPER_LOCKSCREEN);
    } else {
        if (!FileDeal::IsFileExist(systemFile)) {
            HILOG_INFO("systemFile is not exist, userId = %{public}d", userId);
            InitResources(userId, WALLPAPER_SYSTEM);
        }
        if (!FileDeal::IsFileExist(lockFile)) {
            HILOG_INFO("lockFile is not exist, userId = %{public}d", userId);
            InitResources(userId, WALLPAPER_LOCKSCREEN);
        }
    }
    LoadWallpaperState();
    SendWallpaperChangeEvent(userId, WALLPAPER_SYSTEM);
    SendWallpaperChangeEvent(userId, WALLPAPER_LOCKSCREEN);
    SaveColor(userId, WALLPAPER_SYSTEM);
    SaveColor(userId, WALLPAPER_LOCKSCREEN);
    HILOG_INFO("OnSwitchedUser end, newUserId = %{public}d", userId);
}

void WallpaperService::OnBootPhase()
{
    HILOG_INFO("WallpaperService OnBootPhase");
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
    HILOG_DEBUG("GetFileNameFromMap resourceType : %{public}d", static_cast<int32_t>(iterator.second.resourceType));
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
        HILOG_INFO("WallpaperType:%{public}d ,WallpaperMap not found userId: %{public}d", wallpaperType, userId);
        OnInitUser(userId);
        iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                     : lockWallpaperMap_.Find(userId);
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

void WallpaperService::LoadSettingsLocked(int32_t userId)
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
}

ErrorCode WallpaperService::GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    if (wallpaperType == WALLPAPER_SYSTEM) {
        colors.emplace_back(systemWallpaperColor_);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        colors.emplace_back(lockWallpaperColor_);
    }
    HILOG_INFO("GetColors Service End!");
    return E_OK;
}

ErrorCode WallpaperService::GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors)
{
    if (!IsSystemApp()) {
        HILOG_ERROR("CallingApp is not SystemApp");
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
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN)
        && wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }
    auto type = static_cast<WallpaperType>(wallpaperType);
    int32_t userId = QueryActiveUserId();
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

ErrorCode WallpaperService::SetWallpaperByPixelMap(
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    ErrorCode wallpaperErrorCode = SetWallpaperByPixelMap(pixelMap, wallpaperType, PICTURE);
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetWallpaper", static_cast<int32_t>(TraceTaskId::SET_WALLPAPER));
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::SetWallpaperV9(int32_t fd, int32_t wallpaperType, int32_t length)
{
    if (!IsSystemApp()) {
        HILOG_ERROR("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return SetWallpaper(fd, wallpaperType, length);
}

ErrorCode WallpaperService::SetWallpaperV9ByPixelMap(
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType)
{
    if (!IsSystemApp()) {
        HILOG_INFO("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    return SetWallpaperByPixelMap(pixelMap, wallpaperType);
}

ErrorCode WallpaperService::SetWallpaperBackupData(
    int32_t userId, WallpaperResourceType resourceType, const std::string &uriOrPixelMap, WallpaperType wallpaperType)
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
    std::string wallpaperFile;
    WallpaperService::GetWallpaperFile(resourceType, wallpaperData, wallpaperFile);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!FileDeal::CopyFile(uriOrPixelMap, wallpaperFile)) {
            HILOG_ERROR("CopyFile failed !");
            return E_DEAL_FAILED;
        }
        if (!FileDeal::DeleteFile(uriOrPixelMap)) {
            return E_DEAL_FAILED;
        }
    }
    if (!SaveWallpaperState(userId, wallpaperType, resourceType)) {
        HILOG_ERROR("Save wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    if (wallpaperType == WALLPAPER_SYSTEM) {
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    }
    if (!SendWallpaperChangeEvent(userId, wallpaperType)) {
        HILOG_ERROR("Send wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    return E_OK;
}

void WallpaperService::GetWallpaperFile(
    WallpaperResourceType resourceType, const WallpaperData &wallpaperData, std::string &wallpaperFile)
{
    switch (resourceType) {
        case PICTURE:
            wallpaperFile = wallpaperData.wallpaperFile;
            break;
        case DEFAULT:
            wallpaperFile = wallpaperData.wallpaperFile;
            break;
        case VIDEO:
            wallpaperFile = wallpaperData.liveWallpaperFile;
            break;
        case PACKAGE:
            wallpaperFile = wallpaperData.customPackageUri;
            break;
        default:
            HILOG_ERROR("Non-existent error type!");
            break;
    }
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
    HILOG_INFO("Send event start.");
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
    if (callbackProxy_ != nullptr && (wallpaperData.resourceType == PICTURE || wallpaperData.resourceType == DEFAULT)) {
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

ErrorCode WallpaperService::SetCustomWallpaper(int32_t fd, int32_t type, int32_t length)
{
    if (!IsSystemApp()) {
        HILOG_ERROR("current app is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    if (!CheckCallingPermission(WALLPAPER_PERMISSION_NAME_SET_WALLPAPER)) {
        HILOG_ERROR("SetWallpaper no set permission!");
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
    if (!SaveWallpaperState(userId, wallpaperType, PACKAGE)) {
        HILOG_ERROR("Save wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    ErrorCode wallpaperErrorCode = SetWallpaper(fd, wallpaperType, length, PACKAGE);
    wallpaperData.resourceType = PACKAGE;
    wallpaperData.wallpaperId = MakeWallpaperIdLocked();
    if (wallpaperType == WALLPAPER_SYSTEM) {
        systemWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    } else if (wallpaperType == WALLPAPER_LOCKSCREEN) {
        lockWallpaperMap_.InsertOrAssign(userId, wallpaperData);
    }
    if (!SendWallpaperChangeEvent(userId, wallpaperType)) {
        HILOG_ERROR("Send wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    FinishAsyncTrace(HITRACE_TAG_MISC, "SetCustomWallpaper", static_cast<int32_t>(TraceTaskId::SET_CUSTOM_WALLPAPER));
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::GetPixelMap(int32_t wallpaperType, IWallpaperService::FdInfo &fdInfo)
{
    HILOG_INFO("WallpaperService::getPixelMap start ");
    if (!IsSystemApp()) {
        HILOG_ERROR("CallingApp is not SystemApp");
        return E_NOT_SYSTEM_APP;
    }
    if (!CheckCallingPermission(WALLPAPER_PERMISSION_NAME_GET_WALLPAPER)) {
        HILOG_ERROR("GetPixelMap no get permission!");
        return E_NO_PERMISSION;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN)
        && wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }
    auto type = static_cast<WallpaperType>(wallpaperType);
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    // current user's wallpaper is live video, not image
    WallpaperResourceType resType = GetResType(userId, type);
    if (resType != PICTURE && resType != DEFAULT) {
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
        HILOG_ERROR("reset wallpaper no set permission!");
        return E_NO_PERMISSION;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN)
        && wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        HILOG_ERROR("wallpaperType = %{public}d type not support ", wallpaperType);
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
        HILOG_ERROR("CallingApp is not SystemApp");
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
    if (!SaveWallpaperState(userId, wallpaperType, DEFAULT)) {
        HILOG_ERROR("Save wallpaper state failed!");
        return E_DEAL_FAILED;
    }
    wallpaperData.wallpaperId = DEFAULT_WALLPAPER_ID;
    wallpaperData.resourceType = DEFAULT;
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
    HILOG_DEBUG("GetWallpaperSafeLocked start.");
    auto iterator = wallpaperType == WALLPAPER_SYSTEM ? systemWallpaperMap_.Find(userId)
                                                      : lockWallpaperMap_.Find(userId);
    if (!iterator.first) {
        HILOG_INFO(" No Lock wallpaper?  Not tracking for lock-only ");
        LoadSettingsLocked(userId);
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
        HILOG_ERROR("Lock wallpaper already cleared");
        return;
    }
    if (!iterator.second.wallpaperFile.empty() || !iterator.second.liveWallpaperFile.empty()) {
        if (wallpaperType == WALLPAPER_LOCKSCREEN) {
            lockWallpaperMap_.Erase(userId);
        } else if (wallpaperType == WALLPAPER_SYSTEM) {
            systemWallpaperMap_.Erase(userId);
        }
    }
}

bool WallpaperService::CheckCallingPermission(const std::string &permissionName)
{
    AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t result = AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (result != TypePermissionState::PERMISSION_GRANTED) {
        HILOG_ERROR("Check permission failed.");
        return false;
    }
    return true;
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

#ifndef THEME_SERVICE
bool WallpaperService::ConnectExtensionAbility()
{
    HILOG_DEBUG("ConnectAdapter");
    MemoryGuard cacheGuard;
    AAFwk::Want want;
    want.SetElementName(OHOS_WALLPAPER_BUNDLE_NAME, "WallpaperExtAbility");
    ErrCode errCode = AAFwk::AbilityManagerClient::GetInstance()->Connect();
    if (errCode != ERR_OK) {
        HILOG_ERROR("connect ability server failed errCode=%{public}d", errCode);
        return false;
    }
    if (connection_ == nullptr) {
        connection_ = new WallpaperExtensionAbilityConnection(*this);
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectExtensionAbility(want, connection_, DEFAULT_VALUE);
    HILOG_INFO("ConnectExtensionAbility errCode=%{public}d", ret);
    return ret;
}
#endif

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

ErrorCode WallpaperService::SetWallpaper(
    int32_t fd, int32_t wallpaperType, int32_t length, WallpaperResourceType resourceType)
{
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    if (!CheckUserPermissionById(userId)) {
        return E_USER_IDENTITY_ERROR;
    }
    ErrorCode errCode = CheckValid(wallpaperType, length, resourceType);
    if (errCode != E_OK) {
        return errCode;
    }
    std::string uri = wallpaperTmpFullPath_;
    char *paperBuf = new (std::nothrow) char[length]();
    if (paperBuf == nullptr) {
        return E_NO_MEMORY;
    }
    {
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
    }
    WallpaperType type = static_cast<WallpaperType>(wallpaperType);
    ErrorCode wallpaperErrorCode = SetWallpaperBackupData(userId, resourceType, uri, type);
    if (resourceType == PICTURE) {
        SaveColor(userId, type);
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::SetWallpaperByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap,
    int32_t wallpaperType, WallpaperResourceType resourceType)
{
    int32_t userId = QueryActiveUserId();
    HILOG_INFO("QueryCurrentOsAccount userId: %{public}d", userId);
    if (!CheckUserPermissionById(userId)) {
        return E_USER_IDENTITY_ERROR;
    }
    std::string uri = wallpaperTmpFullPath_;
    ErrorCode errCode = WritePixelMapToFile(pixelMap, uri, wallpaperType, resourceType);
    if (errCode != E_OK) {
        HILOG_ERROR("WritePixelMapToFile failed!");
        return errCode;
    }
    WallpaperType type = static_cast<WallpaperType>(wallpaperType);
    ErrorCode wallpaperErrorCode = SetWallpaperBackupData(userId, resourceType, uri, type);
    if (resourceType == PICTURE) {
        SaveColor(userId, type);
    }
    return wallpaperErrorCode;
}

ErrorCode WallpaperService::WritePixelMapToFile(std::shared_ptr<OHOS::Media::PixelMap> pixelMap,
    std::string wallpaperTmpFullPath, int32_t wallpaperType, WallpaperResourceType resourceType)
{
    std::stringbuf stringBuf;
    std::ostream ostream(&stringBuf);
    int32_t mapSize = WritePixelMapToStream(pixelMap, ostream);
    if (mapSize <= 0) {
        HILOG_ERROR("WritePixelMapToStream failed");
        return E_WRITE_PARCEL_ERROR;
    }
    ErrorCode errCode = CheckValid(wallpaperType, mapSize, resourceType);
    if (errCode != E_OK) {
        HILOG_ERROR("CheckValid failed");
        return errCode;
    }
    char *buffer = new (std::nothrow) char[mapSize]();
    if (buffer == nullptr) {
        HILOG_ERROR("buffer failed");
        return E_NO_MEMORY;
    }
    stringBuf.sgetn(buffer, mapSize);
    {
        std::lock_guard<std::mutex> lock(mtx_);
        mode_t mode = S_IRUSR | S_IWUSR;
        int32_t fdw = open(wallpaperTmpFullPath.c_str(), O_WRONLY | O_CREAT, mode);
        if (fdw < 0) {
            HILOG_ERROR("Open wallpaper tmpFullPath failed, errno %{public}d", errno);
            delete[] buffer;
            return E_DEAL_FAILED;
        }
        if (write(fdw, buffer, mapSize) <= 0) {
            HILOG_ERROR("Write to fdw failed, errno %{public}d", errno);
            ReporterFault(FaultType::SET_WALLPAPER_FAULT, FaultCode::RF_DROP_FAILED);
            delete[] buffer;
            close(fdw);
            return E_DEAL_FAILED;
        }
        delete[] buffer;
        close(fdw);
    }
    return E_OK;
}

int64_t WallpaperService::WritePixelMapToStream(std::shared_ptr<OHOS::Media::PixelMap> pixelMap,
    std::ostream &outputStream)
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
        HILOG_ERROR("SetWallpaper no set permission!");
        return E_NO_PERMISSION;
    }
    if (wallpaperType != static_cast<int32_t>(WALLPAPER_LOCKSCREEN)
        && wallpaperType != static_cast<int32_t>(WALLPAPER_SYSTEM)) {
        return E_PARAMETERS_INVALID;
    }

    int maxLength = resourceType == VIDEO ? MAX_VIDEO_SIZE : FOO_MAX_LEN;
    if (length <= 0 || length > maxLength) {
        return E_PARAMETERS_INVALID;
    }
    return E_OK;
}

bool WallpaperService::WallpaperChanged(
    WallpaperType wallpaperType, WallpaperResourceType resType, const std::string &uri)
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

bool WallpaperService::SaveWallpaperState(
    int32_t userId, WallpaperType wallpaperType, WallpaperResourceType resourceType)
{
    WallpaperData systemData;
    WallpaperData lockScreenData;
    if (!GetWallpaperSafeLocked(userId, WALLPAPER_SYSTEM, systemData)
        || !GetWallpaperSafeLocked(userId, WALLPAPER_LOCKSCREEN, lockScreenData)) {
        return false;
    }
    nlohmann::json root;
    if (wallpaperType == WALLPAPER_SYSTEM) {
        root[SYSTEM_RES_TYPE] = static_cast<int32_t>(resourceType);
        root[LOCKSCREEN_RES_TYPE] = static_cast<int32_t>(lockScreenData.resourceType);
    } else {
        root[LOCKSCREEN_RES_TYPE] = static_cast<int32_t>(resourceType);
        root[SYSTEM_RES_TYPE] = static_cast<int32_t>(systemData.resourceType);
    }
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
    if (!GetWallpaperSafeLocked(userId, WALLPAPER_SYSTEM, systemData)
        || !GetWallpaperSafeLocked(userId, WALLPAPER_LOCKSCREEN, lockScreenData)) {
        return;
    }
    if (Json::accept(buf)) {
        auto root = nlohmann::json::parse(buf);
        if (root.contains(SYSTEM_RES_TYPE) && root[SYSTEM_RES_TYPE].is_number()) {
            systemData.resourceType = static_cast<WallpaperResourceType>(root[SYSTEM_RES_TYPE].get<int>());
        }
        if (root.contains(LOCKSCREEN_RES_TYPE) && root[SYSTEM_RES_TYPE].is_number()) {
            lockScreenData.resourceType = static_cast<WallpaperResourceType>(root[LOCKSCREEN_RES_TYPE].get<int>());
        }
    }
}
} // namespace WallpaperMgrService
} // namespace OHOS
