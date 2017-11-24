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

#include "elastos/droid/systemui/volume/ZenModePanel.h"
#include "Elastos.Droid.Animation.h"
#include "Elastos.Droid.Provider.h"
#include "Elastos.CoreLibrary.Core.h"
#include "Elastos.CoreLibrary.Utility.h"
#include "elastos/droid/os/Looper.h"
#include "elastos/droid/systemui/volume/Interaction.h"
#include "elastos/droid/utility/MathUtils.h"
#include "elastos/droid/view/LayoutInflater.h"
#include "R.h"
#include <elastos/core/Math.h>
#include <elastos/core/StringUtils.h>
#include <elastos/core/CoreUtils.h>
#include <elastos/utility/Arrays.h>
#include <elastos/utility/Objects.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Animation::ITimeInterpolator;
using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::EIID_ISharedPreferencesOnSharedPreferenceChangeListener;
using Elastos::Droid::Content::IIntent;
using Elastos::Droid::Content::ISharedPreferencesEditor;
using Elastos::Droid::Content::Res::IResources;
using Elastos::Droid::Os::Looper;
using Elastos::Droid::Provider::ISettings;
using Elastos::Droid::Provider::ISettingsGlobal;
using Elastos::Droid::Service::Notification::CZenModeConfig;
using Elastos::Droid::Service::Notification::CZenModeConfigHelper;
using Elastos::Droid::Service::Notification::EIID_ICondition;
using Elastos::Droid::Service::Notification::IZenModeConfig;
using Elastos::Droid::Service::Notification::IZenModeConfigHelper;
using Elastos::Droid::SystemUI::StatusBar::Policy::EIID_IZenModeControllerCallback;
using Elastos::Droid::SystemUI::Volume::EIID_IInteractionCallback;
using Elastos::Droid::SystemUI::Volume::EIID_ISegmentedButtonsCallback;
using Elastos::Droid::Utility::MathUtils;
using Elastos::Droid::View::EIID_IViewOnClickListener;
using Elastos::Droid::View::IViewGroup;
using Elastos::Droid::View::IViewPropertyAnimator;
using Elastos::Droid::View::LayoutInflater;
using Elastos::Droid::View::Animation::CAnimationUtils;
using Elastos::Droid::View::Animation::IAnimationUtils;
using Elastos::Droid::Widget::EIID_ICompoundButtonOnCheckedChangeListener;
using Elastos::Droid::Widget::ICheckable;
using Elastos::Droid::Widget::IImageView;
using Elastos::Core::CArrayOf;
using Elastos::Core::CInteger32;
using Elastos::Core::CString;
using Elastos::Core::CSystem;
using Elastos::Core::Math;
using Elastos::Core::ICharSequence;
using Elastos::Core::IArrayOf;
using Elastos::Core::IInteger32;
using Elastos::Core::ISystem;
using Elastos::Core::CoreUtils;
using Elastos::Core::StringUtils;
using Elastos::Utility::Arrays;
using Elastos::Utility::Objects;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace SystemUI {
namespace Volume {

const String ZenModePanel::TAG("ZenModePanel");
const Boolean ZenModePanel::DEBUG = FALSE;//Logger::IsLoggable(TAG.string(), Logger::___DEBUG);

//====================================================================
// ZenModePanel::ZenCallback
//====================================================================

ZenModePanel::ZenCallback::ZenCallback(
    /* [in] */ ZenModePanel* host)
    : mHost(host)
{}

ECode ZenModePanel::ZenCallback::OnZenChanged(
    /* [in] */ Int32 zen)
{
    AutoPtr<IMessage> msg;
    mHost->mHandler->ObtainMessage(H::UPDATE_ZEN, zen, 0, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode ZenModePanel::ZenCallback::OnConditionsChanged(
    /* [in] */ ArrayOf<ICondition*>* conditions)
{
    Int32 length = conditions->GetLength();
    AutoPtr<IArrayOf> obj;
    CArrayOf::New(EIID_ICondition, length, (IArrayOf**)&obj);
    for (Int32 i = 0; i < length; i++) {
        obj->Set(i, (*conditions)[i]);
    }
    AutoPtr<IMessage> msg;
    mHost->mHandler->ObtainMessage(H::UPDATE_CONDITIONS, obj, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode ZenModePanel::ZenCallback::OnExitConditionChanged(
    /* [in] */ ICondition* exitCondition)
{
    AutoPtr<IMessage> msg;
    mHost->mHandler->ObtainMessage(H::EXIT_CONDITION_CHANGED, exitCondition, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

//====================================================================
// ZenModePanel::H
//====================================================================

const Int32 ZenModePanel::H::UPDATE_CONDITIONS;
const Int32 ZenModePanel::H::EXIT_CONDITION_CHANGED;
const Int32 ZenModePanel::H::UPDATE_ZEN;

ZenModePanel::H::H(
    /* [in] */ ZenModePanel* host)
    : Handler(Looper::GetMainLooper())
    , mHost(host)
{}

ECode ZenModePanel::H::HandleMessage(
    /* [in] */ IMessage* msg)
{
    Int32 what;
    msg->GetWhat(&what);
    if (what == UPDATE_CONDITIONS) {
        AutoPtr<IInterface> obj;
        msg->GetObj((IInterface**)&obj);
        AutoPtr<IArrayOf> a = IArrayOf::Probe(obj);
        Int32 length;
        a->GetLength(&length);
        AutoPtr<ArrayOf<ICondition*> > aa = ArrayOf<ICondition*>::Alloc(length);
        for (Int32 i = 0; i < length; i++) {
            AutoPtr<IInterface> e;
            a->Get(i, (IInterface**)&e);
            AutoPtr<ICondition> c = ICondition::Probe(e);
            aa->Set(i, c);
        }
        mHost->HandleUpdateConditions(aa);
    }
    else if (what == EXIT_CONDITION_CHANGED) {
        AutoPtr<IInterface> obj;
        msg->GetObj((IInterface**)&obj);
        AutoPtr<ICondition> c = ICondition::Probe(obj);
        mHost->HandleExitConditionChanged(c);
    }
    else if (what == UPDATE_ZEN) {
        Int32 arg1;
        msg->GetArg1(&arg1);
        mHost->HandleUpdateZen(arg1);
    }
    return NOERROR;
}

//====================================================================
// ZenModePanel::Prefs
//====================================================================

const String ZenModePanel::Prefs::KEY_MINUTE_INDEX("minuteIndex");
const String ZenModePanel::Prefs::KEY_NONE_SELECTED("noneSelected");

ZenModePanel::Prefs::Prefs(
    /* [in] */ ZenModePanel* host)
    : mMinuteIndex(0)
    , mNoneSelected(0)
    , mHost(host)
{
    AutoPtr<IResources> resources;
    mHost->mContext->GetResources((IResources**)&resources);
    resources->GetInteger(R::integer::zen_mode_alarm_warning_threshold, &mNoneDangerousThreshold);
    AutoPtr<ISharedPreferences> sp = GetPrefs();
    sp->RegisterOnSharedPreferenceChangeListener(this);
    UpdateMinuteIndex();
    UpdateNoneSelected();
}

CAR_INTERFACE_IMPL(ZenModePanel::Prefs, Object, ISharedPreferencesOnSharedPreferenceChangeListener)

ECode ZenModePanel::Prefs::IsNoneDangerous(
    /* [out] */ Boolean* isNoneDangerous)
{
    VALIDATE_NOT_NULL(isNoneDangerous)
    *isNoneDangerous = mNoneSelected < mNoneDangerousThreshold;
    return NOERROR;
}

ECode ZenModePanel::Prefs::TrackNoneSelected()
{
    mNoneSelected = ClampNoneSelected(mNoneSelected + 1);
    if (DEBUG) Logger::D(mHost->mTag, "Setting none selected: %d threshold=%d", mNoneSelected, mNoneDangerousThreshold);
    AutoPtr<ISharedPreferences> sp = GetPrefs();
    AutoPtr<ISharedPreferencesEditor> spe;
    sp->Edit((ISharedPreferencesEditor**)&spe);
    spe->PutInt32(KEY_NONE_SELECTED, mNoneSelected);
    spe->Apply();
    return NOERROR;
}

ECode ZenModePanel::Prefs::GetMinuteIndex(
    /* [out] */ Int32* index)
{
    VALIDATE_NOT_NULL(index)
    *index = mMinuteIndex;
    return NOERROR;
}

ECode ZenModePanel::Prefs::SetMinuteIndex(
    /* [in] */ Int32 minuteIndex)
{
    minuteIndex = ClampIndex(minuteIndex);
    if (minuteIndex == mMinuteIndex) return NOERROR;
    mMinuteIndex = ClampIndex(minuteIndex);
    if (DEBUG) Logger::D(mHost->mTag, "Setting favorite minute index: %d", mMinuteIndex);
    AutoPtr<ISharedPreferences> sp = GetPrefs();
    AutoPtr<ISharedPreferencesEditor> spe;
    sp->Edit((ISharedPreferencesEditor**)&spe);
    spe->PutInt32(KEY_MINUTE_INDEX, mNoneSelected);
    spe->Apply();
    return NOERROR;
}

ECode ZenModePanel::Prefs::OnSharedPreferenceChanged(
    /* [in] */ ISharedPreferences* prefs,
    /* [in] */ const String& key)
{
    UpdateMinuteIndex();
    UpdateNoneSelected();
    return NOERROR;
}

AutoPtr<ISharedPreferences> ZenModePanel::Prefs::GetPrefs()
{
    AutoPtr<ISharedPreferences> sp;
    mHost->mContext->GetSharedPreferences(
        /*ZenModePanel.class.getSimpleName()*/ String("ZenModePanel"), 0, (ISharedPreferences**)&sp);
    return sp;
}

void ZenModePanel::Prefs::UpdateMinuteIndex()
{
    AutoPtr<ISharedPreferences> sp = GetPrefs();
    Int32 i;
    sp->GetInt32(KEY_MINUTE_INDEX, DEFAULT_BUCKET_INDEX, &i);
    mMinuteIndex = ClampIndex(i);
    if (DEBUG) Logger::D(mHost->mTag, "Favorite minute index: %d", mMinuteIndex);
}

Int32 ZenModePanel::Prefs::ClampIndex(
    /* [in] */ Int32 index)
{
    return MathUtils::Constrain(index, -1, ZenModePanel::MINUTE_BUCKETS->GetLength() - 1);
}

void ZenModePanel::Prefs::UpdateNoneSelected()
{
    AutoPtr<ISharedPreferences> sp = GetPrefs();
    Int32 i;
    sp->GetInt32(KEY_NONE_SELECTED, 0, &i);
    mNoneSelected = ClampNoneSelected(i);
    if (DEBUG) Logger::D(mHost->mTag, "None selected: %d", mNoneSelected);
}

Int32 ZenModePanel::Prefs::ClampNoneSelected(
    /* [in] */ Int32 noneSelected)
{
    return MathUtils::Constrain(noneSelected, 0, Elastos::Core::Math::INT32_MAX_VALUE);
}

//====================================================================
// ZenModePanel::ZenButtonsCallback
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::ZenButtonsCallback, Object, \
    ISegmentedButtonsCallback, IInteractionCallback)

ZenModePanel::ZenButtonsCallback::ZenButtonsCallback(
    /* [in] */ ZenModePanel* host)
    : mHost(host)
{}

ECode ZenModePanel::ZenButtonsCallback::OnSelected(
    /* [in] */ IInterface* value)
{
    Boolean shown;
    mHost->mZenButtons->IsShown(&shown);
    if (value != NULL && shown) {
        if (DEBUG) Logger::D(mHost->mTag, "mZenButtonsCallback selected=%s", TO_CSTR(value));
        AutoPtr<IInteger32> v = IInteger32::Probe(value);
        Int32 i;
        v->GetValue(&i);
        mHost->mController->SetZen(i);
    }
    return NOERROR;
}

ECode ZenModePanel::ZenButtonsCallback::OnInteraction()
{
    mHost->FireInteraction();
    return NOERROR;
}

//====================================================================
// ZenModePanel::InteractionCallback
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::InteractionCallback, Object, IInteractionCallback)

ZenModePanel::InteractionCallback::InteractionCallback(
    /* [in] */ ZenModePanel* host)
    : mHost(host)
{}

ECode ZenModePanel::InteractionCallback::OnInteraction()
{
    mHost->FireInteraction();
    return NOERROR;
}

//====================================================================
// ZenModePanel::ZenSubheadCollapsedOnClickListener
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::ZenSubheadCollapsedOnClickListener, Object, IViewOnClickListener)

ZenModePanel::ZenSubheadCollapsedOnClickListener::ZenSubheadCollapsedOnClickListener(
    /* [in] */ ZenModePanel* host)
    : mHost(host)
{}

ECode ZenModePanel::ZenSubheadCollapsedOnClickListener::OnClick(
    /* [in] */ IView* v)
{
    mHost->SetExpanded(TRUE);
    return NOERROR;
}

//====================================================================
// ZenModePanel::MoreSettingsOnClickListener
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::MoreSettingsOnClickListener, Object, IViewOnClickListener)

ZenModePanel::MoreSettingsOnClickListener::MoreSettingsOnClickListener(
    /* [in] */ ZenModePanel* host)
    : mHost(host)
{}

ECode ZenModePanel::MoreSettingsOnClickListener::OnClick(
    /* [in] */ IView* v)
{
    mHost->FireMoreSettings();
    return NOERROR;
}

//====================================================================
// ZenModePanel::MyAnimatorListener
//====================================================================

ZenModePanel::MyAnimatorListener::MyAnimatorListener(
    /* [in] */ IView* noneButton)
    : mNoneButton(noneButton)
{}

ECode ZenModePanel::MyAnimatorListener::OnAnimationEnd(
    /* [in] */ IAnimator* animation)
{
    AutoPtr<IViewPropertyAnimator> animator;
    mNoneButton->Animate((IViewPropertyAnimator**)&animator);
    animator->ScaleX(1);
    animator->ScaleY(1);
    animator->SetListener(NULL);
    return NOERROR;
}

//====================================================================
// ZenModePanel::OnCheckedChangeListener
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::OnCheckedChangeListener, Object, ICompoundButtonOnCheckedChangeListener)

ZenModePanel::OnCheckedChangeListener::OnCheckedChangeListener(
    /* [in] */ ConditionTag* tag,
    /* [in] */ ZenModePanel* host)
    : mHost(host)
    , mTag(tag)
{}

ECode ZenModePanel::OnCheckedChangeListener::OnCheckedChanged(
    /* [in] */ ICompoundButton* buttonView,
    /* [in] */ Boolean isChecked)
{
    if (mHost->mExpanded && isChecked) {
        if (DEBUG) Logger::D(mHost->mTag, "onCheckedChanged %s", TO_CSTR(mTag->mCondition));
        Int32 N;
        IViewGroup::Probe(mHost->mZenConditions)->GetChildCount(&N);
        for (Int32 i = 0; i < N; i++) {
            AutoPtr<ConditionTag> childTag = mHost->GetConditionTagAt(i);
            if (childTag == mTag) continue;
            ICheckable::Probe(childTag->mRb)->SetChecked(FALSE);
        }
        mHost->Select(mTag->mCondition);
        mHost->AnnounceConditionSelection(mTag);
    }
    return NOERROR;
}

//====================================================================
// ZenModePanel::Button1OnClickListener
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::Button1OnClickListener, Object, IViewOnClickListener)

ZenModePanel::Button1OnClickListener::Button1OnClickListener(
    /* [in] */ IView* row,
    /* [in] */ ConditionTag* tag,
    /* [in] */ ZenModePanel* host)
    : mRow(row)
    , mTag(tag)
    , mHost(host)
{}

ECode ZenModePanel::Button1OnClickListener::OnClick(
    /* [in] */ IView* v)
{
    mHost->OnClickTimeButton(mRow, mTag, FALSE /*down*/);
    return NOERROR;
}

//====================================================================
// ZenModePanel::Button2OnClickListener
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::Button2OnClickListener, Object, IViewOnClickListener)

ZenModePanel::Button2OnClickListener::Button2OnClickListener(
    /* [in] */ IView* row,
    /* [in] */ ConditionTag* tag,
    /* [in] */ ZenModePanel* host)
    : mRow(row)
    , mTag(tag)
    , mHost(host)
{}

ECode ZenModePanel::Button2OnClickListener::OnClick(
    /* [in] */ IView* v)
{
    mHost->OnClickTimeButton(mRow, mTag, TRUE /*down*/);
    return NOERROR;
}

//====================================================================
// ZenModePanel::TitleOnClickListener
//====================================================================

CAR_INTERFACE_IMPL(ZenModePanel::TitleOnClickListener, Object, IViewOnClickListener)

ZenModePanel::TitleOnClickListener::TitleOnClickListener(
    /* [in] */ ConditionTag* tag,
    /* [in] */ ZenModePanel* host)
    : mHost(host)
    , mTag(tag)
{}

ECode ZenModePanel::TitleOnClickListener::OnClick(
    /* [in] */ IView* v)
{
    return ICheckable::Probe(mTag->mRb)->SetChecked(TRUE);
}

//====================================================================
// ZenModePanel
//====================================================================
const Int32 ZenModePanel::SECONDS_MS;
const Int32 ZenModePanel::MINUTES_MS;

// static AutoPtr<ArrayOf<Int32> > InitMINUTE_BUCKETS_dbg()
// {
//     AutoPtr<ArrayOf<Int32> > a = ArrayOf<Int32>::Alloc(12);
//     (*a)[0] = 0;
//     (*a)[1] = 1;
//     (*a)[2] = 2;
//     (*a)[3] = 5;
//     (*a)[4] = 15;
//     (*a)[5] = 30;
//     (*a)[6] = 45;
//     (*a)[7] = 60;
//     (*a)[8] = 120;
//     (*a)[9] = 180;
//     (*a)[10] = 240;
//     (*a)[11] = 480;
//     return a;
// }

static AutoPtr<ArrayOf<Int32> > InitMINUTE_BUCKETS()
{
    AutoPtr<IZenModeConfigHelper> h;
    CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&h);
    AutoPtr<ArrayOf<Int32> > a;
    h->GetMINUTE_BUCKETS((ArrayOf<Int32>**)&a);
    assert(a != NULL);
    return a;
}

static Int32 InitDEFAULT_BUCKET_INDEX()
{
    assert(ZenModePanel::MINUTE_BUCKETS != NULL);
    Int32 index;
    Arrays::BinarySearch(ZenModePanel::MINUTE_BUCKETS, 60, &index);
    return index;
}

static AutoPtr<IIntent> InitZEN_SETTINGS()
{
    AutoPtr<IIntent> i;
    CIntent::New(ISettings::ACTION_ZEN_MODE_SETTINGS, (IIntent**)&i);
    return i;
}

const AutoPtr<ArrayOf<Int32> > ZenModePanel::MINUTE_BUCKETS = InitMINUTE_BUCKETS();
const Int32 ZenModePanel::MIN_BUCKET_MINUTES = (*MINUTE_BUCKETS)[0];
const Int32 ZenModePanel::MAX_BUCKET_MINUTES = (*MINUTE_BUCKETS)[MINUTE_BUCKETS->GetLength() - 1];
const Int32 ZenModePanel::DEFAULT_BUCKET_INDEX = InitDEFAULT_BUCKET_INDEX();

const AutoPtr<IIntent> ZenModePanel::ZEN_SETTINGS = InitZEN_SETTINGS();

const Int32 ZenModePanel::FOREVER_CONDITION_INDEX = 0;
const Int32 ZenModePanel::TIME_CONDITION_INDEX = 1;
const Int32 ZenModePanel::FIRST_CONDITION_INDEX = 2;
const Float ZenModePanel::SILENT_HINT_PULSE_SCALE = 1.1f;

CAR_INTERFACE_IMPL(ZenModePanel, LinearLayout, IZenModePanel)

ZenModePanel::ZenModePanel()
    : mBucketIndex(-1)
    , mExpanded(FALSE)
    , mHidden(FALSE)
    , mSessionZen(0)
    , mAttachedZen(0)
{}

ECode ZenModePanel::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs)
{
    LinearLayout::constructor(context, attrs);

    mHandler = new H(this);
    mPrefs = new Prefs(this);
    AutoPtr<IContext> ctx;
    mContext->GetApplicationContext((IContext**)&ctx);
    LayoutInflater::From(ctx, (ILayoutInflater**)&mInflater);

    AutoPtr<IAnimationUtils> au;
    CAnimationUtils::AcquireSingleton((IAnimationUtils**)&au);
    au->LoadInterpolator(mContext, Elastos::Droid::R::interpolator::fast_out_slow_in,
        (IInterpolator**)&mFastOutSlowInInterpolator);

    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    res->GetColor(R::color::system_warning_color, &mSubheadWarningColor);
    res->GetColor(R::color::qs_subhead, &mSubheadColor);

    mZenToast = new ZenToast();
    mZenToast->constructor(mContext);
    if (DEBUG) Logger::D(mTag, "new ZenModePanel");

    AutoPtr<ISystem> sys;
    CSystem::AcquireSingleton((ISystem**)&sys);
    Int32 hash;
    sys->IdentityHashCode((IZenModePanel*)this, &hash);
    mTag = TAG + "/" + StringUtils::ToHexString(hash);

    mZenCallback = new ZenCallback(this);
    mZenButtonsCallback = new ZenButtonsCallback(this);
    mInteractionCallback = new InteractionCallback(this);
    return NOERROR;
}

ECode ZenModePanel::OnFinishInflate()
{
    LinearLayout::OnFinishInflate();

    AutoPtr<IView> view;
    FindViewById(R::id::zen_buttons, (IView**)&view);
    mZenButtons = (SegmentedButtons*)view.Get();
    AutoPtr<IInteger32> i;
    CInteger32::New(ISettingsGlobal::ZEN_MODE_NO_INTERRUPTIONS, (IInteger32**)&i);
    mZenButtons->AddButton(R::string::interruption_level_none, i);

    AutoPtr<IInteger32> i2;
    CInteger32::New(ISettingsGlobal::ZEN_MODE_IMPORTANT_INTERRUPTIONS, (IInteger32**)&i2);
    mZenButtons->AddButton(R::string::interruption_level_priority, i2);

    AutoPtr<IInteger32> i3;
    CInteger32::New(ISettingsGlobal::ZEN_MODE_OFF, (IInteger32**)&i3);
    mZenButtons->AddButton(R::string::interruption_level_all, i3);
    mZenButtons->SetCallback(mZenButtonsCallback);

    FindViewById(R::id::zen_subhead, (IView**)&mZenSubhead);

    AutoPtr<IView> view2;
    FindViewById(R::id::zen_subhead_collapsed, (IView**)&view2);
    mZenSubheadCollapsed = ITextView::Probe(view2);
    AutoPtr<ZenSubheadCollapsedOnClickListener> vo = new ZenSubheadCollapsedOnClickListener(this);
    view2->SetOnClickListener(vo);
    Interaction::Register(view2, mInteractionCallback);

    AutoPtr<IView> view3;
    FindViewById(R::id::zen_subhead_expanded, (IView**)&view3);
    mZenSubheadExpanded = ITextView::Probe(view3);
    Interaction::Register(view3, mInteractionCallback);

    FindViewById(R::id::zen_more_settings, (IView**)&mMoreSettings);
    AutoPtr<MoreSettingsOnClickListener> vo2 = new MoreSettingsOnClickListener(this);
    mMoreSettings->SetOnClickListener(vo2);
    Interaction::Register(mMoreSettings, mInteractionCallback);

    AutoPtr<IView> view4;
    FindViewById(R::id::zen_conditions, (IView**)&view4);
    mZenConditions = ILinearLayout::Probe(view4);
    return NOERROR;
}

ECode ZenModePanel::OnAttachedToWindow()
{
    LinearLayout::OnAttachedToWindow();
    if (DEBUG) Logger::D(mTag, "onAttachedToWindow");
    mZenToast->Hide();
    mAttachedZen = GetSelectedZen(-1);
    mSessionZen = mAttachedZen;
    mSessionExitCondition = Copy(mExitCondition);
    RefreshExitConditionText();
    UpdateWidgets();
    return NOERROR;
}

ECode ZenModePanel::OnDetachedFromWindow()
{
    LinearLayout::OnDetachedFromWindow();
    if (DEBUG) Logger::D(mTag, "OnDetachedFromWindow");
    CheckForAttachedZenChange();
    mAttachedZen = -1;
    mSessionZen = -1;
    mSessionExitCondition = NULL;
    SetExpanded(FALSE);
    return NOERROR;
}

ECode ZenModePanel::SetHidden(
    /* [in] */ Boolean hidden)
{
    if (mHidden == hidden) return NOERROR;
    mHidden = hidden;
    UpdateWidgets();
    return NOERROR;
}

void ZenModePanel::CheckForAttachedZenChange()
{
    const Int32 selectedZen = GetSelectedZen(-1);
    if (DEBUG) Logger::D(mTag, "SelectedZen=%d", selectedZen);
    if (selectedZen != mAttachedZen) {
        if (DEBUG) Logger::D(mTag, "AttachedZen: %d -> %d", mAttachedZen, selectedZen);
        if (selectedZen == ISettingsGlobal::ZEN_MODE_NO_INTERRUPTIONS) {
            mPrefs->TrackNoneSelected();
        }
        if (selectedZen == ISettingsGlobal::ZEN_MODE_NO_INTERRUPTIONS
                || selectedZen == ISettingsGlobal::ZEN_MODE_IMPORTANT_INTERRUPTIONS) {
            mZenToast->Show(selectedZen);
        }
    }
}

void ZenModePanel::SetExpanded(
    /* [in] */ Boolean expanded)
{
    if (expanded == mExpanded) return;
    mExpanded = expanded;
    UpdateWidgets();
    SetRequestingConditions(mExpanded);
    FireExpanded();
}

void ZenModePanel::SetRequestingConditions(
    /* [in] */ Boolean requesting)
{
    if (mRequestingConditions == requesting) return;
    if (DEBUG) Logger::D(mTag, "SetRequestingConditions %s", requesting ? "TRUE" : "FALSE");
    mRequestingConditions = requesting;
    if (mController != NULL) {
        mController->RequestConditions(mRequestingConditions);
    }
    if (mRequestingConditions) {
        mTimeCondition = ParseExistingTimeCondition(mExitCondition);
        if (mTimeCondition != NULL) {
            mBucketIndex = -1;
        }
        else {
            mBucketIndex = DEFAULT_BUCKET_INDEX;
            AutoPtr<IZenModeConfigHelper> zmc;
            CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
            zmc->ToTimeCondition((*MINUTE_BUCKETS)[mBucketIndex], (ICondition**)&mTimeCondition);
        }
        if (DEBUG) Logger::D(mTag, "Initial bucket index: %d, mTimeCondition:%s", mBucketIndex, TO_CSTR(mTimeCondition));
        mConditions = NULL; // reset conditions
        HandleUpdateConditions();
    }
    else {
        IViewGroup::Probe(mZenConditions)->RemoveAllViews();
    }
}

ECode ZenModePanel::Init(
    /* [in] */ IZenModeController* controller)
{
    mController = controller;
    AutoPtr<ICondition> con;
    mController->GetExitCondition((ICondition**)&con);
    SetExitCondition(con);
    RefreshExitConditionText();
    mSessionZen = GetSelectedZen(-1);
    Int32 zen;
    mController->GetZen(&zen);
    HandleUpdateZen(zen);
    if (DEBUG) Logger::D(mTag, "init mExitCondition=%s", TO_CSTR(mExitCondition));
    IViewGroup::Probe(mZenConditions)->RemoveAllViews();
    mController->AddCallback(mZenCallback);
    return NOERROR;
}

ECode ZenModePanel::UpdateLocale()
{
    return mZenButtons->UpdateLocale();
}

void ZenModePanel::SetExitCondition(
    /* [in] */ ICondition* exitCondition)
{
    if (SameConditionId(mExitCondition, exitCondition)) return;
    mExitCondition = exitCondition;
    RefreshExitConditionText();
    UpdateWidgets();
}

AutoPtr<IUri> ZenModePanel::GetConditionId(
    /* [in] */ ICondition* condition)
{
    AutoPtr<IUri> id;
    if (condition != NULL) {
        condition->GetId((IUri**)&id);
    }
    return id;
}

Boolean ZenModePanel::SameConditionId(
    /* [in] */ ICondition* lhs,
    /* [in] */ ICondition* rhs)
{
    if (lhs == NULL) {
        return rhs == NULL;
    }

    if (rhs != NULL) {
        AutoPtr<IUri> id1, id2;
        lhs->GetId((IUri**)&id1);
        rhs->GetId((IUri**)&id2);
        return Object::Equals(id1, id2);
    }
    return FALSE;
}

AutoPtr<ICondition> ZenModePanel::Copy(
    /* [in] */ ICondition* condition)
{
    AutoPtr<ICondition> c;
    if (condition != NULL) {
        condition->Copy((ICondition**)&c);
    }
    return c;
}

void ZenModePanel::RefreshExitConditionText()
{
    String forever;
    mContext->GetString(Elastos::Droid::R::string::zen_mode_forever, &forever);

    if (mExitCondition == NULL) {
        mExitConditionText = forever;
    }
    else {
        AutoPtr<IZenModeConfigHelper> zmc;
        CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
        AutoPtr<IUri> id;
        mExitCondition->GetId((IUri**)&id);
        Boolean isValid;
        zmc->IsValidCountdownConditionId(id, &isValid);
        if (isValid) {
            AutoPtr<ICondition> condition = ParseExistingTimeCondition(mExitCondition);
            String summary;
            condition->GetSummary(&summary);
            mExitConditionText = condition != NULL ? summary : forever;
        }
        else {
            String summary;
            mExitCondition->GetSummary(&summary);
            mExitConditionText = summary;
        }
    }
}

ECode ZenModePanel::SetCallback(
    /* [in] */ IZenModePanelCallback* callback)
{
    mCallback = callback;
    return NOERROR;
}

ECode ZenModePanel::ShowSilentHint()
{
    if (DEBUG) Logger::D(mTag, "showSilentHint");
    if (mZenButtons == NULL) return NOERROR;
    Int32 count;
    mZenButtons->GetChildCount(&count);
    if (count == 0) return NOERROR;
    AutoPtr<IView> noneButton;
    mZenButtons->GetChildAt(0, (IView**)&noneButton);
    Float sx;
    noneButton->GetScaleX(&sx);
    if (sx != 1) return NOERROR;  // already running
    AutoPtr<IViewPropertyAnimator> animator;
    noneButton->Animate((IViewPropertyAnimator**)&animator);
    animator->Cancel();
    animator->ScaleX(SILENT_HINT_PULSE_SCALE);
    animator->ScaleY(SILENT_HINT_PULSE_SCALE);
    animator->SetInterpolator(ITimeInterpolator::Probe(mFastOutSlowInInterpolator));
    AutoPtr<MyAnimatorListener> al = new MyAnimatorListener(noneButton);
    animator->SetListener(al);
    return NOERROR;
}

void ZenModePanel::HandleUpdateZen(
    /* [in] */ Int32 zen)
{
    if (mSessionZen != -1 && mSessionZen != zen) {
        SetExpanded(zen != ISettingsGlobal::ZEN_MODE_OFF);
        mSessionZen = zen;
    }
    AutoPtr<IInteger32> i;
    CInteger32::New(zen, (IInteger32**)&i);
    mZenButtons->SetSelectedValue(i);
    UpdateWidgets();
}

Int32 ZenModePanel::GetSelectedZen(
    /* [in] */ Int32 defValue)
{
    AutoPtr<IInterface> zen;
    mZenButtons->GetSelectedValue((IInterface**)&zen);
    if (zen != NULL) {
        AutoPtr<IInteger32> i = IInteger32::Probe(zen);
        Int32 value;
        i->GetValue(&value);
        return value;
    }
    return defValue;
}

void ZenModePanel::UpdateWidgets()
{
    Int32 zen = GetSelectedZen(ISettingsGlobal::ZEN_MODE_OFF);
    const Boolean zenOff = zen == ISettingsGlobal::ZEN_MODE_OFF;
    const Boolean zenImportant = zen == ISettingsGlobal::ZEN_MODE_IMPORTANT_INTERRUPTIONS;
    const Boolean zenNone = zen == ISettingsGlobal::ZEN_MODE_NO_INTERRUPTIONS;
    const Boolean expanded = !mHidden && mExpanded;

    mZenButtons->SetVisibility(mHidden ? GONE : VISIBLE);
    mZenSubhead->SetVisibility(!mHidden && !zenOff ? VISIBLE : GONE);
    IView::Probe(mZenSubheadExpanded)->SetVisibility(expanded ? VISIBLE : GONE);
    IView::Probe(mZenSubheadCollapsed)->SetVisibility(!expanded ? VISIBLE : GONE);
    mMoreSettings->SetVisibility(zenImportant && expanded ? VISIBLE : GONE);
    IView::Probe(mZenConditions)->SetVisibility(!zenOff && expanded ? VISIBLE : GONE);

    AutoPtr<ICharSequence> cs;
    CString::New(mExitConditionText, (ICharSequence**)&cs);
    if (zenNone) {
        mZenSubheadExpanded->SetText(R::string::zen_no_interruptions_with_warning);
        mZenSubheadCollapsed->SetText(cs);
    }
    else if (zenImportant) {
        mZenSubheadExpanded->SetText(R::string::zen_important_interruptions);
        mZenSubheadCollapsed->SetText(cs);
    }
    Boolean isNoneDangerous;
    mPrefs->IsNoneDangerous(&isNoneDangerous);
    mZenSubheadExpanded->SetTextColor(zenNone
        && isNoneDangerous ? mSubheadWarningColor : mSubheadColor);
}

AutoPtr<ICondition> ZenModePanel::ParseExistingTimeCondition(
    /* [in] */ ICondition* condition)
{
    if (condition == NULL) return NULL;
    AutoPtr<IZenModeConfigHelper> zmc;
    CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
    AutoPtr<IUri> id;
    condition->GetId((IUri**)&id);
    Int64 time;
    zmc->TryParseCountdownConditionId(id, &time);
    if (time == 0L) return NULL;
    AutoPtr<ISystem> sys;
    CSystem::AcquireSingleton((ISystem**)&sys);
    Int64 span;
    sys->GetCurrentTimeMillis(&span);
    span = time - span;
    if (span <= 0 || span > MAX_BUCKET_MINUTES * MINUTES_MS) return NULL;
    AutoPtr<ICondition> c;
    zmc->ToTimeCondition(time,
        Elastos::Core::Math::Round(span / (Float) MINUTES_MS), (ICondition**)&c);
    return c;
}

void ZenModePanel::HandleUpdateConditions(
    /* [in] */ ArrayOf<ICondition*>* conditions)
{
    mConditions = conditions;
    HandleUpdateConditions();
}

void ZenModePanel::HandleUpdateConditions()
{
    const Int32 conditionCount = mConditions == NULL ? 0 : mConditions->GetLength();
    if (DEBUG) Logger::D(mTag, "HandleUpdateConditions conditionCount=%d", conditionCount);

    IViewGroup* zenConditions = IViewGroup::Probe(mZenConditions);
    Int32 count;
    zenConditions->GetChildCount(&count);
    for (Int32 i = count - 1; i >= FIRST_CONDITION_INDEX; i--) {
        zenConditions->RemoveViewAt(i);
    }
    // forever
    AutoPtr<IView> v;
    zenConditions->GetChildAt(FOREVER_CONDITION_INDEX, (IView**)&v);
    Bind(NULL, v);
    // countdown
    AutoPtr<IView> v2;
    zenConditions->GetChildAt(TIME_CONDITION_INDEX, (IView**)&v2);
    Bind(mTimeCondition, v2);
    // provider conditions
    Boolean foundDowntime = FALSE;
    for (Int32 i = 0; i < conditionCount; i++) {
        AutoPtr<IView> v3;
        zenConditions->GetChildAt(FIRST_CONDITION_INDEX + i, (IView**)&v3);
        Bind((*mConditions)[i], v3);
        foundDowntime |= IsDowntime((*mConditions)[i]);
    }
    // ensure downtime exists, if active
    if (IsDowntime(mSessionExitCondition) && !foundDowntime) {
        Bind(mSessionExitCondition, NULL);
    }
    // ensure something is selected
    CheckForDefault();
}

Boolean ZenModePanel::IsDowntime(
    /* [in] */ ICondition* c)
{
    AutoPtr<IZenModeConfigHelper> zmc;
    CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
    Boolean isDowntime;
    zmc->IsValidDowntimeConditionId(GetConditionId(c), &isDowntime);
    return isDowntime;
}

AutoPtr<ZenModePanel::ConditionTag> ZenModePanel::GetConditionTagAt(
    /* [in] */ Int32 index)
{
    AutoPtr<IView> v;
    IViewGroup::Probe(mZenConditions)->GetChildAt(index, (IView**)&v);
    AutoPtr<IInterface> tag;
    v->GetTag((IInterface**)&tag);
    AutoPtr<ConditionTag> ct = (ConditionTag*)(IObject::Probe(tag));
    return ct;
}

void ZenModePanel::CheckForDefault()
{
    // are we left without anything selected?  if so, set a default
    Int32 count;
    IViewGroup::Probe(mZenConditions)->GetChildCount(&count);
    for (Int32 i = 0; i < count; i++) {
        AutoPtr<ConditionTag> ct = GetConditionTagAt(i);
        Boolean isChecked;
        ICheckable::Probe(ct->mRb)->IsChecked(&isChecked);
        if (isChecked) {
            if (DEBUG) Logger::D(mTag, "Not selecting a default, checked=%s",
                TO_CSTR(ct->mCondition));
            return;
        }
    }
    if (DEBUG) Logger::D(mTag, "Selecting a default");
    Int32 favoriteIndex;
    mPrefs->GetMinuteIndex(&favoriteIndex);
    if (favoriteIndex == -1) {
        AutoPtr<ConditionTag> ct = GetConditionTagAt(FOREVER_CONDITION_INDEX);
        ICheckable::Probe(ct->mRb)->SetChecked(TRUE);
    }
    else {
        AutoPtr<IZenModeConfigHelper> zmc;
        CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
        zmc->ToTimeCondition((*MINUTE_BUCKETS)[favoriteIndex], (ICondition**)&mTimeCondition);
        mBucketIndex = favoriteIndex;
        AutoPtr<IView> v;
        IViewGroup::Probe(mZenConditions)->GetChildAt(TIME_CONDITION_INDEX, (IView**)&v);
        Bind(mTimeCondition, v);
        AutoPtr<ConditionTag> ct = GetConditionTagAt(TIME_CONDITION_INDEX);
        ICheckable::Probe(ct->mRb)->SetChecked(TRUE);
    }
}

void ZenModePanel::HandleExitConditionChanged(
    /* [in] */ ICondition* exitCondition)
{
    SetExitCondition(exitCondition);
    if (DEBUG) Logger::D(mTag, "HandleExitConditionChanged %s", TO_CSTR(mExitCondition));
    Int32 N;
    IViewGroup::Probe(mZenConditions)->GetChildCount(&N);
    for (Int32 i = 0; i < N; i++) {
        const AutoPtr<ConditionTag> tag = GetConditionTagAt(i);
        ICheckable::Probe(tag->mRb)->SetChecked(
            SameConditionId(tag->mCondition, mExitCondition));
    }
}

void ZenModePanel::Bind(
    /* [in] */ ICondition* condition,
    /* [in] */ IView* convertView)
{
    Int32 state;
    Boolean enabled = condition == NULL || ((condition->GetState(&state), state) == ICondition::STATE_TRUE);
    AutoPtr<IView> row;
    if (convertView == NULL) {
        mInflater->Inflate(R::layout::zen_mode_condition, this, FALSE, (IView**)&row);
        if (DEBUG) Logger::D(mTag, "Adding new condition view for: %s, row: %s", TO_CSTR(condition), TO_CSTR(row));
        IViewGroup::Probe(mZenConditions)->AddView(row);
    }
    else {
        row = convertView;
    }

    AutoPtr<IInterface> obj;
    row->GetTag((IInterface**)&obj);
    AutoPtr<ConditionTag> tag;
    if (obj != NULL) {
        tag = (ConditionTag*)IObject::Probe(obj);
    }
    else {
        tag = new ConditionTag();
    }
    row->SetTag((IObject*)tag);
    if (tag->mRb == NULL) {
        AutoPtr<IView> v;
        row->FindViewById(Elastos::Droid::R::id::checkbox, (IView**)&v);
        tag->mRb = IRadioButton::Probe(v);
    }
    tag->mCondition = condition;
    IView::Probe(tag->mRb)->SetEnabled(enabled);
    if (SameConditionId(mSessionExitCondition, tag->mCondition)) {
        ICheckable::Probe(tag->mRb)->SetChecked(TRUE);
    }
    AutoPtr<OnCheckedChangeListener> occl = new OnCheckedChangeListener(tag, this);
    ICompoundButton::Probe(tag->mRb)->SetOnCheckedChangeListener(occl);

    AutoPtr<IView> title = IView::Probe(tag->mTitle);
    if (title == NULL) {
        row->FindViewById(Elastos::Droid::R::id::title, (IView**)&title);
        tag->mTitle = ITextView::Probe(title);
        assert(tag->mTitle != NULL);
    }

    if (condition == NULL) {
        String s;
        mContext->GetString(Elastos::Droid::R::string::zen_mode_forever, &s);
        AutoPtr<ICharSequence> cs = CoreUtils::Convert(s);
        tag->mTitle->SetText(cs);
    }
    else {
        String s;
        condition->GetSummary(&s);
        AutoPtr<ICharSequence> cs = CoreUtils::Convert(s);
        tag->mTitle->SetText(cs);
    }
    title->SetEnabled(enabled);
    title->SetAlpha(enabled ? 1 : .4f);

    AutoPtr<IView> v1;
    row->FindViewById(Elastos::Droid::R::id::button1, (IView**)&v1);
    const AutoPtr<IImageView> button1 = IImageView::Probe(v1);
    AutoPtr<Button1OnClickListener> ocl1 = new Button1OnClickListener(row, tag, this);
    v1->SetOnClickListener(ocl1);

    AutoPtr<IView> v2;
    row->FindViewById(Elastos::Droid::R::id::button2, (IView**)&v2);
    const AutoPtr<IImageView> button2 = IImageView::Probe(v2);
    AutoPtr<Button2OnClickListener> ocl2 = new Button2OnClickListener(row, tag, this);
    v2->SetOnClickListener(ocl2);

    AutoPtr<TitleOnClickListener> ocl3 = new TitleOnClickListener(tag, this);
    title->SetOnClickListener(ocl3);

    AutoPtr<IZenModeConfigHelper> zmc;
    CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
    Int64 time;
    zmc->TryParseCountdownConditionId(GetConditionId(tag->mCondition), &time);

    if (time > 0) {
        if (mBucketIndex > -1) {
            v1->SetEnabled(mBucketIndex > 0);
            v2->SetEnabled(mBucketIndex < MINUTE_BUCKETS->GetLength() - 1);
        }
        else {
            AutoPtr<ISystem> sys;
            CSystem::AcquireSingleton((ISystem**)&sys);
            Int64 span;
            sys->GetCurrentTimeMillis(&span);
            span = time - span;
            v1->SetEnabled(span > MIN_BUCKET_MINUTES * MINUTES_MS);
            AutoPtr<ICondition> maxCondition;
            zmc->ToTimeCondition(MAX_BUCKET_MINUTES, (ICondition**)&maxCondition);
            String s1, s2;
            condition->GetSummary(&s1);
            maxCondition->GetSummary(&s2);
            v2->SetEnabled(!s1.Equals(s2));
        }

        Boolean b1, b2;
        v1->IsEnabled(&b1);
        v2->IsEnabled(&b2);
        v1->SetAlpha(b1 ? 1.0f : .5f);
        v2->SetAlpha(b2 ? 1.0f : .5f);
    }
    else {
        v1->SetVisibility(IView::GONE);
        v2->SetVisibility(IView::GONE);
    }
    // wire up interaction callbacks for newly-added condition rows
    if (convertView == NULL) {
        Interaction::Register(IView::Probe(tag->mRb), mInteractionCallback);
        Interaction::Register(title, mInteractionCallback);
        Interaction::Register(v1, mInteractionCallback);
        Interaction::Register(v2, mInteractionCallback);
    }
}

void ZenModePanel::AnnounceConditionSelection(
    /* [in] */ ConditionTag* tag)
{
    const Int32 zen = GetSelectedZen(ISettingsGlobal::ZEN_MODE_OFF);
    String modeText;
    switch(zen) {
        case ISettingsGlobal::ZEN_MODE_IMPORTANT_INTERRUPTIONS:
            mContext->GetString(R::string::zen_important_interruptions, &modeText);
            break;
        case ISettingsGlobal::ZEN_MODE_NO_INTERRUPTIONS:
            mContext->GetString(R::string::zen_no_interruptions, &modeText);
            break;
         default:
            return;
    }
    AutoPtr<ICharSequence> cs, cs2;
    tag->mTitle->GetText((ICharSequence**)&cs);
    CString::New(modeText, (ICharSequence**)&cs2);
    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, cs2);
    args->Set(1, cs);
    String s;
    mContext->GetString(R::string::zen_mode_and_condition, args, &s);
    AutoPtr<ICharSequence> cs3;
    CString::New(s, (ICharSequence**)&cs3);
    AnnounceForAccessibility(cs3);
}

void ZenModePanel::OnClickTimeButton(
    /* [in] */ IView* row,
    /* [in] */ ConditionTag* tag,
    /* [in] */ Boolean up)
{
    AutoPtr<ICondition> newCondition;
    const Int32 N = MINUTE_BUCKETS->GetLength();
    AutoPtr<IZenModeConfigHelper> zmc;
    CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
    if (mBucketIndex == -1) {
        // not on a known index, search for the next or prev bucket by time
        const AutoPtr<IUri> conditionId = GetConditionId(tag->mCondition);
        Int64 time;
        zmc->TryParseCountdownConditionId(conditionId, &time);
        AutoPtr<ISystem> sys;
        CSystem::AcquireSingleton((ISystem**)&sys);
        Int64 now;
        sys->GetCurrentTimeMillis(&now);
        for (Int32 i = 0; i < N; i++) {
            Int32 j = up ? i : N - 1 - i;
            const Int32 bucketMinutes = (*MINUTE_BUCKETS)[j];
            const Int64 bucketTime = now + bucketMinutes * MINUTES_MS;
            if ((up && bucketTime > time) || (!up && bucketTime < time)) {
                mBucketIndex = j;
                zmc->ToTimeCondition(bucketTime, bucketMinutes, (ICondition**)&newCondition);
                break;
            }
        }
        if (newCondition == NULL) {
            mBucketIndex = DEFAULT_BUCKET_INDEX;
            zmc->ToTimeCondition((*MINUTE_BUCKETS)[mBucketIndex], (ICondition**)&newCondition);
        }
    }
    else {
        // on a known index, simply increment or decrement
        mBucketIndex = Elastos::Core::Math::Max(0, Elastos::Core::Math::Min(N - 1, mBucketIndex + (up ? 1 : -1)));
        zmc->ToTimeCondition((*MINUTE_BUCKETS)[mBucketIndex], (ICondition**)&newCondition);
    }
    mTimeCondition = newCondition;
    Bind(mTimeCondition, row);
    ICheckable::Probe(tag->mRb)->SetChecked(TRUE);
    Select(mTimeCondition);
    AnnounceConditionSelection(tag);
}

void ZenModePanel::Select(
    /* [in] */ ICondition* condition)
{
    if (DEBUG) Logger::D(mTag, "select %s, mPrefs:%s", TO_CSTR(condition), TO_CSTR(mPrefs));
    if (mController != NULL) {
        mController->SetExitCondition(condition);
    }
    SetExitCondition(condition);

    if (condition == NULL) {
        mPrefs->SetMinuteIndex(-1);
    }
    else {
        AutoPtr<IUri> id;
        condition->GetId((IUri**)&id);
        AutoPtr<IZenModeConfigHelper> zmc;
        CZenModeConfigHelper::AcquireSingleton((IZenModeConfigHelper**)&zmc);
        Boolean b;
        zmc->IsValidCountdownConditionId(id, &b);
        if (b && mBucketIndex != -1) {
            mPrefs->SetMinuteIndex(mBucketIndex);
        }
    }

    mSessionExitCondition = Copy(condition);
}

void ZenModePanel::FireMoreSettings()
{
    if (mCallback != NULL) {
        mCallback->OnMoreSettings();
    }
}

void ZenModePanel::FireInteraction()
{
    if (mCallback != NULL) {
        mCallback->OnInteraction();
    }
}

void ZenModePanel::FireExpanded()
{
    if (mCallback != NULL) {
        mCallback->OnExpanded(mExpanded);
    }
}

} // namespace Volume
} // namespace SystemUI
} // namespace Droid
} // namespace Elastos