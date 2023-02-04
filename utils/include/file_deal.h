/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef WALLPAPER_SERVICES_FILE_DEAL_H
#define WALLPAPER_SERVICES_FILE_DEAL_H

#include <fstream>
#include <iostream>
#include <string>
namespace OHOS {
namespace WallpaperMgrService {
class FileDeal {
public:
    FileDeal();
    ~FileDeal();
    static bool IsDirExist(std::string path);
    static bool Mkdir(const std::string &path);
    static bool CopyFile(const std::string &sourceFile, const std::string &newFile);
    static bool DeleteFile(const std::string &sourceFile);
    static bool FileIsExist(const std::string &name);
};
} // namespace WallpaperMgrService
} // namespace OHOS

#endif
