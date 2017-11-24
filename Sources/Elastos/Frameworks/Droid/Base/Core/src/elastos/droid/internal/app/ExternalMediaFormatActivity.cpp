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

#include "elastos/droid/internal/app/ExternalMediaFormatActivity.h"
#include "elastos/droid/internal/os/storage/CExternalStorageFormatter.h"
#include "elastos/droid/content/CIntentFilter.h"
#include "elastos/droid/content/CIntent.h"
#include "elastos/droid/R.h"
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::CIntentFilter;
using Elastos::Droid::Content::EIID_IDialogInterfaceOnClickListener;
using Elastos::Droid::Content::IIntentFilter;
using Elastos::Droid::Internal::Os::Storage::CExternalStorageFormatter;
using Elastos::Droid::Internal::Os::Storage::IExternalStorageFormatter;
using Elastos::Core::ICharSequence;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Internal {
namespace App {

ExternalMediaFormatActivity::StorageReceiver::StorageReceiver(
    /* [in] */ ExternalMediaFormatActivity* host)
    : mHost(host)
{}

ECode ExternalMediaFormatActivity::StorageReceiver::OnReceive(
    /* [in] */ IContext* context,
    /* [in] */ IIntent* intent)
{
    String action;
    intent->GetAction(&action);
    Logger::D("ExternalMediaFormatActivity", "got action %s", action.string());

    if (action == IIntent::ACTION_MEDIA_REMOVED ||
        action == IIntent::ACTION_MEDIA_CHECKING ||
        action == IIntent::ACTION_MEDIA_MOUNTED ||
        action == IIntent::ACTION_MEDIA_SHARED) {
        mHost->Finish();
    }
    return NOERROR;
}

ECode ExternalMediaFormatActivity::StorageReceiver::ToString(
    /* [out] */ String* info)
{
    VALIDATE_NOT_NULL(info);
    *info = String("StorageReceiver: ");
    (*info).AppendFormat("%p", this);
    return NOERROR;
}

CAR_INTERFACE_IMPL(ExternalMediaFormatActivity::OnClickListener, Object, IDialogInterfaceOnClickListener)

ExternalMediaFormatActivity::OnClickListener::OnClickListener(
    /* [in] */ ExternalMediaFormatActivity* host)
    : mHost(host)
{
}

ECode ExternalMediaFormatActivity::OnClickListener::OnClick(
    /* [in] */ IDialogInterface* dialog,
    /* [in] */ Int32 which)
{
    return mHost->OnClick(dialog, which);
}

const Int32 ExternalMediaFormatActivity::POSITIVE_BUTTON = IDialogInterface::BUTTON_POSITIVE;

CAR_INTERFACE_IMPL(ExternalMediaFormatActivity, AlertActivity, IExternalMediaFormatActivity,
    IDialogInterfaceOnClickListener)

ExternalMediaFormatActivity::ExternalMediaFormatActivity()
{
    mStorageReceiver = new StorageReceiver(this);
}

ECode ExternalMediaFormatActivity::OnCreate(
    /* [in] */ IBundle* savedInstanceState)
{
    AlertActivity::OnCreate(savedInstanceState);

    // This is necessary because this class's caller,
    // packages/SystemUI/src/com/android/systemui/usb/StorageNotification.java,
    // supplies the path to be erased/formatted as a String, instead of a
    // StorageVolume. This for-loop gets the correct StorageVolume from the
    // given path.
    AutoPtr<IInterface> service;
    GetSystemService(IContext::STORAGE_SERVICE, (IInterface**)&service);
    mStorageManager = IStorageManager::Probe(service);
    AutoPtr<IIntent> intent;
    GetIntent((IIntent**)&intent);
    String path;
    intent->GetStringExtra(FORMAT_PATH, &path);
    AutoPtr<ArrayOf<IStorageVolume*> > volumes;
    mStorageManager->GetVolumeList((ArrayOf<IStorageVolume*>**)&volumes);

    for (Int32 i = 0; i < volumes->GetLength(); i++) {
        AutoPtr<IStorageVolume> sv = (*volumes)[i];
        String str;
        sv->GetPath(&str);
        if (path.Equals(str)) {
            mStorageVolume = sv;
            break;
        }
    }

    Logger::D("ExternalMediaFormatActivity", "onCreate!");
    String str;
    mStorageVolume->GetPath(&str);
    Logger::D("ExternalMediaFormatActivity", "The storage volume to be formatted is : %s", str.string());

    Int32 id;
    mStorageVolume->GetDescriptionId(&id);
    Boolean isUsbStorage = id == R::string::storage_usb;

    // Set up the "dialog"
    AutoPtr<IAlertControllerAlertParams> p = mAlertParams;
    AutoPtr<ICharSequence> charSequence;
    GetText(R::string::extmedia_format_title, (ICharSequence**)&charSequence);
    p->SetTitle(charSequence);

    charSequence = NULL;
    GetText(isUsbStorage ?
            R::string::usb_extmedia_format_message :
            R::string::sd_extmedia_format_message, (ICharSequence**)&charSequence);
    // p.mMessage = String.format(getString(isUsbStorage ?
    //                 com.android.internal.R.string.usb_extmedia_format_message :
    //                 com.android.internal.R.string.sd_extmedia_format_message),
    //         mStorageVolume.getPath());
    p->SetMessage(charSequence);

    charSequence = NULL;
    GetText(R::string::extmedia_format_button_format, (ICharSequence**)&charSequence);
    p->SetPositiveButtonText(charSequence);
    AutoPtr<OnClickListener> listener = new OnClickListener(this);
    p->SetPositiveButtonListener(listener);
    charSequence = NULL;
    GetText(R::string::cancel, (ICharSequence**)&charSequence);
    p->SetNegativeButtonText(charSequence);
    p->SetNegativeButtonListener(listener);
    SetupAlert();
    return NOERROR;
}

ECode ExternalMediaFormatActivity::OnResume()
{
    AlertActivity::OnResume();

    AutoPtr<IIntentFilter> filter;
    CIntentFilter::New((IIntentFilter**)&filter);
    filter->AddAction(IIntent::ACTION_MEDIA_REMOVED);
    filter->AddAction(IIntent::ACTION_MEDIA_CHECKING);
    filter->AddAction(IIntent::ACTION_MEDIA_MOUNTED);
    filter->AddAction(IIntent::ACTION_MEDIA_SHARED);
    AutoPtr<IIntent> intent;
    RegisterReceiver(mStorageReceiver, filter, (IIntent**)&intent);
    return NOERROR;
}

ECode ExternalMediaFormatActivity::OnPause()
{
    AlertActivity::OnPause();

    UnregisterReceiver(mStorageReceiver);
    return NOERROR;
}

ECode ExternalMediaFormatActivity::OnClick(
    /* [in] */ IDialogInterface* dialog,
    /* [in] */ Int32 which)
{

    if (which == POSITIVE_BUTTON) {
        AutoPtr<IIntent> intent;
        CIntent::New(IExternalStorageFormatter::FORMAT_ONLY, (IIntent**)&intent);
        intent->SetComponent(CExternalStorageFormatter::COMPONENT_NAME);
        intent->PutExtra(IStorageVolume::EXTRA_STORAGE_VOLUME, IParcelable::Probe(mStorageVolume));
        AutoPtr<IComponentName> component;
        StartService(intent, (IComponentName**)&component);
    }

    // No matter what, finish the activity
    Finish();
    return NOERROR;
}

} //namespace App
} //namespace Internal
} //namespace Droid
} //namespace Elastos
