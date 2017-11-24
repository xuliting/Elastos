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

#include <Elastos.CoreLibrary.Utility.h>
#include "Elastos.Droid.Graphics.h"
#include "elastos/droid/ext/frameworkdef.h"
#include "elastos/droid/text/style/LocaleSpan.h"

using Elastos::Utility::CLocale;

namespace Elastos {
namespace Droid {
namespace Text {
namespace Style {


CAR_INTERFACE_IMPL(LocaleSpan, MetricAffectingSpan, ILocaleSpan, IParcelableSpan, IParcelable)

LocaleSpan::LocaleSpan()
{}

LocaleSpan::~LocaleSpan()
{}

ECode LocaleSpan::constructor()
{
    return NOERROR;
}

ECode LocaleSpan::constructor(
    /* [in] */ ILocale* locale)
{
    mLocale = locale;
    return NOERROR;
}

ECode LocaleSpan::GetSpanTypeId(
    /* [out] */ Int32* id)
{
    VALIDATE_NOT_NULL(id)
    *id = ITextUtils::LOCALE_SPAN;
    return NOERROR;
}

ECode LocaleSpan::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    String s1, s2, s3;
    FAIL_RETURN(source->ReadString(&s1));
    FAIL_RETURN(source->ReadString(&s2));
    FAIL_RETURN(source->ReadString(&s3));
    mLocale = NULL;
    return CLocale::New(s1, s2, s3, (ILocale**)&mLocale);
}

ECode LocaleSpan::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    String strLanguage, strCountry, strVariant;
    mLocale->GetLanguage(&strLanguage);
    mLocale->GetCountry(&strCountry);
    mLocale->GetVariant(&strVariant);
    FAIL_RETURN(dest->WriteString(strLanguage));
    FAIL_RETURN(dest->WriteString(strCountry));
    FAIL_RETURN(dest->WriteString(strVariant));
    return NOERROR;
}

ECode LocaleSpan::GetLocale(
    /* [out] */ ILocale** locale)
{
    VALIDATE_NOT_NULL(locale)
    *locale = mLocale;
    REFCOUNT_ADD(*locale)
    return NOERROR;
}

ECode LocaleSpan::UpdateDrawState(
    /* [in] */ ITextPaint* paint)
{
    return Apply(IPaint::Probe(paint), mLocale);
}

ECode LocaleSpan::UpdateMeasureState(
    /* [in] */ ITextPaint* paint)
{
    return Apply(IPaint::Probe(paint), mLocale);
}

ECode LocaleSpan::Apply(
    /* [in] */ IPaint* paint,
    /* [in] */ ILocale* locale)
{
    VALIDATE_NOT_NULL(paint)
    return paint->SetTextLocale(locale);
}

} // namespace Style
} // namespace Text
} // namepsace Droid
} // namespace Elastos
