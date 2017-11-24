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

#include "Enum.h"

using Elastos::IO::EIID_ISerializable;
using Elastos::Core::EIID_IComparable;

namespace Elastos {
namespace Core {

CAR_INTERFACE_IMPL(Enum, Object, IEnum, ISerializable, IComparable)

ECode Enum::Name(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    *str = mName;
    return NOERROR;
}

ECode Enum::Ordinal(
    /* [out] */ Int32* value)
{
    VALIDATE_NOT_NULL(value)

    *value = mOrdinal;
    return NOERROR;
}

ECode Enum::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)

    *str = mName;
    return NOERROR;
}

ECode Enum::Equals(
    /* [in] */ IInterface* other,
    /* [out] */ Boolean* value)
{
    VALIDATE_NOT_NULL(value)

    *value = TO_IINTERFACE(this) == other;
    return NOERROR;
}

ECode Enum::GetHashCode(
    /* [out] */ Int32* value)
{
    VALIDATE_NOT_NULL(value)

    *value = mOrdinal + (mName.IsNull() ? 0 : mName.GetHashCode());
    return NOERROR;
}

ECode Enum::GetDeclaringClass(
    /* [out] */ InterfaceID* riid)
{
    VALIDATE_NOT_NULL(riid)

    assert(0 && "TODO");
    // Class<?> myClass = getClass();
    // Class<?> mySuperClass = myClass.getSuperclass();
    // if (Enum.class == mySuperClass) {
    //     return (Class<E>)myClass;
    // }
    // return (Class<E>)mySuperClass;
    return E_NOT_IMPLEMENTED;
}

ECode Enum::CompareTo(
    /* [in] */ IInterface* o,
    /* [out] */ Int32* value)
{
    VALIDATE_NOT_NULL(value)

    AutoPtr<IEnum> res = IEnum::Probe(o);
    if (res == NULL) {
        return E_CLASS_CAST_EXCEPTION;
    }
    *value = mOrdinal - ((Enum*)res.Get())->mOrdinal;
    return NOERROR;
}

ECode Enum::ValueOf(
    /* [in] */ InterfaceID enumType,
    /* [in] */ const String& mName,
    /* [out] */ IInterface** obj)
{
    VALIDATE_NOT_NULL(obj)
    *obj = NULL;

    // if (enumType == NULL) {
    //     // throw new NullPointerException("enumType == null");
    //     return E_NULL_POINTER_EXCEPTION;
    // }
    // else
    if (mName.IsNull()) {
        // throw new NullPointerException("mName == null");
        return E_NULL_POINTER_EXCEPTION;
    }
    AutoPtr< ArrayOf<IInterface*> > values = GetSharedConstants(enumType);
    if (values == NULL) {
        // throw new IllegalArgumentException(enumType + " is not an enum type");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    for (Int32 i = 0; i < values->GetLength(); i++) {
        AutoPtr<IEnum> value = IEnum::Probe((*values)[i]);
        String str;
        value->Name(&str);
        if (mName.Equals(str)) {
            *obj = value;
            REFCOUNT_ADD(*obj)
            return NOERROR;
        }
    }
    // throw new IllegalArgumentException(mName + " is not a constant in " + enumType.getName());
    return E_ILLEGAL_ARGUMENT_EXCEPTION;
}

AutoPtr< ArrayOf<IInterface*> > Enum::GetSharedConstants(
    /* [in] */ InterfaceID enumType)
{
    // return (T[]) sharedConstantsCache.get(enumType);
    return NULL;
}

Enum::Enum(
    /* [in] */ const String& name,
    /* [in] */ Int32 ordinal)
{
    mName = name;
    mOrdinal = ordinal;
}

ECode Enum::Clone(
    /* [out] */ IInterface** value)
{
    // throw new CloneNotSupportedException("Enums may not be cloned");
    return E_CLONE_NOT_SUPPORTED_EXCEPTION;
}

ECode Enum::Finalize()
{
    return E_NOT_IMPLEMENTED;
}

} // namespace Core
} // namespace Elastos
