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

#include "org/apache/http/auth/CBasicUserPrincipal.h"
#include "org/apache/http/utility/LangUtils.h"
#include "elastos/core/StringBuilder.h"
#include "elastos/core/CString.h"
#include "elastos/utility/logging/Logger.h"

using Elastos::Core::ICharSequence;
using Elastos::Core::CString;
using Elastos::Core::StringBuilder;
using Elastos::Security::EIID_IPrincipal;
using Elastos::Utility::Logging::Logger;
using Org::Apache::Http::Utility::LangUtils;

namespace Org {
namespace Apache {
namespace Http {
namespace Auth {

CAR_INTERFACE_IMPL(CBasicUserPrincipal, Object, IBasicUserPrincipal, IPrincipal)

CAR_OBJECT_IMPL(CBasicUserPrincipal)

ECode CBasicUserPrincipal::GetName(
    /* [out] */ String* name)
{
    VALIDATE_NOT_NULL(name)
    *name = mUsername;
    return NOERROR;
}

ECode CBasicUserPrincipal::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode)
    Int32 hash = LangUtils::HASH_SEED;
    AutoPtr<ICharSequence> cs;
    CString::New(mUsername, (ICharSequence**)&cs);
    hash = LangUtils::HashCode(hash, IObject::Probe(cs));
    *hashCode = hash;
    return NOERROR;
}

ECode CBasicUserPrincipal::Equals(
    /* [in] */ IInterface* obj,
    /* [out] */ Boolean* equals)
{
    VALIDATE_NOT_NULL(equals)

    if (obj == NULL) {
        *equals = FALSE;
        return NOERROR;
    }
    if (TO_IINTERFACE(this) == obj) {
        *equals = TRUE;
        return NOERROR;
    }
    if (IBasicUserPrincipal::Probe(obj) != NULL) {
        AutoPtr<IBasicUserPrincipal> that = IBasicUserPrincipal::Probe(obj);
        AutoPtr<IPrincipal> thatP = IPrincipal::Probe(that);
        String thatName;
        thatP->GetName(&thatName);
        AutoPtr<ICharSequence> cs, thatCS;
        CString::New(mUsername, (ICharSequence**)&cs);
        CString::New(thatName, (ICharSequence**)&thatCS);
        if (LangUtils::Equals(IObject::Probe(cs), IObject::Probe(thatCS))) {
            *equals = TRUE;
            return NOERROR;
        }
    }
    *equals = FALSE;
    return NOERROR;
}

ECode CBasicUserPrincipal::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    StringBuilder buffer;
    buffer.Append("[principal: ");
    buffer.Append(mUsername);
    buffer.Append("]");
    *str = buffer.ToString();
    return NOERROR;
}

ECode CBasicUserPrincipal::constructor(
    /* [in] */ const String& username)
{
    if (username.IsNull()) {
        Logger::E("CBasicUserPrincipal", "User name may not be null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mUsername = username;
    return NOERROR;
}

} // namespace Auth
} // namespace Http
} // namespace Apache
} // namespace Org
