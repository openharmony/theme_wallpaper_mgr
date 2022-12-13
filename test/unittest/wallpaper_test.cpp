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
#include "wallpaper_manager.h"
#include "wallpaper_manager_kits.h"

constexpr int SYSTYEM = 0;
constexpr int LOCKSCREEN = 1;
constexpr int HUNDRED = 100;
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;
using namespace OHOS::MiscServices;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace WallpaperMgrService {
constexpr const char *URI = "/data/test/theme/wallpaper/wallpaper_test.JPG";
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
    static std::unique_ptr<PixelMap> CreateTempPixelMap();
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
    HILOG_INFO("SetUpTestCase end");
}

void WallpaperTest::TearDownTestCase(void)
{
    HILOG_INFO("TearDownTestCase");
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(0);
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(1);
}

void WallpaperTest::SetUp(void)
{
}

void WallpaperTest::TearDown(void)
{
}

class WallpaperColorChangeListenerTestImpl : public OHOS::WallpaperMgrService::WallpaperColorChangeListener {
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
    std::unique_ptr<PixelMap> pixelMap = CreateTempPixelMap();
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

std::unique_ptr<PixelMap> WallpaperTest::CreateTempPixelMap()
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    return pixelMap;
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
    HILOG_INFO("Reset001.");
    int wallpaperType = 0;
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType),
        static_cast<int32_t>(E_OK)) << "Failed to reset.";
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
    int wallpaperType = 1;
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType),
        static_cast<int32_t>(E_OK)) << "Failed to reset.";
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
    int wallpaperType = 2;
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType),
        static_cast<int32_t>(E_PARAMETERS_INVALID)) << "Failed to reset .";
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
    int wallpaperType = 0;
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType),
        static_cast<int32_t>(E_OK)) << "Failed to  reset.";
    /* duplicate reset */
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType),
        static_cast<int32_t>(E_OK)) << "Failed to  reset.";
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
    int wallpaperType = 1;
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType),
        static_cast<int32_t>(E_OK)) << "Failed to reset.";
    int firstId = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(wallpaperType);
    /* duplicate reset */
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType),
        static_cast<int32_t>(E_OK)) << "Failed to  reset.";
    int secondId = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(wallpaperType);
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
    bool ret = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().IsChangePermitted();
    EXPECT_EQ(ret, true);
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
    bool ret = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().IsOperationAllowed();
    EXPECT_EQ(ret, true);
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
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().On("colorChange", listener);

    auto offSubStatus = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().Off("colorChange", listener);
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

    std::vector<uint64_t> color = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(SYSTYEM);
    bool result = color.empty();
    EXPECT_FALSE(result);
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
    std::vector<uint64_t> color = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(LOCKSCREEN);
    bool result = color.empty();
    EXPECT_FALSE(result);
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
    bool result = false;
    int id = -1;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(SYSTYEM);
    id = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(SYSTYEM);
    if (id == -1) {
        result = true;
    }
    EXPECT_TRUE(result);
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
    bool result = false;
    int id = -1;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(LOCKSCREEN);
    id = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(LOCKSCREEN);
    if (id == -1) {
        result = true;
    }
    EXPECT_TRUE(result);
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
    bool result = false;
    int id = -1;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(URI, SYSTYEM);
    id = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(SYSTYEM);
    if (id >= 0) {
        result = true;
    }
    EXPECT_TRUE(result);
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
    bool result = false;
    int id = -1;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(URI, LOCKSCREEN);
    id = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(LOCKSCREEN);
    if (id >= 0) {
        result = true;
    }
    EXPECT_TRUE(result);
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
    int wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(SYSTYEM, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to get File.";
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
    int wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(LOCKSCREEN, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to get File.";
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
    int wallpaperErrorCode = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(2, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_PARAMETERS_INVALID)) << "Failed to throw parameters error";
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
    int wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetFile(SYSTYEM, wallpaperFd);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to get File.";
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
    int hight = 0;
    hight = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinHeight();
    if (hight != 0) {
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
    width = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperMinWidth();
    if (width != 0) {
        result = true;
    }
    EXPECT_TRUE(result);
}
/*********************   GetWallpaperMinWidth   *********************/

/*********************   GetPiexlMap   *********************/
/**
* @tc.name:    GetPiexlMap001
* @tc.desc:    GetPixelMap with wallpaperType[0].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, GetPiexlMap001, TestSize.Level0)
{
    HILOG_INFO("GetPiexlMap001  begin");
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(SYSTYEM, pixelMap);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_NOT_SYSTEM_APP)) << "throw permission error successfully.";
}

/**
* @tc.name:    GetPiexlMap002
* @tc.desc:    GetPixelMap with wallpaperType[1].
* @tc.type:    FUNC
* @tc.require: issueI5UHRG
* @tc.author:  lvbai
*/
HWTEST_F(WallpaperTest, GetPiexlMap002, TestSize.Level0)
{
    HILOG_INFO("GetPiexlMap002  begin");
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(LOCKSCREEN, pixelMap);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_NOT_SYSTEM_APP)) << "throw permission error successfully.";
}
/*********************   GetPiexlMap   *********************/

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
    std::unique_ptr<PixelMap> pixelMap = WallpaperTest::CreateTempPixelMap();
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to set SYSTYEM PiexlMap.";
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
    std::unique_ptr<PixelMap> pixelMap = WallpaperTest::CreateTempPixelMap();
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to set LOCKSCREEN PiexlMap.";
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
    std::unique_ptr<PixelMap> pixelMap = WallpaperTest::CreateTempPixelMap();
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, 2);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_PARAMETERS_INVALID)) << "Failed to throw parameters error";
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
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(URI, SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to set SYSTYEM.";
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
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(URI, LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to set LOCKSCREEN.";
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
    int32_t wallpaperErrorCode = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(URI, 2);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_PARAMETERS_INVALID)) << "Failed to throw error";
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
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper("/data/test/theme/wallpaper/"
                                                                                    "errorURI", 1);
    EXPECT_NE(wallpaperErrorCode, static_cast<int32_t>(E_OK)) << "Failed to throw error";
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
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper("../data/test/theme/wallpaper/"
                                                                                    "errorURI", 1);
    EXPECT_EQ(wallpaperErrorCode, static_cast<int32_t>(E_FILE_ERROR)) << "Failed to throw error";
}

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
    isExist = fileOperation.FileIsExist(URI);
    EXPECT_EQ(isExist, true);
    isExist = fileOperation.FileIsExist("/data/test/theme/wallpaper/errorURI");
    EXPECT_EQ(isExist, false);
}
/*********************   SetWallpaperByUri   *********************/
} // namespace WallpaperMgrService
} // namespace OHOS