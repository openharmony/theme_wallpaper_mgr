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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import wallpaper from '@ohos.wallpaper'

const WALLPAPER_SYSTEM = 0;
const WALLPAPER_LOCKSCREEN = 1;

describe('WallpaperPerfJSTest', function () {
    beforeAll(async function () {
        console.info('beforeAll');
    })

    afterAll(async function () {
        console.info('afterAll');
    })

    const SAMPLE_SIZE = 100;

    /**
     * @tc.name      on_performance_test_001
     * @tc.desc      on interface promise performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('on_performance_test_001', 0, function (done) {
        let info = "on_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.off('colorChange', function (colors, wallpaperType) {
            }, info);
            done();
        })
    })

    /**
     * @tc.name      getColorsSync_performance_test_001
     * @tc.desc      getColorsSync WALLPAPER_SYSTEM interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getColorsSync_performance_test_001', 0, function (done) {
        let info = "getColorsSync_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getColorsSync(WALLPAPER_SYSTEM)
        }, info);
        done();
    })

    /**
     * @tc.name      getColorsSync_performance_test_002
     * @tc.desc      getColorsSync WALLPAPER_LOCKSCREEN interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getColorsSync_performance_test_002', 0, function (done) {
        let info = "getColorsSync_performance_test_002 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getColorsSync(WALLPAPER_LOCKSCREEN)
        }, info);
        done();
    })

    /**
     * @tc.name      getIdSync_performance_test_001
     * @tc.desc      getIdSync WALLPAPER_SYSTEM interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getIdSync_performance_test_001', 0, function (done) {
        let info = "getIdSync_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getIdSync(WALLPAPER_SYSTEM)
        }, info);
        done();
    })

    /**
     * @tc.name      getIdSync_performance_test_002
     * @tc.desc      getIdSync WALLPAPER_LOCKSCREEN interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getIdSync_performance_test_002', 0, function (done) {
        let info = "getIdSync_performance_test_002 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getIdSync(WALLPAPER_LOCKSCREEN)
        }, info);
        done();
    })

    /**
     * @tc.name      getFileSync_performance_test_001
     * @tc.desc      getFileSync WALLPAPER_SYSTEM interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getFileSync_performance_test_001', 0, function (done) {
        let info = "getFileSync_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getFileSync(WALLPAPER_SYSTEM)
        }, info);
        done();
    })

    /**
     * @tc.name      getFileSync_performance_test_002
     * @tc.desc      getFileSync WALLPAPER_LOCKSCREEN interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getFileSync_performance_test_002', 0, function (done) {
        let info = "getFileSync_performance_test_002 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getFileSync(WALLPAPER_LOCKSCREEN)
        }, info);
        done();
    })

    /**
     * @tc.name      getMinHeightSync_performance_test_001
     * @tc.desc      getMinHeightSync interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getMinHeightSync_performance_test_001', 0, function (done) {
        let info = "getMinHeightSync_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getMinHeightSync()
        }, info);
        done();
    })

    /**
     * @tc.name      getMinWidthSync_performance_test_001
     * @tc.desc      getMinWidthSync interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('getMinWidthSync_performance_test_001', 0, function (done) {
        let info = "getMinWidthSync_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.getMinWidthSync()
        }, info);
        done();
    })

    /**
     * @tc.name      isChangeAllowed_performance_test_001
     * @tc.desc      isChangeAllowed interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('isChangeAllowed_performance_test_001', 0, function (done) {
        let info = "isChangeAllowed_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.isChangeAllowed()
        }, info);
        done();
    })

    /**
     * @tc.name      isUserChangeAllowed_performance_test_001
     * @tc.desc      isUserChangeAllowed interface performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('isUserChangeAllowed_performance_test_001', 0, function (done) {
        let info = "isUserChangeAllowed_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.isUserChangeAllowed()
        }, info);
        done();
    })

    /**
     * @tc.name      off_performance_test_001
     * @tc.desc      off interface promise performance test
     * @tc.type      PERF
     * @tc.require
     */
    it('off_performance_test_001', 0, function (done) {
        let info = "off_performance_test_001 averageTime:";
        calcAverageTime(SAMPLE_SIZE, () => {
            wallpaper.off('colorChange', function (colors, wallpaperType) {
            })
        }, info);
        done();
    })

    function calcAverageTime(times, callback, info) {
        let startTime = new Date().getTime();
        for (let index = 0; index < times; index++) {
            callback();
        }
        let endTime = new Date().getTime();
        let averageTime = ((endTime - startTime) * 1000) / times;
        console.info(info + averageTime);
    }
});