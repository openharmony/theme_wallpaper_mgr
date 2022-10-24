/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * miscservices under the License is miscservices on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MISCSERVICES_STATISTIC_REPORTER_H
#define MISCSERVICES_STATISTIC_REPORTER_H
#include <map>
#include <mutex>
#include "dfx_types.h"
namespace OHOS {
namespace MiscServices {
// 统计报告
class StatisticReporter {
public:
    static void ReportUsageTimeStatistic(int userId, const UsageTimeStat &stat);
    static void StartTimerThread();
    static ReportStatus InvokeUsageTime(time_t curTime);

private:
    static std::mutex usageTimeMutex_;
    static std::map<int, std::vector<UsageTimeStat>> usageTimeStat_;

    static std::mutex runMutex_;
    static bool running_;
    static const inline int ONE_DAY_IN_HOURS = 24;
    static const inline int ONE_MINUTE_IN_SECONDS = 60;
    static const inline int ONE_HOUR_IN_SECONDS = 1 * 60 * 60;          // 1 hours

    static const inline int EXEC_HOUR_TIME = 23;
    static const inline int EXEC_MIN_TIME = 60;
};
} // namespace MiscServices
} // namespace OHOS
#endif // MISCSERVICES_STATISTIC_REPORTER_H
