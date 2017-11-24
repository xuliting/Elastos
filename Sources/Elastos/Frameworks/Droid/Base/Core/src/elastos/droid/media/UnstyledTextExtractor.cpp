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

#include "elastos/droid/media/UnstyledTextExtractor.h"
#include "elastos/droid/media/CTextTrackCueSpan.h"
#include <elastos/core/CoreUtils.h>

using Elastos::Core::CArrayOf;
using Elastos::Core::IArrayOf;

namespace Elastos {
namespace Droid {
namespace Media {

UnstyledTextExtractor::UnstyledTextExtractor()
    : mLastTimestamp(0)
{}

UnstyledTextExtractor::~UnstyledTextExtractor()
{}

CAR_INTERFACE_IMPL(UnstyledTextExtractor, Object, IUnstyledTextExtractor, ITokenizerOnTokenListener)

ECode UnstyledTextExtractor::constructor()
{
    Init();
    return NOERROR;
}

void UnstyledTextExtractor::Init()
{
    Int32 length;
    ICharSequence::Probe(mLine)->GetLength(&length);
    mLine->Delete(0, length);
    mLines->Clear();
    mCurrentLine->Clear();
    mLastTimestamp = -1;
}

ECode UnstyledTextExtractor::OnData(
    /* [in] */ const String& s)
{
    return mLine->Append(s);
}

ECode UnstyledTextExtractor::OnStart(
    /* [in] */ const String& tag,
    /* [in] */ ArrayOf<String>* classes,
    /* [in] */ const String& annotation)
{
    return NOERROR;
}

ECode UnstyledTextExtractor::OnEnd(
    /* [in] */ const String& tag)
{
    return NOERROR;
}

ECode UnstyledTextExtractor::OnTimeStamp(
    /* [in] */ Int64 timestampMs)
{
    // finish any prior span
    Int32 length;
    ICharSequence::Probe(mLine)->GetLength(&length);
    if (length > 0 && timestampMs != mLastTimestamp) {
        String str;
        ICharSequence::Probe(mLine)->ToString(&str);
        AutoPtr<ITextTrackCueSpan> ttcs;
        CTextTrackCueSpan::New(str, mLastTimestamp, (ITextTrackCueSpan**)&ttcs);
        mCurrentLine->Add(ttcs);
        mLine->Delete(0, length);
    }
    mLastTimestamp = timestampMs;
    return NOERROR;
}

ECode UnstyledTextExtractor::OnLineEnd()
{
    // finish any pending span
    Int32 length;
    ICharSequence::Probe(mLine)->GetLength(&length);
    if (length > 0) {
        String str;
        ICharSequence::Probe(mLine)->ToString(&str);
        AutoPtr<ITextTrackCueSpan> ttcs;
        CTextTrackCueSpan::New(str, mLastTimestamp, (ITextTrackCueSpan**)&ttcs);
        mCurrentLine->Add(ttcs);
        mLine->Delete(0, length);
    }

    Int32 size;
    mCurrentLine->GetSize(&size);
    AutoPtr<ArrayOf<IInterface*> > spans;
    mCurrentLine->ToArray((ArrayOf<IInterface*>**)&spans);
    mCurrentLine->Clear();
    AutoPtr<IArrayOf> spans_ = CoreUtils::Convert(spans, EIID_ITextTrackCueSpan);
    return mLines->Add(spans_);
}

ECode UnstyledTextExtractor::GetText(
    /* [out, callee] */ ArrayOf<IArrayOf*>** result)
{
    // for politeness, finish last cue-line if it ends abruptly
    Int32 length;
    ICharSequence::Probe(mLine)->GetLength(&length);
    Int32 size;
    mCurrentLine->GetSize(&size);
    if (length > 0 || size > 0) {
        OnLineEnd();
    }
    AutoPtr<ArrayOf<IArrayOf*> > lines = ArrayOf<IArrayOf*>::Alloc(size);
    for (Int32 i = 0; i < size; ++i) {
        AutoPtr<IInterface> inter;
        mLines->Get(i, (IInterface**)&inter);
        lines->Set(i, IArrayOf::Probe(inter));
    }
    Init();
    *result = lines.Get();
    REFCOUNT_ADD(*result)
    return NOERROR;
}

} // namespace Media
} // namepsace Droid
} // namespace Elastos