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
#ifndef CMDRESOLVER_COMMAND_H
#define CMDRESOLVER_COMMAND_H
#include <functional>
#include <string>
#include <vector>
namespace OHOS {
namespace MiscServices {
class Command {
public:
    using Action = std::function<bool(const std::vector<std::string> &input, std::string &output)>;
    Command(const std::vector<std::string> &argsFormat, const std::string &help, const Action &action);
    std::string ShowHelp() const;
    bool DoAction(const std::vector<std::string> &input, std::string &output);
    std::string GetOption();
    std::string GetFormat();

private:
    std::vector<std::string> format_;
    std::string help_;
    Action action_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // CMDRESOLVER_COMMAND_H