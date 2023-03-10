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
#include <unistd.h>

#include <string>

#include "hilog_wrapper.h"
#include "napi/native_node_api.h"
#include "napi_wallpaper_ability.h"

namespace OHOS {
namespace WallpaperNAPI {
enum class WallpaperType {
    /**
     * Indicates the home screen wallpaper.
     */
    WALLPAPER_SYSTEM,
    /**
     * Indicates the lock screen wallpaper.
     */
    WALLPAPER_LOCKSCREEN
};
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    HILOG_INFO("napi_moudule Init start...");
    napi_value WallpaperType = nullptr;
    napi_value wpType_system = nullptr;
    napi_value wpType_lockscreen = nullptr;
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(WALLPAPER_SYSTEM), &wpType_system));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(WALLPAPER_LOCKSCREEN), &wpType_lockscreen));
    NAPI_CALL(env, napi_create_object(env, &WallpaperType));
    NAPI_CALL(env, napi_set_named_property(env, WallpaperType, "WALLPAPER_SYSTEM", wpType_system));
    NAPI_CALL(env, napi_set_named_property(env, WallpaperType, "WALLPAPER_LOCKSCREEN", wpType_lockscreen));
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
        DECLARE_NAPI_STATIC_PROPERTY("WallpaperType", WallpaperType),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    HILOG_INFO("napi_moudule Init end...");
    return exports;
}

EXTERN_C_END

/*
 * Module define
 */
static napi_module _module = {
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
    napi_module_register(&_module);
}
} // namespace WallpaperNAPI
} // namespace OHOS