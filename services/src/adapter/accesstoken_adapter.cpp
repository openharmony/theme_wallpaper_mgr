/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "accesstoken_adapter.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace WallpaperMgrService {
std::shared_ptr<AccessTokenAdapter> AccessTokenProxy::accessToken_ = nullptr;
std::mutex AccessTokenProxy::mtx;
void AccessTokenProxy::Set(std::shared_ptr<AccessTokenAdapter> accessTokenAdapter)
{
    HILOG_INFO("AccessTokenProxy Set.");
    std::lock_guard<std::mutex> lock(mtx);
    accessToken_ = accessTokenAdapter;
}

int AccessTokenProxy::VerifyAccessToken(Security::AccessToken::AccessTokenID tokenID, const std::string &permissionName)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (accessToken_ == nullptr) {
        accessToken_ = std::make_shared<AccessTokenImpl>();
    }
    return accessToken_->VerifyAccessToken(tokenID, permissionName);
}

AccessTokenImpl::~AccessTokenImpl()
{
}

int AccessTokenImpl::VerifyAccessToken(Security::AccessToken::AccessTokenID tokenID, const std::string &permissionName)
{
    HILOG_INFO("Enter VerifyAccessToken.");
    return Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenID, permissionName);
}
} // namespace WallpaperMgrService
} // namespace OHOS
