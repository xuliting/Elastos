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

#include "elastos/droid/ext/frameworkext.h"
#include "elastos/droid/content/pm/PermissionGroupInfo.h"
#include "elastos/droid/text/TextUtils.h"

using Elastos::Droid::Text::TextUtils;

namespace Elastos {
namespace Droid {
namespace Content {
namespace Pm {

CAR_INTERFACE_IMPL(PermissionGroupInfo, PackageItemInfo, IPermissionGroupInfo, IParcelable)

PermissionGroupInfo::PermissionGroupInfo()
    : mDescriptionRes(0)
    , mFlags(0)
    , mPriority(0)
{}

PermissionGroupInfo::~PermissionGroupInfo()
{}

ECode PermissionGroupInfo::constructor()
{
    return NOERROR;
}

ECode PermissionGroupInfo::constructor(
    /* [in] */ IPermissionGroupInfo* orig)
{
    assert(orig != NULL);
    PackageItemInfo::constructor(IPackageItemInfo::Probe(orig));
    orig->GetDescriptionRes(&mDescriptionRes);
    orig->GetNonLocalizedDescription((ICharSequence**)&mNonLocalizedDescription);
    orig->GetFlags(&mFlags);
    orig->GetPriority(&mPriority);
    return NOERROR;
}

ECode PermissionGroupInfo::LoadDescription(
    /* [in] */ IPackageManager* pm,
    /* [out] */ ICharSequence** description)
{
    VALIDATE_NOT_NULL(description);
    *description = NULL;

    if (mNonLocalizedDescription != NULL) {
        *description = mNonLocalizedDescription;
        REFCOUNT_ADD(*description);
        return NOERROR;
    }

    if (mDescriptionRes != 0) {
        pm->GetText(mPackageName, mDescriptionRes, NULL, description);
    }

    return NOERROR;
}

ECode PermissionGroupInfo::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    *str = NULL;
    str->AppendFormat("PermissionGroupInfo{%p %s flgs = 0x%x}", this, mName.string(), mFlags);
    return NOERROR;
}

ECode PermissionGroupInfo::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    PackageItemInfo::ReadFromParcel(source);
    source->ReadInt32(&mDescriptionRes);
    TextUtils::CHAR_SEQUENCE_CREATOR::CreateFromParcel(source, (ICharSequence**)&mNonLocalizedDescription);
    source->ReadInt32(&mFlags);
    source->ReadInt32(&mPriority);
    return NOERROR;
}

ECode PermissionGroupInfo::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    PackageItemInfo::WriteToParcel(dest);
    dest->WriteInt32(mDescriptionRes);
    TextUtils::WriteToParcel(mNonLocalizedDescription, dest);
    dest->WriteInt32(mFlags);
    dest->WriteInt32(mPriority);
    return NOERROR;
}

ECode PermissionGroupInfo::LoadLabel(
    /* [in] */ IPackageManager* pm,
    /* [out] */ ICharSequence** label)
{
    VALIDATE_NOT_NULL(label);
    return PackageItemInfo::LoadLabel(pm, label);
}

ECode PermissionGroupInfo::LoadIcon(
    /* [in] */ IPackageManager* pm,
    /* [out] */ IDrawable** icon)
{
    VALIDATE_NOT_NULL(icon);
    return PackageItemInfo::LoadIcon(pm, icon);
}

ECode PermissionGroupInfo::LoadLogo(
    /* [in] */ IPackageManager* pm,
    /* [out] */ IDrawable** logo)
{
    VALIDATE_NOT_NULL(logo);
    return PackageItemInfo::LoadLogo(pm, logo);
}

ECode PermissionGroupInfo::LoadXmlMetaData(
    /* [in] */ IPackageManager* pm,
    /* [in] */ const String& name,
    /* [out] */ IXmlResourceParser** resource)
{
    VALIDATE_NOT_NULL(resource);
    return PackageItemInfo::LoadXmlMetaData(pm, name, resource);
}

ECode PermissionGroupInfo::GetName(
    /* [out] */ String* name)
{
    VALIDATE_NOT_NULL(name);
    return PackageItemInfo::GetName(name);
}

ECode PermissionGroupInfo::SetName(
    /* [in] */ const String& name)
{
    return PackageItemInfo::SetName(name);
}

ECode PermissionGroupInfo::GetPackageName(
    /* [out] */ String* name)
{
    VALIDATE_NOT_NULL(name);
    return PackageItemInfo::GetPackageName(name);
}

ECode PermissionGroupInfo::SetPackageName(
    /* [in] */ const String& name)
{
    return PackageItemInfo::SetPackageName(name);
}

ECode PermissionGroupInfo::GetLabelRes(
    /* [out] */ Int32* labelRes)
{
    VALIDATE_NOT_NULL(labelRes);
    return PackageItemInfo::GetLabelRes(labelRes);
}

ECode PermissionGroupInfo::SetLabelRes(
    /* [in] */ Int32 labelRes)
{
    return PackageItemInfo::SetLabelRes(labelRes);
}

ECode PermissionGroupInfo::GetNonLocalizedLabel(
    /* [out] */ ICharSequence** label)
{
    VALIDATE_NOT_NULL(label);
    return PackageItemInfo::GetNonLocalizedLabel(label);
}

ECode PermissionGroupInfo::SetNonLocalizedLabel(
    /* [in] */ ICharSequence*  label)
{
    return PackageItemInfo::SetNonLocalizedLabel(label);
}

ECode PermissionGroupInfo::GetIcon(
    /* [out] */ Int32* icon)
{
    VALIDATE_NOT_NULL(icon);
    return PackageItemInfo::GetIcon(icon);
}

ECode PermissionGroupInfo::SetIcon(
    /* [in] */ Int32 icon)
{
    return PackageItemInfo::SetIcon(icon);
}

ECode PermissionGroupInfo::GetLogo(
    /* [out] */ Int32* logo)
{
    VALIDATE_NOT_NULL(logo);
    return PackageItemInfo::GetLogo(logo);
}

ECode PermissionGroupInfo::SetLogo(
    /* [in] */ Int32 logo)
{
    return PackageItemInfo::SetLogo(logo);
}

ECode PermissionGroupInfo::GetMetaData(
    /* [out] */ IBundle** metaData)
{
    VALIDATE_NOT_NULL(metaData);
    return PackageItemInfo::GetMetaData(metaData);
}

ECode PermissionGroupInfo::SetMetaData(
    /* [in] */ IBundle* metaData)
{
    return PackageItemInfo::SetMetaData(metaData);
}

ECode PermissionGroupInfo::GetDescriptionRes(
    /* [out] */ Int32* descriptionRes)
{
    VALIDATE_NOT_NULL(descriptionRes);
    *descriptionRes = mDescriptionRes;
    return NOERROR;
}

ECode PermissionGroupInfo::SetDescriptionRes(
    /* [in] */ Int32 descriptionRes)
{
    mDescriptionRes = descriptionRes;
    return NOERROR;
}

ECode PermissionGroupInfo::GetNonLocalizedDescription(
    /* [out] */ ICharSequence** nonLocalizedDescription)
{
    VALIDATE_NOT_NULL(nonLocalizedDescription);
    *nonLocalizedDescription = mNonLocalizedDescription;
    REFCOUNT_ADD(*nonLocalizedDescription);
    return NOERROR;
}

ECode PermissionGroupInfo::SetNonLocalizedDescription(
    /* [in] */ ICharSequence* nonLocalizedDescription)
{
    mNonLocalizedDescription = nonLocalizedDescription;
    return NOERROR;
}

ECode PermissionGroupInfo::GetFlags(
    /* [out] */ Int32* flags)
{
    VALIDATE_NOT_NULL(flags);
    *flags = mFlags;
    return NOERROR;
}

ECode PermissionGroupInfo::SetFlags(
    /* [in] */ Int32 flags)
{
    mFlags = flags;
    return NOERROR;
}

ECode PermissionGroupInfo::GetPriority(
    /* [out] */ Int32* priority)
{
    VALIDATE_NOT_NULL(priority);
    *priority = mPriority;
    return NOERROR;
}

ECode PermissionGroupInfo::SetPriority(
    /* [in] */ Int32 priority)
{
    mPriority = priority;
    return NOERROR;
}

} // namespace Pm
} // namespace Content
} // namespace Droid
} // namespace Elastos

