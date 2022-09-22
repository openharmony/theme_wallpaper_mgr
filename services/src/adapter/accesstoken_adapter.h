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
#ifndef SERVICES_INCLUDE_ACCESSTOKEN_ADAPTER_H
#define SERVICES_INCLUDE_ACCESSTOKEN_ADAPTER_H

#include <memory>
#include <mutex>
#include "access_token.h"
#include "accesstoken_kit.h"
#include "singleton.h"
namespace OHOS {
namespace WallpaperMgrService {
class AccessTokenAdapter {
public:
    virtual ~AccessTokenAdapter() = default;
    virtual int VerifyAccessToken(Security::AccessToken::AccessTokenID tokenID, const std::string &permissionName) = 0;
};

class AccessTokenProxy {
public:
    virtual ~AccessTokenProxy() = default;
    static void Set(std::shared_ptr<AccessTokenAdapter> accessTokenAdapter);
    static int VerifyAccessToken(Security::AccessToken::AccessTokenID tokenID, const std::string &permissionName);

private:
    static std::shared_ptr<AccessTokenAdapter> accessToken_;
    static std::mutex mtx;
};

class AccessTokenImpl : public AccessTokenAdapter {
public:
    AccessTokenImpl() = default;
    ~AccessTokenImpl() override;

    int VerifyAccessToken(Security::AccessToken::AccessTokenID tokenID, const std::string &permissionName) override;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif