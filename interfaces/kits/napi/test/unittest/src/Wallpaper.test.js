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

let imageSourceSystem = '/system/etc/wallpaper_system.png';
let imageSourceLockscreen = 'system/etc/wallpaper_lock.png';


describe('WallpaperJSTest', function () {
    beforeAll(async function () {
        console.info('beforeAll: Prerequisites at the test suite level, ' + 'which are executed before the test suite is executed.');
    })
    beforeEach(function () {
        console.info('beforeEach: Prerequisites at the test case level, ' + 'which are executed before each test case is executed.');
    })
    afterEach(function () {
        console.info('afterEach: Test case-level clearance conditions,' + ' which are executed after each test case is executed.');
    })
    afterAll(function () {
        console.info('afterAll: Test suite-level cleanup condition, ' + 'which is executed after the test suite is executed');
    })

    /**
     * @tc.name      testGetColorsSync101
     * @tc.desc      Test getColorsSync() WALLPAPER_SYSTEM Colors by syncing
     * @tc.type      Function
     * @tc.require   0
     */
    it('testGetColorsSync101', 0, function () {
        try {
            let data = wallpaper.getColorsSync(WALLPAPER_SYSTEM);
            let RgbaColor = {
                red: data[0][0], green: data[0][1], blue: data[0][2], alpha: data[0][3]
            }
            console.info('testGetColorsSync101 data : ' + JSON.stringify(data));
            if ((data != undefined) && (data != null) && (data != '')) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('testGetColorsSync101 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            console.info('error.code typeof : ' + typeof (error.code));
            expect(true === false).assertTrue();
        }
    })


    /**
     * @tc.name      testGetColorsSync102
     * @tc.desc      Test getColorsSync() WALLPAPER_LOCKSCREEN Colors by syncing
     * @tc.type      Function
     * @tc.require   0
     */
    it('testGetColorsSync102', 0, function () {
        try {
            let data = wallpaper.getColorsSync(WALLPAPER_LOCKSCREEN);
            let RgbaColor = {
                red: data[0][0], green: data[0][1], blue: data[0][2], alpha: data[0][3]
            }
            console.info('testGetColorsSync102 data : ' + JSON.stringify(data));
            if ((data != undefined) && (data != null) && (data != '')) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('testGetColorsSync102 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(true === false).assertTrue();
        }
    })

    /**
     * @tc.name      testGetColorsSync103
     * @tc.desc      Test getColorsSync() throw parameter error
     * @tc.type      Function
     * @tc.require   0
     */
    it('testGetColorsSync103', 0, function () {
        try {
            let data = wallpaper.getColorsSync(2);
            console.info('testGetColorsSync103 data : ' + JSON.stringify(data));
            expect(true === false).assertTrue();
        } catch (error) {
            console.info('testGetColorsSync103 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name      testGetColorsSync104
     * @tc.desc      Test getColorsSync() throw parameter error
     * @tc.type      Function
     * @tc.require   0
     */
    it('testGetColorsSync104', 0, function () {
        try {
            let data = wallpaper.getColorsSync();
            console.info('testGetColorsSync104 data : ' + JSON.stringify(data));
            expect(true === false).assertTrue();
        } catch (error) {
            console.info('testGetColorsSync104 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name      testGetIdSync101
     * @tc.desc      Test getIdSync() to the ID of the wallpaper of the specified type.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetIdSync101', 0, function () {
        try {
            let data = wallpaper.getIdSync(WALLPAPER_SYSTEM);
            console.info('testGetIdSync101 data : ' + JSON.stringify(data));
            if ((data != undefined) && (data != null) && (data != '')) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('testGetIdSync101 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(true === false).assertTrue();
        }
    })

    /**
     * @tc.name      testGetIdSync102
     * @tc.desc      Test getIdSync() to the ID of the wallpaper of the specified type.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetIdSync102', 0, function () {
        try {
            let data = wallpaper.getIdSync(WALLPAPER_LOCKSCREEN);
            console.info('testGetIdSync102 data : ' + JSON.stringify(data));
            if ((data != undefined) && (data != null) && (data != '')) {
                expect(true).assertTrue();
            }
        } catch (error) {
            console.info('testGetIdSync102 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            console.info('error.code typeof : ' + typeof (error.code));
            expect(true === false).assertTrue();
        }
    })

    /**
     * @tc.name      testGetIdSync103
     * @tc.desc      Test getIdSync() throw parameter error.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetIdSync103', 0, function () {
        try {
            let data = wallpaper.getIdSync(3);
            console.info('testGetIdSync103 data : ' + JSON.stringify(data));
            expect(true === false).assertTrue();
        } catch (error) {
            console.info('testGetIdSync103 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name      testGetIdSync104
     * @tc.desc      Test getIdSync() throw parameter error.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetIdSync104', 0, function () {
        try {
            let data = wallpaper.getIdSync();
            console.info('testGetIdSync104 data : ' + JSON.stringify(data));
            expect(true === false).assertTrue();
        } catch (error) {
            console.info('testGetIdSync104 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name      testGetFileSync101
     * @tc.desc      Test getFileSync() to File of the wallpaper of the specified type.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetFileSync101', 0, function () {
        try {
            let data = wallpaper.getFileSync(WALLPAPER_SYSTEM);
            expect(typeof data == "number").assertTrue();
            expect(!isNaN(data)).assertTrue();
            console.info("testGetFileSync101 success to getFile: " + JSON.stringify(data));
        } catch (error) {
            console.error("testGetFileSync101 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name      testGetFileSync102
     * @tc.desc      Test getFileSync() to the ID of the wallpaper of the specified type.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetFileSync102', 0, function () {
        try {
            let data = wallpaper.getFileSync(WALLPAPER_LOCKSCREEN);
            expect(typeof data == "number").assertTrue();
            expect(!isNaN(data)).assertTrue();
            console.info("testGetFileSync102 success to getFile: " + JSON.stringify(data));
        } catch (error) {
            console.error("testGetFileSync102 failed to getFile because: " + JSON.stringify(error));
            expect(null).assertFail()
        }
    })

    /**
     * @tc.name      testGetFileSync103
     * @tc.desc      Test getFileSync() throw parameter error.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetFileSync103', 0, function () {
        try {
            let data = wallpaper.getFileSync(3);
            console.info('testGetFileSync103 data : ' + JSON.stringify(data));
            expect(null).assertFail()
        } catch (error) {
            console.info('testGetFileSync103 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name      testGetFileSync104
     * @tc.desc      Test getFileSync() throw parameter error.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetFileSync104', 0, function () {
        try {
            let data = wallpaper.getFileSync();
            console.info('testGetFileSync104 data : ' + JSON.stringify(data));
            expect(null).assertFail()
        } catch (error) {
            console.info('testGetFileSync104 error.code : ' + error.code + ',' + 'error.message : ' + error.message);
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.name      testGetMinHeightSync101
     * @tc.desc      Test getMinHeightSync() to the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetMinHeightSync101', 0, function () {
        let data = wallpaper.getMinHeightSync();
        console.info('testGetMinHeightSync101 data : ' + JSON.stringify(data));
        if ((data != undefined) && (data != null) && (data != '')) {
            expect(true).assertTrue();
        }
    })

    /**
     * @tc.name      testGetMinWidthSync101
     * @tc.desc      Test getMinWidthSync() to the minHeight of the WALLPAPER_SYSTEM of the specified type.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testGetMinWidthSync101', 0, function () {
        let data = wallpaper.getMinWidthSync();
        console.info('testGetMinWidthSync101 data : ' + JSON.stringify(data));
        if ((data != undefined) && (data != null) && (data != '')) {
            expect(true).assertTrue();
        }
    })

    /**
     * @tc.name      testIsChangeAllowed101
     * @tc.desc      Test isChangeAllowed() to checks whether to allow the application to change the
     *                     wallpaper for the current user.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testIsChangeAllowed101', 0, function () {
        let data = wallpaper.isChangeAllowed();
        console.info('testIsChangeAllowed101 data : ' + JSON.stringify(data));
        if ((data != undefined) && (data != null) && (data != '')) {
            expect(true).assertTrue();
        }
    })

    /**
     * @tc.name      testIsUserChangeAllowed101
     * @tc.desc      Test isUserChangeAllowed() to checks whether a user is allowed to set wallpapers.
     * @tc.type      Function test
     * @tc.require   0
     */
    it('testIsUserChangeAllowed101', 0, function () {
        let data = wallpaper.isUserChangeAllowed();
        console.info('testIsUserChangeAllowed101 data : ' + JSON.stringify(data));
        if ((data != undefined) && (data != null) && (data != '')) {
            expect(true).assertTrue();
        }
    })

    /**
     * @tc.number  testRestoreCallbackSystem101
     * @tc.name    Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestoreCallbackSystem101', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('testRestoreCallbackSystem101 err : ' + JSON.stringify(err));
                    expect(null).assertFail()
                } else {
                    console.info('testRestoreCallbackSystem101 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            })

        } catch (error) {
            console.info('testRestoreCallbackSystem101 error : ' + JSON.stringify(error));
            expect(true === false).assertTrue();
            done();
        }

    })

    /**
     * @tc.number  testRestorePromiseSystem101
     * @tc.name    Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestorePromiseSystem101', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_SYSTEM).then((data) => {
                console.info('testRestorePromiseSystem101 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('testRestorePromiseSystem101 err : ' + JSON.stringify(err));
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testRestoreCallbackLock102
     * @tc.name    Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestoreCallbackLock102', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('testRestoreCallbackLock102 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testRestoreCallbackLock102 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testRestorePromiseLock102
     * @tc.name    Test restore() to removes a wallpaper of the specified type and restores the default one.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestorePromiseLock102', 0, async function (done) {
        try {
            wallpaper.restore(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('testRestorePromiseLock102 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('testRestorePromiseLock102 err : ' + JSON.stringify(err));
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testRestoreCallbackThrowError103
     * @tc.name    Test restore() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestoreCallbackThrowError103', 0, async function (done) {
        try {
            wallpaper.restore(2, function (err, data) {
                if (err) {
                    console.info('testRestoreCallbackThrowError103 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('testRestoreCallbackThrowError103 data : ' + JSON.stringify(data));
                    expect(true === false).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(true === false).assertTrue();
        }
        done();
    })

    /**
     * @tc.number  testRestoreCallbackThrowError104
     * @tc.name    Test restore() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestoreCallbackThrowError104', 0, async function (done) {
        try {
            wallpaper.restore(function (err, data) {
                if (err) {
                    console.info('testRestoreCallbackThrowError104 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testRestoreCallbackThrowError104 data : ' + JSON.stringify(data));
                    expect(true === false).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
            done();
        }
    })

    /**
     * @tc.number  testRestorePromiseThrowError103
     * @tc.name    Test restore() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestorePromiseThrowError103', 0, async function (done) {
        try {
            wallpaper.restore(2).then((data) => {
                expect(true === false).assertTrue();
                done();
            }).catch((err) => {
                expect(err.code == PARAMETER_ERROR).assertEqual(true)
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testRestorePromiseThrowError104
     * @tc.name    Test restore() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testRestorePromiseThrowError104', 0, async function (done) {
        try {
            wallpaper.restore().then((data) => {
                expect(true === false).assertTrue();
                done();
            }).catch((err) => {
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
            done();
        }
    })

    /**
     * @tc.number  testGetImagePromiseLock101
     * @tc.name    Test getImage() to get a PixelMap of the specified type.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImagePromiseLock101', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('testGetImagePromiseLock101 data : ' + JSON.stringify(data));
                if (data != undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info('testGetImagePromiseLock101 err : ' + JSON.stringify(err));
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testGetImageCallbackSystem101
     * @tc.name    Test getImage() to get a PixelMap of the specified type.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImageCallbackSystem101', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('testGetImageCallbackSystem101 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testGetImageCallbackSystem101 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testGetImagePromiseSystem102
     * @tc.name    Test getImage() to get a PixelMap of the specified type.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImagePromiseSystem102', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM).then((data) => {
                console.info('testGetImagePromiseSystem102 data : ' + JSON.stringify(data));
                if (data != undefined) {
                    expect(true).assertTrue();
                }
                done();
            }).catch((err) => {
                console.info('testGetImagePromiseSystem102 err : ' + JSON.stringify(err));
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testGetImageCallbackLock102
     * @tc.name    Test getImage() to get a PixelMap of the specified type.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImageCallbackLock102', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('testGetImageCallbackLock102 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testGetImageCallbackLock102 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(true).assertTrue();
                    }
                }
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testGetImageCallbackThrowError103
     * @tc.name    Test getImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImageCallbackThrowError103', 0, async function (done) {
        try {
            wallpaper.getImage(2, function (err, data) {
                if (err) {
                    console.info('testGetImageCallbackThrowError103 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('testGetImageCallbackThrowError103 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(true === false).assertTrue();
                    }
                }
                done();
            })
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testGetImageCallbackThrowError104
     * @tc.name    Test getImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImageCallbackThrowError104', 0, async function (done) {
        try {
            wallpaper.getImage(function (err, data) {
                console.info('testGetImageCallbackThrowError104 data : ' + JSON.stringify(data));
                if (err) {
                    console.info('testGetImageCallbackThrowError104 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testGetImageCallbackThrowError104 data : ' + JSON.stringify(data));
                    if (data != undefined) {
                        expect(true === false).assertTrue();
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
     * @tc.number  testGetImagePromiseThrowError103
     * @tc.name    Test getImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImagePromiseThrowError103', 0, async function (done) {
        try {
            wallpaper.getImage(2).then((data) => {
                if (data != undefined) {
                    expect(true === false).assertTrue();
                }
                done();
            }).catch((err) => {
                expect(err.code == PARAMETER_ERROR).assertEqual(true)
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testGetImagePromiseThrowError104
     * @tc.name    Test getImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testGetImagePromiseThrowError104', 0, async function (done) {
        try {
            wallpaper.getImage().then((data) => {
                if (data != undefined) {
                    expect(true === false).assertTrue();
                }
                done();
            }).catch((err) => {
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
            done();
        }
    })

    /**
     * @tc.number  testSetImageURLPromiseLock101
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     JPEG or PNG file or the pixel map of a PNG file.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageURLPromiseLock101', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, WALLPAPER_LOCKSCREEN).then((data) => {
                console.info('testSetImageURLPromiseLock101 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('testSetImageURLPromiseLock101 err : ' + JSON.stringify(err));
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageURLCallbackSystem101
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     JPEG or PNG file or the pixel map of a PNG file.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageURLCallbackSystem101', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceSystem, WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info('testSetImageURLCallbackSystem101 err : ' + JSON.stringify(err.message));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testSetImageURLCallbackSystem101 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageURLPromiseSystem102
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     JPEG or PNG file or the pixel map of a PNG file.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageURLPromiseSystem102', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceSystem, WALLPAPER_SYSTEM).then((data) => {
                console.info('testSetImageURLPromiseSystem102 data : ' + JSON.stringify(data));
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info('testSetImageURLPromiseSystem102 err : ' + JSON.stringify(err));
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageURLCallbackLock102
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     JPEG or PNG file or the pixel map of a PNG file.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageURLCallbackLock102', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info('testSetImageURLCallbackLock102 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testSetImageURLCallbackLock102 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                }
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageMapPromiseLock101
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageMapPromiseLock101', 0, async function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN).then((data) => {
                    console.info('testSetImageMapPromiseLock101 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                    done();
                }).catch((err) => {
                    console.info('testSetImageMapPromiseLock101 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                    done();
                });
            })
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageMapCallbackSystem101
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageMapCallbackSystem101', 0, async function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM, function (err, data) {
                    if (err) {
                        console.info('testSetImageMapCallbackSystem101 err : ' + JSON.stringify(err));
                        expect(true === false).assertTrue();
                    } else {
                        console.info('testSetImageMapCallbackSystem101 data : ' + JSON.stringify(data));
                        expect(true).assertTrue();
                    }
                    done();
                });
            })
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageMapPromiseSystem102
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageMapPromiseSystem102', 0, function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM).then((data) => {
                    console.info('testSetImageMapPromiseSystem102 data : ' + JSON.stringify(data));
                    expect(true).assertTrue();
                    done();
                }).catch((err) => {
                    console.info('testSetImageMapPromiseSystem102 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                    done();
                });
            })
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageMapCallbackLock102
     * @tc.name    Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageMapCallbackLock102', 0, async function (done) {
        try {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5}, pixelFormat: 3, editable: true, alphaType: 1, scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then(async (pixelMap) => {
                wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN, function (err, data) {
                    if (err) {
                        console.info('testSetImageMapCallbackLock102 err : ' + JSON.stringify(err));
                        expect(true === false).assertTrue();
                    } else {
                        console.info('testSetImageMapCallbackLock102 data : ' + JSON.stringify(data));
                        expect(true).assertTrue();
                    }
                    done();
                });
            })
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImageCallbackThrowError103
     * @tc.name    Test setImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageCallbackThrowError103', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, 2, function (err, data) {
                if (err) {
                    console.info('testSetImageCallbackThrowError103 err : ' + JSON.stringify(err));
                    expect(err.code == PARAMETER_ERROR).assertEqual(true)
                } else {
                    console.info('testSetImageCallbackThrowError103 data : ' + JSON.stringify(data));
                    expect(true === false).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(true === false).assertTrue();
        }
    })

    /**
     * @tc.number  testSetImageCallbackThrowError104
     * @tc.name    Test setImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImageCallbackThrowError104', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, function (err, data) {
                if (err) {
                    console.info('testSetImageCallbackThrowError104 err : ' + JSON.stringify(err));
                    expect(true === false).assertTrue();
                } else {
                    console.info('testSetImageCallbackThrowError104 data : ' + JSON.stringify(data));
                    expect(true === false).assertTrue();
                }
                done();
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
            done();
        }
    })

    /**
     * @tc.number  testSetImagePromiseThrowError103
     * @tc.name    Test setImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImagePromiseThrowError103', 0, async function (done) {
        try {
            wallpaper.setImage(imageSourceLockscreen, 2).then((data) => {
                expect(true === false).assertTrue();
                done();
            }).catch((err) => {
                expect(err.code == PARAMETER_ERROR).assertEqual(true)
                done();
            });
        } catch (error) {
            expect(true === false).assertTrue();
            done();
        }
    })

    /**
     * @tc.number  testSetImagePromiseThrowError104
     * @tc.name    Test setImage() throw parameter error.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testSetImagePromiseThrowError104', 0, async function (done) {
        try {
            wallpaper.setImage().then((data) => {
                expect(true === false).assertTrue();
                done();
            }).catch((err) => {
                expect(true === false).assertTrue();
                done();
            });
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
            done();
        }
    })

    /**
     * @tc.number  testOnCallback101
     * @tc.name    Test on_colorChange to registers a listener for wallpaper color changes to
     receive notifications about the changes.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testOnCallback101', 0, function () {
        try {
            wallpaper.on('colorChange', function (colors, wallpaperType) {
                console.info('testOnCallback colors : ' + JSON.stringify(colors));
                console.info('testOnCallback wallpaperType : ' + JSON.stringify(wallpaperType));
                if ((colors != undefined) && (colors.size() != 0)) {
                    expect(true).assertTrue();
                }
                if (wallpaperType != undefined) {
                    expect(true).assertTrue();
                }
            })
        } catch (error) {
            expect(true === false).assertTrue();
        }
    })

    /**
     * @tc.number  testOnCallback102
     * @tc.name    Test on_colorChange throw error
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testOnCallback102', 0, function () {
        try {
            wallpaper.on(function (colors, wallpaperType) {
                console.info('testOnCallback102 colors : ' + JSON.stringify(colors));
                console.info('testOnCallback102 wallpaperType : ' + JSON.stringify(wallpaperType));
                if ((colors != undefined) && (colors.size() != 0)) {
                    expect(true === false).assertTrue();
                }
                if (wallpaperType != undefined) {
                    expect(true === false).assertTrue();
                }
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })

    /**
     * @tc.number  testOffCallback101
     * @tc.name    Test off_colorChange to registers a listener for wallpaper color changes to
     receive notifications about the changes.
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testOffCallback101', 0, function () {
        try {
            wallpaper.off('colorChange', function (colors, wallpaperType) {
                console.info('testOffCallback101 colors : ' + JSON.stringify(colors));
                console.info('testOffCallback101 wallpaperType : ' + JSON.stringify(wallpaperType));
                if ((colors != undefined) && (colors.size() != 0)) {
                    expect(true).assertTrue();
                }
                if (wallpaperType != undefined) {
                    expect(true).assertTrue();
                }
            })
        } catch (error) {
            expect(true === false).assertTrue();
        }
    })

    /**
     * @tc.number  testOffCallback102
     * @tc.name    Test off_colorChange throw error
     * @tc.desc    Function test
     * @tc.level   0
     */
    it('testOffCallback102', 0, function () {
        try {
            wallpaper.off(function (colors, wallpaperType) {
                console.info('testOffCallback102 colors : ' + JSON.stringify(colors));
                console.info('testOffCallback102 wallpaperType : ' + JSON.stringify(wallpaperType));
                if ((colors != undefined) && (colors.size() != 0)) {
                    expect(true === false).assertTrue();
                }
                if (wallpaperType != undefined) {
                    expect(true === false).assertTrue();
                }
            })
        } catch (error) {
            expect(error.code == PARAMETER_ERROR).assertEqual(true)
        }
    })
})