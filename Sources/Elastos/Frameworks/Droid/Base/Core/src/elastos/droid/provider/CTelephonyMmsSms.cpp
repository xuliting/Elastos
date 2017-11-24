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

#include "elastos/droid/provider/CTelephonyMmsSms.h"
#include "elastos/droid/provider/Telephony.h"
#include "Elastos.Droid.Net.h"

namespace Elastos {
namespace Droid {
namespace Provider {

CAR_INTERFACE_IMPL(CTelephonyMmsSms, Singleton, ITelephonyMmsSms, IBaseColumns);
CAR_SINGLETON_IMPL(CTelephonyMmsSms);

ECode CTelephonyMmsSms::GetCONTENT_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::MmsSms::CONTENT_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

ECode CTelephonyMmsSms::GetCONTENT_CONVERSATIONS_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::MmsSms::CONTENT_CONVERSATIONS_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

ECode CTelephonyMmsSms::GetCONTENT_FILTER_BYPHONE_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::MmsSms::CONTENT_FILTER_BYPHONE_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

ECode CTelephonyMmsSms::GetCONTENT_UNDELIVERED_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::MmsSms::CONTENT_UNDELIVERED_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

ECode CTelephonyMmsSms::GetCONTENT_DRAFT_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::MmsSms::CONTENT_DRAFT_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

ECode CTelephonyMmsSms::GetCONTENT_LOCKED_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::MmsSms::CONTENT_LOCKED_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

ECode CTelephonyMmsSms::GetSEARCH_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::MmsSms::SEARCH_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

} // namespace Provider
} // namespace Droid
} // namespace Elastos
