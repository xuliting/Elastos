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

#include "elastos/droid/text/style/MetricAffectingSpan.h"
#include "elastos/droid/ext/frameworkext.h"

namespace Elastos {
namespace Droid {
namespace Text {
namespace Style {

//==================================================================
// MetricAffectingSpan
//==================================================================

CAR_INTERFACE_IMPL(MetricAffectingSpan, CharacterStyle, IMetricAffectingSpan, IUpdateLayout, IUpdateAppearance)

MetricAffectingSpan::MetricAffectingSpan()
{
}

MetricAffectingSpan::~MetricAffectingSpan()
{
}

ECode MetricAffectingSpan::GetUnderlying(
    /* [out] */ ICharacterStyle** style)
{
    VALIDATE_NOT_NULL(style)
    *style = this;
    REFCOUNT_ADD(*style);
    return NOERROR;
}

//==================================================================
// MetricAffectingSpanPassthrough
//==================================================================

MetricAffectingSpanPassthrough::MetricAffectingSpanPassthrough(
    /* [in] */ IMetricAffectingSpan* cs)
{
    mStyle = cs;
}

ECode MetricAffectingSpanPassthrough::UpdateDrawState(
    /* [in] */ ITextPaint* tp)
{
    return ICharacterStyle::Probe(mStyle)->UpdateDrawState(tp);
}

ECode MetricAffectingSpanPassthrough::UpdateMeasureState(
    /* [in] */ ITextPaint* tp)
{
    return mStyle->UpdateMeasureState(tp);
}

ECode MetricAffectingSpanPassthrough::GetUnderlying(
    /* [in] */ ICharacterStyle** result)
{
    return ICharacterStyle::Probe(mStyle)->GetUnderlying(result);
}


} // namespace Style
} // namespace Text
} // namepsace Droid
} // namespace Elastos