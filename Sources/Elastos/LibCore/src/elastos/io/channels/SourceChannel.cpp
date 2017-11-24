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

#include "SourceChannel.h"

namespace Elastos {
namespace IO {
namespace Channels {

CAR_INTERFACE_IMPL(SourceChannel, AbstractSelectableChannel, ISourceChannel, IReadableByteChannel, IScatteringByteChannel)

SourceChannel::SourceChannel()
{}

SourceChannel::SourceChannel(
    /* [in] */ ISelectorProvider* provider)
{
    AbstractSelectableChannel::constructor(provider);
    assert(NULL != provider);
}

ECode SourceChannel::GetValidOps(
    /* [out] */ Int32* ret)
{
    VALIDATE_NOT_NULL(ret)

    *ret = ISelectionKey::OP_READ;
    return NOERROR;
}

} // namespace Channels
} // namespace IO
} // namespace Elastoss
