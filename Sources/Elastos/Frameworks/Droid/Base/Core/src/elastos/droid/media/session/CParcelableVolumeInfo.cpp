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

#include "elastos/droid/media/session/CParcelableVolumeInfo.h"

namespace Elastos {
namespace Droid {
namespace Media {
namespace Session {

CAR_INTERFACE_IMPL(CParcelableVolumeInfo, Object, IParcelableVolumeInfo, IParcelable)

CAR_OBJECT_IMPL(CParcelableVolumeInfo)

CParcelableVolumeInfo::CParcelableVolumeInfo()
    : mVolumeType(0)
    , mControlType(0)
    , mMaxVolume(0)
    , mCurrentVolume(0)
{
}

CParcelableVolumeInfo::~CParcelableVolumeInfo()
{
}

ECode CParcelableVolumeInfo::constructor()
{
    return NOERROR;
}

ECode CParcelableVolumeInfo::constructor(
    /* [in] */ Int32 volumeType,
    /* [in] */ IAudioAttributes * audioAttrs,
    /* [in] */ Int32 controlType,
    /* [in] */ Int32 maxVolume,
    /* [in] */ Int32 currentVolume)
{
    mVolumeType = volumeType;
    mAudioAttrs = audioAttrs;
    mControlType = controlType;
    mMaxVolume = maxVolume;
    mCurrentVolume = currentVolume;
    return NOERROR;
}

ECode CParcelableVolumeInfo::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    source->ReadInt32(&mVolumeType);
    source->ReadInt32(&mControlType);
    source->ReadInt32(&mMaxVolume);
    source->ReadInt32(&mCurrentVolume);
    AutoPtr<IInterface> obj;
    source->ReadInterfacePtr((Handle32*)(IInterface**)&obj);
    mAudioAttrs = IAudioAttributes::Probe(obj);
    return NOERROR;
}

ECode CParcelableVolumeInfo::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteInt32(mVolumeType);
    dest->WriteInt32(mControlType);
    dest->WriteInt32(mMaxVolume);
    dest->WriteInt32(mCurrentVolume);
    dest->WriteInterfacePtr(mAudioAttrs);
    return NOERROR;
}

} // namespace Session
} // namespace Media
} // namepsace Droid
} // namespace Elastos
