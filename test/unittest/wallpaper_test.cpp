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

#include <ctime>
#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "directory_ex.h"
#include "file_deal.h"
#include "hilog_wrapper.h"
#include "image_packer.h"
#include "nativetoken_kit.h"
#include "pixel_map.h"
#include "token_setproc.h"
#include "wallpaper_common_event.h"
#include "wallpaper_manager.h"
#include "wallpaper_manager_kits.h"
#include "wallpaper_service.h"

namespace OHOS {
namespace WallpaperMgrService {
constexpr int32_t SYSTYEM = 0;
constexpr int32_t LOCKSCREEN = 1;
constexpr int32_t INVALID_WALLPAPER_TYPE = 2;
constexpr int32_t HUNDRED = 100;
constexpr int32_t DEFAULT_WALLPAPER_ID = -1;
constexpr int32_t FOO_MAX_LEN = 60000000;
constexpr int32_t USERID = 99;
constexpr const char *URI = "/data/test/theme/wallpaper/wallpaper_test.JPG";
constexpr const char *USER_SYSTEM_FILE = "/data/service/el1/public/wallpaper/99/system/wallpaper_system_orig";
constexpr const char *USER_SYSTEM_CROP_FILE = "/data/service/el1/public/wallpaper/99/system/wallpaper_system";
constexpr const char *USER_LOCKSCREEN_FILE = "/data/service/el1/public/wallpaper/99/lockscreen/wallpaper_lock_orig";
constexpr const char *USER_LOCKSCREEN_CROP_FILE = "/data/service/el1/public/wallpaper/99/lockscreen/wallpaper_lock";
constexpr const char *USER_FILE_DIR = "/data/service/el1/public/wallpaper/99";
std::shared_ptr<WallpaperCommonEvent> subscriber = nullptr;

using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;
using namespace OHOS::MiscServices;
using namespace OHOS::Security::AccessToken;

void GrantNativePermission()
{
    const char **perms = new const char *[2];
    perms[0] = "ohos.permission.GET_WALLPAPER";
    perms[1] = "ohos.permission.SET_WALLPAPER";
    TokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "wallpaper_service",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    int res = SetSelfTokenID(tokenId);
    if (res == 0) {
        HILOG_INFO("SetSelfTokenID success!");
    } else {
        HILOG_ERROR("SetSelfTokenID fail!");
    }
    AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}

class WallpaperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void CreateTempImage();
    static std::shared_ptr<PixelMap> CreateTempPixelMap();
    static void SubscribeCommonEvent();
    static void TriggerEvent(const std::string &CommonEventSupport);
};
const std::string VALID_SCHEMA_STRICT_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
                                               "\"SCHEMA_MODE\":\"STRICT\","
                                               "\"SCHEMA_SKIPSIZE\":0,"
                                               "\"SCHEMA_DEFINE\":{"
                                               "\"age\":\"INTEGER, NOT NULL\""
                                               "},"
                                               "\"SCHEMA_INDEXES\":[\"$.age\"]}";

void WallpaperTest::SetUpTestCase(void)
{
    HILOG_INFO("SetUpTestCase");
    GrantNativePermission();
    CreateTempImage();
    SubscribeCommonEvent();
    HILOG_INFO("SetUpTestCase end");
}

void WallpaperTest::TearDownTestCase(void)
{
    HILOG_INFO("TearDownTestCase");
    WallpaperManagerkits::GetInstance().ResetWallpaper(SYSTYEM);
    WallpaperManagerkits::GetInstance().ResetWallpaper(LOCKSCREEN);
}

void WallpaperTest::SetUp(void)
{
}

void WallpaperTest::TearDown(void)
{
}

class WallpaperColorChangeListenerTestImpl : public WallpaperColorChangeListener {
public:
    std::vector<uint64_t> color_;
    int wallpaperType_;
    WallpaperColorChangeListenerTestImpl();
    ~WallpaperColorChangeListenerTestImpl()
    {
    }

    WallpaperColorChangeListenerTestImpl(const WallpaperColorChangeListenerTestImpl &) = delete;
    WallpaperColorChangeListenerTestImpl &operator=(const WallpaperColorChangeListenerTestImpl &) = delete;
    WallpaperColorChangeListenerTestImpl(WallpaperColorChangeListenerTestImpl &&) = delete;
    WallpaperColorChangeListenerTestImpl &operator=(WallpaperColorChangeListenerTestImpl &&) = delete;

    // callback function will be called when the db data is changed.
    void OnColorsChange(const std::vector<uint64_t> &color, int wallpaperType);

    // reset the callCount_ to zero.
    void ResetToZero();

    unsigned long GetCallCount() const;

private:
    unsigned long callCount_;
};

void WallpaperColorChangeListenerTestImpl::OnColorsChange(const std::vector<uint64_t> &color, int wallpaperType)
{
    callCount_++;
    for (auto const &each : color) {
        color_.push_back(each);
    }
    wallpaperType_ = wallpaperType;
}

WallpaperColorChangeListenerTestImpl::WallpaperColorChangeListenerTestImpl()
{
    callCount_ = 0;
    wallpaperType_ = -1;
}

void WallpaperColorChangeListenerTestImpl::ResetToZero()
{
    callCount_ = 0;
}

unsigned long WallpaperColorChangeListenerTestImpl::GetCallCount() const
{
    return callCount_;
}

void WallpaperTest::CreateTempImage()
{
    std::shared_ptr<PixelMap> pixelMap = CreateTempPixelMap();
    ImagePacker imagePacker;
    PackOption option;
    option.format = "image/jpeg";
    option.quality = HUNDRED;
    option.numberHint = 1;
    std::set<std::string> formats;
    imagePacker.GetSupportedFormats(formats);
    imagePacker.StartPacking(URI, option);
    HILOG_INFO("AddImage start");
    imagePacker.AddImage(*pixelMap);
    int64_t packedSize = 0;
    HILOG_INFO("FinalizePacking start");
    imagePacker.FinalizePacking(packedSize);
    if (packedSize == 0) {
        HILOG_INFO("FinalizePacking error");
    }
}

std::shared_ptr<PixelMap> WallpaperTest::CreateTempPixelMap()
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> uniquePixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMap = std::move(uniquePixelMap);
    return pixelMap;
}

void WallpaperTest::SubscribeCommonEvent()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber = std::make_shared<WallpaperCommonEvent>(subscriberInfo);
    if (subscriber == nullptr) {
        HILOG_INFO("subscriber is nullptr");
    }
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber)) {
        HILOG_INFO("SubscribeCommonEvent  failed");
    }
}

void WallpaperTest::TriggerEvent(const std::string &CommonEventSupport)
{
    EventFwk::Want want;
    want.SetAction(CommonEventSupport);
    int32_t code = USERID;
    std::string data(CommonEventSupport);
    EventFwk::CommonEventData eventData(want, code, data);
    subscriber->OnReceiveEvent(eventData);
}

/*********************   ResetWallpaper   *********************/
/**
* @tc.name:    Reset001
* @tc.desc:    Reset wallpaper with wallpaperType[0].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, Reset001, TestSize.Level1)
{
    HILOG_INFO("Reset001 begin.");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to reset.";
}

/**
* @tc.name:    Reset002
* @tc.desc:    Reset wallpaper with wallpaperType[1].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, Reset002, TestSize.Level1)
{
    HILOG_INFO("Reset002 begin.");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to reset.";
}

/**
* @tc.name:    Reset003
* @tc.desc:    Reset wallpaper with wallpaperType[2] throw parameters error.
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, Reset003, TestSize.Level1)
{
    HILOG_INFO("Reset003 begin.");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(INVALID_WALLPAPER_TYPE);
    EXPECT_EQ(wallpaperErrorCode, E_PARAMETERS_INVALID) << "Failed to throw error.";
}

/**
* @tc.name:    Reset004
* @tc.desc:    Reset wallpaper with wallpaperType[0] after resetting wallpaper[0].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, Reset004, TestSize.Level1)
{
    HILOG_INFO("Reset004 begin.");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to reset.";

    /* duplicate reset */
    wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to reset.";
}

/**
* @tc.name:    Reset005
* @tc.desc:    Reset wallpaper with wallpaperType[1] after resetting wallpaper[1] and check whether Id is same one.
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, Reset005, TestSize.Level1)
{
    HILOG_INFO("Reset005 begin.");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to reset.";
    int firstId = WallpaperManagerkits::GetInstance().GetWallpaperId(LOCKSCREEN);

    /* duplicate reset */
    wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to reset.";
    int secondId = WallpaperManagerkits::GetInstance().GetWallpaperId(LOCKSCREEN);
    EXPECT_EQ(firstId, secondId) << "Id should be same one.";
}
/*********************   ResetWallpaper   *********************/

/*********************   IsChangePermitted   *********************/

/**
* @tc.name: IsChangePermitted001
* @tc.desc: check permission.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, IsChangePermitted001, TestSize.Level1)
{
    EXPECT_EQ(WallpaperManagerkits::GetInstance().IsChangePermitted(), true);
}

/*********************   IsChangePermitted   *********************/

/*********************   IsOperationAllowed   *********************/

/**
* @tc.name: IsOperationAllowed001
* @tc.desc: check permission.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, IsOperationAllowed001, TestSize.Level1)
{
    EXPECT_EQ(WallpaperManagerkits::GetInstance().IsOperationAllowed(), true);
}

/*********************   IsOperationAllowed   *********************/

/*********************   On & Off   *********************/

/**
* @tc.name: On001
* @tc.desc: set wallpaper and get callback.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, On001, TestSize.Level1)
{
    HILOG_INFO("On001 begin");
    auto listener = std::make_shared<WallpaperColorChangeListenerTestImpl>();
    WallpaperManagerkits::GetInstance().On("colorChange", listener);

    auto offSubStatus = WallpaperManagerkits::GetInstance().Off("colorChange", listener);
    EXPECT_EQ(offSubStatus, true) << "unsubscribe wallpaper color change.";
}

/*********************   On & Off   *********************/

/*********************   GetColors   *********************/
/**
* @tc.name: GetColors001
* @tc.desc: GetColors with wallpaperType[0].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, GetColors001, TestSize.Level0)
{
    HILOG_INFO("GetColors001 begin");
    std::vector<uint64_t> color = WallpaperManagerkits::GetInstance().GetColors(SYSTYEM);
    EXPECT_FALSE(color.empty());
}

/**
* @tc.name: GetColors002
* @tc.desc: GetColors with wallpaperType[1].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, GetColors002, TestSize.Level0)
{
    HILOG_INFO("GetColors002 begin");
    std::vector<uint64_t> color = WallpaperManagerkits::GetInstance().GetColors(LOCKSCREEN);
    EXPECT_FALSE(color.empty());
}
/*********************   GetColors   *********************/

/*********************   GetId   *********************/
/**
* @tc.name: GetId001
* @tc.desc: GetId with wallpaperType[0].
* @tc.type: FUNC
* @tc.require: issueI65VF1
* @tc.author: lvbai
*/
HWTEST_F(WallpaperTest, GetId001, TestSize.Level0)
{
    HILOG_INFO("GetId001 begin");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to ResetWallpaper";
    int32_t id = WallpaperManagerkits::GetInstance().GetWallpaperId(SYSTYEM);
    EXPECT_EQ(id, DEFAULT_WALLPAPER_ID) << "Failed to GetId";
}

/**
* @tc.name: GetId002
* @tc.desc: GetId with wallpaperType[1].
* @tc.type: FUNC
* @tc.require: issueI65VF1
* @tc.author: lvbai
*/
HWTEST_F(WallpaperTest, GetId002, TestSize.Level0)
{
    HILOG_INFO("GetId002 begin");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().ResetWallpaper(LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to ResetWallpaper";
    int32_t id = WallpaperManagerkits::GetInstance().GetWallpaperId(LOCKSCREEN);
    EXPECT_EQ(id, DEFAULT_WALLPAPER_ID) << "Failed to GetId";
}

/**
* @tc.name: GetId003
* @tc.desc: GetId with wallpaperType[0] after setWallpaper.
* @tc.type: FUNC
* @tc.require: issueI65VF1
* @tc.author: lvbai
*/
HWTEST_F(WallpaperTest, GetId003, TestSize.Level0)
{
    HILOG_INFO("GetId003 begin");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(URI, SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to SetWallpaper";
    int32_t id = WallpaperManagerkits::GetInstance().GetWallpaperId(SYSTYEM);
    EXPECT_GT(id, DEFAULT_WALLPAPER_ID) << "Failed to GetId";
}

/**
* @tc.name: GetId004
* @tc.desc: GetId with wallpaperType[1] after setWallpaper.
* @tc.type: FUNC
* @tc.require: issueI65VF1
* @tc.author: lvbai
*/
HWTEST_F(WallpaperTest, GetId004, TestSize.Level0)
{
    HILOG_INFO("GetId004 begin");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(URI, LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to SetWallpaper";
    int32_t id = WallpaperManagerkits::GetInstance().GetWallpaperId(LOCKSCREEN);
    EXPECT_GT(id, DEFAULT_WALLPAPER_ID) << "Failed to GetId";
}
/*********************   GetId   *********************/

/*********************   GetFile   *********************/
/**
* @tc.name:    GetFile001
* @tc.desc:    GetFile with wallpaperType[0].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, GetFile001, TestSize.Level0)
{
    HILOG_INFO("GetFile001 begin");
    int32_t wallpaperFd = 0;
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().GetFile(SYSTYEM, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to get File.";
}

/**
* @tc.name:    GetFile002
* @tc.desc:    GetFile with wallpaperType[1].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, GetFile002, TestSize.Level0)
{
    HILOG_INFO("GetFile002 begin");
    int32_t wallpaperFd = 0;
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().GetFile(LOCKSCREEN, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to get File.";
}

/**
* @tc.name:    GetFile003
* @tc.desc:    GetFile with wallpaperType[2] throw parameters error.
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, GetFile003, TestSize.Level0)
{
    HILOG_INFO("GetFile003 begin");
    int32_t wallpaperFd = 0;
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().GetFile(INVALID_WALLPAPER_TYPE, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, E_PARAMETERS_INVALID) << "Failed to throw parameters error";
}

/**
* @tc.name:    GetFile004
* @tc.desc:    GetFile with wallpaperType[0].
* @tc.type:    FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, GetFile004, TestSize.Level0)
{
    HILOG_INFO("GetFile001 begin");
    int32_t wallpaperFd = 0;
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().GetFile(SYSTYEM, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to get File.";
}
/*********************   GetFile   *********************/

/*********************   GetWallpaperMinHeight   *********************/
/**
* @tc.name: getMinHeight001
* @tc.desc: GetWallpaperMinHeight .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, getMinHeight001, TestSize.Level0)
{
    HILOG_INFO("WallpaperReset001  begin");
    bool result = false;
    int height = 0;
    height = WallpaperManagerkits::GetInstance().GetWallpaperMinHeight();
    if (height != 0) {
        result = true;
    }
    EXPECT_TRUE(result);
}
/*********************   GetWallpaperMinHeight   *********************/

/*********************   GetWallpaperMinWidth   *********************/
/**
* @tc.name: getMinWidth001
* @tc.desc: GetWallpaperMinWidth .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, getMinWidth001, TestSize.Level0)
{
    HILOG_INFO("getMinWidth001  begin");
    bool result = false;
    int width = 0;
    width = WallpaperManagerkits::GetInstance().GetWallpaperMinWidth();
    if (width != 0) {
        result = true;
    }
    EXPECT_TRUE(result);
}
/*********************   GetWallpaperMinWidth   *********************/

/*********************   GetPixelMap   *********************/
/**
* @tc.name:    GetPixelMap001
* @tc.desc:    GetPixelMap with wallpaperType[0].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, GetPixelMap001, TestSize.Level0)
{
    HILOG_INFO("GetPixelMap001  begin");
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().GetPixelMap(SYSTYEM, pixelMap);
    EXPECT_EQ(wallpaperErrorCode, E_NOT_SYSTEM_APP) << "Failed to throw permission error.";
}

/**
* @tc.name:    GetPixelMap002
* @tc.desc:    GetPixelMap with wallpaperType[1].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, GetPixelMap002, TestSize.Level0)
{
    HILOG_INFO("GetPixelMap002  begin");
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().GetPixelMap(LOCKSCREEN, pixelMap);
    EXPECT_EQ(wallpaperErrorCode, E_NOT_SYSTEM_APP) << "Failed to throw permission error.";
}
/*********************   GetPixelMap   *********************/

/*********************   SetWallpaperByMap   *********************/
/**
* @tc.name:    SetWallpaperByMap001
* @tc.desc:    SetWallpaperByMap with wallpaperType[0].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByMap001, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByMap001  begin");
    std::shared_ptr<PixelMap> pixelMap = WallpaperTest::CreateTempPixelMap();
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to set SYSTYEM PixelMap.";
}

/**
* @tc.name:    SetWallpaperByMap002
* @tc.desc:    SetWallpaperByMap with wallpaperType[1].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByMap002, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByMap002  begin");
    std::shared_ptr<PixelMap> pixelMap = WallpaperTest::CreateTempPixelMap();
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to set LOCKSCREEN PixelMap.";
}

/**
* @tc.name:    SetWallpaperByMap003
* @tc.desc:    SetWallpaperByMap with wallpaperType[2] throw parameters error.
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByMap003, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByMap003  begin");
    std::shared_ptr<PixelMap> pixelMap = WallpaperTest::CreateTempPixelMap();
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, INVALID_WALLPAPER_TYPE);
    EXPECT_EQ(wallpaperErrorCode, E_PARAMETERS_INVALID) << "Failed to throw parameters error";
}
/*********************   SetWallpaperByMap   *********************/

/*********************   SetWallpaperByUri   *********************/
/**
* @tc.name:    SetWallpaperByUri001
* @tc.desc:    SetWallpaperByUri with wallpaperType[0] .
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByUri001, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByUri001  begin");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(URI, SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to set SYSTYEM.";
}

/**
* @tc.name:    SetWallpaperByUri002
* @tc.desc:    SetWallpaperByUri with wallpaperType[1].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByUri002, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByUri002  begin");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(URI, LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "Failed to set LOCKSCREEN.";
}

/**
* @tc.name:    SetWallpaperByUri003
* @tc.desc:    SetWallpaperByUri with wallpaperType[2] throw parameters error.
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByUri003, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByUri003  begin");
    ErrorCode wallpaperErrorCode = WallpaperManagerkits::GetInstance().SetWallpaper(URI, INVALID_WALLPAPER_TYPE);
    EXPECT_EQ(wallpaperErrorCode, E_PARAMETERS_INVALID) << "Failed to throw error";
}

/**
* @tc.name:    SetWallpaperByUri004
* @tc.desc:    SetWallpaperByUri with error uri.
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByUri004, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByUri004  begin");
    ErrorCode wallpaperErrorCode =
        WallpaperManagerkits::GetInstance().SetWallpaper("/data/test/theme/wallpaper/errorURI", LOCKSCREEN);
    EXPECT_NE(wallpaperErrorCode, E_OK) << "Failed to throw error";
}

/**
* @tc.name:    SetWallpaperByUri005
* @tc.desc:    SetWallpaperByUri with unsafe uri.
* @tc.type:    FUNC
* @tc.require: issueI647HI
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, SetWallpaperByUri005, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByUri005  begin");
    ErrorCode wallpaperErrorCode =
        WallpaperManagerkits::GetInstance().SetWallpaper("../data/test/theme/wallpaper/errorURI", LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_FILE_ERROR) << "Failed to return error";
}
/*********************   SetWallpaperByUri   *********************/

/*********************   FILE_DEAL   *********************/
/**
* @tc.name:    FILE_DEAL001
* @tc.desc:    File operation-related interfaces
* @tc.type:    FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, FILE_DEAL001, TestSize.Level0)
{
    HILOG_INFO("FILE_DEAL001  begin");
    FileDeal fileOperation;
    bool isExist = fileOperation.Mkdir("/data/test/theme/wallpaper/");
    EXPECT_EQ(isExist, true);
    isExist = fileOperation.Mkdir("/data/test/theme/errorURI/");
    EXPECT_EQ(isExist, true);
    isExist = fileOperation.IsFileExist(URI);
    EXPECT_EQ(isExist, true);
    isExist = fileOperation.IsFileExist("/data/test/theme/wallpaper/errorURI");
    EXPECT_EQ(isExist, false);
}
/*********************   FILE_DEAL   *********************/

/**
* @tc.name:    SetWallpaper001
* @tc.desc:    SetWallpaper with error length
* @tc.type:    FUNC
* @tc.require: issueI6AW6M
* @tc.author:  weishaoxiong
*/
HWTEST_F(WallpaperTest, SetWallpaper001, TestSize.Level0)
{
    HILOG_INFO("SetWallpaper001  begin");
    ErrorCode wallpaperErrorCode = WallpaperService::GetInstance()->SetWallpaper(0, 0, -1);
    EXPECT_EQ(wallpaperErrorCode, E_PARAMETERS_INVALID) << "Failed to throw error";
    wallpaperErrorCode = WallpaperService::GetInstance()->SetWallpaper(0, 0, FOO_MAX_LEN);
    EXPECT_EQ(wallpaperErrorCode, E_PARAMETERS_INVALID) << "Failed to throw error";
}

/*********************   USER_DEAL   *********************/
/**
* @tc.name:    AddUsersDeal001
* @tc.desc:    Create a user directory after the user is added
* @tc.type:    FUNC
* @tc.require:
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, AddUsersDeal001, TestSize.Level0)
{
    HILOG_INFO("AddUsersDeal001  begin");
    std::string commonEvent = EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED;
    WallpaperTest::TriggerEvent(commonEvent);

    bool ret = FileDeal::IsFileExist(USER_SYSTEM_FILE);
    EXPECT_EQ(ret, true);
    ret = FileDeal::IsFileExist(USER_SYSTEM_CROP_FILE);
    EXPECT_EQ(ret, true);
    ret = FileDeal::IsFileExist(USER_LOCKSCREEN_FILE);
    EXPECT_EQ(ret, true);
    ret = FileDeal::IsFileExist(USER_LOCKSCREEN_CROP_FILE);
    EXPECT_EQ(ret, true);
}

/*********************   USER_DEAL   *********************/
/**
* @tc.name:    RemovedUserDeal001
* @tc.desc:    delete a user directory after the user is removed
* @tc.type:    FUNC
* @tc.require:
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, RemovedUserDeal001, TestSize.Level0)
{
    HILOG_INFO("RemovedUserDeal001  begin");
    std::string commonEvent = EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED;
    WallpaperTest::TriggerEvent(commonEvent);
    EXPECT_EQ(FileDeal::IsDirExist(USER_FILE_DIR), true);

    commonEvent = EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED;
    WallpaperTest::TriggerEvent(commonEvent);
    EXPECT_EQ(FileDeal::IsDirExist(USER_FILE_DIR), false);
}
/*********************   USER_DEAL   *********************/
} // namespace WallpaperMgrService
} // namespace OHOS