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
#include "wm_common.h"
#include "window_option.h"
#include "surface_buffer.h"
#include "display_manager.h"
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#include "image_source.h"
#include "reporter.h"

namespace OHOS {
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
    bool SetWallpaperByFD(int fd, int wallpaperType, int length) override;
    bool SetWallpaperByMap(int fd, int wallpaperType, int length) override;
    mapFD GetPixelMap(int wallpaperType) override;
    std::vector<RgbaColor> GetColors(int wallpaperType) override;
    int GetFile(int wallpaperType) override;
    int  GetWallpaperId(int wallpaperType) override;
    int  GetWallpaperMinHeight() override;
    int  GetWallpaperMinWidth() override;
    bool IsChangePermitted() override;
    bool IsOperationAllowed() override;
    bool ResetWallpaper(int wallpaperType) override;
    bool ScreenshotLiveWallpaper(int scaleNumber, OHOS::Media::PixelMap pixelMap) override;
    bool On(sptr<IWallpaperColorChangeListener> listener) override;
    bool Off(sptr<IWallpaperColorChangeListener> listener) override;
    bool RegisterWallpaperCallback(const sptr<IWallpaperCallback> callback) override;
    int Dump(int fd, const std::vector<std::u16string> &args) override;
public:
    bool SetLockWallpaperCallback(IWallpaperManagerCallback* cb);
    static void OnBootPhase();
    void ReporterFault(MiscServices::FaultType faultType, MiscServices::FaultCode faultCode);
    void ReporterUsageTimeStatisic();
    void RegisterSubscriber(int times);

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
    void LoadSettingsLocked(int userId, bool keepDimensionHints);
    std::string GetWallpaperDir(int userId);
    void NotifyLockWallpaperChanged();
    void MigrateFromOld();
    bool BindWallpaperComponentLocked(ComponentName& componentName, bool force, bool fromUser, WallpaperData wallpaper);
    bool GetWallpaperSafeLocked(int userId, int wpType, WallpaperData paperdata);
    void ClearWallpaperLocked(int userId, int wpType);
    bool SetDefaultDateForWallpaper(int userId, int wpType);
    int MakeWallpaperIdLocked();
    bool  ChangingToSame(ComponentName componentName, WallpaperData wallpaper);
    bool WPCheckCallingPermission(const std::string &promissionName);
    bool WPGetBundleNameByUid(std::int32_t uid, std::string &bname);
    bool MakeCropWallpaper(int wallpaperType);
    bool SetWallpaperBackupData(std::string uriOrPixelMap, int wallpaperType);

private:
    void StartExt();
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
    std::map<int, WallpaperData> wallpaperMap_ ;
    std::map<int, WallpaperData> lockWallpaperMap_ ;
    int wallpaperId_;
    int userId_;
    IWallpaperManagerCallback *keyguardListener_;
    static const std::string WALLPAPER;
    static const std::string WALLPAPER_CROP;
    static const std::string WALLPAPER_LOCK_ORIG;
    static const std::string WALLPAPER_LOCK_CROP;
    static const std::string WALLPAPER_BUNDLE_NAME;
    sptr<IWallpaperCallback> callbackProxy = nullptr;

    std::string name_;
    std::mutex mtx;
};
}
} // OHOS
#endif // SERVICES_INCLUDE_WALLPAPER_SERVICES_H