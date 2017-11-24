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

#include "elastos/droid/javaproxy/CINotificationListenerNative.h"
#include "elastos/droid/javaproxy/Util.h"
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Service::Notification::EIID_IINotificationListener;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace JavaProxy {

const String CINotificationListenerNative::TAG("CINotificationListenerNative");

CAR_INTERFACE_IMPL(CINotificationListenerNative, Object, IINotificationListener, IBinder)

CAR_OBJECT_IMPL(CINotificationListenerNative)

CINotificationListenerNative::~CINotificationListenerNative()
{
    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);
    env->DeleteGlobalRef(mJInstance);
}

ECode CINotificationListenerNative::constructor(
    /* [in] */ Handle64 jVM,
    /* [in] */ Handle64 jInstance)
{
    mJVM = (JavaVM*)jVM;
    mJInstance = (jobject)jInstance;
    return NOERROR;
}

ECode CINotificationListenerNative::OnListenerConnected(
    /* [in] */ INotificationRankingUpdate* update)
{
    assert(0);
    return NOERROR;
}

ECode CINotificationListenerNative::OnNotificationPosted(
    /* [in] */ IIStatusBarNotificationHolder* notificationHolder,
    /* [in] */ INotificationRankingUpdate* update)
{
    assert(0);
    return NOERROR;
}

ECode CINotificationListenerNative::OnNotificationRemoved(
    /* [in] */ IIStatusBarNotificationHolder* notificationHolder,
    /* [in] */ INotificationRankingUpdate* update)
{
    assert(0);
    return NOERROR;
}

ECode CINotificationListenerNative::OnNotificationRankingUpdate(
    /* [in] */ INotificationRankingUpdate* update)
{
    assert(0);
    return NOERROR;
}

ECode CINotificationListenerNative::OnListenerHintsChanged(
    /* [in] */ Int32 hints)
{
    assert(0);
    return NOERROR;
}

ECode CINotificationListenerNative::OnInterruptionFilterChanged(
    /* [in] */ Int32 interruptionFilter)
{
    assert(0);
    return NOERROR;
}

ECode CINotificationListenerNative::ToString(
    /* [out] */ String* str)
{
    // LOGGERD(TAG, "+ CINotificationListenerNative::ToString()");

    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);
    *str = Util::GetJavaToStringResult(env, mJInstance);

    // LOGGERD(TAG, "- CINotificationListenerNative::ToString()");
    return NOERROR;
}

} // namespace JavaProxy
} // namespace Droid
} // namespace Elastos
