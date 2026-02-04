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

#include "permission_utils_mock.h"

#include "gtest/gtest.h"
#include <thread>
#include "hilog_wrapper.h"
namespace OHOS {
namespace WallpaperMgrService {
using namespace OHOS::Security::AccessToken;
namespace {
std::mutex g_lockSetToken;
uint64_t g_shellTokenId = 0;
}

void MockPermission::SetTestEvironment(uint64_t shellTokenId)
{
    std::lock_guard<std::mutex> lock(g_lockSetToken);
    g_shellTokenId = shellTokenId;
}

void MockPermission::ResetTestEvironment()
{
    std::lock_guard<std::mutex> lock(g_lockSetToken);
    g_shellTokenId = 0;
}

uint64_t MockPermission::GetShellTokenId()
{
    std::lock_guard<std::mutex> lock(g_lockSetToken);
    return g_shellTokenId;
}

AccessTokenID GetNativeTokenIdFromProcess(const std::string &process)
{
    uint64_t selfTokenId = GetSelfTokenID();
    EXPECT_EQ(0, SetSelfTokenID(MockPermission::GetShellTokenId()));
    std::string dumpInfo;
    AtmToolsParamInfo info;
    info.processName = process;
    AccessTokenKit::DumpTokenInfo(info, dumpInfo);
    size_t pos = dumpInfo.find("\"tokenID\": ");
    if (pos == std::string::npos) {
        return 0;
    }
    pos += std::string("\"tokenID\": ").length();
    std::string numStr;
    while (pos < dumpInfo.length() && std::isdigit(dumpInfo[pos])) {
        numStr += dumpInfo[pos];
        ++pos;
    }
    EXPECT_EQ(0, SetSelfTokenID(selfTokenId));

    std::istringstream iss(numStr);
    AccessTokenID tokenID;
    iss >> tokenID;
    return tokenID;
}

MockToken::MockToken(const std::string &process)
{
    selfToken_ = GetSelfTokenID();
    uint32_t tokenId = GetNativeTokenIdFromProcess(process);
    SetSelfTokenID(tokenId);
}

MockToken::~MockToken()
{
    SetSelfTokenID(selfToken_);
}

AccessTokenIDEx MockPermission::GetHapTokenIdFromBundle(
    int32_t userID, const std::string &bundleName, int32_t instIndex)
{
    uint64_t selfTokenId = GetSelfTokenID();
    ATokenTypeEnum type = AccessTokenKit::GetTokenTypeFlag(static_cast<AccessTokenID>(selfTokenId));
    if (type != TOKEN_NATIVE) {
        AccessTokenID tokenId1 = GetNativeTokenIdFromProcess("accesstoken_service");
        EXPECT_EQ(0, SetSelfTokenID(tokenId1));
    }
    AccessTokenIDEx tokenIdEx = AccessTokenKit::GetHapTokenIDEx(userID, bundleName, instIndex);

    EXPECT_EQ(0, SetSelfTokenID(selfTokenId));
    return tokenIdEx;
}

AccessTokenIDEx MockPermission::AllocTestHapToken(const HapInfoParams &hapInfo, HapPolicyParams &hapPolicy)
{
    AccessTokenIDEx tokenIdEx = {0};
    uint64_t selfTokenId = GetSelfTokenID();
    for (auto &permissionStateFull : hapPolicy.permStateList) {
        PermissionDef permDefResult;
        if (AccessTokenKit::GetDefPermission(permissionStateFull.permissionName, permDefResult) != RET_SUCCESS) {
            continue;
        }
        if (permDefResult.availableLevel > hapPolicy.apl) {
            hapPolicy.aclRequestedList.emplace_back(permissionStateFull.permissionName);
        }
    }
    if (GetNativeTokenIdFromProcess("foundation") == selfTokenId) {
        AccessTokenKit::InitHapToken(hapInfo, hapPolicy, tokenIdEx);
    } else {
        MockToken mock("foundation");
        AccessTokenKit::InitHapToken(hapInfo, hapPolicy, tokenIdEx);
        EXPECT_EQ(0, SetSelfTokenID(selfTokenId));
    }
    return tokenIdEx;
}

int32_t MockPermission::DeleteTestHapToken(AccessTokenID tokenID)
{
    uint64_t selfTokenId = GetSelfTokenID();
    if (GetNativeTokenIdFromProcess("foundation") == selfTokenId) {
        return AccessTokenKit::DeleteToken(tokenID);
    }
    MockToken mock("foundation");
    int32_t ret = AccessTokenKit::DeleteToken(tokenID);
    EXPECT_EQ(0, SetSelfTokenID(selfTokenId));
    return ret;
}

MockHapToken::MockHapToken(
    const std::string &bundle, const std::vector<std::string> &reqPerm, bool isSystemApp)
{
    selfToken_ = GetSelfTokenID();
    HapInfoParams infoParams = {
        .userID = 0,
        .bundleName = bundle,
        .instIndex = 0,
        .appIDDesc = "AccessTokenTestAppID",
        .apiVersion = MockPermission::DEFAULT_API_VERSION,
        .isSystemApp = isSystemApp,
        .appDistributionType = "",
    };

    HapPolicyParams policyParams = {
        .apl = APL_NORMAL,
        .domain = "accesstoken_test_domain",
    };
    for (size_t i = 0; i < reqPerm.size(); ++i) {
        PermissionDef permDefResult;
        if (AccessTokenKit::GetDefPermission(reqPerm[i], permDefResult) != RET_SUCCESS) {
            continue;
        }
        PermissionStateFull permState = {
            .permissionName = reqPerm[i],
            .isGeneral = true,
            .resDeviceID = {"local3"},
            .grantStatus = {PermissionState::PERMISSION_DENIED},
            .grantFlags = {PermissionFlag::PERMISSION_DEFAULT_FLAG}
        };
        policyParams.permStateList.emplace_back(permState);
        if (permDefResult.availableLevel > policyParams.apl) {
            policyParams.aclRequestedList.emplace_back(reqPerm[i]);
        }
    }

    AccessTokenIDEx tokenIdEx = MockPermission::AllocTestHapToken(infoParams, policyParams);
    mockToken_= tokenIdEx.tokenIdExStruct.tokenID;
    EXPECT_NE(mockToken_, INVALID_TOKENID);
    EXPECT_EQ(0, SetSelfTokenID(tokenIdEx.tokenIDEx));
}

MockHapToken::~MockHapToken()
{
    if (mockToken_ != INVALID_TOKENID) {
        EXPECT_EQ(0, MockPermission::DeleteTestHapToken(mockToken_));
    }
    EXPECT_EQ(0, SetSelfTokenID(selfToken_));
}
} // namespace WallpaperMgrService
} // namespace OHOS