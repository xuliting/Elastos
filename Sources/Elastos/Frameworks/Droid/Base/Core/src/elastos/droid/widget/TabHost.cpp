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

#include "elastos/droid/widget/TabHost.h"
#include "elastos/droid/os/Build.h"
#include "elastos/droid/text/TextUtils.h"
#include "elastos/droid/view/CViewGroupLayoutParams.h"
#include "elastos/droid/view/SoundEffectConstants.h"
#include "elastos/droid/view/CViewGroupLayoutParams.h"
#include "elastos/droid/R.h"
#include <elastos/core/CoreUtils.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Content::Res::IColorStateList;
using Elastos::Droid::Content::Pm::IApplicationInfo;
using Elastos::Droid::Os::Build;
using Elastos::Droid::Text::TextUtils;
using Elastos::Droid::View::EIID_IOnTouchModeChangeListener;
using Elastos::Droid::View::EIID_IViewOnKeyListener;
using Elastos::Droid::View::EIID_IViewGroup;
using Elastos::Droid::View::SoundEffectConstants;
using Elastos::Droid::View::IView;
using Elastos::Droid::View::IViewGroup;
using Elastos::Droid::View::IWindow;
using Elastos::Droid::View::ILayoutInflater;
using Elastos::Droid::View::IViewTreeObserver;
using Elastos::Droid::View::IOnTouchModeChangeListener;
using Elastos::Droid::View::CViewGroupLayoutParams;
using Elastos::Droid::View::Accessibility::IAccessibilityRecord;
using Elastos::Core::CoreUtils;
using Elastos::Utility::CArrayList;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Widget {

//==============================================================================
//                  TabHost::TabSpec
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::TabSpec, Object, ITabSpec)

TabHost::TabSpec::TabSpec(
    /* [in] */ const String& tag,
    /* [in] */ TabHost* owner)
    : mTag(tag)
    , mHost(owner)
{}

/**
 * Specify a label as the tab indicator.
 */
ECode TabHost::TabSpec::SetIndicator(
    /* [in] */ ICharSequence* label)
{
    mIndicatorStrategy = new TabHost::LabelIndicatorStrategy(label, mHost);
    return NOERROR;
}

/**
 * Specify a label and icon as the tab indicator.
 */
ECode TabHost::TabSpec::SetIndicator(
    /* [in] */ ICharSequence* label,
    /* [in] */ IDrawable* icon)
{
    mIndicatorStrategy = new TabHost::LabelAndIconIndicatorStrategy(label, icon, mHost);
    return NOERROR;
}

/**
 * Specify a view as the tab indicator.
 */
ECode TabHost::TabSpec::SetIndicator(
    /* [in] */ IView* view)
{
    mIndicatorStrategy = new TabHost::ViewIndicatorStrategy(view);
    return NOERROR;
}

/**
 * Specify the id of the view that should be used as the content
 * of the tab.
 */
ECode TabHost::TabSpec::SetContent(
    /* [in] */ Int32 viewId)
{
    AutoPtr<ViewIdContentStrategy> vics = new TabHost::ViewIdContentStrategy();
    vics->constructor(viewId, mHost);
    mContentStrategy = (ITabHostContentStrategy*)vics;
    return NOERROR;
}

/**
 * Specify a {@link android.widget.TabHost.TabContentFactory} to use to
 * create the content of the tab.
 */
ECode TabHost::TabSpec::SetContent(
    /* [in] */ ITabHostTabContentFactory* contentFactory)
{
    mContentStrategy = new TabHost::FactoryContentStrategy(CoreUtils::Convert(mTag), contentFactory);
    return NOERROR;
}

/**
 * Specify an intent to use to launch an activity as the tab content.
 */
ECode TabHost::TabSpec::SetContent(
    /* [in] */ IIntent* intent)
{
    mContentStrategy = new TabHost::IntentContentStrategy(mTag, intent, mHost);
    return NOERROR;
}

ECode TabHost::TabSpec::GetTag(
    /* [out] */ String* tag)
{
    VALIDATE_NOT_NULL(tag);
    *tag = mTag;
    return NOERROR;
}

//==============================================================================
//                  TabHost::LabelIndicatorStrategy
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::LabelIndicatorStrategy, Object, ITabHostIndicatorStrategy)

TabHost::LabelIndicatorStrategy::LabelIndicatorStrategy(
    /* [in] */ ICharSequence* label,
    /* [in] */ TabHost* owner)
    : mLabel(label)
    , mHost(owner)
{}

ECode TabHost::LabelIndicatorStrategy::CreateIndicatorView(
    /* [out] */ IView** view)
{
    VALIDATE_NOT_NULL(view);

    AutoPtr<IContext> context;
    mHost->GetContext((IContext**)&context);
    AutoPtr<IInterface> obj;
    context->GetSystemService(IContext::LAYOUT_INFLATER_SERVICE, (IInterface**)&obj);
    AutoPtr<ILayoutInflater> inflater = ILayoutInflater::Probe(obj);
    AutoPtr<IView> tabIndicator;
    inflater->Inflate(mHost->mTabLayoutId,
           IViewGroup::Probe(mHost->mTabWidget), // tab widget is the parent
           FALSE, (IView**)&tabIndicator); // no inflate params

    AutoPtr<IView> temp;
    tabIndicator->FindViewById(R::id::title, (IView**)&temp);
    AutoPtr<ITextView> tv = ITextView::Probe(temp);

    tv->SetText(mLabel);

    AutoPtr<IApplicationInfo> ai;
    context->GetApplicationInfo((IApplicationInfo**)&ai);
    Int32 version;
    ai->GetTargetSdkVersion(&version);
    if (version <= Build::VERSION_CODES::DONUT) {
        // Donut apps get old color scheme
        tabIndicator->SetBackgroundResource(R::drawable::tab_indicator_v4);
        AutoPtr<IResources> resources;
        context->GetResources((IResources**)&resources);
        AutoPtr<IColorStateList> sl;
        resources->GetColorStateList(R::color::tab_indicator_text_v4,
                (IColorStateList**)&sl);
        tv->SetTextColor(sl);
    }

    *view = tabIndicator;
    REFCOUNT_ADD(*view);
    return NOERROR;
}

//==============================================================================
//                  TabHost::LabelAndIconIndicatorStrategy
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::LabelAndIconIndicatorStrategy, Object, ITabHostIndicatorStrategy)

TabHost::LabelAndIconIndicatorStrategy::LabelAndIconIndicatorStrategy(
    /* [in] */ ICharSequence* label,
    /* [in] */ IDrawable* icon,
    /* [in] */ TabHost* owner)
    : mLabel(label)
    , mIcon(icon)
    , mHost(owner)
{}

ECode TabHost::LabelAndIconIndicatorStrategy::CreateIndicatorView(
    /* [out] */ IView** view)
{
    VALIDATE_NOT_NULL(view);

    AutoPtr<IContext> context;
    mHost->GetContext((IContext**)&context);
    AutoPtr<IInterface> obj;
    context->GetSystemService(IContext::LAYOUT_INFLATER_SERVICE, (IInterface**)&obj);
    AutoPtr<ILayoutInflater> inflater = ILayoutInflater::Probe(obj);
    AutoPtr<IView> tabIndicator;
    inflater->Inflate(mHost->mTabLayoutId,
           IViewGroup::Probe(mHost->mTabWidget), // tab widget is the parent
           FALSE, (IView**)&tabIndicator); // no inflate params

    AutoPtr<IView> temp;
    tabIndicator->FindViewById(R::id::title, (IView**)&temp);
    assert(temp != NULL);
    AutoPtr<ITextView> tv = ITextView::Probe(temp);

    temp = NULL;
    tabIndicator->FindViewById(R::id::icon, (IView**)&temp);
    AutoPtr<IImageView> iconView = IImageView::Probe(temp);

    // when icon is gone by default, we're in exclusive mode
    Int32 visibility;
    IView::Probe(iconView)->GetVisibility(&visibility);
    Boolean exclusive = (visibility == IView::GONE);
    Boolean bindIcon = !exclusive || TextUtils::IsEmpty(mLabel);

    tv->SetText(mLabel);

    if (bindIcon && mIcon != NULL) {
        iconView->SetImageDrawable(mIcon);
        IView::Probe(iconView)->SetVisibility(IView::VISIBLE);
    }

    AutoPtr<IApplicationInfo> ai;
    context->GetApplicationInfo((IApplicationInfo**)&ai);
    Int32 version;
    ai->GetTargetSdkVersion(&version);
    if (version <= Build::VERSION_CODES::DONUT) {
        // Donut apps get old color scheme
        tabIndicator->SetBackgroundResource(R::drawable::tab_indicator_v4);

        AutoPtr<IResources> resources;
        context->GetResources((IResources**)&resources);

        AutoPtr<IColorStateList> sl;
        resources->GetColorStateList(R::color::tab_indicator_text_v4,
                (IColorStateList**)&sl);

        tv->SetTextColor(sl);
    }

    *view = tabIndicator;
    REFCOUNT_ADD(*view);
    return NOERROR;
}

//==============================================================================
//                  TabHost::ViewIndicatorStrategy
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::ViewIndicatorStrategy, Object, ITabHostViewIndicatorStrategy, ITabHostIndicatorStrategy)

TabHost::ViewIndicatorStrategy::ViewIndicatorStrategy(
    /* [in] */ IView* view)
    : mView(view)
{}

ECode TabHost::ViewIndicatorStrategy::CreateIndicatorView(
    /* [out] */ IView** view)
{
    VALIDATE_NOT_NULL(view);
    *view = mView;
    REFCOUNT_ADD(*view);

    return NOERROR;
}

//==============================================================================
//                  TabHost::ViewIdContentStrategy
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::ViewIdContentStrategy, Object, ITabHostContentStrategy)

TabHost::ViewIdContentStrategy::ViewIdContentStrategy()
{
}

ECode TabHost::ViewIdContentStrategy::constructor(
    /* [in] */ Int32 id,
    /* [in] */ TabHost* host)
{
    mHost = host;
    IView::Probe(mHost->mTabContent)->FindViewById(id, (IView**)&mView);
    if (mView != NULL) {
        mView->SetVisibility(IView::GONE);
        return NOERROR;
    }
    else {
        Logger::E("TabHost::ViewIdContentStrategy::constructor", "Could not create tab content because "\
                "could not find view with id %d", id);
        return E_RUNTIME_EXCEPTION;
        /*throw new RuntimeException("Could not create tab content because " +
                "could not find view with id " + viewId);*/
    }
}

ECode TabHost::ViewIdContentStrategy::GetContentView(
    /* [out] */ IView** view)
{
    mView->SetVisibility(IView::VISIBLE);
    *view = mView;
    REFCOUNT_ADD(*view);
    return NOERROR;
}

ECode TabHost::ViewIdContentStrategy::TabClosed()
{
    return mView->SetVisibility(IView::GONE);
}

//==============================================================================
//                  TabHost::FactoryContentStrategy
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::FactoryContentStrategy, Object, ITabHostContentStrategy)

TabHost::FactoryContentStrategy::FactoryContentStrategy(
    /* [in] */ ICharSequence* tag,
    /* [in] */ ITabHostTabContentFactory* factory)
    : mTag(tag)
    , mFactory(factory)
{}

ECode TabHost::FactoryContentStrategy::GetContentView(
    /* [out] */ IView** view)
{
    VALIDATE_NOT_NULL(view);
    if (mTabContent == NULL) {
        String str;
        mTag->ToString(&str);
        mFactory->CreateTabContent(str, (IView**)&mTabContent);
    }
    mTabContent->SetVisibility(IView::VISIBLE);
    *view = mTabContent;
    REFCOUNT_ADD(*view);

    return NOERROR;
}

ECode TabHost::FactoryContentStrategy::TabClosed()
{
    return mTabContent->SetVisibility(IView::GONE);
}

//==============================================================================
//                  TabHost::IntentContentStrategy
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::IntentContentStrategy, Object, ITabHostContentStrategy)

TabHost::IntentContentStrategy::IntentContentStrategy(
    /* [in] */ const String& tag,
    /* [in] */ IIntent* intent,
    /* [in] */ TabHost* host)
    : mHost(host)
    , mTag(tag)
    , mIntent(intent)
{}

ECode TabHost::IntentContentStrategy::GetContentView(
    /* [out] */ IView** view)
{
    VALIDATE_NOT_NULL(view);
    *view = NULL;

    if (mHost->mLocalActivityManager == NULL) {
        Logger::E("TabHost::IntentContentStrategy::GetContentView", "Did you forget to call 'public void setup(LocalActivityManager activityGroup)'?");
        return E_ILLEGAL_STATE_EXCEPTION;
        //throw new IllegalStateException("Did you forget to call 'public void setup(LocalActivityManager activityGroup)'?");
    }

    AutoPtr<IWindow> w;
    mHost->mLocalActivityManager->StartActivity(mTag, mIntent, (IWindow**)&w);
    AutoPtr<IView> wd;
    if (w != NULL) {
        w->GetDecorView((IView**)&wd);
    }

    if (mLaunchedView != wd && mLaunchedView != NULL) {
        AutoPtr<IViewParent> parent;
        mLaunchedView->GetParent((IViewParent**)&parent);
        if (parent != NULL) {
            IViewGroup::Probe(mHost->mTabContent)->RemoveViewInLayout(mLaunchedView);
        }
    }
    mLaunchedView = wd;

    // XXX Set FOCUS_AFTER_DESCENDANTS on embedded activities for now so they can get
    // focus if none of their children have it. They need focus to be able to
    // display menu items.
    //
    // Replace this with something better when Bug 628886 is fixed...
    //
    if (mLaunchedView != NULL) {
        mLaunchedView->SetVisibility(IView::VISIBLE);
        mLaunchedView->SetFocusableInTouchMode(TRUE);
        IViewGroup* vg = IViewGroup::Probe(mLaunchedView);
        if (vg) {
            vg->SetDescendantFocusability(IViewGroup::FOCUS_AFTER_DESCENDANTS);
        }
    }

    *view = mLaunchedView;
    REFCOUNT_ADD(*view);
    return NOERROR;
}

ECode TabHost::IntentContentStrategy::TabClosed()
{
    if (mLaunchedView != NULL) {
        mLaunchedView->SetVisibility(IView::GONE);
    }

    return NOERROR;
}

//==============================================================================
//                  TabHost::TabKeyListener
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::TabKeyListener, Object, IViewOnKeyListener)

TabHost::TabKeyListener::TabKeyListener(
    /* [in] */ TabHost* owner)
    : mHost(owner)
{}

ECode TabHost::TabKeyListener::OnKey(
    /* [in] */ IView* v,
    /* [in] */ Int32 keyCode,
    /* [in] */ IKeyEvent* event,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    switch (keyCode) {
        case IKeyEvent::KEYCODE_DPAD_CENTER:
        case IKeyEvent::KEYCODE_DPAD_LEFT:
        case IKeyEvent::KEYCODE_DPAD_RIGHT:
        case IKeyEvent::KEYCODE_DPAD_UP:
        case IKeyEvent::KEYCODE_DPAD_DOWN:
        case IKeyEvent::KEYCODE_ENTER:
            *result = FALSE;
            return NOERROR;
    }

    IView::Probe(mHost->mTabContent)->RequestFocus(IView::FOCUS_FORWARD, result);
    return IView::Probe(mHost->mTabContent)->DispatchKeyEvent(event, result);
}

//==============================================================================
//                  TabHost::TabSelectionListener
//==============================================================================
CAR_INTERFACE_IMPL(TabHost::TabSelectionListener, Object, ITabWidgetOnTabSelectionChanged)

TabHost::TabSelectionListener::TabSelectionListener(
    /* [in] */ TabHost* owner)
    : mHost(owner)
{
}

ECode TabHost::TabSelectionListener::OnTabSelectionChanged(
    /* [in] */ Int32 tabIndex,
    /* [in] */ Boolean clicked)
{
    mHost->SetCurrentTab(tabIndex);
    if (clicked) {
        Boolean res;
        IView::Probe(mHost->mTabContent)->RequestFocus(IView::FOCUS_FORWARD, &res);
    }
    return NOERROR;
}

//==============================================================================
//                  TabHost
//==============================================================================

CAR_INTERFACE_IMPL(TabHost, FrameLayout, ITabHost, IOnTouchModeChangeListener)

const Int32 TabHost::TABWIDGET_LOCATION_LEFT;
const Int32 TabHost::TABWIDGET_LOCATION_TOP;
const Int32 TabHost::TABWIDGET_LOCATION_RIGHT;
const Int32 TabHost::TABWIDGET_LOCATION_BOTTOM;

TabHost::TabHost()
    : mCurrentTab(-1)
    , mTabLayoutId(0)
{
    CArrayList::New(2, (IList**)&mTabSpecs);
}

ECode TabHost::constructor(
    /* [in] */ IContext* context)
{
    FAIL_RETURN(FrameLayout::constructor(context))
    InitTabHost();
    return NOERROR;
}

ECode TabHost::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs)
{
    return constructor(context, attrs, R::attr::tabWidgetStyle);
}

ECode TabHost::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs,
    /* [in] */ Int32 defStyleAttr)
{
    return constructor(context, attrs, defStyleAttr, 0);
}

ECode TabHost::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs,
    /* [in] */ Int32 defStyleAttr,
    /* [in] */ Int32 defStyleRes)
{
    FAIL_RETURN(FrameLayout::constructor(context, attrs))

    AutoPtr<ArrayOf<Int32> > attrIds = TO_ATTRS_ARRAYOF(R::styleable::TabWidget);
    AutoPtr<ITypedArray> a;
    context->ObtainStyledAttributes(attrs, attrIds,
            defStyleAttr, defStyleRes, (ITypedArray**)&a);

    a->GetResourceId(R::styleable::TabWidget_tabLayout, 0, &mTabLayoutId);

    a->Recycle();

    if (mTabLayoutId == 0) {
        // In case the tabWidgetStyle does not inherit from Widget.TabWidget and tabLayout is
        // not defined.
        mTabLayoutId = R::layout::tab_indicator_holo;
    }

    InitTabHost();
    return NOERROR;
}

void TabHost::InitTabHost()
{
    SetFocusableInTouchMode(TRUE);
    SetDescendantFocusability(IViewGroup::FOCUS_AFTER_DESCENDANTS);

    mCurrentTab = -1;
    mCurrentView = NULL;
}

/**
 * Get a new {@link TabSpec} associated with this tab host.
 * @param tag required tag of tab.
 */
ECode TabHost::NewTabSpec(
    /* [in] */ const String& tag,
    /* [out] */ ITabSpec** spec)
{
    VALIDATE_NOT_NULL(spec)
    AutoPtr<ITabSpec> tabSpec = new TabHost::TabSpec(tag, this);
    *spec = tabSpec;
    REFCOUNT_ADD(*spec)
    return NOERROR;
}

/**
  * <p>Call setup() before adding tabs if loading TabHost using findViewById().
  * <i><b>However</i></b>: You do not need to call setup() after getTabHost()
  * in {@link android.app.TabActivity TabActivity}.
  * Example:</p>
<pre>mTabHost = (TabHost)findViewById(R::id::tabhost);
mTabHost.setup();
mTabHost.addTab(TAB_TAG_1, "Hello, world!", "Tab 1");
  */
ECode TabHost::Setup()
{
    AutoPtr<IView> v;
    FindViewById(R::id::tabs, (IView**)&v);
    mTabWidget = ITabWidget::Probe(v);
    if (mTabWidget == NULL) {
        Logger::E("TabHost::Setup", "Your TabHost must have a TabWidget whose id attribute is 'android.R::id::tabs'");
//        throw new RuntimeException(
//                "Your TabHost must have a TabWidget whose id attribute is 'android.R::id::tabs'");
        return E_RUNTIME_EXCEPTION;
    }

    // KeyListener to attach to all tabs. Detects non-navigation keys
    // and relays them to the tab content.
    mTabKeyListener = new TabKeyListener(this);

    AutoPtr<ITabWidgetOnTabSelectionChanged> listener = new TabSelectionListener(this);
    mTabWidget->SetTabSelectionListener(listener);
    v = NULL;
    FindViewById(R::id::tabcontent, (IView**)&v);
    mTabContent = IFrameLayout::Probe(v);
    if (mTabContent == NULL) {
        Logger::E("TabHost::Setup", "Your TabHost must have a FrameLayout whose id attribute is 'android.R::id::tabcontent'");
//        throw new RuntimeException(
//                "Your TabHost must have a FrameLayout whose id attribute is "
//                        + "'android.R::id::tabcontent'");
        return E_RUNTIME_EXCEPTION;
    }

    return NOERROR;
}

ECode TabHost::SendAccessibilityEvent(
    /* [in] */ Int32 eventType)
{
    /* avoid super class behavior - TabWidget sends the right events */
    return NOERROR;
}

/**
 * If you are using {@link TabSpec#setContent(android.content.Intent)}, this
 * must be called since the activityGroup is needed to launch the local activity.
 *
 * This is done for you if you extend {@link android.app.TabActivity}.
 * @param activityGroup Used to launch activities for tab content.
 */
ECode TabHost::Setup(
    /* [in ] */ ILocalActivityManager* activityGroup)
{
    Setup();
    mLocalActivityManager = activityGroup;
    return NOERROR;
}

ECode TabHost::OnAttachedToWindow()
{
    FrameLayout::OnAttachedToWindow();
    AutoPtr<IViewTreeObserver> treeObserver;
    GetViewTreeObserver((IViewTreeObserver**)&treeObserver);
    treeObserver->AddOnTouchModeChangeListener(this);
    return NOERROR;
}

ECode TabHost::OnDetachedFromWindow()
{
    FrameLayout::OnDetachedFromWindow();
    AutoPtr<IViewTreeObserver> treeObserver;
    GetViewTreeObserver((IViewTreeObserver**)&treeObserver);
    treeObserver->RemoveOnTouchModeChangeListener(this);
    return NOERROR;
}

/**
 * {@inheritDoc}
 */
ECode TabHost::OnTouchModeChanged(
    /* [in] */ Boolean isInTouchMode)
{
    if (!isInTouchMode) {
        // leaving touch mode.. if nothing has focus, let's give it to
        // the indicator of the current tab
        if (mCurrentView != NULL) {
            Boolean hasFocus, isFocused;
            mCurrentView->HasFocus(&hasFocus);
            mCurrentView->IsFocused(&isFocused);
            if (!hasFocus|| isFocused) {
                AutoPtr<IView> tabView;
                mTabWidget->GetChildTabViewAt(mCurrentTab, (IView**)&tabView);

                Boolean res;
                tabView->RequestFocus(&res);
            }
        }
    }

    return NOERROR;
}

/**
 * Add a tab.
 * @param tabSpec Specifies how to create the indicator and content.
 */
ECode TabHost::AddTab(
    /* [in] */ ITabSpec* tabSpec)
{
    VALIDATE_NOT_NULL(tabSpec);

    AutoPtr<TabSpec> ts = (TabSpec*)tabSpec;
    if (ts->mIndicatorStrategy == NULL) {
        Logger::E("TabHost::AddTab", "you must specify a way to create the tab indicator.");
//        throw new IllegalArgumentException("you must specify a way to create the tab indicator.");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    if (ts->mContentStrategy == NULL) {
        Logger::E("TabHost::AddTab", "you must specify a way to create the tab content");
//        throw new IllegalArgumentException("you must specify a way to create the tab content");
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    AutoPtr<IView> tabIndicator;
    ts->mIndicatorStrategy->CreateIndicatorView((IView**)&tabIndicator);
    tabIndicator->SetOnKeyListener(mTabKeyListener);

    // If this is a custom view, then do not draw the bottom strips for
    // the tab indicators.

    if (ITabHostViewIndicatorStrategy::Probe(ts->mIndicatorStrategy) != NULL) {
        mTabWidget->SetStripEnabled(FALSE);
    }

    IViewGroup::Probe(mTabWidget)->AddView(tabIndicator);
    mTabSpecs->Add(tabSpec);

    if (mCurrentTab == -1) {
        SetCurrentTab(0);
    }

    return NOERROR;
}

/**
 * Removes all tabs from the tab widget associated with this tab host.
 */
ECode TabHost::ClearAllTabs()
{
    IViewGroup::Probe(mTabWidget)->RemoveAllViews();
    InitTabHost();
    IViewGroup::Probe(mTabContent)->RemoveAllViews();
    mTabSpecs->Clear();
    RequestLayout();
    Invalidate();

    return NOERROR;
}

ECode TabHost::GetTabWidget(
    /* [out] */ ITabWidget** tabWidget)
{
    VALIDATE_NOT_NULL(tabWidget)
    *tabWidget = mTabWidget;
    REFCOUNT_ADD(*tabWidget)
    return NOERROR;
}

ECode TabHost::GetCurrentTab(
    /* [out] */ Int32* tab)
{
    VALIDATE_NOT_NULL(tab)
    *tab = mCurrentTab;
    return NOERROR;
}

ECode TabHost::GetCurrentTabTag(
    /* [out] */ String* tag)
{
    VALIDATE_NOT_NULL(tag)
    Int32 size;
    mTabSpecs->GetSize(&size);
    if (mCurrentTab >= 0 && mCurrentTab < size) {
        AutoPtr<IInterface> obj;
        mTabSpecs->Get(mCurrentTab, (IInterface**)&obj);
        return ITabSpec::Probe(obj)->GetTag(tag);

    }
    *tag = String(NULL);
    return NOERROR;
}

ECode TabHost::GetCurrentTabView(
    /* [out] */ IView** res)
{
    VALIDATE_NOT_NULL(res)
    Int32 size;
    mTabSpecs->GetSize(&size);
    if (mCurrentTab >= 0 && mCurrentTab < size) {
        return mTabWidget->GetChildTabViewAt(mCurrentTab, res);
    }
    *res = NULL;
    return NOERROR;
}

ECode TabHost::GetCurrentView(
    /* [out] */ IView** view)
{
    VALIDATE_NOT_NULL(view)
    *view = mCurrentView;
    REFCOUNT_ADD(*view)
    return NOERROR;
}

ECode TabHost::SetCurrentTabByTag(
    /* [in] */ const String& tag)
{
    Int32 size;
    mTabSpecs->GetSize(&size);
    for (Int32 i = 0; i < size; i++) {
        AutoPtr<IInterface> obj;
        mTabSpecs->Get(i, (IInterface**)&obj);
        String str;
        ITabSpec::Probe(obj)->GetTag(&str);
        if (str.Equals(tag)) {
            SetCurrentTab(i);
            break;
        }
    }

    return NOERROR;
}

/**
 * Get the FrameLayout which holds tab content
 */
ECode TabHost::GetTabContentView(
    /* [out] */ IFrameLayout** view)
{
    VALIDATE_NOT_NULL(view)
    *view = mTabContent;
    REFCOUNT_ADD(*view)
    return NOERROR;
}

/**
 * Get the location of the TabWidget.
 *
 * @return The TabWidget location.
 */
Int32 TabHost::GetTabWidgetLocation()
{
    Int32 location = TABWIDGET_LOCATION_TOP;

    Int32 orientation;
    ILinearLayout::Probe(mTabWidget)->GetOrientation(&orientation);
    switch (orientation) {
        case ILinearLayout::VERTICAL: {
            Int32 cl, wl;
            IView::Probe(mTabContent)->GetLeft(&cl);
            IView::Probe(mTabWidget)->GetLeft(&wl);
            location = (cl < wl) ? TABWIDGET_LOCATION_RIGHT : TABWIDGET_LOCATION_LEFT;
        }
        break;
        case ILinearLayout::HORIZONTAL:
        default: {
            Int32 ct, wt;
            IView::Probe(mTabContent)->GetTop(&ct);
            IView::Probe(mTabWidget)->GetTop(&wt);
            location = (ct < wt) ? TABWIDGET_LOCATION_BOTTOM : TABWIDGET_LOCATION_TOP;
        }
        break;
    }
    return location;
}

ECode TabHost::DispatchKeyEvent(
    /* [in] */ IKeyEvent* event,
    /* [out] */ Boolean* res)
{
    VALIDATE_NOT_NULL(res);
    Boolean handled;
    FrameLayout::DispatchKeyEvent(event, &handled);

    Int32 action, keyCode;
    event->GetAction(&action);

    Boolean result;
    AutoPtr<IView> view;
    AutoPtr<IView> v;
    // unhandled key ups change focus to tab indicator for embedded activities
    // when there is nothing that will take focus from default focus searching
    if (!handled
            && (action == IKeyEvent::ACTION_DOWN)
            && (mCurrentView != NULL)
            && (mCurrentView->IsRootNamespace(&result), result)
            && (mCurrentView->HasFocus(&result), result)) {

        Int32 keyCodeShouldChangeFocus = IKeyEvent::KEYCODE_DPAD_UP;
        Int32 directionShouldChangeFocus = IView::FOCUS_UP;
        Int32 soundEffect = SoundEffectConstants::NAVIGATION_UP;

        switch (GetTabWidgetLocation()) {
            case TABWIDGET_LOCATION_LEFT:
                keyCodeShouldChangeFocus = IKeyEvent::KEYCODE_DPAD_LEFT;
                directionShouldChangeFocus = IView::FOCUS_LEFT;
                soundEffect = SoundEffectConstants::NAVIGATION_LEFT;
                break;
            case TABWIDGET_LOCATION_RIGHT:
                keyCodeShouldChangeFocus = IKeyEvent::KEYCODE_DPAD_RIGHT;
                directionShouldChangeFocus = IView::FOCUS_RIGHT;
                soundEffect = SoundEffectConstants::NAVIGATION_RIGHT;
                break;
            case TABWIDGET_LOCATION_BOTTOM:
                keyCodeShouldChangeFocus = IKeyEvent::KEYCODE_DPAD_DOWN;
                directionShouldChangeFocus = IView::FOCUS_DOWN;
                soundEffect = SoundEffectConstants::NAVIGATION_DOWN;
                break;
            case TABWIDGET_LOCATION_TOP:
            default:
                keyCodeShouldChangeFocus = IKeyEvent::KEYCODE_DPAD_UP;
                directionShouldChangeFocus = IView::FOCUS_UP;
                soundEffect = SoundEffectConstants::NAVIGATION_UP;
                break;
        }

        event->GetKeyCode(&keyCode);
        if (keyCode == keyCodeShouldChangeFocus) {
            AutoPtr<IView> view, focusView;
            mCurrentView->FindFocus((IView**)&view);
            view->FocusSearch(directionShouldChangeFocus, (IView**)&focusView);
            if (focusView == NULL) {
                AutoPtr<IView> tabView;
                mTabWidget->GetChildTabViewAt(mCurrentTab, (IView**)&tabView);
                Boolean hasFocus;
                tabView->RequestFocus(&hasFocus);
                PlaySoundEffect(soundEffect);
                *res = TRUE;
                return NOERROR;
            }
        }
    }

    *res = handled;
    return NOERROR;
}


ECode TabHost::DispatchWindowFocusChanged(
    /* [in] */ Boolean hasFocus)
{
    if (mCurrentView != NULL){
        mCurrentView->DispatchWindowFocusChanged(hasFocus);
    }

    return NOERROR;
}

ECode TabHost::OnInitializeAccessibilityEvent(
    /* [in] */ IAccessibilityEvent* event)
{
    FrameLayout::OnInitializeAccessibilityEvent(event);
    IAccessibilityRecord::Probe(event)->SetClassName(CoreUtils::Convert("CTabHost"));
    return NOERROR;
}

ECode TabHost::OnInitializeAccessibilityNodeInfo(
    /* [in] */ IAccessibilityNodeInfo* info)
{
    FrameLayout::OnInitializeAccessibilityNodeInfo(info);
    info->SetClassName(CoreUtils::Convert("CTabHost"));
    return NOERROR;
}

ECode TabHost::SetCurrentTab(
    /* [in] */ Int32 index)
{
    Int32 size;
    mTabSpecs->GetSize(&size);
    if (index < 0 || index >= size) {
        return NOERROR;
    }

    if (index == mCurrentTab) {
        return NOERROR;
    }

    // notify old tab content
    if (mCurrentTab != -1) {
        AutoPtr<IInterface> obj;
        mTabSpecs->Get(mCurrentTab, (IInterface**)&obj);
        TabSpec* ts = (TabSpec*)ITabSpec::Probe(obj);
        ts->mContentStrategy->TabClosed();
    }

    mCurrentTab = index;
    AutoPtr<IInterface> obj;
    mTabSpecs->Get(index, (IInterface**)&obj);
    AutoPtr<TabSpec> spec = (TabSpec*)ITabSpec::Probe(obj);

    // Call the tab widget's focusCurrentTab(), instead of just
    // selecting the tab.
    mTabWidget->FocusCurrentTab(mCurrentTab);

    // tab content
    mCurrentView = NULL;
    spec->mContentStrategy->GetContentView((IView**)&mCurrentView);

    AutoPtr<IViewParent> view;
    mCurrentView->GetParent((IViewParent**)&view);
    if (view == NULL) {
        AutoPtr<IViewGroupLayoutParams> lp;
        CViewGroupLayoutParams::New(
                IViewGroupLayoutParams::MATCH_PARENT,
                IViewGroupLayoutParams::MATCH_PARENT,
                (IViewGroupLayoutParams**)&lp);
        IViewGroup::Probe(mTabContent)->AddView(mCurrentView, lp);
    }

    Boolean hasFocus;
    IView::Probe(mTabWidget)->HasFocus(&hasFocus);
    if (!hasFocus) {
        // if the tab widget didn't take focus (likely because we're in touch mode)
        // give the current tab content view a shot
        IView::Probe(mCurrentView)->RequestFocus(&hasFocus);
    }

    IView::Probe(mTabContent)->RequestFocus(&hasFocus);
    InvokeOnTabChangeListener();
    return NOERROR;
}

/**
 * Register a callback to be invoked when the selected state of any of the items
 * in this list changes
 * @param l
 * The callback that will run
 */
ECode TabHost::SetOnTabChangedListener(
    /* [in] */ ITabHostOnTabChangeListener* l)
{
    mOnTabChangeListener = l;

    return NOERROR;
}

void TabHost::InvokeOnTabChangeListener()
{
    if (mOnTabChangeListener != NULL) {
        String str;
        GetCurrentTabTag(&str);
        mOnTabChangeListener->OnTabChanged(str);
    }
}

} // namespace Widget
} // namespace Droid
} // namespace Elastos
