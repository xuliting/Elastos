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

#include "elastos/droid/media/browse/CMediaBrowserMediaItem.h"
#include <elastos/core/StringBuilder.h>

using Elastos::Core::StringBuilder;
using Elastos::Droid::Media::IMediaDescription;

namespace Elastos {
namespace Droid {
namespace Media {
namespace Browse {

CAR_INTERFACE_IMPL(CMediaBrowserMediaItem, Object, IMediaBrowserMediaItem, IParcelable)

CAR_OBJECT_IMPL(CMediaBrowserMediaItem)

CMediaBrowserMediaItem::CMediaBrowserMediaItem()
    : mFlags(0)
{}

CMediaBrowserMediaItem::~CMediaBrowserMediaItem()
{
}

ECode CMediaBrowserMediaItem::constructor()
{
    return NOERROR;
}

ECode CMediaBrowserMediaItem::constructor(
    /* [in] */ IMediaDescription * description,
    /* [in] */ Int32 flags)
{
    if (description == NULL) {
        // throw new IllegalArgumentException("description cannot be null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    String mediaId;
    description->GetMediaId(&mediaId);
    if (mediaId.IsEmpty()) {
        // throw new IllegalArgumentException("description must have a non-empty media id");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mFlags = flags;
    mDescription = description;
    return NOERROR;
}

ECode CMediaBrowserMediaItem::ToString(
    /* [out] */ String* str)
{
    StringBuilder sb("MediaItem{");
    sb += "mFlags=";
    sb += mFlags;
    sb += ", mDescription=";
    sb += mDescription;
    sb += '}';
    return sb.ToString(str);
}

ECode CMediaBrowserMediaItem::GetFlags(
    /* [out] */ Int32 * result)
{
    VALIDATE_NOT_NULL(result)
    *result = mFlags;
    return NOERROR;
}

ECode CMediaBrowserMediaItem::IsBrowsable(
    /* [out] */ Boolean * result)
{
    VALIDATE_NOT_NULL(result)
    *result = ((mFlags & IMediaBrowserMediaItem::FLAG_BROWSABLE) != 0);
    return NOERROR;
}

ECode CMediaBrowserMediaItem::IsPlayable(
    /* [out] */ Boolean * result)
{
    VALIDATE_NOT_NULL(result)
    *result = ((mFlags & IMediaBrowserMediaItem::FLAG_PLAYABLE) != 0);
    return NOERROR;
}

ECode CMediaBrowserMediaItem::GetDescription(
    /* [out] */ IMediaDescription ** result)
{
    VALIDATE_NOT_NULL(result)
    *result = mDescription;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CMediaBrowserMediaItem::GetMediaId(
    /* [out] */ String * result)
{
    VALIDATE_NOT_NULL(result)
    return mDescription->GetMediaId(result);
}

ECode CMediaBrowserMediaItem::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    AutoPtr<IInterface> obj;
    source->ReadInt32(&mFlags);
    source->ReadInterfacePtr((Handle32*)(IInterface**)&obj);
    mDescription = IMediaDescription::Probe(obj);
    return NOERROR;
}

ECode CMediaBrowserMediaItem::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteInt32(mFlags);
    dest->WriteInterfacePtr(mDescription);
    return NOERROR;
}

} // namespace Browse
} // namespace Media
} // namepsace Droid
} // namespace Elastos
