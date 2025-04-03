/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <vector>

#include "wallpaper_picture_info_by_parcel.h"

namespace OHOS::WallpaperMgrService {
WallpaperPictureInfoByParcel::WallpaperPictureInfoByParcel()
{
}

bool WallpaperPictureInfoByParcel::Marshalling(Parcel &parcel) const
{
    bool status = true;
    status &= parcel.WriteInt32(wallpaperPictureInfo_.size());
    for (const auto &wallpaperInfo : wallpaperPictureInfo_) {
        status &= parcel.WriteInt32(wallpaperInfo.foldState);
        status &= parcel.WriteInt32(wallpaperInfo.rotateState);
        status &= parcel.WriteInt32(wallpaperInfo.fd);
        status &= parcel.WriteInt32(wallpaperInfo.length);
    }
    return status;
}

WallpaperPictureInfoByParcel *WallpaperPictureInfoByParcel::Unmarshalling(Parcel &parcel)
{
    WallpaperPictureInfoByParcel *obj = new (std::nothrow) WallpaperPictureInfoByParcel();
    if (obj == nullptr) {
        return nullptr;
    }
    int32_t vectorSize = parcel.ReadInt32();
    for (int32_t i = 0; i < vectorSize; i++) {
        WallpaperPictureInfo wallpaperInfo;
        int32_t foldStateVale = parcel.ReadInt32();
        int32_t rotateStateVale = parcel.ReadInt32();
        if (foldStateVale == static_cast<int32_t>(FoldState::NORMAL)) {
            wallpaperInfo.foldState = NORMAL;
        } else if (foldStateVale == static_cast<int32_t>(FoldState::UNFOLD_1)) {
            wallpaperInfo.foldState = UNFOLD_1;
        } else if (foldStateVale == static_cast<int32_t>(FoldState::UNFOLD_2)) {
            wallpaperInfo.foldState = UNFOLD_2;
        }
        if (rotateStateVale == static_cast<int32_t>(RotateState::PORT)) {
            wallpaperInfo.rotateState = PORT;
        } else if (rotateStateVale == static_cast<int32_t>(RotateState::LAND)) {
            wallpaperInfo.rotateState = LAND;
        }
        wallpaperInfo.fd = parcel.ReadInt32();
        wallpaperInfo.length = parcel.ReadInt32();
        obj->wallpaperPictureInfo_.push_back(wallpaperInfo);
    }
    return obj;
}

} // namespace OHOS::WallpaperMgrService