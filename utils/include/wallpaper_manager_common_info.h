/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INNERKITSIMPL_WALLPAPER_MANAGER_COMMON_INFO_H
#define INNERKITSIMPL_WALLPAPER_MANAGER_COMMON_INFO_H

#include <string>

static const std::string WALLPAPER_USERID_PATH = "/data/service/el1/public/wallpaper/";
static const std::string WALLPAPER_LOCKSCREEN_KVFILENAME = "wallpaper_lockscreen";
static const std::string WALLPAPER_SYSTEM_KVFILENAME = "wallpaper_system";
static const std::string WALLPAPER_SYSTEM_DIRNAME = "system";
static const std::string WALLPAPER_TMP_DIRNAME = "fwsettmp";
static const std::string WALLPAPER_LOCKSCREEN_DIRNAME = "lockscreen";
static const std::string WALLPAPER_DEFAULT_FILEFULLPATH = "/system/etc/wallpaperdefault.jpeg";
static const std::string WALLPAPER_DEFAULT_LOCK_FILEFULLPATH = "/system/etc/wallpaperlockdefault.jpeg";
static const std::string WALLPAPER_CROP_PICTURE = "crop_file";
enum WallpaperType {
    /**
     * Indicates the home screen wallpaper.
     */
    WALLPAPER_SYSTEM,
    /**
     * Indicates the lock screen wallpaper.
     */
    WALLPAPER_LOCKSCREEN
};
#endif