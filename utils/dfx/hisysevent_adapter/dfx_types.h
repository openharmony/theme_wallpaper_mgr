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

#ifndef MISCSERVICES_DFX_TYPES_H
#define MISCSERVICES_DFX_TYPES_H

#include <string>

namespace OHOS {
namespace MiscServices {
enum class FaultCode {
    // Service Fault
    SF_SERVICE_UNAVAIABLE = 0,
    SF_SERVICE_SUBSCRIBECOMMINEVENT,
    SF_STARTABILITY_FAILED,

    // Runtime Fault
    RF_DROP_FAILED = 10,
    RF_FD_INPUT_FAILED,
};

enum class FaultType {
    SERVICE_FAULT = 0,
    SET_WALLPAPER_FAULT,
    LOAD_WALLPAPER_FAULT,
};

struct FaultMsg {
    FaultType faultType;
    std::string moduleName;
    FaultCode errorCode;
};

struct UsageTimeStat {
    std::string packagesName;
    time_t startTime;
    std::string cumulativeTime;
};

enum class ReportStatus {
    SUCCESS = 0,
    ERROR = 1,
};

enum class TraceTaskId : int32_t {
    SET_WALLPAPER = 0,
    SET_WALLPAPER_BY_FD,
    SET_WALLPAPER_BY_MAP,
    ONSTART_EXTENSION,
    ONSTART_MIDDLE_EXTENSION,
    ONCREATE_EXTENSION,
    ONCONNECT_EXTENSION,
    ONCONNECT_MIDDLE_EXTENSION,
};
} // namespace MiscServices
} // namespace OHOS
#endif // MISCSERVICES_DFX_TYPES_H
