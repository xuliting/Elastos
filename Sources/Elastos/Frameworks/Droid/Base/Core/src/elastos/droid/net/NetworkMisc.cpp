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

#include "elastos/droid/net/NetworkMisc.h"

namespace Elastos {
namespace Droid {
namespace Net {

CAR_INTERFACE_IMPL(NetworkMisc, Object, IParcelable, INetworkMisc)

NetworkMisc::NetworkMisc()
    : mAllowBypass(FALSE)
    , mExplicitlySelected(FALSE)
{}

ECode NetworkMisc::constructor()
{
    return NOERROR;
}

ECode NetworkMisc::constructor(
    /* [in] */ INetworkMisc* nm)
{
    if (nm != NULL) {
        mAllowBypass = ((NetworkMisc*)nm)->mAllowBypass;
        mExplicitlySelected = ((NetworkMisc*)nm)->mExplicitlySelected;
    }
    return NOERROR;
}

ECode NetworkMisc::ReadFromParcel(
    /* [in] */ IParcel* parcel)
{
    Int32 i;
    parcel->ReadInt32(&i);
    mAllowBypass = i != 0;
    parcel->ReadInt32(&i);
    mExplicitlySelected = i != 0;
    return NOERROR;
}

ECode NetworkMisc::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteInt32(mAllowBypass ? 1 : 0);
    dest->WriteInt32(mExplicitlySelected ? 1 : 0);
    return NOERROR;
}

ECode NetworkMisc::GetAllowBypass(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mAllowBypass;
    return NOERROR;
}

ECode NetworkMisc::SetAllowBypass(
    /* [in] */ Boolean allowBypass)
{
    mAllowBypass = allowBypass;
    return NOERROR;
}

ECode NetworkMisc::GetExplicitlySelected(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mExplicitlySelected;
    return NOERROR;
}

ECode NetworkMisc::SetExplicitlySelected(
    /* [in] */ Boolean explicitlySelected)
{
    mExplicitlySelected = explicitlySelected;
    return NOERROR;
}

} // namespace Net
} // namespace Droid
} // namespace Elastos
