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

#ifndef SERVICES_INCLUDE_WALLPAPER_COMPONENT_NAME_H
#define SERVICES_INCLUDE_WALLPAPER_COMPONENT_NAME_H
#include <string>

namespace OHOS {
namespace WallpaperMgrService {
class ComponentName {
public:
    ComponentName();
    ComponentName(std::string pkg, std::string cls);
    std::string GetPackageName() const;
    std::string GetClassName() const;
    bool equals(const ComponentName &obj) const;
    void SetComponentInfo(std::string pkg, std::string cls);

private:
    std::string package_;
    std::string class_;
};
} // namespace WallpaperMgrService
} // namespace OHOS

#endif // SERVICES_INCLUDE_WALLPAPER_COMPONENT_NAME_H
