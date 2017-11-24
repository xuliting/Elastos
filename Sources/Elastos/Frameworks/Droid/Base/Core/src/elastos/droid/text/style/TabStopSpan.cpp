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

#include "elastos/droid/text/style/TabStopSpan.h"
#include "elastos/droid/ext/frameworkext.h"

namespace Elastos {
namespace Droid {
namespace Text {
namespace Style {


//=====================================================================
// TabStopSpan
//=====================================================================

CAR_INTERFACE_IMPL(TabStopSpan, Object, ITabStopSpan, IParagraphStyle)

TabStopSpan::TabStopSpan()
{}

TabStopSpan::~TabStopSpan()
{}

//=====================================================================
// TabStopSpanStandard
//=====================================================================
CAR_INTERFACE_IMPL(TabStopSpanStandard, TabStopSpan, ITabStopSpanStandard)

TabStopSpanStandard::TabStopSpanStandard()
    : mTab(0)
{}

TabStopSpanStandard::~TabStopSpanStandard()
{}

ECode TabStopSpanStandard::constructor(
    /* [in] */ Int32 where)
{
    mTab = where;
    return NOERROR;
}

ECode TabStopSpanStandard::GetTabStop(
    /* [out] */ Int32* ret)
{
    VALIDATE_NOT_NULL(ret)
    *ret = mTab;
    return NOERROR;
}

} // namespace Style
} // namespace Text
} // namepsace Droid
} // namespace Elastos