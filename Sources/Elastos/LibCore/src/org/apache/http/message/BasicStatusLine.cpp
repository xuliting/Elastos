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

#include "org/apache/http/message/BasicStatusLine.h"
#include "org/apache/http/message/BasicLineFormatter.h"
#include "elastos/utility/logging/Logger.h"

using Elastos::Core::EIID_ICloneable;
using Elastos::Utility::Logging::Logger;
using Org::Apache::Http::Utility::ICharArrayBuffer;

namespace Org {
namespace Apache {
namespace Http {
namespace Message {

BasicStatusLine::BasicStatusLine()
    : mStatusCode(0)
{}

CAR_INTERFACE_IMPL(BasicStatusLine, Object, IStatusLine, ICloneable)

ECode BasicStatusLine::Init(
    /* [in] */ IProtocolVersion* version,
    /* [in] */ Int32 statusCode,
    /* [in] */ const String& reasonPhrase)
{
    if (version == NULL) {
        Logger::E("BasicStatusLine", "Protocol version may not be null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (statusCode < 0) {
        Logger::E("BasicStatusLine", "Status code may not be null");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    mProtoVersion = version;
    mStatusCode   = statusCode;
    mReasonPhrase = reasonPhrase;
    return NOERROR;
}

ECode BasicStatusLine::GetStatusCode(
    /* [out] */ Int32* code)
{
    VALIDATE_NOT_NULL(code)
    *code = mStatusCode;
    return NOERROR;
}

ECode BasicStatusLine::GetProtocolVersion(
    /* [out] */ IProtocolVersion** ver)
{
    VALIDATE_NOT_NULL(ver)
    *ver = mProtoVersion;
    REFCOUNT_ADD(*ver)
    return NOERROR;
}

ECode BasicStatusLine::GetReasonPhrase(
    /* [out] */ String* phrase)
{
    VALIDATE_NOT_NULL(phrase)
    *phrase = mReasonPhrase;
    return NOERROR;
}

ECode BasicStatusLine::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    // no need for non-default formatting in toString()
    AutoPtr<ICharArrayBuffer> buf;
    ILineFormatter::Probe(BasicLineFormatter::DEFAULT)->FormatStatusLine(
            NULL, this, (ICharArrayBuffer**)&buf);
    return IObject::Probe(buf)->ToString(str);
}

void BasicStatusLine::CloneImpl(
    /* [in] */ BasicStatusLine* obj)
{
    obj->mReasonPhrase = mReasonPhrase;
    obj->mProtoVersion = mProtoVersion;
    obj->mStatusCode = mStatusCode;
}

} // namespace Message
} // namespace Http
} // namespace Apache
} // namespace Org
