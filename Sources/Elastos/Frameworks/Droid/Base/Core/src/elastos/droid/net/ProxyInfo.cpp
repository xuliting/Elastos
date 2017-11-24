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

#include <Elastos.CoreLibrary.Net.h>
#include "elastos/droid/net/ProxyInfo.h"
#include "elastos/droid/net/CProxy.h"
#include "elastos/droid/net/CProxyInfo.h"
#include "elastos/droid/net/Proxy.h"
#include "elastos/droid/net/Uri.h"
#include "elastos/droid/os/Build.h"
#include "elastos/droid/text/TextUtils.h"
#include <elastos/core/StringBuilder.h>
#include <elastos/core/StringUtils.h>
#include <elastos/utility/etl/List.h>

using Elastos::Droid::Os::Build;
using Elastos::Droid::Text::TextUtils;

using Elastos::Core::CString;
using Elastos::Core::ICharSequence;
using Elastos::Core::StringBuilder;
using Elastos::Core::StringUtils;
using Elastos::Net::CInetSocketAddress;
using Elastos::Net::CProxy;
using Elastos::Net::IInetSocketAddress;
using Elastos::Net::ISocketAddress;
using Elastos::Utility::Etl::List;
using Elastos::Utility::IIterator;
using Elastos::Utility::IList;

namespace Elastos {
namespace Droid {
namespace Net {

CAR_INTERFACE_IMPL(ProxyInfo, Object, IParcelable, IProxyInfo)

ProxyInfo::ProxyInfo()
    : mPort(0)
{}

ECode ProxyInfo::BuildDirectProxy(
    /* [in] */ const String& host,
    /* [in] */ Int32 port,
    /* [out] */ IProxyInfo** result)
{
    return CProxyInfo::New(host, port, String(NULL), result);
}

ECode ProxyInfo::BuildDirectProxy(
    /* [in] */ const String& host,
    /* [in] */ Int32 port,
    /* [in] */ IList* exclList,
    /* [out] */ IProxyInfo** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ArrayOf<IInterface*> > array;
    exclList->ToArray((ArrayOf<IInterface*>**)&array);
    String s(",");
    AutoPtr<ICharSequence> csq;
    CString::New(s, (ICharSequence**)&csq);
    String join = TextUtils::Join(csq, IIterable::Probe(exclList));
    AutoPtr<ArrayOf<String> > arrayString = ArrayOf<String>::Alloc(array->GetLength());
    for (Int32 i = 0; i < array->GetLength(); ++i) {
        ICharSequence::Probe((*array)[i])->ToString(&s);
        arrayString->Set(i, s);
    }
    AutoPtr<CProxyInfo> rev = new CProxyInfo();
    rev->constructor(host, port, join, arrayString);
    *result = IProxyInfo::Probe(rev);
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode ProxyInfo::BuildPacProxy(
    /* [in] */ IUri* pacUri,
    /* [out] */ IProxyInfo** result)
{
    return CProxyInfo::New(pacUri, result);
}

ECode ProxyInfo::constructor()
{
    return NOERROR;
}

ECode ProxyInfo::constructor(
    /* [in] */ const String& host,
    /* [in] */ Int32 port,
    /* [in] */ const String& exclList)
{
    mHost = host;
    mPort = port;
    SetExclusionList(exclList);
    Uri::GetEMPTY((IUri**)&mPacFileUrl);
    return NOERROR;
}

ECode ProxyInfo::constructor(
    /* [in] */ IUri* pacFileUrl)
{
    mHost = LOCAL_HOST;
    mPort = LOCAL_PORT;
    SetExclusionList(LOCAL_EXCL_LIST);
    if (pacFileUrl == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    mPacFileUrl = pacFileUrl;
    return NOERROR;
}

ECode ProxyInfo::constructor(
    /* [in] */ const String& pacFileUrl)
{
    mHost = LOCAL_HOST;
    mPort = LOCAL_PORT;
    SetExclusionList(LOCAL_EXCL_LIST);
    Uri::Parse(pacFileUrl, (IUri**)&mPacFileUrl);
    return NOERROR;
}

ECode ProxyInfo::constructor(
    /* [in] */ IUri* pacFileUrl,
    /* [in] */ Int32 localProxyPort)
{
    mHost = LOCAL_HOST;
    mPort = localProxyPort;
    SetExclusionList(LOCAL_EXCL_LIST);
    if (pacFileUrl == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    mPacFileUrl = pacFileUrl;
    return NOERROR;
}

ECode ProxyInfo::constructor(
    /* [in] */ const String& host,
    /* [in] */ Int32 port,
    /* [in] */ const String& exclList,
    /* [in] */ ArrayOf<String>* parsedExclList)
{
    mHost = host;
    mPort = port;
    mExclusionList = exclList;
    mParsedExclusionList = parsedExclList;
    Uri::GetEMPTY((IUri**)&mPacFileUrl);
    return NOERROR;
}

ECode ProxyInfo::constructor(
    /* [in] */ IProxyInfo* source)
{
    if (source != NULL) {
        source->GetHost(&mHost);
        source->GetPort(&mPort);
        mPacFileUrl = ((ProxyInfo*)source)->mPacFileUrl;
        source->GetExclusionListAsString(&mExclusionList);
        mParsedExclusionList = ((ProxyInfo*)source)->mParsedExclusionList;
    } else {
        Uri::GetEMPTY((IUri**)&mPacFileUrl);
    }
    return NOERROR;
}

ECode ProxyInfo::GetSocketAddress(
    /* [out] */ IInetSocketAddress** result)
{
    VALIDATE_NOT_NULL(result)

    *result = NULL;
    // try {
    ECode ec = CInetSocketAddress::New(mHost, mPort, result);
    // } catch (IllegalArgumentException e) { }
    if (FAILED(ec)) {
        if (ec != (ECode)E_ILLEGAL_ARGUMENT_EXCEPTION) return ec;
    }
    return NOERROR;
}

ECode ProxyInfo::GetPacFileUrl(
    /* [out] */ IUri** result)
{
    VALIDATE_NOT_NULL(result)

    *result = mPacFileUrl;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode ProxyInfo::SetPacFileUrl(
    /* [in] */ IUri* pacUri)
{
    mPacFileUrl = pacUri;
    return NOERROR;
}

ECode ProxyInfo::GetHost(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mHost;
    return NOERROR;
}

ECode ProxyInfo::GetPort(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mPort;
    return NOERROR;
}

ECode ProxyInfo::GetExclusionList(
    /* [out, callee] */ ArrayOf<String>** result)
{
    VALIDATE_NOT_NULL(result);

    *result = mParsedExclusionList;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode ProxyInfo::SetExclusionList(
        /* [in] */ ArrayOf<String>* exclusionList)
{
    mParsedExclusionList = exclusionList;
    return NOERROR;
}

ECode ProxyInfo::GetExclusionListAsString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    *result = mExclusionList;
    return NOERROR;
}

ECode ProxyInfo::SetExclusionList(
    /* [in] */ const String& exclusionList)
{
    mExclusionList = exclusionList;
    if (mExclusionList == NULL) {
        mParsedExclusionList = ArrayOf<String>::Alloc(0);
    }
    else {
        StringUtils::Split(exclusionList.ToLowerCase(), String(","), (ArrayOf<String>**)&mParsedExclusionList);
    }
    return NOERROR;
}

ECode ProxyInfo::IsValid(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<IUri> empty;
    Uri::GetEMPTY((IUri**)&empty);
    Boolean isEqual;
    IObject::Probe(empty)->Equals(mPacFileUrl, &isEqual);
    if (!isEqual) {
        *result = TRUE;
        return NOERROR;
    }
    Int32 validCode;
    Proxy::Validate(mHost == NULL ? String("") : mHost,
            mPort == 0 ? String("") : StringUtils::ToString(mPort),
            mExclusionList == NULL ? String("") : mExclusionList,
            &validCode);
    *result = IProxy::PROXY_VALID == validCode;
    return NOERROR;
}

ECode ProxyInfo::MakeProxy(
    /* [out] */ Elastos::Net::IProxy** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<Elastos::Net::IProxyHelper> helper;
    Elastos::Net::CProxyHelper::AcquireSingleton((Elastos::Net::IProxyHelper**)&helper);
    AutoPtr<Elastos::Net::IProxy> proxy;
    helper->GetNO_PROXY((Elastos::Net::IProxy**)&proxy);
    if (mHost != String(NULL)) {
        // try {
        AutoPtr<IInetSocketAddress> inetSocketAddress;
        ECode ec = CInetSocketAddress::New(mHost, mPort, (IInetSocketAddress**)&inetSocketAddress);
        if (FAILED(ec)) {
            if (ec != (ECode)E_ILLEGAL_ARGUMENT_EXCEPTION) return ec;
        }
        AutoPtr<Elastos::Net::IProxy> proxy;
        ec = Elastos::Net::CProxy::New(Elastos::Net::ProxyType_HTTP, ISocketAddress::Probe(inetSocketAddress), (Elastos::Net::IProxy**)&proxy);
        if (FAILED(ec)) {
            if (ec != (ECode)E_ILLEGAL_ARGUMENT_EXCEPTION) return ec;
        }
        // } catch (IllegalArgumentException e) {
        // }
    }
    *result = proxy;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode ProxyInfo::ToString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    StringBuilder sb;
    AutoPtr<IUri> empty;
    Uri::GetEMPTY((IUri**)&empty);
    Boolean isEqual;
    IObject::Probe(empty)->Equals(mPacFileUrl, &isEqual);
    if (!isEqual) {
        sb.Append("PAC Script: ");
        String s;
        IObject::Probe(mPacFileUrl)->ToString(&s);
        sb.Append(s);
    }
    else if (mHost != NULL) {
        sb.Append("[");
        sb.Append(mHost);
        sb.Append("] ");
        sb.Append(mPort);
        if (mExclusionList != NULL) {
                sb.Append(" xl=");
                sb.Append(mExclusionList);
        }
    }
    else {
        sb.Append("[ProxyProperties.mHost == NULL]");
    }
    sb.ToString(result);
    return NOERROR;
}

ECode ProxyInfo::Equals(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (TO_IINTERFACE(this) == IInterface::Probe(o)) {
        *result = TRUE;
        return NOERROR;
    }
    if (IProxyInfo::Probe(o) == NULL) {
        *result = FALSE;
        return NOERROR;
    }
    ProxyInfo* p = (ProxyInfo*)(IProxyInfo::Probe(o));
    // If PAC URL is present in either then they must be equal.
    // Other parameters will only be for fall back.
    AutoPtr<IUri> empty;
    Uri::GetEMPTY((IUri**)&empty);
    if (!Object::Equals(empty, mPacFileUrl)) {
        AutoPtr<IUri> othUri;
        p->GetPacFileUrl((IUri**)&othUri);
        *result = Object::Equals(mPacFileUrl, othUri) && mPort == p->mPort;
        return NOERROR;
    }
    if (!Object::Equals(empty, p->mPacFileUrl)) {
        *result = FALSE;
        return NOERROR;
    }
    String othExclusionList;
    if (!mExclusionList.IsNull() &&
            (p->GetExclusionListAsString(&othExclusionList), !mExclusionList.Equals(othExclusionList))) {
        *result = FALSE;
        return NOERROR;
    }
    String othHost;
    if (!mHost.IsNull() && (p->GetHost(&othHost), !othHost.IsNull()) && !mHost.Equals(othHost)) {
        *result = FALSE;
        return NOERROR;
    }
    if (!mHost.IsNull() && p->mHost.IsNull()) {
        *result = FALSE;
        return NOERROR;
    }
    if (mHost.IsNull() && !p->mHost.IsNull()) {
        *result = FALSE;
        return NOERROR;
    }
    if (mPort != p->mPort) {
        *result = FALSE;
        return NOERROR;
    }
    *result = TRUE;
    return NOERROR;
}

ECode ProxyInfo::GetHashCode(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)

    *result = (mHost.IsNull() ? 0 : mHost.GetHashCode())
            + (mExclusionList.IsNull() ? 0 : mExclusionList.GetHashCode())
            + mPort;
    return NOERROR;
}

ECode ProxyInfo::ReadFromParcel(
    /* [in] */ IParcel* parcel)
{
    Byte byte = 0;
    parcel->ReadByte(&byte);
    if (byte != 0) {
        AutoPtr<IInterface> obj;
        parcel->ReadInterfacePtr((Handle32*)&obj);
        parcel->ReadInt32(&mPort);
        return NOERROR;
    }
    byte = 0;
    parcel->ReadByte(&byte);
    if (byte != 0) {
        parcel->ReadString(&mHost);
        parcel->ReadInt32(&mPort);
    }
    parcel->ReadString(&mExclusionList);
    parcel->ReadArrayOfString((ArrayOf<String>**)&mParsedExclusionList);
    return NOERROR;
}

ECode ProxyInfo::WriteToParcel(
    /* [in] */ IParcel* dest)
{
    Boolean eqEmpty;
    AutoPtr<IUri> empty;
    Uri::Uri::GetEMPTY((IUri**)&empty);
    IObject::Probe(empty)->Equals(mPacFileUrl, &eqEmpty);
    if (!eqEmpty) {
        dest->WriteByte((Byte)1);
        dest->WriteInterfacePtr(mPacFileUrl);
        dest->WriteInt32(mPort);
        return NOERROR;
    }
    else {
        dest->WriteByte((Byte)0);
    }

    if (!mHost.IsNull()) {
        dest->WriteByte((Byte)1);
        dest->WriteString(mHost);
        dest->WriteInt32(mPort);
    }
    else {
        dest->WriteByte((Byte)0);
    }
    dest->WriteString(mExclusionList);
    dest->WriteArrayOfString(mParsedExclusionList);
    return NOERROR;
}

} // namespace Net
} // namespace Droid
} // namespace Elastos
