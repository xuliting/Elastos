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

#include "elastos/droid/hardware/camera2/utils/LongParcelable.h"
#include <elastos/core/StringBuffer.h>

using Elastos::Core::StringBuffer;

namespace Elastos {
namespace Droid {
namespace Hardware {
namespace Camera2 {
namespace Utils {

CAR_INTERFACE_IMPL(LongParcelable, Object, ILongParcelable, IParcelable)

ECode LongParcelable::constructor()
{
    mNumber = 0;
    return NOERROR;
}

ECode LongParcelable::constructor(
        /* [in] */ Int64 number)
{
    mNumber = number;
    return NOERROR;
}

ECode LongParcelable::constructor(
    /* [in] */ IParcel* _in)
{
    return ReadFromParcel(_in);
}

ECode LongParcelable::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    return dest->WriteInt64(mNumber);
}

ECode LongParcelable::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    return source->ReadInt64(&mNumber);
}

ECode LongParcelable::GetNumber(
    /* [out] */ Int64* number)
{
    VALIDATE_NOT_NULL(number);

    *number = mNumber;
    return NOERROR;
}

ECode LongParcelable::SetNumber(
    /* [in] */ Int64 number)
{
    mNumber = number;
    return NOERROR;
}

} // namespace Utils
} // namespace Camera2
} // namespace Hardware
} // namespace Droid
} // namespace Elastos
