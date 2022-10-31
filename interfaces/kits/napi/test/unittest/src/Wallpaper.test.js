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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

const WALLPAPER_SYSTEM = 0;
const WALLPAPER_LOCKSCREEN = 1;
const PARAMETER_ERROR = "401";

let imageSourceSystem = '/system/etc/wallpaper_system.JPG';
let imageSourceLockscreen = 'system/etc/wallpaper_lock.JPG';


describe('WallpaperJSTest', function () {
    beforeAll(async function () {
        // input testsuit setup step，setup invoked before all testcases
        console.info('beforeAll called')
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

    /**
     * @tc.name:      getColorsSyncTest001
     * @tc.desc:      Test getColorsSync() WALLPAPER_SYSTEM Colors by syncing.
     * @tc.type:      FUNC
     * @tc.require:   issueI5UHRG
     */
    it('getColorsSyncTest001', 0, function () {
        try {
            let data = wallpaper.getColorsSync(WALLPAPER_SYSTEM);
            let RgbaColor = {
                red: data[0][0], green: data[0][1], blue: data[0][2], alpha: data[0][3]
            }
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
            let RgbaColor = {
                red: data[0][0], green: data[0][1], blue: data[0][2], alpha: data[0][3]
            }
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
     * @tc.name:      getIdSyncTest005
     * @tc.desc:      Test getIdSync() to the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest005', 0, function () {
        try {
            let data = wallpaper.getIdSync(WALLPAPER_SYSTEM);
            console.info('getIdSyncTest005 data : ' + JSON.stringify(data));
            if (data != undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('getIdSyncTest005 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      getIdSyncTest006
     * @tc.desc:      Test getIdSync() to the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest006', 0, function () {
        try {
            let data = wallpaper.getIdSync(WALLPAPER_LOCKSCREEN);
            console.info('getIdSyncTest006 data : ' + JSON.stringify(data));
            if (data != undefined) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('getIdSyncTest006 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            console.info('error.code typeof : ' + typeof (error.code));
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      getIdSyncTest007
     * @tc.desc:      Test getIdSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest007', 0, function () {
        try {
            let data = wallpaper.getIdSync(3);
            console.info('getIdSyncTest007 data : ' + JSON.stringify(data));
            expect(null).assertFail();
        } catch (error) {
            console.info('getIdSyncTest007 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getIdSyncTest008
     * @tc.desc:      Test getIdSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getIdSyncTest008', 0, function () {
        try {
            let data = wallpaper.getIdSync();
            console.info('getIdSyncTest008 data : ' + JSON.stringify(data));
            expect(null).assertFail();
        } catch (error) {
            console.info('getIdSyncTest008 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getFileSyncTest009
     * @tc.desc:      Test getFileSync() to File of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest009', 0, function () {
        try {
            let data = wallpaper.getFileSync(WALLPAPER_SYSTEM);
            expect(typeof data == "number").assertTrue();
            expect(!isNaN(data)).assertTrue();
            console.info("getFileSyncTest009 success to getFile: " + JSON.stringify(data));
        } catch (error) {
            console.error("getFileSyncTest009 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getFileSyncTest010
     * @tc.desc:      Test getFileSync() to the ID of the wallpaper of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest010', 0, function () {
        try {
            let data = wallpaper.getFileSync(WALLPAPER_LOCKSCREEN);
            expect(typeof data == "number").assertTrue();
            expect(!isNaN(data)).assertTrue();
            console.info("getFileSyncTest010 success to getFile: " + JSON.stringify(data));
        } catch (error) {
            console.error("getFileSyncTest010 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getFileSyncTest011
     * @tc.desc:      Test getFileSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest011', 0, function () {
        try {
            let data = wallpaper.getFileSync(3);
            console.info('getFileSyncTest011 data : ' + JSON.stringify(data));
            expect(null).assertFail()
        } catch (error) {
            console.info('getFileSyncTest011 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getFileSyncTest012
     * @tc.desc:      Test getFileSync() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getFileSyncTest012', 0, function () {
        try {
            let data = wallpaper.getFileSync();
            console.info('getFileSyncTest012 data : ' + JSON.stringify(data));
            expect(null).assertFail()
        } catch (error) {
            console.info('getFileSyncTest012 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      getMinHeightSyncTest013
     * @tc.desc:      Test getMinHeightSync() to the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinHeightSyncTest013', 0, function () {
        let data = wallpaper.getMinHeightSync();
        console.info('getMinHeightSyncTest013 data : ' + JSON.stringify(data));
        if (data != undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      getMinWidthSyncTest014
     * @tc.desc:      Test getMinWidthSync() to the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getMinWidthSyncTest014', 0, function () {
        let data = wallpaper.getMinWidthSync();
        console.info('getMinWidthSyncTest014 data : ' + JSON.stringify(data));
        if (data != undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      isChangeAllowedTest015
     * @tc.desc:      Test isChangeAllowed() to checks whether to allow the application to change the
     *                    wallpaper for the current user.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isChangeAllowedTest015', 0, function () {
        let data = wallpaper.isChangeAllowed();
        console.info('isChangeAllowedTest015 data : ' + JSON.stringify(data));
        if (data != undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      isUserChangeAllowedTest016
     * @tc.desc:      Test isUserChangeAllowed() to checks whether a user is allowed to set wallpapers.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('isUserChangeAllowedTest016', 0, function () {
        let data = wallpaper.isUserChangeAllowed();
        console.info('isUserChangeAllowedTest016 data : ' + JSON.stringify(data));
        if (data != undefined) {
            expect(true).assertTrue();
        } else {
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name:      restoreCallbackSystemTest017
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackSystemTest017', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('restoreCallbackSystemTest017 err : ' + JSON.stringify(err));
                    expect(null).assertFail()
                } else {
                    console.info('restoreCallbackSystemTest017 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            })

        } catch (error) {
            console.info('restoreCallbackSystemTest017 error : ' + JSON.stringify(error));
            expect(null).assertFail();
            done();
        }

    })

    /**
     * @tc.name:      restorePromiseSystemTest018
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseSystemTest018', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_SYSTEM).then((data) => {
                console.info('restorePromiseSystemTest018 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('restorePromiseSystemTest018 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      restoreCallbackLockTest019
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackLockTest019', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('restoreCallbackLockTest019 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('restoreCallbackLockTest019 data : ' + JSON.stringify(data));
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
     * @tc.name:      restorePromiseLockTest020
     * @tc.desc:      Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseLockTest020', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('restorePromiseLockTest020 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('restorePromiseLockTest020 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      restoreCallbackThrowErrorTest021
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackThrowErrorTest021', 0, async function (done) {
        try {
            wallpaper.restore(2, function (err, data) {
                if (err) {
                    console.info('restoreCallbackThrowErrorTest021 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('restoreCallbackThrowErrorTest021 data : ' + JSON.stringify(data));
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
     * @tc.name:      restoreCallbackThrowErrorTest022
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restoreCallbackThrowErrorTest022', 0, async function (done) {
        try {
            wallpaper.restore(function (err, data) {
                if (err) {
                    console.info('restoreCallbackThrowErrorTest022 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('restoreCallbackThrowErrorTest022 data : ' + JSON.stringify(data));
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
     * @tc.name:      restorePromiseThrowErrorTest023
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseThrowErrorTest023', 0, async function (done) {
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
     * @tc.name:      restorePromiseThrowErrorTest024
     * @tc.desc:      Test restore() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('restorePromiseThrowErrorTest024', 0, async function (done) {
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
     * @tc.name:      getImagePromiseLockTest025
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseLockTest025', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('getImagePromiseLockTest025 data : ' + JSON.stringify(data));
                if (data != undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info('getImagePromiseLockTest025 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getImageCallbackSystemTest026
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackSystemTest026', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('getImageCallbackSystemTest026 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('getImageCallbackSystemTest026 data : ' + JSON.stringify(data));
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
     * @tc.name:      getImagePromiseSystemTest027
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseSystemTest027', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM).then((data) => {
                console.info('getImagePromiseSystemTest027 data : ' + JSON.stringify(data));
                if (data != undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info('getImagePromiseSystemTest027 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      getImageCallbackLockTest028
     * @tc.desc:      Test getImage() to get a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackLockTest028', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('getImageCallbackLockTest028 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('getImageCallbackLockTest028 data : ' + JSON.stringify(data));
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
     * @tc.name:      getImageCallbackThrowErrorTest029
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackThrowErrorTest029', 0, async function (done) {
        try {
            wallpaper.getImage(2, function (err, data) {
                if (err) {
                    console.info('getImageCallbackThrowErrorTest029 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('getImageCallbackThrowErrorTest029 data : ' + JSON.stringify(data));
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
     * @tc.name:      getImageCallbackThrowErrorTest030
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImageCallbackThrowErrorTest030', 0, async function (done) {
        try {
            wallpaper.getImage(function (err, data) {
                if (err) {
                    console.info('getImageCallbackThrowErrorTest030 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('getImageCallbackThrowErrorTest030 data : ' + JSON.stringify(data));
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
     * @tc.name:      getImagePromiseThrowErrorTest031
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseThrowErrorTest031', 0, async function (done) {
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
     * @tc.name:      getImagePromiseThrowErrorTest032
     * @tc.desc:      Test getImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseThrowErrorTest032', 0, async function (done) {
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
     * @tc.name:      setImageURLPromiseLockTest033
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLPromiseLockTest033', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('setImageURLPromiseLockTest033 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('setImageURLPromiseLockTest033 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageURLCallbackSystemTest034
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLCallbackSystemTest034', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceSystem, WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('setImageURLCallbackSystemTest034 err : ' + JSON.stringify(err.message));
                    expect(null).assertFail();
                } else {
                    console.info('setImageURLCallbackSystemTest034 data : ' + JSON.stringify(data));
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
     * @tc.name:      setImageURLPromiseSystemTest035
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLPromiseSystemTest035', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceSystem, WALLPAPER_SYSTEM).then((data) => {
                console.info('setImageURLPromiseSystemTest035 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('setImageURLPromiseSystemTest035 err : ' + JSON.stringify(err));
                expect(null).assertFail();
                done();
            });
        } catch (error) {
            expect(null).assertFail();
            done();
        }
    })

    /**
     * @tc.name:      setImageURLCallbackLockTest036
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLCallbackLockTest036', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('setImageURLCallbackLockTest036 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('setImageURLCallbackLockTest036 data : ' + JSON.stringify(data));
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
     * @tc.name:      setImageMapPromiseLockTest037
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapPromiseLockTest037', 0, async function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN).then((data) => {
                    console.info('setImageMapPromiseLockTest037 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                    done();
                }).catch((err) => {
                    console.info('setImageMapPromiseLockTest037 err : ' + JSON.stringify(err));
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
     * @tc.name:      setImageMapCallbackSystemTest038
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapCallbackSystemTest038', 0, async function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM, function (err, data) {
                    if (err) {
                        console.info('setImageMapCallbackSystemTest038 err : ' + JSON.stringify(err));
                        expect(null).assertFail();
                    } else {
                        console.info('setImageMapCallbackSystemTest038 data : ' + JSON.stringify(data));
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
     * @tc.name:      setImageMapPromiseSystemTest039
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapPromiseSystemTest039', 0, function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM).then((data) => {
                    console.info('setImageMapPromiseSystemTest039 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                    done();
                }).catch((err) => {
                    console.info('setImageMapPromiseSystemTest039 err : ' + JSON.stringify(err));
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
     * @tc.name:      setImageMapCallbackLockTest040
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapCallbackLockTest040', 0, async function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN, function (err, data) {
                    if (err) {
                        console.info('setImageMapCallbackLockTest040 err : ' + JSON.stringify(err));
                        expect(null).assertFail();
                    } else {
                        console.info('setImageMapCallbackLockTest040 data : ' + JSON.stringify(data));
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
     * @tc.name:      setImageCallbackThrowErrorTest041
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageCallbackThrowErrorTest041', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, 2, function (err, data) {
                if (err) {
                    console.info('setImageCallbackThrowErrorTest041 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('setImageCallbackThrowErrorTest041 data : ' + JSON.stringify(data));
                    expect(null).assertFail();
                }
                done();
            })
        } catch (error) {
            expect(null).assertFail();
        }
    })

    /**
     * @tc.name:      setImageCallbackThrowErrorTest042
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageCallbackThrowErrorTest042', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, function (err, data) {
                if (err) {
                    console.info('setImageCallbackThrowErrorTest042 err : ' + JSON.stringify(err));
                    expect(null).assertFail();
                } else {
                    console.info('setImageCallbackThrowErrorTest042 data : ' + JSON.stringify(data));
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
     * @tc.name:      setImagePromiseThrowErrorTest043
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImagePromiseThrowErrorTest043', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, 2).then((data) => {
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
     * @tc.name:      setImagePromiseThrowErrorTest044
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImagePromiseThrowErrorTest044', 0, async function (done) {
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
     * @tc.name:      onCallbackTest045
     * @tc.desc:      Test on_colorChange to registers a listener for wallpaper color changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('onCallbackTest045', 0, function () {
        try {
            wallpaper.on('colorChange', function (colors, wallpaperType) {
                console.info('onCallbackTest045 colors : ' + JSON.stringify(colors));
                console.info('onCallbackTest045 wallpaperType : ' + JSON.stringify(wallpaperType));
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
     * @tc.name:      onCallbackThrowErrorTest046
     * @tc.desc:      Test on_colorChange throw error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('onCallbackThrowErrorTest046', 0, function () {
        try {
            wallpaper.on(function (colors, wallpaperType) {
                console.info('onCallbackThrowErrorTest046 colors : ' + JSON.stringify(colors));
                console.info('onCallbackThrowErrorTest046 wallpaperType : ' + JSON.stringify(wallpaperType));
                expect(null).assertFail();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name:      offCallbackTest047
     * @tc.desc:      Test off_colorChange to registers a listener for wallpaper color changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest047', 0, function () {
        try {
            wallpaper.off('colorChange', function (colors, wallpaperType) {
                console.info('offCallbackTest047 colors : ' + JSON.stringify(colors));
                console.info('offCallbackTest047 wallpaperType : ' + JSON.stringify(wallpaperType));
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
     * @tc.name:      offCallbackThrowErrorTest048
     * @tc.desc:      Test off_colorChange throw error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackThrowErrorTest048', 0, function () {
        try {
            wallpaper.off(function (colors, wallpaperType) {
                console.info('offCallbackThrowErrorTest048 colors : ' + JSON.stringify(colors));
                console.info('offCallbackThrowErrorTest048 wallpaperType : ' + JSON.stringify(wallpaperType));
                expect(null).assertFail();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })
})