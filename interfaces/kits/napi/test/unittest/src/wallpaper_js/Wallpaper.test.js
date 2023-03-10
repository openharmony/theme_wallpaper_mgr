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
const PARAMETER_ERROR = 401;
const URL = "/data/storage/el2/base/haps/js.jpeg";

describe('WallpaperJSTest', function () {
    beforeAll(async function () {
        // input testsuit setup step，setup invoked before all testcases
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
    afterAll(function () {
        // input testsuit teardown step，teardown invoked after all testcases
        console.info('afterAll called')
        wallpaper.restore(WALLPAPER_SYSTEM, function (err, data) {
            if (err) {
                console.info(`restore WALLPAPER_SYSTEM err ${err}`)
            } else {
                console.info('restore successful');
            }
        })
        wallpaper.restore(WALLPAPER_LOCKSCREEN, function (err, data) {
            if (err) {
                console.info(`restore WALLPAPER_LOCKSCREEN err ${err}`)
            } else {
                console.info('restore successful');
            }
        })
    })

    async function createTempImage() {
        let pixelMap = await createTempPixelMap();
        const imagePackerApi = image.createImagePacker();
        let packOpts = {format: "image/jpeg", quality: 98};
        imagePackerApi.packing(pixelMap, packOpts, (err, data) => {
            if (err) {
                console.info(`packing error: ${err}`)
            } else {
                let fd = fileio.openSync(URL, 0o2 | 0o100, 0o666);
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
            if (data != undefined) {
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
            if (data != undefined) {
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
            let data = wallpaper.getColorsSync(2);
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
        if (data != undefined) {
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
            wallpaper.restore(WALLPAPER_SYSTEM).then((data) => {
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
            wallpaper.restore(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info(`restoreCallbackLockTest003 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
            wallpaper.restore(2, function (err, data) {
                if (err) {
                    console.info(`restoreCallbackThrowErrorTest005 err : ${err}`);
                    expect(err.code === PARAMETER_ERROR).assertTrue()
                } else {
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
                    console.info(`restoreCallbackThrowErrorTest006 err : ${err}`);
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
            wallpaper.restore().then((data) => {
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
                if (data != undefined) {
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
                    console.info(`getImageCallbackSystemTest002 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getImageCallbackSystemTest002 data : ${data}`);
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
     * @tc.desc:      Test getImage() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getImagePromiseSystemTest003', 0, async function (done) {
        try {
            wallpaper.getImage(WALLPAPER_SYSTEM).then((data) => {
                console.info(`getImagePromiseSystemTest003 data : ${data}`);
                if (data != undefined) {
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
                    console.info(`getImageCallbackLockTest004 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getImageCallbackLockTest004 data : ${data}`);
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
                    console.info(`getImageCallbackThrowErrorTest005 err : ${err}`);
                    expect(err.code === PARAMETER_ERROR).assertTrue()
                } else {
                    console.info(`getImageCallbackThrowErrorTest005 data : ${data}`);
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
                    console.info(`getImageCallbackThrowErrorTest006 err : ${err}`);
                    expect(null).assertFail();
                } else {
                    console.info(`getImageCallbackThrowErrorTest006 data : ${data}`);
                    if (data != undefined) {
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
            wallpaper.getImage(2).then((data) => {
                console.info(`getImagePromiseThrowErrorTest007 data : ${data}`);
                if (data != undefined) {
                    expect(null).assertFail();
                }
                done();
            }).catch((err) => {
                console.info(`getImagePromiseThrowErrorTest007 err : ${err}`);
                expect(err.code === PARAMETER_ERROR).assertTrue()
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
                console.info(`getImagePromiseThrowErrorTest008 data : ${data}`);
                if (data != undefined) {
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
     * @tc.name:      setImageURLPromiseLockTest001
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on the uri path from a
     *                    JPEG or PNG file or the pixel map of a PNG file.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageURLPromiseLockTest001', 0, async function (done) {
        try {
            wallpaper.setImage(URL, WALLPAPER_LOCKSCREEN).then((data) => {
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`setImageURLPromiseLockTest001 err : ${err}`);
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
                    console.info(`setImageURLCallbackSystemTest002 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
                expect(true).assertTrue();
                done();
            }).catch((err) => {
                console.info(`setImageURLPromiseSystemTest003 err : ${err}`);
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
                    console.info(`setImageURLCallbackLockTest004 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
            let pixelMap = await createTempPixelMap();
            wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN).then((data) => {
                expect(true).assertTrue();
                done();
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
            wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info(`setImageMapCallbackSystemTest006 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
     * @tc.name:      setImageMapPromiseSystemTest007
     * @tc.desc:      Test setImage() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageMapPromiseSystemTest007', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setImage(pixelMap, WALLPAPER_SYSTEM).then((data) => {
                expect(true).assertTrue();
                done();
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
            wallpaper.setImage(pixelMap, WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info(`setImageMapCallbackLockTest008 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
     * @tc.name:      setImageCallbackThrowErrorTest009
     * @tc.desc:      Test setImage() throw parameter error.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setImageCallbackThrowErrorTest009', 0, async function (done) {
        try {
            wallpaper.setImage(URL, 2, function (err, data) {
                if (err) {
                    console.info(`setImageCallbackThrowErrorTest009 err : ${err}`);
                    expect(err.code === PARAMETER_ERROR).assertTrue()
                } else {
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
                    console.info(`setImageCallbackThrowErrorTest010 err : ${err}`);
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
            wallpaper.setImage().then((data) => {
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
            wallpaper.setWallpaper(pixelMap, WALLPAPER_LOCKSCREEN).then((data) => {
                expect(true).assertTrue();
                done();
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
            wallpaper.setWallpaper(pixelMap, WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info(`setWallpaperMapCallbackSystemTest002 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
     * @tc.name:      setWallpaperMapPromiseSystemTest003
     * @tc.desc:      Test setWallpaper() to sets a wallpaper of the specified type based on Map.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('setWallpaperMapPromiseSystemTest003', 0, async function (done) {
        try {
            let pixelMap = await createTempPixelMap();
            wallpaper.setWallpaper(pixelMap, WALLPAPER_SYSTEM).then((data) => {
                expect(true).assertTrue();
                done();
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
            wallpaper.setWallpaper(pixelMap, WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info(`setWallpaperMapCallbackLockTest004 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
     * @tc.name:      getPixelMapPromiseLockTest001
     * @tc.desc:      Test getPixelMap() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getPixelMapPromiseLockTest001', 0, async function (done) {
        try {
            wallpaper.getPixelMap(WALLPAPER_LOCKSCREEN).then((data) => {
                if (data != undefined) {
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
                    console.info(`getPixelMapCallbackSystemTest002 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
     * @tc.name:      getPixelMapPromiseSystemTest003
     * @tc.desc:      Test getPixelMap() to gets a PixelMap of the specified type.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('getPixelMapPromiseSystemTest003', 0, async function (done) {
        try {
            wallpaper.getPixelMap(WALLPAPER_SYSTEM).then((data) => {
                if (data != undefined) {
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
                    console.info(`getPixelMapCallbackLockTest004 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
     * @tc.name:      resetCallbackSystemTest001
     * @tc.desc:      Test reset() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('resetCallbackSystemTest001', 0, async function (done) {
        try {
            wallpaper.reset(WALLPAPER_SYSTEM, function (err, data) {
                if (err) {
                    console.info(`resetCallbackSystemTest001 err : ${err}`);
                    expect(null).assertFail()
                } else {
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
     * @tc.name:      resetPromiseSystemTest002
     * @tc.desc:      Test reset() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('resetPromiseSystemTest002', 0, async function (done) {
        try {
            wallpaper.reset(WALLPAPER_SYSTEM).then((data) => {
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
            wallpaper.reset(WALLPAPER_LOCKSCREEN, function (err, data) {
                if (err) {
                    console.info(`resetCallbackLockTest003 err : ${err}`);
                    expect(null).assertFail();
                } else {
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
     * @tc.name:      resetPromiseLockTest004
     * @tc.desc:      Test reset() to removes a wallpaper of the specified type and restores the default one.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('resetPromiseLockTest004', 0, async function (done) {
        try {
            wallpaper.reset(WALLPAPER_LOCKSCREEN).then((data) => {
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
            console.info(`onCallbackTest001 error : ${error}`);
            expect(null).assertFail();
            done();
        }
        await wallpaper.setImage(URL, WALLPAPER_LOCKSCREEN);
    })

    /**
     * @tc.name:      offCallbackTest001
     * @tc.desc:      Test off_colorChange to logoff a listener for wallpaper color changes to
     *                    receive notifications about the changes.
     * @tc.type:      FUNC test
     * @tc.require:   issueI5UHRG
     */
    it('offCallbackTest001', 0, async function (done) {
        let callbackTimes = 0;
        await wallpaper.restore(WALLPAPER_SYSTEM);
        try {
            wallpaper.on('colorChange', (colors, wallpaperType) => {
                console.info(`offCallbackTest001 colors : ${colors}`);
                callbackTimes = callbackTimes + 1;
                wallpaper.off('colorChange');
            })
        } catch (error) {
            console.info(`offCallbackTest001 error : ${error}`);
            expect(null).assertFail();
            done();
        }
        await wallpaper.setImage(URL, WALLPAPER_SYSTEM);
        await wallpaper.restore(WALLPAPER_SYSTEM);
        await wallpaper.setImage(URL, WALLPAPER_SYSTEM);
        expect(callbackTimes === 1).assertTrue();
        done();
    })
})