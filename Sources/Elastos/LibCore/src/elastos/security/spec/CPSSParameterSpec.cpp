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

#include "CPSSParameterSpec.h"
#include "CMGF1ParameterSpec.h"

namespace Elastos {
namespace Security {
namespace Spec {

const AutoPtr<IPSSParameterSpec> CPSSParameterSpec::DEFAULT = CPSSParameterSpec::InitStatic();

const AutoPtr<IPSSParameterSpec> CPSSParameterSpec::InitStatic()
{
    AutoPtr<CPSSParameterSpec> tmp;
    CPSSParameterSpec::NewByFriend(20, (CPSSParameterSpec**)&tmp);
    return tmp;
}

CAR_OBJECT_IMPL(CPSSParameterSpec)
CAR_INTERFACE_IMPL(CPSSParameterSpec, Object, IPSSParameterSpec, IAlgorithmParameterSpec)
ECode CPSSParameterSpec::GetSaltLength(
    /* [out] */ Int32 *saltLength)
{
    VALIDATE_NOT_NULL(saltLength)
    *saltLength = mSaltLen;
    return NOERROR;
}

ECode CPSSParameterSpec::GetDigestAlgorithm(
    /* [out] */ String *digestAlgo)
{
    VALIDATE_NOT_NULL(digestAlgo)
    *digestAlgo = mMdName;
    return NOERROR;
}

ECode CPSSParameterSpec::GetMGFAlgorithm(
    /* [out] */ String *mgfaAlgo)
{
    VALIDATE_NOT_NULL(mgfaAlgo)
    *mgfaAlgo = mMgfName;
    return NOERROR;
}

ECode CPSSParameterSpec::GetMGFParameters(
    /* [out] */ IAlgorithmParameterSpec **mgfParams)
{
    VALIDATE_NOT_NULL(mgfParams)
    *mgfParams = mMgfSpec;
    REFCOUNT_ADD(*mgfParams)
    return NOERROR;
}

ECode CPSSParameterSpec::GetTrailerField(
    /* [out] */ Int32 *trailerField)
{
    VALIDATE_NOT_NULL(trailerField)
    *trailerField = mTrailerField;
    return NOERROR;
}

ECode CPSSParameterSpec::constructor(
    /* [in] */ Int32 saltLen)
{
    if (saltLen < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mSaltLen = saltLen;
    mMdName = String("SHA-1");
    mMgfName = String("MGF1");
    mMgfSpec = IAlgorithmParameterSpec::Probe(CMGF1ParameterSpec::SHA1);
    mTrailerField = 1;
    return NOERROR;
}

ECode CPSSParameterSpec::constructor(
    /* [in] */ const String& mdName,
    /* [in] */ const String& mgfName,
    /* [in] */ IAlgorithmParameterSpec *mgfSpec,
    /* [in] */ Int32 saltLen,
    /* [in] */ Int32 trailerField)
{
    if (mdName.IsNull()) {
        //throw new NullPointerException("mdName == null");
        return E_NULL_POINTER_EXCEPTION;
    }
    if (mgfName.IsNull()) {
        //throw new NullPointerException("mgfName == null");
        return E_NULL_POINTER_EXCEPTION;
    }
    if (saltLen < 0) {
        //throw new IllegalArgumentException("saltLen < 0");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (trailerField < 0) {
        //throw new IllegalArgumentException("trailerField < 0");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mMdName = mdName;
    mMgfName = mgfName;
    mMgfSpec = mgfSpec;
    mSaltLen = saltLen;
    mTrailerField = trailerField;
    return NOERROR;
}

}
}
}
