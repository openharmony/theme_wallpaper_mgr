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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ctime>

#include "accesstoken_adapter.h"
#include "directory_ex.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "pixel_map.h"
#include "wallpaper_manager.h"
#include "wallpaper_manager_kits.h"
#include "wallpaper_service.h"
#include "wallpaper_service_proxy.h"

constexpr int SYSTYEM = 0;
constexpr int LOCKSCREEN = 1;
constexpr int HUNDRED = 100;
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;
using namespace OHOS::MiscServices;

namespace OHOS {
namespace WallpaperMgrService {
constexpr const char *url = "/system/etc/wallpaper_white.png";
class AccessTokenMock : public AccessTokenAdapter {
public:
    AccessTokenMock() = default;
    virtual ~AccessTokenMock() = default;

    MOCK_METHOD2(VerifyAccessToken, int(Security::AccessToken::AccessTokenID, const std::string &));
};

class RemoteObjectMock : public WallpaperService {
public:
    RemoteObjectMock() = default;
    virtual ~RemoteObjectMock() = default;

    virtual int32_t ResetWallpaper(int wallpaperType)
    {
        return WallpaperService::GetInstance()->ResetWallpaper(wallpaperType);
    }
    virtual int32_t GetPixelMap(int wallpaperType, FdInfo &fdInfo)
    {
        return WallpaperService::GetInstance()->GetPixelMap(wallpaperType, fdInfo);
    }
    virtual int32_t SetWallpaperByMap(int fd, int wallpaperType, int length)
    {
        return WallpaperService::GetInstance()->SetWallpaperByMap(fd, wallpaperType, length);
    }
};

class WallpaperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::shared_ptr<AccessTokenMock> accessTokenMock_;
    static sptr<RemoteObjectMock> remoteObjMock_;
};

std::shared_ptr<AccessTokenMock> WallpaperTest::accessTokenMock_;
sptr<RemoteObjectMock> WallpaperTest::remoteObjMock_;
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
    accessTokenMock_ = std::make_shared<AccessTokenMock>();
    AccessTokenProxy::Set(accessTokenMock_);
    EXPECT_CALL(*accessTokenMock_, VerifyAccessToken(_, _)).WillRepeatedly(Return(0));
    remoteObjMock_ = new RemoteObjectMock();
    HILOG_INFO("SetUpTestCase end");
}

void WallpaperTest::TearDownTestCase(void)
{
    HILOG_INFO("TearDownTestCase");
    accessTokenMock_.reset();
    AccessTokenProxy::Set(nullptr);
    remoteObjMock_ = nullptr;
}

void WallpaperTest::SetUp(void)
{
}

void WallpaperTest::TearDown(void)
{
}

class WallpaperColorChangeListenerTestImpl : public OHOS::WallpaperMgrService::WallpaperColorChangeListener {
public:
    std::vector<RgbaColor> color_;
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
    void onColorsChange(std::vector<RgbaColor> color, int wallpaperType);

    // reset the callCount_ to zero.
    void ResetToZero();

    unsigned long GetCallCount() const;

private:
    unsigned long callCount_;
};

void WallpaperColorChangeListenerTestImpl::onColorsChange(std::vector<RgbaColor> color, int wallpaperType)
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
    color_ = {};
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

/*********************   ResetWallpaper   *********************/
/**
* @tc.name: Reset001
* @tc.desc: Reset wallpaper with wallpaperType[0].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, Reset001, TestSize.Level1)
{
    HILOG_INFO("Reset001.");
    int wallpaperType = 0;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType), E_OK)
        << "should reset successfully.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}

/**
* @tc.name: Reset002
* @tc.desc: Reset wallpaper with wallpaperType[1].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, Reset002, TestSize.Level1)
{
    int wallpaperType = 1;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType), E_OK)
        << "should reset successfully.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}

/**
* @tc.name: Reset003
* @tc.desc: Reset wallpaper with wallpaperType[2].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, Reset003, TestSize.Level1)
{
    int wallpaperType = 2;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    EXPECT_NE(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType), E_OK)
        << "shouldn't reset successfully.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}

/**
* @tc.name: Reset004
* @tc.desc: Reset wallpaper with wallpaperType[0] after resetting wallpaper[0].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, Reset004, TestSize.Level1)
{
    int wallpaperType = 0;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType), E_OK)
        << "should reset successfully.";
    /* duplicate reset */
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType), E_OK)
        << "should reset successfully.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}

/**
* @tc.name: Reset005
* @tc.desc: Reset wallpaper with wallpaperType[1] after resetting wallpaper[1] and check whether Id is same one.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, Reset005, TestSize.Level1)
{
    int wallpaperType = 1;
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType), E_OK)
        << "should reset successfully.";
    int firstId = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(wallpaperType);
    /* duplicate reset */
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWallpaper(wallpaperType), E_OK)
        << "should reset successfully.";
    int secondId = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(wallpaperType);
    EXPECT_EQ(firstId, secondId) << "Id should be same one.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
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
    auto listener = std::make_shared<WallpaperColorChangeListenerTestImpl>();
    auto onStatus = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().On(listener);
    EXPECT_EQ(onStatus, true) << "subscribe wallpaper color change failed.";

    auto offSubStatus = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().Off(listener);
    EXPECT_EQ(offSubStatus, true) << "unsubscribe wallpaper color change failed.";
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
    HILOG_INFO("GetColors001 GetColors001 begin");

    std::vector<RgbaColor> Color = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(SYSTYEM);
    bool result = Color.empty();
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
    HILOG_INFO("GetColors002 GetColors001 begin");
    std::vector<RgbaColor> Color = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetColors(LOCKSCREEN);
    bool result = Color.empty();
    EXPECT_FALSE(result);
}
/*********************   GetColors   *********************/

/*********************   GetId   *********************/
/**
* @tc.name: GetId001
* @tc.desc: GetId with wallpaperType[0].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, GetId001, TestSize.Level0)
{
    HILOG_INFO("GetId001 GetId001 begin");
    bool result = false;
    int ida = HUNDRED;
    ida = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(SYSTYEM);
    if (ida != HUNDRED) {
        result = true;
    }
    EXPECT_TRUE(result);
}

/**
* @tc.name: GetId002
* @tc.desc: GetId with wallpaperType[1].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, GetId002, TestSize.Level0)
{
    HILOG_INFO("GetId002 GetId002 begin");
    bool result = false;
    int ida = HUNDRED;
    ida = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetWallpaperId(LOCKSCREEN);
    if (ida != HUNDRED) {
        result = true;
    }
    EXPECT_TRUE(result);
}
/*********************   GetId   *********************/

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
* @tc.name: GetPiexlMap001
* @tc.desc: GetPixelMap with wallpaperType[0] .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, GetPiexlMap001, TestSize.Level0)
{
    HILOG_INFO("GetPiexlMap001  begin");
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(SYSTYEM, pixelMap);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "get SYSTYEM PiexlMap success.";
    ASSERT_NE(pixelMap, nullptr) << "get LOCKSCREEN PiexlMap ptr not nullptr.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}

/**
* @tc.name: GetPiexlMap002
* @tc.desc: GetPixelMap with wallpaperType[1].
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, GetPiexlMap002, TestSize.Level0)
{
    HILOG_INFO("GetPiexlMap002  begin");
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap;
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().GetPixelMap(LOCKSCREEN, pixelMap);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "get LOCKSCREEN PiexlMap success.";
    ASSERT_NE(pixelMap, nullptr) << "get LOCKSCREEN PiexlMap ptr not nullptr.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}
/*********************   GetPiexlMap   *********************/

/*********************   SetWallpaperByMap   *********************/
/**
* @tc.name: SetWallpaperByMap001
* @tc.desc: SetWallpaperByMap with wallpaperType[0] .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, SetWallpaperByMap001, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByMap001  begin");
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "set SYSTYEM PiexlMap success.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}

/**
* @tc.name: SetWallpaperByMap002
* @tc.desc: SetWallpaperByMap with wallpaperType[1] .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, SetWallpaperByMap002, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByMap002  begin");
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, OHOS::Media::PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(pixelMap, LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "set LOCKSCREEN PiexlMap success.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}
/*********************   SetWallpaperByMap   *********************/

/*********************   SetWallpaperByUrl   *********************/
/**
* @tc.name: SetWallpaperByUrl001
* @tc.desc: SetWallpaperByUrl with wallpaperType[0] .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, SetWallpaperByUrl001, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByUrl001  begin");
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(url, SYSTYEM);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "set SYSTYEM success.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}

/**
* @tc.name: SetWallpaperByUrl002
* @tc.desc: SetWallpaperByUrl with wallpaperType[1] .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, SetWallpaperByUrl002, TestSize.Level0)
{
    HILOG_INFO("SetWallpaperByUrl002  begin");
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWpProxy(
        iface_cast<WallpaperServiceProxy>(this->remoteObjMock_));
    int32_t wallpaperErrorCode =
        OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().SetWallpaper(url, LOCKSCREEN);
    EXPECT_EQ(wallpaperErrorCode, E_OK) << "set LOCKSCREEN success.";
    OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().ResetWpProxy();
}
/*********************   SetWallpaperByUrl   *********************/
} // namespace WallpaperMgrService
} // namespace OHOS