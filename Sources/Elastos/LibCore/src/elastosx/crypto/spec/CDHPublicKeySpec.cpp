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

#include "CDHPublicKeySpec.h"

namespace Elastosx {
namespace Crypto {
namespace Spec {

CAR_OBJECT_IMPL(CDHPublicKeySpec)
CAR_INTERFACE_IMPL(CDHPublicKeySpec, Object, IDHPublicKeySpec, IKeySpec)

CDHPublicKeySpec::CDHPublicKeySpec()
{
}

ECode CDHPublicKeySpec::constructor(
    /* [in] */ IBigInteger * y,
    /* [in] */ IBigInteger * p,
    /* [in] */ IBigInteger * g)
{
    mY = y;
    mP = p;
    mG = g;
    return NOERROR;
}

ECode CDHPublicKeySpec::GetY(
    /* [out] */ IBigInteger ** result)
{
    VALIDATE_NOT_NULL(result)
    *result = mY;
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CDHPublicKeySpec::GetP(
    /* [out] */ IBigInteger ** result)
{
    VALIDATE_NOT_NULL(result)
    *result = mP;
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CDHPublicKeySpec::GetG(
    /* [out] */ IBigInteger ** result)
{
    VALIDATE_NOT_NULL(result)
    *result = mG;
    REFCOUNT_ADD(*result)
    return NOERROR;
}


} // Spec
} // Crypto
} // Elastosx