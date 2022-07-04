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

#include <gtest/gtest.h>
#include <ctime>
#include "wallpaper_manager_kits.h"
#include "wallpaper_manager.h"
#include "hilog_wrapper.h"
#include "directory_ex.h"
#include "dfx_types.h"
#include "reporter.h"

constexpr int SYSTYEM = 0;
constexpr int LOCKSCREEN = 1;
constexpr int HUNDRED = 100;
using namespace testing::ext;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;
using namespace OHOS::MiscServices;

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
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
}

void WallpaperTest::TearDownTestCase(void)
{
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
    {}

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
    int wallpaperType = 0;
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        ResetWallpaper(wallpaperType), true) << "should reset successfully.";
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
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        ResetWallpaper(wallpaperType), true) << "should reset successfully.";
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
    EXPECT_NE(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        ResetWallpaper(wallpaperType), true) << "shouldn't reset successfully.";
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
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        ResetWallpaper(wallpaperType), true) << "should reset successfully.";
    /* duplicate reset */
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        ResetWallpaper(wallpaperType), true) << "should reset successfully.";
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
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        ResetWallpaper(wallpaperType), true) << "should reset successfully.";
    int firstId = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetWallpaperId(wallpaperType);
    /* duplicate reset */
    EXPECT_EQ(OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        ResetWallpaper(wallpaperType), true) << "should reset successfully.";
    int secondId = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetWallpaperId(wallpaperType);
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
    auto onStatus = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        On(listener);
    EXPECT_EQ(onStatus, true) << "subscribe wallpaper color change failed.";

    auto offSubStatus = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        Off(listener);
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

    std::vector<RgbaColor> Color = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetColors(SYSTYEM);
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
    std::vector<RgbaColor> Color = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetColors(LOCKSCREEN);
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
    ida = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetWallpaperId(SYSTYEM);
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
    ida = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetWallpaperId(LOCKSCREEN);
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
    hight = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetWallpaperMinHeight();
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
    width = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetWallpaperMinWidth();
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
    HILOG_INFO("SetWallpaper&GetPiexlMap001  begin");
    auto PixelMap = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetPixelMap(SYSTYEM);
    EXPECT_TRUE(PixelMap);
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
    HILOG_INFO("SetWallpaper&GetPiexlMap002  begin");
    auto PixelMap = OHOS::WallpaperMgrService::WallpaperManagerkits::GetInstance().
        GetPixelMap(LOCKSCREEN);
    EXPECT_TRUE(PixelMap);
}
/*********************   GetPiexlMap   *********************/

/*********************   DFX   *********************/
/**
* @tc.name: DFX001
* @tc.desc: check ReportServiceFault
* @tc.type: send data
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, DFX001, TestSize.Level0)
{
    HILOG_INFO("DFX001 ReportServiceFault begin");
    FaultMsg msg;
    msg.faultType = FaultType::SERVICE_FAULT;
    msg.errorCode = FaultCode::SF_SERVICE_UNAVAIABLE;
    msg.moduleName = "com.ohos.wallpaper";
    ReportStatus repStatus = Reporter::GetInstance().Fault().ReportServiceFault(msg);
    EXPECT_TRUE(repStatus == ReportStatus::SUCCESS);
}

/**
* @tc.name: DFX002
* @tc.desc: check ReportRuntimeFault
* @tc.type: send data
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, DFX002, TestSize.Level0)
{
    HILOG_INFO("DFX002 ReportRuntimeFault begin");
    FaultMsg msg;
    msg.faultType = FaultType::SET_WALLPAPER_FAULT;
    msg.errorCode = FaultCode::RF_DROP_FAILED;
    ReportStatus repStatus = Reporter::GetInstance().Fault().ReportRuntimeFault(msg);
    EXPECT_TRUE(repStatus == ReportStatus::SUCCESS);
}

/**
* @tc.name: DFX002
* @tc.desc: check ReportUsageTimeStatistic
* @tc.type: send data
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperTest, DFX003, TestSize.Level0)
{
    HILOG_INFO("DFX002 ReportUsageTimeStatistic begin");
    
    int userId = static_cast<int>(IPCSkeleton::GetCallingUid());
    UsageTimeStat timeStat;
    timeStat.packagesName = "unittest";
    timeStat.startTime = time(nullptr);
    Reporter::GetInstance().UsageTimeStatistic().ReportUsageTimeStatistic(userId, timeStat);
    ReportStatus repStatus = Reporter::GetInstance().UsageTimeStatistic().InvokeUsageTime(time(nullptr));
    EXPECT_TRUE(repStatus == ReportStatus::SUCCESS);
}
/*********************   DFX   *********************/
} // wallpaperservice
} // OHOS