/*
* Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef WALLPAPER_EXTENSION_STUB_H
#define WALLPAPER_EXTENSION_STUB_H
#include "i_wallpaper_extension.h"
#include "iremote_stub.h"

namespace OHOS {
namespace WallpaperMgrService {
class WallpaperExtensionStub : public IRemoteStub<IWallpaperExtension> {};
} // namespace WallpaperMgrService
} // namespace OHOS
#endif // WALLPAPER_EXTENSION_STUB_H