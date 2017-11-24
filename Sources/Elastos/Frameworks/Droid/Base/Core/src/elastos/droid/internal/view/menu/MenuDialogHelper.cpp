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

#include "Elastos.Droid.App.h"
#include "elastos/droid/internal/view/menu/MenuDialogHelper.h"
#include "elastos/droid/internal/view/menu/CListMenuPresenter.h"
#include "elastos/droid/app/CAlertDialogBuilder.h"
#include "elastos/droid/view/CWindowManagerLayoutParams.h"
#include "elastos/droid/R.h"

using Elastos::Droid::App::IAlertDialogBuilder;
using Elastos::Droid::App::CAlertDialogBuilder;
using Elastos::Droid::Content::EIID_IDialogInterfaceOnClickListener;
using Elastos::Droid::Content::EIID_IDialogInterfaceOnKeyListener;
using Elastos::Droid::Content::EIID_IDialogInterfaceOnDismissListener;
using Elastos::Droid::Graphics::Drawable::IDrawable;
using Elastos::Droid::View::IDispatcherState;
using Elastos::Droid::View::IMenu;
using Elastos::Droid::View::IMenuItem;
using Elastos::Droid::View::IView;
using Elastos::Droid::View::IWindow;
using Elastos::Droid::View::IWindowManagerLayoutParams;
using Elastos::Droid::Widget::IAdapter;
using Elastos::Droid::Widget::IListAdapter;

namespace Elastos {
namespace Droid {
namespace Internal {
namespace View {
namespace Menu {

CAR_INTERFACE_IMPL(MenuDialogHelper::Listener, Object, IDialogInterfaceOnKeyListener,
    IDialogInterfaceOnClickListener, IDialogInterfaceOnDismissListener, IMenuPresenterCallback)

MenuDialogHelper::Listener::Listener(
    /* [in] */ MenuDialogHelper* owner)
{
    owner->GetWeakReference((IWeakReference**)&mWeakOwner);
}

ECode MenuDialogHelper::Listener::OnKey(
    /* [in] */ IDialogInterface* dialog,
    /* [in] */ Int32 keyCode,
    /* [in] */ IKeyEvent* event,
    /* [out] */ Boolean* flag)
{
    AutoPtr<IInterface> obj;
    mWeakOwner->Resolve(EIID_IInterface, (IInterface**)&obj);
    if (obj) {
        MenuDialogHelper* owner = (MenuDialogHelper*)IMenuDialogHelper::Probe(obj);
        return owner->OnKey(dialog, keyCode, event, flag);
    }
    return NOERROR;
}

ECode MenuDialogHelper::Listener::OnDismiss(
    /* [in] */ IDialogInterface* dialog)
{
    AutoPtr<IInterface> obj;
    mWeakOwner->Resolve(EIID_IInterface, (IInterface**)&obj);
    if (obj) {
        MenuDialogHelper* owner = (MenuDialogHelper*)IMenuDialogHelper::Probe(obj);
        return owner->OnDismiss(dialog);
    }
    return NOERROR;
}

ECode MenuDialogHelper::Listener::OnClick(
    /* [in] */ IDialogInterface* dialog,
    /* [in] */ Int32 which)
{
    AutoPtr<IInterface> obj;
    mWeakOwner->Resolve(EIID_IInterface, (IInterface**)&obj);
    if (obj) {
        MenuDialogHelper* owner = (MenuDialogHelper*)IMenuDialogHelper::Probe(obj);
        return owner->OnClick(dialog, which);
    }
    return NOERROR;
}

ECode MenuDialogHelper::Listener::OnCloseMenu(
    /* [in] */ IMenuBuilder* menu,
    /* [in] */ Boolean allMenusAreClosing)
{
    AutoPtr<IInterface> obj;
    mWeakOwner->Resolve(EIID_IInterface, (IInterface**)&obj);
    if (obj) {
        MenuDialogHelper* owner = (MenuDialogHelper*)IMenuDialogHelper::Probe(obj);
        return owner->OnCloseMenu(menu, allMenusAreClosing);
    }
    return NOERROR;
}

ECode MenuDialogHelper::Listener::OnOpenSubMenu(
    /* [in] */ IMenuBuilder* subMenu,
    /* [out] */ Boolean* result)
{
    AutoPtr<IInterface> obj;
    mWeakOwner->Resolve(EIID_IInterface, (IInterface**)&obj);
    if (obj) {
        MenuDialogHelper* owner = (MenuDialogHelper*)IMenuDialogHelper::Probe(obj);
        return owner->OnOpenSubMenu(subMenu, result);
    }
    return NOERROR;
}

CAR_INTERFACE_IMPL(MenuDialogHelper, Object, IMenuDialogHelper, IDialogInterfaceOnKeyListener,
    IDialogInterfaceOnClickListener, IDialogInterfaceOnDismissListener, IMenuPresenterCallback)

ECode MenuDialogHelper::constructor(
    /* [in] */ IMenuBuilder* menu)
{
    mMenu = menu;
    return NOERROR;
}

ECode MenuDialogHelper::Show(
    /* [in] */ IBinder* windowToken)
{
    // Many references to mMenu, create local reference
    AutoPtr<IMenuBuilder> menu = mMenu;

    // Get the builder for the dialog
    AutoPtr<IContext> context;
    menu->GetContext((IContext**)&context);
    AutoPtr<IAlertDialogBuilder> builder;
    CAlertDialogBuilder::New(context, (IAlertDialogBuilder**)&builder);

    AutoPtr<IContext> alctx;
    builder->GetContext((IContext**)&alctx);

    mPresenter = NULL;
    CListMenuPresenter::New(alctx, R::layout::list_menu_item_layout, (IListMenuPresenter**)&mPresenter);

    AutoPtr<Listener> listener = new Listener(this);
    IMenuPresenter::Probe(mPresenter)->SetCallback(listener);
    mMenu->AddMenuPresenter(IMenuPresenter::Probe(mPresenter));

    AutoPtr<IListAdapter> adapter;
    mPresenter->GetAdapter((IListAdapter**)&adapter);
    builder->SetAdapter(adapter, listener);

    // Set the title
    AutoPtr<IView> headerView;
    menu->GetHeaderView((IView**)&headerView);
    if (headerView != NULL) {
        // Menu's client has given a custom header view, use it
        builder->SetCustomTitle(headerView);
    }
    else {
        // Otherwise use the (text) title and icon
        AutoPtr<IDrawable> icon;
        menu->GetHeaderIcon((IDrawable**)&icon);
        builder->SetIcon(icon);

        AutoPtr<ICharSequence> csq;
        menu->GetHeaderTitle((ICharSequence**)&csq);
        builder->SetTitle(csq);
    }

    // Set the key listener
    builder->SetOnKeyListener(listener);

    // Show the menu
    AutoPtr<IAlertDialog> dialog;
    builder->Create((IAlertDialog**)&dialog);
    mDialog = IDialog::Probe(dialog);
    mDialog->SetOnDismissListener(listener);

    AutoPtr<IWindow> window;
    mDialog->GetWindow((IWindow**)&window);
    AutoPtr<IWindowManagerLayoutParams> lp;
    window->GetAttributes((IWindowManagerLayoutParams**)&lp);
    lp->SetType(IWindowManagerLayoutParams::TYPE_APPLICATION_ATTACHED_DIALOG);
    if (windowToken != NULL) {
        lp->SetToken(windowToken);
    }
    Int32 flag;
    lp->GetFlags(&flag);
    lp->SetFlags(flag |= IWindowManagerLayoutParams::FLAG_ALT_FOCUSABLE_IM);

    mDialog->Show();
    return NOERROR;
}

ECode MenuDialogHelper::OnKey(
        /* [in] */ IDialogInterface* dialog,
        /* [in] */ Int32 keyCode,
        /* [in] */ IKeyEvent* event,
        /* [out] */ Boolean* flag)
{
    VALIDATE_NOT_NULL(flag);

    if (keyCode == IKeyEvent::KEYCODE_MENU || keyCode == IKeyEvent::KEYCODE_BACK) {
        Int32 action, repeatCount;
        event->GetAction(&action);
        event->GetRepeatCount(&repeatCount);
        Boolean isCanceled;
        if (action == IKeyEvent::ACTION_DOWN && repeatCount == 0) {
            AutoPtr<IWindow> win;
            mDialog->GetWindow((IWindow**)&win);
            if (win != NULL) {
                AutoPtr<IView> decor;
                win->GetDecorView((IView**)&decor);
                if (decor != NULL) {
                    AutoPtr<IDispatcherState> dispatcher;
                    decor->GetKeyDispatcherState((IDispatcherState**)&dispatcher);
                    if (dispatcher != NULL) {
                        dispatcher->StartTracking(event, TO_IINTERFACE(this));
                        *flag = TRUE;
                        return NOERROR;
                    }
                }
            }
        }
        else if (action == IKeyEvent::ACTION_UP && (event->IsCanceled(&isCanceled), !isCanceled)) {
            AutoPtr<IWindow> win;
            mDialog->GetWindow((IWindow**)&win);
            if (win != NULL) {
                AutoPtr<IView> decor;
                win->GetDecorView((IView**)&decor);
                if (decor != NULL) {
                    AutoPtr<IDispatcherState> dispatcher;
                    decor->GetKeyDispatcherState((IDispatcherState**)&dispatcher);
                    Boolean isTracking;
                    if (dispatcher != NULL && (dispatcher->IsTracking(event, &isTracking), isTracking)) {
                        mMenu->Close(TRUE);
                        dialog->Dismiss();
                        *flag = TRUE;
                        return NOERROR;
                    }
                }
            }
        }
    }

    // Menu shortcut matching
    return IMenu::Probe(mMenu)->PerformShortcut(keyCode, event, 0, flag);
}

ECode MenuDialogHelper::SetPresenterCallback(
    /* [in] */ IMenuPresenterCallback* cb)
{
    mPresenterCallback = cb;
    return NOERROR;
}

ECode MenuDialogHelper::Dismiss()
{
    if (mDialog != NULL) {
        IDialogInterface::Probe(mDialog)->Dismiss();
    }
    return NOERROR;
}

ECode MenuDialogHelper::OnDismiss(
    /* [in] */ IDialogInterface* dialog)
{
    return IMenuPresenter::Probe(mPresenter)->OnCloseMenu(mMenu, TRUE);
}

ECode MenuDialogHelper::OnCloseMenu(
    /* [in] */ IMenuBuilder* menu,
    /* [in] */ Boolean allMenusAreClosing)
{
    if (allMenusAreClosing || menu == mMenu) {
        Dismiss();
    }
    if (mPresenterCallback != NULL) {
        return mPresenterCallback->OnCloseMenu(menu, allMenusAreClosing);
    }

    return NOERROR;
}

ECode MenuDialogHelper::OnOpenSubMenu(
    /* [in] */ IMenuBuilder* subMenu,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    *result = FALSE;
    if (mPresenterCallback != NULL) {
        return mPresenterCallback->OnOpenSubMenu(subMenu, result);
    }

    return NOERROR;
}

ECode MenuDialogHelper::OnClick(
    /* [in] */ IDialogInterface* dialog,
    /* [in] */ Int32 which)
{
    AutoPtr<IListAdapter> adapter;
    mPresenter->GetAdapter((IListAdapter**)&adapter);
    AutoPtr<IInterface> item;
    IAdapter::Probe(adapter)->GetItem(which, (IInterface**)&item);

    Boolean state;
    return mMenu->PerformItemAction(IMenuItem::Probe(item) , 0, &state);
}

} // namespace Menu
} // namespace View
} // namespace Internal
} // namespace Droid
} // namespace Elastos
