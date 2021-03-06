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

#ifndef __ELASTOS_DROID_APP_APP_IMPORTANCE_MONITOR_H__
#define __ELASTOS_DROID_APP_APP_IMPORTANCE_MONITOR_H__

#include "elastos/droid/ext/frameworkext.h"
#include "Elastos.Droid.App.h"
#include "elastos/droid/view/ViewGroup.h"
#include "elastos/droid/os/Handler.h"
#include <elastos/utility/etl/HashMap.h>

using Elastos::Droid::Os::IBinder;
using Elastos::Droid::Os::Handler;
using Elastos::Droid::Content::IContext;
using Elastos::Utility::Etl::HashMap;

namespace Elastos {
namespace Droid {
namespace App {


/**
 * Helper for monitoring the current importance of applications.
 * @hide
 */
class AppImportanceMonitor
    : public Object
    , public IAppImportanceMonitor
{
private:
    class MyHandler
        : public Handler
    {
    public:
        TO_STRING_IMPL("AppImportanceMonitor::MyHandler")

        MyHandler(
            /* [in] */ AppImportanceMonitor* host);

        CARAPI HandleMessage(
            /* [in] */ IMessage* msg);

    private:
        AppImportanceMonitor* mHost;
    };

    class AppEntry
        : public Object
    {
    public:
        Int32 mUid;
        HashMap<Int32, Int32> mProcs;
        Int32 mImportance;

        AppEntry(
            /* [in] */ Int32 uid);
    };

public:
    class ProcessObserver
        : public Object
        , public IIProcessObserver
        , public IBinder
    {
    public:
        CAR_INTERFACE_DECL()

        ProcessObserver();

        virtual ~ProcessObserver();

        CARAPI constructor(
            /* [in] */ IAppImportanceMonitor* host);

        CARAPI OnForegroundActivitiesChanged(
            /* [in] */ Int32 pid,
            /* [in] */ Int32 uid,
            /* [in] */ Boolean foregroundActivities);

        CARAPI OnProcessStateChanged(
            /* [in] */ Int32 pid,
            /* [in] */ Int32 uid,
            /* [in] */ Int32 procState);

        CARAPI OnProcessDied(
            /* [in] */ Int32 pid,
            /* [in] */ Int32 uid);

        CARAPI ToString(
            /* [out] */ String* str);

    private:
        AppImportanceMonitor* mHost;
    };

public:
    CAR_INTERFACE_DECL()

    AppImportanceMonitor();

    virtual ~AppImportanceMonitor();

    CARAPI constructor(
        /* [in] */ IContext* context,
        /* [in] */ ILooper* looper);

    CARAPI GetImportance(
        /* [in] */ Int32 uid,
        /* [out] */ Int32* result);

private:
    /**
     * Report when an app's importance changed. Called on looper given to constructor.
     */
    CARAPI OnImportanceChanged(
        /* [in] */ Int32 uid,
        /* [in] */ Int32 importance,
        /* [in] */ Int32 oldImportance);

    void UpdateImportanceLocked(
        /* [in] */ Int32 uid,
        /* [in] */ Int32 pid,
        /* [in] */ Int32 importance,
        /* [in] */ Boolean repChange);

    void UpdateImportanceLocked(
        /* [in] */ AppEntry* ent,
        /* [in] */ Boolean repChange);

private:
    friend class MyHandler;
    friend class ProcessObserver;

    AutoPtr<IContext> mContext;

    HashMap<Int32, AutoPtr<AppEntry> > mApps;
    Object mAppsLock;

    AutoPtr<IIProcessObserver> mProcessObserver;

    static const Int32 MSG_UPDATE;

    AutoPtr<MyHandler> mHandler;
};


} // namespace App
} // namespace Droid
} // namespace Elastos

#endif //__ELASTOS_DROID_APP_APP_IMPORTANCE_MONITOR_H__

