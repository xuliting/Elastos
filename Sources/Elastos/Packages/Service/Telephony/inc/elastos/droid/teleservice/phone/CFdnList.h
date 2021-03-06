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

#ifndef  __ELASTOS_DROID_PHONE_CFDNLIST_H__
#define  __ELASTOS_DROID_PHONE_CFDNLIST_H__

#include "_Elastos_Droid_TeleService_Phone_CFdnList.h"
#include "elastos/droid/ext/frameworkext.h"
#include "elastos/droid/teleservice/phone/ADNList.h"
#include "Elastos.Droid.Net.h"
#include "Elastos.Droid.Os.h"
#include "Elastos.Droid.View.h"
#include "Elastos.Droid.Widget.h"

using Elastos::Droid::Net::IUri;
using Elastos::Droid::Os::IBundle;
using Elastos::Droid::View::IMenu;
using Elastos::Droid::View::IMenuItem;
using Elastos::Droid::Widget::IListView;

namespace Elastos {
namespace Droid {
namespace TeleService {
namespace Phone {

/**
 * Fixed Dialing Number (FDN) List UI for the Phone app. FDN is a feature of the service provider
 * that allows a user to specify a limited set of phone numbers that the SIM can dial.
 */
CarClass(CFdnList)
    , public ADNList
{
public:
    CAR_OBJECT_DECL();

    CARAPI constructor();

    //@Override
    CARAPI OnCreate(
        /* [in] */ IBundle* icicle);

    //@Override
    CARAPI OnCreateOptionsMenu(
        /* [in] */ IMenu* menu,
        /* [out] */ Boolean* result);

    //@Override
    CARAPI OnPrepareOptionsMenu(
        /* [in] */ IMenu* menu,
        /* [out] */ Boolean* result);

    //@Override
    CARAPI OnOptionsItemSelected(
        /* [in] */ IMenuItem* item,
        /* [out] */ Boolean* result);

    //@Override
    CARAPI OnListItemClick(
        /* [in] */ IListView* l,
        /* [in] */ IView* v,
        /* [in] */ Int32 position,
        /* [in] */ Int64 id);

protected:
    //@Override
    CARAPI ResolveIntent(
        /* [out] */ IUri** uri);

private:
    CARAPI_(void) AddContact();

    /**
     * Overloaded to call editSelected with the current selection
     * by default.  This method may have a problem with touch UI
     * since touch UI does not really have a concept of "selected"
     * items.
     */
    CARAPI_(void) EditSelected();

    /**
     * Edit the item at the selected position in the list.
     */
    CARAPI_(void) EditSelected(
        /* [in] */ Int32 position);

    CARAPI_(void) DeleteSelected();

private:
    static const Int32 MENU_ADD = 1;
    static const Int32 MENU_EDIT = 2;
    static const Int32 MENU_DELETE = 3;

    static const String INTENT_EXTRA_NAME;
    static const String INTENT_EXTRA_NUMBER;
};

} // namespace Phone
} // namespace TeleService
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_PHONE_CFDNLIST_H__