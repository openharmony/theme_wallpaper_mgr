/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <ctime>

#include "hilog_wrapper.h"
#include "wallpaper_cjson_mock.h"
#include "wallpaper_service.h"

namespace OHOS {
namespace WallpaperMgrService {

constexpr const char *WALLPAPER_MOCK_CJSON_NAME = "wallpaper_mock_cjson_name";
constexpr const char *WALLPAPER = "wallpaper";

using namespace testing::ext;
using namespace testing;
using namespace OHOS::MiscServices;
using namespace OHOS::Security::AccessToken;

class WallpaperTestMock : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<WallpaperMgrCjsonMock> wallpaperMock = nullptr;
};
const std::string VALID_SCHEMA_STRICT_DEFINE = "{\"SCHEMA_VERSION\":\"1.0\","
                                               "\"SCHEMA_MODE\":\"STRICT\","
                                               "\"SCHEMA_SKIPSIZE\":0,"
                                               "\"SCHEMA_DEFINE\":{"
                                               "\"age\":\"INTEGER, NOT NULL\""
                                               "},"
                                               "\"SCHEMA_INDEXES\":[\"$.age\"]}";

void WallpaperTestMock::SetUpTestCase(void)
{
    HILOG_INFO("SetUpTestCase");
    HILOG_INFO("SetUpTestCase end");
}

void WallpaperTestMock::TearDownTestCase(void)
{
    HILOG_INFO("TearDownTestCase");
    HILOG_INFO("TearDownTestCase end");
}

void WallpaperTestMock::SetUp()
{
    wallpaperMock = std::make_shared<WallpaperMgrCjsonMock>();
    OHOS::WallpaperMgrCjson::wallpaperMgrCjson = wallpaperMock;
}

void WallpaperTestMock::TearDown()
{
    wallpaperMock = nullptr;
}

/*********************   Wallpaper_SaveWallpaperState_mock    *********************/
/**
* @tc.name: SaveWallpaperState
* @tc.desc: save wallpaper state err test, cJSON_AddNumberToObject return nullptr.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(WallpaperTestMock, SaveWallpaperState001, TestSize.Level0)
{
    HILOG_INFO("SaveWallpaperState001 begin");
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    wallpaperService->LoadWallpaperState();
    int32_t userId = wallpaperService->QueryActiveUserId();
    cJSON *image = cJSON_CreateObject();
    cJSON_AddStringToObject(image, "wallpaper", WALLPAPER_MOCK_CJSON_NAME);
    EXPECT_CALL(*wallpaperMock, cJSON_AddNumberToObject(_, _, _)).WillOnce(Return(nullptr));
    auto ret =
        wallpaperService->SaveWallpaperState(userId, WallpaperType::WALLPAPER_SYSTEM, WallpaperResourceType::DEFAULT);
    if (image != nullptr) {
        cJSON_Delete(image);
    }
    EXPECT_EQ(ret, false) << "Failed to SaveWallpaperState";
}

/**
* @tc.name: SaveWallpaperState
* @tc.desc: save wallpaper state err test, cJSON_Print return nullptr.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(WallpaperTestMock, SaveWallpaperState002, TestSize.Level0)
{
    HILOG_INFO("SaveWallpaperState002 begin");
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    wallpaperService->LoadWallpaperState();
    int32_t userId = wallpaperService->QueryActiveUserId();
    cJSON *image = cJSON_CreateObject();
    cJSON_AddStringToObject(image, WALLPAPER, WALLPAPER_MOCK_CJSON_NAME);
    EXPECT_CALL(*wallpaperMock, cJSON_AddNumberToObject(_, _, _)).WillRepeatedly(Return(image));
    EXPECT_CALL(*wallpaperMock, cJSON_Print(_)).WillOnce(Return(nullptr));
    auto ret =
        wallpaperService->SaveWallpaperState(userId, WallpaperType::WALLPAPER_SYSTEM, WallpaperResourceType::DEFAULT);
    if (image != nullptr) {
        cJSON_Delete(image);
    }
    EXPECT_EQ(ret, false) << "Failed to SaveWallpaperState";
}

/**
* @tc.name: SaveWallpaperState
* @tc.desc: save wallpaper state err test, cJSON_Print return empty.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(WallpaperTestMock, SaveWallpaperState003, TestSize.Level0)
{
    HILOG_INFO("SaveWallpaperState003 begin");
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    wallpaperService->LoadWallpaperState();
    int32_t userId = wallpaperService->QueryActiveUserId();
    cJSON *image = cJSON_CreateObject();
    cJSON_AddStringToObject(image, WALLPAPER, WALLPAPER_MOCK_CJSON_NAME);
    EXPECT_CALL(*wallpaperMock, cJSON_AddNumberToObject(_, _, _)).WillRepeatedly(Return(image));
    EXPECT_CALL(*wallpaperMock, cJSON_Print(_)).WillOnce(Return(const_cast<char *>("")));
    auto ret =
        wallpaperService->SaveWallpaperState(userId, WallpaperType::WALLPAPER_SYSTEM, WallpaperResourceType::DEFAULT);
    if (image != nullptr) {
        cJSON_Delete(image);
    }
    EXPECT_EQ(ret, false) << "Failed to SaveWallpaperState";
}

/**
* @tc.name: SaveWallpaperState
* @tc.desc: save wallpaper state err test, cJSON_Parse return nullptr.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(WallpaperTestMock, SaveWallpaperState004, TestSize.Level0)
{
    HILOG_INFO("SaveWallpaperState004 begin");
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    wallpaperService->LoadWallpaperState();
    EXPECT_CALL(*wallpaperMock, cJSON_Parse(_)).WillRepeatedly(Return(nullptr));
    int32_t userId = wallpaperService->QueryActiveUserId();
    auto ret =
        wallpaperService->SaveWallpaperState(userId, WallpaperType::WALLPAPER_SYSTEM, WallpaperResourceType::DEFAULT);
    EXPECT_EQ(ret, false) << "Failed to SaveWallpaperState";
}

/**
* @tc.name: GetWallpaperPathInJson
* @tc.desc: get wallpaper path err test, cJSON_Parse return nullptr.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(WallpaperTestMock, GetWallpaperPathInJson001, TestSize.Level0)
{
    HILOG_INFO("GetWallpaperPathInJson001 begin");
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    EXPECT_CALL(*wallpaperMock, cJSON_Parse(_)).WillRepeatedly(Return(nullptr));
    std::string manifestName = "home/manifest.json";
    std::string filePath = "";
    auto ret = wallpaperService->GetWallpaperPathInJson(manifestName, filePath);
    EXPECT_TRUE(ret.empty()) << "Failed to GetWallpaperPathInJson";
}

/**
* @tc.name: GetWallpaperPathInJson
* @tc.desc: get wallpaper path err test, cJSON_GetObjectItemCaseSensitive return nullptr.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(WallpaperTestMock, GetWallpaperPathInJson002, TestSize.Level0)
{
    HILOG_INFO("GetWallpaperPathInJson002 begin");
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    std::string resPath = wallpaperService->GetDefaultResDir();
    if (resPath.empty() && !FileDeal::IsDirExist(resPath)) {
        HILOG_ERROR("wallpaperDefaultDir get failed!");
    }
    std::string manifestName = "home/manifest.json";
    std::string manifestFile = resPath + manifestName;
    std::ifstream file(manifestFile);
    if (!file.is_open()) {
        HILOG_ERROR("open fail:%{public}s", manifestFile.c_str());
        file.close();
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    cJSON *image = cJSON_Parse(content.c_str());
    if (image == nullptr) {
        HILOG_ERROR("Failed to parse json.");
    }
    EXPECT_CALL(*wallpaperMock, cJSON_Parse(_)).WillRepeatedly(Return(image));
    EXPECT_CALL(*wallpaperMock, cJSON_GetObjectItemCaseSensitive(image, _)).WillRepeatedly(Return(nullptr));
    std::string filePath = "";
    auto ret = wallpaperService->GetWallpaperPathInJson(manifestName, filePath);
    if (image != nullptr) {
        cJSON_Delete(image);
    }
    EXPECT_TRUE(ret.empty()) << "Failed to GetWallpaperPathInJson";
}

/**
* @tc.name: GetWallpaperPathInJson
* @tc.desc: get wallpaper path err test, cJSON is not object.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(WallpaperTestMock, GetWallpaperPathInJson003, TestSize.Level0)
{
    HILOG_INFO("GetWallpaperPathInJson003 begin");
    std::shared_ptr<WallpaperService> wallpaperService = std::make_shared<WallpaperService>();
    std::string resPath = wallpaperService->GetDefaultResDir();
    if (resPath.empty() && !FileDeal::IsDirExist(resPath)) {
        HILOG_ERROR("wallpaperDefaultDir get failed!");
    }
    std::string manifestName = "home/manifest.json";
    std::string manifestFile = resPath + manifestName;
    std::ifstream file(manifestFile);
    if (!file.is_open()) {
        HILOG_ERROR("open fail:%{public}s", manifestFile.c_str());
        file.close();
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    cJSON *image = cJSON_Parse(content.c_str());
    if (image == nullptr) {
        HILOG_ERROR("Failed to parse json.");
    }
    EXPECT_CALL(*wallpaperMock, cJSON_Parse(_)).WillRepeatedly(Return(image));
    EXPECT_CALL(*wallpaperMock, cJSON_GetObjectItemCaseSensitive(image, _)).WillRepeatedly(Return(image));
    std::string filePath = "";
    auto ret = wallpaperService->GetWallpaperPathInJson(manifestName, filePath);
    if (image != nullptr) {
        cJSON_Delete(image);
    }
    EXPECT_TRUE(ret.empty()) << "Failed to GetWallpaperPathInJson";
}
} // namespace WallpaperMgrService
} // namespace OHOS