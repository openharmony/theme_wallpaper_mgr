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

namespace OHOS {
namespace MiscServices {
namespace {
constexpr int32_t SUB_CMD_NAME = 0;
constexpr int32_t CMD_NO_PARAM = 1;
constexpr const char *CMD_HELP = "-h";
constexpr const char *CMD_ALL = "-all";
constexpr const char *ILLEGAL_INFOMATION = "The arguments are illegal and you can enter '-h' for help.\n";
} // namespace

void DumpHelper::AddDumpOperation(const DumpNoParamFunc &dumpAll)
{
    if (dumpAll == nullptr) {
        return;
    }
    dumpAll_ = dumpAll;
}

bool DumpHelper::Dump(int fd, const std::vector<std::string> &args)
{
    std::lock_guard<std::mutex> lock(hidumperMutex_);
    std::string command = "";
    std::string param = "";

    if (args.size() == CMD_NO_PARAM) {
        command = args.at(SUB_CMD_NAME);
    } else {
        ShowIllealInfomation(fd);
    }

    if (command == CMD_HELP) {
        ShowHelp(fd);
    } else if (command == CMD_ALL) {
        if (!dumpAll_) {
            return false;
        }
        dumpAll_(fd);
    } else {
        ShowIllealInfomation(fd);
    }
    return true;
}

void DumpHelper::ShowHelp(int fd)
{
    std::string result;
    result.append("Usage:hidumper  wallpaper_fwk\n")
        .append("Description:\n")
        .append("-h")
        .append("                        ")
        .append("--help show help\n")
        .append("-all")
        .append("                      ")
        .append("dump wallpaper information in the system\n")
        .append("  width:                  ")
        .append("dump information about the wallpaper width in the system\n")
        .append("  height:                 ")
        .append("dump information about the wallpaper height in the system\n")
        .append("  WallpaperExtension:     ")
        .append("dump information about the ExtensionInfo  in the system\n");
    dprintf(fd, "%s\n", result.c_str());
}

void DumpHelper::ShowIllealInfomation(int fd)
{
    dprintf(fd, "%s\n", ILLEGAL_INFOMATION);
}
} // namespace MiscServices
} // namespace OHOS
