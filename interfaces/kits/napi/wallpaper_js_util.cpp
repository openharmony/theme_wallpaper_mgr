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
#define LOG_TAG "WallpaperJSUtil"
#include "wallpaper_js_util.h"
#include "export/color.h"
#include "hilog_wrapper.h"

namespace OHOS::WallpaperNAPI {
constexpr const uint32_t COLOR_MASK{ 0xFF };

std::string WallpaperJSUtil::Convert2String(napi_env env, napi_value jsString)
{
    size_t maxLen = WallpaperJSUtil::MAX_LEN;
    napi_status status = napi_get_value_string_utf8(env, jsString, NULL, 0, &maxLen);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
        maxLen = WallpaperJSUtil::MAX_LEN;
    }
    if (maxLen == 0) {
        return std::string();
    }
    char *buf = new (std::nothrow) char[maxLen + 1]();
    if (buf == nullptr) {
        return std::string();
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, jsString, buf, maxLen + 1, &len);
    if (status != napi_ok) {
        GET_AND_THROW_LAST_ERROR((env));
    }
    buf[len] = 0;
    std::string value(buf);
    delete[] buf;
    return value;
}

napi_value WallpaperJSUtil::Convert2JSRgbaArray(napi_env env, const std::vector<uint64_t> &color)
{
    HILOG_DEBUG("Convert2JSRgbaArray in");
    napi_value result = nullptr;
    napi_status status = napi_create_array_with_length(env, color.size(), &result);
    if (status != napi_ok) {
        HILOG_DEBUG("Convert2JSRgbaArray failed");
        return nullptr;
    }
    int index = 0;
    for (const auto it : color) {
        HILOG_DEBUG("Convert2JSRgbaArray for");
        ColorManager::Color colors(it);
        napi_value red = nullptr;
        napi_value green = nullptr;
        napi_value blue = nullptr;
        napi_value alpha = nullptr;
        napi_create_int32(env, static_cast<int32_t>(colors.r * COLOR_MASK), &red);
        napi_create_int32(env, static_cast<int32_t>(colors.g * COLOR_MASK), &green);
        napi_create_int32(env, static_cast<int32_t>(colors.b * COLOR_MASK), &blue);
        napi_create_int32(env, static_cast<int32_t>(colors.a * COLOR_MASK), &alpha);
        napi_value element = nullptr;
        napi_create_object(env, &element);
        napi_set_named_property(env, element, "red", red);
        napi_set_named_property(env, element, "green", green);
        napi_set_named_property(env, element, "blue", blue);
        napi_set_named_property(env, element, "alpha", alpha);
        napi_set_element(env, result, index++, element);
    }
    return result;
}
} // namespace OHOS::WallpaperNAPI