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

#include <napi/native_api.h>
#include <pthread.h>
#include <string>
#include <unistd.h>

#include "hilog_wrapper.h"
#include "napi/native_node_api.h"
#include "napi_wallpaper_ability.h"
#include "wallpaper_manager_common_info.h"

namespace OHOS {
namespace WallpaperNAPI {

EXTERN_C_START
static napi_value InitWallpaperType(napi_env &env)
{
    napi_value wallpaperType = nullptr;
    napi_value systemType = nullptr;
    napi_value lockscreenType = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(WALLPAPER_SYSTEM), &systemType));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(WALLPAPER_LOCKSCREEN), &lockscreenType));
    NAPI_CALL(env, napi_create_object(env, &wallpaperType));
    NAPI_CALL(env, napi_set_named_property(env, wallpaperType, "WALLPAPER_SYSTEM", systemType));
    NAPI_CALL(env, napi_set_named_property(env, wallpaperType, "WALLPAPER_LOCKSCREEN", lockscreenType));
    return wallpaperType;
}

static napi_value InitWallpaperResourceType(napi_env &env)
{
    napi_value wallpaperResourceType = nullptr;
    napi_value wallpaperResDefault = nullptr;
    napi_value wallpaperResPicture = nullptr;
    napi_value wallpaperResVideo = nullptr;
    napi_value wallpaperResPackage = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(DEFAULT), &wallpaperResDefault));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PICTURE), &wallpaperResPicture));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(VIDEO), &wallpaperResVideo));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(PACKAGE), &wallpaperResPackage));
    NAPI_CALL(env, napi_create_object(env, &wallpaperResourceType));
    NAPI_CALL(env, napi_set_named_property(env, wallpaperResourceType, "DEFAULT", wallpaperResDefault));
    NAPI_CALL(env, napi_set_named_property(env, wallpaperResourceType, "PICTURE", wallpaperResPicture));
    NAPI_CALL(env, napi_set_named_property(env, wallpaperResourceType, "VIDEO", wallpaperResVideo));
    NAPI_CALL(env, napi_set_named_property(env, wallpaperResourceType, "PACKAGE", wallpaperResPackage));
    return wallpaperResourceType;
}

static napi_value Init(napi_env env, napi_value exports)
{
    HILOG_DEBUG("napi_module Init start...");
    napi_value wallpaperType = InitWallpaperType(env);
    napi_value wallpaperResourceType = InitWallpaperResourceType(env);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getColors", NAPI_GetColors),
        DECLARE_NAPI_FUNCTION("getColorsSync", NAPI_GetColorsSync),
        DECLARE_NAPI_FUNCTION("getId", NAPI_GetId),
        DECLARE_NAPI_FUNCTION("getFile", NAPI_GetFile),
        DECLARE_NAPI_FUNCTION("getMinHeight", NAPI_GetMinHeight),
        DECLARE_NAPI_FUNCTION("getMinHeightSync", NAPI_GetMinHeightSync),
        DECLARE_NAPI_FUNCTION("getMinWidth", NAPI_GetMinWidth),
        DECLARE_NAPI_FUNCTION("getMinWidthSync", NAPI_GetMinWidthSync),
        DECLARE_NAPI_FUNCTION("isChangePermitted", NAPI_IsChangePermitted),
        DECLARE_NAPI_FUNCTION("isOperationAllowed", NAPI_IsOperationAllowed),
        DECLARE_NAPI_FUNCTION("reset", NAPI_Reset),
        DECLARE_NAPI_FUNCTION("restore", NAPI_Restore),
        DECLARE_NAPI_FUNCTION("setWallpaper", NAPI_SetWallpaper),
        DECLARE_NAPI_FUNCTION("setImage", NAPI_SetImage),
        DECLARE_NAPI_FUNCTION("getPixelMap", NAPI_GetPixelMap),
        DECLARE_NAPI_FUNCTION("getImage", NAPI_GetImage),
        DECLARE_NAPI_FUNCTION("on", NAPI_On),
        DECLARE_NAPI_FUNCTION("off", NAPI_Off),
        DECLARE_NAPI_FUNCTION("setVideo", NAPI_SetVideo),
        DECLARE_NAPI_FUNCTION("sendEvent", NAPI_SendEvent),
        DECLARE_NAPI_FUNCTION("setCustomWallpaper", NAPI_SetCustomWallpaper),
        DECLARE_NAPI_STATIC_PROPERTY("WallpaperType", wallpaperType),
        DECLARE_NAPI_STATIC_PROPERTY("WallpaperResourceType", wallpaperResourceType),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    HILOG_DEBUG("napi_module Init end...");
    return exports;
}

EXTERN_C_END

/*
 * Module define
 */
static napi_module g_wallpaperExtensionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "wallpaper",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_wallpaperExtensionModule);
}
} // namespace WallpaperNAPI
} // namespace OHOS