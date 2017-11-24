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

#include "org/apache/http/message/BasicNameValuePair.h"
#include "org/apache/http/utility/CCharArrayBuffer.h"
#include "elastos/utility/logging/Logger.h"

using Elastos::Core::EIID_ICloneable;
using Elastos::Utility::Logging::Logger;
using Org::Apache::Http::Utility::ICharArrayBuffer;
using Org::Apache::Http::Utility::CCharArrayBuffer;

namespace Org {
namespace Apache {
namespace Http {
namespace Message {

CAR_INTERFACE_IMPL(BasicNameValuePair, Object, INameValuePair, ICloneable)

ECode BasicNameValuePair::GetName(
    /* [out] */ String* name)
{
    VALIDATE_NOT_NULL(name)
    *name = mName;
    return NOERROR;
}

ECode BasicNameValuePair::GetValue(
    /* [out] */ String* value)
{
    VALIDATE_NOT_NULL(value)
    *value = mValue;
    return NOERROR;
}

ECode BasicNameValuePair::ToString(
    /* [out] */ String* string)
{
    VALIDATE_NOT_NULL(string)
    // don't call complex default formatting for a simple toString

    Int32 len = mName.GetLength();
    if (!mValue.IsNull()) {
        len += 1 + mValue.GetLength();
    }
    AutoPtr<ICharArrayBuffer> buffer;
    CCharArrayBuffer::New(len, (ICharArrayBuffer**)&buffer);

    buffer->Append(mName);
    if (!mValue.IsNull()) {
        buffer->Append(String("="));
        buffer->Append(mValue);
    }
    return IObject::Probe(buffer)->ToString(string);
}

ECode BasicNameValuePair::Equals(
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
    AutoPtr<INameValuePair> pair = INameValuePair::Probe(obj);
    if (pair != NULL) {
        AutoPtr<BasicNameValuePair> that = (BasicNameValuePair*)(IBasicNameValuePair*)obj;
        *equals = mName.Equals(that->mName) && mValue.Equals(that->mValue);
        return NOERROR;
    }
    else {
        *equals = FALSE;
        return NOERROR;
    }
}

ECode BasicNameValuePair::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode)
    // int hash = LangUtils.HASH_SEED;
    // hash = LangUtils.hashCode(hash, this.name);
    // hash = LangUtils.hashCode(hash, this.value);
    // return hash;
    return E_NOT_IMPLEMENTED;
}

ECode BasicNameValuePair::Init(
    /* [in] */ const String& name,
    /* [in] */ const String& value)
{
    if (name.IsNull()) {
        Logger::E("BasicNameValuePair", "Name may not be null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mName = name;
    mValue = value;
    return NOERROR;
}

void BasicNameValuePair::CloneImpl(
    /* [in] */ BasicNameValuePair* obj)
{
    obj->mName = mName;
    obj->mValue = mValue;
}

} // namespace Message
} // namespace Http
} // namespace Apache
} // namespace Org
