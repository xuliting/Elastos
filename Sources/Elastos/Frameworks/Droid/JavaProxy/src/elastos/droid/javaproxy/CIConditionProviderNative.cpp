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

#include "elastos/droid/javaproxy/CIConditionProviderNative.h"
#include "elastos/droid/javaproxy/Util.h"
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Service::Notification::EIID_IIConditionProvider;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace JavaProxy {

const String CIConditionProviderNative::TAG("CIConditionProviderNative");

CAR_INTERFACE_IMPL(CIConditionProviderNative, Object, IIConditionProvider, IBinder)

CAR_OBJECT_IMPL(CIConditionProviderNative)

CIConditionProviderNative::~CIConditionProviderNative()
{
    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);
    env->DeleteGlobalRef(mJInstance);
}

ECode CIConditionProviderNative::constructor(
    /* [in] */ Handle64 jVM,
    /* [in] */ Handle64 jInstance)
{
    mJVM = (JavaVM*)jVM;
    mJInstance = (jobject)jInstance;
    return NOERROR;
}

ECode CIConditionProviderNative::OnConnected()
{
    assert(0);
    return NOERROR;
}

ECode CIConditionProviderNative::OnRequestConditions(
    /* [in] */ Int32 relevance)
{
    assert(0);
    return NOERROR;
}

ECode CIConditionProviderNative::OnSubscribe(
    /* [in] */ IUri* conditionId)
{
    assert(0);
    return NOERROR;
}

ECode CIConditionProviderNative::OnUnsubscribe(
    /* [in] */ IUri* conditionId)
{
    assert(0);
    return NOERROR;
}

ECode CIConditionProviderNative::ToString(
    /* [out] */ String* str)
{
    // LOGGERD(TAG, "+ CIConditionProviderNative::ToString()");

    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);
    *str = Util::GetJavaToStringResult(env, mJInstance);

    // LOGGERD(TAG, "- CIConditionProviderNative::ToString()");
    return NOERROR;
}

} // namespace JavaProxy
} // namespace Droid
} // namespace Elastos
