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

#include "Elastos.Droid.Os.h"
#include "elastos/droid/text/style/TtsSpan.h"
// #include "elastos/droid/os/CPersistableBundle.h."

// using Elastos::Droid::Os::CPersistableBundle;

namespace Elastos {
namespace Droid {
namespace Text {
namespace Style {


CAR_INTERFACE_IMPL(TtsSpan, Object, ITtsSpan, IParcelableSpan, IParcelable)

TtsSpan::TtsSpan()
{}

TtsSpan::~TtsSpan()
{
}

ECode TtsSpan::constructor()
{
    return NOERROR;
}

ECode TtsSpan::constructor(
    /* [in] */ const String& type,
    /* [in] */ IPersistableBundle* args)
{
    mType = type;
    mArgs = args;
    return NOERROR;
}

ECode TtsSpan::GetType(
    /* [out] */ String* type)
{
    VALIDATE_NOT_NULL(type)
    *type = mType;
    return NOERROR;
}

ECode TtsSpan::GetArgs(
    /* [out] */ IPersistableBundle** source)
{
    VALIDATE_NOT_NULL(source)
    *source = mArgs;
    REFCOUNT_ADD(*source)
    return NOERROR;
}

ECode TtsSpan::GetSpanTypeId(
    /* [out] */ Int32* id)
{
    VALIDATE_NOT_NULL(id)
    *id = ITextUtils::TTS_SPAN;
    return NOERROR;
}

ECode TtsSpan::ReadFromParcel(
    /* [in] */ IParcel* src)
{
    src->ReadString(&mType);

    mArgs = NULL;
    Int32 ival;
    src->ReadInt32(&ival);
    if (ival == 1) {
        // CPersistableBundle::New((IPersistableBundle**)&mArgs);
        IParcelable::Probe(mArgs)->ReadFromParcel(src);
    }
    return NOERROR;
}

ECode TtsSpan::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteString(mType);
    if (mArgs != NULL) {
        dest->WriteInt32(1);
        IParcelable::Probe(mArgs)->WriteToParcel(dest);
    }
    else {
        dest->WriteInt32(0);
    }
    return NOERROR;
}


} // namespace Style
} // namespace Text
} // namepsace Droid
} // namespace Elastos
