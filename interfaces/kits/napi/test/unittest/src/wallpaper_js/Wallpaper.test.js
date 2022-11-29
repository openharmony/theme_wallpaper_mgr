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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

const WALLPAPER_SYSTEM = 0;
const WALLPAPER_LOCKSCREEN = 1;
const PARAMETER_ERROR = "401";
const URL = "/data/storage/el2/base/haps/wp.jpeg";

describe('WallpaperJSTest', function () {
    beforeAll(async function () {
        // input testsuit setup step，setup invoked before all testcases
        console.info('beforeAll called')
        CreateImageToUrl();
    })
    beforeEach(function () {
        // input testcase setup step，setup invoked before each testcases
        console.info('beforeEach called')
    })
    afterEach(function () {
        // input testcase teardown step，teardown invoked after each testcases
        console.info('afterEach called')
    })
    afterAll(function () {
        // input testsuit teardown step，teardown invoked after all testcases
        console.info('afterAll called')
    })

    function CreateImageToUrl()
    {
        const color = new ArrayBuffer(96);
        let opts = {editable: true, pixelFormat: 3, size: {height: 4, width: 6}};
        image.createPixelMap(color, opts).then((pixelMap) => {
            const imagePackerApi = image.createImagePacker();
            let packOpts = {format: "image/jpeg", quality: 98};
            imagePackerApi.packing(
                pixelMap, packOpts, (err, data) => {
                    let fd = fileio.openSync(URL, 0o2 | 0o100, 0o666);
                    let ret = fileio.writeSync(fd, data);
                    fileio.close(fd);
                    console.log("file write ret:" + JSON.stringify(ret));
                }
            )
        })
    }

    /**
     * @tc.name:      getColorsSyncTest001
     * @tc.desc:      Test getColorsSync() WALLPAPER_SYSTEM Colors by syncing.
     * @tc.type:      FUNC
     * @tc.require:   issueI5UHRG
     */
    it('getColorsSyncTest001', 0, function () {
        try {
            let data = wallpaper.getColorsSync(WALLPAPER_SYSTEM);
            console.info('getColorsSyncTest001 data : ' + JSON.stringify(data));
            if (data != undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('getColorsSyncTest001 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(null).assertFail();
        }
    })


    /**
     * @tc.name:      getColorsSyncTest002
     * @tc.desc:      Test getColorsSync() WALLPAPER_LOCKSCREEN Colors by syncing.
     * @tc.type:      FUNC
     * @tc.require:   issueI5UHRG
     */
    it('getColorsSyncTest002', 0, function () {
        try {
            let data = wallpaper.getColorsSync(WALLPAPER_LOCKSCREEN);
            console.info('getColorsSyncTest002 data : ' + JSON.stringify(data));
            if (data != undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('getColorsSyncTest002 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
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
            let data = wallpaper.getColorsSync(2);
            console.info('getColorsSyncTest003 data : ' + JSON.stringify(data));
            expect(null).assertFail();
        } catch (error) {
            console.info('getColorsSyncTest003 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
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
            console.info('getColorsSyncTest004 data : ' + JSON.stringify(data));
            expect(null).assertFail();
        } catch (error) {
            console.info('getColorsSyncTest004 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getIdSyncTest001
     * @tc.desc:      Test getIdSync() to the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest001', 0, function () {
        try {
            let data = wallpaper.getIdSync(WALLPAPER_SYSTEM);
            console.info('getIdSyncTest001 data : ' + JSON.stringify(data));
            if (data != undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('getIdSyncTest001 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      getIdSyncTest002
     * @tc.desc:      Test getIdSync() to the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest002', 0, function () {
        try {
            let data = wallpaper.getIdSync(WALLPAPER_LOCKSCREEN);
            console.info('getIdSyncTest002 data : ' + JSON.stringify(data));
            if (data != undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('getIdSyncTest002 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            console.info('error.code typeof : ' + typeof (error.code));
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      getIdSyncTest003
     * @tc.desc:      Test getIdSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest003', 0, function () {
        try {
            let data = wallpaper.getIdSync(3);
            console.info('getIdSyncTest003 data : ' + JSON.stringify(data));
            expect(null).assertFail();
        } catch (error) {
            console.info('getIdSyncTest003 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getIdSyncTest004
     * @tc.desc:      Test getIdSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest004', 0, function () {
        try {
            let data = wallpaper.getIdSync();
            console.info('getIdSyncTest004 data : ' + JSON.stringify(data));
            expect(null).assertFail();
        } catch (error) {
            console.info('getIdSyncTest004 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getFileSyncTest001
     * @tc.desc:      Test getFileSync() to File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest001', 0, function () {
        try {
            let data = wallpaper.getFileSync(WALLPAPER_SYSTEM);
            expect(typeof data == "number").assertTrue();
            expect(!isNaN(data)).assertTrue();
            console.info("getFileSyncTest001 success to getFile: " + JSON.stringify(data));
        } catch (error) {
            console.error("getFileSyncTest001 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getFileSyncTest002
     * @tc.desc:      Test getFileSync() to the File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest002', 0, function () {
        try {
            let data = wallpaper.getFileSync(WALLPAPER_LOCKSCREEN);
            expect(typeof data == "number").assertTrue();
            expect(!isNaN(data)).assertTrue();
            console.info("getFileSyncTest002 success to getFile: " + JSON.stringify(data));
        } catch (error) {
            console.error("getFileSyncTest002 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getFileSyncTest003
     * @tc.desc:      Test getFileSync() after getFileSync() to File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI60MT1
     */
    it('getFileSyncTest003', 0, function () {
        try {
            let fd1 = wallpaper.getFileSync(WALLPAPER_SYSTEM);
            expect(typeof fd1 == "number").assertTrue();
            expect(!isNaN(fd1)).assertTrue();
            console.info("getFileSyncTest003 success to getFile: " + JSON.stringify(fd1));
            let fd2 = wallpaper.getFileSync(WALLPAPER_SYSTEM);
            expect(typeof fd2 == "number").assertTrue();
            expect(!isNaN(fd2)).assertTrue();
            console.info("getFileSyncTest003 success to getFile: " + JSON.stringify(fd2));
        } catch (error) {
            console.error("getFileSyncTest003 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getFileSyncTest004
     * @tc.desc:      Test getFileSync() after getFileSync() to the File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI60MT1
     */
    it('getFileSyncTest004', 0, function () {
        try {
            let fd1 = wallpaper.getFileSync(WALLPAPER_LOCKSCREEN);
            expect(typeof fd1 == "number").assertTrue();
            expect(!isNaN(fd1)).assertTrue();
            console.info("getFileSyncTest004 success to getFile: " + JSON.stringify(fd1));
            let fd2 = wallpaper.getFileSync(WALLPAPER_LOCKSCREEN);
            expect(typeof fd2 == "number").assertTrue();
            expect(!isNaN(fd2)).assertTrue();
            console.info("getFileSyncTest004 success to getFile: " + JSON.stringify(fd2));
        } catch (error) {
            console.error("getFileSyncTest004 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getFileSyncTest005
     * @tc.desc:      Test getFileSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest005', 0, function () {
        try {
            let data = wallpaper.getFileSync(3);
            console.info('getFileSyncTest005 data : ' + JSON.stringify(data));
            expect(null).assertFail()
        } catch (error) {
            console.info('getFileSyncTest005 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getFileSyncTest006
     * @tc.desc:      Test getFileSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest006', 0, function () {
        try {
            let data = wallpaper.getFileSync();
            console.info('getFileSyncTest006 data : ' + JSON.stringify(data));
            expect(null).assertFail()
        } catch (error) {
            console.info('getFileSyncTest006 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getMinHeightSyncTest001
     * @tc.desc:      Test getMinHeightSync() to the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinHeightSyncTest001', 0, function () {
        let data = wallpaper.getMinHeightSync();
        console.info('getMinHeightSyncTest001 data : ' + JSON.stringify(data));
        if (data != undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getMinWidthSyncTest001
     * @tc.desc:      Test getMinWidthSync() to the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinWidthSyncTest001', 0, function () {
        let data = wallpaper.getMinWidthSync();
        console.info('getMinWidthSyncTest001 data : ' + JSON.stringify(data));
        if (data != undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      isChangeAllowedTest001
     * @tc.desc:      Test isChangeAllowed() to checks whether to allow the application to change the
     *                    wallpaper for the current user.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isChangeAllowedTest001', 0, function () {
        let data = wallpaper.isChangeAllowed();
        console.info('isChangeAllowedTest001 data : ' + JSON.stringify(data));
        if (data != undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      isUserChangeAllowedTest001
     * @tc.desc:      Test isUserChangeAllowed() to checks whether a user is allowed to set wallpapers.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isUserChangeAllowedTest001', 0, function () {
        let data = wallpaper.isUserChangeAllowed();
        console.info('isUserChangeAllowedTest001 data : ' + JSON.stringify(data));
        if (data != undefined) {
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
            wallpaper.restore(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('restoreCallbackSystemTest001 err : ' + JSON.stringify(err));
                    expect(null).assertFail()
                } else {
                    console.info('restoreCallbackSystemTest001 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            })

        } catch (error) {
            console.info('restoreCallbackSystemTest001 error : ' + JSON.stringify(error));
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
            wallpaper.restore(WALLPAPER_SYSTEM).then((data) => {
                console.info('restorePromiseSystemTest002 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('restorePromiseSystemTest002 err : ' + JSON.stringify(err));
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
            wallpaper.restore(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('restoreCallbackLockTest003 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('restoreCallbackLockTest003 data : ' + JSON.stringify(data));
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
     * @tc.name:      restorePromiseLockTest004
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseLockTest004', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('restorePromiseLockTest004 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('restorePromiseLockTest004 err : ' + JSON.stringify(err));
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
            wallpaper.restore(2, function (err, data) {
                if (err) {
                    console.info('restoreCallbackThrowErrorTest005 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('restoreCallbackThrowErrorTest005 data : ' + JSON.stringify(data));
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
        }
        done();
    })

    /**
     * @tc.name:      restoreCallbackThrowErrorTest006
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackThrowErrorTest006', 0, async function (done) {
        try {
            wallpaper.restore(function (err, data) {
                if (err) {
                    console.info('restoreCallbackThrowErrorTest006 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('restoreCallbackThrowErrorTest006 data : ' + JSON.stringify(data));
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
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
            wallpaper.restore(2).then((data) => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                expect(err.code == PARAMETER_ERROR).assertEqual(true)
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
            wallpaper.restore().then((data) => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
            done();
        }
    })

    /**
     * @tc.name:      getImagePromiseLockTest001
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseLockTest001', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('getImagePromiseLockTest001 data : ' + JSON.stringify(data));
                if (data != undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info('getImagePromiseLockTest001 err : ' + JSON.stringify(err));
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
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackSystemTest002', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('getImageCallbackSystemTest002 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('getImageCallbackSystemTest002 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getImagePromiseSystemTest003
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseSystemTest003', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM).then((data) => {
                console.info('getImagePromiseSystemTest003 data : ' + JSON.stringify(data));
                if (data != undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info('getImagePromiseSystemTest003 err : ' + JSON.stringify(err));
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
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackLockTest004', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('getImageCallbackLockTest004 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('getImageCallbackLockTest004 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(null).assertFail();
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
            wallpaper.getImage(2, function (err, data) {
                if (err) {
                    console.info('getImageCallbackThrowErrorTest005 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('getImageCallbackThrowErrorTest005 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(null).assertFail();
                    }
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
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
                    console.info('getImageCallbackThrowErrorTest006 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('getImageCallbackThrowErrorTest006 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(null).assertFail();
                    }
                }
                done();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
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
            wallpaper.getImage(2).then((data) => {
                if (data != undefined) {
                    expect(null).assertFail();
                }
                done();
            }).catch((err) => {
                expect(err.code == PARAMETER_ERROR).assertEqual(true)
                done();
            });
        } catch (error) {
            expect(null).assertFail();
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
                if (data != undefined) {
                    expect(null).assertFail();
                }
                done();
            }).catch((err) => {
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
            done();
        }
    })

    /**
     * @tc.name:      setImageURLPromiseLockTest001
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLPromiseLockTest001', 0, async function (done) {
        try {
            wallpaper.setImage(URL, WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('setImageURLPromiseLockTest001 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('setImageURLPromiseLockTest001 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageURLCallbackSystemTest002
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLCallbackSystemTest002', 0, async function (done) {
        try {
            wallpaper.setImage(URL, WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('setImageURLCallbackSystemTest002 err : ' + JSON.stringify(err.message));
                    expect(null).assertFail();
                } else {
                    console.info('setImageURLCallbackSystemTest002 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageURLPromiseSystemTest003
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLPromiseSystemTest003', 0, async function (done) {
        try {
            wallpaper.setImage(URL, WALLPAPER_SYSTEM).then((data) => {
                console.info('setImageURLPromiseSystemTest003 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('setImageURLPromiseSystemTest003 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageURLCallbackLockTest004
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLCallbackLockTest004', 0, async function (done) {
        try {
            wallpaper.setImage(URL, WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('setImageURLCallbackLockTest004 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('setImageURLCallbackLockTest004 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            });
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
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN).then((data) => {
                    console.info('setImageMapPromiseLockTest005 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                    done();
                }).catch((err) => {
                    console.info('setImageMapPromiseLockTest005 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                    done();
                });
            })
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
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM, function (err, data) {
                    if (err) {
                        console.info('setImageMapCallbackSystemTest006 err : ' + JSON.stringify(err));
                        expect(null).assertFail();
                    } else {
                        console.info('setImageMapCallbackSystemTest006 data : ' + JSON.stringify(data));
                        expect(true).assertTrue();
                    }
                    done();
                });
            })
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageMapPromiseSystemTest007
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapPromiseSystemTest007', 0, function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM).then((data) => {
                    console.info('setImageMapPromiseSystemTest007 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                    done();
                }).catch((err) => {
                    console.info('setImageMapPromiseSystemTest007 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                    done();
                });
            })
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
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN, function (err, data) {
                    if (err) {
                        console.info('setImageMapCallbackLockTest008 err : ' + JSON.stringify(err));
                        expect(null).assertFail();
                    } else {
                        console.info('setImageMapCallbackLockTest008 data : ' + JSON.stringify(data));
                        expect(true).assertTrue();
                    }
                    done();
                });
            })
        } catch (error) {
            expect(null).assertFail();
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
            wallpaper.setImage(URL, 2, function (err, data) {
                if (err) {
                    console.info('setImageCallbackThrowErrorTest009 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('setImageCallbackThrowErrorTest009 data : ' + JSON.stringify(data));
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
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
            wallpaper.setImage(URL, function (err, data) {
                if (err) {
                    console.info('setImageCallbackThrowErrorTest010 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('setImageCallbackThrowErrorTest010 data : ' + JSON.stringify(data));
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
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
            wallpaper.setImage(URL, 2).then((data) => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                expect(err.code == PARAMETER_ERROR).assertEqual(true)
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
            wallpaper.setImage().then((data) => {
                expect(null).assertFail();
                done();
            }).catch((err) => {
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
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
    it('onCallbackTest001', 0, function () {
        try {
            wallpaper.on('colorChange', function (colors, wallpaperType) {
                console.info('onCallbackTest001 colors : ' + JSON.stringify(colors));
                console.info('onCallbackTest001 wallpaperType : ' + JSON.stringify(wallpaperType));
                if ((colors != undefined) && (colors.size() != 0) && (wallpaperType != undefined)) {
                    expect(true).assertTrue();
                } else {
                    expect(null).assertFail();
                }
            })
        } catch (error) {
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      onCallbackThrowErrorTest002
     * @tc.desc:      Test on_colorChange throw error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('onCallbackThrowErrorTest002', 0, function () {
        try {
            wallpaper.on(function (colors, wallpaperType) {
                console.info('onCallbackThrowErrorTest002 colors : ' + JSON.stringify(colors));
                console.info('onCallbackThrowErrorTest002 wallpaperType : ' + JSON.stringify(wallpaperType));
                expect(null).assertFail();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      offCallbackTest001
     * @tc.desc:      Test off_colorChange to registers a listener for wallpaper color changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest001', 0, function () {
        try {
            wallpaper.off('colorChange', function (colors, wallpaperType) {
                console.info('offCallbackTest001 colors : ' + JSON.stringify(colors));
                console.info('offCallbackTest001 wallpaperType : ' + JSON.stringify(wallpaperType));
                if ((colors != undefined) && (colors.size() != 0) && (wallpaperType != undefined)) {
                    expect(true).assertTrue();
                } else {
                    expect(null).assertFail();
                }
            })
        } catch (error) {
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      offCallbackThrowErrorTest002
     * @tc.desc:      Test off_colorChange throw error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackThrowErrorTest002', 0, function () {
        try {
            wallpaper.off(function (colors, wallpaperType) {
                console.info('offCallbackThrowErrorTest002 colors : ' + JSON.stringify(colors));
                console.info('offCallbackThrowErrorTest002 wallpaperType : ' + JSON.stringify(wallpaperType));
                expect(null).assertFail();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })
})