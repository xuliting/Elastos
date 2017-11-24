//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include "elastos/droid/net/NetworkTemplate.h"
#include "elastos/droid/content/res/CResources.h"
#include "elastos/droid/R.h"
#include "elastos/droid/internal/utility/ArrayUtils.h"
#include "elastos/droid/net/CNetwork.h"
#include "elastos/droid/net/CNetworkIdentity.h"
#include "elastos/droid/net/CNetworkTemplate.h"
#include "elastos/droid/net/Network.h"
#include "elastos/droid/net/NetworkIdentity.h"
#include "elastos/droid/os/Build.h"
#include "elastos/droid/telephony/CTelephonyManager.h"
#include <elastos/core/StringBuilder.h>
#include <elastos/utility/logging/Logger.h>
#include <elastos/utility/logging/Slogger.h>

using Elastos::Droid::Content::Res::CResources;
using Elastos::Droid::Internal::Utility::ArrayUtils;
using Elastos::Droid::Net::IConnectivityManager;
using Elastos::Droid::Os::Build;
using Elastos::Droid::R;
using Elastos::Droid::Telephony::ITelephonyManager;
using Elastos::Droid::Telephony::CTelephonyManager;
using Elastos::Core::StringBuilder;
using Elastos::Utility::Logging::Logger;
using Elastos::Utility::Logging::Slogger;

namespace Elastos {
namespace Droid {
namespace Net {

CAR_INTERFACE_IMPL(NetworkTemplate, Object, IParcelable, INetworkTemplate)

Boolean NetworkTemplate::sForceAllNetworkTypes = FALSE;

AutoPtr<ArrayOf<Int32> > NetworkTemplate::DATA_USAGE_NETWORK_TYPES;

AutoPtr<ArrayOf<Int32> > NetworkTemplate::GetDataUsageNetworkTypes()
{
    if (DATA_USAGE_NETWORK_TYPES == NULL) {
        CResources::GetSystem()->GetInt32Array(
                R::array::config_data_usage_network_types, (ArrayOf<Int32>**)&DATA_USAGE_NETWORK_TYPES);
    }
    return DATA_USAGE_NETWORK_TYPES;
}

NetworkTemplate::NetworkTemplate()
{}

ECode NetworkTemplate::ForceAllNetworkTypes()
{
    sForceAllNetworkTypes = TRUE;
    return NOERROR;
}

ECode NetworkTemplate::BuildTemplateMobileAll(
    /* [in] */ const String& subscriberId,
    /* [out] */ INetworkTemplate** result)
{
    VALIDATE_NOT_NULL(result);

    String str;
    AutoPtr<INetworkTemplate> networktemplate;
    CNetworkTemplate::New(MATCH_MOBILE_ALL, subscriberId, str, (INetworkTemplate**)&networktemplate);
    *result = networktemplate;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkTemplate::BuildTemplateMobile3gLower(
    /* [in] */ const String& subscriberId,
    /* [out] */ INetworkTemplate** result)
{
    VALIDATE_NOT_NULL(result);

    String str;
    AutoPtr<INetworkTemplate> networktemplate;
    CNetworkTemplate::New(MATCH_MOBILE_3G_LOWER, subscriberId, str, (INetworkTemplate**)&networktemplate);
    *result = networktemplate;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkTemplate::BuildTemplateMobile4g(
    /* [in] */ const String& subscriberId,
    /* [out] */ INetworkTemplate** result)
{
    VALIDATE_NOT_NULL(result);

    String str;
    AutoPtr<INetworkTemplate> networktemplate;
    CNetworkTemplate::New(MATCH_MOBILE_4G, subscriberId, str, (INetworkTemplate**)&networktemplate);
    *result = networktemplate;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkTemplate::BuildTemplateMobileWildcard(
    /* [out] */ INetworkTemplate** result)
{
    VALIDATE_NOT_NULL(result);

    String str;
    AutoPtr<INetworkTemplate> networktemplate;
    CNetworkTemplate::New(MATCH_MOBILE_WILDCARD, str, str, (INetworkTemplate**)&networktemplate);
    *result = networktemplate;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkTemplate::BuildTemplateWifiWildcard(
    /* [out] */ INetworkTemplate** result)
{
    VALIDATE_NOT_NULL(result);

    String str;
    AutoPtr<INetworkTemplate> networktemplate;
    CNetworkTemplate::New(MATCH_WIFI_WILDCARD, str, str, (INetworkTemplate**)&networktemplate);
    *result = networktemplate;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkTemplate::BuildTemplateWifi(
    /* [out] */ INetworkTemplate** result)
{
    return BuildTemplateWifiWildcard(result);
}

ECode NetworkTemplate::BuildTemplateWifi(
    /* [in] */ const String& networkId,
    /* [out] */ INetworkTemplate** result)
{
    VALIDATE_NOT_NULL(result);

    String str;
    AutoPtr<INetworkTemplate> networktemplate;
    CNetworkTemplate::New(MATCH_WIFI, str, networkId, (INetworkTemplate**)&networktemplate);
    *result = networktemplate;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkTemplate::BuildTemplateEthernet(
    /* [out] */ INetworkTemplate** result)
{
    VALIDATE_NOT_NULL(result);

    String str;
    AutoPtr<INetworkTemplate> networktemplate;
    CNetworkTemplate::New(MATCH_ETHERNET, str, str, (INetworkTemplate**)&networktemplate);
    *result = networktemplate;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkTemplate::constructor()
{
    return NOERROR;
}

ECode NetworkTemplate::constructor(
    /* [in] */ Int32 matchRule,
    /* [in] */ const String& subscriberId,
    /* [in] */ const String& networkId)
{
    mMatchRule = matchRule;
    mSubscriberId = subscriberId;
    mNetworkId = networkId;
    return NOERROR;
}

NetworkTemplate::NetworkTemplate(
    /* [in] */ IParcel* in)
{
    ReadFromParcel(in);
}

ECode NetworkTemplate::ToString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result);

    StringBuilder builder("NetworkTemplate: ");
    builder += String("matchRule=");
    String s;
    GetMatchRuleName(mMatchRule, &s);
    builder += s;
    if (!(mSubscriberId.IsNull())) {
        builder += String(", subscriberId=");
        String subscriberId;
        CNetworkIdentity::ScrubSubscriberId(mSubscriberId, &subscriberId);
        builder += subscriberId;
    }
    if (!(mNetworkId.IsNull())) {
        builder += String(", networkId=");
        builder += mNetworkId;
    }
    *result = builder.ToString();
    return NOERROR;
}

ECode NetworkTemplate::GetHashCode(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);

    Int32 hashCode = 1;
    hashCode = 31 * hashCode + mMatchRule;
    hashCode = 31 * hashCode + mSubscriberId.GetHashCode();
    hashCode = 31 * hashCode + mNetworkId.GetHashCode();
    *result = hashCode;
    return NOERROR;
}

ECode NetworkTemplate::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    if (TO_IINTERFACE(this) == IInterface::Probe(obj)) {
        *result = TRUE;
        return NOERROR;
    }
    if (INetworkTemplate::Probe(obj) != NULL) {
        AutoPtr<INetworkTemplate> other = INetworkTemplate::Probe(obj);
        Int32 matchrule;
        other->GetMatchRule(&matchrule);
        String subscriberId;
        other->GetSubscriberId(&subscriberId);
        String networkId;
        other->GetNetworkId(&networkId);
        *result = mMatchRule == matchrule &&
                  mSubscriberId.Equals(subscriberId) &&
                  mNetworkId.Equals(networkId);
        return NOERROR;
    }
    *result = FALSE;
    return NOERROR;
}

ECode NetworkTemplate::GetMatchRule(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);

    *result = mMatchRule;
    return NOERROR;
}

ECode NetworkTemplate::GetSubscriberId(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result);

    *result = mSubscriberId;
    return NOERROR;
}

ECode NetworkTemplate::GetNetworkId(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result);

    *result = mNetworkId;
    return NOERROR;
}

ECode NetworkTemplate::Matches(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    switch (mMatchRule) {
        case MATCH_MOBILE_ALL:
            MatchesMobile(ident, result);
        case MATCH_MOBILE_3G_LOWER:
            MatchesMobile3gLower(ident, result);
        case MATCH_MOBILE_4G:
            MatchesMobile4g(ident, result);
        case MATCH_WIFI:
            MatchesWifi(ident, result);
        case MATCH_ETHERNET:
            MatchesEthernet(ident, result);
        case MATCH_MOBILE_WILDCARD:
            MatchesMobileWildcard(ident, result);
        case MATCH_WIFI_WILDCARD:
            MatchesWifiWildcard(ident, result);
        default:
            Logger::E("NetworkTemplate", "unknown network template");
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return NOERROR;
}

ECode NetworkTemplate::MatchesMobile(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    Int32 type;
    ident->GetType(&type);
    if (type == IConnectivityManager::TYPE_WIMAX) {
        // TODO: consider matching against WiMAX subscriber identity
        return TRUE;
    }
    else {
        Boolean bol = ArrayUtils::Contains(GetDataUsageNetworkTypes().Get(), type);
        String subscriberid;
        ident->GetSubscriberId(&subscriberid);
        return ((sForceAllNetworkTypes || bol) && mSubscriberId.Equals(subscriberid));
    }
}

ECode NetworkTemplate::MatchesMobile3gLower(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    EnsureSubtypeAvailable();
    Int32 type;
    ident->GetType(&type);
    Boolean isMatchesMobile;
    MatchesMobile(ident, &isMatchesMobile);
    if (type == IConnectivityManager::TYPE_WIMAX) {
        return FALSE;
    } else if (isMatchesMobile) {
        Int32 subtype;
        ident->GetSubType(&subtype);
        Int32 type;
        CTelephonyManager::GetNetworkClass(subtype, &type);
        switch (type) {
            case ITelephonyManager::NETWORK_CLASS_UNKNOWN:
            case ITelephonyManager::NETWORK_CLASS_2_G:
            case ITelephonyManager::NETWORK_CLASS_3_G:
                return TRUE;
        }
    }
    return FALSE;
}

ECode NetworkTemplate::MatchesMobile4g(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    EnsureSubtypeAvailable();
    Int32 type;
    ident->GetType(&type);
    Boolean isMatchesMobile;
    MatchesMobile(ident, &isMatchesMobile);
    if (type == IConnectivityManager::TYPE_WIMAX) {
        // TODO: consider matching against WiMAX subscriber identity
        return TRUE;
    } else if (isMatchesMobile) {
        Int32 subtype;
        ident->GetSubType(&subtype);
        Int32 type;
        CTelephonyManager::GetNetworkClass(subtype, &type);
        switch (type) {
            case ITelephonyManager::NETWORK_CLASS_4_G:
                return TRUE;
        }
    }
    return FALSE;
}

ECode NetworkTemplate::MatchesWifi(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    Int32 type;
    ident->GetType(&type);
    switch (type) {
        case IConnectivityManager::TYPE_WIFI: {
            String IdenNetworkid;
            ident->GetNetworkId(&IdenNetworkid);
            return mNetworkId.Equals(IdenNetworkid);
        }
        default:
            return FALSE;
    }
}

ECode NetworkTemplate::MatchesEthernet(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    Int32 type;
    ident->GetType(&type);
    if (type == IConnectivityManager::TYPE_ETHERNET) {
        return TRUE;
    }
    return FALSE;
}

ECode NetworkTemplate::MatchesMobileWildcard(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    Int32 type;
    ident->GetType(&type);
    if (type == IConnectivityManager::TYPE_WIMAX) {
        return TRUE;
    } else {
        Boolean bol = ArrayUtils::Contains(DATA_USAGE_NETWORK_TYPES.Get(), type);
        return sForceAllNetworkTypes || bol;
    }
}

ECode NetworkTemplate::MatchesWifiWildcard(
    /* [in] */ INetworkIdentity* ident,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    Int32 type;
    ident->GetType(&type);
    switch (type) {
        case IConnectivityManager::TYPE_WIFI:
        case IConnectivityManager::TYPE_WIFI_P2P:
            return TRUE;
        default:
            return FALSE;
    }
}

ECode NetworkTemplate::GetMatchRuleName(
    /* [in] */ Int32 matchRule,
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result);

    switch (matchRule) {
        case MATCH_MOBILE_3G_LOWER:
            *result = String("MOBILE_3G_LOWER");
            return NOERROR;
        case MATCH_MOBILE_4G:
            *result = String("MOBILE_4G");
            return NOERROR;
        case MATCH_MOBILE_ALL:
            *result = String("MOBILE_ALL");
            return NOERROR;
        case MATCH_WIFI:
            *result = String("WIFI");
            return NOERROR;
        case MATCH_ETHERNET:
            *result = String("ETHERNET");
            return NOERROR;
        case MATCH_MOBILE_WILDCARD:
            *result = String("MOBILE_WILDCARD");
            return NOERROR;
        case MATCH_WIFI_WILDCARD:
            *result = String("WIFI_WILDCARD");
            return NOERROR;
        default:
            *result = String("UNKNOWN");
            return NOERROR;
    }
    return NOERROR;
}

ECode NetworkTemplate::EnsureSubtypeAvailable()
{
    if (INetworkIdentity::COMBINE_SUBTYPE_ENABLED) {
        // throw new IllegalArgumentException(
        //         "Unable to enforce 3G_LOWER template on combined data.");
        Slogger::E("NetworkTemplate", "Unable to enforce 3G_LOWER template on combined data.");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return NOERROR;
}

ECode NetworkTemplate::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteInt32(mMatchRule);
    dest->WriteString(mSubscriberId);
    dest->WriteString(mNetworkId);
    return NOERROR;
}

ECode NetworkTemplate::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    source->ReadInt32(&mMatchRule);
    source->ReadString(&mSubscriberId);
    source->ReadString(&mNetworkId);
    return NOERROR;
}

} // namespace Net
} // namespace Droid
} // namespace Elastos
