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
#include <dirent.h>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include "hilog_wrapper.h"
#include "file_deal.h"

namespace OHOS {
namespace WallpaperMgrService {
FileDeal::FileDeal(void)
{
}
FileDeal::~FileDeal()
{
}
bool FileDeal::DirIsExist(std::string path)
{
    DIR* dp;
    if ((dp = opendir(path.c_str())) == NULL) {
        HILOG_INFO("FileDeal : opendir  %{public}s is not exist", path.c_str());
        return false;
    }
    closedir(dp);
    return true;
}

bool FileDeal::Mkdir(std::string path)
{
    if (!DirIsExist(path)) {
        int isCreate = ::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
        HILOG_INFO("FileDeal : mkdir result= %{public}d,errinfo=%{public}s ,path =  %{public}s ",
            isCreate, strerror(errno), path.c_str());
        return isCreate == 0 ? true : false;
    }
    return true;
}

bool FileDeal::CopyFile(const std::string &sourceFile, const std::string &newFile)
{
    HILOG_INFO("Copy file Star:from [%{public}s] to [%{public}s]", sourceFile.c_str(), newFile.c_str());
    std::ifstream in;
    std::ofstream out;

    in.open(sourceFile.c_str(), std::ios::binary);
    if (in.fail()) {
        HILOG_INFO("open file  %{public}s failed", sourceFile.c_str());
        in.close();
        out.close();
        return false;
    }
    out.open(newFile.c_str(), std::ios::binary);
    if (out.fail()) {
        HILOG_INFO("open file  %{public}s failed", newFile.c_str());
        out.close();
        in.close();
        return false;
    }
    out << in.rdbuf();
    out.close();
    in.close();
    HILOG_INFO("copy file is %{public}s, new file is %{public}s,success", sourceFile.c_str(), newFile.c_str());
    return true;
}

bool FileDeal::FileIsExist(const std::string& name)
{
    if (access(name.c_str(), F_OK) == 0) {
        return true;
    }
    return false;
}
}
}

