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

#include "hilog_wrapper.h"
#include "wallpaper_picture_info_by_parcel.h"

namespace OHOS::WallpaperMgrService {
constexpr int32_t VECTOR_MAX_SIZE = 6;
WallpaperPictureInfoByParcel::WallpaperPictureInfoByParcel()
{
}

bool WallpaperPictureInfoByParcel::Marshalling(Parcel &parcel) const
{
    bool status = true;
    status &= parcel.WriteInt32(wallpaperPictureInfo_.size());
    for (const auto &wallpaperInfo : wallpaperPictureInfo_) {
        status &= parcel.WriteInt32(static_cast<int32_t>(wallpaperInfo.foldState));
        status &= parcel.WriteInt32(static_cast<int32_t>(wallpaperInfo.rotateState));
        status &= parcel.WriteInt32(wallpaperInfo.fd);
        status &= parcel.WriteInt64(wallpaperInfo.length);
    }
    return status;
}

WallpaperPictureInfoByParcel *WallpaperPictureInfoByParcel::Unmarshalling(Parcel &parcel)
{
    WallpaperPictureInfoByParcel *obj = new (std::nothrow) WallpaperPictureInfoByParcel();
    if (obj == nullptr) {
        HILOG_ERROR("obj is nullptr");
        return nullptr;
    }
    int32_t vectorSize = parcel.ReadInt32();
    if (vectorSize > VECTOR_MAX_SIZE) {
        HILOG_ERROR("More than maxNum 6 of wallpaper pictures");
        delete obj;
        return nullptr;
    }
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
        wallpaperInfo.length = parcel.ReadInt64();
        obj->wallpaperPictureInfo_.push_back(wallpaperInfo);
    }
    return obj;
}

} // namespace OHOS::WallpaperMgrService