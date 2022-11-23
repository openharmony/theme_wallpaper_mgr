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

#include "hilog_wrapper.h"

namespace OHOS::WallpaperNAPI {
const static int32_t ARRAY_LENGTH = 4;
const static int32_t ZERO = 0;
const static int32_t ONE = 1;
const static int32_t TWO = 2;
const static int32_t THREE = 3;

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
    char *buf = new (std::nothrow) char[maxLen + 1];
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

napi_value WallpaperJSUtil::Convert2JSRgbaArray(napi_env env, const std::vector<uint32_t> &color)
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
        RgbaColor color;
        color.red = (it >> RED_OFFSET) & BYTE_MASK;
        color.green = (it >> GREEN_OFFSET) & BYTE_MASK;
        color.blue = (it >> BLUE_OFFSET) & BYTE_MASK;
        color.alpha = (it >> ALPHA_OFFSET) & BYTE_MASK;
        napi_value element = nullptr;
        napi_create_array_with_length(env, ARRAY_LENGTH, &element);
        napi_value jsRgba = nullptr;
        napi_create_int32(env, color.red, &jsRgba);
        napi_set_element(env, element, ZERO, jsRgba);
        jsRgba = nullptr;
        napi_create_int32(env, color.green, &jsRgba);
        napi_set_element(env, element, ONE, jsRgba);
        jsRgba = nullptr;
        napi_create_int32(env, color.blue, &jsRgba);
        napi_set_element(env, element, TWO, jsRgba);
        jsRgba = nullptr;
        napi_create_int32(env, color.alpha, &jsRgba);
        napi_set_element(env, element, THREE, jsRgba);
        napi_set_element(env, result, index++, element);
    }
    return result;
}
} // namespace OHOS::WallpaperNAPI