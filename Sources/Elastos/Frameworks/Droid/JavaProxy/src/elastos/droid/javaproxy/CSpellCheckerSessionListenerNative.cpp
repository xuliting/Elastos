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

#include "elastos/droid/javaproxy/CSpellCheckerSessionListenerNative.h"
#include "elastos/droid/javaproxy/Util.h"
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Internal::TextService::EIID_IISpellCheckerSessionListener;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace JavaProxy {

const String CSpellCheckerSessionListenerNative::TAG("CSpellCheckerSessionListenerNative");

CAR_INTERFACE_IMPL(CSpellCheckerSessionListenerNative, Object, IISpellCheckerSessionListener, IBinder)

CAR_OBJECT_IMPL(CSpellCheckerSessionListenerNative)

CSpellCheckerSessionListenerNative::~CSpellCheckerSessionListenerNative()
{
    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);
    env->DeleteGlobalRef(mJInstance);
}

ECode CSpellCheckerSessionListenerNative::constructor(
    /* [in] */ Handle64 jVM,
    /* [in] */ Handle64 jInstance)
{
    mJVM = (JavaVM*)jVM;
    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);
    mJInstance = env->NewGlobalRef((jobject)jInstance);

    return NOERROR;
}

ECode CSpellCheckerSessionListenerNative::OnGetSuggestions(
    /* [in] */ ArrayOf<ISuggestionsInfo*>* results)
{
    // LOGGERD(TAG, "+ CSpellCheckerSessionListenerNative::OnGetSuggestions()");

    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);

    jobjectArray jresults = NULL;
    Int32 count = results->GetLength();
    if (count > 0) {
        jclass siKlass = env->FindClass("android/view/textservice/SuggestionsInfo");
        Util::CheckErrorAndLog(env, TAG, "FindClass: SuggestionsInfo %d", __LINE__);

        jresults = env->NewObjectArray((jsize)count, siKlass, 0);
        Util::CheckErrorAndLog(env, TAG, "NewObjectArray: SuggestionsInfo %d", __LINE__);

        for (Int32 i = 0; i < count; i++) {
            AutoPtr<ISuggestionsInfo> info = (*results)[i];
            jobject jinfo = Util::ToJavaSuggestionsInfo(env, info);

            env->SetObjectArrayElement(jresults, i, jinfo);
            Util::CheckErrorAndLog(env, TAG, "SetObjectArrayElement: SuggestionsInfo %d", __LINE__);

            env->DeleteLocalRef(jinfo);
        }

        env->DeleteLocalRef(siKlass);
    }

    jclass c = env->FindClass("com/android/internal/textservice/ISpellCheckerSessionListener");
    Util::CheckErrorAndLog(env, TAG, "FindClass: ISpellCheckerSessionListener %d", __LINE__);

    jmethodID m = env->GetMethodID(c, "onGetSuggestions", "([Landroid/view/textservice/SuggestionsInfo;)V");
    Util::CheckErrorAndLog(env, TAG, "GetMethodID: onGetSuggestions %d", __LINE__);

    env->CallVoidMethod(mJInstance, m, jresults);
    Util::CheckErrorAndLog(env, TAG, "CallVoidMethod: onGetSuggestions %d", __LINE__);

    env->DeleteLocalRef(c);
    env->DeleteLocalRef(jresults);

    // LOGGERD(TAG, "- CSpellCheckerSessionListenerNative::OnGetSuggestions()");
    return NOERROR;
}

ECode CSpellCheckerSessionListenerNative::OnGetSentenceSuggestions(
    /* [in] */ ArrayOf<ISentenceSuggestionsInfo*>* results)
{
    // LOGGERD(TAG, "+ CSpellCheckerSessionListenerNative::OnGetSentenceSuggestions()");

    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);

    jobjectArray jresults = NULL;
    Int32 count = results->GetLength();
    if (count > 0) {
        jclass ssiKlass = env->FindClass("android/view/textservice/SentenceSuggestionsInfo");
        Util::CheckErrorAndLog(env, TAG, "FindClass: SentenceSuggestionsInfo %d", __LINE__);

        jresults = env->NewObjectArray((jsize)count, ssiKlass, 0);
        Util::CheckErrorAndLog(env, TAG, "NewObjectArray: SentenceSuggestionsInfo %d", __LINE__);

        for (Int32 i = 0; i < count; i++) {
            AutoPtr<ISentenceSuggestionsInfo> info = (*results)[i];
            jobject jinfo = Util::ToJavaSentenceSuggestionsInfo(env, info);

            env->SetObjectArrayElement(jresults, i, jinfo);
            Util::CheckErrorAndLog(env, TAG, "SetObjectArrayElement: SentenceSuggestionsInfo %d", __LINE__);

            env->DeleteLocalRef(jinfo);
        }

        env->DeleteLocalRef(ssiKlass);
    }

    jclass c = env->FindClass("com/android/internal/textservice/ISpellCheckerSessionListener");
    Util::CheckErrorAndLog(env, TAG, "FindClass: ISpellCheckerSessionListener %d", __LINE__);

    jmethodID m = env->GetMethodID(c, "onGetSentenceSuggestions", "([Landroid/view/textservice/SentenceSuggestionsInfo;)V");
    Util::CheckErrorAndLog(env, TAG, "GetMethodID: onGetSentenceSuggestions %d", __LINE__);

    env->CallVoidMethod(mJInstance, m, jresults);
    Util::CheckErrorAndLog(env, TAG, "CallVoidMethod: onGetSentenceSuggestions %d", __LINE__);

    env->DeleteLocalRef(c);

    // LOGGERD(TAG, "- CSpellCheckerSessionListenerNative::OnGetSentenceSuggestions()");
    return NOERROR;
}

ECode CSpellCheckerSessionListenerNative::ToString(
    /* [out] */ String* str)
{
    // LOGGERD(TAG, "+ CSpellCheckerSessionListenerNative::ToString()");
    JNIEnv* env;
    mJVM->AttachCurrentThread(&env, NULL);

    jclass c = env->FindClass("java/lang/Object");
    Util::CheckErrorAndLog(env, TAG, "FindClass: Object", __LINE__);

    jmethodID m = env->GetMethodID(c, "toString", "()Ljava/lang/String;");
    Util::CheckErrorAndLog(env, TAG, "GetMethodID: toString", __LINE__);

    jstring jstr = (jstring)env->CallObjectMethod(mJInstance, m);
    Util::CheckErrorAndLog(env, TAG, "CallVoidMethod: toString", __LINE__);

    *str = Util::GetElString(env, jstr);

    env->DeleteLocalRef(c);
    env->DeleteLocalRef(jstr);

    // LOGGERD(TAG, "- CSpellCheckerSessionListenerNative::ToString()");
    return NOERROR;
}

}
}
}

