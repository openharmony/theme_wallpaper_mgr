/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#include "command.h"
namespace OHOS {
namespace MiscServices {
Command::Command(const std::vector<std::string> &argsFormat, const std::string &help, const Command::Action &action)
    : format_(argsFormat), help_(help), action_(action)
{
}

std::string Command::ShowHelp() const
{
    return help_;
}

bool Command::DoAction(const std::vector<std::string> &input, std::string &output)
{
    return action_(input, output);
}

std::string Command::GetOption()
{
    return format_.at(0);
}

std::string Command::GetFormat()
{
    std::string formatStr;
    for (const auto &seg : format_) {
        formatStr += seg;
        formatStr += " ";
    }
    return formatStr;
}
} // namespace MiscServices
} // namespace OHOS
