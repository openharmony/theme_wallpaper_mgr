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

/**
* class of wallpaper extension.
*
* @systemapi Hide this for inner system use.
* @since 9
* @sysCap SystemCapability.Miscservices.Wallpaper
* @StageModelOnly
*/
declare class WallpaperExtension {
    /**
     * Called once to initialize the extension.
     *
     * @systemapi Hide this for inner system use.
     * @since 9
     * @sysCap SystemCapability.Miscservices.Wallpaper
     * @return -
     * @StageModelOnly
     */
    onCreated(want: object): void;

    /**
     * Called when the extension's size is changed.
     * @systemapi Hide this for inner system use.
     * @since 9
     * @sysCap SystemCapability.Miscservices.Wallpaper
     * @param width Indicates the wallpaper width
     * @param height Indicates the wallpaper height
     */
    onWallpaperChanged(wallpaperType: number): void;

    /**
     * Called to set ui content of the extension.
     *
     * @systemapi Hide this for inner system use.
     * @since 9
     * @param context extension's context
     * @param uri the uri of the ui layout file
     * @sysCap SystemCapability.Miscservices.Wallpaper
     * @return -
     * @StageModelOnly
     */
    setUiContent(uri:string): void;

    /**
     * Called once to destroy the exten.
     *
     * @systemapi Hide this for inner system use.
     * @since 9
     * @sysCap SystemCapability.Miscservices.Wallpaper
     * @return -
     * @StageModelOnly
     */
    onDestroy(): void;
}

export default WallpaperExtension;
