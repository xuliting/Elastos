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

#include "elastos/droid/net/WifiKey.h"
#include <elastos/utility/Objects.h>
#include <elastos/utility/Arrays.h>
#include <elastos/utility/logging/Logger.h>
#include <elastos/utility/regex/Pattern.h>

using Elastos::Utility::Arrays;
using Elastos::Utility::Logging::Logger;
using Elastos::Utility::Objects;
using Elastos::Utility::Regex::IMatcher;
using Elastos::Utility::Regex::Pattern;

namespace Elastos {
namespace Droid {
namespace Net {

const AutoPtr<IPattern> WifiKey::SSID_PATTERN = InitPattern(String("(\".*\")|(0x[\\p{XDigit}]+)"));
const AutoPtr<IPattern> WifiKey::BSSID_PATTERN = InitPattern(String("([\\p{XDigit}]{2}:){5}[\\p{XDigit}]{2}"));

CAR_INTERFACE_IMPL(WifiKey, Object, IParcelable, IWifiKey)

WifiKey::WifiKey()
{}

AutoPtr<IPattern> WifiKey::InitPattern(
    /* [in] */ const String& pattern)
{
    AutoPtr<IPattern> rev;
    Pattern::Compile(pattern, (IPattern**)&rev);
    return rev;
}

ECode WifiKey::constructor()
{
    return NOERROR;
}

ECode WifiKey::constructor(
    /* [in] */ const String& ssid,
    /* [in] */ const String& bssid)
{
    AutoPtr<IMatcher> matcher;
    SSID_PATTERN->Matcher(ssid, (IMatcher**)&matcher);
    Boolean match;
    if (matcher->Matches(&match), !match) {
        Logger::E("WifiKey", "Invalid ssid: %s", mSsid.string());
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    matcher = NULL;
    BSSID_PATTERN->Matcher(bssid, (IMatcher**)&matcher);
    if (matcher->Matches(&match), !match) {
        Logger::E("WifiKey", "Invalid bssid: %s", mBssid.string());
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mSsid = ssid;
    mBssid = bssid;
    return NOERROR;
}

ECode WifiKey::Equals(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (TO_IINTERFACE(this) == IInterface::Probe(o)) {
        *result = TRUE;
        return NOERROR;
    }
    if (o == NULL) {
        *result = FALSE;
        return NOERROR;
    }
    ClassID this_cid, o_cid;
    GetClassID(&this_cid);
    IObject::Probe(o)->GetClassID(&o_cid);
    if (this_cid != o_cid) {
        *result = FALSE;
        return NOERROR;
    }
    AutoPtr<WifiKey> wifiKey = (WifiKey*) IWifiKey::Probe(o);
    *result = mSsid.Equals(wifiKey->mSsid) && mBssid.Equals(wifiKey->mBssid);
    return NOERROR;
}

ECode WifiKey::GetHashCode(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ArrayOf<Int32> > array = ArrayOf<Int32>::Alloc(0);
    (*array)[0] = mSsid.GetHashCode();
    (*array)[1] = mBssid.GetHashCode();
    *result = Arrays::GetHashCode(array);
    return NOERROR;
}

ECode WifiKey::ToString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    *result = String("WifiKey[SSID=") + mSsid + ",BSSID=" + mBssid + "]";
    return NOERROR;
}

ECode WifiKey::ReadFromParcel(
        /* [in] */ IParcel* parcel)
{
    parcel->ReadString(&mSsid);
    parcel->ReadString(&mBssid);
    return NOERROR;
}

ECode WifiKey::WriteToParcel(
        /* [in] */ IParcel* dest)
{
    dest->WriteString(mSsid);
    dest->WriteString(mBssid);
    return NOERROR;
}

ECode WifiKey::GetSsid(
        /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mSsid;
    return NOERROR;
}

ECode WifiKey::GetBssid(
        /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mBssid;
    return NOERROR;
}

} // namespace Net
} // namespace Droid
} // namespace Elastos
