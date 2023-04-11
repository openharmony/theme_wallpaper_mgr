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

let WindowManager = requireNapi('window');
let WindowName = 'wallpaper';
let windowType = 2000;
let windowsCreated = false;
let WIDTH = 480;
let HEIGHT = 960;

class WallpaperExtension {
  createWallpaperWin() {
    console.log(WindowName + ' createWallpaperWin');
    console.log(this.context);
    console.log(WindowName + JSON.stringify(this.context));
    console.log(windowType);
    console.log(WindowName + JSON.stringify(windowType));

    WindowManager.create(this.context, WindowName, windowType).then((win) => {
      console.log(WindowName + 'wallpaperWindow');
      this.wallpaperWindow = win;
      console.log(WindowName + JSON.stringify(this.wallpaperWindow));
      console.log(this.wallpaperWindow);
      console.log(WindowName + 'moveTo');
      this.wallpaperWindow.moveTo(0, 0).then(() => {
        console.log(WindowName + 'resetSize');
        this.wallpaperWindow.resetSize(WIDTH, HEIGHT).then(() => {
          console.log(WindowName + ' resetSize' + JSON.stringify(this.wallpaperURL));
          this.loadUiContent(this.wallpaperURL);
          console.log(WindowName + ' window created');
          windowsCreated = true;
        });
      });
    }, (error) => {
      console.log(WindowName + ' window createFailed, error.code = ' + error.code);
    });
  }
  onCreated(want) {
    console.log(WindowName + 'onWallpaperExtensionCreated');
  }

  setUiContent(url) {
    console.log(WindowName + ' setUiContent');
    if (windowsCreated) {
      console.log(WindowName + ' loadUiContent');
      loadUiContent(url);
    } else {
      console.log(WindowName + ' save wallpaperURL');
      this.wallpaperURL = url;
    }
  }

  loadUiContent(url) {
    console.log(WindowName + 'initUiContent' + url);
    console.log(WindowName + JSON.stringify(this.wallpaperWindow));
    console.log(WindowName + JSON.stringify(this.wallpaperWindow.loadContent));
    console.log(WindowName + JSON.stringify(this.wallpaperWindow.loadContent(url)));
    this.wallpaperWindow.loadContent(url).then(() => {
      console.log(WindowName + ' loadContent');
      this.wallpaperWindow.show().then(() => {
        console.log(WindowName + ' window is show');
      });
    });
  }

  onWallpaperChanged(wallpaperType) {
    console.log(WindowName + 'onWallpaperChanged' + JSON.stringify(wallpaperType));
  }

  onDestroy() {
    console.log(WindowName + 'onWallpaperExtensionDestroy');
  }
}

export default WallpaperExtension;