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
#ifndef FRAMEWORKS_UV_QUEUE_H
#define FRAMEWORKS_UV_QUEUE_H

#include <uv.h>

#include "napi/native_api.h"

namespace OHOS::MiscServices {
struct WorkData {
    napi_env env_;
    napi_ref callback_;
    int32_t wallpaperType;
    WorkData(napi_env env, napi_ref callbackRef = nullptr, int32_t wallpaperType = 0)
        : env_(env), callback_(callbackRef), wallpaperType(wallpaperType)
    {
    }
};
} // namespace OHOS::MiscServices
#endif // FRAMEWORKS_UV_QUEUE_H
