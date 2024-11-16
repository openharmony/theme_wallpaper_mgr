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
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>

#include "file_deal.h"
#include "hilog_wrapper.h"

namespace fs = std::filesystem;
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
        HILOG_ERROR("FileDeal : openDir  is not exist, errInfo=%{public}s", strerror(errno));
        return false;
    }
    closedir(dp);
    return true;
}

bool FileDeal::Mkdir(const std::string &path)
{
    if (!IsDirExist(path)) {
        if (mkdir(path.c_str(), MODE) != 0) {
            HILOG_ERROR("FileDeal : mkdir errInfo=%{public}s", strerror(errno));
            return false;
        }
    }
    return true;
}

bool FileDeal::CopyFile(const std::string &sourceFile, const std::string &newFile)
{
    std::filesystem::path dstPath(newFile);
    std::filesystem::path srcPath(sourceFile);
    std::error_code errCode;
    if (!std::filesystem::copy_file(srcPath, dstPath, std::filesystem::copy_options::overwrite_existing, errCode)) {
        HILOG_ERROR("Failed to copy file, error code: %{public}d", errCode.value());
        return false;
    }
    if (!ForcedRefreshDisk(newFile)) {
        HILOG_WARN("ForcedRefreshDisk failed!");
    }
    return true;
}

bool FileDeal::DeleteFile(const std::string &sourceFile)
{
    if (!IsFileExist(sourceFile)) {
        return true;
    }
    if (remove(sourceFile.c_str()) < 0) {
        HILOG_ERROR("Failed to remove source file, errInfo=%{public}s.", strerror(errno));
        return false;
    }
    return true;
}

bool FileDeal::DeleteDir(const std::string &path, bool deleteRootDir)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return false;
    }
    dirent *dirent;
    while ((dirent = readdir(dir)) != nullptr) {
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
            continue;
        }
        std::string fullPath = path;
        if (path[path.size() - 1] != '/') {
            fullPath += '/';
        }
        fullPath += dirent->d_name;
        if (dirent->d_type == DT_DIR) {
            if (!DeleteDir(fullPath)) {
                closedir(dir);
                return false;
            }
        } else if (remove(fullPath.c_str()) < 0) {
            HILOG_ERROR("remove failed, fullPath=%{public}s, errInfo=%{public}s", fullPath.c_str(), strerror(errno));
            closedir(dir);
            return false;
        }
    }
    closedir(dir);
    if (deleteRootDir && rmdir(path.c_str()) != 0) {
        HILOG_ERROR("remove failed, path=%{public}s, errInfo=%{public}s", path.c_str(), strerror(errno));
        return false;
    }
    return true;
}

bool FileDeal::IsFileExist(const std::string &name)
{
    if (access(name.c_str(), F_OK) != 0) {
        HILOG_DEBUG("FileDeal : access errInfo=%{public}s", strerror(errno));
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
bool FileDeal::GetRealPath(const std::string &inOriPath, std::string &outRealPath)
{
    char realPath[PATH_MAX + 1] = { 0x00 };
    if (inOriPath.size() > PATH_MAX || realpath(inOriPath.c_str(), realPath) == nullptr) {
        HILOG_ERROR("get real path fail!");
        return false;
    }
    outRealPath = std::string(realPath);
    if (!IsFileExist(outRealPath)) {
        HILOG_ERROR("real path file is not exist! %{public}s", outRealPath.c_str());
        return false;
    }
    if (outRealPath != inOriPath) {
        HILOG_ERROR("illegal file path input %{public}s", inOriPath.c_str());
        return false;
    }
    return true;
}
bool FileDeal::IsZipFile(const std::string &filePath)
{
    fs::path file(filePath);
    if (fs::exists(file) && fs::is_regular_file(file)) {
        std::string extension = file.extension().string();
        if (extension == ".zip") {
            return true;
        }
    }
    HILOG_ERROR("this is not a zip.filePath:%{private}s", filePath.c_str());
    return false;
}
bool FileDeal::ForcedRefreshDisk(const std::string &sourcePath)
{
    std::string fileRealPath;
    if (!GetRealPath(sourcePath, fileRealPath)) {
        HILOG_ERROR("get real path file failed!");
        return false;
    }
    FILE *file = std::fopen(fileRealPath.c_str(), "rb");
    if (file == nullptr) {
        HILOG_ERROR("Fopen failed, %{public}s, %{public}s", fileRealPath.c_str(), strerror(errno));
        return false;
    }
    if (fflush(file) != 0) {
        HILOG_ERROR("fflush file failed, errno=%{public}d", errno);
        fclose(file);
        return false;
    }
    if (fsync(fileno(file)) != 0) {
        HILOG_ERROR("fsync file failed, errno=%{public}d", errno);
        fclose(file);
        return false;
    }
    (void)fclose(file);
    return true;
}
bool FileDeal::IsFileExistInDir(const std::string &path)
{
    DIR *dp = opendir(path.c_str());
    if (dp == nullptr) {
        HILOG_ERROR("FileDeal : openDir  is error, errInfo=%{public}s", strerror(errno));
        return false;
    }
    dirent *dir = readdir(dp);
    while (dir != nullptr) {
        if (dir->d_type == DT_DIR) {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                closedir(dp);
                return true;
            }
        } else {
            closedir(dp);
            return true;
        }
        dir = readdir(dp);
    }
    closedir(dp);
    HILOG_INFO("Dir is empty");
    return false;
}
} // namespace WallpaperMgrService
} // namespace OHOS
