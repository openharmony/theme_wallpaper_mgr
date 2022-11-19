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

#ifndef NAPI_WALLPAPER_ABILITY_H
#define NAPI_WALLPAPER_ABILITY_H

#include <map>
#include <string>
#include <vector>

#include "async_call.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "pixel_map.h"
#include "pixel_map_napi.h"
#include "wallpaper_color_change_listener.h"
#include "wallpaper_js_util.h"
#include "wallpaper_manager_common_info.h"

#define BUFFER_LENGTH_MAX (128)
#define DEFAULT_STACK_ID (1)
#define DEFAULT_LAST_MEMORY_LEVEL (-1)
#define DEFAULT_WEIGHT (-1)

#define MAX_MISSION_NUM (65535)
#define QUERY_RECENT_RUNNING_MISSION_INFO_TYPE (2)
#define BUSINESS_ERROR_CODE_OK 0
namespace OHOS {
namespace WallpaperNAPI {

struct GetContextInfo : public AsyncCall::Context {
    int wallpaperType = 0;
    std::vector<RgbaColor> colors;
    int wallpaperId = 0;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    napi_status status = napi_generic_failure;
    GetContextInfo() : Context(nullptr, nullptr){};
    GetContextInfo(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetMinContextInfo : public AsyncCall::Context {
    int minHeight = 0;
    int minWidth = 0;
    napi_status status = napi_generic_failure;
    GetMinContextInfo() : Context(nullptr, nullptr){};
    GetMinContextInfo(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct PermissionContextInfo : public AsyncCall::Context {
    bool isChangePermitted = false;
    bool isOperationAllowed = false;
    napi_status status = napi_generic_failure;
    PermissionContextInfo() : Context(nullptr, nullptr){};
    PermissionContextInfo(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct SetContextInfo : public AsyncCall::Context {
    int wallpaperType = 0;
    std::string url = "";
    std::unique_ptr<OHOS::Media::PixelMap> pixelMap;
    napi_status status = napi_generic_failure;
    bool isPixelEmp = false;
    SetContextInfo() : Context(nullptr, nullptr){};
    SetContextInfo(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetFileContextInfo : public AsyncCall::Context {
    int32_t wallpaperType = 0;
    int32_t wallpaperFd = 0;
    napi_status status = napi_generic_failure;
    GetFileContextInfo() : Context(nullptr, nullptr){};
    GetFileContextInfo(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

class NapiWallpaperAbility : public WallpaperMgrService::WallpaperColorChangeListener {
public:
    NapiWallpaperAbility(napi_env env, napi_value callback);
    virtual ~NapiWallpaperAbility();
    void onColorsChange(std::vector<RgbaColor> color, int wallpaperType) override;
    static bool IsValidArgCount(size_t argc, size_t expectationSize);
    static bool IsValidArgType(napi_env env, napi_value argValue, napi_valuetype expectationType);
    static bool IsValidArgRange(napi_env env, napi_value argValue);
    static bool CheckValidArgWallpaperType(napi_env env, size_t argc, napi_value argValue,
        std::shared_ptr<AsyncCall::Context> ctx);
    static void GetColorsInner(std::shared_ptr<GetContextInfo> context);
    static void GetIdInner(std::shared_ptr<GetContextInfo> context);
    static void GetFileInner(std::shared_ptr<GetFileContextInfo> context);
    static void GetMinHeightInner(std::shared_ptr<GetMinContextInfo> context);
    static void GetMinWidthInner(std::shared_ptr<GetMinContextInfo> context);
    static void IsChangeAllowedInner(std::shared_ptr<PermissionContextInfo> context);
    static void IsUserChangeAllowedInner(std::shared_ptr<PermissionContextInfo> context);
    static void RestoreInner(std::shared_ptr<SetContextInfo> context);
    static void SetImageInput(std::shared_ptr<SetContextInfo> context);
    static void SetImageExec(std::shared_ptr<SetContextInfo> context);
    static void GetImageInner(std::shared_ptr<GetContextInfo> context);

private:
    struct EventDataWorker {
        const NapiWallpaperAbility *listener = nullptr;
        const std::vector<RgbaColor> color;
        const int wallpaperType;
        EventDataWorker(const NapiWallpaperAbility *const &listenerIn, const std::vector<RgbaColor> &colorIn,
            const int wallpaperTypeIn)
            : listener(listenerIn), color(colorIn), wallpaperType(wallpaperTypeIn)
        {
        }
    };
    napi_ref callback_ = nullptr;
    napi_env env_;
    uv_loop_s *loop_ = nullptr;
};

napi_value NAPI_GetColors(napi_env env, napi_callback_info info);
napi_value NAPI_GetColorsSync(napi_env env, napi_callback_info info);
napi_value NAPI_GetId(napi_env env, napi_callback_info info);
napi_value NAPI_GetIdSync(napi_env env, napi_callback_info info);
napi_value NAPI_GetFile(napi_env env, napi_callback_info info);
napi_value NAPI_GetFileSync(napi_env env, napi_callback_info info);
napi_value NAPI_GetMinHeight(napi_env env, napi_callback_info info);
napi_value NAPI_GetMinHeightSync(napi_env env, napi_callback_info info);
napi_value NAPI_GetMinWidth(napi_env env, napi_callback_info info);
napi_value NAPI_GetMinWidthSync(napi_env env, napi_callback_info info);
napi_value NAPI_IsChangePermitted(napi_env env, napi_callback_info info);
napi_value NAPI_IsChangeAllowed(napi_env env, napi_callback_info info);
napi_value NAPI_IsOperationAllowed(napi_env env, napi_callback_info info);
napi_value NAPI_IsUserChangeAllowed(napi_env env, napi_callback_info info);
napi_value NAPI_Reset(napi_env env, napi_callback_info info);
napi_value NAPI_Restore(napi_env env, napi_callback_info info);
napi_value NAPI_SetWallpaper(napi_env env, napi_callback_info info);
napi_value NAPI_SetImage(napi_env env, napi_callback_info info);
napi_value NAPI_GetPixelMap(napi_env env, napi_callback_info info);
napi_value NAPI_GetImage(napi_env env, napi_callback_info info);
napi_value NAPI_ScreenshotLiveWallpaper(napi_env env, napi_callback_info info);
napi_value NAPI_On(napi_env env, napi_callback_info info);
napi_value NAPI_Off(napi_env env, napi_callback_info info);
} // namespace WallpaperNAPI
} // namespace OHOS
#endif //  NAPI_WALLPAPER_ABILITY_H