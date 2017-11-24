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

#include "elastos/droid/server/pm/CParcelFileDescriptorFactory.h"
#include "elastos/droid/os/FileUtils.h"
#include <elastos/droid/system/Os.h>
#include <elastos/droid/system/OsConstants.h>
#include <elastos/utility/logging/Slogger.h>

using Elastos::Utility::Logging::Slogger;
using Elastos::Droid::Internal::Os::EIID_IIParcelFileDescriptorFactory;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Droid::Os::FileUtils;
using Elastos::Droid::Os::CParcelFileDescriptor;
using Elastos::Droid::System::Os;
using Elastos::Droid::System::OsConstants;
using Elastos::IO::CFile;

namespace Elastos {
namespace Droid {
namespace Server {
namespace Pm {

CAR_INTERFACE_IMPL(CParcelFileDescriptorFactory, Object, IIParcelFileDescriptorFactory, IBinder)

CAR_OBJECT_IMPL(CParcelFileDescriptorFactory)

ECode CParcelFileDescriptorFactory::constructor(
    /* [in] */ IFile* codeFile)
{
    mCodeFile = codeFile;
    return NOERROR;
}

ECode CParcelFileDescriptorFactory::Open(
    /* [in] */ const String& name,
    /* [in] */ Int32 mode,
    /* [out] */ IParcelFileDescriptor** pfd)
{
    VALIDATE_NOT_NULL(pfd)
    *pfd = NULL;
    if (!FileUtils::IsValidExtFilename(name)) {
        Slogger::E("CPackageMangerService", "Invalid filename: %s", name.string());
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    // try {
    AutoPtr<IFile> file;
    ECode ec = CFile::New(mCodeFile, name, (IFile**)&file);
    if (FAILED(ec)) {
        Slogger::E("CPackageMangerService", "Failed to open: ox%08x", ec);
        return E_REMOTE_EXCEPTION;
    }
    String absolutePath;
    file->GetAbsolutePath(&absolutePath);
    AutoPtr<IFileDescriptor> fd;
    ec = Elastos::Droid::System::Os::Open(absolutePath, OsConstants::_O_RDWR | OsConstants::_O_CREAT, 0644, (IFileDescriptor**)&fd);
    if (FAILED(ec)) {
        Slogger::E("CPackageMangerService", "Failed to open: ox%08x", ec);
        return E_REMOTE_EXCEPTION;
    }
    ec = Elastos::Droid::System::Os::Chmod(absolutePath, 0644);
    if (FAILED(ec)) {
        Slogger::E("CPackageMangerService", "Failed to open: ox%08x", ec);
        return E_REMOTE_EXCEPTION;
    }
    return CParcelFileDescriptor::New(fd, pfd);
    // } catch (ErrnoException e) {
    //     throw new RemoteException( + e.getMessage());
    // }
}

ECode CParcelFileDescriptorFactory::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str);
    return Object::ToString(str);
}

} // namespace Pm
} // namespace Server
} // namespace Droid
} // namespace Elastos
