/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// @ts-nocheck
import wallpaper from '@ohos.wallpaper'
import image from '@ohos.multimedia.image'
import fileio from '@ohos.fileio'
import bundleManager from "@ohos.bundle.bundleManager"

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

const WALLPAPER_SYSTEM = 0;
const WALLPAPER_LOCKSCREEN = 1;
const INVALID_WALLPAPER_TYPE = 2;
const NORMAL = 0;
const UNFOLD_1 = 1;
const UNFOLD_2 = 2;
const INVALID_FOLDSTATE = 3;
const PORT = 0;
const LAND = 1;
const INVALID_ROTATESTATE = 2;
const DEFAULT_WALLPAPER_ID = -1;
const PARAMETER_ERROR = 401;
const PARAMETER_ERROR_MESSAGE = "BusinessError 401: Parameter error.";
const PARAMETER_COUNT = "Mandatory parameters are left unspecified.";
const WALLPAPERTYPE_PARAMETER_TYPE = "The type must be WallpaperType, parameter range must be " +
    "WALLPAPER_LOCKSCREEN or WALLPAPER_SYSTEM.";
const FOLDSTATE_PARAMETER_TYPE = "The type must be FoldState, parameter range must be " +
    "NORMAL or UNFOLD_1 or UNFOLD_2.";
const ROTATESTATE_PARAMETER_TYPE = "The type must be RotateState, parameter range must be " +
    "PORT or LAND.";
const URI = "/data/storage/el2/base/haps/js.jpeg";
const URI_ZIP = "/system/etc/test.zip";
const URI_30FPS_3S_MP4 = "/system/etc/30fps_3s.mp4";
const URI_15FPS_7S_MP4 = "/system/etc/15fps_7s.mp4";
const URI_30FPS_3S_MOV = "/system/etc/30fps_3s.mov";

const SHOW_SYSTEM_SCREEN = "SHOW_SYSTEMSCREEN";
const SHOW_LOCK_SCREEN = "SHOW_LOCKSCREEN";
const BUNDLE_NAME = "com.ohos.sceneboard";

describe('WallpaperJSTest', function () {
    beforeAll(async function () {
        // input testsuite setup step，setup invoked before all testcases
        console.info('beforeAll called')
        await createTempImage();
    })
    beforeEach(function () {
        // input testcase setup step，setup invoked before each testcases
        console.info('beforeEach called')
    })
    afterEach(function () {
        // input testcase teardown step，teardown invoked after each testcases
        console.info('afterEach called')
    })
    afterAll(async function () {
        // input testsuite teardown step，teardown invoked after all testcases
        console.info('afterAll called')
        await wallpaper.restore(WALLPAPER_SYSTEM);
        await wallpaper.restore(WALLPAPER_LOCKSCREEN);
    })

    async function createTempImage() {
        let pixelMap = await createTempPixelMap();
        const imagePackerApi = image.createImagePacker();
        let packOpts = {format: "image/jpeg", quality: 98};
        imagePackerApi.packing(pixelMap, packOpts, (err, data) => {
            if (err) {
                console.info(`packing error: ${err}`)
            } else {
                let fd = fileio.openSync(URI, 0o2 | 0o100, 0o666);
                let ret = fileio.writeSync(fd, data);
                fileio.close(fd);
                console.log(`file write ret: ${ret}`);
            }
        })
    }

    async function createTempPixelMap() {
        const color = new ArrayBuffer(96);
        let opts = {editable: true, pixelFormat: 3, size: {height: 4, width: 6}};
        let pixelMap = await image.createPixelMap(color, opts);
        return pixelMap;
    }

    function isBundleNameExists() {
        try {
            bundleManager.getBundleInfo(BUNDLE_NAME, bundleManager.BundleFlag.GET_BUNDLE_INFO_DEFAULT, (e) => {
                if (e) {
                    console.info(`getBundleInfo error ${e.code}`);
                    return false;
                } else {
                    console.info(`Wallpaper : getBundleInfo is success`);
                    return true;
                }
            })
        } catch (error) {
            console.info(`getBundleInfo error ${error.code}`);
            return false;
        }

    }

    /**
     * @tc.name:      getColorsSyncTest001
     * @tc.desc:      Test getColorsSync() to gets WALLPAPER_SYSTEM Colors by syncing.
     * @tc.type:      FUNC
     * @tc.require:   issueI5UHRG
     */
    it('getColorsSyncTest001', 0, function () {
        try {
            let data = wallpaper.getColorsSync(WALLPAPER_SYSTEM);
            console.info(`getColorsSyncTest001 data : ${data}`);
            if (data !== undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info(`getColorsSyncTest001 error ${error}`);
            expect(null).assertFail();
        }
    })


    /**
     * @tc.name:      getColorsSyncTest002
     * @tc.desc:      Test getColorsSync() to gets WALLPAPER_LOCKSCREEN Colors by syncing.
     * @tc.type:      FUNC
     * @tc.require:   issueI5UHRG
     */
    it('getColorsSyncTest002', 0, function () {
        try {
            let data = wallpaper.getColorsSync(WALLPAPER_LOCKSCREEN);
            console.info(`getColorsSyncTest002 data : ${data}`);
            if (data !== undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info(`getColorsSyncTest002 error : ${error}`);
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      getColorsSyncTest003
     * @tc.desc:      Test getColorsSync() throw parameter error.
     * @tc.type:      FUNC
     * @tc.require:   issueI5UHRG
     */
    it('getColorsSyncTest003', 0, function () {
        try {
            let data = wallpaper.getColorsSync(INVALID_WALLPAPER_TYPE);
            console.info(`getColorsSyncTest003 data : ${data}`);
            expect(null).assertFail();
        } catch (error) {
            console.info(`getColorsSyncTest003 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue()
        }
    })

    /**
     * @tc.name:      getColorsSyncTest004
     * @tc.desc:      Test getColorsSync() throw parameter error.
     * @tc.type:      FUNC
     * @tc.require:   issueI5UHRG
     */
    it('getColorsSyncTest004', 0, function () {
        try {
            let data = wallpaper.getColorsSync();
            console.info(`getColorsSyncTest004 data : ${data}`);
            expect(null).assertFail();
        } catch (error) {
            console.info(`getColorsSyncTest004 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue()
        }
    })

    /**
     * @tc.name:      getMinHeightSyncTest001
     * @tc.desc:      Test getMinHeightSync() to gets the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinHeightSyncTest001', 0, function () {
        let data = wallpaper.getMinHeightSync();
        console.info(`getMinHeightSyncTest001 data : ${data}`);
        if (data !== undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getMinWidthSyncTest001
     * @tc.desc:      Test getMinWidthSync() to gets the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinWidthSyncTest001', 0, function () {
        let data = wallpaper.getMinWidthSync();
        console.info(`getMinWidthSyncTest001 data : ${data}`);
        if (data !== undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      restoreCallbackSystemTest001
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackSystemTest001', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_SYSTEM, function (err) {
                if (err) {
                    console.info(`restoreCallbackSystemTest001 err : ${err}`);
                    expect(null).assertFail()
                } else {
                    expect(true).assertTrue();
                }
                done();
            })

        } catch (error) {
            console.info(`restoreCallbackSystemTest001 err : ${error}`);
            expect(null).assertFail();
            done();
        }

    })

    /**
     * @tc.name:      restorePromiseSystemTest002
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseSystemTest002', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_SYSTEM).then(() => {
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`restorePromiseSystemTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      restoreCallbackLockTest003
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackLockTest003', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_LOCKSCREEN, function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`restore CallbackLockTest003 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            console.info(`restore CallbackLockTest003 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      restorePromiseLockTest004
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseLockTest004', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_LOCKSCREEN).then(() => {
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`restorePromiseLockTest004 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      restoreCallbackThrowErrorTest005
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackThrowErrorTest005', 0, async function (done) {
        try {
            wallpaper.restore(INVALID_WALLPAPER_TYPE, function (err) {
                if (err) {
                    expect(err.code === PARAMETER_ERROR).assertTrue()
                    console.info(`restore CallbackThrowErrorTest005 fail : ${err}`);
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            console.info(`restore CallbackThrowErrorTest005 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      restoreCallbackThrowErrorTest006
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackThrowErrorTest006', 0, async function (done) {
        try {
            wallpaper.restore(function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`restore CallbackThrowErrorTest006 fail : ${err}`);
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            console.info(`restore CallbackThrowErrorTest006 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      restorePromiseThrowErrorTest007
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseThrowErrorTest007', 0, async function (done) {
        try {
            wallpaper.restore(INVALID_WALLPAPER_TYPE).then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`restorePromiseThrowErrorTest007 err : ${err}`);
                expect(err.code === PARAMETER_ERROR).assertTrue()
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      restorePromiseThrowErrorTest008
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseThrowErrorTest008', 0, async function (done) {
        try {
            wallpaper.restore().then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`restorePromiseThrowErrorTest008 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      getImagePromiseLockTest001
     * @tc.desc:      Test getImage() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseLockTest001', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info(`getImagePromiseLockTest001 data : ${data}`);
                if (data !== undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info(`getImagePromiseLockTest001 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getImageCallbackSystemTest002
     * @tc.desc:      Test getImage() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackSystemTest002', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`get Image CallbackSystemTest002 fail : ${err}`);
                } else {
                    console.info(`get Image CallbackSystemTest002 data : ${data}`);
                    if (data !== undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`get Image CallbackSystemTest002 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      getImagePromiseSystemTest003
     * @tc.desc:      Test getImage() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseSystemTest003', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM).then((data) => {
                console.info(`getImagePromiseSystemTest003 data : ${data}`);
                if (data !== undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info(`getImagePromiseSystemTest003 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getImageCallbackLockTest004
     * @tc.desc:      Test getImage() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackLockTest004', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`get Image CallbackLockTest004 fail : ${err}`);
                } else {
                    console.info(`get Image CallbackLockTest004 data : ${data}`);
                    if (data !== undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`get Image CallbackLockTest004 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      getImageCallbackThrowErrorTest005
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackThrowErrorTest005', 0, async function (done) {
        try {
            wallpaper.getImage(INVALID_WALLPAPER_TYPE, function (err, data) {
                console.error(`getImageCallbackThrowErrorTest005 err : ${err}`);
                console.error(`getImageCallbackThrowErrorTest005 data : ${data}`);
                expect(null).assertTrue();
                done();
            })
        } catch (error) {
            console.info(`getImageCallbackThrowErrorTest005 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + WALLPAPERTYPE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:      getImageCallbackThrowErrorTest006
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackThrowErrorTest006', 0, async function (done) {
        try {
            wallpaper.getImage(function (err, data) {
                if (err) {
                    console.info(`getImageCallbackThrowErrorTest006 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getImageCallbackThrowErrorTest006 data : ${data}`);
                    if (data !== undefined) {
                        expect(null).assertFail();
                    }
                }
                done();
            })
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      getImagePromiseThrowErrorTest007
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseThrowErrorTest007', 0, async function (done) {
        try {
            wallpaper.getImage(INVALID_WALLPAPER_TYPE).then((data) => {
                console.error(`getImagePromiseThrowErrorTest007 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getImagePromiseThrowErrorTest007 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getImagePromiseThrowErrorTest007 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + WALLPAPERTYPE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:      getImagePromiseThrowErrorTest008
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseThrowErrorTest008', 0, async function (done) {
        try {
            wallpaper.getImage().then((data) => {
                console.info(`getImagePromiseThrowErrorTest008 data : ${data}`);
                if (data !== undefined) {
                    expect(null).assertFail();
                }
                done();
            }).catch((err) => {
                console.info(`getImagePromiseThrowErrorTest008 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest001
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 000.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest001', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, NORMAL, PORT).then((data) => {
                console.info(`getCorrespondWallpaperTest001 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest001 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest001 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest002
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 001.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest002', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, NORMAL, LAND).then((data) => {
                console.info(`getCorrespondWallpaperTest002 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest002 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest002 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest003
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 010.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest003', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, UNFOLD_1, PORT).then((data) => {
                console.info(`getCorrespondWallpaperTest003 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest003 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest003 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest004
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 011.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest004', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, UNFOLD_1, LAND).then((data) => {
                console.info(`getCorrespondWallpaperTest004 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest004 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest004 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest005
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 020.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest005', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, UNFOLD_2, PORT).then((data) => {
                console.info(`getCorrespondWallpaperTest005 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest005 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest005 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest006
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 021.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest006', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, UNFOLD_2, LAND).then((data) => {
                console.info(`getCorrespondWallpaperTest006 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest006 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest006 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest007
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 100.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest007', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_LOCKSCREEN, NORMAL, PORT).then((data) => {
                console.info(`getCorrespondWallpaperTest007 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest007 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest007 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })
    /**
     * @tc.name:    getCorrespondWallpaperTest008
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 101.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest008', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_LOCKSCREEN, NORMAL, LAND).then((data) => {
                console.info(`getCorrespondWallpaperTest008 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest008 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest008 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })
    /**
     * @tc.name:    getCorrespondWallpaperTest009
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 110.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest009', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_LOCKSCREEN, UNFOLD_1, PORT).then((data) => {
                console.info(`getCorrespondWallpaperTest009 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest009 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest009 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest010
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 111.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest010', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_LOCKSCREEN, UNFOLD_1, LAND).then((data) => {
                console.info(`getCorrespondWallpaperTest010 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest010 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest010 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest011
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 120.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest011', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_LOCKSCREEN, UNFOLD_2, PORT).then((data) => {
                console.info(`getCorrespondWallpaperTest011 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest011 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest011 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest012
     * @tc.desc:    Test getCorrespondWallpaper() with normal argc 121.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest012', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_LOCKSCREEN, UNFOLD_2, LAND).then((data) => {
                console.info(`getCorrespondWallpaperTest012 data : ${data}`);
                expect(data !== undefined && data !== null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest012 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.error(`getCorrespondWallpaperTest012 error : ${error}`);
            expect(null).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest013
     * @tc.desc:    Test getCorrespondWallpaper() lack of argc.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest013', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_LOCKSCREEN, UNFOLD_1).then((data) => {
                console.error(`getCorrespondWallpaperTest013 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest013 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getCorrespondWallpaperTest013 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + PARAMETER_COUNT).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest014
     * @tc.desc:    Test getCorrespondWallpaper() with invalid wallpaper type.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest014', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(INVALID_WALLPAPER_TYPE, UNFOLD_2, LAND).then((data) => {
                console.error(`getCorrespondWallpaperTest014 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest014 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getCorrespondWallpaperTest014 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + WALLPAPERTYPE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest015
     * @tc.desc:    Test getCorrespondWallpaper() with invalid foldstate.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest015', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, INVALID_FOLDSTATE, PORT).then((data) => {
                console.error(`getCorrespondWallpaperTest015 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest015 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getCorrespondWallpaperTest015 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + FOLDSTATE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest016
     * @tc.desc:    Test getCorrespondWallpaper() with invalid rotatestate.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest016', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, NORMAL, INVALID_ROTATESTATE).then((data) => {
                console.error(`getCorrespondWallpaperTest016 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest016 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getCorrespondWallpaperTest016 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + ROTATESTATE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest017
     * @tc.desc:    Test getCorrespondWallpaper() with invalid wallpaper type.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest017', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper("INVALID_TYPE", NORMAL, PORT).then((data) => {
                console.error(`getCorrespondWallpaperTest017 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest017 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getCorrespondWallpaperTest017 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + WALLPAPERTYPE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest018
     * @tc.desc:    Test getCorrespondWallpaper() to get wallpaper.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest018', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, "INVALID_TYPE", PORT).then((data) => {
                console.error(`getCorrespondWallpaperTest018 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest018 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getCorrespondWallpaperTest018 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + FOLDSTATE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:    getCorrespondWallpaperTest019
     * @tc.desc:    Test getCorrespondWallpaper() to get wallpaper.
     * @tc.type:    FUNC
     */
    it('getCorrespondWallpaperTest019', 0, async function (done) {
        try {
            wallpaper.getCorrespondWallpaper(WALLPAPER_SYSTEM, NORMAL, "INVALID_TYPE").then((data) => {
                console.error(`getCorrespondWallpaperTest019 data : ${data}`);
                expect(null).assertTrue();
                done();
            }).catch((err) => {
                console.error(`getCorrespondWallpaperTest019 err : ${err}`);
                expect(null).assertTrue();
                done();
            });
        } catch (error) {
            console.info(`getCorrespondWallpaperTest019 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            expect(error.message === PARAMETER_ERROR_MESSAGE + ROTATESTATE_PARAMETER_TYPE).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:      setImageURIPromiseLockTest001
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURIPromiseLockTest001', 0, async function (done) {
        try {
            wallpaper.setImage(URI, WALLPAPER_LOCKSCREEN).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            }).catch((err) => {
                console.info(`setImageURIPromiseLockTest001 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageURICallbackSystemTest002
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURICallbackSystemTest002', 0, async function (done) {
        try {
            wallpaper.setImage(URI, WALLPAPER_SYSTEM, async function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`set Image URI CallbackSystemTest002 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Image URI CallbackSystemTest002 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setImageURIPromiseSystemTest003
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURIPromiseSystemTest003', 0, async function (done) {
        try {
            wallpaper.setImage(URI, WALLPAPER_SYSTEM).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            }).catch((err) => {
                console.info(`setImageURIPromiseSystemTest003 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageURICallbackLockTest004
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURICallbackLockTest004', 0, async function (done) {
        try {
            wallpaper.setImage(URI, WALLPAPER_LOCKSCREEN, async function (err) {
                if (err) {
                    console.info(`setImageURICallbackLockTest004 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    expect(true).assertTrue();
                }
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            });
            await wallpaper.restore(WALLPAPER_LOCKSCREEN);
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageMapPromiseLockTest005
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapPromiseLockTest005', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            }).catch((err) => {
                console.info(`setImageMapPromiseLockTest005 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageMapCallbackSystemTest006
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapCallbackSystemTest006', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM, async function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`set ImageMap CallbackSystemTest006 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`set ImageMap CallbackSystemTest006 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setImageMapPromiseSystemTest007
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapPromiseSystemTest007', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            }).catch((err) => {
                console.info(`setImageMapPromiseSystemTest007 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageMapCallbackLockTest008
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapCallbackLockTest008', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN, async function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`set ImageMap CallbackLockTest008 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`set ImageMap CallbackLockTest008 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setImageCallbackThrowErrorTest009
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageCallbackThrowErrorTest009', 0, async function (done) {
        try {
            wallpaper.setImage(URI, INVALID_WALLPAPER_TYPE, function (err) {
                if (err) {
                    expect(err.code === PARAMETER_ERROR).assertTrue()
                    console.info(`set Image CallbackThrowErrorTest009 fail : ${err}`);
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Image CallbackThrowErrorTest009 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setImageCallbackThrowErrorTest010
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageCallbackThrowErrorTest010', 0, async function (done) {
        try {
            wallpaper.setImage(URI, function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`set Image CallbackThrowErrorTest010 fail : ${err}`);
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            console.info(`set Image CallbackThrowErrorTest010 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setImagePromiseThrowErrorTest011
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImagePromiseThrowErrorTest011', 0, async function (done) {
        try {
            wallpaper.setImage(URI, INVALID_WALLPAPER_TYPE).then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`setImagePromiseThrowErrorTest011 err : ${err}`);
                expect(err.code === PARAMETER_ERROR).assertTrue()
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImagePromiseThrowErrorTest012
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImagePromiseThrowErrorTest012', 0, async function (done) {
        try {
            wallpaper.setImage().then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`setImagePromiseThrowErrorTest012 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      setWallpaperMapPromiseLockTest001
     * @tc.desc:      Test setWallpaper() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setWallpaperMapPromiseLockTest001', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setWallpaper(pixelMap, WALLPAPER_LOCKSCREEN).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            }).catch((err) => {
                console.info(`setWallpaperMapPromiseLockTest001 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setWallpaperMapCallbackSystemTest002
     * @tc.desc:      Test setWallpaper() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setWallpaperMapCallbackSystemTest002', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setWallpaper(pixelMap, WALLPAPER_SYSTEM, async function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`set Wallpaper Map CallbackSystemTest002 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Wallpaper Map CallbackSystemTest002 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setWallpaperMapPromiseSystemTest003
     * @tc.desc:      Test setWallpaper() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setWallpaperMapPromiseSystemTest003', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setWallpaper(pixelMap, WALLPAPER_SYSTEM).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            }).catch((err) => {
                console.info(`setWallpaperMapPromiseSystemTest003 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setWallpaperMapCallbackLockTest004
     * @tc.desc:      Test setWallpaper() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setWallpaperMapCallbackLockTest004', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setWallpaper(pixelMap, WALLPAPER_LOCKSCREEN, async function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`set Wallpaper Map CallbackLockTest004 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Wallpaper Map CallbackLockTest004 fail : ${error}`);
            done();
        }
    })


    /**
     * @tc.name:      getPixelMapPromiseLockTest001
     * @tc.desc:      Test getPixelMap() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getPixelMapPromiseLockTest001', 0, async function (done) {
        try {
            wallpaper.getPixelMap(WALLPAPER_LOCKSCREEN).then((data) => {
                if (data !== undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info(`getPixelMapPromiseLockTest001 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getPixelMapCallbackSystemTest002
     * @tc.desc:      Test getPixelMap() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getPixelMapCallbackSystemTest002', 0, async function (done) {
        try {
            wallpaper.getPixelMap(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`get Pixel Map CallbackSystemTest002 fail : ${err}`);
                } else {
                    if (data !== undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`get Pixel MapCallbackSystemTest002 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      getPixelMapPromiseSystemTest003
     * @tc.desc:      Test getPixelMap() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getPixelMapPromiseSystemTest003', 0, async function (done) {
        try {
            wallpaper.getPixelMap(WALLPAPER_SYSTEM).then((data) => {
                if (data !== undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info(`getPixelMapPromiseSystemTest003 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getPixelMapCallbackLockTest004
     * @tc.desc:      Test getPixelMap() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getPixelMapCallbackLockTest004', 0, async function (done) {
        try {
            wallpaper.getPixelMap(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`get Pixel Map CallbackLockTest004 fail : ${err}`);
                } else {
                    if (data !== undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`get Pixel Map CallbackLockTest004 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      resetCallbackSystemTest001
     * @tc.desc:      Test reset() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('resetCallbackSystemTest001', 0, async function (done) {
        try {
            wallpaper.reset(WALLPAPER_SYSTEM, function (err) {
                if (err) {
                    expect(null).assertFail()
                    console.info(`reset CallbackSystemTest001 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
            })

        } catch (error) {
            expect(null).assertFail();
            console.info(`reset CallbackSystemTest001 fail : ${error}`);
            done();
        }

    })

    /**
     * @tc.name:      resetPromiseSystemTest002
     * @tc.desc:      Test reset() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('resetPromiseSystemTest002', 0, async function (done) {
        try {
            wallpaper.reset(WALLPAPER_SYSTEM).then(() => {
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`resetPromiseSystemTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      resetCallbackLockTest003
     * @tc.desc:      Test reset() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('resetCallbackLockTest003', 0, async function (done) {
        try {
            wallpaper.reset(WALLPAPER_LOCKSCREEN, function (err) {
                if (err) {
                    expect(null).assertFail();
                    console.info(`reset CallbackLockTest003 fail : ${err}`);
                } else {
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            console.info(`reset CallbackLockTest003 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      resetPromiseLockTest004
     * @tc.desc:      Test reset() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('resetPromiseLockTest004', 0, async function (done) {
        try {
            wallpaper.reset(WALLPAPER_LOCKSCREEN).then(() => {
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`resetPromiseLockTest004 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      isOperationAllowedCallbackTest001
     * @tc.desc:      Test isOperationAllowed() to checks whether a user is allowed to set wallpapers.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isOperationAllowedCallbackTest001', 0, async function (done) {
        try {
            wallpaper.isOperationAllowed(function (err, data) {
                if (err) {
                    console.info(`isOperationAllowedCallbackTest001 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`isOperationAllowedCallbackTest001 data : ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      isOperationAllowedPromiseTest002
     * @tc.desc:      Test isOperationAllowed() to checks whether a user is allowed to set wallpapers.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isOperationAllowedPromiseTest002', 0, async function (done) {
        try {
            wallpaper.isOperationAllowed().then((data) => {
                console.info(`isOperationAllowedPromiseTest002 data : ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`isOperationAllowedPromiseTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      isChangePermittedCallbackTest001
     * @tc.desc:      Test isChangePermitted() to checks whether to allow the application to change the
     *                    wallpaper for the current user.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isChangePermittedCallbackTest001', 0, async function (done) {
        try {
            wallpaper.isChangePermitted(function (err, data) {
                if (err) {
                    console.info(`isChangePermittedCallbackTest001 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`isChangePermittedCallbackTest001 data : ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      isChangePermittedPromiseTest002
     * @tc.desc:      Test isChangePermitted() to checks whether to allow the application to change the
     *                    wallpaper for the current user.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isChangePermittedPromiseTest002', 0, async function (done) {
        try {
            wallpaper.isChangePermitted().then((data) => {
                console.info(`isChangePermittedPromiseTest002 data : ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`isChangePermittedPromiseTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getMinWidthCallbackTest001
     * @tc.desc:      Test getMinWidth() to gets the minWidth of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinWidthCallbackTest001', 0, async function (done) {
        try {
            wallpaper.getMinWidth(function (err, data) {
                if (err) {
                    console.info(`getMinWidthCallbackTest001 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getMinWidthCallbackTest001 data : ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getMinWidthPromiseTest002
     * @tc.desc:      Test getMinWidth() to gets the minWidth of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinWidthPromiseTest002', 0, async function (done) {
        try {
            wallpaper.getMinWidth().then((data) => {
                console.info(`getMinWidthPromiseTest002 data : ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getMinWidthPromiseTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getMinHeightCallbackTest001
     * @tc.desc:      Test getMinHeight() to gets the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinHeightCallbackTest001', 0, async function (done) {
        try {
            wallpaper.getMinHeight(function (err, data) {
                if (err) {
                    console.info(`getMinHeightCallbackTest001 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getMinHeightCallbackTest001 data : ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getMinHeightPromiseTest002
     * @tc.desc:      Test getMinHeight() to gets the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinHeightPromiseTest002', 0, async function (done) {
        try {
            wallpaper.getMinHeight().then((data) => {
                console.info(`getMinHeightPromiseTest002 data : ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getMinHeightPromiseTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getFileCallbackTest001
     * @tc.desc:      Test getFile() to gets the File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileCallbackTest001', 0, async function (done) {
        try {
            wallpaper.getFile(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info(`getFileCallbackTest001 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getFileCallbackTest001 data : ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getFilePromiseTest002
     * @tc.desc:      Test getFile() to get the File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFilePromiseTest002', 0, async function (done) {
        try {
            wallpaper.getFile(WALLPAPER_SYSTEM).then((data) => {
                console.info(`getFilePromiseTest002 data : ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getFilePromiseTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getFileCallbackTest003
     * @tc.desc:      Test getFile() to gets the File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileCallbackTest003', 0, async function (done) {
        try {
            wallpaper.getFile(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info(`getFileCallbackTest003 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getFileCallbackTest003 data : ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getFilePromiseTest004
     * @tc.desc:      Test getFile() to gets the File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFilePromiseTest004', 0, async function (done) {
        try {
            wallpaper.getFile(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info(`getFilePromiseTest004 data : ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getFilePromiseTest004 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getIdCallbackTest001
     * @tc.desc:      Test getId() to gets the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdCallbackTest001', 0, async function (done) {
        try {
            wallpaper.getId(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info(`getIdCallbackTest001 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getIdCallbackTest001 data ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getIdPromiseTest002
     * @tc.desc:      Test getId() to gets the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdPromiseTest002', 0, async function (done) {
        try {
            wallpaper.getId(WALLPAPER_SYSTEM).then((data) => {
                console.info(`getIdPromiseTest002 data ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getIdPromiseTest002 err ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getIdCallbackTest003
     * @tc.desc:      Test getId() to gets the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdCallbackTest003', 0, async function (done) {
        try {
            wallpaper.getId(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info(`getIdCallbackTest003 err ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getIdCallbackTest003 data ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getIdPromiseTest004
     * @tc.desc:      Test getId() to gets the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdPromiseTest004', 0, async function (done) {
        try {
            wallpaper.getId(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info(`getIdCallbackTest003 data ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getIdCallbackTest003 err ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getColorsCallbackTest001
     * @tc.desc:      Test getColors() to gets WALLPAPER_SYSTEM Colors.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getColorsCallbackTest001', 0, async function (done) {
        try {
            wallpaper.getColors(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info(`getColorsCallbackTest001 err ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getColorsCallbackTest001 data ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getColorsPromiseTest002
     * @tc.desc:      Test getColors() to gets WALLPAPER_SYSTEM Colors.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getColorsPromiseTest002', 0, async function (done) {
        try {
            wallpaper.getColors(WALLPAPER_SYSTEM).then((data) => {
                console.info(`getColorsPromiseTest002 data ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getColorsPromiseTest002 err ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getColorsCallbackTest003
     * @tc.desc:      Test getColors() to gets WALLPAPER_LOCKSCREEN Colors.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getColorsCallbackTest003', 0, async function (done) {
        try {
            wallpaper.getColors(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info(`getColorsCallbackTest003 err ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getColorsCallbackTest003 data ${data}`);
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getColorsPromiseTest004
     * @tc.desc:      Test getColors() to gets WALLPAPER_LOCKSCREEN Colors.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getColorsPromiseTest004', 0, async function (done) {
        try {
            wallpaper.getColors(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info(`getColorsPromiseTest004 data ${data}`);
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`getColorsPromiseTest004 err ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      onCallbackTest001
     * @tc.desc:      Test on_colorChange to registers a listener for wallpaper color changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('onCallbackTest001', 0, async function (done) {
        await wallpaper.restore(WALLPAPER_LOCKSCREEN);
        try {
            wallpaper.on('colorChange', (colors, wallpaperType) => {
                console.info(`onCallbackTest001 colors : ${colors}`);
                expect(colors != null).assertTrue();
                expect(wallpaperType != null).assertTrue();
                wallpaper.off('colorChange');
                done();
            })
        } catch (error) {
            console.info(`onCallbackTest001 error : ${error.message}`);
            expect(null).assertFail();
            done();
        }
        await wallpaper.setImage(URI, WALLPAPER_LOCKSCREEN);
        await wallpaper.restore(WALLPAPER_LOCKSCREEN);
    })

    /**
     * @tc.name:      onCallbackTest002
     * @tc.desc:      Test on_wallpaperChange to registers a listener for wallpaper changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('onCallbackTest002', 0, async function (done) {
        await wallpaper.restore(WALLPAPER_SYSTEM);
        try {
            wallpaper.on('wallpaperChange', (wallpaperType, resourceType) => {
                expect(wallpaperType != null).assertTrue();
                expect(resourceType != null).assertTrue();
                wallpaper.off('wallpaperChange');
                done();
            })
        } catch (error) {
            console.info(`onCallbackTest002 error : ${error.message}`);
            expect(null).assertFail();
            done();
        }
        await wallpaper.setImage(URI, WALLPAPER_SYSTEM);
        await wallpaper.restore(WALLPAPER_SYSTEM);
    })

    /**
     * @tc.name:      onCallbackTest003
     * @tc.desc:      Test to register not exist event
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('onCallbackTest003', 0, async function (done) {
        await wallpaper.restore(WALLPAPER_SYSTEM);
        try {
            wallpaper.on('wallpaperChangeX', (wallpaperType, resourceType) => {
                expect(null).assertFail();
                done();
            })
        } catch (error) {
            console.info(`onCallbackTest003 error : ${error.message}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            done();
        }
        await wallpaper.setImage(URI, WALLPAPER_SYSTEM);
        await wallpaper.restore(WALLPAPER_SYSTEM);
    })

    /**
     * @tc.name:      onCallbackTest004
     * @tc.desc:      Test on_wallpaperChange to registers a listener for wallpaper changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('onCallbackTest004', 0, async function (done) {
        await wallpaper.restore(WALLPAPER_SYSTEM);
        try {
            wallpaper.on('wallpaperChange', async (wallpaperType, resourceType, uri) => {
                expect(wallpaperType != null).assertTrue();
                expect(resourceType != null).assertTrue();
                expect(uri !== "").assertTrue();
                wallpaper.off('wallpaperChange');
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            })
            if (isBundleNameExists()) {
                await wallpaper.setCustomWallpaper(URI, WALLPAPER_SYSTEM);
            } else {
                wallpaper.off('wallpaperChange');
                expect(true).assertTrue();
                done();
            }
        } catch (error) {
            console.info(`onCallbackTest004 error : ${error.message}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      offCallbackTest001
     * @tc.desc:      Test off_colorChange to log off a listener for wallpaper color changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest001', 0, async function (done) {
        let callbackTimes = 0;
        await wallpaper.setImage(URI, WALLPAPER_SYSTEM);
        try {
            wallpaper.on('colorChange', async (colors, wallpaperType) => {
                console.info(`offCallbackTest001 colors : ${colors}`);
                callbackTimes = callbackTimes + 1;
                wallpaper.off('colorChange');
                await wallpaper.setImage(URI, WALLPAPER_SYSTEM);
                await wallpaper.restore(WALLPAPER_SYSTEM);
                expect(callbackTimes === 1).assertTrue();
                done();
            })
        } catch (error) {
            console.info(`offCallbackTest001 error : ${error}`);
            expect(null).assertFail();
            done();
        }
        await wallpaper.restore(WALLPAPER_SYSTEM);
    })

    /**
     * @tc.name:      offCallbackTest002
     * @tc.desc:      Test wallpaperChange to log off a listener for wallpaper changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest002', 0, async function (done) {
        let callbackTimes = 0;
        await wallpaper.setImage(URI, WALLPAPER_SYSTEM);
        try {
            wallpaper.on('wallpaperChange', async (wallpaperType, resourceType) => {
                expect(wallpaperType != null).assertTrue();
                expect(resourceType != null).assertTrue();
                callbackTimes = callbackTimes + 1;
                wallpaper.off('wallpaperChange', async (wallpaperType, resourceType) => {
                })
                await wallpaper.setImage(URI, WALLPAPER_SYSTEM);
                await wallpaper.restore(WALLPAPER_SYSTEM);
                expect(callbackTimes === 1).assertTrue();
                done();
            })
        } catch (error) {
            console.info(`offCallbackTest002 error : ${error.message}`);
            expect(null).assertFail();
            done();
        }
        await wallpaper.restore(WALLPAPER_SYSTEM);
    })

    /**
     * @tc.name:      offCallbackTest003
     * @tc.desc:      Test wallpaperChange to log off a listener for wallpaper changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest003', 0, async function (done) {
        try {
            wallpaper.off('wallpaperChange', async (wallpaperType, resourceType) => {
            }, 'other');
            expect(true).assertTrue();
            done();
        } catch (error) {
            console.info(`offCallbackTest003 error : ${error.message}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      offCallbackTest004
     * @tc.desc:      Test wallpaperChange to log off a listener for wallpaper changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest004', 0, async function (done) {
        try {
            wallpaper.off('wallpaperChange');
            expect(true).assertTrue();
            done();
        } catch (error) {
            console.info(`offCallbackTest004 error : ${error.message}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      offCallbackTest005
     * @tc.desc:      Test wallpaperChange to log off a listener for wallpaper changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest005', 0, async function (done) {
        try {
            wallpaper.off('wallpaperChange', 'other');
            expect(null).assertFail();
            done();
        } catch (error) {
            console.info(`offCallbackTest005 error : ${error.message}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            done();
        }
    })

    /**
     * @tc.name:      offCallbackTest006
     * @tc.desc:      Test not exist event wallpaperChangeX to off
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest006', 0, async function (done) {
        await wallpaper.setImage(URI, WALLPAPER_SYSTEM);
        try {
            wallpaper.off('wallpaperChangeX');
            expect(null).assertFail();
            done();
        } catch (error) {
            console.info(`offCallbackTest006 error : ${error.message}`);
            expect(error.code === PARAMETER_ERROR).assertTrue();
            done();
        }
        await wallpaper.restore(WALLPAPER_SYSTEM);
    })

    /**
     * @tc.name:      setVideoTest001
     * @tc.desc:      Test setVideo to set live wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI6R07J
     */
    it('setVideoTest001', 0, async function (done) {
        try {
            wallpaper.setVideo(URI_30FPS_3S_MP4, WALLPAPER_SYSTEM, (error) => {
                if (error != undefined) {
                    console.info(`setVideoTest001 error : ${error}`);
                    expect(null).assertFail();
                } else {
                    expect(true).assertTrue();
                    wallpaper.reset(WALLPAPER_SYSTEM);
                }
                done();
            })
        } catch (error) {
            console.info(`setVideoTest001 error : ${error}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setVideoTest002
     * @tc.desc:      Test setVideo to set live wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI6R07J
     */
    it('setVideoTest002', 0, async function (done) {
        try {
            wallpaper.setVideo(URI_30FPS_3S_MOV, WALLPAPER_SYSTEM, (error) => {
                if (error != undefined) {
                    console.info(`setVideoTest002 error : ${error}`);
                    expect(true).assertTrue();
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            console.info(`setVideoTest002 error : ${error}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setVideoTest003
     * @tc.desc:      Test setVideo to set live wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI6R07J
     */
    it('setVideoTest003', 0, async function (done) {
        try {
            wallpaper.setVideo(URI_15FPS_7S_MP4, WALLPAPER_SYSTEM, (error) => {
                if (error != undefined) {
                    console.info(`setVideoTest002 error : ${error}`);
                    expect(true).assertTrue();
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            console.info(`setVideoTest003 error : ${error}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setVideoTest004
     * @tc.desc:      Test setVideo to set live wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI6R07J
     */
    it('setVideoTest004', 0, async function (done) {
        try {
            wallpaper.setVideo(URI_30FPS_3S_MP4, WALLPAPER_LOCKSCREEN, (error) => {
                if (error != undefined) {
                    console.info(`setVideoTest004 error : ${error}`);
                    expect(null).assertFail();
                } else {
                    expect(true).assertTrue();
                    wallpaper.reset(WALLPAPER_LOCKSCREEN);
                }
                done();
            })
        } catch (error) {
            console.info(`setVideoTest004 error : ${error}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setVideoTest005
     * @tc.desc:      Test setVideo to set live wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI6R07J
     */
    it('setVideoTest005', 0, async function (done) {
        try {
            wallpaper.setVideo(URI_30FPS_3S_MOV, WALLPAPER_LOCKSCREEN, (error) => {
                if (error != undefined) {
                    console.info(`setVideoTest005 error : ${error}`);
                    expect(true).assertTrue();
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            console.info(`setVideoTest005 error : ${error}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setVideoTest006
     * @tc.desc:      Test setVideo to set live wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI6R07J
     */
    it('setVideoTest006', 0, async function (done) {
        try {
            wallpaper.setVideo(URI_15FPS_7S_MP4, WALLPAPER_LOCKSCREEN, (error) => {
                if (error != undefined) {
                    console.info(`setVideoTest006 error : ${error}`);
                    expect(true).assertTrue();
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            console.info(`setVideoTest006 error : ${error}`);
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperCallbackTest001
     * @tc.desc:      Test setCustomWallpaper to set a custom system wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomWallpaperTest001', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper(URI_ZIP, WALLPAPER_SYSTEM, (error) => {
                if (isBundleNameExists()) {
                    if (error !== undefined) {
                        expect(null).assertFail();
                        console.info(`set CustomWallpaperTest001 fail : ${error}`);
                    } else {
                        expect(true).assertTrue();
                        wallpaper.reset(WALLPAPER_SYSTEM);
                    }
                } else {
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Custom WallpaperTest001 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperPromiseTest002
     * @tc.desc:      Test setCustomWallpaper to sets a custom system wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomWallpaperPromiseTest002', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper(URI_ZIP, WALLPAPER_SYSTEM).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            }).catch((err) => {
                if (isBundleNameExists()) {
                    expect(null).assertFail();
                    console.info(`set Custom WallpaperPromiseTest002 fail : ${err}`);
                    done();
                } else {
                    expect(true).assertTrue();
                    done();
                }
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Custom WallpaperPromiseTest002 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperCallbackTest003
     * @tc.desc:      Test setCustomWallpaper to sets a custom lockscreen wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomWallpaperCallbackTest003', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper(URI_ZIP, WALLPAPER_LOCKSCREEN, (error) => {
                if (isBundleNameExists()) {
                    if (error !== undefined) {
                        expect(null).assertFail();
                        console.info(`set Custom WallpaperCallbackTest003 fail : ${error}`);
                    } else {
                        expect(true).assertTrue();
                        wallpaper.reset(WALLPAPER_SYSTEM);
                    }
                } else {
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Custom WallpaperCallbackTest003 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperPromiseTest004
     * @tc.desc:      Test setCustomWallpaper to sets a custom lockscreen wallpaper.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomWallpaperPromiseTest004', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper(URI_ZIP, WALLPAPER_LOCKSCREEN).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            }).catch((err) => {
                if (isBundleNameExists()) {
                    expect(null).assertFail();
                    console.info(`set Custom WallpaperPromiseTest004 fail : ${err}`);
                    done();
                } else {
                    expect(true).assertTrue();
                    done();
                }
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Custom WallpaperPromiseTest004 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperCallbackThrowErrorTest005
     * @tc.desc:      Test setCustomWallpaper throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomCallbackThrowErrorTest005', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper(URI_ZIP, INVALID_WALLPAPER_TYPE, function (err) {
                if (err) {
                    expect(err.code === PARAMETER_ERROR).assertTrue()
                    console.info(`set Custom CallbackThrowErrorTest005 fail : ${err}`);
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
            console.info(`set Custom CallbackThrowErrorTest005 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperCallbackThrowErrorTest006
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomWallpaperCallbackThrowErrorTest006', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper(URI_ZIP, function (err) {
                if (err) {
                    console.info(`setCustomWallpaperCallbackThrowErrorTest006 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperPromiseThrowErrorTest007
     * @tc.desc:      Test setCustomWallpaper throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomWallpaperPromiseThrowErrorTest007', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper(URI_ZIP, INVALID_WALLPAPER_TYPE).then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`setCustomWallpaperPromiseThrowErrorTest007 err : ${err}`);
                expect(err.code === PARAMETER_ERROR).assertTrue()
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setCustomWallpaperPromiseThrowErrorTest008
     * @tc.desc:      Test setCustomWallpaper throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI7AAMU
     */
    it('setCustomWallpaperPromiseThrowErrorTest008', 0, async function (done) {
        try {
            wallpaper.setCustomWallpaper().then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`setCustomWallpaperPromiseThrowErrorTest008 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      setAllWallpapersThrowErrorTest001
     * @tc.desc:      Test setAllWallpapers throw parameter error, parameter count error
     * @tc.type:      FUNC test
     * @tc.require:
     */
    it('setAllWallpapersThrowErrorTest001', 0, async function (done) {
        try {
            wallpaper.setAllWallpapers(wallpaper.WallpaperType.WALLPAPER_SYSTEM).then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`setAllWallpapersThrowErrorTest001 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            console.info(`setAllWallpapersThrowErrorTest001 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      setAllWallpapersThrowErrorTest002
     * @tc.desc:      Test setAllWallpapers throw parameter error, wallpaperType range error
     * @tc.type:      FUNC test
     * @tc.require:
     */
    it('setAllWallpapersThrowErrorTest002', 0, async function (done) {
        const wallpaperInfo1 = {
            foldState: wallpaper.FoldState.NORMAL,
            rotateState: wallpaper.RotateState.PORT,
            source: URI
        }
        let wallpaperInfos = [wallpaperInfo1];
        try {
            wallpaper.setAllWallpapers(wallpaperInfos, 2).then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`setAllWallpapersThrowErrorTest002 err : ${err}`);
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            console.info(`setAllWallpapersThrowErrorTest002 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      setAllWallpapersThrowErrorTest003
     * @tc.desc:      Test setAllWallpapers throw parameter error, wallpaperInfo source error
     * @tc.type:      FUNC test
     * @tc.require:
     */
    it('setAllWallpapersThrowErrorTest003', 0, async function (done) {
        const wallpaperInfo1 = {
            foldState: wallpaper.FoldState.NORMAL,
            rotateState: wallpaper.RotateState.PORT,
            source: URI_ZIP
        }
        let wallpaperInfos = [wallpaperInfo1];
        try {
            wallpaper.setAllWallpapers(wallpaperInfos, wallpaper.WallpaperType.WALLPAPER_SYSTEM).then(() => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                console.info(`setAllWallpapersThrowErrorTest003 err : ${err}`);
                expect(err.code === PARAMETER_ERROR).assertTrue()
                done();
            });
        } catch (error) {
            console.info(`setAllWallpapersThrowErrorTest003 error : ${error}`);
            expect(error.code === PARAMETER_ERROR).assertTrue()
            done();
        }
    })

    /**
     * @tc.name:      setAllWallpapersTest001
     * @tc.desc:      Test setAllWallpapers.
     * @tc.type:      FUNC test
     * @tc.require:
     */
    it('setAllWallpapersTest001', 0, async function (done) {
        const wallpaperInfo1 = {
            foldState: wallpaper.FoldState.NORMAL,
            rotateState: wallpaper.RotateState.PORT,
            source: URI
        }
        let wallpaperInfos = [wallpaperInfo1];
        try {
            wallpaper.setAllWallpapers(wallpaperInfos, wallpaper.WallpaperType.WALLPAPER_SYSTEM).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_SYSTEM);
            }).catch((err) => {
                console.info(`setAllWallpapersTest001 fail : ${err}`);
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`setAllWallpapersTest001 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setAllWallpapersTest002
     * @tc.desc:      Test setAllWallpapers.
     * @tc.type:      FUNC test
     * @tc.require:
     */
    it('setAllWallpapersTest002', 0, async function (done) {
        const wallpaperInfo1 = {
            foldState: wallpaper.FoldState.NORMAL,
            rotateState: wallpaper.RotateState.PORT,
            source: URI
        }
        const wallpaperInfo2 = {
            foldState: wallpaper.FoldState.UNFOLD_1,
            rotateState: wallpaper.RotateState.LAND,
            source: URI
        }
        let wallpaperInfos = [wallpaperInfo1, wallpaperInfo2];
        try {
            wallpaper.setAllWallpapers(wallpaperInfos, wallpaper.WallpaperType.WALLPAPER_LOCKSCREEN).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            }).catch((err) => {
                console.info(`setAllWallpapersTest002 fail : ${err}`);
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`setAllWallpapersTest002 fail : ${error}`);
            done();
        }
    })

    /**
     * @tc.name:      setAllWallpapersTest003
     * @tc.desc:      Test setAllWallpapers.
     * @tc.type:      FUNC test
     * @tc.require:
     */
    it('setAllWallpapersTest003', 0, async function (done) {
        const wallpaperInfo1 = {
            foldState: wallpaper.FoldState.NORMAL,
            rotateState: wallpaper.RotateState.PORT,
            source: URI
        }
        const wallpaperInfo2 = {
            foldState: wallpaper.FoldState.UNFOLD_1,
            rotateState: wallpaper.RotateState.LAND,
            source: URI
        }
        const wallpaperInfo3 = {
            foldState: wallpaper.FoldState.UNFOLD_2,
            rotateState: wallpaper.RotateState.PORT,
            source: URI
        }
        const wallpaperInfo4 = {
            foldState: wallpaper.FoldState.UNFOLD_2,
            rotateState: wallpaper.RotateState.LAND,
            source: URI
        }
        let wallpaperInfos = [wallpaperInfo1, wallpaperInfo2, wallpaperInfo3, wallpaperInfo4];
        try {
            wallpaper.setAllWallpapers(wallpaperInfos, wallpaper.WallpaperType.WALLPAPER_LOCKSCREEN).then(async () => {
                expect(true).assertTrue();
                done();
                await wallpaper.restore(WALLPAPER_LOCKSCREEN);
            }).catch((err) => {
                console.info(`setAllWallpapersTest003 fail : ${err}`);
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            console.info(`setAllWallpapersTest003 fail : ${error}`);
            done();
        }
    })
})
