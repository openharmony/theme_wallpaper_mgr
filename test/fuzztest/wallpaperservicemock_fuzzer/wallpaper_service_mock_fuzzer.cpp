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

#include "wallpaper_service_mock_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include <cstdint>
#include <iostream>

#include "iwallpaper_service.h"
#include "message_parcel.h"
#include "wallpaper_service_stub.h"

namespace {
using namespace OHOS;
using namespace OHOS::WallpaperMgrService;

constexpr uint32_t CODE_MIN = 0;
constexpr uint32_t CODE_MAX =
    static_cast<uint32_t>(IWallpaperServiceIpcCode::COMMAND_IS_DEFAULT_WALLPAPER_RESOURCE) + 1;

const std::u16string WALLPAPERSERVICES_INTERFACE_TOKEN = u"OHOS.WallpaperMgrService.IWallpaperService";

class WallpaperServiceStubMock : public WallpaperServiceStub {
public:
    ErrCode SetWallpaper(int fd, int32_t wallpaperType, int32_t length) override
    {
        (void)fd;
        (void)wallpaperType;
        (void)length;
        return 0;
    }

    ErrCode SetAllWallpapers(const WallpaperPictureInfoByParcel &allWallpaperPictures, int32_t wallpaperType,
        const std::vector<int> &fdVector) override
    {
        (void)allWallpaperPictures;
        (void)wallpaperType;
        (void)fdVector;
        return 0;
    }

    ErrCode SetWallpaperByPixelMap(const WallpaperRawData &wallpaperRawdata, int32_t wallpaperType) override
    {
        (void)wallpaperRawdata;
        (void)wallpaperType;
        return 0;
    }

    ErrCode GetPixelMap(int32_t wallpaperType, int32_t &size, int &fd) override
    {
        (void)wallpaperType;
        (void)size;
        (void)fd;
        return 0;
    }

    ErrCode GetCorrespondWallpaper(
        int32_t wallpaperType, int32_t foldState, int32_t rotateState, int32_t &size, int &fd) override
    {
        (void)wallpaperType;
        (void)foldState;
        (void)rotateState;
        (void)size;
        (void)fd;
        return 0;
    }

    ErrCode GetColors(int32_t wallpaperType, std::vector<uint64_t> &colors) override
    {
        (void)wallpaperType;
        (void)colors;
        return 0;
    }

    ErrCode GetFile(int32_t wallpaperType, int &wallpaperFd) override
    {
        (void)wallpaperType;
        (void)wallpaperFd;
        return 0;
    }

    ErrCode GetWallpaperId(int32_t wallpaperType) override
    {
        (void)wallpaperType;
        return 0;
    }

    ErrCode IsChangePermitted(bool &isChangePermitted) override
    {
        (void)isChangePermitted;
        return 0;
    }

    ErrCode IsOperationAllowed(bool &isOperationAllowed) override
    {
        (void)isOperationAllowed;
        return 0;
    }

    ErrCode ResetWallpaper(int32_t wallpaperType) override
    {
        (void)wallpaperType;
        return 0;
    }

    ErrCode On(const std::string &type, const sptr<IWallpaperEventListener> &listener) override
    {
        (void)type;
        (void)listener;
        return 0;
    }

    ErrCode Off(const std::string &type, const sptr<IWallpaperEventListener> &listener) override
    {
        (void)type;
        (void)listener;
        return 0;
    }

    ErrCode RegisterWallpaperCallback(
        const sptr<IWallpaperCallback> &wallpaperCallback, bool &registerWallpaperCallback) override
    {
        (void)wallpaperCallback;
        (void)registerWallpaperCallback;
        return 0;
    }

    ErrCode SetWallpaperV9(int fd, int32_t wallpaperType, int32_t length) override
    {
        (void)fd;
        (void)wallpaperType;
        (void)length;
        return 0;
    }

    ErrCode SetWallpaperV9ByPixelMap(const WallpaperRawData &wallpaperRawdata, int32_t wallpaperType) override
    {
        (void)wallpaperRawdata;
        (void)wallpaperType;
        return 0;
    }

    ErrCode GetPixelMapV9(int32_t wallpaperType, int32_t &size, int &fd) override
    {
        (void)wallpaperType;
        (void)size;
        (void)fd;
        return 0;
    }

    ErrCode GetColorsV9(int32_t wallpaperType, std::vector<uint64_t> &colors) override
    {
        (void)wallpaperType;
        (void)colors;
        return 0;
    }

    ErrCode ResetWallpaperV9(int32_t wallpaperType) override
    {
        (void)wallpaperType;
        return 0;
    }

    ErrCode SetVideo(int fd, int32_t wallpaperType, int32_t length) override
    {
        (void)fd;
        (void)wallpaperType;
        (void)length;
        return 0;
    }

    ErrCode SetCustomWallpaper(int fd, int32_t wallpaperType, int32_t length) override
    {
        (void)fd;
        (void)wallpaperType;
        (void)length;
        return 0;
    }

    ErrCode SendEvent(const std::string &eventType) override
    {
        (void)eventType;
        return 0;
    }

    ErrCode IsDefaultWallpaperResource(
        int32_t userId, int32_t wallpaperType, bool &isDefaultWallpaperResource) override
    {
        (void)userId;
        (void)wallpaperType;
        (void)isDefaultWallpaperResource;
        return 0;
    }

    int32_t CallbackParcel(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    };
};

class WallpaperFuzzTest {
public:
    WallpaperFuzzTest()
    {
        isInited_ = false;
        stub_ = std::make_shared<WallpaperServiceStubMock>();
        if (stub_ == nullptr) {
            return;
        }
        isInited_ = true;
    }

    ~WallpaperFuzzTest()
    {
        stub_ = nullptr;
        isInited_ = false;
    }

    bool IsInited() const noexcept
    {
        return isInited_;
    }

    void DoRemoteRequest(FuzzedDataProvider &provider)
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = provider.ConsumeIntegralInRange<uint32_t>(CODE_MIN, CODE_MAX);
        data.WriteInterfaceToken(WALLPAPERSERVICES_INTERFACE_TOKEN);
        std::vector<uint8_t> remaining_data = provider.ConsumeRemainingBytes<uint8_t>();
        data.WriteBuffer(static_cast<void *>(remaining_data.data()), remaining_data.size());
        data.RewindRead(0);
        if (stub_ != nullptr) {
            stub_->OnRemoteRequest(code, data, reply, option);
        }
    }

private:
    std::atomic<bool> isInited_;
    std::shared_ptr<WallpaperServiceStubMock> stub_;
};
} // namespace

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    static WallpaperFuzzTest wallpaperFuzzTest;
    if (!wallpaperFuzzTest.IsInited()) {
        return 0;
    }
    FuzzedDataProvider provider(data, size);
    /* Run your code on data */
    wallpaperFuzzTest.DoRemoteRequest(provider);
    return 0;
}