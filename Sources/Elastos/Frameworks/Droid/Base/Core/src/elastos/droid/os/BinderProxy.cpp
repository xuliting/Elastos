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
#include "elastos/droid/os/BinderProxy.h"
#include "elastos/droid/ext/frameworkext.h"
#include <elastos/core/StringBuilder.h>
#include <elastos/core/StringUtils.h>
#include <binder/IBinder.h>

using Elastos::Core::StringBuilder;
using Elastos::Core::StringUtils;

namespace Elastos {
namespace Droid {
namespace Os {

CAR_INTERFACE_IMPL(BinderProxy, Object, IBinderProxy, IBinder)

BinderProxy::BinderProxy()
    : mObject(0)
{}

BinderProxy::~BinderProxy()
{
    Destroy();
}

ECode BinderProxy::GetHashCode(
    /* [out] */ Int32* hash)
{
    VALIDATE_NOT_NULL(hash);
    *hash = (Int32)this;
    return NOERROR;
}

ECode BinderProxy::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    StringBuilder sb("BinderProxy:{");
    sb += StringUtils::ToHexString((Int32)this);
    sb += "}";
    *str = sb.ToString();
    return NOERROR;
}

void BinderProxy::Destroy()
{
    android::IBinder* b = (android::IBinder*)mObject;

    b->decStrong(this);
}

} // namespace Os
} // namespace Droid
} // namespace Elastos
