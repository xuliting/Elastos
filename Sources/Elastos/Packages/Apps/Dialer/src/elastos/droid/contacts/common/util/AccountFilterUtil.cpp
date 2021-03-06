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
#include "Elastos.Droid.Widget.h"
#include "Elastos.CoreLibrary.Core.h"
#include "elastos/droid/contacts/common/util/AccountFilterUtil.h"
#include "elastos/droid/contacts/common/list/CAccountFilterActivity.h"
#include <elastos/utility/logging/Logger.h>
#include "R.h"

using Elastos::Droid::App::IActivity;
using Elastos::Droid::Content::IContext;
using Elastos::Droid::Content::IIntent;
using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Contacts::Common::List::CAccountFilterActivity;
using Elastos::Droid::Contacts::Common::List::IAccountFilterActivity;
using Elastos::Droid::Contacts::Common::List::ECLSID_CAccountFilterActivity;
using Elastos::Droid::Widget::ITextView;
using Elastos::Core::ICharSequence;
using Elastos::Core::CString;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Contacts {
namespace Common {
namespace Util {

const String AccountFilterUtil::TAG("AccountFilterUtil");

Boolean AccountFilterUtil::UpdateAccountFilterTitleForPeople(
    /* [in] */ IView* filterContainer,
    /* [in] */ IContactListFilter* filter,
    /* [in] */ Boolean showTitleForAllAccounts)
{
    return UpdateAccountFilterTitle(filterContainer, filter, showTitleForAllAccounts, FALSE);
}

Boolean AccountFilterUtil::UpdateAccountFilterTitleForPhone(
    /* [in] */ IView* filterContainer,
    /* [in] */ IContactListFilter* filter,
    /* [in] */ Boolean showTitleForAllAccounts)
{
    return UpdateAccountFilterTitle(
            filterContainer, filter, showTitleForAllAccounts, TRUE);
}

Boolean AccountFilterUtil::UpdateAccountFilterTitle(
    /* [in] */ IView* filterContainer,
    /* [in] */ IContactListFilter* filter,
    /* [in] */ Boolean showTitleForAllAccounts,
    /* [in] */ Boolean forPhone)
{
    AutoPtr<IContext> context;
    filterContainer->GetContext((IContext**)&context);
    AutoPtr<IView> temp;
    filterContainer->FindViewById(Elastos::Droid::Dialer::R::id::account_filter_header, (IView**)&temp);
    AutoPtr<ITextView> headerTextView = ITextView::Probe(temp);

    Boolean textWasSet = FALSE;
    if (filter != NULL) {
        if (forPhone) {
            Int32 filterType;
            if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_ALL_ACCOUNTS) {
                if (showTitleForAllAccounts) {
                    headerTextView->SetText(Elastos::Droid::Dialer::R::string::list_filter_phones);
                    textWasSet = TRUE;
                }
            }
            else if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_ACCOUNT) {
                String accountName;
                filter->GetAccountName(&accountName);
                AutoPtr<ICharSequence> cs;
                CString::New(accountName, (ICharSequence**)&cs);
                AutoPtr<ArrayOf<IInterface*> > atts = ArrayOf<IInterface*>::Alloc(1);
                atts->Set(0, cs);
                String text;
                context->GetString(
                        Elastos::Droid::Dialer::R::string::listAllContactsInAccount, atts, &text);
                AutoPtr<ICharSequence> textCS;
                CString::New(text, (ICharSequence**)&textCS);
                headerTextView->SetText(textCS);
                textWasSet = TRUE;
            }
            else if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_CUSTOM) {
                headerTextView->SetText(Elastos::Droid::Dialer::R::string::listCustomView);
                textWasSet = TRUE;
            }
            else {
                Logger::W(TAG, "Filter type \"%d\" isn't expected.", filterType);
            }
        }
        else {
            Int32 filterType;
            if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_ALL_ACCOUNTS) {
                if (showTitleForAllAccounts) {
                    headerTextView->SetText(Elastos::Droid::Dialer::R::string::list_filter_all_accounts);
                    textWasSet = TRUE;
                }
            }
            else if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_ACCOUNT) {
                String accountName;
                filter->GetAccountName(&accountName);
                AutoPtr<ICharSequence> cs;
                CString::New(accountName, (ICharSequence**)&cs);
                AutoPtr<ArrayOf<IInterface*> > atts = ArrayOf<IInterface*>::Alloc(1);
                atts->Set(0, cs);
                String text;
                context->GetString(
                        Elastos::Droid::Dialer::R::string::listAllContactsInAccount, atts, &text);
                AutoPtr<ICharSequence> textCS;
                CString::New(text, (ICharSequence**)&textCS);
                headerTextView->SetText(textCS);
                textWasSet = TRUE;
            }
            else if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_CUSTOM) {
                headerTextView->SetText(Elastos::Droid::Dialer::R::string::listCustomView);
                textWasSet = TRUE;
            }
            else if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_SINGLE_CONTACT) {
                headerTextView->SetText(Elastos::Droid::Dialer::R::string::listSingleContact);
                textWasSet = TRUE;
            }
            else {
                Logger::W(TAG, "Filter type \"%d\" isn't expected.", filterType);
            }
        }
    }
    else {
        Logger::W(TAG, "Filter is null.");
    }
    return textWasSet;
}

void AccountFilterUtil::StartAccountFilterActivityForResult(
    /* [in] */ IActivity* activity,
    /* [in] */ Int32 requestCode,
    /* [in] */ IContactListFilter* currentFilter)
{
    AutoPtr<IIntent> intent;
    CIntent::New(IContext::Probe(activity), ECLSID_CAccountFilterActivity, (IIntent**)&intent);
    intent->PutExtra(IAccountFilterActivity::KEY_EXTRA_CURRENT_FILTER, IParcelable::Probe(currentFilter));
    activity->StartActivityForResult(intent, requestCode);
}

void AccountFilterUtil::StartAccountFilterActivityForResult(
    /* [in] */ IFragment* fragment,
    /* [in] */ Int32 requestCode,
    /* [in] */ IContactListFilter* currentFilter)
{
    AutoPtr<IActivity> activity;
    fragment->GetActivity((IActivity**)&activity);
    if (activity != NULL) {
        AutoPtr<IIntent> intent;
        CIntent::New(IContext::Probe(activity), ECLSID_CAccountFilterActivity, (IIntent**)&intent);
        intent->PutExtra(IAccountFilterActivity::KEY_EXTRA_CURRENT_FILTER, IParcelable::Probe(currentFilter));
        fragment->StartActivityForResult(intent, requestCode);
    }
    else {
        Logger::W(TAG, "getActivity() returned null. Ignored");
    }
}

void AccountFilterUtil::HandleAccountFilterResult(
    /* [in] */ IContactListFilterController* filterController,
    /* [in] */ Int32 resultCode,
    /* [in] */ IIntent* data)
{
    if (resultCode == IActivity::RESULT_OK) {
        AutoPtr<IParcelable> parcelable;
        data->GetParcelableExtra(IAccountFilterActivity::KEY_EXTRA_CONTACT_LIST_FILTER, (IParcelable**)&parcelable);
        AutoPtr<IContactListFilter> filter = IContactListFilter::Probe(parcelable);
        if (filter == NULL) {
            return;
        }
        Int32 filterType;
        if (filter->GetFilterType(&filterType), filterType == IContactListFilter::FILTER_TYPE_CUSTOM) {
            filterController->SelectCustomFilter();
        }
        else {
            filterController->SetContactListFilter(filter, TRUE);
        }
    }
}

} // namespace Util
} // namespace Common
} // namespace Contacts
} // namespace Droid
} // namespace Elastos
