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

#include "MonkeyTrackballEvent.h"

namespace Elastos {
namespace Droid {
namespace Commands {
namespace Monkey {

MonkeyTrackballEvent::MonkeyTrackballEvent()
{}

MonkeyTrackballEvent::MonkeyTrackballEvent(
    /* [in] */ Int32 action)
    : MonkeyMotionEvent(IMonkeyEvent::EVENT_TYPE_TRACKBALL
                        , IInputDevice::SOURCE_TRACKBALL
                        , action)
{}

CARAPI MonkeyTrackballEvent::Init(
    /* [in] */ Int32 action)
{
    MonkeyMotionEvent::Init(
        IMonkeyEvent::EVENT_TYPE_TRACKBALL
        , IInputDevice::SOURCE_TRACKBALL
        , action);
    return NOERROR;
}

String MonkeyTrackballEvent::GetTypeLabel()
{
    return String("Trackball");
}

} // namespace Monkey
} // namespace Commands
} // namespace Droid
} // namespace Elastos

