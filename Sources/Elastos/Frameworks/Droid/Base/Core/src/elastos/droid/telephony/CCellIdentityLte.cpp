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

#include "elastos/droid/telephony/CCellIdentityLte.h"
#include <elastos/core/Math.h>
#include <elastos/core/StringBuilder.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Core::StringBuilder;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Telephony {

const String CCellIdentityLte::TAG("CCellIdentityLte");
const Boolean CCellIdentityLte::DBG;

CAR_INTERFACE_IMPL(CCellIdentityLte, Object, ICellIdentityLte, IParcelable)

CAR_OBJECT_IMPL(CCellIdentityLte)

CCellIdentityLte::CCellIdentityLte()
    : mMcc(Elastos::Core::Math::INT32_MAX_VALUE)
    , mMnc(Elastos::Core::Math::INT32_MAX_VALUE)
    , mCi(Elastos::Core::Math::INT32_MAX_VALUE)
    , mPci(Elastos::Core::Math::INT32_MAX_VALUE)
    , mTac(Elastos::Core::Math::INT32_MAX_VALUE)
{
}

CCellIdentityLte::~CCellIdentityLte()
{
}

ECode CCellIdentityLte::constructor()
{
    return NOERROR;
}

ECode CCellIdentityLte::constructor(
    /* [in] */ Int32 mcc,
    /* [in] */ Int32 mnc,
    /* [in] */ Int32 ci,
    /* [in] */ Int32 pci,
    /* [in] */ Int32 tac)
{
    mMcc = mcc;
    mMnc = mnc;
    mCi = ci;
    mPci = pci;
    mTac = tac;
    return NOERROR;
}

ECode CCellIdentityLte::constructor(
    /* [in] */ ICellIdentityLte* cid)
{
    cid->GetMcc(&mMcc);
    cid->GetMnc(&mMnc);
    cid->GetCi(&mCi);
    cid->GetPci(&mPci);
    cid->GetTac(&mTac);
    return NOERROR;
}

ECode CCellIdentityLte::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    source->ReadInt32(&mMcc);
    source->ReadInt32(&mMnc);
    source->ReadInt32(&mCi);
    source->ReadInt32(&mPci);
    source->ReadInt32(&mTac);
    return NOERROR;
}

ECode CCellIdentityLte::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    if (DBG) {
        String str;
        ToString(&str);
        Logger::I(TAG, "writeToParcel(Parcel, int): %s", str.string());
    }
    dest->WriteInt32(mMcc);
    dest->WriteInt32(mMnc);
    dest->WriteInt32(mCi);
    dest->WriteInt32(mPci);
    dest->WriteInt32(mTac);
    return NOERROR;
}

ECode CCellIdentityLte::Copy(
    /* [out] */ ICellIdentityLte** cid)
{
    CCellIdentityLte::New(this, cid);
    return NOERROR;
}

ECode CCellIdentityLte::GetMcc(
    /* [out] */ Int32* mcc)
{
    VALIDATE_NOT_NULL(mcc);
    *mcc = mMcc;
    return NOERROR;
}

ECode CCellIdentityLte::GetMnc(
    /* [out] */ Int32* mnc)
{
    VALIDATE_NOT_NULL(mnc);
    *mnc = mMnc;
    return NOERROR;
}

ECode CCellIdentityLte::GetCi(
    /* [out] */ Int32* ci)
{
    VALIDATE_NOT_NULL(ci);
    *ci = mCi;
    return NOERROR;
}

ECode CCellIdentityLte::GetPci(
    /* [out] */ Int32* pci)
{
    VALIDATE_NOT_NULL(pci);
    *pci = mPci;
    return NOERROR;
}

ECode CCellIdentityLte::GetTac(
    /* [out] */ Int32* tac)
{
    VALIDATE_NOT_NULL(tac);
    *tac = mTac;
    return NOERROR;
}

ECode CCellIdentityLte::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode);

    Int32 primeNum = 31;
    *hashCode = ((mMcc * primeNum) + (mMnc * primeNum) + (mCi * primeNum) + (mPci * primeNum) +
            (mTac * primeNum));

    return NOERROR;
}

ECode CCellIdentityLte::Equals(
    /* [in] */ IInterface* other,
    /* [out] */ Boolean* res)
{
    VALIDATE_NOT_NULL(res);

    //if (super.equals(other)) {
    //try {
    if (ICellIdentityLte::Probe(other) == NULL) {
        *res = FALSE;
        return NOERROR;
    }

    AutoPtr<ICellIdentityLte> o = ICellIdentityLte::Probe(other);
    Int32 mcc, mnc, ci, pci, tac;
    o->GetMcc(&mcc);
    o->GetMnc(&mnc);
    o->GetPci(&pci);
    o->GetCi(&ci);
    o->GetTac(&tac);
    *res = (mMcc == mcc &&
            mMnc == mnc &&
            mCi == ci &&
            mPci == pci &&
            mTac == tac);
    //     } catch (ClassCastException e) {
    //         return false;
    //     }
    // } else {
    //     return false;
    // }

    return NOERROR;
}

ECode CCellIdentityLte::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str);
    StringBuilder sb("CellIdentityLte:{");
    sb.Append(" mMcc="); sb.Append(mMcc);
    sb.Append(" mMnc="); sb.Append(mMnc);
    sb.Append(" mCi="); sb.Append(mCi);
    sb.Append(" mPci="); sb.Append(mPci);
    sb.Append(" mTac="); sb.Append(mTac);
    sb.Append("}");

    *str = sb.ToString();

    return NOERROR;
}

} // namespace Telephony
} // namespace Droid
} // namespace Elastos
