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
#include "hisysevent.h"
#include "fault_reporter.h"

namespace OHOS {
using namespace HiviewDFX;
namespace MiscServices {
constexpr const char *SERVICE_FAULT = "SERVICE_FAULT";
constexpr const char *RUNTIME_FAULT = "RUNTIME_FAULT";
constexpr const char *FAULT_TYPE = "FAULT_TYPE";
constexpr const char *MODULE_NAME = "MODULE_NAME";
constexpr const char *ERROR_TYPE = "ERROR_TYPE";

ReportStatus FaultReporter::ReportServiceFault(const FaultMsg &msg)
{
    int nRet = HiSysEvent::Write(HiSysEvent::Domain::THEME, SERVICE_FAULT, HiSysEvent::EventType::FAULT,
        FAULT_TYPE, static_cast<int>(msg.faultType), MODULE_NAME, msg.moduleName, ERROR_TYPE,
        static_cast<int>(msg.errorCode));
    return nRet == 0 ? ReportStatus::SUCCESS : ReportStatus::ERROR;
}

ReportStatus FaultReporter::ReportRuntimeFault(const FaultMsg &msg)
{
    int nRet = HiSysEvent::Write(HiSysEvent::Domain::THEME, RUNTIME_FAULT, HiSysEvent::EventType::FAULT,
        FAULT_TYPE, static_cast<int>(msg.faultType), ERROR_TYPE, static_cast<int>(msg.errorCode));
    return nRet == 0 ? ReportStatus::SUCCESS : ReportStatus::ERROR;
}
} // namespace MiscServices
} // namespace OHOS
