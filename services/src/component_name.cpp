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

#include "component_name.h"
namespace OHOS {
namespace WallpaperMgrService {
ComponentName::ComponentName()
{
}

ComponentName::ComponentName(std::string pkg, std::string cls)
{
    package_ = std::move(pkg);
    class_ = std::move(cls);
}

void ComponentName::SetComponentInfo(std::string pkg, std::string cls)
{
    package_ = std::move(pkg);
    class_ = std::move(cls);
}

std::string ComponentName::GetPackageName() const
{
    return package_;
}
std::string ComponentName::GetClassName() const
{
    return class_;
}

bool ComponentName::equals(const ComponentName &obj) const
{
    return package_ == obj.GetPackageName() && class_ == obj.GetClassName();
}
} // namespace WallpaperMgrService
} // namespace OHOS