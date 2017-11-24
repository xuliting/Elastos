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

#include "elastos/droid/net/NetworkCapabilities.h"
#include "elastos/droid/net/Network.h"
#include "elastos/droid/text/TextUtils.h"
#include <elastos/core/Math.h>
#include <elastos/core/StringUtils.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Text::TextUtils;

using Elastos::Core::Math;
using Elastos::Core::StringUtils;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Net {

CAR_INTERFACE_IMPL(NetworkCapabilities, Object, IParcelable, INetworkCapabilities)

const String NetworkCapabilities::TAG("NetworkCapabilities");
const Boolean NetworkCapabilities::DBG = FALSE;
const Int32 NetworkCapabilities::MIN_NET_CAPABILITY = INetworkCapabilities::NET_CAPABILITY_MMS;
const Int32 NetworkCapabilities::MAX_NET_CAPABILITY = INetworkCapabilities::NET_CAPABILITY_NOT_VPN;
const Int32 NetworkCapabilities::MIN_TRANSPORT = INetworkCapabilities::TRANSPORT_CELLULAR;
const Int32 NetworkCapabilities::MAX_TRANSPORT = INetworkCapabilities::TRANSPORT_VPN;

NetworkCapabilities::NetworkCapabilities()
    : mNetworkCapabilities((1 << INetworkCapabilities::NET_CAPABILITY_NOT_RESTRICTED) | (1 << INetworkCapabilities::NET_CAPABILITY_TRUSTED) | (1 << INetworkCapabilities::NET_CAPABILITY_NOT_VPN))
{}

ECode NetworkCapabilities::constructor()
{
    return NOERROR;
}

ECode NetworkCapabilities::constructor(
    /* [in] */ INetworkCapabilities* nc)
{
    if (nc != NULL) {
        mNetworkCapabilities = ((NetworkCapabilities*)nc)->mNetworkCapabilities;
        mTransportTypes = ((NetworkCapabilities*)nc)->mTransportTypes;
        mLinkUpBandwidthKbps = ((NetworkCapabilities*)nc)->mLinkUpBandwidthKbps;
        mLinkDownBandwidthKbps = ((NetworkCapabilities*)nc)->mLinkDownBandwidthKbps;
        mNetworkSpecifier = ((NetworkCapabilities*)nc)->mNetworkSpecifier;
    }
    return NOERROR;
}

ECode NetworkCapabilities::AddCapability(
    /* [in] */ Int32 capability)
{
    if (capability < MIN_NET_CAPABILITY || capability > MAX_NET_CAPABILITY) {
        Logger::E(TAG, "NetworkCapability out of range");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mNetworkCapabilities |= 1 << capability;
    return NOERROR;
}

ECode NetworkCapabilities::RemoveCapability(
    /* [in] */ Int32 capability)
{
    if (capability < MIN_NET_CAPABILITY || capability > MAX_NET_CAPABILITY) {
        Logger::E(TAG, "NetworkCapability out of range");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mNetworkCapabilities &= ~(1 << capability);
    return NOERROR;
}

ECode NetworkCapabilities::GetCapabilities(
    /* [out, callee] */ ArrayOf<Int32>** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ArrayOf<Int32> > array = EnumerateBits(mNetworkCapabilities);
    *result = array;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkCapabilities::HasCapability(
    /* [in] */ Int32 capability,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (capability < MIN_NET_CAPABILITY || capability > MAX_NET_CAPABILITY) {
        *result = FALSE;
        return NOERROR;
    }
    *result = ((mNetworkCapabilities & (1 << capability)) != 0);
    return NOERROR;
}

AutoPtr<ArrayOf<Int32> > NetworkCapabilities::EnumerateBits(
    /* [in] */ Int64 val)
{
    Int32 size = Elastos::Core::Math::BitCount(val);
    AutoPtr<ArrayOf<Int32> > result = ArrayOf<Int32>::Alloc(size);
    Int32 index = 0;
    Int32 resource = 0;
    while (val > 0) {
        if ((val & 1) == 1) (*result)[index++] = resource;
        val = val >> 1;
        resource++;
    }
    return result;
}

ECode NetworkCapabilities::CombineNetCapabilities(
    /* [in] */ INetworkCapabilities* nc)
{
    mNetworkCapabilities |= ((NetworkCapabilities*)nc)->mNetworkCapabilities;
    return NOERROR;
}

Boolean NetworkCapabilities::SatisfiedByNetCapabilities(
    /* [in] */ INetworkCapabilities* nc)
{
    return ((((NetworkCapabilities*)nc)->mNetworkCapabilities & mNetworkCapabilities) == mNetworkCapabilities);
}

Boolean NetworkCapabilities::EqualsNetCapabilities(
    /* [in] */ INetworkCapabilities* nc)
{
    return (((NetworkCapabilities*)nc)->mNetworkCapabilities == mNetworkCapabilities);
}

ECode NetworkCapabilities::AddTransportType(
    /* [in] */ Int32 transportType)
{
    if (transportType < MIN_TRANSPORT || transportType > MAX_TRANSPORT) {
        Logger::E(TAG, "TransportType out of range");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mTransportTypes |= 1 << transportType;
    SetNetworkSpecifier(mNetworkSpecifier); // used for exception checking
    return NOERROR;
}

ECode NetworkCapabilities::RemoveTransportType(
    /* [in] */ Int32 transportType)
{
    if (transportType < MIN_TRANSPORT || transportType > MAX_TRANSPORT) {
        Logger::E(TAG, "TransportType out of range");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mTransportTypes &= ~(1 << transportType);
    SetNetworkSpecifier(mNetworkSpecifier); // used for exception checking
    return NOERROR;
}

ECode NetworkCapabilities::GetTransportTypes(
    /* [out, callee] */ ArrayOf<Int32>** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ArrayOf<Int32> > array = EnumerateBits(mTransportTypes);
    *result = array;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode NetworkCapabilities::HasTransport(
    /* [in] */ Int32 transportType,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (transportType < MIN_TRANSPORT || transportType > MAX_TRANSPORT) {
        return FALSE;
    }
    *result = ((mTransportTypes & (1 << transportType)) != 0);
    return NOERROR;
}

ECode NetworkCapabilities::CombineTransportTypes(
    /* [in] */ INetworkCapabilities* nc)
{
    mTransportTypes |= ((NetworkCapabilities*)nc)->mTransportTypes;
    return NOERROR;
}

Boolean NetworkCapabilities::SatisfiedByTransportTypes(
    /* [in] */ INetworkCapabilities* nc)
{
    return ((mTransportTypes == 0) ||
            ((mTransportTypes & ((NetworkCapabilities*)nc)->mTransportTypes) != 0));
}

Boolean NetworkCapabilities::EqualsTransportTypes(
    /* [in] */ INetworkCapabilities* nc)
{
    return (((NetworkCapabilities*)nc)->mTransportTypes == mTransportTypes);
}

ECode NetworkCapabilities::SetLinkUpstreamBandwidthKbps(
    /* [in] */ Int32 upKbps)
{
    mLinkUpBandwidthKbps = upKbps;
    return NOERROR;
}

ECode NetworkCapabilities::GetLinkUpstreamBandwidthKbps(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mLinkUpBandwidthKbps;
    return NOERROR;
}

ECode NetworkCapabilities::SetLinkDownstreamBandwidthKbps(
    /* [in] */ Int32 downKbps)
{
    mLinkDownBandwidthKbps = downKbps;
    return NOERROR;
}

ECode NetworkCapabilities::GetLinkDownstreamBandwidthKbps(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mLinkDownBandwidthKbps;
    return NOERROR;
}

ECode NetworkCapabilities::CombineLinkBandwidths(
    /* [in] */ INetworkCapabilities* nc)
{
    mLinkUpBandwidthKbps =
            Elastos::Core::Math::Max(mLinkUpBandwidthKbps, ((NetworkCapabilities*)nc)->mLinkUpBandwidthKbps);
    mLinkDownBandwidthKbps =
            Elastos::Core::Math::Max(mLinkDownBandwidthKbps, ((NetworkCapabilities*)nc)->mLinkDownBandwidthKbps);
    return NOERROR;
}

Boolean NetworkCapabilities::SatisfiedByLinkBandwidths(
    /* [in] */ INetworkCapabilities* nc)
{
    return !(mLinkUpBandwidthKbps > ((NetworkCapabilities*)nc)->mLinkUpBandwidthKbps ||
            mLinkDownBandwidthKbps > ((NetworkCapabilities*)nc)->mLinkDownBandwidthKbps);
}

Boolean NetworkCapabilities::EqualsLinkBandwidths(
    /* [in] */ INetworkCapabilities* nc)
{
    return (mLinkUpBandwidthKbps == ((NetworkCapabilities*)nc)->mLinkUpBandwidthKbps &&
            mLinkDownBandwidthKbps == ((NetworkCapabilities*)nc)->mLinkDownBandwidthKbps);
}

ECode NetworkCapabilities::SetNetworkSpecifier(
    /* [in] */ const String& networkSpecifier)
{
    if (TextUtils::IsEmpty(networkSpecifier) == FALSE && Elastos::Core::Math::BitCount(mTransportTypes) != 1) {
        Logger::E(TAG, ("Must have a single transport specified to use setNetworkSpecifier"));
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    mNetworkSpecifier = networkSpecifier;
    return NOERROR;
}

ECode NetworkCapabilities::GetNetworkSpecifier(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mNetworkSpecifier;
    return NOERROR;
}

ECode NetworkCapabilities::CombineSpecifiers(
    /* [in] */ INetworkCapabilities* nc)
{
    String otherSpecifier;
    ((NetworkCapabilities*)nc)->GetNetworkSpecifier(&otherSpecifier);
    if (TextUtils::IsEmpty(otherSpecifier)) return NOERROR;
    if (TextUtils::IsEmpty(mNetworkSpecifier) == FALSE) {
        Logger::E(TAG, "Can't combine two networkSpecifiers");
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    SetNetworkSpecifier(otherSpecifier);
    return NOERROR;
}

Boolean NetworkCapabilities::SatisfiedBySpecifier(
    /* [in] */ INetworkCapabilities* nc)
{
    return (TextUtils::IsEmpty(mNetworkSpecifier) ||
            mNetworkSpecifier.Equals(((NetworkCapabilities*)nc)->mNetworkSpecifier));
}

Boolean NetworkCapabilities::EqualsSpecifier(
    /* [in] */ INetworkCapabilities* nc)
{
    if (TextUtils::IsEmpty(mNetworkSpecifier)) {
        return TextUtils::IsEmpty(((NetworkCapabilities*)nc)->mNetworkSpecifier);
    }
    return mNetworkSpecifier.Equals(((NetworkCapabilities*)nc)->mNetworkSpecifier);
}

ECode NetworkCapabilities::CombineCapabilities(
    /* [in] */ INetworkCapabilities* nc)
{
    CombineNetCapabilities(nc);
    CombineTransportTypes(nc);
    CombineLinkBandwidths(nc);
    CombineSpecifiers(nc);
    return NOERROR;
}

ECode NetworkCapabilities::SatisfiedByNetworkCapabilities(
    /* [in] */ INetworkCapabilities* nc,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    *result = nc != NULL &&
            SatisfiedByNetCapabilities(nc) &&
            SatisfiedByTransportTypes(nc) &&
            SatisfiedByLinkBandwidths(nc) &&
            SatisfiedBySpecifier(nc);
    return NOERROR;
}

ECode NetworkCapabilities::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (TO_IINTERFACE(this) != IInterface::Probe(obj)) {
        *result = FALSE;
        return NOERROR;
    }
    if (obj == NULL || (INetworkCapabilities::Probe(obj) == NULL)) {
        *result = FALSE;
        return NOERROR;
    }
    AutoPtr<NetworkCapabilities> that = (NetworkCapabilities*)INetworkCapabilities::Probe(obj);
    *result = EqualsNetCapabilities(that) &&
            EqualsTransportTypes(that) &&
            EqualsLinkBandwidths(that) &&
            EqualsSpecifier(that);
    return NOERROR;
}

ECode NetworkCapabilities::GetHashCode(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    *result = (Int32)(mNetworkCapabilities & 0xFFFFFFFF) +
            ((Int32)(mNetworkCapabilities >> 32) * 3) +
            ((Int32)(mTransportTypes & 0xFFFFFFFF) * 5) +
            ((Int32)(mTransportTypes >> 32) * 7) +
            (mLinkUpBandwidthKbps * 11) +
            (mLinkDownBandwidthKbps * 13) +
            (TextUtils::IsEmpty(mNetworkSpecifier) ? 0 : mNetworkSpecifier.GetHashCode() * 17);
    return NOERROR;
}

ECode NetworkCapabilities::ReadFromParcel(
    /* [in] */ IParcel* parcel)
{
    parcel->ReadInt64(&mTransportTypes);
    parcel->ReadInt32(&mLinkUpBandwidthKbps);
    parcel->ReadInt32(&mLinkDownBandwidthKbps);
    parcel->ReadString(&mNetworkSpecifier);
    parcel->ReadInt64(&mNetworkCapabilities);
    return NOERROR;
}

ECode NetworkCapabilities::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteInt64(mTransportTypes);
    dest->WriteInt32(mLinkUpBandwidthKbps);
    dest->WriteInt32(mLinkDownBandwidthKbps);
    dest->WriteString(mNetworkSpecifier);
    dest->WriteInt64(mNetworkCapabilities);
    return NOERROR;
}

ECode NetworkCapabilities::ToString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ArrayOf<Int32> > types;
    GetTransportTypes((ArrayOf<Int32>**)&types);
    String transports = (types->GetLength() > 0 ? String(" Transports: ") : String(""));
    for (Int32 i = 0; i < types->GetLength();) {
        switch ((*types)[i]) {
            case TRANSPORT_CELLULAR:
                transports += "CELLULAR";
                break;
            case TRANSPORT_WIFI:
                transports += "WIFI";
                break;
            case TRANSPORT_BLUETOOTH:
                transports += "BLUETOOTH";
                break;
            case TRANSPORT_ETHERNET:
                transports += "ETHERNET";
                break;
            case TRANSPORT_VPN:
                transports += "VPN";
                break;
        }
        if (++i < types->GetLength()) transports += "|";
    }
    types = NULL;
    GetCapabilities((ArrayOf<Int32>**)&types);
    String capabilities = (types->GetLength() > 0 ? String(" Capabilities: ") : String(""));
    for (Int32 i = 0; i < types->GetLength(); ) {
        switch ((*types)[i]) {
            case NET_CAPABILITY_MMS:
                capabilities += "MMS";
                break;
            case NET_CAPABILITY_SUPL:
                capabilities += "SUPL";
                break;
            case NET_CAPABILITY_DUN:
                capabilities += "DUN";
                break;
            case NET_CAPABILITY_FOTA:
                capabilities += "FOTA";
                break;
            case NET_CAPABILITY_IMS:
                capabilities += "IMS";
                break;
            case NET_CAPABILITY_CBS:
                capabilities += "CBS";
                break;
            case NET_CAPABILITY_WIFI_P2P:
                capabilities += "WIFI_P2P";
                break;
            case NET_CAPABILITY_IA:
                capabilities += "IA";
                break;
            case NET_CAPABILITY_RCS:
                capabilities += "RCS";
                break;
            case NET_CAPABILITY_XCAP:
                capabilities += "XCAP";
                break;
            case NET_CAPABILITY_EIMS:
                capabilities += "EIMS";
                break;
            case NET_CAPABILITY_NOT_METERED:
                capabilities += "NOT_METERED";
                break;
            case NET_CAPABILITY_INTERNET:
                capabilities += "INTERNET";
                break;
            case NET_CAPABILITY_NOT_RESTRICTED:
                capabilities += "NOT_RESTRICTED";
                break;
            case NET_CAPABILITY_TRUSTED:
                capabilities += "TRUSTED";
                break;
            case NET_CAPABILITY_NOT_VPN:
                capabilities += "NOT_VPN";
                break;
        }
        if (++i < types->GetLength()) capabilities += "&";
    }
    String upBand = ((mLinkUpBandwidthKbps > 0) ? String(" LinkUpBandwidth>=") +
            StringUtils::ToString(mLinkUpBandwidthKbps) + "Kbps" : String(""));
    String dnBand = ((mLinkDownBandwidthKbps > 0) ? String(" LinkDnBandwidth>=") +
            StringUtils::ToString(mLinkDownBandwidthKbps) + "Kbps" : String(""));
    String specifier = (mNetworkSpecifier == NULL ?
            String("") : String(" Specifier: <") + mNetworkSpecifier + ">");
    *result = String("[") + transports + capabilities + upBand + dnBand + specifier + "]";
    return NOERROR;
}

} // namespace Net
} // namespace Droid
} // namespace Elastos
