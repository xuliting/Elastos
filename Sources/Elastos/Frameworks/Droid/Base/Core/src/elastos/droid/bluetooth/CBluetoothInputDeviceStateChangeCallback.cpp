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

#include "elastos/droid/bluetooth/CBluetoothInputDeviceStateChangeCallback.h"
#include "elastos/droid/content/CIntent.h"
#include "elastos/core/AutoLock.h"
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::IIntent;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Core::AutoLock;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Bluetooth {

CAR_INTERFACE_IMPL(CBluetoothInputDeviceStateChangeCallback, Object, IIBluetoothStateChangeCallback, IBinder);

CAR_OBJECT_IMPL(CBluetoothInputDeviceStateChangeCallback);

CBluetoothInputDeviceStateChangeCallback::CBluetoothInputDeviceStateChangeCallback()
{
}

ECode CBluetoothInputDeviceStateChangeCallback::OnBluetoothStateChange(
    /* [in] */ Boolean on)
{
    if (BluetoothInputDevice::DBG) Logger::D(BluetoothInputDevice::TAG, "onBluetoothStateChange: up=%d", on);
    if (!on) {
        if (BluetoothInputDevice::VDBG) Logger::D(BluetoothInputDevice::TAG,"Unbinding service...");
        AutoLock lock(mHost->mConnectionLock);
        // try {
        mHost->mService = NULL;
        ECode ec = mHost->mContext->UnbindService(mHost->mConnection);
        if (FAILED(ec)) {
            Logger::E(BluetoothInputDevice::TAG, "0x%08x", ec);
        }
        // } catch (Exception re) {
        //     Logger::E(BluetoothInputDevice::TAG,"",re);
        // }
    }
    else {
        AutoLock lock(mHost->mConnectionLock);
        // try {
        if (mHost->mService == NULL) {
            if (BluetoothInputDevice::VDBG) Logger::D(BluetoothInputDevice::TAG, "Binding service...");
            //AutoPtr<IIntent> intent;
            //CIntent::New(String("IBluetoothInputDevice")/*IBluetoothInputDevice.class.getName()*/, (IIntent**)&intent);
            //Boolean result;
            //if (mHost->mContext->BindService(intent, mHost->mConnection, 0, &result), !result) {
            //    Logger::E(BluetoothInputDevice::TAG, "Could not bind to Bluetooth HID Service");
            //}
            Boolean bind;
            mHost->DoBind(&bind);
        }
        // } catch (Exception re) {
        //     Logger::E(BluetoothInputDevice::TAG,"",re);
        // }
    }
    return NOERROR;
}

ECode CBluetoothInputDeviceStateChangeCallback::constructor(
    /* [in] */ IInterface* host)
{
    mHost = (BluetoothInputDevice*)(IBluetoothInputDevice::Probe(host));
    return NOERROR;
}

} // namespace Bluetooth
} // namespace Droid
} // namespace Elastos
