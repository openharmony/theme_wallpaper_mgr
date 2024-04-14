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

#include "wallpaper_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t WallpaperExtensionContext::CONTEXT_TYPE_ID(std::hash<const char *>{}("WallpaperExtensionContext"));
int32_t WallpaperExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode WallpaperExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    HILOG_DEBUG("%{public}s. End calling StartAbility. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("WallpaperContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode WallpaperExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, ILLEGAL_REQUEST_CODE);
    HILOG_DEBUG("%{public}s. End calling StartAbility. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("WallpaperContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

bool WallpaperExtensionContext::ConnectAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode ret = ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    HILOG_INFO("WallpaperExtensionContext::ConnectAbility ErrorCode = %{public}d", ret);
    return ret == ERR_OK;
}

ErrCode WallpaperExtensionContext::StartAbilityWithAccount(const AAFwk::Want &want, int32_t accountId) const
{
    HILOG_INFO("%{public}s begin. %{public}d accountId:", __func__, accountId);
    ErrCode err =
        AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE, accountId);
    HILOG_DEBUG("%{public}s. End calling StartAbilityWithAccount. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("WallpaperContext::StartAbilityWithAccount is failed %{public}d", err);
    }
    return err;
}

ErrCode WallpaperExtensionContext::StartAbilityWithAccount(const AAFwk::Want &want, int32_t accountId,
    const AAFwk::StartOptions &startOptions) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE, accountId);
    HILOG_DEBUG("%{public}s. End calling StartAbilityWithAccount. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("WallpaperContext::StartAbilityWithAccount is failed %{public}d", err);
    }
    return err;
}

bool WallpaperExtensionContext::ConnectAbilityWithAccount(const AAFwk::Want &want, int32_t accountId,
    const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode ret = ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    HILOG_INFO("WallpaperExtensionContext::ConnectAbilityWithAccount ErrorCode = %{public}d", ret);
    return ret == ERR_OK;
}

ErrCode WallpaperExtensionContext::DisconnectAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode ret = ConnectionManager::GetInstance().DisconnectAbility(token_, want.GetElement(), connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s end DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
    return ret;
}

ErrCode WallpaperExtensionContext::TerminateAbility()
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        HILOG_ERROR("WallpaperExtensionContext::TerminateAbility is failed %{public}d", err);
    }
    return err;
}

AppExecFwk::AbilityType WallpaperExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("WallpaperContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}
} // namespace AbilityRuntime
} // namespace OHOS