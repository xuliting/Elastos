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

#include "elastos/droid/telephony/cdma/CCdmaSmsCbProgramData.h"
#include <elastos/core/StringBuilder.h>

using Elastos::Core::StringBuilder;

namespace Elastos {
namespace Droid {
namespace Telephony {
namespace Cdma {

CAR_INTERFACE_IMPL(CCdmaSmsCbProgramData, Object, ICdmaSmsCbProgramData, IParcelable)

CAR_OBJECT_IMPL(CCdmaSmsCbProgramData)

CCdmaSmsCbProgramData::CCdmaSmsCbProgramData()
    : mOperation(0)
    , mCategory(0)
    , mLanguage(0)
    , mMaxMessages(0)
    , mAlertOption(0)
{
}

CCdmaSmsCbProgramData::~CCdmaSmsCbProgramData()
{
}

ECode CCdmaSmsCbProgramData::constructor()
{
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::constructor(
    /* [in] */ Int32 operation,
    /* [in] */ Int32 category,
    /* [in] */ Int32 language,
    /* [in] */ Int32 maxMessages,
    /* [in] */ Int32 alertOption,
    /* [in] */ const String& categoryName)
{
    mOperation = operation;
    mCategory = category;
    mLanguage = language;
    mMaxMessages = maxMessages;
    mAlertOption = alertOption;
    mCategoryName = categoryName;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    source->ReadInt32(&mOperation);
    source->ReadInt32(&mCategory);
    source->ReadInt32(&mLanguage);
    source->ReadInt32(&mMaxMessages);
    source->ReadInt32(&mAlertOption);
    source->ReadString(&mCategoryName);
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    dest->WriteInt32(mOperation);
    dest->WriteInt32(mCategory);
    dest->WriteInt32(mLanguage);
    dest->WriteInt32(mMaxMessages);
    dest->WriteInt32(mAlertOption);
    dest->WriteString(mCategoryName);
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::GetOperation(
    /* [out] */ Int32* operation)
{
    VALIDATE_NOT_NULL(operation);
    *operation = mOperation;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::GetCategory(
    /* [out] */ Int32* category)
{
    VALIDATE_NOT_NULL(category);
    *category = mCategory;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::GetLanguage(
    /* [out] */ Int32* language)
{
    VALIDATE_NOT_NULL(language);
    *language = mLanguage;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::GetMaxMessages(
    /* [out] */ Int32* maxmessages)
{
    VALIDATE_NOT_NULL(maxmessages);
    *maxmessages = mMaxMessages;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::GetAlertOption(
    /* [out] */ Int32* alertoption)
{
    VALIDATE_NOT_NULL(alertoption);
    *alertoption = mAlertOption;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::GetCategoryName(
    /* [out] */ String* categoryname)
{
    VALIDATE_NOT_NULL(categoryname);
    *categoryname = mCategoryName;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::SetOperation(
    /* [in] */ Int32 operation)
{
    mOperation = operation;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::SetCategory(
    /* [in] */ Int32 category)
{
    mCategory = category;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::SetLanguage(
    /* [in] */ Int32 language)
{
    mLanguage = language;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::SetMaxMessages(
    /* [in] */ Int32 maxmessages)
{
    mMaxMessages = maxmessages;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::SetAlertOption(
    /* [in] */ Int32 alertoption)
{
    mAlertOption = alertoption;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::SetCategoryName(
    /* [in] */ const String& categoryname)
{
    mCategoryName = categoryname;
    return NOERROR;
}

ECode CCdmaSmsCbProgramData::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str);
    StringBuilder sb;

    sb.Append("CdmaSmsCbProgramData{operation=");
    sb.Append(mOperation);
    sb.Append(", category=");
    sb.Append(mCategory);
    sb.Append(", language=");
    sb.Append(mLanguage);
    sb.Append(", max messages=");
    sb.Append(mMaxMessages);
    sb.Append(", alert option=");
    sb.Append(mAlertOption);
    sb.Append(", category name=");
    sb.Append(mCategoryName);
    sb.Append("}");
    *str = sb.ToString();

    return NOERROR;
}

} // namespace Cdma
} // namespace Telephony
} // namespace Droid
} // namespace Elastos
