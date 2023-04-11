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

#include <mutex>
#include <list>
#include <map>
#include "concurrent_map.h"
#include "system_ability.h"
#include "wallpaper_service_stub.h"
#include "wallpaper_common.h"
#include "event_handler.h"
#include "pixel_map.h"
#include "wallpaper_manager_common_info.h"
#include "wallpaper_color_change_listener.h"
#include "wallpaper_data.h"
#include "i_wallpaper_manager_callback.h"
#include "component_name.h"
#include "file_deal.h"
#include "fault_reporter.h"
#include "statistic_reporter.h"
#include "wm_common.h"
#include "window_option.h"
#include "surface_buffer.h"
#include "display_manager.h"
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#include "image_source.h"
#include "want.h"
#include "wallpaper_extension_ability_connection.h"
#include "ability_manager_errors.h"
#include "os_account_manager.h"
#include "ability_connect_callback_interface.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace ColorManager {
class Color;
}
namespace WallpaperMgrService {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

class WallpaperService : public SystemAbility, public WallpaperServiceStub {
    DECLARE_SYSTEM_ABILITY(WallpaperService);

public:
    DISALLOW_COPY_AND_MOVE(WallpaperService);

    WallpaperService(int32_t systemAbilityId, bool runOnCreate);
    WallpaperService();
    ~WallpaperService();

    static sptr<WallpaperService> GetInstance();
    int32_t SetWallpaperByFD(int fd, int wallpaperType, int length) override;
    int32_t SetWallpaperByMap(int fd, int wallpaperType, int length) override;
    int32_t GetPixelMap(int wallpaperType, FdInfo &fdInfo) override;
    int32_t GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    int32_t GetFile(int32_t wallpaperType, int32_t &wallpaperFd) override;
    int32_t GetWallpaperId(int32_t wallpaperType) override;
    int32_t GetWallpaperMinHeight(int32_t &minHeight) override;
    int32_t GetWallpaperMinWidth(int32_t &minWidth) override;
    bool IsChangePermitted() override;
    bool IsOperationAllowed() override;
    int32_t ResetWallpaper(int wallpaperType) override;
    bool On(sptr<IWallpaperColorChangeListener> listener) override;
    bool Off(sptr<IWallpaperColorChangeListener> listener) override;
    bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    int32_t SetWallpaperByFDV9(int32_t fd, int32_t wallpaperType, int32_t length) override;
    int32_t SetWallpaperByMapV9(int32_t fd, int32_t wallpaperType, int32_t length) override;
    int32_t GetPixelMapV9(int32_t wallpaperType, FdInfo &fdInfo) override;
    int32_t GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors) override;
    int32_t GetWallpaperMinHeightV9(int32_t &minHeight) override;
    int32_t GetWallpaperMinWidthV9(int32_t &minWidth) override;
    int32_t ResetWallpaperV9(int32_t wallpaperType) override;
public:
    bool SetLockWallpaperCallback(IWallpaperManagerCallback* cb);
    static void OnBootPhase();
    void ReporterFault(MiscServices::FaultType faultType, MiscServices::FaultCode faultCode);
    void ReporterUsageTimeStatisic();
    void RegisterSubscriber(int times);
    void StartWallpaperExtension();
    void AddWallpaperExtensionDeathRecipient(const sptr<IRemoteObject> &remoteObject);

protected:
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    /**
     * Get current user id.
     * @param none
     * @return  userid
     */
    int GetUserId();

    /**
     * Get current Display Id.
     * @param none
     * @return  displayid
     */
    int GetDisplayId();

     /**
    *  initData ,such as dir,filename,and so on.
    * @param
    * @return none
    */
    void InitData();
    int64_t WritePixelMapToFile(const std::string &filePath, std::unique_ptr< OHOS::Media::PixelMap> pixelMap);
    bool CompareColor(const uint64_t &localColor, const ColorManager::Color &color);
    bool SaveColor(int wallpaperType);
    void LoadSettingsLocked(int userId, bool keepDimensionHints);
    void MigrateFromOld();
    std::string GetWallpaperDir();
    bool GetWallpaperSafeLocked(int userId, int wpType, WallpaperData paperdata);
    void ClearWallpaperLocked(int userId, int wpType);
    int32_t SetDefaultDateForWallpaper(int userId, int wpType);
    int MakeWallpaperIdLocked();
    bool WPCheckCallingPermission(const std::string &promissionName);
    bool WPGetBundleNameByUid(std::int32_t uid, std::string &bname);
    bool MakeCropWallpaper(int wallpaperType);
    int32_t SetWallpaperBackupData(std::string uriOrPixelMap, int wallpaperType);
    int32_t ConnectExtensionAbility(const OHOS::AAFwk::Want& want);
    int32_t GetFilePath(int wallpaperType, std::string &filePath);
    void SetPixelMapCropParameters(std::unique_ptr<Media::PixelMap> wallpaperPixelMap,
        Media::DecodeOptions &decodeOpts);
    bool IsSystemApp();
    OHOS::sptr<OHOS::AppExecFwk::IBundleMgr> GetBundleMgr();
    int32_t GetImageFd(int32_t wallpaperType, int32_t &fd);
    int32_t GetImageSize(int32_t wallpaperType, int32_t &size);

private:

    int32_t Init();
    ServiceRunningState state_;
    void InitServiceHandler();
    bool CopySystemWallpaper();
    bool CopyScreenLockWallpaper();

    static std::mutex instanceLock_;
    static sptr<WallpaperService> instance_;
    static std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    std::string wallpaperLockScreenFilePath_;
    std::string wallpaperSystemFilePath_;
    std::string wallpaperLockScreenFileFullPath_;
    std::string wallpaperSystemFileFullPath_;
    std::string wallpaperLockScreenCropFileFullPath_;
    std::string wallpaperSystemCropFileFullPath_;
    std::string wallpaperTmpFullPath_;
    std::string wallpaperCropPath;
    typedef std::map<int, WallpaperColorChangeListener*> DISPLAYIDCOLORSLISTENERMAP;
    typedef std::map<int, DISPLAYIDCOLORSLISTENERMAP> COLORSLISTENERMAP;
    typedef std::list<WallpaperColorChangeListener*> LISTENERLIST;
    LISTENERLIST colorListeners_;
    COLORSLISTENERMAP colorsChangedListeners_;
    ConcurrentMap<int, WallpaperData> wallpaperMap_;
    ConcurrentMap<int, WallpaperData> lockWallpaperMap_;
    atomic<int32_t> wallpaperId_;
    int userId_;
    static const std::string WALLPAPER;
    static const std::string WALLPAPER_CROP;
    static const std::string WALLPAPER_LOCK_ORIG;
    static const std::string WALLPAPER_LOCK_CROP;
    static const std::string WALLPAPER_BUNDLE_NAME;
    sptr<IWallpaperCallback> callbackProxy = nullptr;
    sptr<IRemoteObject::DeathRecipient> recipient_;

    std::string name_;
    std::mutex mtx;
    uint64_t lockWallpaperColor_;
    uint64_t systemWallpaperColor_;
    std::map<int, sptr<IWallpaperColorChangeListener>> colorChangeListenerMap_;
    std::mutex listenerMapMutex_;
};
}
} // OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICES_H