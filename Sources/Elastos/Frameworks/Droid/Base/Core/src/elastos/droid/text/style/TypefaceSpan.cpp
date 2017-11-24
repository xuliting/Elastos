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

#include "elastos/droid/text/style/TypefaceSpan.h"
#include "elastos/droid/graphics/CTypeface.h"
#include "elastos/droid/ext/frameworkext.h"

using Elastos::Droid::Graphics::ITypeface;
using Elastos::Droid::Graphics::CTypeface;

namespace Elastos {
namespace Droid {
namespace Text {
namespace Style {

CAR_INTERFACE_IMPL(TypefaceSpan, MetricAffectingSpan, ITypefaceSpan, IParcelableSpan, IParcelable)

TypefaceSpan::TypefaceSpan()
{}

TypefaceSpan::~TypefaceSpan()
{}

ECode TypefaceSpan::constructor()
{
    return NOERROR;
}

ECode TypefaceSpan::constructor(
    /* [in] */ const String& family)
{
    mFamily = family;
    return NOERROR;
}

ECode TypefaceSpan::GetSpanTypeId(
    /* [out] */ Int32* id)
{
    VALIDATE_NOT_NULL(id)
    *id = ITextUtils::TYPEFACE_SPAN;
    return NOERROR;
}

ECode TypefaceSpan::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    return source->ReadString(&mFamily);
}

ECode TypefaceSpan::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    return dest->WriteString(mFamily);
}

ECode TypefaceSpan::GetFamily(
    /* [out] */ String* family)
{
    VALIDATE_NOT_NULL(family)
    *family = mFamily;
    return NOERROR;
}

ECode TypefaceSpan::UpdateDrawState(
    /* [in] */ ITextPaint* ds)
{
    return Apply(IPaint::Probe(ds), mFamily);
}

ECode TypefaceSpan::UpdateMeasureState(
    /* [in] */ ITextPaint* paint)
{
    Apply(IPaint::Probe(paint), mFamily);
    return NOERROR;
}

ECode TypefaceSpan::Apply(
    /* [in] */ IPaint* paint,
    /* [in] */ const String& family)
{
    assert(paint != NULL);
    Int32 oldStyle;

    AutoPtr<ITypeface> old;
    paint->GetTypeface( (ITypeface**)&old);
    if (old == NULL) {
        oldStyle = 0;
    } else {
        old->GetStyle(&oldStyle);
    }

    AutoPtr<ITypeface> tf;
    CTypeface::Create(family, oldStyle, (ITypeface**)&tf);
    Int32 tfStyle;
    tf->GetStyle(&tfStyle);
    Int32 fake = oldStyle & ~tfStyle;

    if ((fake & ITypeface::BOLD) != 0) {
        paint->SetFakeBoldText(TRUE);
    }

    if ((fake & ITypeface::ITALIC) != 0) {
        paint->SetTextSkewX(-0.25f);
    }

    paint->SetTypeface(tf);
    return NOERROR;
}

} // namespace Style
} // namespace Text
} // namepsace Droid
} // namespace Elastos