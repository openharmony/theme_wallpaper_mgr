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

#include <pthread.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>
#include <uv.h>
#include "hilog_wrapper.h"
#include "wallpaper_manager.h"
#include "wallpaper_manager_common_info.h"
#include "napi_wallpaper_ability.h"

using namespace OHOS::Media;
namespace OHOS {
namespace WallpaperNAPI {
const int32_t ONE = 1;
const int32_t TWO = 2;

napi_value NAPI_GetColors(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetColors in");
    auto context = std::make_shared<GetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->SetErrInfo(AsyncCall::ErrorCode::NOT_SUPPORT, "not support");
        HILOG_DEBUG("exec-----NAPI_GetColors in");
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), ONE);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_GetId(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetId in");
    auto context = std::make_shared<GetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 1 || argc == 2, " should 1 or 2 parameters!", napi_invalid_arg);
        HILOG_DEBUG("input ---- argc : %{public}d", argc);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "param type wrong!", napi_invalid_arg);
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input ---- wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->wallpaperId, result);
        HILOG_DEBUG("output ---- napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec ---- GetWallpaperId");
        context->wallpaperId = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(
            context->wallpaperType);
        HILOG_DEBUG("exec ---- GetWallpaperId wallpaperId : %{public}d", context->wallpaperId);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 1);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_GetPixelMap(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetPixelMap in");
    auto context = std::make_shared<GetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 1 || argc == 2, " should 1 or 2 parameters!", napi_invalid_arg);
        HILOG_DEBUG("input ---- argc : %{public}d", argc);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "param type wrong!", napi_invalid_arg);
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input ---- wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value pixelVal = PixelMapNapi::CreatePixelMap(env, context->pixelMap);
        HILOG_DEBUG("output ---- PixelMapNapi::CreatePixelMap != nullptr[%{public}d]", pixelVal != nullptr);
        *result = pixelVal;
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec ---- GetPixelMap");
        auto pixel =
            WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(context->wallpaperType);
        HILOG_DEBUG("exec ---- CreatPixelMap end");
        if (pixel != nullptr) {
            HILOG_DEBUG("exec ---- GetPixelMap pixel != nullptr");
            context->status = napi_ok;
            context->pixelMap = std::move(pixel);
            HILOG_DEBUG("exec ---- GetPixelMap pixel != nullptr");
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 1);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_GetMinHeight(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinHeight in");
    auto context = std::make_shared<GetMinContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should 0 or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->minHeight, result);
        HILOG_DEBUG("output ---- napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec ---- GetWallpaperMinHeight");
        context->minHeight = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinHeight();
        HILOG_DEBUG("exec ---- GetWallpaperMinHeight minHeight : %{public}d", context->minHeight);
        if (context->minHeight) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 0);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_GetMinWidth(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetMinWidth in");
    auto context = std::make_shared<GetMinContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should 0 or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_create_int32(env, context->minWidth, result);
        HILOG_DEBUG("output ---- napi_create_int32[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec ---- GetWallpaperMinWidth");
        context->minWidth = WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinWidth();
        HILOG_DEBUG("exec ---- GetWallpaperMinWidth minWidth : %{public}d", context->minWidth);
        if (context->minWidth) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 0);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_IsChangePermitted(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_IsChangePermitted in");
    auto context = std::make_shared<PermissionContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should 0 or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->isChangePermitted, result);
        HILOG_DEBUG("output ---- napi_get_boolean[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec ---- IsChangePermitted");
        context->isChangePermitted = WallpaperMgrService::WallpaperManagerkits::GetInstance().IsChangePermitted();
        HILOG_DEBUG("exec ---- IsChangePermitted : %{public}d", context->isChangePermitted);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 0);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_IsOperationAllowed(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_IsOperationAllowed in");
    auto context = std::make_shared<PermissionContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should 0 or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->isOperationAllowed, result);
        HILOG_DEBUG("output ---- napi_get_boolean[%{public}d]", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec ---- IsOperationAllowed");
        context->isOperationAllowed = WallpaperMgrService::WallpaperManagerkits::GetInstance().IsOperationAllowed();
        HILOG_DEBUG("exec ---- IsOperationAllowed[%{public}d]", context->isOperationAllowed);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 0);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_Reset(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_Reset in");
    auto context = std::make_shared<SetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 1 || argc == 2, " should 1 or 2 parameters!", napi_invalid_arg);
        HILOG_DEBUG("input ---- argc : %{public}d", argc);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "param type wrong!", napi_invalid_arg);
        napi_get_value_int32(env, argv[0], &context->wallpaperType);
        HILOG_DEBUG("input ---- wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        HILOG_DEBUG("exec ---- ResetWallpaper");
        bool status = WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(context->wallpaperType);
        HILOG_DEBUG("exec ---- ResetWallpaper[%{public}d]", status);
        if (status) {
            context->status = napi_ok;
        }
        HILOG_DEBUG("exec ---- status[%{public}d], context->status[%{public}d]", status, context->status);
    };
    context->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 1);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_SetWallpaper(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<SetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 2 || argc == 3, " should 2 or 3 parameters!", napi_invalid_arg);
        HILOG_DEBUG("input ---- argc : %{public}d", argc);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        NAPI_ASSERT_BASE(env, valueType == napi_string || valueType == napi_object, "first param type wrong!",
            napi_invalid_arg);
        if (valueType == napi_string) {
            context->url = WallpaperJSUtil::Convert2String(env, argv[0]);
        } else {
            std::shared_ptr<PixelMap> tmpPixelMap = PixelMapNapi::GetPixelMap(env, argv[0]);
            if (tmpPixelMap == nullptr) {
                HILOG_ERROR("PixelMapNapi::GetPixelMap error");
                context->isPixelEmp = true;
                return napi_generic_failure;
            } else {
                context->isPixelEmp = false;
            }
            context->pixelMap = std::make_unique<PixelMap>(*tmpPixelMap);
        }
        valueType = napi_undefined;
        napi_typeof(env, argv[1], &valueType);
        NAPI_ASSERT_BASE(env, valueType == napi_number, "second param type wrong!", napi_invalid_arg);
        napi_get_value_int32(env, argv[1], &context->wallpaperType);
        HILOG_DEBUG("input ---- wallpaperType : %{public}d", context->wallpaperType);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        bool urlret = false;
        if (context->url.length() == 0) {
            HILOG_DEBUG("exec ---- setWallpaper by pixelMap");
            if (!context->isPixelEmp) {
                urlret = WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(context->pixelMap,
                    context->wallpaperType);
            }
        } else {
            HILOG_DEBUG("exec ---- setWallpaper by url");
            urlret = WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(context->url,
                context->wallpaperType);
        }
        if (urlret) {
            context->status = napi_ok;
        }
        HILOG_DEBUG("exec ---- context->status[%{public}d]", context->status);
    };
    context->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), TWO);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_ScreenshotLiveWallpaper(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_ScreenshotLiveWallpaper in");
    auto context = std::make_shared<GetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->SetErrInfo(AsyncCall::ErrorCode::NOT_SUPPORT, "not support");
        HILOG_DEBUG("exec-----NAPI_ScreenshotLiveWallpaper in");
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), TWO);
    return asyncCall.Call(env, exec);
}

std::shared_ptr<WallpaperMgrService::WallpaperColorChangeListener> colorChangeListener_;

napi_value NAPI_On(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_On in");
    auto context = std::make_shared<GetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->SetErrInfo(AsyncCall::ErrorCode::NOT_SUPPORT, "not support");
        HILOG_DEBUG("exec-----NAPI_On in");
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), ONE);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_Off(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_Off in");
    auto context = std::make_shared<GetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->SetErrInfo(AsyncCall::ErrorCode::NOT_SUPPORT, "not support");
        HILOG_DEBUG("exec-----NAPI_Off in");
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), ONE);
    return asyncCall.Call(env, exec);
}

napi_value NAPI_GetFile(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("NAPI_GetFile in");
    auto context = std::make_shared<GetFileContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->SetErrInfo(AsyncCall::ErrorCode::NOT_SUPPORT, "not support");
        HILOG_DEBUG("exec ---- NAPI_GetFile in");
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), 1);
    return asyncCall.Call(env, exec);
}

NapiWallpaperAbility::NapiWallpaperAbility(napi_env env, napi_value callback)
    : env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

NapiWallpaperAbility::~NapiWallpaperAbility()
{
    napi_delete_reference(env_, callback_);
}

void NapiWallpaperAbility::onColorsChange(std::vector<RgbaColor> color, int wallpaperType)
{
    WallpaperMgrService::WallpaperColorChangeListener::onColorsChange(color, wallpaperType);
    EventDataWorker *eventDataWorker = new EventDataWorker(this, color, wallpaperType);
    uv_work_t *work = new uv_work_t;
    work->data = eventDataWorker;
    uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            EventDataWorker *eventDataInner = reinterpret_cast<EventDataWorker *>(work->data);
            napi_value jsWallpaperType = nullptr;
            napi_create_int32(eventDataInner->listener->env_, eventDataInner->wallpaperType, &jsWallpaperType);
            napi_value jsRgbaArray = WallpaperJSUtil::Convert2JSRgbaArray(eventDataInner->listener->env_,
                eventDataInner->color);
            napi_value callback = nullptr;
            napi_value args[2] = {jsRgbaArray, jsWallpaperType};
            napi_get_reference_value(eventDataInner->listener->env_, eventDataInner->listener->callback_, &callback);
            napi_value global = nullptr;
            napi_get_global(eventDataInner->listener->env_, &global);
            napi_value result;
            napi_status callStatus = napi_call_function(eventDataInner->listener->env_, global, callback, 1, args,
                &result);
            if (callStatus != napi_ok) {
                HILOG_ERROR("notify data change failed callStatus:%{public}d callback:%{public}p", callStatus,
                    callback);
            }
            delete eventDataInner;
            eventDataInner = nullptr;
            delete work;
            work = nullptr;
        });
}
}
}
