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

#ifndef __ELASTOS_DROID_OS_STORAGE_CSTORAGEVOLUMEHELPER_H__
#define __ELASTOS_DROID_OS_STORAGE_CSTORAGEVOLUMEHELPER_H__

#include "_Elastos_Droid_Os_Storage_CStorageVolumeHelper.h"
#include "elastos/droid/os/storage/CStorageVolume.h"
#include "Elastos.Droid.Os.h"
#include "Elastos.Droid.Content.h"

#include <elastos/core/Singleton.h>

using Elastos::Core::Singleton;
using Elastos::IO::IFile;
using Elastos::Droid::Content::IContext;

namespace Elastos {
namespace Droid {
namespace Os {
namespace Storage {

CarClass(CStorageVolumeHelper)
    , public Singleton
    , public IStorageVolumeHelper
{
public:
    CAR_INTERFACE_DECL()

    CAR_SINGLETON_DECL()

    CARAPI FromTemplate(
        /* [in] */ IStorageVolume* temp,
        /* [in] */ IFile* path,
        /* [in] */ IUserHandle* owner,
        /* [out] */ IStorageVolume** volume);
};

} // namespace Storage
} // namespace Os
} // namepsace Droid
} // namespace Elastos


#endif // __ELASTOS_DROID_OS_STORAGE_CSTORAGEVOLUMEHELPER_H__
