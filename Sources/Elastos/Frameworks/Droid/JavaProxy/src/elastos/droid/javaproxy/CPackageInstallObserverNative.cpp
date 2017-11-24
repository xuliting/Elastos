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

#include "elastos/droid/javaproxy/CPackageInstallObserverNative.h"
#include "elastos/droid/javaproxy/Util.h"
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Content::Pm::EIID_IIPackageInstallObserver;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace JavaProxy {

const String CPackageInstallObserverNative::TAG("CPackageInstallObserverNative");

CAR_INTERFACE_IMPL(CPackageInstallObserverNative, Object, IIPackageInstallObserver, IBinder)

CAR_OBJECT_IMPL(CPackageInstallObserverNative)

CPackageInstallObserverNative::~CPackageInstallObserverNative()
{
    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);
    env->DeleteGlobalRef(mJInstance);
}

ECode CPackageInstallObserverNative::constructor(
    /* [in] */ Handle64 jVM,
    /* [in] */ Handle64 jInstance)
{
    mJVM = (JavaVM*)jVM;
    mJInstance = (jobject)jInstance;
    return NOERROR;
}

ECode CPackageInstallObserverNative::PackageInstalled(
    /* [in] */ const String& packageName,
    /* [in] */ Int32 returnCode)
{
    // LOGGERD(TAG, "+ CPackageInstallObserverNative::PackageInstalled()");

    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);

    jstring jstr = Util::ToJavaString(env, packageName);

    jclass c = env->FindClass("android/content/pm/IPackageInstallObserver");
    Util::CheckErrorAndLog(env, TAG, "Fail FindClass: IPackageInstallObserver %d", __LINE__);

    jmethodID m = env->GetMethodID(c, "packageInstalled", "(Ljava/lang/String;I)V");
    Util::CheckErrorAndLog(env, TAG, "GetMethodID: packageInstalled %d", __LINE__);

    env->CallVoidMethod(mJInstance, m, jstr, (jint)returnCode);
    Util::CheckErrorAndLog(env, TAG, "CallVoidMethod: packageInstalled %d", __LINE__);

    env->DeleteLocalRef(c);
    env->DeleteLocalRef(jstr);

    // LOGGERD(TAG, "- CPackageInstallObserverNative::PackageInstalled()");
    return NOERROR;
}

ECode CPackageInstallObserverNative::ToString(
    /* [out] */ String* str)
{
    // LOGGERD(TAG, "+ CPackageInstallObserverNative::ToString()");
    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);

    jclass c = env->FindClass("java/lang/Object");
    Util::CheckErrorAndLog(env, "ToString", "FindClass: Object", __LINE__);

    jmethodID m = env->GetMethodID(c, "toString", "()Ljava/lang/String;");
    Util::CheckErrorAndLog(env, TAG, "GetMethodID: toString", __LINE__);

    jstring jstr = (jstring)env->CallObjectMethod(mJInstance, m);
    Util::CheckErrorAndLog(env, TAG, "CallVoidMethod: toString", __LINE__);

    *str = Util::GetElString(env, jstr);

    env->DeleteLocalRef(c);
    env->DeleteLocalRef(jstr);

    // LOGGERD(TAG, "- CPackageInstallObserverNative::ToString()");
    return NOERROR;
}

}
}
}

