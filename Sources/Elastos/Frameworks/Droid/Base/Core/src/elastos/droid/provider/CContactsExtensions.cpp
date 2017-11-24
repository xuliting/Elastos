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

#include "elastos/droid/net/Uri.h"
#include "elastos/droid/provider/CContactsExtensions.h"

using Elastos::Droid::Net::IUri;
using Elastos::Droid::Net::Uri;

namespace Elastos {
namespace Droid {
namespace Provider {

CAR_SINGLETON_IMPL(CContactsExtensions)

CAR_INTERFACE_IMPL(CContactsExtensions, Singleton, IContactsExtensions, IBaseColumns, IContactsExtensionsColumns)

ECode CContactsExtensions::constructor()
{
    return NOERROR;
}

ECode CContactsExtensions::GetCONTENT_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);

    return Uri::Parse(String("content://contacts/extensions"), uri);
}

} // Provider
} // Droid
} // Elastos