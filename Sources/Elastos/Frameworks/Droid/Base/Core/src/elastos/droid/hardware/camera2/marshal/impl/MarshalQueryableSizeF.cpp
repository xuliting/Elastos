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

#include "elastos/droid/hardware/camera2/marshal/impl/MarshalQueryableSizeF.h"
#include "elastos/droid/utility/CSizeF.h"

using Elastos::Droid::Hardware::Camera2::Impl::ICameraMetadataNative;
using Elastos::Droid::Utility::ISizeF;
using Elastos::Droid::Utility::CSizeF;
using Elastos::Droid::Utility::ECLSID_CSizeF;


namespace Elastos {
namespace Droid {
namespace Hardware {
namespace Camera2 {
namespace Marshal {
namespace Impl {

MarshalQueryableSizeF::MarshalerSizeF::MarshalerSizeF(
    /* [in] */ ITypeReference* typeReference,
    /* [in] */ Int32 nativeType,
    /* [in] */ MarshalQueryableSizeF* host)
{
    Marshaler::constructor(host, typeReference, nativeType);
}

ECode MarshalQueryableSizeF::MarshalerSizeF::Marshal(
    /* [in] */ IInterface* value,
    /* [in] */ IByteBuffer* buffer)
{
    AutoPtr<ISizeF> sizef = ISizeF::Probe(value);

    Float width;
    sizef->GetWidth(&width);
    buffer->PutFloat(width);
    Float height;
    sizef->GetHeight(&height);
    return buffer->PutFloat(height);
}

ECode MarshalQueryableSizeF::MarshalerSizeF::Unmarshal(
    /* [in] */ IByteBuffer* buffer,
    /* [out] */ IInterface** outface)
{
    VALIDATE_NOT_NULL(outface);
    *outface = NULL;

    Float width;
    buffer->GetFloat(&width);
    Float height;
    buffer->GetFloat(&height);

    AutoPtr<ISizeF> sizef;
    CSizeF::New(width, height, (ISizeF**)&sizef);
    *outface = TO_IINTERFACE(sizef);
    REFCOUNT_ADD(*outface);
    return NOERROR;
}

ECode MarshalQueryableSizeF::MarshalerSizeF::GetNativeSize(
    /* [out] */ Int32* value)
{
    VALIDATE_NOT_NULL(value);

    *value = SIZE;
    return NOERROR;
}

const Int32 MarshalQueryableSizeF::SIZE = IMarshalHelpers::SIZEOF_FLOAT * 2;

CAR_INTERFACE_IMPL(MarshalQueryableSizeF, Object,
        IMarshalQueryableSizeF, IMarshalQueryable)

ECode MarshalQueryableSizeF::CreateMarshaler(
    /* [in] */ ITypeReference* managedType,
    /* [in] */ Int32 nativeType,
    /* [out] */ IMarshaler** outmar)
{
    VALIDATE_NOT_NULL(outmar);

    AutoPtr<IMarshaler> _outmar = new MarshalerSizeF(managedType, nativeType, this);
    *outmar = _outmar;
    REFCOUNT_ADD(*outmar);
    return NOERROR;
}

ECode MarshalQueryableSizeF::IsTypeMappingSupported(
    /* [in] */ ITypeReference* managedType,
    /* [in] */ Int32 nativeType,
    /* [out] */ Boolean* value)
{
    VALIDATE_NOT_NULL(value);
    *value = FALSE;

    if (nativeType == ICameraMetadataNative::TYPE_FLOAT) {
        ClassID cls;
        managedType->GetClassType(&cls);
        if (cls == ECLSID_CSizeF) {
            *value = TRUE;
            return NOERROR;
        }
    }
    return NOERROR;
}

} // namespace Impl
} // namespace Marshal
} // namespace Camera2
} // namespace Hardware
} // namespace Droid
} // namespace Elastos
