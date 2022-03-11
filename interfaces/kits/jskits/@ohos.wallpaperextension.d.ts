/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

/**
 * WallpaperExtension
 * @sysCap SystemCapability.Miscservices.Wallpaper
 * @devices phone, tablet, tv, wearable, car
 * @import import WallpaperExtension from '@ohos.application.WallpaperExtension';
 * @since 7
 */
declare class WallpaperExtension {
    /**
     * Called once to initialize the engine.
     */
    onCreated(want: object): void;

    /**
     * the wallpaper's size is changed.
     * @param width Indicates the wallpaper width
     * @param height Indicates the wallpaper height
     */
    onWallpaperChanged(wallpaperType: number): void;

    /**
     * set UI layout
     * @param url the ui layout 
     */
    setUiContent(url:string): void;

    /**
     * Called once the engine is destroyed.
     */
    onDestroy(): void;
}

export default WallpaperExtension;
