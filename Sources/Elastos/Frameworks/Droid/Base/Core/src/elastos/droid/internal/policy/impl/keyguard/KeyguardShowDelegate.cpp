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

#include "elastos/droid/ext/frameworkext.h"
#include "elastos/droid/internal/policy/impl/keyguard/KeyguardShowDelegate.h"
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Internal {
namespace Policy {
namespace Impl {
namespace Keyguard {

CAR_INTERFACE_IMPL(KeyguardShowDelegate, Object, IBinder, IIKeyguardShowCallback)

const Boolean KeyguardShowDelegate::DEBUG = TRUE;

KeyguardShowDelegate::KeyguardShowDelegate()
{
}

ECode KeyguardShowDelegate::constructor(
    /* [in] */ IKeyguardServiceDelegate* host,
    /* [in] */ IKeyguardServiceDelegateShowListener* showListener)
{
    mHost = host;
    mShowListener = showListener;
    return NOERROR;
}

//@Override
ECode KeyguardShowDelegate::OnShown(
    /* [in] */ IBinder* windowToken)
{
    if (DEBUG) Logger::V("KeyguardShowDelegate", "**** SHOWN CALLED ****");
    if (mShowListener != NULL) {
        mShowListener->OnShown(windowToken);
    }
    mHost->HideScrim();
    return NOERROR;
}

ECode KeyguardShowDelegate::ToString(
    /* [out] */ String* info)
{
    return Object::ToString(info);
}

} // namespace Keyguard
} // namespace Impl
} // namespace Policy
} // namespace Internal
} // namespace Droid
} // namespace Elastos
