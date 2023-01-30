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
#include "statistic_reporter.h"
#include <unistd.h>
#include <ctime>
#include <thread>
#include <vector>
#include "hilog_wrapper.h"
#include "hisysevent.h"
#include "memory_guard.h"

using namespace OHOS::WallpaperMgrService;
namespace OHOS {
using namespace HiviewDFX;
namespace MiscServices {
constexpr const char *USER_ID = "USER_ID";
constexpr const char *PACKAGES_NAME = "PACKAGES_NAME";
constexpr const char *CUMULATIVE_TIME = "CUMULATIVE_TIME";
constexpr const char *USAGETIME_STATISTIC = "USAGETIME_STATISTIC";
constexpr const char *WALLPAPER_INFO = "WALLPAPER_INFO";

bool StatisticReporter::running_ = false;
std::mutex StatisticReporter::runMutex_;
std::mutex StatisticReporter::usageTimeMutex_;
std::map<int, std::vector<UsageTimeStat>> StatisticReporter::usageTimeStat_;

void StatisticReporter::ReportUsageTimeStatistic(int userId, const UsageTimeStat &stat)
{
    std::lock_guard<std::mutex> lock(usageTimeMutex_);
    std::map<int, std::vector<UsageTimeStat>>::iterator iter = usageTimeStat_.find(userId);
    if (iter != usageTimeStat_.end()) {
        UsageTimeStat &timeStat = iter->second.back();
        int nDiff = stat.startTime - timeStat.startTime;
        timeStat.cumulativeTime = std::to_string(nDiff / ONE_HOUR_IN_SECONDS);
        iter->second.push_back(stat);
    } else {
        std::vector<UsageTimeStat> vecUsageTime;
        vecUsageTime.push_back(stat);
        usageTimeStat_[userId] = vecUsageTime;
    }
}

void StatisticReporter::StartTimerThread()
{
    {
        std::lock_guard<std::mutex> lock(runMutex_);
        if (running_) {
            return;
        }
        running_ = true;
    }
    auto fun = []() {
        MemoryGuard cacheGuard;
        while (true) {
            time_t current = time(nullptr);
            if (current == -1) {
                sleep(ONE_HOUR_IN_SECONDS);
                continue;
            }
            
            tm localTime = { 0 };
            tm *result = localtime_r(&current, &localTime);
            if (result == nullptr) {
                sleep(ONE_HOUR_IN_SECONDS);
                continue;
            }
            int currentHour = localTime.tm_hour;
            int currentMin = localTime.tm_min;
            if ((EXEC_MIN_TIME - currentMin) != EXEC_MIN_TIME) {
                int nHours = EXEC_HOUR_TIME - currentHour;
                int nMin = EXEC_MIN_TIME - currentMin;
                int nTime = (nMin)*ONE_MINUTE_IN_SECONDS + (nHours)*ONE_HOUR_IN_SECONDS;
                HILOG_INFO(
                    " StartTimerThread if nHours=%{public}d,nMin=%{public}d,nTime=%{public}d", nHours, nMin, nTime);
                sleep(nTime);
                current = time(nullptr);
                if (current == -1) {
                    sleep(ONE_HOUR_IN_SECONDS);
                    continue;
                }
                InvokeUsageTime(current);
            } else {
                sleep(ONE_HOUR_IN_SECONDS * (ONE_DAY_IN_HOURS - currentHour));
                current = time(nullptr);
                if (current == -1) {
                    sleep(ONE_HOUR_IN_SECONDS);
                    continue;
                }
                InvokeUsageTime(current);
            }
        }
    };
    std::thread th = std::thread(fun);
    th.detach();
}

ReportStatus StatisticReporter::InvokeUsageTime(time_t curTime)
{
    HILOG_INFO(" InvokeUsageTime start.");
    std::string statisicMsg;
    std::lock_guard<std::mutex> lock(usageTimeMutex_);
    for (auto const &uasgeTime : usageTimeStat_) {
        statisicMsg.append(USER_ID).append(":" + std::to_string(uasgeTime.first) + ",");
        std::vector<UsageTimeStat> vecUasgeTime = uasgeTime.second;
        for (auto const &stat : vecUasgeTime) {
            std::string cumulativeTime(stat.cumulativeTime);
            if (cumulativeTime.empty()) {
                int nDiff = curTime - stat.startTime;
                cumulativeTime = std::to_string(nDiff / ONE_HOUR_IN_SECONDS);
            }

            statisicMsg.append(PACKAGES_NAME)
                .append(":" + stat.packagesName + ",")
                .append(CUMULATIVE_TIME)
                .append(":" + cumulativeTime + "hours,");
        }
    }
    if (statisicMsg.empty()) {
        return ReportStatus::ERROR;
    }

    HiSysEvent::Write(HiSysEvent::Domain::THEME, USAGETIME_STATISTIC, HiSysEvent::EventType::STATISTIC,
        WALLPAPER_INFO, statisicMsg);
    HILOG_INFO(" InvokeUsageTime end.");
    return ReportStatus::SUCCESS;
}
} // namespace MiscServices
} // namespace OHOS
