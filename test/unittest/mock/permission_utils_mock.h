/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef WALLPAPER_MGR_MOCK_PERMISSION_UTILS_H
#define WALLPAPER_MGR_MOCK_PERMISSION_UTILS_H
#include "access_token.h"
#include "accesstoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace WallpaperMgrService {
using namespace OHOS::Security::AccessToken;
class MockToken {
public:
    explicit MockToken(const std::string &process);
    ~MockToken();
private:
    uint64_t selfToken_;
};

class MockPermission {
public:
    static constexpr int32_t DEFAULT_API_VERSION = 12;
    static void SetTestEvironment(uint64_t shellTokenId);
    static void ResetTestEvironment();
    static uint64_t GetShellTokenId();
    static AccessTokenIDEx AllocTestHapToken(const HapInfoParams &hapInfo, HapPolicyParams &hapPolicy);
    static int32_t DeleteTestHapToken(AccessTokenID tokenID);
    static AccessTokenIDEx GetHapTokenIdFromBundle(
        int32_t userID, const std::string &bundleName, int32_t instIndex);
};

class MockHapToken {
public:
    explicit MockHapToken(
        const std::string &bundle, const std::vector<std::string> &reqPerm, bool isSystemApp = true);
    ~MockHapToken();
private:
    uint64_t selfToken_;
    uint32_t mockToken_;
};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // WALLPAPER_MGR_MOCK_PERMISSION_UTILS_H