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
#include <cstdint>
#include <ctime>
#include <gtest/gtest.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#include "dfx_types.h"
#include "hilog_wrapper.h"
#include "securec.h"
#include "statistic_reporter.h"

using namespace testing::ext;
using namespace OHOS::MiscServices;
namespace OHOS {
namespace WallpaperMgrService {
constexpr const uint16_t EACH_LINE_LENGTH = 100;
constexpr const uint16_t TOTAL_LENGTH = 1000;
constexpr const char *CMD1 = "hidumper -s 3705";
constexpr const char *CMD2 = "hidumper -s 3705 -a -h";
constexpr const char *CMD3 = "hidumper -s 3705 -a -all";
constexpr const int USER_ID = 1000;
constexpr const char *BUNDLE_NAME = "WALLPAPER_SERVICE";
class WallpaperDfxTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    static bool ExecuteCmd(const std::string &cmd, std::string &result);
    void SetUp();
    void TearDown();
};

void WallpaperDfxTest::SetUpTestCase(void)
{
    HILOG_INFO("WallpaperDfxTest::SetUpTestCase");
}

void WallpaperDfxTest::TearDownTestCase(void)
{
    HILOG_INFO("WallpaperDfxTest::TearDownTestCase");
}

void WallpaperDfxTest::SetUp(void)
{
    HILOG_INFO("WallpaperDfxTest::SetUp");
}

void WallpaperDfxTest::TearDown(void)
{
    HILOG_INFO("WallpaperDfxTest::TearDown");
}

bool WallpaperDfxTest::ExecuteCmd(const std::string &cmd, std::string &result)
{
    char buff[EACH_LINE_LENGTH] = { 0x00 };
    char output[TOTAL_LENGTH] = { 0x00 };
    FILE *ptr = popen(cmd.c_str(), "r");
    if (ptr != nullptr) {
        while (fgets(buff, sizeof(buff), ptr) != nullptr) {
            if (strcat_s(output, sizeof(output), buff) != 0) {
                pclose(ptr);
                ptr = nullptr;
                return false;
            }
        }
        pclose(ptr);
        ptr = nullptr;
    } else {
        return false;
    }
    result = std::string(output);
    return true;
}

/**
* @tc.name: WallpaperDfxTest_DumpMethod_001
* @tc.desc: DumpAllMethod
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperDfxTest, WallpaperDfxTest_DumpMethod_001, TestSize.Level0)
{
    std::string result;
    auto ret = WallpaperDfxTest::ExecuteCmd(CMD1, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("Description"), std::string::npos);
    EXPECT_NE(result.find("Show all"), std::string::npos);
}

/**
* @tc.name: WallpaperDfxTest_Dump_ShowHelp_001
* @tc.desc: Dump ShowHelp.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperDfxTest, WallpaperDfxTest_Dump_ShowHelp_001, TestSize.Level0)
{
    std::string result;
    auto ret = WallpaperDfxTest::ExecuteCmd(CMD2, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("Description"), std::string::npos);
    EXPECT_NE(result.find("Show all"), std::string::npos);
}

/**
* @tc.name: WallpaperDfxTest_DumpAllMethod_001
* @tc.desc: Dump ShowIllealInfomation.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperDfxTest, WallpaperDfxTest_DumpAllMethod_001, TestSize.Level0)
{
    std::string result;
    auto ret = WallpaperDfxTest::ExecuteCmd(CMD3, result);
    EXPECT_TRUE(ret);
    EXPECT_NE(result.find("height"), std::string::npos);
    EXPECT_NE(result.find("width"), std::string::npos);
    EXPECT_NE(result.find("WallpaperExtension"), std::string::npos);
}

/**
* @tc.name: WallpaperDfxTest_StatisticReporter_001
* @tc.desc: Statistic Reporter .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(WallpaperDfxTest, WallpaperDfxTest_StatisticReporter_001, TestSize.Level0)
{
    StatisticReporter::StartTimerThread();
    UsageTimeStat timeStat;
    timeStat.packagesName = BUNDLE_NAME;
    timeStat.startTime = time(nullptr);
    StatisticReporter::ReportUsageTimeStatistic(USER_ID, timeStat);
    time_t current = time(nullptr);
    auto status = StatisticReporter::InvokeUsageTime(current);
    EXPECT_EQ(status, ReportStatus::SUCCESS);
}
} // namespace WallpaperMgrService
} // namespace OHOS