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

#ifndef __ELASTOS_DROID_BLUETOOTH_LE_BLUETOOTHLEUTILS_H__
#define __ELASTOS_DROID_BLUETOOTH_LE_BLUETOOTHLEUTILS_H__

#include "elastos/droid/ext/frameworkext.h"
#include <elastos/core/Object.h>

using Elastos::Droid::Bluetooth::IBluetoothAdapter;
using Elastos::Droid::Utility::ISparseArray;
// import java.util.Arrays;
// import java.util.Iterator;
using Elastos::Utility::IMap;
// import java.util.Objects;
// import java.util.ISet;


namespace Elastos {
namespace Droid {
namespace Bluetooth {
namespace LE {

/**
  * Helper class for Bluetooth LE utils.
  *
  * @hide
  */
class BluetoothLeUtils
{
public:
    /**
      * Returns a string composed from a {@link Map}.
      */
    static CARAPI_(String) ToString(
        /* [in] */ IMap* map); // IInterface*, ArrayOf<Byte>

    /**
      * Check whether two {@link Map} equal.
      */
    static CARAPI_(Boolean) Equals(
        /* [in] */ IMap* map, // IInteface*, ArrayOf<Byte>
        /* [in] */ IMap* otherMap);// IInterface*, ArrayOf<Byte>

    /**
      * Returns a string composed from a {@link SparseArray}.
      */
    static CARAPI_(String) ToString(
        /* [in] */ ISparseArray* array);// ArrayOf<Byte>

    /**
      * Check whether two {@link SparseArray} equal.
      */
    static CARAPI_(Boolean) Equals(
        /* [in] */ ISparseArray* array, // ArrayOf<Byte>
        /* [in] */ ISparseArray* otherArray); // ArrayOf<Byte>

    /**
      * Ensure Bluetooth is turned on.
      *
      * @throws IllegalStateException If {@code adapter} is null or Bluetooth state is not
      *             {@link BluetoothAdapter#STATE_ON}.
      */
    static CARAPI CheckAdapterStateOn(
        /* [in] */ IBluetoothAdapter* adapter);
};

} // namespace LE
} // namespace Bluetooth
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_BLUETOOTH_LE_BLUETOOTHLEUTILS_H__
