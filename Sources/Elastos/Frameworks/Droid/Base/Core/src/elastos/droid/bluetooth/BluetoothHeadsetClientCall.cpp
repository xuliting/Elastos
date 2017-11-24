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

#include "elastos/droid/bluetooth/BluetoothHeadsetClientCall.h"
#include "elastos/core/StringBuilder.h"

using Elastos::Core::StringBuilder;

namespace Elastos {
namespace Droid {
namespace Bluetooth {

//=====================================================================
//                      BluetoothHeadsetClientCall
//=====================================================================
CAR_INTERFACE_IMPL(BluetoothHeadsetClientCall, Object, IBluetoothHeadsetClientCall, IParcelable);

BluetoothHeadsetClientCall::BluetoothHeadsetClientCall()
{
}

ECode BluetoothHeadsetClientCall::constructor()
{
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::constructor(
    /* [in] */ Int32 id,
    /* [in] */ Int32 state,
    /* [in] */ const String& number,
    /* [in] */ Boolean multiParty,
    /* [in] */ Boolean outgoing)
{
    mId = id;
    mState = state;
    mNumber = number;// != null ? number : "";
    mMultiParty = multiParty;
    mOutgoing = outgoing;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::SetState(
    /* [in] */ Int32 state)
{
    mState = state;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::SetNumber(
    /* [in] */ const String& number)
{
    mNumber = number;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::SetMultiParty(
    /* [in] */ Boolean multiParty)
{
    mMultiParty = multiParty;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::GetId(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mId;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::GetState(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mState;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::GetNumber(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mNumber;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::IsMultiParty(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mMultiParty;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::IsOutgoing(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = mOutgoing;
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::ToString(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<StringBuilder> builder = new StringBuilder("BluetoothHeadsetClientCall{mId: ");
    builder->Append(mId);
    builder->Append(", mState: ");
    switch (mState) {
        case CALL_STATE_ACTIVE: builder->Append("ACTIVE"); break;
        case CALL_STATE_HELD: builder->Append("HELD"); break;
        case CALL_STATE_DIALING: builder->Append("DIALING"); break;
        case CALL_STATE_ALERTING: builder->Append("ALERTING"); break;
        case CALL_STATE_INCOMING: builder->Append("INCOMING"); break;
        case CALL_STATE_WAITING: builder->Append("WAITING"); break;
        case CALL_STATE_HELD_BY_RESPONSE_AND_HOLD: builder->Append("HELD_BY_RESPONSE_AND_HOLD"); break;
        case CALL_STATE_TERMINATED: builder->Append("TERMINATED"); break;
        default: builder->Append(mState); break;
    }
    builder->Append(", mNumber: ");
    builder->Append(mNumber);
    builder->Append(", mMultiParty: ");
    builder->Append(mMultiParty);
    builder->Append(", mOutgoing: ");
    builder->Append(mOutgoing);
    builder->Append("}");
    *result = builder->ToString();
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::WriteToParcel(
    /* [in] */ IParcel* out)
{
    out->WriteInt32(mId);
    out->WriteInt32(mState);
    out->WriteString(mNumber);
    out->WriteInt32(mMultiParty ? 1 : 0);
    out->WriteInt32(mOutgoing ? 1 : 0);
    return NOERROR;
}

ECode BluetoothHeadsetClientCall::ReadFromParcel(
    /* [in] */ IParcel* source)
{
    source->ReadInt32(&mId);
    source->ReadInt32(&mState);
    source->ReadString(&mNumber);
    Int32 mp;
    source->ReadInt32(&mp);
    mMultiParty = mp == 1;
    Int32 og;
    source->ReadInt32(&og);
    mOutgoing = og == 1;
    return NOERROR;
}

} // namespace Bluetooth
} // namespace Droid
} // namespace Elastos
