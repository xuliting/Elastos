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

#include "CCodeSigner.h"
#include "StringBuilder.h"
#include "Elastos.CoreLibrary.Utility.h"

using Elastos::Core::StringBuilder;
using Elastos::IO::EIID_ISerializable;
using Elastos::Utility::IList;

namespace Elastos {
namespace Security {

CAR_OBJECT_IMPL(CCodeSigner)
CAR_INTERFACE_IMPL(CCodeSigner, Object, ICodeSigner, ISerializable)
CCodeSigner::CCodeSigner()
    : mHash(0)
{}

ECode CCodeSigner::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    if (TO_IINTERFACE(this) == obj) {
        *result = TRUE;
        return NOERROR;
    }
    if (ICodeSigner::Probe(obj)) {
        AutoPtr<ICodeSigner> that = ICodeSigner::Probe(obj);
        Boolean isEqual;
        if ((IObject::Probe(mSignerCertPath)->Equals(((CCodeSigner*)that.Get())->mSignerCertPath, &isEqual), !isEqual)) {
            *result = FALSE;
            return NOERROR;
        }
        if (mTimestamp == NULL) {
            *result = ((CCodeSigner*)that.Get())->mTimestamp == NULL;
        }
        else {
            mTimestamp->Equals(((CCodeSigner*)that.Get())->mTimestamp, result);
        }
        return NOERROR;
    }
    *result = FALSE;
    return NOERROR;
}

ECode CCodeSigner::GetSignerCertPath(
    /* [out] */ ICertPath** certPath)
{
    VALIDATE_NOT_NULL(certPath)
    *certPath = mSignerCertPath;
    REFCOUNT_ADD(*certPath)
    return NOERROR;
}

ECode CCodeSigner::GetTimestamp(
    /* [out] */ ITimestamp** timestamp)
{
    VALIDATE_NOT_NULL(timestamp)
    *timestamp = mTimestamp;
    REFCOUNT_ADD(*timestamp)
    return NOERROR;
}

ECode CCodeSigner::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode)
    if (mHash == 0) {
        IObject::Probe(mSignerCertPath)->GetHashCode(&mHash);
        Int32 hash = mTimestamp == NULL ? 0 : mTimestamp->GetHashCode(&hash);
        mHash ^= hash;
    }
    *hashCode = mHash;
    return NOERROR;
}

ECode CCodeSigner::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    // There is no any special reason for '256' here, it's taken abruptly
    StringBuilder buf("CodeSigner [");
    // The javadoc says nothing, and the others implementations behavior seems as
    // dumping only the first certificate. Well, let's do the same.
    AutoPtr<IList> certificates;
    mSignerCertPath->GetCertificates((IList**)&certificates);
    AutoPtr<IInterface> cert;
    certificates->Get(0, (IInterface**)&cert);
    buf.Append(cert);
    if (mTimestamp != NULL) {
        buf.Append("; ");
        buf.Append(mTimestamp);
    }
    buf.Append("]");
    buf.ToString(str);
    return NOERROR;
}

ECode CCodeSigner::constructor()
{
    return NOERROR;
}

ECode CCodeSigner::constructor(
    /* [in] */ ICertPath* signerCertPath,
    /* [in] */ ITimestamp* timestamp)
{
    if (signerCertPath == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    mSignerCertPath = signerCertPath;
    mTimestamp = timestamp;
    return NOERROR;
}

} // end Security
} // end Elastos
