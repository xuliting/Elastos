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

#ifndef __ELASTOS_DROID_SYSTEMUI_NET_NETWORKOVERLIMITACTIVITY_H__
#define __ELASTOS_DROID_SYSTEMUI_NET_NETWORKOVERLIMITACTIVITY_H__

#include "_Elastos.Droid.SystemUI.h"
#include "Elastos.Droid.App.h"
#include "Elastos.Droid.Content.h"
#include "Elastos.Droid.Net.h"
#include "Elastos.Droid.Os.h"
#include "elastos/droid/app/Activity.h"

using Elastos::Droid::App::Activity;
using Elastos::Droid::Content::IDialogInterface;
using Elastos::Droid::Content::IDialogInterfaceOnClickListener;
using Elastos::Droid::Content::IDialogInterfaceOnDismissListener;
using Elastos::Droid::Net::INetworkTemplate;
using Elastos::Droid::Os::IBundle;

namespace Elastos {
namespace Droid {
namespace SystemUI {
namespace Net {

/**
 * Notify user that a {@link NetworkTemplate} is over its
 * {@link NetworkPolicy#limitBytes}, giving them the choice of acknowledging or
 * "snoozing" the limit.
 */
class NetworkOverLimitActivity
    : public Activity
    , public INetworkOverLimitActivity
{
private:
    class MyDialogInterfaceOnClickListener
        : public Object
        , public IDialogInterfaceOnClickListener
    {
    public:
        CAR_INTERFACE_DECL()

        MyDialogInterfaceOnClickListener(
            /* [in] */ INetworkTemplate* temp,
            /* [in] */ NetworkOverLimitActivity* host);

        CARAPI OnClick(
            /* [in] */ IDialogInterface* dialog,
            /* [in] */ Int32 which);

    private:
        NetworkOverLimitActivity* mHost;
        AutoPtr<INetworkTemplate> mTemp;
    };

    class MyDialogInterfaceOnDismissListener
        : public Object
        , public IDialogInterfaceOnDismissListener
    {
    public:
        CAR_INTERFACE_DECL()

        MyDialogInterfaceOnDismissListener(
            /* [in] */ NetworkOverLimitActivity* host);

        CARAPI OnDismiss(
            /* [in] */ IDialogInterface* dialog);

    private:
        NetworkOverLimitActivity* mHost;
    };

public:
    CAR_INTERFACE_DECL()

    CARAPI constructor();

    // @Override
    CARAPI OnCreate(
        /* [in] */ IBundle* icicle);

private:
    CARAPI SnoozePolicy(
        /* [in] */ INetworkTemplate* temp);

    static CARAPI_(Int32) GetLimitedDialogTitleForTemplate(
        /* [in] */ INetworkTemplate* temp);

private:
    const static String TAG;
};

} // namespace Net
} // namespace SystemUI
} // namespace Droid
} // namespace Elastos

#endif //__ELASTOS_DROID_SYSTEMUI_NET_NETWORKOVERLIMITACTIVITY_H__