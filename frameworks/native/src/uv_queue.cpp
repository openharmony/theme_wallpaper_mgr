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
#include "uv_queue.h"

#include "hilog_wrapper.h"

namespace OHOS::MiscServices {
using namespace WallpaperMgrService;
bool UvQueue::Call(napi_env env, void *data, uv_after_work_cb afterCallback)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env, &loop);
    if (loop == nullptr) {
        return false;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        delete static_cast<WorkData *>(data);
        return false;
    }
    work->data = data;
    auto ret = uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {}, afterCallback, uv_qos_user_initiated);
    if (ret != 0) {
        HILOG_ERROR("uv_queue_work_with_qos faild retCode:%{public}d", ret);
        delete static_cast<WorkData *>(data);
        delete work;
        return false;
    }
    return true;
}
} // namespace OHOS::MiscServices