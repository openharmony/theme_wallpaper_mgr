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

bool FileDeal::CopyFile(char* SourceFile, char* NewFile)
{
    HILOG_INFO("wallpaperservice Copy file Star:from [%{public}s] to [%{public}s]", SourceFile, NewFile);
    bool bFlag = false;
    std::ifstream in;
    std::ofstream out;

        in.open(SourceFile, std::ios::binary);
        if (in.fail()) {
            HILOG_INFO("FileDeal : open file  %{public}s failed", SourceFile);
            in.close();
            out.close();
            return bFlag;
        }
        out.open(NewFile, std::ios::binary);
        if (out.fail()) {
            HILOG_INFO("FileDeal : open file  %{public}s failed", NewFile);
            out.close();
            in.close();
            return bFlag;
        } else {
            out << in.rdbuf();
            out.close();
            in.close();
            bFlag = true;
            HILOG_INFO("FileDeal : copy file  %{public}s, new file=%{public}s,success", SourceFile, NewFile);
            return bFlag;
        }
    return bFlag;
}

bool FileDeal::FileIsExist(const std::string& name)
{
    bool bFlag = false;
    if (access(name.c_str(), F_OK) == 0) {
        bFlag = true;
        return bFlag;
    }
    return false;
}
}
}

