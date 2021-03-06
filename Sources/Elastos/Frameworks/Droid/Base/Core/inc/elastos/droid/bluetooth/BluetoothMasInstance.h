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

#ifndef __ELASTOS_DROID_BLUETOOTH_BLUETOOTHMASINSTANCE_H__
#define __ELASTOS_DROID_BLUETOOTH_BLUETOOTHMASINSTANCE_H__

#include "Elastos.Droid.Bluetooth.h"
#include <elastos/core/Object.h>
#include "elastos/droid/ext/frameworkext.h"

namespace Elastos {
namespace Droid {
namespace Bluetooth {

/** @hide */
class BluetoothMasInstance
    : public Object
    , public IBluetoothMasInstance
    , public IParcelable
{
public:
    CAR_INTERFACE_DECL()

    BluetoothMasInstance();

    CARAPI constructor();

    CARAPI constructor(
        /* [in] */ Int32 id,
        /* [in] */ const String& name,
        /* [in] */ Int32 channel,
        /* [in] */ Int32 msgTypes);

    // @Override
    CARAPI Equals(
        /* [in] */ IInterface* other,
        /* [out] */ Boolean* result);

    // @Override
    CARAPI GetHashCode(
        /* [out] */ Int32* hashCode);

    // @Override
    CARAPI ToString(
        /* [out] */ String* info);

    CARAPI WriteToParcel(
        /* [in] */ IParcel* out);
    //    /* [in] */ Int32 flags);

    // @Override
    CARAPI ReadFromParcel(
        /* [in] */ IParcel* source);

    CARAPI GetId(
        /* [out] */ Int32* result);

    CARAPI GetName(
        /* [out] */ String* result);

    CARAPI GetChannel(
        /* [out] */ Int32* result);

    CARAPI GetMsgTypes(
        /* [out] */ Int32* result);

    CARAPI MsgSupported(
        /* [in] */ Int32 msg,
        /* [out] */ Boolean* result);

private:
    Int32 mId;
    String mName;
    Int32 mChannel;
    Int32 mMsgTypes;
};

} // namespace Bluetooth
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_BLUETOOTH_BLUETOOTHMASINSTANCE_H__
