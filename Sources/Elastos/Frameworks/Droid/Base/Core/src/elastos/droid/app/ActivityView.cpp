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
#include "elastos/droid/app/ActivityView.h"
#include "elastos/droid/app/ActivityManagerNative.h"
#include "elastos/droid/app/CActivityViewActivityContainerCallback.h"
#include "elastos/droid/view/Surface.h"
#include "elastos/droid/view/CTextureView.h"
#include "elastos/droid/utility/CDisplayMetrics.h"
#include <elastos/utility/logging/Logger.h>

#include <elastos/core/AutoLock.h>
using Elastos::Core::AutoLock;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Droid::Content::IContext;
using Elastos::Droid::Content::IContextWrapper;
using Elastos::Droid::View::Surface;
using Elastos::Droid::View::IDisplay;
using Elastos::Droid::View::IInputDevice;
using Elastos::Droid::View::ITextureView;
using Elastos::Droid::View::CTextureView;
using Elastos::Droid::View::IViewGroup;
using Elastos::Droid::View::IWindowManager;
using Elastos::Droid::View::ISurfaceTextureListener;
using Elastos::Droid::View::EIID_ISurfaceTextureListener;
using Elastos::Droid::Utility::CDisplayMetrics;
using Elastos::Core::ICloseGuardHelper;
using Elastos::Core::CCloseGuardHelper;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace App {

//=========================================================================
// ActivityView::ActivityViewSurfaceTextureListener
//=========================================================================

CAR_INTERFACE_IMPL(ActivityView::ActivityViewSurfaceTextureListener, Object, ISurfaceTextureListener)

ActivityView::ActivityViewSurfaceTextureListener::ActivityViewSurfaceTextureListener(
    /* [in] */ ActivityView* host)
    : mHost(host)
{
}

ECode ActivityView::ActivityViewSurfaceTextureListener::OnSurfaceTextureAvailable(
    /* [in] */ ISurfaceTexture* surfaceTexture,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height)
{
    if (mHost->mActivityContainer == NULL) {
        return NOERROR;
    }
    if (ActivityView::DEBUG)
        Logger::D("ActivityView::ActivityViewSurfaceTextureListener",
            "onSurfaceTextureAvailable: width=%d height=%d", width, height);

    mHost->mWidth = width;
    mHost->mHeight = height;
    mHost->AttachToSurfaceWhenReady();
    return NOERROR;
}

ECode ActivityView::ActivityViewSurfaceTextureListener::OnSurfaceTextureSizeChanged(
    /* [in] */ ISurfaceTexture* surfaceTexture,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height)
{
    if (mHost->mActivityContainer == NULL) {
        return NOERROR;
    }

    if (ActivityView::DEBUG)
        Logger::D("ActivityView::ActivityViewSurfaceTextureListener",
            "onSurfaceTextureSizeChanged: width=%d height=%d", width, height);
    return NOERROR;
}

ECode ActivityView::ActivityViewSurfaceTextureListener::OnSurfaceTextureDestroyed(
    /* [in] */ ISurfaceTexture* surfaceTexture,
    /* [out] */ Boolean* bval)
{
    VALIDATE_NOT_NULL(bval)
    *bval = TRUE;

    if (mHost->mActivityContainer == NULL) {
        return NOERROR;
    }
    if (ActivityView::DEBUG)
        Logger::D("ActivityView::ActivityViewSurfaceTextureListener",
            "OnSurfaceTextureDestroyed");

    mHost->mSurface->ReleaseResources();
    mHost->mSurface = NULL;
    // try {
    Int32 dpi;
    mHost->mMetrics->GetDensityDpi(&dpi);
    ECode ec = mHost->mActivityContainer->SetSurface(
        NULL, mHost->mWidth, mHost->mHeight, dpi);
    // } catch (RemoteException e) {
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        Logger::E("ActivityView::ActivityViewSurfaceTextureListener",
            "Unable to set surface of ActivityContainer. ");
        return E_RUNTIME_EXCEPTION;
    }
    // }
    return NOERROR;
}

ECode ActivityView::ActivityViewSurfaceTextureListener::OnSurfaceTextureUpdated(
    /* [in] */ ISurfaceTexture* surfaceTexture)
{
    Logger::D("ActivityView::ActivityViewSurfaceTextureListener", "onSurfaceTextureUpdated");
    return NOERROR;
}

ECode ActivityView::ActivityViewSurfaceTextureListener::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    *str = String("ActivityView::ActivityViewSurfaceTextureListener");
    return NOERROR;
}

//=========================================================================
// ActivityView::AllActivitiesCompleteRunnable
//=========================================================================
ActivityView::AllActivitiesCompleteRunnable::AllActivitiesCompleteRunnable(
    /* [in] */ IActivityViewCallback* callback,
    /* [in] */ IActivityView* view)
    : mCallback(callback)
    , mActivityView(view)
{}

ECode ActivityView::AllActivitiesCompleteRunnable::Run()
{
    return mCallback->OnAllActivitiesComplete(mActivityView);
}

//=========================================================================
// ActivityView::ActivityContainerCallback
//=========================================================================

CAR_INTERFACE_IMPL(ActivityView::ActivityContainerCallback, Object, IActivityContainerCallback, IBinder)

ActivityView::ActivityContainerCallback::ActivityContainerCallback()
{
}

ActivityView::ActivityContainerCallback::~ActivityContainerCallback()
{
}

ECode ActivityView::ActivityContainerCallback::constructor(
    /* [in] */ IActivityView* activityView)
{
    AutoPtr<IWeakReferenceSource> wrs = IWeakReferenceSource::Probe(activityView);
    return wrs->GetWeakReference((IWeakReference**)&mActivityViewWeakReference);
}

ECode ActivityView::ActivityContainerCallback::SetVisible(
    /* [in] */ IBinder* container,
    /* [in] */ Boolean visible)
{
    if (ActivityView::DEBUG)
        Logger::V("ActivityView::ActivityContainerCallback",
            "setVisible(): container=%s visible=%d.",
            TO_CSTR(container), visible);
    return NOERROR;
}

ECode ActivityView::ActivityContainerCallback::OnAllActivitiesComplete(
    /* [in] */ IBinder* container)
{
    AutoPtr<IActivityView> activityView;
    mActivityViewWeakReference->Resolve(EIID_IActivityView, (IInterface**)&activityView);

    if (activityView != NULL) {
        ActivityView* av = (ActivityView*)activityView.Get();
        AutoPtr<IActivityViewCallback> callback = av->mActivityViewCallback;
        if (callback != NULL) {
            AutoPtr<IRunnable> runnable = new AllActivitiesCompleteRunnable(callback, activityView);
            Boolean result;
            av->Post(runnable, &result);
        }
    }

    return NOERROR;
}

ECode ActivityView::ActivityContainerCallback::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    *str = String("ActivityView::ActivityContainerCallback");
    return NOERROR;
}

//=========================================================================
// ActivityView::ActivityContainerWrapper
//=========================================================================

ActivityView::ActivityContainerWrapper::ActivityContainerWrapper(
    /* [in] */ IIActivityContainer* container)
    : mOpened(TRUE)
{
    mActivityContainer = container;
    mOpened = TRUE;

    AutoPtr<ICloseGuardHelper> helper;
    CCloseGuardHelper::AcquireSingleton((ICloseGuardHelper**)&helper);
    helper->Get((ICloseGuard**)&mGuard);
    mGuard->Open(String("ActivityView::Release"));
}

ActivityView::ActivityContainerWrapper::~ActivityContainerWrapper()
{
    Finalize();
}

ECode ActivityView::ActivityContainerWrapper::AttachToDisplay(
    /* [in] */ Int32 displayId)
{
    // try {
    return mActivityContainer->AttachToDisplay(displayId);
    // } catch (RemoteException e) {
    // }
}

ECode ActivityView::ActivityContainerWrapper::SetSurface(
    /* [in] */ ISurface* surface,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height,
    /* [in] */ Int32 density)
{
    return mActivityContainer->SetSurface(surface, width, height, density);
}

ECode ActivityView::ActivityContainerWrapper::StartActivity(
    /* [in] */ IIntent* intent,
    /* [out] */ Int32* status)
{
    VALIDATE_NOT_NULL(status)

    // try {
    ECode ec = mActivityContainer->StartActivity(intent, status);
    // } catch (RemoteException e) {
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        Logger::E("ActivityView::ActivityContainerWrapper", "Unable to startActivity. ");
        ec = E_RUNTIME_EXCEPTION;
    }
    // }
    return ec;
}

ECode ActivityView::ActivityContainerWrapper::StartActivityIntentSender(
    /* [in] */ IIIntentSender* intentSender,
    /* [out] */ Int32* status)
{
    VALIDATE_NOT_NULL(status)
    // try {
    ECode ec = mActivityContainer->StartActivityIntentSender(intentSender, status);
    // } catch (RemoteException e) {
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        Logger::E("ActivityView::ActivityContainerWrapper", "Unable to startActivity from IntentSender. ");
        ec = E_RUNTIME_EXCEPTION;
    }
    // }
    return ec;
}

ECode ActivityView::ActivityContainerWrapper::CheckEmbeddedAllowed(
    /* [in] */ IIntent* intent)
{
    // try {
    ECode ec = mActivityContainer->CheckEmbeddedAllowed(intent);
    // } catch (RemoteException e) {
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        Logger::E("ActivityView::ActivityContainerWrapper", "Unable to startActivity from Intent.");
        ec = E_RUNTIME_EXCEPTION;
    }
    // }
    return ec;
}

ECode ActivityView::ActivityContainerWrapper::CheckEmbeddedAllowedIntentSender(
    /* [in] */ IIIntentSender* intentSender)
{
    // try {
    ECode ec = mActivityContainer->CheckEmbeddedAllowedIntentSender(intentSender);
    // } catch (RemoteException e) {
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        Logger::E("ActivityView::ActivityContainerWrapper", "Unable to startActivity from IntentSender.");
        ec = E_RUNTIME_EXCEPTION;
    }
    // }
    return ec;
}

ECode ActivityView::ActivityContainerWrapper::GetDisplayId(
    /* [out] */ Int32* status)
{
    VALIDATE_NOT_NULL(status)

    ECode ec = mActivityContainer->GetDisplayId(status);
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        ec = NOERROR;
        *status = -1;
    }
    return ec;
}

ECode ActivityView::ActivityContainerWrapper::InjectEvent(
    /* [in] */ IInputEvent* event,
    /* [out] */ Boolean* bval)
{
    VALIDATE_NOT_NULL(bval)

    ECode ec = mActivityContainer->InjectEvent(event, bval);
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        ec = NOERROR;
        *bval = FALSE;
    }
    return ec;
}

ECode ActivityView::ActivityContainerWrapper::ReleaseResources()
{
    {    AutoLock syncLock(mGuard);
        if (mOpened) {
            if (ActivityView::DEBUG)
                Logger::V("ActivityView", "ActivityContainerWrapper: release called");
            // try {
                mActivityContainer->ReleaseResources();
                mGuard->Close();
            // } catch (RemoteException e) {
            // }
            mOpened = FALSE;
        }
    }
    return NOERROR;
}

void ActivityView::ActivityContainerWrapper::Finalize()
{
    if (ActivityView::DEBUG)
        Logger::V("ActivityView", "ActivityContainerWrapper: finalize called");
    // try {
        if (mGuard != NULL) {
            mGuard->WarnIfOpen();
            ReleaseResources();
        }
    // } finally {
    //     super.finalize();
    // }
}

ECode ActivityView::ActivityContainerWrapper::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)
    *str = String("ActivityView::ActivityContainerWrapper");
    return NOERROR;
}

//=========================================================================
// ActivityView
//=========================================================================

const String ActivityView::TAG("ActivityView");
const Boolean ActivityView::DEBUG = FALSE;

CAR_INTERFACE_IMPL(ActivityView, ViewGroup, IActivityView)

ActivityView::ActivityView()
    : mWidth(0)
    , mHeight(0)
    , mLastVisibility(0)
{}

ActivityView::~ActivityView()
{}

ECode ActivityView::constructor(
    /* [in] */ IContext* context)
{
    return constructor(context, NULL);
}

ECode ActivityView::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs)
{
    return constructor(context, attrs, 0);
}

ECode ActivityView::constructor(
    /* [in] */ IContext* ctx,
    /* [in] */ IAttributeSet* attrs,
    /* [in] */ Int32 defStyle)
{
    FAIL_RETURN(ViewGroup::constructor(ctx, attrs, defStyle))

    AutoPtr<IContext> context = ctx;
    while (IContextWrapper::Probe(context) != NULL) {
        if (IActivity::Probe(context) != NULL) {
            mActivity = IActivity::Probe(context);
            break;
        }
        AutoPtr<IContextWrapper> cw = IContextWrapper::Probe(context);
        context = NULL;
        cw->GetBaseContext((IContext**)&context);
    }
    if (mActivity == NULL) {
        Logger::E(TAG, "The ActivityView's Context is not an Activity.");
        return E_ILLEGAL_STATE_EXCEPTION;
    }

    // try {
    AutoPtr<IBinder> token;
    mActivity->GetActivityToken((IBinder**)&token);
    AutoPtr<IActivityContainerCallback> cb;
    CActivityViewActivityContainerCallback::New(this, (IActivityContainerCallback**)&cb);
    AutoPtr<IIActivityContainer> ac;
    ECode ec = ActivityManagerNative::GetDefault()->CreateActivityContainer(
        token, cb, (IIActivityContainer**)&ac);

    // } catch (RemoteException e)
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        Logger::E(TAG, "ActivityView: Unable to create ActivityContainer. ");
        return E_RUNTIME_EXCEPTION;
    }

    mActivityContainer = new ActivityContainerWrapper(ac);

    CTextureView::New(context, (ITextureView**)&mTextureView);
    AutoPtr<ISurfaceTextureListener> listener = new ActivityViewSurfaceTextureListener(this);
    mTextureView->SetSurfaceTextureListener(listener);
    AddView(IView::Probe(mTextureView));

    AutoPtr<IInterface> obj;
    IContext::Probe(mActivity)->GetSystemService(IContext::WINDOW_SERVICE, (IInterface**)&obj);
    AutoPtr<IWindowManager> wm = IWindowManager::Probe(obj);
    CDisplayMetrics::New((IDisplayMetrics**)&mMetrics);
    AutoPtr<IDisplay> display;
    wm->GetDefaultDisplay((IDisplay**)&display);
    display->GetMetrics(mMetrics);

    GetVisibility(&mLastVisibility);

    if (DEBUG) Logger::V(TAG, "ctor()");
    return NOERROR;
}

ECode ActivityView::OnLayout(
    /* [in] */ Boolean changed,
    /* [in] */ Int32 l,
    /* [in] */ Int32 t,
    /* [in] */ Int32 r,
    /* [in] */ Int32 b)
{
    return IView::Probe(mTextureView)->Layout(0, 0, r - l, b - t);
}

ECode ActivityView::OnVisibilityChanged(
    /* [in] */ IView* changedView,
    /* [in] */ Int32 visibility)
{
    FAIL_RETURN(ViewGroup::OnVisibilityChanged(changedView, visibility))

    if (mSurface != NULL) {
        // try {
            Int32 dpi;
            if (visibility == IView::GONE) {
                mMetrics->GetDensityDpi(&dpi);
                mActivityContainer->SetSurface(NULL, mWidth, mHeight, dpi);
            } else if (mLastVisibility == IView::GONE) {
                mMetrics->GetDensityDpi(&dpi);
                // Don't change surface when going between IView::VISIBLE and IView::INVISIBLE.
                mActivityContainer->SetSurface(mSurface, mWidth, mHeight, dpi);
            }
        // } catch (RemoteException e) {
            Logger::E(TAG, "ActivityView: Unable to set surface of ActivityContainer. ");
            return E_RUNTIME_EXCEPTION;
        // }
    }
    mLastVisibility = visibility;
    return NOERROR;
}

Boolean ActivityView::InjectInputEvent(
    /* [in] */ IInputEvent* event)
{
    if (mActivityContainer == NULL)
        return FALSE;

    Boolean val;
    mActivityContainer->InjectEvent(event, &val);
    return val;
}

ECode ActivityView::OnTouchEvent(
    /* [in] */ IMotionEvent* event,
    /* [out] */ Boolean* bval)
{
    VALIDATE_NOT_NULL(bval)
    IInputEvent* e = IInputEvent::Probe(event);
    *bval = InjectInputEvent(e);
    if (*bval == FALSE) {
        return ViewGroup::OnTouchEvent(event, bval);
    }
    return NOERROR;
}

ECode ActivityView::OnGenericMotionEvent(
    /* [in] */ IMotionEvent* event,
    /* [out] */ Boolean* bval)
{
    VALIDATE_NOT_NULL(bval)
    *bval = FALSE;

    IInputEvent* e = IInputEvent::Probe(event);
    Boolean fromSource;
    e->IsFromSource(IInputDevice::SOURCE_CLASS_POINTER, &fromSource);
    if (fromSource) {
        if (InjectInputEvent(e)) {
            *bval = TRUE;
            return NOERROR;
        }
    }
    return ViewGroup::OnGenericMotionEvent(event, bval);
}

ECode ActivityView::OnAttachedToWindow()
{
    if (DEBUG) Logger::V(TAG, "onAttachedToWindow(): mActivityContainer=  mSurface=");
    return NOERROR;
}

ECode ActivityView::OnDetachedFromWindow()
{
    if (DEBUG) Logger::V(TAG, "onDetachedFromWindow(): mActivityContainer= mSurface=");
    return NOERROR;
}

ECode ActivityView::IsAttachedToDisplay(
    /* [out] */ Boolean* bval)
{
    VALIDATE_NOT_NULL(bval)
    *bval = mSurface != NULL;
    return NOERROR;
}

ECode ActivityView::StartActivity(
    /* [in] */ IIntent* intent)
{
    if (mActivityContainer == NULL) {
        Logger::E(TAG, "Attempt to call startActivity after release");
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    Boolean bval;
    IsAttachedToDisplay(&bval);
    if (DEBUG) Logger::V(TAG, "startActivity(): intent=%s %s attached",
            TO_CSTR(intent), (bval ? "" : "not"));
    if (mSurface != NULL) {
        Int32 ival;
        mActivityContainer->StartActivity(intent, &ival);
    }
    else {
        FAIL_RETURN(mActivityContainer->CheckEmbeddedAllowed(intent))
        mQueuedIntent = intent;
        mQueuedPendingIntent = NULL;
    }
    return NOERROR;
}

ECode ActivityView::StartActivity(
    /* [in] */ IIntentSender* intentSender)
{
    if (mActivityContainer == NULL) {
        Logger::E(TAG, "Attempt to call startActivity after release");
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    Boolean bval;
    IsAttachedToDisplay(&bval);
    if (DEBUG) Logger::V(TAG, "startActivityIntentSender(): intentSender=%s %s attached",
        TO_CSTR(intentSender), (bval ? "" : "not"));
    AutoPtr<IIIntentSender> iIntentSender;
    intentSender->GetTarget((IIIntentSender**)&iIntentSender);
    if (mSurface != NULL) {
        Int32 ival;
        mActivityContainer->StartActivityIntentSender(iIntentSender, &ival);
    }
    else {
        FAIL_RETURN(mActivityContainer->CheckEmbeddedAllowedIntentSender(iIntentSender))
        mQueuedPendingIntent = iIntentSender;
        mQueuedIntent = NULL;
    }
    return NOERROR;
}

ECode ActivityView::StartActivity(
    /* [in] */ IPendingIntent* pendingIntent)
{
    if (mActivityContainer == NULL) {
        Logger::E(TAG, "Attempt to call startActivity after release");
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    if (DEBUG) {
        Boolean bval;
        IsAttachedToDisplay(&bval);
        Logger::V(TAG, "startActivityPendingIntent(): PendingIntent=%s %s attached",
            TO_CSTR(pendingIntent), bval ? "" : "not");
    }
    AutoPtr<IIIntentSender> iIntentSender;
    pendingIntent->GetTarget((IIIntentSender**)&iIntentSender);
    if (mSurface != NULL) {
        Int32 ival;
        mActivityContainer->StartActivityIntentSender(iIntentSender, &ival);
    }
    else {
        FAIL_RETURN(mActivityContainer->CheckEmbeddedAllowedIntentSender(iIntentSender))
        mQueuedPendingIntent = iIntentSender;
        mQueuedIntent = NULL;
    }
    return NOERROR;
}

ECode ActivityView::ReleaseResources()
{
    if (DEBUG) Logger::V(TAG, "release() mActivityContainer= mSurface=");
    if (mActivityContainer == NULL) {
        Logger::E(TAG, "Duplicate call to release");
        return NOERROR;
    }
    mActivityContainer->ReleaseResources();
    mActivityContainer = NULL;

    if (mSurface != NULL) {
        mSurface->ReleaseResources();
        mSurface = NULL;
    }

    return mTextureView->SetSurfaceTextureListener(NULL);
}

ECode ActivityView::AttachToSurfaceWhenReady()
{
    AutoPtr<ISurfaceTexture> surfaceTexture;
    mTextureView->GetSurfaceTexture((ISurfaceTexture**)&surfaceTexture);
    if (surfaceTexture == NULL || mSurface != NULL) {
        // Either not ready to attach, or already attached.
        return NOERROR;
    }

    AutoPtr<Surface> surface = new  Surface();
    surface->constructor(surfaceTexture);
    mSurface = (ISurface*)surface.Get();

    // try {
    Int32 dpi;
    mMetrics->GetDensityDpi(&dpi);
    ECode ec = mActivityContainer->SetSurface(mSurface, mWidth, mHeight, dpi);
    // } catch (RemoteException e) {
    if (ec == (ECode)E_REMOTE_EXCEPTION) {
        mSurface->ReleaseResources();
        mSurface = NULL;
        Logger::E(TAG, "ActivityView: Unable to create ActivityContainer. ");
        return E_RUNTIME_EXCEPTION;
    }

    if (DEBUG) Logger::V(TAG, "attachToSurfaceWhenReady: %s  queued intent",
        (mQueuedIntent != NULL || mQueuedPendingIntent != NULL ? "" : "no"));
    if (mQueuedIntent != NULL) {
        Int32 ival;
        mActivityContainer->StartActivity(mQueuedIntent, &ival);
        mQueuedIntent = NULL;
    }
    else if (mQueuedPendingIntent != NULL) {
        Int32 ival;
        mActivityContainer->StartActivityIntentSender(mQueuedPendingIntent, &ival);
        mQueuedPendingIntent = NULL;
    }
    return NOERROR;
}

ECode ActivityView::SetCallback(
    /* [in] */ IActivityViewCallback* callback)
{
    mActivityViewCallback = callback;
    return NOERROR;
}

} // namespace App
} // namespace Droid
} // namespace Elastos
