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

#ifndef SERVICES_INCLUDE_WALLPAPER_SERVICES_H
#define SERVICES_INCLUDE_WALLPAPER_SERVICES_H

#include <list>
#include <map>
#include <mutex>

#include "accesstoken_kit.h"
#include "component_name.h"
#include "concurrent_map.h"
#include "event_handler.h"
#include "fault_reporter.h"
#include "file_deal.h"
#include "i_wallpaper_manager_callback.h"
#include "image_source.h"
#include "ipc_skeleton.h"
#include "os_account_manager.h"
#include "pixel_map.h"
#include "system_ability.h"
#include "wallpaper_common.h"
#include "wallpaper_common_event_subscriber.h"
#include "wallpaper_data.h"
#include "wallpaper_event_listener.h"
#include "wallpaper_manager_common_info.h"
#include "wallpaper_service_stub.h"
#include "want.h"

#ifndef THEME_SERVICE
#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "wallpaper_extension_ability_connection.h"
#endif
namespace OHOS {
namespace ColorManager {
class Color;
}
namespace WallpaperMgrService {
class WallpaperService : public SystemAbility, public WallpaperServiceStub {
    DECLARE_SYSTEM_ABILITY(WallpaperService);
    enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
    enum class FileType : uint8_t { WALLPAPER_FILE, CROP_FILE };
    using WallpaperListenerMap = std::map<int32_t, sptr<IWallpaperEventListener>>;

public:
    DISALLOW_COPY_AND_MOVE(WallpaperService);

    WallpaperService(int32_t systemAbilityId, bool runOnCreate);
    WallpaperService();
    ~WallpaperService();

    ErrorCode SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SetWallpaperByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType) override;
    ErrorCode GetPixelMap(int32_t wallpaperType, FdInfo &fdInfo) override;
    ErrorCode GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    ErrorCode GetFile(int32_t wallpaperType, int32_t &wallpaperFd) override;
    int32_t GetWallpaperId(int32_t wallpaperType) override;
    bool IsChangePermitted() override;
    bool IsOperationAllowed() override;
    ErrorCode ResetWallpaper(int32_t wallpaperType) override;
    ErrorCode On(const std::string &type, sptr<IWallpaperEventListener> listener) override;
    ErrorCode Off(const std::string &type, sptr<IWallpaperEventListener> listener) override;
    bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    ErrorCode SetWallpaperV9(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SetWallpaperV9ByPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType) override;
    ErrorCode GetPixelMapV9(int32_t wallpaperType, FdInfo &fdInfo) override;
    ErrorCode GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    ErrorCode ResetWallpaperV9(int32_t wallpaperType) override;
    ErrorCode SetVideo(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SetCustomWallpaper(int32_t fd, int32_t wallpaperType, int32_t length) override;
    ErrorCode SendEvent(const std::string &eventType) override;
    ErrorCode SetAllWallpapers(std::vector<WallpaperPictureInfo> allWallpaperInfo, int32_t wallpaperType) override;
    ErrorCode GetCorrespondWallpaper(
        int32_t wallpaperType, int32_t foldState, int32_t rotateState, IWallpaperService::FdInfo &fdInfo) override;
    bool IsDefaultWallpaperResource(int32_t userId, int32_t wallpaperType) override;

public:
    void OnInitUser(int32_t newUserId);
    void OnRemovedUser(int32_t userId);
    void OnSwitchedUser(int32_t userId);
    void ReporterFault(MiscServices::FaultType faultType, MiscServices::FaultCode faultCode);
    void RegisterSubscriber(int32_t times);
#ifndef THEME_SERVICE
    void AddWallpaperExtensionDeathRecipient(const sptr<IRemoteObject> &remoteObject);
    void StartExtensionAbility(int32_t times);
#endif
protected:
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    void InitData();
    void InitQueryUserId(int32_t times);
    bool InitUsersOnBoot();
    bool CompareColor(const uint64_t &localColor, const ColorManager::Color &color);
    bool SaveColor(int32_t userId, WallpaperType wallpaperType);
    void UpdataWallpaperMap(int32_t userId, WallpaperType wallpaperType);
    std::string GetWallpaperDir(int32_t userId, WallpaperType wallpaperType);
    bool GetFileNameFromMap(int32_t userId, WallpaperType wallpaperType, std::string &fileName);
    bool GetPictureFileName(int32_t userId, WallpaperType wallpaperType, std::string &fileName);
    bool GetWallpaperSafeLocked(int32_t userId, WallpaperType wallpaperType, WallpaperData &wallpaperData);
    void ClearWallpaperLocked(int32_t userId, WallpaperType wallpaperType);
    ErrorCode SetDefaultDataForWallpaper(int32_t userId, WallpaperType wallpaperType);
    int32_t MakeWallpaperIdLocked();
    bool CheckCallingPermission(const std::string &permissionName);
    ErrorCode SetWallpaperBackupData(int32_t userId, WallpaperResourceType resourceType,
        const std::string &uriOrPixelMap, WallpaperType wallpaperType);
    ErrorCode WritePixelMapToFile(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, std::string wallpaperTmpFullPath,
        int32_t wallpaperType, WallpaperResourceType resourceType);
    int64_t WritePixelMapToStream(std::shared_ptr<OHOS::Media::PixelMap> pixelMap, std::ostream &outputStream);
#ifndef THEME_SERVICE
    bool ConnectExtensionAbility();
#endif
    bool IsSystemApp();
    bool IsNativeSa();
    ErrorCode GetImageFd(int32_t userId, WallpaperType wallpaperType, int32_t &fd);
    ErrorCode GetImageSize(int32_t userId, WallpaperType wallpaperType, int32_t &size);
    bool RestoreUserResources(int32_t userId, WallpaperData &wallpaperData, WallpaperType wallpaperType);
    bool InitUserDir(int32_t userId);
    int32_t QueryActiveUserId();
    bool CheckUserPermissionById(int32_t userId);

    bool SendWallpaperChangeEvent(int32_t userId, WallpaperType wallpaperType);
    ErrorCode SetWallpaper(int32_t fd, int32_t wallpaperType, int32_t length, WallpaperResourceType resourceType);
    ErrorCode SetWallpaperByPixelMap(
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap, int32_t wallpaperType, WallpaperResourceType resourceType);
    void OnColorsChange(WallpaperType wallpaperType, const ColorManager::Color &color);
    ErrorCode CheckValid(int32_t wallpaperType, int32_t length, WallpaperResourceType resourceType);
    bool WallpaperChanged(WallpaperType wallpaperType, WallpaperResourceType resType, const std::string &uri);
    void NotifyColorChange(const std::vector<uint64_t> &colors, const WallpaperType &wallpaperType);
    bool SaveWallpaperState(int32_t userId, WallpaperType wallpaperType, WallpaperResourceType resourceType);
    void LoadWallpaperState();
    WallpaperResourceType GetResType(int32_t userId, WallpaperType wallpaperType);
    void RemoveExtensionDeathRecipient();
    static void GetWallpaperFile(
        WallpaperResourceType resourceType, const WallpaperData &wallpaperData, std::string &wallpaperFile);
    std::string GetDefaultResDir();
    WallpaperData GetWallpaperDefaultPath(WallpaperType wallpaperType);
    std::string GetWallpaperPathInJson(const std::string manifestName, const std::string filePath);
    void ClearRedundantFile(int32_t userId, WallpaperType wallpaperType, std::string fileName);
    std::string GetExistFilePath(const std::string &filePath);
    ErrorCode SetAllWallpapers(
        std::vector<WallpaperPictureInfo> allWallpaperInfo, int32_t wallpaperType, WallpaperResourceType resourceType);
    ErrorCode WriteFdToFile(WallpaperPictureInfo &wallpaperPictureInfo, std::string &path);
    ErrorCode SetAllWallpaperBackupData(std::vector<WallpaperPictureInfo> allWallpaperInfos, int32_t userId,
        WallpaperType wallpaperType, WallpaperData &wallpaperData);
    std::string GetWallpaperDataFile(
        WallpaperPictureInfo &wallpaperPictureInfo, int32_t userId, WallpaperType wallpaperType);
    ErrorCode GetImageSize(
        int32_t userId, WallpaperType wallpaperType, int32_t &size, int32_t foldState, int32_t rotateState);
    bool GetWallpaperDataPath(int32_t userId, WallpaperType wallpaperType, std::string &filePathName,
        int32_t foldState, int32_t rotateState);
    ErrorCode GetImageFd(
        int32_t userId, WallpaperType wallpaperType, int32_t &fd, int32_t foldState, int32_t rotateState);
    void DeleteTempResource(std::vector<WallpaperPictureInfo> &tempResourceFiles);
    void UpdateWallpaperDataFile(WallpaperPictureInfo &wallpaperPictureInfo, int32_t userId,
        WallpaperType wallpaperType, WallpaperData &wallpaperData);
    ErrorCode UpdateWallpaperData(
        std::vector<WallpaperPictureInfo> allWallpaperInfos, int32_t userId, WallpaperType wallpaperType);
    std::string GetWallpaperPath(WallpaperData wallpaperData);
    void ClearnWallpaperDataFile(WallpaperData &wallpaperData);
    std::string GetFoldStateName(FoldState foldState);
    std::string GetRotateStateName(RotateState rotateState);
    std::string GetWallpaperPath(int32_t foldState, int32_t rotateState, WallpaperData &wallpaperData);

private:
    int32_t Init();
    ServiceRunningState state_;
    void InitServiceHandler();
    static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    std::string wallpaperLockScreenFilePath_;
    std::string wallpaperSystemFilePath_;
    std::string wallpaperLockScreenFileFullPath_;
    std::string wallpaperSystemFileFullPath_;
    std::string wallpaperLockScreenCropFileFullPath_;
    std::string wallpaperSystemCropFileFullPath_;
    std::string wallpaperTmpFullPath_;
    std::string wallpaperCropPath_;
    typedef std::map<int32_t, WallpaperEventListener *> DISPLAYIDCOLORSLISTENERMAP;
    typedef std::map<int32_t, DISPLAYIDCOLORSLISTENERMAP> COLORSLISTENERMAP;
    typedef std::list<WallpaperEventListener *> LISTENERLIST;
    LISTENERLIST colorListeners_;
    COLORSLISTENERMAP colorsChangedListeners_;
    ConcurrentMap<int32_t, WallpaperData> systemWallpaperMap_;
    ConcurrentMap<int32_t, WallpaperData> lockWallpaperMap_;
    atomic<int32_t> wallpaperId_;
    sptr<IWallpaperCallback> callbackProxy_ = nullptr;
    std::shared_ptr<WallpaperCommonEventSubscriber> subscriber_;
#ifndef THEME_SERVICE
    sptr<WallpaperExtensionAbilityConnection> connection_;
#endif
    sptr<IRemoteObject::DeathRecipient> recipient_;
    sptr<IRemoteObject> extensionRemoteObject_;
    std::mutex remoteObjectMutex_;

    std::mutex mtx_;
    uint64_t lockWallpaperColor_;
    uint64_t systemWallpaperColor_;
    std::map<std::string, WallpaperListenerMap> wallpaperEventMap_;
    std::mutex listenerMapMutex_;
    std::int32_t currentUserId_;
    std::string appBundleName_;
    std::mutex wallpaperColorMtx_;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICES_H