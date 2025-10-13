/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ohos.wallpaper.proj.hpp"
#include "ohos.wallpaper.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

#include "hilog_wrapper.h"
#include "js_error.h"
#include "napi/native_node_api.h"
#include "uv_queue.h"
#include "wallpaper_manager.h"
#include "pixel_map_taihe_ani.h"
#include "wallpaper_manager_common_info.h"
#include "ani_wallpaper.h"

using namespace OHOS::WallpaperMgrService;
namespace {

std::recursive_mutex cbWallpaperChangeListMutex_;
std::vector<std::shared_ptr<ani_wallpaper::WallpaperObserver>> jsWallpaperChangeCbList_;

enum WallpaperErrorCode : int32_t {
    PERMISSION_ERROR = 201,
    NOT_SYSTEM_API = 202,
    PARAMETERS_ERROR = 401,
};

static float Alpha(uint64_t color)
{
    return ((color >> 56) & 0xff) / 255.0f; // shift 56 bits to get alpha value and 255.0f is the max value
}

static float Red(uint64_t color)
{
    return ((color >> 48) & 0xff) / 255.0f; // shift 48 bits to get red value and 255.0f is the max value
}

static float Green(uint64_t color)
{
    return ((color >> 40) & 0xff) / 255.0f; // shift 40 bits to get green value and 255.0f is the max value
}

static float Blue(uint64_t color)
{
    return ((color >> 32) & 0xff) / 255.0f; // shift 32 bits to get blue value and 255.0f is the max value
}

void setErrorCode(OHOS::WallpaperMgrService::ErrorCode wallpaperErrorCode)
{
    switch (wallpaperErrorCode) {
        case ErrorCode::E_NOT_SYSTEM_APP:
            taihe::set_business_error(WallpaperErrorCode::NOT_SYSTEM_API, "Permission verification failed,"
                                              "application which is not a system application uses system API!");
            break;
        case ErrorCode::E_NO_PERMISSION:
            taihe::set_business_error(WallpaperErrorCode::PERMISSION_ERROR, "Permission Denied!");
            break;
        default:
            taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "Invalid parameter");
            break;
    }
}

void SetImageAsync(::ohos::wallpaper::SourceType const& sourceType, ::ohos::wallpaper::WallpaperType wallpaperType)
{
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM
        && wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "Invalid wallpaperType parameter");
        return;
    }
    ApiInfo apiInfo{ true, true };
    ErrorCode wallpaperErrorCode = E_UNKNOWN;
    if (sourceType.get_tag() == ::ohos::wallpaper::SourceType::tag_t::pixelMap) {
        HILOG_DEBUG("exec setWallpaper by pixelMap.");
        uintptr_t pixelMap = sourceType.get_pixelMap_ref();
        ani_object object = reinterpret_cast<ani_object>(pixelMap);
        auto newPixelMap = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(taihe::get_env(), object);
        if (newPixelMap == nullptr) {
            HILOG_ERROR("Invalid pixelmap is nullptr");
            taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "Invalid sourceType parameter");
            return;
        }
        wallpaperErrorCode = WallpaperManager::GetInstance().SetWallpaper(newPixelMap, wallpaperType, apiInfo);
    } else {
        std::string str = std::string(sourceType.get_source_ref());
        if (!str.empty()) {
            HILOG_DEBUG("exec setWallpaper by uri.");
            wallpaperErrorCode =WallpaperManager::GetInstance().SetWallpaper(str, wallpaperType, apiInfo);
        }
    }
    if (wallpaperErrorCode != ErrorCode::E_OK) {
        setErrorCode(wallpaperErrorCode);
    }
}

void RestoreAsync(::ohos::wallpaper::WallpaperType wallpaperType)
{
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM
        && wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "Invalid wallpaperType parameter");
        return;
    }
    ApiInfo apiInfo{ true, true };
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().ResetWallpaper(wallpaperType, apiInfo);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
    }
}

::taihe::array<::ohos::wallpaper::RgbaColor> GetColorsSync(::ohos::wallpaper::WallpaperType wallpaperType)
{
    std::vector<uint64_t> colors;
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM &&
        wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "Invalid wallpaperType parameter");
        return ::taihe::array<::ohos::wallpaper::RgbaColor>(::taihe::copy_data_t{},
            static_cast<::ohos::wallpaper::RgbaColor*>(nullptr), 0);
    }
    ApiInfo apiInfo{ true, true };
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().GetColors(wallpaperType, apiInfo, colors);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
    }
    std::vector<::ohos::wallpaper::RgbaColor> rgbaColors;
    rgbaColors.reserve(colors.size());
    for (uint64_t color : colors) {
        ::ohos::wallpaper::RgbaColor rgbaColor;
        rgbaColor.alpha = Alpha(color);
        rgbaColor.red = Red(color);
        rgbaColor.green = Green(color);
        rgbaColor.blue = Blue(color);
        rgbaColors.push_back(rgbaColor);
    }
    return ::taihe::array<::ohos::wallpaper::RgbaColor>(::taihe::copy_data_t{}, rgbaColors.data(), rgbaColors.size());
}

int32_t GetMinHeightSync()
{
    int32_t minHeight = 0;
    ApiInfo apiInfo{ true, true };
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().GetWallpaperMinHeight(apiInfo, minHeight);
    if (wallpaperErrorCode != E_OK || minHeight <= 0) {
        setErrorCode(wallpaperErrorCode);
    }
    return minHeight;
}

int32_t GetMinWidthSync()
{
    int32_t minWidth = 0;
    ApiInfo apiInfo{ true, true };
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().GetWallpaperMinWidth(apiInfo, minWidth);
    if (wallpaperErrorCode != E_OK || minWidth <= 0) {
        setErrorCode(wallpaperErrorCode);
    }
    return minWidth;
}

void SetCustomWallpaperAsync(::taihe::string_view source, ::ohos::wallpaper::WallpaperType wallpaperType)
{
    std::string str = std::string(source);
    if (str.empty()) {
        HILOG_ERROR("Invalid source parameter, source is empty");
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "Source cannot be empty");
        return;
    }
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM &&
        wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR,
            "Invalid wallpaperType, must be WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN");
        return;
    }
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().SetCustomWallpaper(str, wallpaperType);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
    }
}

void SetVideoAsync(::taihe::string_view source, ::ohos::wallpaper::WallpaperType wallpaperType)
{
    std::string str = std::string(source);
    if (str.empty()) {
        HILOG_ERROR("Invalid source parameter, source is empty");
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "Source cannot be empty");
        return;
    }
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM &&
        wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR,
            "Invalid wallpaperType, must be WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN");
        return;
    }
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().SetVideo(str, wallpaperType);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
    }
}

uintptr_t GetImageAsync(::ohos::wallpaper::WallpaperType wallpaperType)
{
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM &&
        wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR,
            "Invalid wallpaperType, must be WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN");
        return 0;
    }
    ApiInfo apiInfo{ true, true };
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().GetPixelMap(wallpaperType, apiInfo, pixelMap);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
        return 0;
    }
    return reinterpret_cast<uintptr_t>(pixelMap.get());
}

uintptr_t GetWallpaperByStateSync(::ohos::wallpaper::WallpaperType wallpaperType,
    ::ohos::wallpaper::FoldState foldState, ::ohos::wallpaper::RotateState rotateState)
{
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM &&
        wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR,
            "Invalid wallpaperType, must be WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN");
        return 0;
    }
    if (foldState.get_key() != ::ohos::wallpaper::FoldState::key_t::NORMAL &&
        foldState.get_key() != ::ohos::wallpaper::FoldState::key_t::UNFOLD_ONCE_STATE &&
        foldState.get_key() != ::ohos::wallpaper::FoldState::key_t::UNFOLD_TWICE_STATE) {
        HILOG_ERROR("Invalid wallpaperType parameter, foldState:%{public}d", int32_t(foldState));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR,
            "Invalid wallpaperType, must be NORMAL or UNFOLD_ONCE_STATE or UNFOLD_TWICE_STATE");
        return 0;
    }
    if (rotateState.get_key() != ::ohos::wallpaper::RotateState::key_t::PORTRAIT &&
        rotateState.get_key() != ::ohos::wallpaper::RotateState::key_t::LANDSCAPE) {
        HILOG_ERROR("Invalid wallpaperType parameter, rotateState:%{public}d", int32_t(rotateState));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR,
            "Invalid wallpaperType, must be PORTRAIT or LANDSCAPE");
        return 0;
    }
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().GetCorrespondWallpaper(
        wallpaperType, foldState, rotateState, pixelMap);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
        return 0;
    }
    return reinterpret_cast<uintptr_t>(pixelMap.get());
}

void SetAllWallpapersSync(::taihe::array_view<::ohos::wallpaper::WallpaperInfo> wallpaperInfos,
    ::ohos::wallpaper::WallpaperType wallpaperType)
{
    if (wallpaperInfos.empty()) {
        HILOG_ERROR("Invalid wallpaperInfos parameter, wallpaperInfos is empty");
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR, "wallpaperInfos cannot be empty");
        return;
    }
    if (wallpaperType != WallpaperType::WALLPAPER_SYSTEM &&
        wallpaperType != WallpaperType::WALLPAPER_LOCKSCREEN) {
        HILOG_ERROR("Invalid wallpaperType parameter, wallpaperType:%{public}d", int32_t(wallpaperType));
        taihe::set_business_error(WallpaperErrorCode::PARAMETERS_ERROR,
            "Invalid wallpaperType, must be WALLPAPER_SYSTEM or WALLPAPER_LOCKSCREEN");
        return;
    }
    std::vector<WallpaperInfo> infos;
    infos.reserve(wallpaperInfos.size());
    for (auto it = wallpaperInfos.begin(); it != wallpaperInfos.end(); ++it) {
        auto const &wallpaperInfo = *it;
        int32_t newFoldState = wallpaperInfo.foldState.get_value();
        int32_t newRotateState = wallpaperInfo.rotateState.get_value();
        std::string newSource = std::string(wallpaperInfo.source);
        WallpaperInfo info = {
            .foldState = static_cast<FoldState>(newFoldState),
            .rotateState = static_cast<RotateState>(newRotateState),
            .source = static_cast<std::string>(newSource),
        };
        infos.push_back(info);
    }
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().SetAllWallpapers(infos, wallpaperType);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
        return;
    }
}

void onWallpaperChange(::taihe::callback_view<void(::ohos::wallpaper::WallpaperType wallpaperType,
    ::ohos::wallpaper::WallpaperResourceType resourceType,
    ::taihe::optional_view<::taihe::string> uri)> f, uintptr_t opq)
{
    std::lock_guard<std::recursive_mutex> lock(cbWallpaperChangeListMutex_);
    ani_object callbackObj = reinterpret_cast<ani_object>(opq);
    ani_ref callbackRef = ani_wallpaper::CreateCallbackRefIfNotDuplicate(jsWallpaperChangeCbList_, callbackObj);
    if (callbackRef == nullptr) {
        HILOG_ERROR("failed to register");
        return;
    }
    auto observer = std::make_shared<ani_wallpaper::WallpaperObserver>(f, callbackRef);
    ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().On("wallpaperChange", observer);
    if (wallpaperErrorCode != E_OK) {
        setErrorCode(wallpaperErrorCode);
        return;
    }
    jsWallpaperChangeCbList_.emplace_back(std::move(observer));
}

void offWallpaperChange(::taihe::optional_view<uintptr_t> opq)
{
    ani_env *env = taihe::get_env();
    if (env == nullptr) {
        HILOG_ERROR("failed to get_env");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(cbWallpaperChangeListMutex_);
    ani_ref jsCallbackRef = nullptr;
    ani_object callbackObj = nullptr;
    if (opq.has_value()) {
        callbackObj = reinterpret_cast<ani_object>(opq.value());
    }
    ani_wallpaper::GlobalRefGuard guard(env, callbackObj);
    if (callbackObj != nullptr && !guard) {
        HILOG_ERROR("GlobalRefGuard is false!");
        return;
    }
    jsCallbackRef = guard.get();
    auto pred = [env, jsCallbackRef](std::shared_ptr<ani_wallpaper::WallpaperObserver> &obj) {
        ani_boolean is_equal = false;
        if (jsCallbackRef == nullptr) {
            return true;
        }
        return (ANI_OK == env->Reference_StrictEquals(jsCallbackRef, obj->jsCallbackRef_, &is_equal)) && is_equal;
    };
    for (auto iter = jsWallpaperChangeCbList_.begin(); iter != jsWallpaperChangeCbList_.end();) {
        if (pred(*iter) == true) {
            HILOG_INFO("jsWallpaperChangeCbList_ erase item");
            (*iter)->Release();
            ErrorCode wallpaperErrorCode = WallpaperManager::GetInstance().Off("wallpaperChange", *iter);
            if (wallpaperErrorCode != E_OK) {
                setErrorCode(wallpaperErrorCode);
                return;
            }
            iter = jsWallpaperChangeCbList_.erase(iter);
        } else {
            ++iter;
        }
    }
}
} // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_SetImageAsync(SetImageAsync);
TH_EXPORT_CPP_API_RestoreAsync(RestoreAsync);
TH_EXPORT_CPP_API_GetColorsSync(GetColorsSync);
TH_EXPORT_CPP_API_GetMinHeightSync(GetMinHeightSync);
TH_EXPORT_CPP_API_GetMinWidthSync(GetMinWidthSync);
TH_EXPORT_CPP_API_SetCustomWallpaperAsync(SetCustomWallpaperAsync);
TH_EXPORT_CPP_API_SetVideoAsync(SetVideoAsync);
TH_EXPORT_CPP_API_GetImageAsync(GetImageAsync);
TH_EXPORT_CPP_API_GetWallpaperByStateSync(GetWallpaperByStateSync);
TH_EXPORT_CPP_API_SetAllWallpapersSync(SetAllWallpapersSync);
TH_EXPORT_CPP_API_onWallpaperChange(onWallpaperChange);
TH_EXPORT_CPP_API_offWallpaperChange(offWallpaperChange);
// NOLINTEND