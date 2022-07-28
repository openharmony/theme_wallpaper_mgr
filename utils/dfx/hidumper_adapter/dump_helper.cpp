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
#include "dump_helper.h"
#include <cstdio>
#include <utility>
#include "hilog_wrapper.h"
using namespace OHOS::WallpaperMgrService;
namespace OHOS {
namespace MiscServices {
DumpHelper &DumpHelper::GetInstance()
{
    static DumpHelper instance;
    return instance;
}

void DumpHelper::RegisterCommand(std::shared_ptr<Command> &cmd)
{
    cmdHandler_.insert(std::make_pair(cmd->GetOption(), cmd));
}

bool DumpHelper::Dispatch(int fd, const std::vector<std::string> &args)
{
    if (args.empty() || args.at(0) == "-h") {
        dprintf(fd, "\n%-15s: %-20s", "Option", "Description");
        for (auto &[key, handler] : cmdHandler_) {
            dprintf(fd, "\n%-15s: %-20s", handler->GetFormat().c_str(), handler->ShowHelp().c_str());
        }
        return false;
    }
    auto handler = cmdHandler_.find(args.at(0));
    if (handler != cmdHandler_.end()) {
        std::string output;
        bool ret = handler->second->DoAction(args, output);
        if (!ret) {
            HILOG_INFO("DoAction faild");
        }
        dprintf(fd, "\n%s", output.c_str());
        return ret;
    }
    return false;
}
} // namespace MiscServices
} // namespace OHOS
