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

#ifndef SERVICES_INCLUDE_WALLPAPER_COMMONENT_NAME_H
#define SERVICES_INCLUDE_WALLPAPER_COMMONENT_NAME_H
#include <string>

namespace OHOS {
namespace WallpaperMgrService {
class ComponentName {
public:
    ComponentName();
    ComponentName(std::string pkg, std::string cls);
    /**
     * * Return the package name of this component.
    */
    std::string GetPackageName() const;
    
    /**
     * Return the class name of this component.
     */
    std::string GetClassName() const;
    bool equals(const ComponentName& obj);
    void SetComponentInfo(std::string pkg, std::string cls);
private:
    std::string mPackage;
    std::string mClass;
};
}
}

#endif
