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

#include "CInteger16.h"
#include "IntegralToString.h"

using Elastos::IO::EIID_ISerializable;
using Elastos::Core::IntegralToString;

namespace Elastos {
namespace Core {

CAR_INTERFACE_IMPL(CInteger16, Object, IInteger16, INumber, ISerializable, IComparable)

CAR_OBJECT_IMPL(CInteger16)

ECode CInteger16::constructor(
    /* [in] */ Int16 value)
{
    mValue = value;
    return NOERROR;
}

ECode CInteger16::GetValue(
    /* [out] */ Int16* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mValue;
    return NOERROR;
}

ECode CInteger16::ByteValue(
    /* [out] */ Byte* result)
{
    VALIDATE_NOT_NULL(result);
    *result = (Byte)mValue;

    return NOERROR;
}

ECode CInteger16::Int16Value(
    /* [out] */ Int16* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mValue;

    return NOERROR;
}

ECode CInteger16::Int32Value(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mValue;

    return NOERROR;
}

ECode CInteger16::Int64Value(
    /* [out] */ Int64* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mValue;

    return NOERROR;
}

ECode CInteger16::FloatValue(
    /* [out] */ Float* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mValue;

    return NOERROR;
}

ECode CInteger16::DoubleValue(
    /* [out] */ Double* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mValue;

    return NOERROR;
}

ECode CInteger16::CompareTo(
    /* [in] */ IInterface* _other,
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = -1;
    VALIDATE_NOT_NULL(_other);

    IInteger16* other = IInteger16::Probe(_other);
    if (other == NULL) {
        return NOERROR;
    }
    Int16 otherValue;
    other->GetValue(&otherValue);
    *result = (mValue > otherValue ? 1 : (mValue < otherValue ? -1 : 0));
    return NOERROR;
}

ECode CInteger16::Equals(
    /* [in] */ IInterface* other,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = FALSE;
    VALIDATE_NOT_NULL(other);

    if (IInteger16::Probe(other) == NULL) return NOERROR;

    Int16 otherValue;
    IInteger16::Probe(other)->GetValue(&otherValue);
    *result = (mValue == otherValue);
    return NOERROR;
}

ECode CInteger16::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode);
    *hashCode = mValue;
    return NOERROR;
}

ECode CInteger16::ToString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result);
    *result = IntegralToString::ToString(mValue);

    return NOERROR;
}

} // namespace Core
} // namespace Elastos
