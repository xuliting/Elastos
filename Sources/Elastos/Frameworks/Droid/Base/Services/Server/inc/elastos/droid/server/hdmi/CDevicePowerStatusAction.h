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

#ifndef __ELASTOS_DROID_SERVER_HDMI_CDEVICEPOWERSTATUSACTION_H__
#define __ELASTOS_DROID_SERVER_HDMI_CDEVICEPOWERSTATUSACTION_H__

#include "_Elastos_Droid_Server_Hdmi_CDevicePowerStatusAction.h"
#include "elastos/droid/server/hdmi/DevicePowerStatusAction.h"

namespace Elastos {
namespace Droid {
namespace Server {
namespace Hdmi {

/**
 * Feature action that queries the power status of other device. This action is initiated via
 * {@link HdmiPlaybackClient#queryDisplayStatus(DisplayStatusCallback)} from the Android system
 * working as playback device to get the power status of TV device.
 * <p>
 * Package-private, accessed by {@link HdmiControlService} only.
 */
CarClass(CDevicePowerStatusAction)
    , public DevicePowerStatusAction
{
public:
    CAR_OBJECT_DECL()
};

} // namespace Hdmi
} // namespace Server
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_SERVER_HDMI_CDEVICEPOWERSTATUSACTION_H__
