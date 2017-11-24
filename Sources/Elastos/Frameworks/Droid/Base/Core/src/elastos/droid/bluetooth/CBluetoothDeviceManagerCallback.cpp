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

#include "elastos/droid/bluetooth/CBluetoothDeviceManagerCallback.h"
#include "elastos/droid/bluetooth/CBluetoothDevice.h"
#include "elastos/core/AutoLock.h"

using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Core::AutoLock;

namespace Elastos {
namespace Droid {
namespace Bluetooth {

CAR_INTERFACE_IMPL(CBluetoothDeviceManagerCallback, Object, IIBluetoothManagerCallback, IBinder);

CAR_OBJECT_IMPL(CBluetoothDeviceManagerCallback);

CBluetoothDeviceManagerCallback::CBluetoothDeviceManagerCallback()
{
}

ECode CBluetoothDeviceManagerCallback::OnBluetoothServiceUp(
    /* [in] */ IIBluetooth* bluetoothService)
{
    AutoLock lock(CBluetoothDevice::sLock);
    CBluetoothDevice::sService = bluetoothService;
    return NOERROR;
}

ECode CBluetoothDeviceManagerCallback::OnBluetoothServiceDown()
{
    AutoLock lock(CBluetoothDevice::sLock);
    CBluetoothDevice::sService = NULL;
    return NOERROR;
}

} // namespace Bluetooth
} // namespace Droid
} // namespace Elastos
