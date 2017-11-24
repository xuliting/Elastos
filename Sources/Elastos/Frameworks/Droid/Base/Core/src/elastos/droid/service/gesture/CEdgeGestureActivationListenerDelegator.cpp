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

#include "elastos/droid/service/gesture/CEdgeGestureActivationListenerDelegator.h"
#include "elastos/droid/service/gesture/EdgeGestureManager.h"

using Elastos::Droid::Os::EIID_IBinder;

namespace Elastos {
namespace Droid {
namespace Service {
namespace Gesture {

CAR_INTERFACE_IMPL(CEdgeGestureActivationListenerDelegator, Object, IIEdgeGestureActivationListener, IBinder)

CAR_OBJECT_IMPL(CEdgeGestureActivationListenerDelegator)

ECode CEdgeGestureActivationListenerDelegator::constructor(
    /* [in] */ IEdgeGestureActivationListener* weakHost)
{
    IWeakReferenceSource* wrs = IWeakReferenceSource::Probe(weakHost);
    wrs->GetWeakReference((IWeakReference**)&mWeakHost);
    return NOERROR;
}

ECode CEdgeGestureActivationListenerDelegator::OnEdgeGestureActivation(
    /* [in] */ Int32 touchX,
    /* [in] */ Int32 touchY,
    /* [in] */ Int32 positionIndex,
    /* [in] */ Int32 flags)
{
    AutoPtr<IEdgeGestureActivationListener> egal;
    mWeakHost->Resolve(EIID_IEdgeGestureActivationListener, (IInterface**)&egal);
    if (egal != NULL) {
        EdgeGestureManager::EdgeGestureActivationListener* listener;
        listener = (EdgeGestureManager::EdgeGestureActivationListener*)egal.Get();
        listener->OnEdgeGestureActivationInner(touchX, touchY, positionIndex, flags);
    }
    return NOERROR;
}

ECode CEdgeGestureActivationListenerDelegator::ToString(
    /* [out] */ String* str)
{
    return Object::ToString(str);
}


} // namespace Gesture
} // namespace Service
} // namepsace Droid
} // namespace Elastos