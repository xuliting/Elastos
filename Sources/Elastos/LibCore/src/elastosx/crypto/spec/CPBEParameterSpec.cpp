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

#include "CPBEParameterSpec.h"

namespace Elastosx {
namespace Crypto {
namespace Spec {

CAR_OBJECT_IMPL(CPBEParameterSpec)
CAR_INTERFACE_IMPL(CPBEParameterSpec, Object, IPBEParameterSpec, IAlgorithmParameterSpec)

CPBEParameterSpec::CPBEParameterSpec()
    : mIterationCount(0)
{
}

ECode CPBEParameterSpec::constructor(
    /* [in] */ ArrayOf<Byte> * salt,
    /* [in] */ Int32 iterationCount)
{
    if (salt == NULL) {
        // throw new NullPointerException("salt == NULL");
        return E_NULL_POINTER_EXCEPTION;
    }
    mSalt = ArrayOf<Byte>::Alloc(salt->GetLength());
    mSalt->Copy(0, salt, 0, salt->GetLength());
    return NOERROR;
}

ECode CPBEParameterSpec::GetSalt(
    /* [out, callee] */ ArrayOf<Byte> ** result)
{
    VALIDATE_NOT_NULL(result)
    mSalt = ArrayOf<Byte>::Alloc(mSalt->GetLength());
    mSalt->Copy(0, mSalt, 0, mSalt->GetLength());
    return NOERROR;
}

ECode CPBEParameterSpec::GetIterationCount(
    /* [out] */ Int32 * result)
{
    VALIDATE_NOT_NULL(result)
    *result = mIterationCount;
    return NOERROR;
}

} // Spec
} // Crypto
} // Elastosx