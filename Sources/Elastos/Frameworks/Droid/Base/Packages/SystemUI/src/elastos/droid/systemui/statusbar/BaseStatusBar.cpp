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

#include "elastos/droid/systemui/statusbar/BaseStatusBar.h"
#include "elastos/droid/systemui/statusbar/CBaseBroadcastReceiver.h"
#include "elastos/droid/systemui/statusbar/CCommandQueue.h"
#include "elastos/droid/systemui/statusbar/CLockscreenSettingsObserver.h"
#include "elastos/droid/systemui/statusbar/CNotificationListenerService.h"
#include "elastos/droid/systemui/statusbar/CStatusBarIconView.h"
#include "elastos/droid/systemui/statusbar/CSettingsObserver.h"
#include "elastos/droid/systemui/statusbar/phone/KeyguardTouchDelegate.h"
#include "elastos/droid/systemui/statusbar/policy/PreviewInflater.h"
#include "R.h"
#include "Elastos.Droid.App.h"
#include "Elastos.Droid.Internal.h"
#include "Elastos.Droid.Provider.h"
#include <elastos/droid/app/ActivityManagerNative.h>
#include <elastos/droid/os/Build.h>
#include <elastos/droid/os/AsyncTask.h>
#include <elastos/droid/os/UserHandle.h>
#include <elastos/droid/os/ServiceManager.h>
#include <elastos/droid/provider/Settings.h>
#include <elastos/droid/R.h>
#include <elastos/droid/text/TextUtils.h>
#include <elastos/droid/view/LayoutInflater.h>
#include <elastos/core/AutoLock.h>
#include <elastos/core/Math.h>
#include <elastos/core/StringUtils.h>
#include <elastos/core/CoreUtils.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::App::ActivityManagerNative;
using Elastos::Droid::App::CActivityManager;
using Elastos::Droid::App::CActivityManagerHelper;
using Elastos::Droid::App::CNotificationBuilder;
using Elastos::Droid::App::CNotificationBuilderHelper;
using Elastos::Droid::App::CPendingIntentHelper;
using Elastos::Droid::App::IActivityManagerHelper;
using Elastos::Droid::App::INotificationBuilder;
using Elastos::Droid::App::INotificationBuilderHelper;
using Elastos::Droid::App::INotificationManager;
using Elastos::Droid::App::IPendingIntentHelper;
using Elastos::Droid::App::IKeyguardManager;
using Elastos::Droid::App::ITaskStackBuilder;
using Elastos::Droid::App::ITaskStackBuilderHelper;
using Elastos::Droid::App::CTaskStackBuilderHelper;
using Elastos::Droid::App::IActivityOptions;
using Elastos::Droid::App::IActivityOptionsHelper;
using Elastos::Droid::App::CActivityOptionsHelper;
using Elastos::Droid::App::CWallpaperManagerHelper;
using Elastos::Droid::Content::CComponentName;
using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::CIntentFilter;
using Elastos::Droid::Content::IContentResolver;
using Elastos::Droid::Content::IIntentFilter;
using Elastos::Droid::Content::Pm::IActivityInfo;
using Elastos::Droid::Content::Pm::IApplicationInfo;
using Elastos::Droid::Content::Pm::IPackageItemInfo;
using Elastos::Droid::Content::Pm::IPackageManager;
using Elastos::Droid::Content::Pm::IUserInfo;
using Elastos::Droid::Graphics::IRect;
using Elastos::Droid::Graphics::IPaint;
using Elastos::Droid::Graphics::CPaint;
using Elastos::Droid::Graphics::CBitmapFactory;
using Elastos::Droid::Graphics::IColorFilter;
using Elastos::Droid::Graphics::IPaintFontMetricsInt;
using Elastos::Droid::Graphics::PorterDuffMode_SRC_ATOP;
using Elastos::Droid::Internal::Widget::CLockPatternUtils;
using Elastos::Droid::Internal::Widget::CSizeAdaptiveLayoutLayoutParams;
using Elastos::Droid::Internal::Widget::ILockPatternUtils;
using Elastos::Droid::Internal::Widget::ISizeAdaptiveLayoutLayoutParams;
using Elastos::Droid::Internal::Utility::CNotificationColorUtilHelper;
using Elastos::Droid::Internal::Utility::INotificationColorUtilHelper;
using Elastos::Droid::Os::Build;
using Elastos::Droid::Os::AsyncTask;
using Elastos::Droid::Os::UserHandle;
using Elastos::Droid::Os::CUserHandle;
using Elastos::Droid::Os::CHandler;
using Elastos::Droid::Os::EIID_IHandler;
using Elastos::Droid::Os::ServiceManager;
using Elastos::Droid::Provider::ISettings;
using Elastos::Droid::Provider::ISettingsGlobal;
using Elastos::Droid::Provider::ISettingsSecure;
using Elastos::Droid::R;
using Elastos::Droid::Service::Dreams::IDreamService;
using Elastos::Droid::SystemUI::Keyguard::EIID_IKeyguardHostViewOnDismissAction;
using Elastos::Droid::Text::TextUtils;
using Elastos::Droid::Utility::CDisplayMetrics;
using Elastos::Droid::Utility::CParcelableList;
using Elastos::Droid::Utility::CSparseArray;
using Elastos::Droid::Utility::CSparseBooleanArray;
using Elastos::Droid::View::Animation::CAnimationUtils;
using Elastos::Droid::View::Animation::IAnimationUtils;
using Elastos::Droid::View::Animation::IInterpolator;
using Elastos::Droid::View::CViewAnimationUtilsHelper;
using Elastos::Droid::View::CWindowManagerGlobal;
using Elastos::Droid::View::CWindowManagerGlobalHelper;
using Elastos::Droid::View::EIID_IViewOnClickListener;
using Elastos::Droid::View::EIID_IViewOnTouchListener;
using Elastos::Droid::View::EIID_IViewOnLongClickListener;
using Elastos::Droid::View::IDisplay;
using Elastos::Droid::View::ILayoutInflater;
using Elastos::Droid::View::IViewManager;
using Elastos::Droid::View::IViewParent;
using Elastos::Droid::View::IViewStub;
using Elastos::Droid::View::IViewAnimationUtilsHelper;
using Elastos::Droid::View::IWindowManagerGlobal;
using Elastos::Droid::View::IWindowManagerGlobalHelper;
using Elastos::Droid::View::LayoutInflater;
using Elastos::Droid::Widget::CPopupMenu;
using Elastos::Droid::Widget::CLinearLayout;
using Elastos::Droid::Widget::IDateTimeView;
using Elastos::Droid::Widget::ILinearLayout;
using Elastos::Droid::Widget::ITextView;
using Elastos::Droid::Widget::IRemoteViews;
using Elastos::Droid::Widget::ImageViewScaleType_CENTER_INSIDE;
using Elastos::Droid::Widget::EIID_IPopupMenuOnMenuItemClickListener;
using Elastos::Droid::Internal::StatusBar::CStatusBarIcon;
using Elastos::Droid::Internal::StatusBar::CStatusBarIconList;
using Elastos::Droid::Internal::StatusBar::EIID_IIStatusBar;
using Elastos::Droid::Internal::StatusBar::IIStatusBar;
using Elastos::Droid::Internal::StatusBar::IStatusBarIconList;
using Elastos::Droid::SystemUI::StatusBar::Phone::KeyguardTouchDelegate;
using Elastos::Droid::SystemUI::StatusBar::Policy::PreviewInflater;
using Elastos::Core::AutoLock;
using Elastos::Core::CBoolean;
using Elastos::Core::CString;
using Elastos::Core::IBoolean;
using Elastos::Core::StringUtils;
using Elastos::Core::CoreUtils;
using Elastos::Utility::CArrayList;
using Elastos::Utility::IIterator;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace SystemUI {
namespace StatusBar {

const String TAG("BaseStatusBar");

const Boolean BaseStatusBar::DEBUG = FALSE;//Logger::IsLoggable(TAG, Logger::___DEBUG);
const Boolean BaseStatusBar::MULTIUSER_DEBUG = FALSE;
const Int32 BaseStatusBar::MSG_SHOW_RECENT_APPS;
const Int32 BaseStatusBar::MSG_HIDE_RECENT_APPS;
const Int32 BaseStatusBar::MSG_TOGGLE_RECENTS_APPS;
const Int32 BaseStatusBar::MSG_PRELOAD_RECENT_APPS;
const Int32 BaseStatusBar::MSG_CANCEL_PRELOAD_RECENT_APPS;
const Int32 BaseStatusBar::MSG_SHOW_NEXT_AFFILIATED_TASK;
const Int32 BaseStatusBar::MSG_SHOW_PREV_AFFILIATED_TASK;
const Int32 BaseStatusBar::MSG_CLOSE_SEARCH_PANEL;
const Int32 BaseStatusBar::MSG_SHOW_HEADS_UP;
const Int32 BaseStatusBar::MSG_HIDE_HEADS_UP;
const Int32 BaseStatusBar::MSG_ESCALATE_HEADS_UP;
const Int32 BaseStatusBar::MSG_DECAY_HEADS_UP;
const Boolean BaseStatusBar::ENABLE_HEADS_UP = TRUE;
const Int32 BaseStatusBar::INTERRUPTION_THRESHOLD = 10;
const String BaseStatusBar::SETTING_HEADS_UP_TICKER("ticker_gets_heads_up");
const String BaseStatusBar::SYSTEM_DIALOG_REASON_RECENT_APPS("recentapps");
const Int32 BaseStatusBar::EXPANDED_LEAVE_ALONE = -10000;
const Int32 BaseStatusBar::EXPANDED_FULL_OPEN = -10001;
const Int32 BaseStatusBar::HIDDEN_NOTIFICATION_ID = 10000;
const String BaseStatusBar::BANNER_ACTION_CANCEL("com.android.systemui.statusbar.banner_action_cancel");
const String BaseStatusBar::BANNER_ACTION_SETUP("com.android.systemui.statusbar.banner_action_setup");

CAR_INTERFACE_IMPL(BaseStatusBar::ActivatableNotificationViewListener, Object, \
    IActivatableNotificationViewOnActivatedListener)

BaseStatusBar::ActivatableNotificationViewListener::ActivatableNotificationViewListener(
    /* [in] */ BaseStatusBar* host)
    : mHost(host)
{
}

// @Override
ECode BaseStatusBar::ActivatableNotificationViewListener::OnActivated(
    /* [in] */ IActivatableNotificationView* view)
{
    return mHost->OnActivated(view);
}

// @Override
ECode BaseStatusBar::ActivatableNotificationViewListener::OnActivationReset(
    /* [in] */ IActivatableNotificationView* view)
{
    return mHost->OnActivationReset(view);
}

//==============================================================================
//                  CSettingsObserver
//==============================================================================
CAR_OBJECT_IMPL(CSettingsObserver)

ECode CSettingsObserver::constructor(
    /* [in] */ IHandler* handler,
    /* [in] */ IBaseStatusBar* host)
{
    ContentObserver::constructor(handler);
    mHost = (BaseStatusBar*)host;
    return NOERROR;
}

ECode CSettingsObserver::OnChange(
    /* [in] */ Boolean selfChange)
{
    AutoPtr<IContentResolver> cr;
    mHost->mContext->GetContentResolver((IContentResolver**)&cr);
    Int32 value = 0;
    Elastos::Droid::Provider::Settings::Global::GetInt32(
            cr, ISettingsGlobal::DEVICE_PROVISIONED, 0, &value);
    Boolean provisioned = 0 != value;
    Logger::I(TAG, " TODO >> CSettingsObserver::OnChange: ISettingsGlobal::DEVICE_PROVISIONED: %d", provisioned);
    provisioned = TRUE; // TODO
    if (provisioned != mHost->mDeviceProvisioned) {
        mHost->mDeviceProvisioned = provisioned;
        mHost->UpdateNotifications();
    }
    Int32 mode = 0;
    Elastos::Droid::Provider::Settings::Global::GetInt32(cr,
        ISettingsGlobal::ZEN_MODE, ISettingsGlobal::ZEN_MODE_OFF, &mode);
    Logger::I(TAG, " >> Update ZEN_MODE: %d", mode);
    mHost->SetZenMode(mode);

    mHost->UpdateLockscreenNotificationSetting();
    return NOERROR;
}

//==============================================================================
//                  CLockscreenSettingsObserver
//==============================================================================
CAR_OBJECT_IMPL(CLockscreenSettingsObserver)

ECode CLockscreenSettingsObserver::constructor(
    /* [in] */ IHandler* handler,
    /* [in] */ IBaseStatusBar* host)
{
    ContentObserver::constructor(handler);
    mHost = (BaseStatusBar*)host;
    return NOERROR;
}

ECode CLockscreenSettingsObserver::OnChange(
    /* [in] */ Boolean selfChange)
{
    // We don't know which user changed LOCK_SCREEN_ALLOW_PRIVATE_NOTIFICATIONS,
    // so we just dump our cache ...
    mHost->mUsersAllowingPrivateNotifications->Clear();
    // ... and refresh all the notifications
    mHost->UpdateNotifications();
    return NOERROR;
}


//==============================================================================
// BaseStatusBar::NotificationRemoteViewsOnDismissAction
//==============================================================================

CAR_INTERFACE_IMPL(BaseStatusBar::NotificationRemoteViewsOnDismissAction, Object, \
    IKeyguardHostViewOnDismissAction)

BaseStatusBar::NotificationRemoteViewsOnDismissAction::NotificationRemoteViewsOnDismissAction(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ NotificationRemoteViewsOnClickHandler* clickHandler,
    /* [in] */ Boolean keyguardShowing,
    /* [in] */ Boolean afterKeyguardGone,
    /* [in] */ IView* view,
    /* [in] */ IPendingIntent* pendingIntent,
    /* [in] */ IIntent* fillInIntent)
    : mHost(host)
    , mClickHandler(clickHandler)
    , mKeyguardShowing(keyguardShowing)
    , mAfterKeyguardGone(afterKeyguardGone)
    , mView(view)
    , mPendingIntent(pendingIntent)
    , mFillInIntent(fillInIntent)
{
}

ECode BaseStatusBar::NotificationRemoteViewsOnDismissAction::OnDismiss(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    if (mKeyguardShowing && !mAfterKeyguardGone) {
        ActivityManagerNative::GetDefault()->KeyguardWaitingForActivityDrawn();
    }

    Boolean handled = mClickHandler->SuperOnClickHandler(mView, mPendingIntent, mFillInIntent);
    mHost->OverrideActivityPendingAppTransition(mKeyguardShowing && !mAfterKeyguardGone);

    // close the shade if it was open
    if (handled) {
        mHost->AnimateCollapsePanels(ICommandQueue::FLAG_EXCLUDE_NONE, TRUE /* force */);
        mHost->VisibilityChanged(FALSE);
    }
    // Wait for activity start.
    *result = handled;
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::NotificationRemoteViewsOnClickHandler
//==============================================================================
BaseStatusBar::NotificationRemoteViewsOnClickHandler::NotificationRemoteViewsOnClickHandler(
    /* [in] */ BaseStatusBar* host)
    : mHost(host)
{}

ECode BaseStatusBar::NotificationRemoteViewsOnClickHandler::OnClickHandler(
    /* [in] */ IView* view,
    /* [in] */ IPendingIntent* pendingIntent,
    /* [in] */ IIntent* fillInIntent,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    if (DEBUG) {
        Logger::V(TAG, "Notification click handler invoked for intent: %s, fillInIntent:%s",
            TO_CSTR(pendingIntent), TO_CSTR(fillInIntent));
    }

    // The intent we are sending is for the application, which
    // won't have permission to immediately start an activity after
    // the user switches to home.  We know it is safe to do at this
    // point, so make sure new activity switches are now allowed.
    ActivityManagerNative::GetDefault()->ResumeAppSwitches();
    Boolean isActivity = FALSE;
    pendingIntent->IsActivity(&isActivity);
    if (isActivity) {
        Boolean keyguardShowing = FALSE;
        mHost->mStatusBarKeyguardViewManager->IsShowing(&keyguardShowing);
        AutoPtr<IIntent> intent;
        pendingIntent->GetIntent((IIntent**)&intent);
        Boolean afterKeyguardGone = PreviewInflater::WouldLaunchResolverActivity(
            mHost->mContext, intent, mHost->mCurrentUserId);
        AutoPtr<NotificationRemoteViewsOnDismissAction> action;
        action = new NotificationRemoteViewsOnDismissAction(mHost, this,
            keyguardShowing, afterKeyguardGone, view, pendingIntent, fillInIntent);
        mHost->DismissKeyguardThenExecute(action, afterKeyguardGone);
        *result = TRUE;
        return NOERROR;
    }

    return RemoteViewsOnClickHandler::OnClickHandler(view, pendingIntent, fillInIntent, result);
}

Boolean BaseStatusBar::NotificationRemoteViewsOnClickHandler::SuperOnClickHandler(
    /* [in] */ IView* view,
    /* [in] */ IPendingIntent* pendingIntent,
    /* [in] */ IIntent* fillInIntent)
{
    Boolean result = FALSE;
    RemoteViews::RemoteViewsOnClickHandler::OnClickHandler(view, pendingIntent, fillInIntent, &result);
    return result;
}

//==============================================================================
//                  CBaseBroadcastReceiver
//==============================================================================
CAR_OBJECT_IMPL(CBaseBroadcastReceiver)

ECode CBaseBroadcastReceiver::constructor(
    /* [in] */ IBaseStatusBar* host)
{
    mHost = (BaseStatusBar*)host;
    return NOERROR;
}

ECode CBaseBroadcastReceiver::OnReceive(
    /* [in] */ IContext* context,
    /* [in] */ IIntent* intent)
{
    String action;
    intent->GetAction(&action);
    Logger::I("CBaseBroadcastReceiver", " >> OnReceive: %s", action.string());
    if (IIntent::ACTION_USER_SWITCHED.Equals(action)) {
        intent->GetInt32Extra(IIntent::EXTRA_USER_HANDLE, -1, &mHost->mCurrentUserId);
        mHost->UpdateCurrentProfilesCache();
        if (TRUE) Logger::V(TAG, "userId %d is in the house", mHost->mCurrentUserId);

        mHost->UpdateLockscreenNotificationSetting();
        mHost->UserSwitched(mHost->mCurrentUserId);
    }
    else if (IIntent::ACTION_USER_ADDED.Equals(action)) {
        mHost->UpdateCurrentProfilesCache();
    }
    else if (IDevicePolicyManager::ACTION_DEVICE_POLICY_MANAGER_STATE_CHANGED.Equals(
            action)) {
        mHost->mUsersAllowingPrivateNotifications->Clear();
        mHost->UpdateLockscreenNotificationSetting();
        mHost->UpdateNotifications();
    }
    else if (BaseStatusBar::BANNER_ACTION_CANCEL.Equals(action)
        || BaseStatusBar::BANNER_ACTION_SETUP.Equals(action)) {
        AutoPtr<IInterface> obj;
        mHost->mContext->GetSystemService(IContext::NOTIFICATION_SERVICE, (IInterface**)&obj);
        AutoPtr<INotificationManager> noMan = INotificationManager::Probe(obj);

        noMan->Cancel(BaseStatusBar::HIDDEN_NOTIFICATION_ID);

        AutoPtr<IContentResolver> cr;
        mHost->mContext->GetContentResolver((IContentResolver**)&cr);
        Boolean tmp = FALSE;
        Elastos::Droid::Provider::Settings::Secure::PutInt32(cr,
            ISettingsSecure::SHOW_NOTE_ABOUT_NOTIFICATION_HIDING, 0, &tmp);
        if (BaseStatusBar::BANNER_ACTION_SETUP.Equals(action)) {
            mHost->AnimateCollapsePanels(ICommandQueue::FLAG_EXCLUDE_NONE, TRUE /* force */);
            AutoPtr<IIntent> i;
            CIntent::New(ISettings::ACTION_APP_NOTIFICATION_REDACTION, (IIntent**)&i);
            i->AddFlags(IIntent::FLAG_ACTIVITY_NEW_TASK);
            mHost->mContext->StartActivity(i);
        }
    }
    return NOERROR;
}

//==============================================================================
//                  CNotificationListenerService::AddNotificationRunnable
//==============================================================================
CNotificationListenerService::AddNotificationRunnable::AddNotificationRunnable(
    /* [in] */ CNotificationListenerService* service,
    /* [in] */ ArrayOf<IStatusBarNotification*>* notifications,
    /* [in] */ INotificationListenerServiceRankingMap* currentRanking)
    : mService(service)
    , mNotifications(notifications)
    , mCurrentRanking(currentRanking)
{}

ECode CNotificationListenerService::AddNotificationRunnable::Run()
{
    Logger::I("CNotificationListenerService", "AddNotificationRunnable:%d", mNotifications->GetLength());
    for (Int32 i = 0; i < mNotifications->GetLength(); i++) {
        AutoPtr<IStatusBarNotification> sbn = (*mNotifications)[i];
        mService->mHost->AddNotification(sbn, mCurrentRanking);
    }
    return NOERROR;
}

//==============================================================================
//                  CNotificationListenerService::UpdateNotificationRunnable
//==============================================================================
CNotificationListenerService::UpdateNotificationRunnable::UpdateNotificationRunnable(
    /* [in] */ CNotificationListenerService* service,
    /* [in] */ IStatusBarNotification* sbn,
    /* [in] */ INotificationListenerServiceRankingMap* rankingMap)
    : mService(service)
    , mSbn(sbn)
    , mRankingMap(rankingMap)
{}

ECode CNotificationListenerService::UpdateNotificationRunnable::Run()
{
    Logger::I(TAG, "UpdateNotificationRunnable");
    AutoPtr<INotification> n;
    mSbn->GetNotification((INotification**)&n);
    String key;
    mSbn->GetKey(&key);
    AutoPtr<INotificationDataEntry> entry;
    mService->mHost->mNotificationData->Get(key, (INotificationDataEntry**)&entry);
    Boolean tmp = FALSE;
    Boolean isUpdate = entry.Get() != NULL || (mService->mHost->IsHeadsUp(key, &tmp), tmp);

    // Ignore children of notifications that have a summary, since we're not
    // going to show them anyway. This is TRUE also when the summary is canceled,
    // because children are automatically canceled by NoMan in that case.
    String gkey;
    Boolean tmp2 = FALSE;
    mSbn->GetGroupKey(&gkey);
    if ((n->IsGroupChild(&tmp), tmp) &&
            (mService->mHost->mNotificationData->IsGroupWithSummary(gkey, &tmp2), tmp2)) {
        if (mService->mHost->DEBUG) {
            Logger::D(TAG, "Ignoring group child due to existing summary: %p", mSbn.Get());
        }

        // Remove existing notification to avoid stale data.
        if (isUpdate) {
            mService->mHost->RemoveNotification(key, mRankingMap);
        }
        else {
            mService->mHost->mNotificationData->UpdateRanking(mRankingMap);
        }
        return NOERROR;
    }
    if (isUpdate) {
        mService->mHost->UpdateNotification(mSbn, mRankingMap);
    }
    else {
        mService->mHost->AddNotification(mSbn, mRankingMap);
    }
    return NOERROR;
}

//==============================================================================
//                  CNotificationListenerService::RemoveNotificationRunnable
//==============================================================================
CNotificationListenerService::RemoveNotificationRunnable::RemoveNotificationRunnable(
    /* [in] */ CNotificationListenerService* service,
    /* [in] */ IStatusBarNotification* sbn,
    /* [in] */ INotificationListenerServiceRankingMap* rankingMap)
    : mService(service)
    , mSbn(sbn)
    , mRankingMap(rankingMap)
{}

ECode CNotificationListenerService::RemoveNotificationRunnable::Run()
{
    Logger::I(TAG, "RemoveNotificationRunnable");
    String key;
    mSbn->GetKey(&key);
    mService->mHost->RemoveNotification(key, mRankingMap);
    return NOERROR;
}

//==============================================================================
//                  CNotificationListenerService::UpdateNotificationRankingRunnable
//==============================================================================
CNotificationListenerService::UpdateNotificationRankingRunnable::UpdateNotificationRankingRunnable(
    /* [in] */ CNotificationListenerService* service,
    /* [in] */ INotificationListenerServiceRankingMap* rankingMap)
    : mService(service)
    , mRankingMap(rankingMap)
{}

ECode CNotificationListenerService::UpdateNotificationRankingRunnable::Run()
{
    return mService->mHost->UpdateNotificationRanking(mRankingMap);
}

//==============================================================================
//                  CNotificationListenerService
//==============================================================================
CAR_OBJECT_IMPL(CNotificationListenerService)

ECode CNotificationListenerService::constructor()
{
    return NotificationListenerService::constructor();
}

ECode CNotificationListenerService::constructor(
    /* [in] */ IBaseStatusBar* host)
{
    mHost = (BaseStatusBar*)host;
    return NotificationListenerService::constructor();
}

ECode CNotificationListenerService::OnListenerConnected()
{
    AutoPtr<ArrayOf<IStatusBarNotification*> > notifications;
    GetActiveNotifications((ArrayOf<IStatusBarNotification*>**)&notifications);
    AutoPtr<INotificationListenerServiceRankingMap> currentRanking;
    GetCurrentRanking((INotificationListenerServiceRankingMap**)&currentRanking);
    AutoPtr<AddNotificationRunnable> run = new AddNotificationRunnable(this, notifications, currentRanking);
    Boolean tmp = FALSE;
    mHost->mHandler->Post(run, &tmp);
    return NOERROR;
}

ECode CNotificationListenerService::OnNotificationPosted(
    /* [in] */ IStatusBarNotification* sbn,
    /* [in] */ INotificationListenerServiceRankingMap* rankingMap)
{
    if (mHost->DEBUG) Logger::D(TAG, "OnNotificationPosted: %p", sbn);
    AutoPtr<UpdateNotificationRunnable> run = new UpdateNotificationRunnable(this, sbn, rankingMap);
    Boolean tmp = FALSE;
    mHost->mHandler->Post(run, &tmp);
    return NOERROR;
}

ECode CNotificationListenerService::OnNotificationRemoved(
    /* [in] */ IStatusBarNotification* sbn,
    /* [in] */ INotificationListenerServiceRankingMap* rankingMap)
{
    if (mHost->DEBUG) Logger::D(TAG, "OnNotificationRemoved: %p", sbn);
    AutoPtr<RemoveNotificationRunnable> run = new RemoveNotificationRunnable(this, sbn, rankingMap);
    Boolean tmp = FALSE;
    mHost->mHandler->Post(run, &tmp);
    return NOERROR;
}

ECode CNotificationListenerService::OnNotificationRankingUpdate(
    /* [in] */ INotificationListenerServiceRankingMap* rankingMap)
{
    if (mHost->DEBUG) Logger::D(TAG, "OnNotificationRankingUpdate");
    AutoPtr<UpdateNotificationRankingRunnable> run = new UpdateNotificationRankingRunnable(this, rankingMap);
    Boolean tmp = FALSE;
    mHost->mHandler->Post(run, &tmp);
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::RecentsPreloadOnTouchListener
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::RecentsPreloadOnTouchListener, Object, IViewOnTouchListener)

BaseStatusBar::RecentsPreloadOnTouchListener::RecentsPreloadOnTouchListener(
    /* [in] */ BaseStatusBar* host)
    : mHost(host)
{}

ECode BaseStatusBar::RecentsPreloadOnTouchListener::OnTouch(
    /* [in] */ IView* v,
    /* [in] */ IMotionEvent* event,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    Int32 a = 0;
    event->GetAction(&a);
    Int32 action = a & IMotionEvent::ACTION_MASK;
    if (action == IMotionEvent::ACTION_DOWN) {
        mHost->PreloadRecents();
    }
    else if (action == IMotionEvent::ACTION_CANCEL) {
        mHost->CancelPreloadingRecents();
    }
    else if (action == IMotionEvent::ACTION_UP) {
        Boolean tmp = FALSE;
        if (v->IsPressed(&tmp), !tmp) {
            mHost->CancelPreloadingRecents();
        }

    }
    *result = FALSE;
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::H
//==============================================================================
BaseStatusBar::H::H(
    /* [in] */ BaseStatusBar* host)
    : mHost(host)
{
    Handler::constructor();
}

ECode BaseStatusBar::H::HandleMessage(
    /* [in] */ IMessage* m)
{
    Int32 what = 0;
    m->GetWhat(&what);
    switch (what) {
        case MSG_SHOW_RECENT_APPS: {
            Int32 arg1 = 0;
            m->GetArg1(&arg1);
            mHost->ShowRecents(arg1 > 0);
            break;
        }
        case MSG_HIDE_RECENT_APPS: {
            Int32 arg1 = 0, arg2 = 0;
            m->GetArg1(&arg1);
            m->GetArg2(&arg2);
            mHost->HideRecents(arg1 > 0, arg2 > 0);
            break;
        }
        case MSG_TOGGLE_RECENTS_APPS:
            mHost->ToggleRecents();
            break;
        case MSG_PRELOAD_RECENT_APPS:
            mHost->PreloadRecents();
            break;
        case MSG_CANCEL_PRELOAD_RECENT_APPS:
            mHost->CancelPreloadingRecents();
            break;
        case MSG_SHOW_NEXT_AFFILIATED_TASK:
            mHost->ShowRecentsNextAffiliatedTask();
            break;
        case MSG_SHOW_PREV_AFFILIATED_TASK:
            mHost->ShowRecentsPreviousAffiliatedTask();
            break;
        case MSG_CLOSE_SEARCH_PANEL: {
            if (DEBUG) Logger::D(TAG, "closing search panel");
            Boolean tmp = FALSE;
            if (mHost->mSearchPanelView != NULL && (mHost->mSearchPanelView->IsShowing(&tmp), tmp)) {
                mHost->mSearchPanelView->Show(FALSE, TRUE);
            }
            break;
        }
    }
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::TouchOutsideListener
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::TouchOutsideListener, Object, IViewOnTouchListener)

BaseStatusBar::TouchOutsideListener::TouchOutsideListener(
    /* [in] */ Int32 msg,
    /* [in] */ IStatusBarPanel* panel,
    /* [in] */ BaseStatusBar* host)
    : mMsg(msg)
    , mPanel(panel)
    , mHost(host)
{}

ECode BaseStatusBar::TouchOutsideListener::OnTouch(
    /* [in] */ IView* v,
    /* [in] */ IMotionEvent* ev,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    Int32 action = 0;
    ev->GetAction(&action);
    Float x = 0, y = 0;
    ev->GetX(&x);
    ev->GetY(&y);
    Boolean tmp = FALSE;
    if (action == IMotionEvent::ACTION_OUTSIDE
        || (action == IMotionEvent::ACTION_DOWN
            && (mPanel->IsInContentArea((Int32)x, (Int32)y, &tmp), !tmp))) {
        mHost->mHandler->RemoveMessages(mMsg);
        mHost->mHandler->SendEmptyMessage(mMsg, &tmp);
        *result = TRUE;
        return NOERROR;
    }
    *result = FALSE;
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::NotificationGutsRunnable
//==============================================================================
BaseStatusBar::NotificationGutsRunnable::NotificationGutsRunnable(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ NotificationGutsOnDismissAction* action)
    : mHost(host)
    , mAction(action)
{}

ECode BaseStatusBar::NotificationGutsRunnable::Run()
{
    if (mAction->mKeyguardShowing) {
        ActivityManagerNative::GetDefault()->KeyguardWaitingForActivityDrawn();
    }

    AutoPtr<ITaskStackBuilderHelper> helper;
    CTaskStackBuilderHelper::AcquireSingleton((ITaskStackBuilderHelper**)&helper);
    AutoPtr<ITaskStackBuilder> builder;
    helper->Create(mHost->mContext, (ITaskStackBuilder**)&builder);
    builder->AddNextIntentWithParentStack(mAction->mIntent);
    AutoPtr<IUserHandle> user;
    CUserHandle::New(UserHandle::GetUserId(mAction->mAppUid), (IUserHandle**)&user);
    builder->StartActivities(NULL, user);
    return mHost->OverrideActivityPendingAppTransition(mAction->mKeyguardShowing);
}

//==============================================================================
// BaseStatusBar::NotificationGutsOnDismissAction
//==============================================================================

CAR_INTERFACE_IMPL(BaseStatusBar::NotificationGutsOnDismissAction, Object, \
    IKeyguardHostViewOnDismissAction)

BaseStatusBar::NotificationGutsOnDismissAction::NotificationGutsOnDismissAction(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ Boolean keyguardShowing,
    /* [in] */ IIntent* intent,
    /* [in] */ Int32 appUid)
    : mHost(host)
    , mKeyguardShowing(keyguardShowing)
    , mIntent(intent)
    , mAppUid(appUid)
{
}

ECode BaseStatusBar::NotificationGutsOnDismissAction::OnDismiss(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    AutoPtr<Runnable> runnable = new NotificationGutsRunnable(mHost, this);
    AsyncTask::Execute(runnable);
    mHost->AnimateCollapsePanels(ICommandQueue::FLAG_EXCLUDE_NONE, TRUE /* force */);
    *result = TRUE;
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::KeyguardHostViewOnDismissActionThread
//==============================================================================

BaseStatusBar::KeyguardHostViewOnDismissActionThread::KeyguardHostViewOnDismissActionThread(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ KeyguardHostViewOnDismissAction* action)
    : mHost(host)
    , mAction(action)
{
    mAction->AddRef();              // holder ref-count
    mAction->mClicker->AddRef();    // holder ref-count
}

ECode BaseStatusBar::KeyguardHostViewOnDismissActionThread::Run()
{
    AutoPtr<IIActivityManager> am = ActivityManagerNative::GetDefault();
    if (mAction->mKeyguardShowing && !mAction->mAfterKeyguardGone) {
        am->KeyguardWaitingForActivityDrawn();
    }

    // The intent we are sending is for the application, which
    // won't have permission to immediately start an activity after
    // the user switches to home.  We know it is safe to do at this
    // point, so make sure new activity switches are now allowed.
    am->ResumeAppSwitches();

    if (mAction->mClicker->mIntent != NULL) {
        ECode ec = mAction->mClicker->mIntent->Send();
        if (FAILED(ec)) {
            // the stack trace isn't very helpful here.
            // Just log the exception message.
            Logger::W(TAG, "Sending contentIntent failed: ec=%08x", ec);
            // TODO: Dismiss Keyguard.
        }

        Boolean isActivity;
        mAction->mClicker->mIntent->IsActivity(&isActivity);
        if (isActivity) {
            mHost->OverrideActivityPendingAppTransition(
                mAction->mKeyguardShowing && !mAction->mAfterKeyguardGone);
        }
    }

    mHost->mBarService->OnNotificationClick(mAction->mClicker->mNotificationKey);

    REFCOUNT_RELEASE(mAction);              // release ref-count
    REFCOUNT_RELEASE(mAction->mClicker);    // release ref-count
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::KeyguardHostViewOnDismissAction
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::KeyguardHostViewOnDismissAction, Object, IKeyguardHostViewOnDismissAction)

BaseStatusBar::KeyguardHostViewOnDismissAction::KeyguardHostViewOnDismissAction(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ NotificationClicker* clicker,
    /* [in] */ Boolean keyguardShowing,
    /* [in] */ Boolean afterKeyguardGone)
    : mHost(host)
    , mClicker(clicker)
    , mKeyguardShowing(keyguardShowing)
    , mAfterKeyguardGone(afterKeyguardGone)
{
}

/**
 * @return true if the dismiss should be deferred
 */
ECode BaseStatusBar::KeyguardHostViewOnDismissAction::OnDismiss(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    if (mClicker->mIsHeadsUp) {
        mHost->mHeadsUpNotificationView->Clear();
    }

    AutoPtr<Thread> thread = new KeyguardHostViewOnDismissActionThread(mHost, this);
    thread->constructor();
    thread->Start();

    // close the shade if it was open
    mHost->AnimateCollapsePanels(ICommandQueue::FLAG_EXCLUDE_NONE, TRUE /* force */);
    mHost->VisibilityChanged(FALSE);

    if (mClicker->mIntent != NULL) {
        mClicker->mIntent->IsActivity(result);
    }
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::NotificationClicker
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::NotificationClicker, Object, IViewOnClickListener, INotificationClicker)

BaseStatusBar::NotificationClicker::NotificationClicker(
    /* [in] */ IPendingIntent* intent,
    /* [in] */ const String& notificationKey,
    /* [in] */ Boolean forHun,
    /* [in] */ BaseStatusBar* host)
{
    mIntent = intent;
    mNotificationKey = notificationKey;
    mIsHeadsUp = forHun;
    mHost = host;
}

ECode BaseStatusBar::NotificationClicker::OnClick(
    /* [in] */ IView* v)
{
    Boolean keyguardShowing = FALSE;
    mHost->mStatusBarKeyguardViewManager->IsShowing(&keyguardShowing);
    Boolean afterKeyguardGone;
    mIntent->IsActivity(&afterKeyguardGone);
    if (afterKeyguardGone) {
        AutoPtr<IIntent> intent;
        mIntent->GetIntent((IIntent**)&intent);
        afterKeyguardGone = PreviewInflater::WouldLaunchResolverActivity(
            mHost->mContext, intent, mHost->mCurrentUserId);
    }

    AutoPtr<KeyguardHostViewOnDismissAction> action;
    action = new KeyguardHostViewOnDismissAction(mHost, this, keyguardShowing, afterKeyguardGone);
    mHost->DismissKeyguardThenExecute(action, afterKeyguardGone);
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::RecentsComponentCallbacks
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::RecentsComponentCallbacks, Object, IRecentsComponentCallbacks)

BaseStatusBar::RecentsComponentCallbacks::RecentsComponentCallbacks(
    /* [in] */ BaseStatusBar* host)
    : mHost(host)
{}

ECode BaseStatusBar::RecentsComponentCallbacks::OnVisibilityChanged(
    /* [in] */ Boolean visible)
{
    return mHost->OnVisibilityChanged(visible);
}

//==============================================================================
// BaseStatusBar::ViewOnClickListener1
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::ViewOnClickListener1, Object, IViewOnClickListener)

BaseStatusBar::ViewOnClickListener1::ViewOnClickListener1(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ const String& pkg,
    /* [in] */ const String& tag,
    /* [in] */ Int32 id,
    /* [in] */ Int32 userid)
    : mHost(host)
    , mPkg(pkg)
    , mTag(tag)
    , mId(id)
    , mUserId(userid)
{}

ECode BaseStatusBar::ViewOnClickListener1::OnClick(
    /* [in] */ IView* v)
{
    // Accessibility feedback
    String s;
    mHost->mContext->GetString(R::string::accessibility_notification_dismissed, &s);
    AutoPtr<ICharSequence> value;
    CString::New(s, (ICharSequence**)&value);
    v->AnnounceForAccessibility(value);
    // try {
    mHost->mBarService->OnNotificationClear(mPkg, mTag, mId, mUserId);
    // } catch (RemoteException ex) {
    //     // system process is dead if we're here.
    // }
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::ViewOnClickListener2
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::ViewOnClickListener2, Object, IViewOnClickListener)

BaseStatusBar::ViewOnClickListener2::ViewOnClickListener2(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ const String& pkg,
    /* [in] */ Int32 appUidF)
    : mHost(host)
    , mPkg(pkg)
    , mAppUidF(appUidF)
{}

ECode BaseStatusBar::ViewOnClickListener2::OnClick(
    /* [in] */ IView* v)
{
    mHost->StartAppNotificationSettingsActivity(mPkg, mAppUidF);
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::ViewOnClickListener3
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::ViewOnClickListener3, Object, IViewOnClickListener)

BaseStatusBar::ViewOnClickListener3::ViewOnClickListener3(
    /* [in] */ BaseStatusBar* host,
    /* [in] */ IIntent* appSettingsLaunchIntent,
    /* [in] */ IStatusBarNotification* sbn,
    /* [in] */ Int32 appUidF)
    : mHost(host)
    , mIntent(appSettingsLaunchIntent)
    , mSbn(sbn)
    , mAppUidF(appUidF)
{}

ECode BaseStatusBar::ViewOnClickListener3::OnClick(
    /* [in] */ IView* v)
{
    Int32 id = 0;
    mSbn->GetId(&id);
    String tag;
    mSbn->GetTag(&tag);
    mHost->StartAppOwnNotificationSettingsActivity(mIntent, id, tag, mAppUidF);
    return NOERROR;
}

//==============================================================================
// BaseStatusBar::BaseLongPressListener
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar::BaseLongPressListener, Object, ISwipeHelperLongPressListener)

BaseStatusBar::BaseLongPressListener::BaseLongPressListener(
    /* [in] */ BaseStatusBar* host)
    : mHost(host)
{}

ECode BaseStatusBar::BaseLongPressListener::OnLongPress(
    /* [in] */ IView* v,
    /* [in] */ Int32 x,
    /* [in] */ Int32 y,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    mHost->DismissPopups();

    if (IExpandableNotificationRow::Probe(v) == NULL) {
        *result = FALSE;
        return NOERROR;
    }

    AutoPtr<IBinder> token;
    v->GetWindowToken((IBinder**)&token);
    if (token.Get() == NULL) {
        Logger::E(TAG, "Trying to show notification guts, but not attached to window");
        *result = FALSE;
        return NOERROR;
    }

    mHost->InflateGuts(IExpandableNotificationRow::Probe(v));

    // Assume we are a status_bar_notification_row
    AutoPtr<IView> view;
    v->FindViewById(R::id::notification_guts, (IView**)&view);
    AutoPtr<INotificationGuts> guts = INotificationGuts::Probe(view);
    if (guts == NULL) {
        // This view has no guts. Examples are the more card or the dismiss all view
        *result = FALSE;
        return NOERROR;
    }

    // Already showing?
    Int32 visible = 0;
    if ((IView::Probe(guts)->GetVisibility(&visible), visible) == IView::VISIBLE) {
        Logger::E(TAG, "Trying to show notification guts, but already visible");
        *result = FALSE;
        return NOERROR;
    }

    IView::Probe(guts)->SetVisibility(IView::VISIBLE);
    Int32 value = 0;
    IView::Probe(guts)->GetWidth(&value);
    Double horz = Elastos::Core::Math::Max(value - x, x);
    guts->GetActualHeight(&value);
    Double vert = Elastos::Core::Math::Max(value - y, y);
    Float r = (Float) Elastos::Core::Math::Hypot(horz, vert);

    AutoPtr<IViewAnimationUtilsHelper> helper;
    CViewAnimationUtilsHelper::AcquireSingleton((IViewAnimationUtilsHelper**)&helper);
    AutoPtr<IAnimator> a;
    helper->CreateCircularReveal(IView::Probe(guts), x, y, 0, r, (IAnimator**)&a);
    a->SetDuration(400);
    a->SetInterpolator(mHost->mLinearOutSlowIn);
    a->Start();

    mHost->mNotificationGutsExposed = guts;

    *result = TRUE;
    return NOERROR;
}


//==============================================================================
// BaseStatusBar::BaseAnimatorListenerAdapter
//==============================================================================
BaseStatusBar::BaseAnimatorListenerAdapter::BaseAnimatorListenerAdapter(
    /* [in] */ IView* v)
    : mView(v)
{}

ECode BaseStatusBar::BaseAnimatorListenerAdapter::OnAnimationEnd(
    /* [in] */ IAnimator* animation)
{
    AnimatorListenerAdapter::OnAnimationEnd(animation);
    mView->SetVisibility(IView::GONE);
    return NOERROR;
}

//==============================================================================
//                  BaseStatusBar
//==============================================================================
CAR_INTERFACE_IMPL(BaseStatusBar, SystemUI, IBaseStatusBar, ICommandQueueCallbacks, \
    IRecentsComponentCallbacks, IExpansionLogger, INotificationEnvironment);

BaseStatusBar::BaseStatusBar()
    : mHeadsUpNotificationDecay(0)
    , mPanelSlightlyVisible(FALSE)
    , mCurrentUserId(0)
    , mLayoutDirection(-1)
    , mUseHeadsUp(FALSE)
    , mHeadsUpTicker(FALSE)
    , mDisableNotificationAlerts(FALSE)
    , mRowMinHeight(0)
    , mRowMaxHeight(0)
    , mZenMode(0)
    , mState(IStatusBarState::SHADE)
    , mBouncerShowing(FALSE)
    , mShowLockscreenNotifications(FALSE)
    , mLockscreenPublicMode(FALSE)
    , mFontScale(0)
    , mDeviceProvisioned(FALSE)
{
}

ECode BaseStatusBar::constructor()
{
    mOnClickHandler = new NotificationRemoteViewsOnClickHandler(this);
    CBaseBroadcastReceiver::New(this, (IBroadcastReceiver**)&mBroadcastReceiver);
    CNotificationListenerService::New(this, (INotificationListenerService**)&mNotificationListener);
    mRecentsPreloadOnTouchListener = new RecentsPreloadOnTouchListener(this);
    mHandler = CreateHandler();

    CSettingsObserver::New(mHandler.Get(), this, (IContentObserver**)&mSettingsObserver);
    CLockscreenSettingsObserver::New(mHandler.Get(), this, (IContentObserver**)&mLockscreenSettingsObserver);

    CSparseArray::New((ISparseArray**)&mCurrentProfiles);
    CSparseBooleanArray::New((ISparseBooleanArray**)&mUsersAllowingPrivateNotifications);
    return NOERROR;
}

ECode BaseStatusBar::IsDeviceProvisioned(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mDeviceProvisioned;
    return NOERROR;
}

void BaseStatusBar::UpdateCurrentProfilesCache()
{
    AutoLock syncLock(mCurrentProfiles);
    mCurrentProfiles->Clear();
    if (mUserManager != NULL) {
        AutoPtr<IList> lists;
        mUserManager->GetProfiles(mCurrentUserId, (IList**)&lists);
        if (lists != NULL) {
            AutoPtr<IIterator> ator;
            lists->GetIterator((IIterator**)&ator);
            Boolean has = FALSE;
            while (ator->HasNext(&has), has) {
                AutoPtr<IInterface> obj;
                ator->GetNext((IInterface**)&obj);
                AutoPtr<IUserInfo> user = IUserInfo::Probe(obj);
                Int32 id = 0;
                user->GetId(&id);
                mCurrentProfiles->Put(id, user);
            }
        }
    }
}

ECode BaseStatusBar::Start()
{
    AutoPtr<IInterface> obj;
    mContext->GetSystemService(IContext::WINDOW_SERVICE, (IInterface**)&obj);
    mWindowManager = IWindowManager::Probe(obj);

    AutoPtr<IWindowManagerGlobalHelper> helper;
    CWindowManagerGlobalHelper::AcquireSingleton((IWindowManagerGlobalHelper**)&helper);
    helper->GetWindowManagerService((IIWindowManager**)&mWindowManagerService);
    mWindowManager->GetDefaultDisplay((IDisplay**)&mDisplay);

    obj = NULL;
    mContext->GetSystemService(IContext::DEVICE_POLICY_SERVICE, (IInterface**)&obj);
    mDevicePolicyManager = IDevicePolicyManager::Probe(obj);

    AutoPtr<INotificationColorUtilHelper> ncuHelper;
    CNotificationColorUtilHelper::AcquireSingleton((INotificationColorUtilHelper**)&ncuHelper);
    ncuHelper->GetInstance(mContext, (INotificationColorUtil**)&mNotificationColorUtil);

    mNotificationData = new NotificationData(this);

    obj = NULL;
    mContext->GetSystemService(IContext::ACCESSIBILITY_SERVICE, (IInterface**)&obj);
    mAccessibilityManager = IAccessibilityManager::Probe(obj);

    obj = ServiceManager::GetService(IDreamService::DREAM_SERVICE);
    mDreamManager = IIDreamManager::Probe(obj);

    obj = NULL;
    mContext->GetSystemService(IContext::POWER_SERVICE, (IInterface**)&obj);
    mPowerManager = IPowerManager::Probe(obj);

    mSettingsObserver->OnChange(FALSE); // set up

    AutoPtr<IContentResolver> cr;
    mContext->GetContentResolver((IContentResolver**)&cr);
    AutoPtr<IUri> uri;
    Elastos::Droid::Provider::Settings::Global::GetUriFor(ISettingsGlobal::DEVICE_PROVISIONED, (IUri**)&uri);
    cr->RegisterContentObserver(uri, TRUE, mSettingsObserver);

    uri = NULL;
    Elastos::Droid::Provider::Settings::Global::GetUriFor(ISettingsGlobal::ZEN_MODE, (IUri**)&uri);
    cr->RegisterContentObserver(uri, FALSE, mSettingsObserver);

    uri = NULL;
    Elastos::Droid::Provider::Settings::Secure::GetUriFor(ISettingsSecure::LOCK_SCREEN_SHOW_NOTIFICATIONS, (IUri**)&uri);
    cr->RegisterContentObserver(uri, FALSE, mSettingsObserver, IUserHandle::USER_ALL);

    uri = NULL;
    Elastos::Droid::Provider::Settings::Secure::GetUriFor(ISettingsSecure::LOCK_SCREEN_ALLOW_PRIVATE_NOTIFICATIONS, (IUri**)&uri);
    cr->RegisterContentObserver(uri, TRUE, mLockscreenSettingsObserver, IUserHandle::USER_ALL);

    obj = ServiceManager::GetService(IContext::STATUS_BAR_SERVICE);
    mBarService = IIStatusBarService::Probe(obj);

    obj = GetComponent(String("EIID_IRecentsComponent"));
    mRecents = IRecentsComponent::Probe(obj);

    AutoPtr<RecentsComponentCallbacks> c = new RecentsComponentCallbacks(this);
    mRecents->SetCallback(c);

    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    AutoPtr<IConfiguration> currentConfig;
    res->GetConfiguration((IConfiguration**)&currentConfig);
    currentConfig->GetLocale((ILocale**)&mLocale);
    mLayoutDirection = TextUtils::GetLayoutDirectionFromLocale(mLocale);
    currentConfig->GetFontScale(&mFontScale);

    obj = NULL;
    mContext->GetSystemService(IContext::USER_SERVICE, (IInterface**)&obj);
    mUserManager = IUserManager::Probe(obj);

    AutoPtr<IAnimationUtils> au;
    CAnimationUtils::AcquireSingleton((IAnimationUtils**)&au);
    AutoPtr<IInterpolator> o;
    au->LoadInterpolator(mContext,
            Elastos::Droid::R::interpolator::linear_out_slow_in, (IInterpolator**)&o);
    mLinearOutSlowIn = ITimeInterpolator::Probe(o);
    o = NULL;
    au->LoadInterpolator(mContext,
            Elastos::Droid::R::interpolator::fast_out_linear_in, (IInterpolator**)&o);
    mFastOutLinearIn = ITimeInterpolator::Probe(o);

    // Connect in to the status bar manager service
    AutoPtr<IStatusBarIconList> iconList;
    CStatusBarIconList::New((IStatusBarIconList**)&iconList);

    CCommandQueue::New(this, iconList, (ICommandQueue**)&mCommandQueue);

    AutoPtr<ArrayOf<Int32> > switches = ArrayOf<Int32>::Alloc(8);
    AutoPtr<IList> binders;
    CParcelableList::New((IList**)&binders);
    // try {
    if (mBarService != NULL) {
        assert(iconList != NULL);
        AutoPtr<IStatusBarIconList> outIconList;
        AutoPtr<ArrayOf<Int32> > outSwitches;
        AutoPtr<IList> outBinders;
        mBarService->RegisterStatusBar(IIStatusBar::Probe(mCommandQueue),
            iconList, switches, binders, (IStatusBarIconList**)&outIconList,
            (ArrayOf<Int32>**)&outSwitches, (IList**)&outBinders);
        iconList = outIconList;
        switches = outSwitches;
        binders = outBinders;
    }
    // } catch (RemoteException ex) {
    //     // If the system process isn't there we're doomed anyway.
    // }

    CreateAndAddWindows();

    Disable((*switches)[0], FALSE /* animate */);
    SetSystemUiVisibility((*switches)[1], 0xffffffff);
    TopAppWindowChanged((*switches)[2] != 0);
    // StatusBarManagerService has a back up of IME token and it's restored here.
    obj = NULL;
    binders->Get(0, (IInterface**)&obj);
    AutoPtr<IBinder> binder = IBinder::Probe(obj);
    SetImeWindowStatus(binder, (*switches)[3], (*switches)[4], (*switches)[5] != 0);

    // Set up the initial icon state
    Int32 N = 0;
    iconList->Size(&N);
    Int32 viewIndex = 0;
    for (Int32 i = 0; i < N; i++) {
        AutoPtr<IStatusBarIcon> icon;
        iconList->GetIcon(i, (IStatusBarIcon**)&icon);
        if (icon != NULL) {
            String slot;
            iconList->GetSlot(i, &slot);
            AddIcon(slot, i, viewIndex, icon);
            viewIndex++;
        }
    }

    // Set up the initial notification state.
    String pn;
    mContext->GetPackageName(&pn);
    String name;
    name = Object::GetFullClassName((IBaseStatusBar*)this);
    AutoPtr<IComponentName> cn;
    CComponentName::New(pn, name, (IComponentName**)&cn);
    if (FAILED(mNotificationListener->RegisterAsSystemService(mContext, cn, IUserHandle::USER_ALL))) {
        Logger::E(TAG, "Unable to register notification listener");
    }

    if (DEBUG) {
        Logger::D(TAG, "init: icons=%d disabled=0x%08x lights=0x%08x menu=0x%08x imeButton=0x%08x",
            N, (*switches)[0], (*switches)[1], (*switches)[2], (*switches)[3]);
    }

    AutoPtr<IActivityManagerHelper> amHelper;
    CActivityManagerHelper::AcquireSingleton((IActivityManagerHelper**)&amHelper);
    amHelper->GetCurrentUser(&mCurrentUserId);

    AutoPtr<IIntentFilter> filter;
    CIntentFilter::New((IIntentFilter**)&filter);
    filter->AddAction(IIntent::ACTION_USER_SWITCHED);
    filter->AddAction(IIntent::ACTION_USER_ADDED);
    filter->AddAction(BANNER_ACTION_CANCEL);
    filter->AddAction(BANNER_ACTION_SETUP);
    filter->AddAction(IDevicePolicyManager::ACTION_DEVICE_POLICY_MANAGER_STATE_CHANGED);
    AutoPtr<IIntent> intent;
    mContext->RegisterReceiver(mBroadcastReceiver, filter, (IIntent**)&intent);
    UpdateCurrentProfilesCache();

    // TODO delete. luo.zhaohui
    UpdateLockscreenNotificationSetting();
    UserSwitched(0);
    return NOERROR;
}

void BaseStatusBar::NotifyUserAboutHiddenNotifications()
{
    AutoPtr<IContentResolver> cr;
    mContext->GetContentResolver((IContentResolver**)&cr);
    Int32 value = 0;
    Elastos::Droid::Provider::Settings::Secure::GetInt32(cr,
        ISettingsSecure::SHOW_NOTE_ABOUT_NOTIFICATION_HIDING, 1, &value);
    if (0 != value) {
        Logger::D(TAG, "user hasn't seen notification about hidden notifications");
        AutoPtr<ILockPatternUtils> lockPatternUtils;
        CLockPatternUtils::New(mContext, (ILockPatternUtils**)&lockPatternUtils);
        Boolean tmp = FALSE;
        if (lockPatternUtils->IsSecure(&tmp), !tmp) {
            Logger::D(TAG, "insecure lockscreen, skipping notification");
            Elastos::Droid::Provider::Settings::Secure::PutInt32(cr,
                    ISettingsSecure::SHOW_NOTE_ABOUT_NOTIFICATION_HIDING, 0, &tmp);
            return;
        }
        Logger::D(TAG, "disabling lockecreen notifications and alerting the user");
        // disable lockscreen notifications until user acts on the banner.
        Elastos::Droid::Provider::Settings::Secure::PutInt32(cr,
                ISettingsSecure::LOCK_SCREEN_SHOW_NOTIFICATIONS, 0, &tmp);
        Elastos::Droid::Provider::Settings::Secure::PutInt32(cr,
                ISettingsSecure::LOCK_SCREEN_ALLOW_PRIVATE_NOTIFICATIONS, 0, &tmp);

        String packageName;
        mContext->GetPackageName(&packageName);
        AutoPtr<IPendingIntentHelper> piHelper;
        CPendingIntentHelper::AcquireSingleton((IPendingIntentHelper**)&piHelper);
        AutoPtr<IPendingIntent> cancelIntent;
        AutoPtr<IIntent> intent;
        CIntent::New(BANNER_ACTION_CANCEL, (IIntent**)&intent);
        intent->SetPackage(packageName);
        piHelper->GetBroadcast(mContext, 0, intent,
                IPendingIntent::FLAG_CANCEL_CURRENT, (IPendingIntent**)&cancelIntent);

        AutoPtr<IPendingIntent> setupIntent;
        intent = NULL;
        CIntent::New(BANNER_ACTION_SETUP, (IIntent**)&intent);
        intent->SetPackage(packageName);
        piHelper->GetBroadcast(mContext, 0, intent,
                IPendingIntent::FLAG_CANCEL_CURRENT, (IPendingIntent**)&setupIntent);

        AutoPtr<IResources> res;
        mContext->GetResources((IResources**)&res);
        const Int32 colorRes = Elastos::Droid::R::color::system_notification_accent_color;
        AutoPtr<INotificationBuilder> note;
        CNotificationBuilder::New(mContext, (INotificationBuilder**)&note);
        note->SetSmallIcon(R::drawable::ic_android);

        String v;
        mContext->GetString(R::string::hidden_notifications_title, &v);
        AutoPtr<ICharSequence> str;
        CString::New(v, (ICharSequence**)&str);
        note->SetContentTitle(str);

        str = NULL;
        mContext->GetString(R::string::hidden_notifications_text, &v);
        CString::New(v, (ICharSequence**)&str);
        note->SetContentText(str);
        note->SetPriority(INotification::PRIORITY_HIGH);
        note->SetOngoing(TRUE);

        Int32 color = 0;
        res->GetColor(colorRes, &color);
        note->SetColor(color);
        note->SetContentIntent(setupIntent);

        str = NULL;
        mContext->GetString(R::string::hidden_notifications_cancel, &v);
        CString::New(v, (ICharSequence**)&str);
        note->AddAction(R::drawable::ic_close, str, cancelIntent);

        str = NULL;
        mContext->GetString(R::string::hidden_notifications_setup, &v);
        CString::New(v, (ICharSequence**)&str);
        note->AddAction(R::drawable::ic_settings, str, setupIntent);

        AutoPtr<IInterface> obj;
        mContext->GetSystemService(IContext::NOTIFICATION_SERVICE, (IInterface**)&obj);
        AutoPtr<INotificationManager> noMan = INotificationManager::Probe(obj);

        AutoPtr<INotification> n;
        note->Build((INotification**)&n);
        noMan->Notify(HIDDEN_NOTIFICATION_ID, n);
    }
}

ECode BaseStatusBar::UserSwitched(
    /* [in] */ Int32 newUserId)
{
    // should be overridden
    return NOERROR;
}

ECode BaseStatusBar::IsHeadsUp(
    /* [in] */ const String& key,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    Boolean tmp = FALSE;
    *result = mHeadsUpNotificationView != NULL && (mHeadsUpNotificationView->IsShowing(key, &tmp), tmp);
    return NOERROR;
}

ECode BaseStatusBar::IsNotificationForCurrentProfiles(
    /* [in] */ IStatusBarNotification* n,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    const Int32 thisUserId = mCurrentUserId;
    Int32 notificationUserId = 0;
    n->GetUserId(&notificationUserId);
    if (DEBUG && MULTIUSER_DEBUG) {
        Logger::V(TAG, "%s: current userid: %d, notification userid: %d",
            n, thisUserId, notificationUserId);
    }
    {
        AutoLock syncLock(mCurrentProfiles);
        AutoPtr<IInterface> obj;
        mCurrentProfiles->Get(notificationUserId, (IInterface**)&obj);
        *result = notificationUserId == IUserHandle::USER_ALL
                || obj.Get() != NULL;
        return NOERROR;
    }
    assert(0);
    return NOERROR;
}

ECode BaseStatusBar::GetCurrentMediaNotificationKey(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str);
    *str = NULL;
    return NOERROR;
}

void BaseStatusBar::DismissKeyguardThenExecute(
    /* [in] */ IKeyguardHostViewOnDismissAction* action,
    /* [in] */ Boolean afterKeyguardGone)
{
    Boolean bval;
    action->OnDismiss(&bval);
}

ECode BaseStatusBar::OnConfigurationChanged(
    /* [in] */ IConfiguration* newConfig)
{
    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    AutoPtr<IConfiguration> config;
    res->GetConfiguration((IConfiguration**)&config);
    AutoPtr<ILocale> locale;
    config->GetLocale((ILocale**)&locale);
    const Int32 ld = TextUtils::GetLayoutDirectionFromLocale(locale);
    Float fontScale = 0;
    newConfig->GetFontScale(&fontScale);

    Boolean tmp = FALSE;
    if ((locale->Equals(mLocale, &tmp), !tmp) || ld != mLayoutDirection || fontScale != mFontScale) {
        if (DEBUG) {
            Logger::V(TAG,  "config changed locale/LD: %s (%d) -> %s (%d)",
                TO_CSTR(mLocale), mLayoutDirection, TO_CSTR(locale), ld);
        }
        mLocale = locale;
        mLayoutDirection = ld;
        RefreshLayout(ld);
    }
    return NOERROR;
}

AutoPtr<IView> BaseStatusBar::UpdateNotificationVetoButton(
    /* [in] */ IView* row,
    /* [in] */ IStatusBarNotification* n)
{
    AutoPtr<IView> vetoButton;
    row->FindViewById(R::id::veto, (IView**)&vetoButton);
    Boolean tmp = FALSE;
    AutoPtr<INotificationDataEntry> entry;
    AutoPtr<IExpandableNotificationRow> r;
    if ((n->IsClearable(&tmp), tmp)
        || ((mHeadsUpNotificationView->GetEntry((INotificationDataEntry**)&entry), entry.Get()) != NULL
            && (entry->GetRow((IExpandableNotificationRow**)&r), IView::Probe(r)) == row)) {
        String _pkg;
        n->GetPackageName(&_pkg);
        String _tag;
        n->GetTag(&_tag);
        Int32 _id;
        n->GetId(&_id);
        Int32 _userId;
        n->GetUserId(&_userId);
        AutoPtr<ViewOnClickListener1> listener = new ViewOnClickListener1(this, _pkg, _tag, _id, _userId);
        vetoButton->SetOnClickListener(listener);
        vetoButton->SetVisibility(IView::VISIBLE);
    }
    else {
        vetoButton->SetVisibility(IView::GONE);
    }
    vetoButton->SetImportantForAccessibility(IView::IMPORTANT_FOR_ACCESSIBILITY_NO);
    return vetoButton;
}


void BaseStatusBar::ApplyColorsAndBackgrounds(
    /* [in] */ IStatusBarNotification* sbn,
    /* [in] */ INotificationDataEntry* entry)
{
    AutoPtr<IView> expanded;
    entry->GetExpanded((IView**)&expanded);
    Int32 id = 0, targetSdk = 0;
    expanded->GetId(&id);
    entry->GetTargetSdk(&targetSdk);
    if (id != Elastos::Droid::R::id::status_bar_latest_event_content) {
        // Using custom RemoteViews
        if (targetSdk >= Build::VERSION_CODES::GINGERBREAD
                && targetSdk < Build::VERSION_CODES::LOLLIPOP) {
            AutoPtr<IExpandableNotificationRow> row;
            entry->GetRow((IExpandableNotificationRow**)&row);
            IActivatableNotificationView::Probe(row)->SetShowingLegacyBackground(TRUE);
            entry->SetLegacy(TRUE);
        }
    }
    else {
        // Using platform templates
        AutoPtr<INotification> n;
        sbn->GetNotification((INotification**)&n);
        Int32 color = 0;
        n->GetColor(&color);
        Boolean tmp = FALSE;
        if (IsMediaNotification(entry, &tmp), tmp) {
            AutoPtr<IExpandableNotificationRow> row;
            entry->GetRow((IExpandableNotificationRow**)&row);

            AutoPtr<IResources> res;
            mContext->GetResources((IResources**)&res);
            Int32 c = 0;
            res->GetColor(R::color::notification_material_background_media_default_color, &c);
            IActivatableNotificationView::Probe(row)->SetTintColor(color == INotification::COLOR_DEFAULT ? c : color);
        }
    }

    AutoPtr<IStatusBarIconView> icon;
    entry->GetIcon((IStatusBarIconView**)&icon);
    if (icon.Get() != NULL) {
        if (targetSdk >= Build::VERSION_CODES::LOLLIPOP) {
            AutoPtr<IResources> res;
            mContext->GetResources((IResources**)&res);
            Int32 c = 0;
            res->GetColor(Elastos::Droid::R::color::white, &c);
            IImageView::Probe(icon)->SetColorFilter(c);
        }
        else {
            IImageView::Probe(icon)->SetColorFilter((IColorFilter*)NULL);
        }
    }
}

ECode BaseStatusBar::IsMediaNotification(
    /* [in] */ INotificationDataEntry* entry,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    // TODO: confirm that there's a valid media key
    AutoPtr<IView> expandedBig;
    entry->GetExpandedBig((IView**)&expandedBig);
    AutoPtr<IView> view;
    *result = expandedBig.Get() != NULL &&
        (expandedBig->FindViewById(Elastos::Droid::R::id::media_actions, (IView**)&view), view.Get()) != NULL;
    return NOERROR;
}

void BaseStatusBar::StartAppOwnNotificationSettingsActivity(
    /* [in] */ IIntent* intent,
    /* [in] */ Int32 notificationId,
    /* [in] */ const String& notificationTag,
    /* [in] */ Int32 appUid)
{
    intent->PutExtra(String("notification_id"), notificationId);
    intent->PutExtra(String("notification_tag"), notificationTag);
    StartNotificationGutsIntent(intent, appUid);
}

void BaseStatusBar::StartAppNotificationSettingsActivity(
    /* [in] */ const String& packageName,
    /* [in] */ Int32 appUid)
{
    AutoPtr<IIntent> intent;
    CIntent::New(ISettings::ACTION_APP_NOTIFICATION_SETTINGS, (IIntent**)&intent);
    intent->PutExtra(ISettings::EXTRA_APP_PACKAGE, packageName);
    intent->PutExtra(ISettings::EXTRA_APP_UID, appUid);
    StartNotificationGutsIntent(intent, appUid);
}

void BaseStatusBar::StartNotificationGutsIntent(
    /* [in] */ IIntent* intent,
    /* [in] */ Int32 appUid)
{
    Boolean keyguardShowing = FALSE;
    mStatusBarKeyguardViewManager->IsShowing(&keyguardShowing);
    AutoPtr<NotificationGutsOnDismissAction> action = new NotificationGutsOnDismissAction(
        this, keyguardShowing, intent, appUid);
    DismissKeyguardThenExecute(action, FALSE /* afterKeyguardGone */);
}

void BaseStatusBar::InflateGuts(
    /* [in] */ IExpandableNotificationRow* row)
{
    AutoPtr<IView> view;
    IView::Probe(row)->FindViewById(R::id::notification_guts_stub, (IView**)&view);
    AutoPtr<IViewStub> stub = IViewStub::Probe(view);
    if (stub != NULL) {
        view = NULL;
        stub->Inflate((IView**)&view);
    }
    AutoPtr<IStatusBarNotification> sbn;
    row->GetStatusBarNotification((IStatusBarNotification**)&sbn);

    AutoPtr<IUserHandle> uh;
    sbn->GetUser((IUserHandle**)&uh);
    Int32 userid = 0;
    uh->GetIdentifier(&userid);
    AutoPtr<IPackageManager> pmUser = GetPackageManagerForUser(userid);

    String n;
    sbn->GetPackageName(&n);
    AutoPtr<ICharSequence> tag;
    CString::New(n, (ICharSequence**)&tag);
    IView::Probe(row)->SetTag(tag);
    AutoPtr<IView> guts;
    IView::Probe(row)->FindViewById(R::id::notification_guts, (IView**)&guts);
    String pkg = n;
    String appname = pkg;
    AutoPtr<IDrawable> pkgicon;
    Int32 appUid = -1;

    AutoPtr<IApplicationInfo> info;
    ECode ec = pmUser->GetApplicationInfo(pkg,
            IPackageManager::GET_UNINSTALLED_PACKAGES | IPackageManager::GET_DISABLED_COMPONENTS,
            (IApplicationInfo**)&info);
    if (info != NULL) {
        AutoPtr<ICharSequence> label;
        pmUser->GetApplicationLabel(info, (ICharSequence**)&label);
        label->ToString(&appname);
        pmUser->GetApplicationIcon(info, (IDrawable**)&pkgicon);
        info->GetUid(&appUid);
    }
    if (ec == (Int32)E_NAME_NOT_FOUND_EXCEPTION) {
        // app is gone, just show package name and generic icon
        pmUser->GetDefaultActivityIcon((IDrawable**)&pkgicon);
    }

    view = NULL;
    IView::Probe(row)->FindViewById(Elastos::Droid::R::id::icon, (IView**)&view);
    IImageView::Probe(view)->SetImageDrawable(pkgicon);

    view = NULL;
    IView::Probe(row)->FindViewById(R::id::timestamp, (IView**)&view);
    Int64 t = 0;
    sbn->GetPostTime(&t);
    IDateTimeView::Probe(view)->SetTime(t);

    view = NULL;
    IView::Probe(row)->FindViewById(R::id::pkgname, (IView**)&view);
    AutoPtr<ICharSequence> an;
    CString::New(appname, (ICharSequence**)&an);
    ITextView::Probe(view)->SetText(an);
    AutoPtr<IView> settingsButton;
    guts->FindViewById(R::id::notification_inspect_item, (IView**)&settingsButton);
    AutoPtr<IView> appSettingsButton;
    guts->FindViewById(R::id::notification_inspect_app_provided_settings, (IView**)&appSettingsButton);
    if (appUid >= 0) {
        const Int32 appUidF = appUid;
        AutoPtr<ViewOnClickListener2> listener = new ViewOnClickListener2(this, pkg, appUidF);
        settingsButton->SetOnClickListener(listener);

        AutoPtr<IIntent> appSettingsQueryIntent;
        CIntent::New(IIntent::ACTION_MAIN, (IIntent**)&appSettingsQueryIntent);
        appSettingsQueryIntent->AddCategory(INotification::INTENT_CATEGORY_NOTIFICATION_PREFERENCES);
        appSettingsQueryIntent->SetPackage(pkg);
        AutoPtr<IList> infos;  /*<ResolveInfo*/
        pmUser->QueryIntentActivities(appSettingsQueryIntent, 0, (IList**)&infos);
        Int32 size = 0;
        infos->GetSize(&size);
        if (size > 0) {
            appSettingsButton->SetVisibility(IView::VISIBLE);

            AutoPtr<IResources> res;
            mContext->GetResources((IResources**)&res);
            String s;
            AutoPtr<ArrayOf<IInterface*> > ss = ArrayOf<IInterface*>::Alloc(1);
            ss->Set(0, an);
            res->GetString(R::string::status_bar_notification_app_settings_title, ss, &s);
            AutoPtr<ICharSequence> des;
            CString::New(s, (ICharSequence**)&des);
            appSettingsButton->SetContentDescription(des);
            AutoPtr<IIntent> appSettingsLaunchIntent;
            CIntent::New(appSettingsQueryIntent, (IIntent**)&appSettingsLaunchIntent);

            AutoPtr<IActivityInfo> activityInfo;
            AutoPtr<IInterface> obj;
            infos->Get(0, (IInterface**)&obj);
            IResolveInfo::Probe(obj)->GetActivityInfo((IActivityInfo**)&activityInfo);
            String name;
            IPackageItemInfo::Probe(activityInfo)->GetName(&name);
            appSettingsLaunchIntent->SetClassName(pkg, name);

            AutoPtr<ViewOnClickListener3> listener = new ViewOnClickListener3(this,
                appSettingsLaunchIntent, sbn, appUidF);
            appSettingsButton->SetOnClickListener(listener);
        }
        else {
            appSettingsButton->SetVisibility(IView::GONE);
        }
    }
    else {
        settingsButton->SetVisibility(IView::GONE);
        appSettingsButton->SetVisibility(IView::GONE);
    }
}

AutoPtr<ISwipeHelperLongPressListener> BaseStatusBar::GetNotificationLongClicker()
{
    AutoPtr<ISwipeHelperLongPressListener> listener = new BaseLongPressListener(this);
    return listener;
}

ECode BaseStatusBar::DismissPopups()
{
    if (mNotificationGutsExposed != NULL) {
        AutoPtr<INotificationGuts> v = mNotificationGutsExposed;
        mNotificationGutsExposed = NULL;

        AutoPtr<IBinder> token;
        IView::Probe(v)->GetWindowToken((IBinder**)&token);
        if (token.Get() == NULL) return NOERROR;

        Int32 iv1 = 0, iv2 = 0;
        Int32 x = ((IView::Probe(v)->GetLeft(&iv1), iv1) + (IView::Probe(v)->GetRight(&iv2), iv2)) / 2;
        Int32 y = ((IView::Probe(v)->GetTop(&iv1), iv1) + (v->GetActualHeight(&iv2), iv2) / 2);

        AutoPtr<IViewAnimationUtilsHelper> helper;
        CViewAnimationUtilsHelper::AcquireSingleton((IViewAnimationUtilsHelper**)&helper);
        AutoPtr<IAnimator> a;
        helper->CreateCircularReveal(IView::Probe(v), x, y, x, 0, (IAnimator**)&a);
        a->SetDuration(200);
        a->SetInterpolator(mFastOutLinearIn);
        AutoPtr<BaseAnimatorListenerAdapter> listener = new BaseAnimatorListenerAdapter(IView::Probe(v));
        a->AddListener(listener);
        a->Start();
    }
    return NOERROR;
}

ECode BaseStatusBar::OnHeadsUpDismissed()
{
    return NOERROR;
}

ECode BaseStatusBar::ShowRecentApps(
    /* [in] */ Boolean triggeredFromAltTab)
{
    Int32 msg = MSG_SHOW_RECENT_APPS;
    mHandler->RemoveMessages(msg);
    AutoPtr<IMessage> m;
    mHandler->ObtainMessage(msg, triggeredFromAltTab ? 1 : 0, 0, (IMessage**)&m);
    m->SendToTarget();
    return NOERROR;
}

ECode BaseStatusBar::HideRecentApps(
    /* [in] */ Boolean triggeredFromAltTab,
    /* [in] */ Boolean triggeredFromHomeKey)
{
    Int32 msg = MSG_HIDE_RECENT_APPS;
    mHandler->RemoveMessages(msg);
    AutoPtr<IMessage> m;
    mHandler->ObtainMessage(msg, triggeredFromAltTab ? 1 : 0,
            triggeredFromHomeKey ? 1 : 0, (IMessage**)&m);
    m->SendToTarget();
    return NOERROR;
}

ECode BaseStatusBar::ToggleRecentApps()
{
    Int32 msg = MSG_TOGGLE_RECENTS_APPS;
    mHandler->RemoveMessages(msg);
    Boolean tmp = FALSE;
    mHandler->SendEmptyMessage(msg, &tmp);
    return NOERROR;
}

ECode BaseStatusBar::PreloadRecentApps()
{
    Int32 msg = MSG_PRELOAD_RECENT_APPS;
    mHandler->RemoveMessages(msg);
    Boolean tmp = FALSE;
    mHandler->SendEmptyMessage(msg, &tmp);
    return NOERROR;
}

ECode BaseStatusBar::CancelPreloadRecentApps()
{
    Int32 msg = MSG_CANCEL_PRELOAD_RECENT_APPS;
    mHandler->RemoveMessages(msg);
    Boolean tmp = FALSE;
    mHandler->SendEmptyMessage(msg, &tmp);
    return NOERROR;
}

ECode BaseStatusBar::ShowNextAffiliatedTask()
{
    Int32 msg = MSG_SHOW_NEXT_AFFILIATED_TASK;
    mHandler->RemoveMessages(msg);
    Boolean tmp = FALSE;
    mHandler->SendEmptyMessage(msg, &tmp);
    return NOERROR;
}

ECode BaseStatusBar::ShowPreviousAffiliatedTask()
{
    Int32 msg = MSG_SHOW_PREV_AFFILIATED_TASK;
    mHandler->RemoveMessages(msg);
    Boolean tmp = FALSE;
    mHandler->SendEmptyMessage(msg, &tmp);
    return NOERROR;
}

ECode BaseStatusBar::ShowSearchPanel()
{
    Boolean tmp = FALSE;
    if (mSearchPanelView != NULL && (mSearchPanelView->IsAssistantAvailable(&tmp), tmp)) {
        mSearchPanelView->Show(TRUE, TRUE);
    }
    return NOERROR;
}

ECode BaseStatusBar::HideSearchPanel()
{
    Int32 msg = MSG_CLOSE_SEARCH_PANEL;
    mHandler->RemoveMessages(msg);
    Boolean tmp = FALSE;
    mHandler->SendEmptyMessage(msg, &tmp);
    return NOERROR;
}

void BaseStatusBar::UpdateSearchPanel()
{
    // Search Panel
    Boolean visible = FALSE;
    if (mSearchPanelView != NULL) {
        mSearchPanelView->IsShowing(&visible);
        IViewManager::Probe(mWindowManager)->RemoveView(IView::Probe(mSearchPanelView));
    }

    // Provide SearchPanel with a temporary parent to allow layout params to work.
    AutoPtr<ILinearLayout> tmpRoot;
    CLinearLayout::New(mContext, (ILinearLayout**)&tmpRoot);

    AutoPtr<ILayoutInflater> inflater;
    LayoutInflater::From(mContext, (ILayoutInflater**)&inflater);
    AutoPtr<IView> view;
    inflater->Inflate(R::layout::status_bar_search_panel, IViewGroup::Probe(tmpRoot), FALSE, (IView**)&view);
    mSearchPanelView = ISearchPanelView::Probe(view);
    assert(IView::Probe(mSearchPanelView) != NULL);
    view->SetOnTouchListener(
        new TouchOutsideListener(MSG_CLOSE_SEARCH_PANEL, IStatusBarPanel::Probe(mSearchPanelView), this));
    view->SetVisibility(IView::GONE);

    Boolean tmp = FALSE;
    Boolean vertical = mNavigationBarView != NULL && (mNavigationBarView->IsVertical(&tmp), tmp);
    mSearchPanelView->SetHorizontal(vertical);

    AutoPtr<IViewGroupLayoutParams> vp;
    IView::Probe(mSearchPanelView)->GetLayoutParams((IViewGroupLayoutParams**)&vp);
    AutoPtr<IWindowManagerLayoutParams> lp = GetSearchLayoutParams(vp);

    IViewManager::Probe(mWindowManager)->AddView(view, IViewGroupLayoutParams::Probe(lp));
    mSearchPanelView->SetBar(this);
    if (visible) {
        mSearchPanelView->Show(TRUE, FALSE);
    }
}

AutoPtr<BaseStatusBar::H> BaseStatusBar::CreateHandler()
{
    AutoPtr<H> h = new H(this);
    return h;
}

void BaseStatusBar::SendCloseSystemWindows(
    /* [in] */ IContext* context,
    /* [in] */ const String& reason)
{
    if (ActivityManagerNative::IsSystemReady()) {
        ActivityManagerNative::GetDefault()->CloseSystemDialogs(reason);
    }
}

void BaseStatusBar::ShowRecents(
    /* [in] */ Boolean triggeredFromAltTab)
{
    if (mRecents != NULL) {
        SendCloseSystemWindows(mContext, SYSTEM_DIALOG_REASON_RECENT_APPS);
        AutoPtr<IView> view;
        GetStatusBarView((IView**)&view);
        mRecents->ShowRecents(triggeredFromAltTab, view);
    }
}

void BaseStatusBar::HideRecents(
    /* [in] */ Boolean triggeredFromAltTab,
    /* [in] */ Boolean triggeredFromHomeKey)
{
    if (mRecents != NULL) {
        mRecents->HideRecents(triggeredFromAltTab, triggeredFromHomeKey);
    }
}

void BaseStatusBar::ToggleRecents()
{
    if (mRecents != NULL) {
        SendCloseSystemWindows(mContext, SYSTEM_DIALOG_REASON_RECENT_APPS);
        AutoPtr<IView> view;
        GetStatusBarView((IView**)&view);
        mRecents->ToggleRecents(mDisplay, mLayoutDirection, view);
    }
}

void BaseStatusBar::PreloadRecents()
{
    if (mRecents != NULL) {
        mRecents->PreloadRecents();
    }
}

void BaseStatusBar::CancelPreloadingRecents()
{
    if (mRecents != NULL) {
        mRecents->CancelPreloadingRecents();
    }
}

void BaseStatusBar::ShowRecentsNextAffiliatedTask()
{
    if (mRecents != NULL) {
        mRecents->ShowNextAffiliatedTask();
    }
}

void BaseStatusBar::ShowRecentsPreviousAffiliatedTask()
{
    if (mRecents != NULL) {
        mRecents->ShowPrevAffiliatedTask();
    }
}

ECode BaseStatusBar::OnVisibilityChanged(
    /* [in] */ Boolean visible)
{
    // Do nothing
    return NOERROR;
}

ECode BaseStatusBar::SetLockscreenPublicMode(
    /* [in] */ Boolean publicMode)
{
    mLockscreenPublicMode = publicMode;
    return NOERROR;
}

ECode BaseStatusBar::IsLockscreenPublicMode(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mLockscreenPublicMode;
    return NOERROR;
}

ECode BaseStatusBar::UserAllowsPrivateNotificationsInPublic(
    /* [in] */ Int32 userHandle,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    if (userHandle == IUserHandle::USER_ALL) {
        *result = TRUE;
        return NOERROR;
    }

    Int32 value = 0;
    mUsersAllowingPrivateNotifications->IndexOfKey(userHandle, &value);
    if (value < 0) {
        AutoPtr<IContentResolver> cr;
        mContext->GetContentResolver((IContentResolver**)&cr);
        Elastos::Droid::Provider::Settings::Secure::GetInt32ForUser(cr,
                ISettingsSecure::LOCK_SCREEN_ALLOW_PRIVATE_NOTIFICATIONS, 0, userHandle, &value);
        Boolean allowed = 0 != value;

        Int32 dpmFlags = 0;
        if (mDevicePolicyManager != NULL) {
            mDevicePolicyManager->GetKeyguardDisabledFeatures(NULL /* admin */, userHandle, &dpmFlags);
        }
        Boolean allowedByDpm = (dpmFlags
                & IDevicePolicyManager::KEYGUARD_DISABLE_UNREDACTED_NOTIFICATIONS) == 0;
        mUsersAllowingPrivateNotifications->Append(userHandle, allowed && allowedByDpm);
        *result = allowed;
        return NOERROR;
    }

    return mUsersAllowingPrivateNotifications->Get(userHandle, result);
}

ECode BaseStatusBar::ShouldHideSensitiveContents(
    /* [in] */ Int32 userid,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    Boolean f1 = FALSE, f2 = FALSE;
    *result = (IsLockscreenPublicMode(&f1), f1)
            && (UserAllowsPrivateNotificationsInPublic(userid, &f2), !f2);
    return NOERROR;
}

ECode BaseStatusBar::OnNotificationClear(
    /* [in] */ IStatusBarNotification* notification)
{
    String pkg, tag;
    notification->GetPackageName(&pkg);
    notification->GetTag(&tag);

    Int32 id = 0, userid = 0;
    notification->GetId(&id);
    notification->GetUserId(&userid);
    mBarService->OnNotificationClear(pkg, tag, id, userid);
    return NOERROR;
}

void BaseStatusBar::WorkAroundBadLayerDrawableOpacity(
    /* [in] */ IView* v)
{
}

Boolean BaseStatusBar::InflateViews(
    /* [in] */ INotificationDataEntry* entry,
    /* [in] */ IViewGroup* parent)
{
    return InflateViews(entry, parent, FALSE);
}

Boolean BaseStatusBar::InflateViewsForHeadsUp(
    /* [in] */ INotificationDataEntry* entry,
    /* [in] */ IViewGroup* parent)
{
    return InflateViews(entry, parent, TRUE);
}

Boolean BaseStatusBar::InflateViews(
    /* [in] */ INotificationDataEntry* entry,
    /* [in] */ IViewGroup* parent,
    /* [in] */ Boolean isHeadsUp)
{
    AutoPtr<IStatusBarNotification> sbn;
    entry->GetNotification((IStatusBarNotification**)&sbn);

    AutoPtr<IUserHandle> uh;
    sbn->GetUser((IUserHandle**)&uh);
    Int32 userid = 0;
    uh->GetIdentifier(&userid);

    AutoPtr<IPackageManager> pmUser = GetPackageManagerForUser(userid);

    Int32 maxHeight = mRowMaxHeight;
    AutoPtr<INotification> n;
    sbn->GetNotification((INotification**)&n);
    AutoPtr<IRemoteViews> contentView;
    n->GetContentView((IRemoteViews**)&contentView);
    AutoPtr<IRemoteViews> bigContentView;
    n->GetBigContentView((IRemoteViews**)&bigContentView);

    if (isHeadsUp) {
        AutoPtr<IResources> res;
        mContext->GetResources((IResources**)&res);
        res->GetDimensionPixelSize(R::dimen::notification_mid_height, &maxHeight);
        bigContentView = NULL;
        n->GetHeadsUpContentView((IRemoteViews**)&bigContentView);
    }

    if (contentView == NULL) {
        return FALSE;
    }

    AutoPtr<INotification> publicNotification;
    n->GetPublicVersion((INotification**)&publicNotification);
    if (DEBUG) {
        Logger::V(TAG, "publicNotification: %s", TO_CSTR(publicNotification));
    }

    AutoPtr<IExpandableNotificationRow> row;

    // Stash away previous user expansion state so we can restore it at
    // the end.
    Boolean hasUserChangedExpansion = FALSE;
    Boolean userExpanded = FALSE;
    Boolean userLocked = FALSE;

    AutoPtr<IExpandableNotificationRow> r;
    entry->GetRow((IExpandableNotificationRow**)&r);
    if (r.Get() != NULL) {
        row = r;
        row->HasUserChangedExpansion(&hasUserChangedExpansion);
        row->IsUserExpanded(&userExpanded);
        row->IsUserLocked(&userLocked);
        entry->Reset();
        if (hasUserChangedExpansion) {
            row->SetUserExpanded(userExpanded);
        }
    }
    else {
        // create the row view
        AutoPtr<IInterface> obj;
        mContext->GetSystemService(IContext::LAYOUT_INFLATER_SERVICE, (IInterface**)&obj);
        AutoPtr<ILayoutInflater> inflater = ILayoutInflater::Probe(obj);

        AutoPtr<IView> view;
        inflater->Inflate(R::layout::status_bar_notification_row, parent, FALSE, (IView**)&view);
        row = IExpandableNotificationRow::Probe(view);

        String key;
        sbn->GetKey(&key);
        row->SetExpansionLogger(this, key);
    }

    IView* rowView = IView::Probe(row);
    WorkAroundBadLayerDrawableOpacity(rowView);
    AutoPtr<IView> vetoButton = UpdateNotificationVetoButton(rowView, sbn);

    String str;
    mContext->GetString(R::string::accessibility_remove_notification, &str);
    AutoPtr<ICharSequence> des = CoreUtils::Convert(str);
    vetoButton->SetContentDescription(des);

    // NB: the large icon is now handled entirely by the template

    // bind the click event to the content area
    AutoPtr<IView> view;
    rowView->FindViewById(R::id::expanded, (IView**)&view);
    AutoPtr<INotificationContentView> expanded = INotificationContentView::Probe(view);

    view = NULL;
    rowView->FindViewById(R::id::expandedPublic, (IView**)&view);
    AutoPtr<INotificationContentView> expandedPublic = INotificationContentView::Probe(view);

    IViewGroup::Probe(row)->SetDescendantFocusability(IViewGroup::FOCUS_BLOCK_DESCENDANTS);

    AutoPtr<IPendingIntent> contentIntent;
    n->GetContentIntent((IPendingIntent**)&contentIntent);
    if (contentIntent != NULL) {
        String key;
        sbn->GetKey(&key);
        AutoPtr<INotificationClicker> l;
        MakeClicker(contentIntent, key, isHeadsUp, (INotificationClicker**)&l);
        AutoPtr<IViewOnClickListener> listener = IViewOnClickListener::Probe(l);
        rowView->SetOnClickListener(listener);
    }
    else {
        rowView->SetOnClickListener(NULL);
    }

    // set up the adaptive layout
    AutoPtr<IView> contentViewLocal;
    AutoPtr<IView> bigContentViewLocal;

    ECode ec = contentView->Apply(mContext, IViewGroup::Probe(expanded), mOnClickHandler,
            (IView**)&contentViewLocal);
    if (ec == (Int32)E_RUNTIME_EXCEPTION) {
        String p;
        sbn->GetPackageName(&p);
        Int32 id = 0;
        sbn->GetId(&id);
        Logger::E(TAG, "couldn't inflate view for notification %s/0x%08x",  p.string(), id);
        return FALSE;
    }
    if (bigContentView != NULL) {
        ec = bigContentView->Apply(mContext, IViewGroup::Probe(expanded), mOnClickHandler,
                (IView**)&bigContentViewLocal);
        if (ec == (Int32)E_RUNTIME_EXCEPTION) {
            String p;
            sbn->GetPackageName(&p);
            Int32 id = 0;
            sbn->GetId(&id);
            Logger::E(TAG, "couldn't inflate view for notification %s/0x%08x",  p.string(), id);
            return FALSE;
        }
    }

    if (contentViewLocal != NULL) {
        contentViewLocal->SetIsRootNamespace(TRUE);
        expanded->SetContractedChild(contentViewLocal);
    }
    if (bigContentViewLocal != NULL) {
        bigContentViewLocal->SetIsRootNamespace(TRUE);
        expanded->SetExpandedChild(bigContentViewLocal);
    }

    // now the public version
    AutoPtr<IView> publicViewLocal;
    if (publicNotification != NULL) {
        AutoPtr<IRemoteViews> rv;
        publicNotification->GetContentView((IRemoteViews**)&rv);
        ec = rv->Apply(mContext, IViewGroup::Probe(expandedPublic),
                mOnClickHandler, (IView**)&publicViewLocal);

        if (ec == (ECode)E_RUNTIME_EXCEPTION) {
            String p;
            sbn->GetPackageName(&p);
            Int32 id = 0;
            sbn->GetId(&id);
            Logger::E(TAG, "couldn't inflate view for notification %s/0x%08x",  p.string(), id);
            publicViewLocal = NULL;
        }
        else {
            if (publicViewLocal != NULL) {
                publicViewLocal->SetIsRootNamespace(TRUE);
                expandedPublic->SetContractedChild(publicViewLocal);
            }
        }
    }

    // Extract target SDK version.
    AutoPtr<IApplicationInfo> info;
    String pkgname;
    sbn->GetPackageName(&pkgname);
    ec = pmUser->GetApplicationInfo(pkgname, 0, (IApplicationInfo**)&info);
    if (ec == (ECode)E_NAME_NOT_FOUND_EXCEPTION) {
        Logger::E(TAG, "Failed looking up ApplicationInfo for %s", pkgname.string());
    }
    else {
        Int32 targetSdkVersion = 0;
        info->GetTargetSdkVersion(&targetSdkVersion);
        entry->SetTargetSdk(targetSdkVersion);
    }

    if (publicViewLocal == NULL) {
        // Add a basic notification template
        AutoPtr<ILayoutInflater> inflater;
        LayoutInflater::From(mContext, (ILayoutInflater**)&inflater);
        inflater->Inflate(R::layout::notification_public_default,
                IViewGroup::Probe(expandedPublic), FALSE, (IView**)&publicViewLocal);
        publicViewLocal->SetIsRootNamespace(TRUE);
        expandedPublic->SetContractedChild(publicViewLocal);

        view = NULL;
        publicViewLocal->FindViewById(R::id::title, (IView**)&view);
        AutoPtr<ITextView> title = ITextView::Probe(view);

        info = NULL;
        if (pmUser->GetApplicationInfo(pkgname, 0, (IApplicationInfo**)&info) == (ECode)E_NAME_NOT_FOUND_EXCEPTION) {
            AutoPtr<ICharSequence> text;
            CString::New(pkgname, (ICharSequence**)&text);
            title->SetText(text);
        }
        else {
            AutoPtr<ICharSequence> label;
            pmUser->GetApplicationLabel(info, (ICharSequence**)&label);
            title->SetText(label);
        }

        view = NULL;
        publicViewLocal->FindViewById(R::id::icon, (IView**)&view);
        AutoPtr<IImageView> icon = IImageView::Probe(view);

        view = NULL;
        publicViewLocal->FindViewById(R::id::profile_badge_line3, (IView**)&view);
        AutoPtr<IImageView> profileBadge = IImageView::Probe(view);

        Int32 icon1, iconLevel = 0, number = 0;
        n->GetIcon(&icon1);
        n->GetIconLevel(&iconLevel);
        n->GetNumber(&number);

        AutoPtr<ICharSequence> tickerText;
        n->GetTickerText((ICharSequence**)&tickerText);
        AutoPtr<IStatusBarIcon> ic;
        CStatusBarIcon::New(pkgname, uh, icon1, iconLevel,
                number, tickerText, (IStatusBarIcon**)&ic);

        AutoPtr<IDrawable> iconDrawable = StatusBarIconView::GetIcon(mContext, ic);
        icon->SetImageDrawable(iconDrawable);
        Int32 targetSdk = 0;
        entry->GetTargetSdk(&targetSdk);
        Boolean tmp = FALSE;
        if (targetSdk >= Build::VERSION_CODES::LOLLIPOP
                || (mNotificationColorUtil->IsGrayscaleIcon(iconDrawable, &tmp), tmp)) {
            IView::Probe(icon)->SetBackgroundResource(
                Elastos::Droid::R::drawable::notification_icon_legacy_bg);

            AutoPtr<IResources> res;
            mContext->GetResources((IResources**)&res);
            Int32 padding = 0;
            res->GetDimensionPixelSize(
                Elastos::Droid::R::dimen::notification_large_icon_circle_padding, &padding);
            IView::Probe(icon)->SetPadding(padding, padding, padding, padding);
            Int32 color = 0;
            n->GetColor(&color);
            if (color != INotification::COLOR_DEFAULT) {
                AutoPtr<IDrawable> bk;
                IView::Probe(icon)->GetBackground((IDrawable**)&bk);
                bk->SetColorFilter(color, PorterDuffMode_SRC_ATOP);
            }
        }

        if (profileBadge != NULL) {
            AutoPtr<IPackageManager> pm;
            mContext->GetPackageManager((IPackageManager**)&pm);
            AutoPtr<IDrawable> profileDrawable;
            pm->GetUserBadgeForDensity(uh, 0, (IDrawable**)&profileDrawable);
            if (profileDrawable != NULL) {
                profileBadge->SetImageDrawable(profileDrawable);
                IView::Probe(profileBadge)->SetVisibility(IView::VISIBLE);
            }
            else {
                IView::Probe(profileBadge)->SetVisibility(IView::GONE);
            }
        }

        AutoPtr<IView> privateTime;
        contentViewLocal->FindViewById(Elastos::Droid::R::id::time, (IView**)&privateTime);

        view = NULL;
        publicViewLocal->FindViewById(R::id::time, (IView**)&view);
        AutoPtr<IDateTimeView> time = IDateTimeView::Probe(view);
        Int32 v = 0;
        if (privateTime != NULL && (privateTime->GetVisibility(&v), v) == IView::VISIBLE) {
            IView::Probe(time)->SetVisibility(IView::VISIBLE);
            Int64 when = 0;
            n->GetWhen(&when);
            time->SetTime(when);
        }

        view = NULL;
        publicViewLocal->FindViewById(R::id::text, (IView**)&view);
        AutoPtr<ITextView> text = ITextView::Probe(view);
        if (text != NULL) {
            text->SetText(R::string::notification_hidden_text);
            text->SetTextAppearance(mContext,
                R::style::TextAppearance_Material_Notification_Parenthetical);
        }

        AutoPtr<IResources> res;
        mContext->GetResources((IResources**)&res);
        AutoPtr<IConfiguration> config;
        res->GetConfiguration((IConfiguration**)&config);
        Float fontScale = 0;
        config->GetFontScale(&fontScale);

        AutoPtr<INotificationBuilderHelper> nBHelper;
        CNotificationBuilderHelper::AcquireSingleton((INotificationBuilderHelper**)&nBHelper);
        Int32 topPadding = 0;
        nBHelper->CalculateTopPadding(mContext, FALSE /* hasThreeLines */,
                fontScale, &topPadding);
        IView::Probe(title)->SetPadding(0, topPadding, 0, 0);

        entry->SetAutoRedacted(TRUE);
    }

    Boolean tmp = FALSE;
    sbn->IsClearable(&tmp);
    row->SetClearable(tmp);

    if (MULTIUSER_DEBUG) {
        view = NULL;
        rowView->FindViewById(R::id::debug_info, (IView**)&view);
        AutoPtr<ITextView> debug = ITextView::Probe(view);
        if (debug != NULL) {
            IView::Probe(debug)->SetVisibility(IView::VISIBLE);
            Int32 userid = 0;
            sbn->GetUserId(&userid);
            String s = String("CU ") + StringUtils::ToString(mCurrentUserId) +" NU "
                    + StringUtils::ToString(userid);
            AutoPtr<ICharSequence> text;
            CString::New(s, (ICharSequence**)&text);
            debug->SetText(text);
        }
    }
    entry->SetRow(row);
    AutoPtr<IExpandableNotificationRow> nr;
    entry->GetRow((IExpandableNotificationRow**)&nr);
    nr->SetHeightRange(mRowMinHeight, maxHeight);
    AutoPtr<ActivatableNotificationViewListener> listener = new ActivatableNotificationViewListener(this);
    IActivatableNotificationView::Probe(nr)->SetOnActivatedListener(listener);
    entry->SetExpanded(contentViewLocal);
    entry->SetExpandedPublic(publicViewLocal);
    entry->SetBigContentView(bigContentViewLocal);

    ApplyColorsAndBackgrounds(sbn, entry);

    // Restore previous flags.
    if (hasUserChangedExpansion) {
        // Note: setUserExpanded() conveniently ignores calls with
        //       userExpanded=TRUE if !isExpandable().
        row->SetUserExpanded(userExpanded);
    }
    row->SetUserLocked(userLocked);
    row->SetStatusBarNotification(sbn);
    return TRUE;
}

ECode BaseStatusBar::MakeClicker(
    /* [in] */ IPendingIntent* intent,
    /* [in] */ const String& notificationKey,
    /* [in] */ Boolean forHun,
    /* [out] */ INotificationClicker** clicker)
{
    VALIDATE_NOT_NULL(clicker);
    AutoPtr<NotificationClicker> c = new NotificationClicker(intent, notificationKey, forHun, this);
    *clicker = (INotificationClicker*)c.Get();
    REFCOUNT_ADD(*clicker);
    return NOERROR;
}

ECode BaseStatusBar::AnimateCollapsePanels(
    /* [in] */ Int32 flags,
    /* [in] */ Boolean force)
{
    return NOERROR;
}

ECode BaseStatusBar::OverrideActivityPendingAppTransition(
    /* [in] */ Boolean keyguardShowing)
{
    if (keyguardShowing) {
        if (mWindowManagerService->OverridePendingAppTransition(String(NULL), 0, 0, NULL) == (ECode)E_REMOTE_EXCEPTION) {
            Logger::W(TAG, "Error overriding app transition: ");
        }
    }
    return NOERROR;
}

void BaseStatusBar::VisibilityChanged(
    /* [in] */ Boolean visible)
{
    if (mPanelSlightlyVisible != visible) {
        mPanelSlightlyVisible = visible;
        if (!visible) {
            DismissPopups();
        }
        // try {
        if (visible) {
            mBarService->OnPanelRevealed();
        }
        else {
            mBarService->OnPanelHidden();
        }
        // } catch (RemoteException ex) {
        //     // Won't fail unless the world has ended.
        // }
    }
}

void BaseStatusBar::HandleNotificationError(
    /* [in] */ IStatusBarNotification* n,
    /* [in] */ const String& message)
{
    String key;
    n->GetKey(&key);
    RemoveNotification(key, NULL);
    // try {
    String pn, tag;
    n->GetPackageName(&pn);
    n->GetTag(&tag);
    Int32 id = 0, uid = 0, pid = 0, userid = 0;
    n->GetId(&id);
    n->GetUid(&uid);
    n->GetInitialPid(&pid);
    n->GetUserId(&userid);
    mBarService->OnNotificationError(pn, tag, id, uid,
            pid, message, userid);
    // } catch (RemoteException ex) {
    //     // The end is nigh.
    // }
}

AutoPtr<IStatusBarNotification> BaseStatusBar::RemoveNotificationViews(
    /* [in] */ const String& key,
    /* [in] */ INotificationListenerServiceRankingMap* ranking)
{
    AutoPtr<INotificationDataEntry> entry;
    mNotificationData->Remove(key, ranking, (INotificationDataEntry**)&entry);
    if (entry == NULL) {
        Logger::W(TAG, "removeNotification for unknown key: %s", key.string());
        return NULL;
    }
    UpdateNotifications();
    AutoPtr<IStatusBarNotification> notification;
    entry->GetNotification((IStatusBarNotification**)&notification);
    return notification;
}

AutoPtr<INotificationDataEntry> BaseStatusBar::CreateNotificationViews(
    /* [in] */ IStatusBarNotification* sbn)
{
    if (DEBUG) {
        Logger::D(TAG, "CreateNotificationViews(notification=%p", sbn);
    }
    // Construct the icon.
    AutoPtr<INotification> n;
    sbn->GetNotification((INotification**)&n);

    String pn;
    sbn->GetPackageName(&pn);
    Int32 id = 0;
    sbn->GetId(&id);
    String slot = pn + "/0x" + StringUtils::ToHexString(id);
    AutoPtr<IStatusBarIconView> iconView;
    CStatusBarIconView::New(mContext, slot, n, (IStatusBarIconView**)&iconView);
    IImageView::Probe(iconView)->SetScaleType(ImageViewScaleType_CENTER_INSIDE);

    AutoPtr<IUserHandle> uh;
    sbn->GetUser((IUserHandle**)&uh);

    Int32 icon;
    n->GetIcon(&icon);
    Int32 iconLevel = 0, number = 0;
    n->GetIconLevel(&iconLevel);
    n->GetNumber(&number);

    AutoPtr<ICharSequence> tickerText;
    n->GetTickerText((ICharSequence**)&tickerText);

    AutoPtr<IStatusBarIcon> ic;
    CStatusBarIcon::New(pn, uh, icon, iconLevel, number,
                tickerText, (IStatusBarIcon**)&ic);

    Boolean tmp = FALSE;
    if (iconView->Set(ic, &tmp), !tmp) {
        String str;
        str.AppendFormat("Couldn't create icon: %p", ic.Get());
        HandleNotificationError(sbn, str);
        return NULL;
    }
    // Construct the expanded view.
    AutoPtr<INotificationDataEntry> entry = new NotificationData::Entry(sbn, iconView);
    if (!InflateViews(entry, IViewGroup::Probe(mStackScroller))) {
        String str;
        str.AppendFormat("Couldn't expand RemoteViews for: %p", sbn);
        HandleNotificationError(sbn, str);
        return NULL;
    }
    return entry;
}

void BaseStatusBar::AddNotificationViews(
    /* [in] */ INotificationDataEntry* entry,
    /* [in] */ INotificationListenerServiceRankingMap* ranking)
{
    if (entry == NULL) {
        return;
    }
    // Add the expanded view and icon.
    mNotificationData->Add(entry, ranking);
    UpdateNotifications();
}

void BaseStatusBar::UpdateRowStates()
{
    Int32 maxKeyguardNotifications = GetMaxKeyguardNotifications();
    AutoPtr<INotificationOverflowIconsView> nov;
    mKeyguardIconOverflowContainer->GetIconsView((INotificationOverflowIconsView**)&nov);
    IViewGroup::Probe(nov)->RemoveAllViews();

    AutoPtr<IArrayList> activeNotifications;  /*<Entry*/
    mNotificationData->GetActiveNotifications((IArrayList**)&activeNotifications);
    Int32 N = 0;
    activeNotifications->GetSize(&N);
    Int32 visibleNotifications = 0;
    Boolean onKeyguard = mState == IStatusBarState::KEYGUARD;
    for (Int32 i = 0; i < N; i++) {
        AutoPtr<IInterface> obj;
        activeNotifications->Get(i, (IInterface**)&obj);
        AutoPtr<INotificationDataEntry> entry = INotificationDataEntry::Probe(obj);

        Boolean tmp = FALSE;
        AutoPtr<IExpandableNotificationRow> row;
        entry->GetRow((IExpandableNotificationRow**)&row);
        if (onKeyguard) {
            row->SetExpansionDisabled(TRUE);
        }
        else {
            row->SetExpansionDisabled(FALSE);
            if (row->IsUserLocked(&tmp), tmp) {
                Boolean top = (i == 0);
                row->SetSystemExpanded(top);
            }
        }

        AutoPtr<IStatusBarNotification> notification;
        entry->GetNotification((IStatusBarNotification**)&notification);
        Boolean showOnKeyguard = ShouldShowOnKeyguard(notification);
        if (((IsLockscreenPublicMode(&tmp), tmp) && !mShowLockscreenNotifications) ||
                (onKeyguard && (visibleNotifications >= maxKeyguardNotifications
                        || !showOnKeyguard))) {
            IView::Probe(row)->SetVisibility(IView::GONE);
            if (onKeyguard && showOnKeyguard) {
                nov->AddNotification(entry);
            }
        }
        else {
            Int32 v = 0;
            Boolean wasGone = (IView::Probe(row)->GetVisibility(&v), v) == IView::GONE;
            IView::Probe(row)->SetVisibility(IView::VISIBLE);
            if (wasGone) {
                // notify the scroller of a child addition
                mStackScroller->GenerateAddAnimation(IView::Probe(row), TRUE /* fromMoreCard */);
            }
            visibleNotifications++;
        }
    }

    Int32 count = 0;
    if (onKeyguard && (IViewGroup::Probe(nov)->GetChildCount(&count), count) > 0) {
        IView::Probe(mKeyguardIconOverflowContainer)->SetVisibility(IView::VISIBLE);
    }
    else {
        IView::Probe(mKeyguardIconOverflowContainer)->SetVisibility(IView::GONE);
    }

    IViewGroup::Probe(mStackScroller)->GetChildCount(&count);
    mStackScroller->ChangeViewPosition(IView::Probe(mKeyguardIconOverflowContainer), count - 3);
    mStackScroller->ChangeViewPosition(IView::Probe(mEmptyShadeView), count - 2);
    mStackScroller->ChangeViewPosition(IView::Probe(mDismissView), count - 1);
}

Boolean BaseStatusBar::ShouldShowOnKeyguard(
    /* [in] */ IStatusBarNotification* sbn)
{
    String key;
    sbn->GetKey(&key);
    Boolean tmp = FALSE;
    return mShowLockscreenNotifications && (mNotificationData->IsAmbient(key, &tmp), !tmp);
}

void BaseStatusBar::SetZenMode(
    /* [in] */ Int32 mode)
{
    Boolean tmp = FALSE;
    if (IsDeviceProvisioned(&tmp), !tmp) return;
    mZenMode = mode;
    UpdateNotifications();
}

void BaseStatusBar::SetShowLockscreenNotifications(
    /* [in] */ Boolean show)
{
    mShowLockscreenNotifications = show;
}

void BaseStatusBar::UpdateLockscreenNotificationSetting()
{
    AutoPtr<IContentResolver> cr;
    mContext->GetContentResolver((IContentResolver**)&cr);
    Int32 value = 0;
    Elastos::Droid::Provider::Settings::Secure::GetInt32ForUser(cr,
            ISettingsSecure::LOCK_SCREEN_SHOW_NOTIFICATIONS, 1,
            mCurrentUserId, &value);
    Boolean show = value != 0;
    Int32 dpmFlags = 0;
    if (mDevicePolicyManager != NULL) {
        mDevicePolicyManager->GetKeyguardDisabledFeatures(NULL /* admin */, mCurrentUserId, &dpmFlags);
    }
    Boolean allowedByDpm = (dpmFlags
            & IDevicePolicyManager::KEYGUARD_DISABLE_SECURE_NOTIFICATIONS) == 0;
    SetShowLockscreenNotifications(show && allowedByDpm);
}

ECode BaseStatusBar::UpdateNotification(
    /* [in] */ IStatusBarNotification* notification,
    /* [in] */ INotificationListenerServiceRankingMap* ranking)
{
    if (DEBUG) Logger::D(TAG, "UpdateNotification(%s)", TO_CSTR(notification));

    String key;
    notification->GetKey(&key);
    Boolean wasHeadsUp = FALSE;
    IsHeadsUp(key, &wasHeadsUp);
    AutoPtr<INotificationDataEntry> oldEntry;
    if (wasHeadsUp) {
        mHeadsUpNotificationView->GetEntry((INotificationDataEntry**)&oldEntry);
    }
    else {
        mNotificationData->Get(key, (INotificationDataEntry**)&oldEntry);
    }
    if (oldEntry == NULL) {
        return NOERROR;
    }

    AutoPtr<IStatusBarNotification> oldNotification;
    oldEntry->GetNotification((IStatusBarNotification**)&oldNotification);

    // XXX: modify when we do something more intelligent with the two content views
    AutoPtr<IRemoteViews> oldContentView;
    AutoPtr<INotification> on;
    oldNotification->GetNotification((INotification**)&on);
    on->GetContentView((IRemoteViews**)&oldContentView);
    AutoPtr<INotification> n;
    notification->GetNotification((INotification**)&n);
    AutoPtr<IRemoteViews> contentView;
    n->GetContentView((IRemoteViews**)&contentView);
    AutoPtr<IRemoteViews> oldBigContentView;
    on->GetBigContentView((IRemoteViews**)&oldBigContentView);
    AutoPtr<IRemoteViews> bigContentView;
    n->GetBigContentView((IRemoteViews**)&bigContentView);
    AutoPtr<IRemoteViews> oldHeadsUpContentView;
    on->GetHeadsUpContentView((IRemoteViews**)&oldHeadsUpContentView);
    AutoPtr<IRemoteViews> headsUpContentView;
    n->GetHeadsUpContentView((IRemoteViews**)&headsUpContentView);
    AutoPtr<INotification> oldPublicNotification;
    on->GetPublicVersion((INotification**)&oldPublicNotification);
    AutoPtr<IRemoteViews> oldPublicContentView;
    if (oldPublicNotification != NULL) {
        oldPublicNotification->GetContentView((IRemoteViews**)&oldPublicContentView);
    }

    AutoPtr<INotification> publicNotification;
    n->GetPublicVersion((INotification**)&publicNotification);
    AutoPtr<IRemoteViews> publicContentView;
    if (publicNotification != NULL) {
        publicNotification->GetContentView((IRemoteViews**)&publicContentView);
    }

    if (DEBUG) {
        Int64 when = 0;
        on->GetWhen(&when);
        Boolean going = FALSE;
        oldNotification->IsOngoing(&going);
        AutoPtr<IView> expanded;
        oldEntry->GetExpanded((IView**)&expanded);
        AutoPtr<IExpandableNotificationRow> row;
        oldEntry->GetRow((IExpandableNotificationRow**)&row);
        AutoPtr<IViewParent> parent;
        IView::Probe(row)->GetParent((IViewParent**)&parent);
        Logger::D(TAG, "old notification: when=%lld ongoing=%d expanded=%p contentView=%p bigContentView=%p publicView=%p rowParent=%p"
                , when, going, expanded.Get(), oldContentView.Get(), oldBigContentView.Get(),
                oldPublicContentView.Get(), parent.Get());

        n->GetWhen(&when);
        Logger::D(TAG, "new notification: when=%lld ongoing=%d contentView=%p bigContentView=%p publicView=%p",
                when, going, contentView.Get(), bigContentView.Get(), publicContentView.Get());
    }

    // Can we just reapply the RemoteViews in place?

    // 1U is never NULL
    AutoPtr<IView> expanded;
    oldEntry->GetExpanded((IView**)&expanded);
    String package;
    contentView->GetPackage(&package);
    String oldPackage;
    oldContentView->GetPackage(&oldPackage);
    Int32 nId = 0, oldId = 0;
    contentView->GetLayoutId(&nId);
    oldContentView->GetLayoutId(&oldId);
    Boolean contentsUnchanged = expanded.Get() != NULL
            && package != NULL
            && oldPackage != NULL
            && oldPackage.Equals(package)
            && oldId == nId;
    // large view may be NULL
    AutoPtr<IView> oldEntryBCView;
    oldEntry->GetBigContentView((IView**)&oldEntryBCView);
    Boolean bigContentsUnchanged =
            (oldEntryBCView.Get() == NULL && bigContentView == NULL)
            || ((oldEntryBCView.Get() != NULL && bigContentView != NULL)
                && (bigContentView->GetPackage(&package), package) != NULL
                && (oldBigContentView->GetPackage(&oldPackage), oldPackage) != NULL
                && oldPackage.Equals(package)
                && (oldBigContentView->GetLayoutId(&oldId), oldId) == (bigContentView->GetLayoutId(&nId), nId));

    Boolean headsUpContentsUnchanged =
            (oldHeadsUpContentView == NULL && headsUpContentView == NULL)
            || ((oldHeadsUpContentView != NULL && headsUpContentView != NULL)
                && (headsUpContentView->GetPackage(&package), package) != NULL
                && (oldHeadsUpContentView->GetPackage(&oldPackage), oldPackage) != NULL
                && oldPackage.Equals(package)
                && (oldHeadsUpContentView->GetLayoutId(&oldId), oldId) == (headsUpContentView->GetLayoutId(&nId), nId));

    Boolean publicUnchanged  =
            (oldPublicContentView == NULL && publicContentView == NULL)
            || ((oldPublicContentView != NULL && publicContentView != NULL)
                    && (publicContentView->GetPackage(&package), package) != NULL
                    && (oldPublicContentView->GetPackage(&oldPackage), oldPackage) != NULL
                    && oldPackage.Equals(package)
                    && (oldPublicContentView->GetLayoutId(&oldId), oldId) == (publicContentView->GetLayoutId(&nId), nId));

    AutoPtr<ICharSequence> tickerText;
    n->GetTickerText((ICharSequence**)&tickerText);
    AutoPtr<ICharSequence> oldTickerText;
    on->GetTickerText((ICharSequence**)&oldTickerText);
    Boolean updateTicker = tickerText.Get() != NULL
            && !TextUtils::Equals(tickerText, oldTickerText);

    Boolean shouldInterrupt = ShouldInterrupt(notification);
    Boolean alertAgain = AlertAgain(oldEntry, n);
    Boolean updateSuccessful = FALSE;
    if (contentsUnchanged && bigContentsUnchanged && headsUpContentsUnchanged
            && publicUnchanged) {
        if (DEBUG) Logger::D(TAG, "reusing notification for key: %s", key.string());
        oldEntry->SetNotification(notification);
        ECode ec = NOERROR;
        do {
            AutoPtr<IStatusBarIconView> icon;
            ec = oldEntry->GetIcon((IStatusBarIconView**)&icon);
            if (icon.Get() != NULL) {
                // Update the icon
                AutoPtr<IStatusBarIcon> ic;
                String name;
                notification->GetPackageName(&name);
                AutoPtr<IUserHandle> uh;
                notification->GetUser((IUserHandle**)&uh);
                Int32 nIcon;
                n->GetIcon(&nIcon);
                Int32 iconLevel = 0, number = 0;
                n->GetIconLevel(&iconLevel);
                n->GetNumber(&number);

                ec = CStatusBarIcon::New(name, uh, nIcon, iconLevel, number,
                        tickerText, (IStatusBarIcon**)&ic);
                icon->SetNotification(n);
                Boolean tmp = FALSE;
                if (icon->Set(ic, &tmp), !tmp) {
                    String str;
                    str.AppendFormat("Couldn't update icon: %p", ic.Get());
                    HandleNotificationError(notification, str);
                    return NOERROR;
                }
            }

            if (wasHeadsUp) {
                if (shouldInterrupt) {
                    UpdateHeadsUpViews(oldEntry, notification);
                    if (alertAgain) {
                        ResetHeadsUpDecayTimer();
                    }
                }
                else {
                    // we updated the notification above, so release to build a new shade entry
                    mHeadsUpNotificationView->ReleaseAndClose();
                    return NOERROR;
                }
            }
            else {
                if (shouldInterrupt && alertAgain) {
                    RemoveNotificationViews(key, ranking);
                    ec = AddNotification(notification, ranking);  //this will pop the headsup
                }
                else {
                    UpdateNotificationViews(oldEntry, notification);
                }
            }
            ec = mNotificationData->UpdateRanking(ranking);
            UpdateNotifications();
            updateSuccessful = TRUE;
        } while (0);
        if (ec == (Int32)E_RUNTIME_EXCEPTION) {
            // It failed to add cleanly.  Log, and remove the view from the panel.
            contentView->GetPackage(&package);
            Logger::W(TAG, "Couldn't reapply views for package %s", package.string());
        }
    }

    if (!updateSuccessful) {
        if (DEBUG) Logger::D(TAG, "not reusing notification for key: %s", key.string());
        if (wasHeadsUp) {
            if (shouldInterrupt) {
                if (DEBUG) Logger::D(TAG, "rebuilding heads up for key: %s", key.string());
                AutoPtr<INotificationDataEntry> newEntry = new NotificationData::Entry(notification, NULL);
                AutoPtr<IViewGroup> holder;
                mHeadsUpNotificationView->GetHolder((IViewGroup**)&holder);
                if (InflateViewsForHeadsUp(newEntry, holder)) {
                    Boolean tmp = FALSE;
                    mHeadsUpNotificationView->ShowNotification(newEntry, &tmp);
                    if (alertAgain) {
                        ResetHeadsUpDecayTimer();
                    }
                }
                else {
                    contentView->GetPackage(&package);
                    Logger::W(TAG, "Couldn't create new updated headsup for package %s", package.string());
                }
            }
            else {
                if (DEBUG) Logger::D(TAG, "releasing heads up for key: %s", key.string());
                oldEntry->SetNotification(notification);
                mHeadsUpNotificationView->ReleaseAndClose();
                return NOERROR;
            }
        }
        else {
            if (shouldInterrupt && alertAgain) {
                if (DEBUG) Logger::D(TAG, "reposting to invoke heads up for key: %s", key.string());
                RemoveNotificationViews(key, ranking);
                AddNotification(notification, ranking);  //this will pop the headsup
            }
            else {
                if (DEBUG) Logger::D(TAG, "rebuilding update in place for key: %s", key.string());
                oldEntry->SetNotification(notification);

                AutoPtr<IStatusBarIcon> ic;
                String name;
                notification->GetPackageName(&name);
                AutoPtr<IUserHandle> uh;
                notification->GetUser((IUserHandle**)&uh);
                Int32 icon;
                n->GetIcon(&icon);
                Int32 iconLevel = 0, number = 0;
                n->GetIconLevel(&iconLevel);
                n->GetNumber(&number);

                CStatusBarIcon::New(name, uh, icon, iconLevel, number,
                        tickerText, (IStatusBarIcon**)&ic);

                AutoPtr<IStatusBarIconView> oldEntryIcon;
                oldEntry->GetIcon((IStatusBarIconView**)&oldEntryIcon);
                oldEntryIcon->SetNotification(n);
                Boolean succeeded = FALSE;
                oldEntryIcon->Set(ic, &succeeded);
                InflateViews(oldEntry, IViewGroup::Probe(mStackScroller), wasHeadsUp);
                mNotificationData->UpdateRanking(ranking);
                UpdateNotifications();
            }
        }
    }

    // Update the veto button accordingly (and as a result, whether this row is
    // swipe-dismissable)
    AutoPtr<IExpandableNotificationRow> row;
    oldEntry->GetRow((IExpandableNotificationRow**)&row);
    UpdateNotificationVetoButton(IView::Probe(row), notification);

    // Is this for you?
    Boolean isForCurrentUser = FALSE;
    IsNotificationForCurrentProfiles(notification, &isForCurrentUser);
    if (DEBUG) Logger::D(TAG, "notification is %s for you", (isForCurrentUser ? "" : "not "));

    // Restart the ticker if it's still running
    if (updateTicker && isForCurrentUser) {
        HaltTicker();
        Tick(notification, FALSE);
    }

    // Recalculate the position of the sliding windows and the titles.
    SetAreThereNotifications();
    UpdateExpandedViewPos(EXPANDED_LEAVE_ALONE);
    return NOERROR;
}

void BaseStatusBar::UpdateNotificationViews(
    /* [in] */ INotificationDataEntry* entry,
    /* [in] */ IStatusBarNotification* notification)
{
    UpdateNotificationViews(entry, notification, FALSE);
}

void BaseStatusBar::UpdateHeadsUpViews(
    /* [in] */ INotificationDataEntry* entry,
    /* [in] */ IStatusBarNotification* notification)
{
    UpdateNotificationViews(entry, notification, TRUE);
}

void BaseStatusBar::UpdateNotificationViews(
    /* [in] */ INotificationDataEntry* entry,
    /* [in] */ IStatusBarNotification* notification,
    /* [in] */ Boolean isHeadsUp)
{
    AutoPtr<INotification> n;
    notification->GetNotification((INotification**)&n);
    AutoPtr<IRemoteViews> contentView;
    n->GetContentView((IRemoteViews**)&contentView);
    AutoPtr<IRemoteViews> headsUpContentView;
    n->GetHeadsUpContentView((IRemoteViews**)&headsUpContentView);
    AutoPtr<IRemoteViews> bContentView;
    n->GetBigContentView((IRemoteViews**)&bContentView);

    AutoPtr<IRemoteViews> bigContentView = isHeadsUp ? headsUpContentView : bContentView;
    AutoPtr<INotification> publicVersion;
    n->GetPublicVersion((INotification**)&publicVersion);

    AutoPtr<IRemoteViews> publicContentView;
    if (publicVersion != NULL) {
        publicVersion->GetContentView((IRemoteViews**)&publicContentView);
    }

    // Reapply the RemoteViews
    AutoPtr<IView> expanded;
    entry->GetExpanded((IView**)&expanded);
    contentView->Reapply(mContext, expanded, mOnClickHandler);
    AutoPtr<IView> v;
    entry->GetBigContentView((IView**)&v);
    if (bigContentView != NULL && v.Get() != NULL) {
        bigContentView->Reapply(mContext, v, mOnClickHandler);
    }
    v = NULL;
    if (publicContentView != NULL && (entry->GetPublicContentView((IView**)&v), v.Get()) != NULL) {
        publicContentView->Reapply(mContext, v, mOnClickHandler);
    }

    // update the contentIntent
    AutoPtr<IPendingIntent> contentIntent;
    n->GetContentIntent((IPendingIntent**)&contentIntent);
    AutoPtr<IExpandableNotificationRow> row;
    entry->GetRow((IExpandableNotificationRow**)&row);
    if (contentIntent != NULL) {
        String key;
        notification->GetKey(&key);
        AutoPtr<INotificationClicker> l;
        MakeClicker(contentIntent, key, isHeadsUp, (INotificationClicker**)&l);
        AutoPtr<IViewOnClickListener> listener = IViewOnClickListener::Probe(l);
        IView::Probe(row)->SetOnClickListener(listener);
    }
    else {
        IView::Probe(row)->SetOnClickListener(NULL);
    }
    row->SetStatusBarNotification(notification);
    row->NotifyContentUpdated();
    row->ResetHeight();
}

void BaseStatusBar::NotifyHeadsUpScreenOn(
    /* [in] */ Boolean screenOn)
{
    if (!screenOn) {
        ScheduleHeadsUpEscalation();
    }
}

Boolean BaseStatusBar::AlertAgain(
    /* [in] */ INotificationDataEntry* oldEntry,
    /* [in] */ INotification* newNotification)
{
    Boolean tmp = FALSE;
    Int32 flags = 0;
    return oldEntry == NULL || (oldEntry->HasInterrupted(&tmp), !tmp)
            || ((newNotification->GetFlags(&flags), flags) & INotification::FLAG_ONLY_ALERT_ONCE) == 0;
}

Boolean BaseStatusBar::ShouldInterrupt(
    /* [in] */ IStatusBarNotification* sbn)
{
    if (mNotificationData->ShouldFilterOut(sbn)) {
        if (DEBUG) {
            String key;
            sbn->GetKey(&key);
            Logger::D(TAG, "Skipping HUN check for %s since it's filtered out.", key.string());
        }
        return FALSE;
    }

    AutoPtr<INotification> notification;
    sbn->GetNotification((INotification**)&notification);
    // some predicates to make the Boolean logic legible
    Int32 defaults = 0;
    notification->GetDefaults(&defaults);
    AutoPtr<IUri> sound;
    AutoPtr<ArrayOf<Int64> > vibrate;
    Boolean isNoisy = (defaults & INotification::DEFAULT_SOUND) != 0
            || (defaults & INotification::DEFAULT_VIBRATE) != 0
            || (notification->GetSound((IUri**)&sound), sound.Get()) != NULL
            || (notification->GetVibrate((ArrayOf<Int64>**)&vibrate), vibrate.Get()) != NULL;

    Int32 score = 0;
    sbn->GetScore(&score);
    Boolean isHighPriority = score >= INTERRUPTION_THRESHOLD;

    AutoPtr<IPendingIntent> fullScreenIntent;
    notification->GetFullScreenIntent((IPendingIntent**)&fullScreenIntent);
    Boolean isFullscreen = fullScreenIntent.Get() != NULL;

    AutoPtr<ICharSequence> tickerText;
    notification->GetTickerText((ICharSequence**)&tickerText);
    Boolean hasTicker = mHeadsUpTicker && !TextUtils::IsEmpty(tickerText);

    AutoPtr<IBundle> extras;
    notification->GetExtras((IBundle**)&extras);
    Int32 value = 0;
    extras->GetInt32(INotification::EXTRA_AS_HEADS_UP, INotification::HEADS_UP_ALLOWED, &value);
    Boolean isAllowed = value != INotification::HEADS_UP_NEVER;

    Boolean tmp = FALSE;
    mAccessibilityManager->IsTouchExplorationEnabled(&tmp);
    Boolean accessibilityForcesLaunch = isFullscreen && tmp;

    AutoPtr<KeyguardTouchDelegate> keyguard = KeyguardTouchDelegate::GetInstance(mContext);
    Boolean interrupt = (isFullscreen || (isHighPriority && (isNoisy || hasTicker)))
            && isAllowed
            && !accessibilityForcesLaunch
            && (mPowerManager->IsScreenOn(&tmp), tmp)
            && (keyguard->IsShowingAndNotOccluded(&tmp), !tmp)
            && (keyguard->IsInputRestricted(&tmp), !tmp);

    if (mDreamManager != NULL) {
        mDreamManager->IsDreaming(&tmp);
        interrupt = interrupt && !tmp;
    }

    if (DEBUG) Logger::D(TAG, "interrupt: %d", interrupt);
    return interrupt;
}

ECode BaseStatusBar::InKeyguardRestrictedInputMode(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<KeyguardTouchDelegate> keyguard = KeyguardTouchDelegate::GetInstance(mContext);
    return keyguard->IsInputRestricted(result);
}

ECode BaseStatusBar::SetInteracting(
    /* [in] */ Int32 barWindow,
    /* [in] */ Boolean interacting)
{
    // hook for subclasses
    return NOERROR;
}

ECode BaseStatusBar::SetBouncerShowing(
    /* [in] */ Boolean bouncerShowing)
{
    mBouncerShowing = bouncerShowing;
    return NOERROR;
}

ECode BaseStatusBar::IsBouncerShowing(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mBouncerShowing;
    return NOERROR;
}

ECode BaseStatusBar::Destroy()
{
    if (mSearchPanelView != NULL) {
        mWindowManager->RemoveViewImmediate(IView::Probe(mSearchPanelView));
    }
    mContext->UnregisterReceiver(mBroadcastReceiver);
    // try {
    mNotificationListener->UnregisterAsSystemService();
    // } catch (RemoteException e) {
    //     // Ignore.
    // }
    return NOERROR;
}

AutoPtr<IPackageManager> BaseStatusBar::GetPackageManagerForUser(
    /* [in] */ Int32 userId)
{
    AutoPtr<IContext> contextForUser = mContext;
    // UserHandle defines special userId as negative values, e.g. USER_ALL
    if (userId >= 0) {
        // try {
        // Create a context for the correct user so if a package isn't installed
        // for user 0 we can still load information about the package.
        String pkgname;
        mContext->GetPackageName(&pkgname);
        AutoPtr<IUserHandle> uh;
        CUserHandle::New(userId, (IUserHandle**)&uh);
        contextForUser = NULL;
        mContext->CreatePackageContextAsUser(pkgname, IContext::CONTEXT_RESTRICTED,
            uh, (IContext**)&contextForUser);
        mContextForUser = contextForUser; // someone should hold contextForUser's reference
        // } catch (NameNotFoundException e) {
        //     // Shouldn't fail to find the package name for system ui.
        // }
    }
    AutoPtr<IPackageManager> pm;
    contextForUser->GetPackageManager((IPackageManager**)&pm);
    return pm;
}

ECode BaseStatusBar::LogNotificationExpansion(
    /* [in] */ const String& key,
    /* [in] */ Boolean userAction,
    /* [in] */ Boolean expanded)
{
    mBarService->OnNotificationExpansionChanged(key, userAction, expanded);
    return NOERROR;
}

} // namespace StatusBar
} // namespace SystemUI
} // namespace Droid
} // namespace Elastos
