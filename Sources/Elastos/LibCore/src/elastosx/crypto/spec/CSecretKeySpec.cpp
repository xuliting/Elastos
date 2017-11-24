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

#include "CSecretKeySpec.h"
#include "Arrays.h"

using Elastos::Security::EIID_IKey;
using Elastos::Utility::Arrays;

namespace Elastosx {
namespace Crypto {
namespace Spec {

CAR_OBJECT_IMPL(CSecretKeySpec)
CAR_INTERFACE_IMPL(CSecretKeySpec, Object, ISecretKeySpec, ISecretKey, IKey, IKeySpec, ISerializable)
Int64 CSecretKeySpec::sSerialVersionUID = 6577238317307289933L;

CSecretKeySpec::CSecretKeySpec()
    : mAlgorithm(String(NULL))
{
}

ECode CSecretKeySpec::constructor(
    /* [in] */ ArrayOf<Byte> * key,
    /* [in] */ const String& algorithm)
{
    if (key == NULL) {
        // throw new IllegalArgumentException("key == NULL");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (key->GetLength() == 0) {
        // throw new IllegalArgumentException("key.length == 0");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (algorithm == NULL) {
        // throw new IllegalArgumentException("algorithm == NULL");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    mAlgorithm = algorithm;
    mKey = ArrayOf<Byte>::Alloc(key->GetLength());
    mKey->Copy(0, key, 0, key->GetLength());
    return NOERROR;
}

ECode CSecretKeySpec::constructor(
    /* [in] */ ArrayOf<Byte> * key,
    /* [in] */ Int32 offset,
    /* [in] */ Int32 len,
    /* [in] */ const String& algorithm)
{
    if (key == NULL) {
        // throw new IllegalArgumentException("key == NULL");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (key->GetLength() == 0) {
        // throw new IllegalArgumentException("key.length == 0");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (len < 0 || offset < 0) {
        // throw new ArrayIndexOutOfBoundsException("len < 0 || offset < 0");
        return E_ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    }
    if (key->GetLength() - offset < len) {
        // throw new IllegalArgumentException("key too short");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (algorithm == NULL) {
        // throw new IllegalArgumentException("algorithm == NULL");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mAlgorithm = algorithm;
    mKey = ArrayOf<Byte>::Alloc(len);
    mKey->Copy(0, key, offset, len);
    return NOERROR;
}

ECode CSecretKeySpec::GetAlgorithm(
    /* [out] */ String * value)
{
    VALIDATE_NOT_NULL(value)
    *value = mAlgorithm;
    return NOERROR;
}

ECode CSecretKeySpec::GetFormat(
    /* [out] */ String * value)
{
    VALIDATE_NOT_NULL(value)
    *value = String("RAW");
    return NOERROR;
}

ECode CSecretKeySpec::GetEncoded(
    /* [out, callee] */ ArrayOf<Byte> ** value)
{
    VALIDATE_NOT_NULL(value)
    *value = ArrayOf<Byte>::Alloc(mKey->GetLength());
    (*value)->Copy(0, mKey, 0, mKey->GetLength());

    REFCOUNT_ADD(*value);
    return NOERROR;
}

ECode CSecretKeySpec::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode)
    *hashCode = mAlgorithm.GetLength();
    if (mKey == NULL) {
        return NOERROR;
    }
    for(Int32 i = 0; i < mKey->GetLength(); i++) {
        *hashCode += (*mKey)[i];
    }
    return NOERROR;
}

ECode CSecretKeySpec::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    if ((CSecretKeySpec*)(ISecretKeySpec*)obj == this) {
        *result = TRUE;
        return NOERROR;
    }
    if (ISecretKeySpec::Probe(obj) == NULL) {
        *result = FALSE;
        return NOERROR;
    }
    AutoPtr<CSecretKeySpec> ks = (CSecretKeySpec*)(ISecretKeySpec*) obj;
    *result = (mAlgorithm == ks->mAlgorithm)
        && (Arrays::Equals(mKey, ks->mKey));
    return NOERROR;
}

} // Spec
} // Crypto
} // Elastosx