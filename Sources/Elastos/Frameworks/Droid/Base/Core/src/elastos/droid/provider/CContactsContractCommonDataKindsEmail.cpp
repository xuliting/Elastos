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

#include "Elastos.Droid.Content.h"
#include "elastos/droid/provider/CContactsContractCommonDataKindsEmail.h"
#include "elastos/droid/provider/ContactsContractData.h"
#include "elastos/droid/net/Uri.h"
#include "elastos/droid/text/TextUtils.h"
#include "elastos/droid/R.h"

using Elastos::Droid::Text::TextUtils;
using Elastos::Droid::Net::Uri;

namespace Elastos {
namespace Droid {
namespace Provider {

CAR_SINGLETON_IMPL(CContactsContractCommonDataKindsEmail)

CAR_INTERFACE_IMPL(CContactsContractCommonDataKindsEmail, Singleton
    , IContactsContractCommonDataKindsEmail
    , IContactsContractDataColumnsWithJoins
    , IContactsContractCommonDataKindsCommonColumns)

ECode CContactsContractCommonDataKindsEmail::GetCONTENT_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);

    AutoPtr<IUri> auUri;
    FAIL_RETURN(ContactsContractData::GetCONTENT_URI((IUri**)&auUri))
    return Uri::WithAppendedPath(auUri, String("emails"), uri);
}

ECode CContactsContractCommonDataKindsEmail::GetCONTENT_LOOKUP_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);

    AutoPtr<IUri> auUri;
    FAIL_RETURN(GetCONTENT_URI((IUri**)&auUri))
    return Uri::WithAppendedPath(auUri, String("lookup"), uri);
}

ECode CContactsContractCommonDataKindsEmail::GetCONTENT_FILTER_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);

    AutoPtr<IUri> auUri;
    FAIL_RETURN(GetCONTENT_URI((IUri**)&auUri))
    return Uri::WithAppendedPath(auUri, String("filter"), uri);
}

ECode CContactsContractCommonDataKindsEmail::GetTypeLabelResource(
    /* [in] */ Int32 type,
    /* [out] */ Int32* resource)
{
    VALIDATE_NOT_NULL(resource);

    switch (type) {
        case TYPE_HOME:
            *resource = Elastos::Droid::R::string::emailTypeHome;
            break;
        case TYPE_WORK:
            *resource = Elastos::Droid::R::string::emailTypeWork;
            break;
        case TYPE_OTHER:
            *resource = Elastos::Droid::R::string::emailTypeOther;
            break;
        case TYPE_MOBILE:
            *resource = Elastos::Droid::R::string::emailTypeMobile;
            break;
        default:
            *resource = Elastos::Droid::R::string::emailTypeCustom;
            break;
    }
    return NOERROR;
}

ECode CContactsContractCommonDataKindsEmail::GetTypeLabel(
    /* [in] */ IResources* res,
    /* [in] */ Int32 type,
    /* [in] */ ICharSequence* label,
    /* [out] */ ICharSequence** lb)
{
    if (type == IContactsContractCommonDataKindsBaseTypes::TYPE_CUSTOM && !TextUtils::IsEmpty(label)) {
        *lb = label;
        REFCOUNT_ADD(*lb);
        return NOERROR;
    } else {
        Int32 labelRes;
        FAIL_RETURN(GetTypeLabelResource(type, &labelRes))
        return res->GetText(labelRes, lb);
    }
}

} //Provider
} //Droid
} //Elastos