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

#include "elastos/droid/server/CSerialService.h"
#include "elastos/droid/ext/frameworkdef.h"
#include <elastos/utility/etl/List.h>
#include <elastos/utility/logging/Logger.h>
#include <elastos/utility/logging/Slogger.h>
#include "elastos/droid/Manifest.h"
#include "elastos/droid/R.h"
#include <fcntl.h>
#include <Elastos.CoreLibrary.IO.h>
#include <Elastos.CoreLibrary.Utility.h>

using Elastos::Droid::Os::CParcelFileDescriptor;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Droid::Hardware::EIID_IISerialManager;
using Elastos::Droid::Content::Res::IResources;
using Elastos::IO::CFile;
using Elastos::IO::IFile;
using Elastos::IO::IFileDescriptor;
using Elastos::IO::CFileDescriptor;
using Elastos::Utility::Logging::Logger;
using Elastos::Utility::Logging::Slogger;
using Elastos::Utility::Etl::List;

namespace Elastos {
namespace Droid {
namespace Server {

CAR_INTERFACE_IMPL(CSerialService, Object, IISerialManager, IBinder)

CAR_OBJECT_IMPL(CSerialService)

ECode CSerialService::constructor(
    /* [in] */ IContext* context)
{
    mContext = context;

    AutoPtr<IResources> res;
    context->GetResources((IResources**)&res);
    assert(res!=NULL);
    return res->GetStringArray(R::array::config_serialPorts, (ArrayOf<String>**)&mSerialPorts);
}

ECode CSerialService::GetSerialPorts(
    /* [out, callee] */ ArrayOf<String>** serialPorts)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::SERIAL_PORT, String(NULL)));

    List<String> ports;
    Int32 length = mSerialPorts->GetLength();
    for (Int32 i = 0; i < length; i++) {
        String path((*mSerialPorts)[i]);
        AutoPtr<IFile> file;
        CFile::New(path, (IFile**)&file);
        Boolean isExist;
        file->Exists(&isExist);
        if (isExist) {
            ports.PushBack(path);
        }
    }

    *serialPorts = ArrayOf<String>::Alloc(ports.GetSize());
    REFCOUNT_ADD(*serialPorts);
    List<String>::Iterator it = ports.Begin();
    for (Int32 i = 0; it != ports.End(); ++it, i++) {
        (**serialPorts)[i] = *it;
    }

    return NOERROR;
}

ECode CSerialService::OpenSerialPort(
    /* [in] */ const String& path,
    /* [out] */ IParcelFileDescriptor** descriptor)
{
    VALIDATE_NOT_NULL(descriptor);
    *descriptor = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::SERIAL_PORT, String(NULL)));

    for (Int32 i = 0; i < mSerialPorts->GetLength(); i++) {
        if ((*mSerialPorts)[i].Equals(path)) {
            return NativeOpen(path, descriptor);
        }
    }

    Logger::E("CSerialService", "Invalid serial port ", path.string());
    return E_ILLEGAL_ARGUMENT_EXCEPTION;
}

ECode CSerialService::NativeOpen(
    /* [in] */ const String& path,
    /* [out] */ IParcelFileDescriptor** descriptor)
{
    VALIDATE_NOT_NULL(descriptor);
    *descriptor = NULL;

    Int32 fd = open(path.string(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        Logger::E("CSerialService", "could not open %s", path.string());
        *descriptor = NULL;
        return NOERROR;
    }

    AutoPtr<IFileDescriptor> fileDes;
    CFileDescriptor::New((IFileDescriptor**)&fileDes);
    fileDes->SetDescriptor(fd);
    return CParcelFileDescriptor::New(fileDes, descriptor);
}

} // Server
} // Droid
} // Elastos
