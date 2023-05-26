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
#include "file_deal.h"

#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hilog_wrapper.h"

namespace OHOS {
namespace WallpaperMgrService {
constexpr mode_t MODE = 0740;
FileDeal::FileDeal(void)
{
}
FileDeal::~FileDeal()
{
}
bool FileDeal::IsDirExist(std::string path)
{
    DIR *dp;
    if ((dp = opendir(path.c_str())) == NULL) {
        HILOG_INFO("FileDeal : openDir  is not exist, errInfo=%{public}s", strerror(errno));
        return false;
    }
    closedir(dp);
    return true;
}

bool FileDeal::Mkdir(const std::string &path)
{
    if (!IsDirExist(path)) {
        if (mkdir(path.c_str(), MODE) != 0) {
            HILOG_INFO("FileDeal : mkdir errInfo=%{public}s", strerror(errno));
            return false;
        }
    }
    return true;
}

bool FileDeal::CopyFile(const std::string &sourceFile, const std::string &newFile)
{
    std::ifstream in;
    std::ofstream out;

    in.open(sourceFile.c_str(), std::ios::binary);
    if (in.fail()) {
        HILOG_INFO("open file failed, errInfo=%{public}s", strerror(errno));
        in.close();
        return false;
    }
    out.open(newFile.c_str(), std::ios::binary);
    if (out.fail()) {
        HILOG_INFO("open file failed, errInfo=%{public}s", strerror(errno));
        out.close();
        in.close();
        return false;
    }
    out << in.rdbuf();
    out.close();
    in.close();
    HILOG_INFO("copy file success");
    return true;
}

bool FileDeal::DeleteFile(const std::string &sourceFile)
{
    if (remove(sourceFile.c_str()) < 0) {
        HILOG_INFO("Failed to remove source file, errInfo=%{public}s.", strerror(errno));
        return false;
    }
    return true;
}

bool FileDeal::IsFileExist(const std::string &name)
{
    if (access(name.c_str(), F_OK) != 0) {
        HILOG_INFO("FileDeal : access errInfo=%{public}s", strerror(errno));
        return false;
    }
    return true;
}

std::string FileDeal::GetExtension(const std::string &filePath)
{
    std::string filename = filePath;
    std::string extension = "";
    std::string::size_type pos = filePath.find_last_of('/');
    if (pos != std::string::npos) {
        if (pos + 1 < filePath.length()) {
            filename = filePath.substr(pos + 1);
        }
    }

    pos = filename.find_last_of('.');
    if (pos != std::string::npos && pos + 1 < filename.length()) {
        extension = filename.substr(pos);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    }
    return extension;
}
} // namespace WallpaperMgrService
} // namespace OHOS
