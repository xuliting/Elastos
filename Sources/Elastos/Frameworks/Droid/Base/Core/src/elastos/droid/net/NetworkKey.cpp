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

#include "elastos/droid/net/NetworkKey.h"
#include "elastos/droid/net/Network.h"
#include <elastos/utility/Arrays.h>
#include <elastos/utility/Objects.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Utility::Arrays;
using Elastos::Utility::Logging::Logger;
using Elastos::Utility::Objects;

namespace Elastos {
namespace Droid {
namespace Net {

CAR_INTERFACE_IMPL(NetworkKey, Object, IParcelable, INetworkKey)

ECode NetworkKey::constructor()
{
    return NOERROR;
}

ECode NetworkKey::constructor(
    /* [in] */ IWifiKey* wifiKey)
{
    mType = TYPE_WIFI;
    mWifiKey = wifiKey;
    return NOERROR;
}

NetworkKey::NetworkKey()
    : mType(0)
{}

ECode NetworkKey::Equals(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (TO_IINTERFACE(this) == IInterface::Probe(o)) {
        *result = TRUE;
        return NOERROR;
    }
    ClassID this_cid, o_cid;
    GetClassID(&this_cid);
    if (o == NULL || (IObject::Probe(o)->GetClassID(&o_cid), this_cid != o_cid)) {
        *result = FALSE;
        return NOERROR;
    }
    NetworkKey* that = (NetworkKey*)INetworkKey::Probe(o);
    *result = mType == that->mType && Objects::Equals(mWifiKey, that->mWifiKey);
    return NOERROR;
}

ECode NetworkKey::GetHashCode(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ArrayOf<Int32> > array = ArrayOf<Int32>::Alloc(5);
    (*array)[0] = mType;
    Int32 hashCode;
    IObject::Probe(mWifiKey)->GetHashCode(&hashCode);
    (*array)[1] = hashCode;
    *result = Arrays::GetHashCode(array);
    return NOERROR;
}

ECode NetworkKey::ToString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    switch (mType) {
        case TYPE_WIFI:
            return IObject::Probe(mWifiKey)->ToString(result);
        default:
            // Don't throw an exception here in case someone is logging this object in a catch
            // block for debugging purposes.
            *result = String("InvalidKey");
            return NOERROR;
    }
    return NOERROR;
}

ECode NetworkKey::ReadFromParcel(
    /* [in] */ IParcel* parcel)
{
    parcel->ReadInt32(&mType);
    AutoPtr<IInterface> obj;
    switch (mType) {
        case TYPE_WIFI:
            parcel->ReadInterfacePtr((Handle32*)&obj);
            mWifiKey = IWifiKey::Probe(obj);
            break;
        default:
            Logger::E("NetworkKey", "Parcel has unknown type: %d", mType);
            return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    return NOERROR;
}

ECode NetworkKey::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteInt32(mType);
    dest->WriteInterfacePtr(mWifiKey);
    return NOERROR;
}

ECode NetworkKey::GetType(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mType;
    return NOERROR;
}

ECode NetworkKey::GetWifiKey(
    /* [out] */ IWifiKey** result)
{
    VALIDATE_NOT_NULL(result)

    *result = mWifiKey;
    REFCOUNT_ADD(*result)
    return NOERROR;
}

} // namespace Net
} // namespace Droid
} // namespace Elastos
