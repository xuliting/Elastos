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

#include "elastos/droid/app/ExpandableListActivity.h"
#include "elastos/droid/R.h"
#include "elastos/core/AutoLock.h"
#include <elastos/utility/logging/Logger.h>

#include <elastos/core/AutoLock.h>
using Elastos::Core::AutoLock;
using Elastos::Droid::R;
using Elastos::Droid::Widget::IAdapterView;
using Elastos::Droid::Widget::EIID_IExpandableListViewOnChildClickListener;
using Elastos::Droid::Widget::EIID_IExpandableListViewOnGroupCollapseListener;
using Elastos::Droid::Widget::EIID_IExpandableListViewOnGroupExpandListener;

using Elastos::Utility::IList;
using Elastos::Utility::IMap;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace App {


CAR_INTERFACE_IMPL(ExpandableListActivity::InnerListener, Object, \
    IExpandableListViewOnChildClickListener, \
    IExpandableListViewOnGroupCollapseListener, \
    IExpandableListViewOnGroupExpandListener)

ExpandableListActivity::InnerListener::InnerListener(
    /* [in] */ ExpandableListActivity* host)
    : mHost(host)
{}

ECode ExpandableListActivity::InnerListener::OnChildClick(
    /* [in] */ IExpandableListView* parent,
    /* [in] */ IView* v,
    /* [in] */ Int32 groupPosition,
    /* [in] */ Int32 childPosition,
    /* [in] */ Int64 id,
    /* [out] */ Boolean* result)
{
    return mHost->OnChildClick(parent, v, groupPosition, childPosition, id, result);
}

ECode ExpandableListActivity::InnerListener::OnGroupCollapse(
    /* [in] */ Int32 groupPosition)
{
    return mHost->OnGroupCollapse(groupPosition);
}

ECode ExpandableListActivity::InnerListener::OnGroupExpand(
    /* [in] */ Int32 groupPosition)
{
    return mHost->OnGroupExpand(groupPosition);
}

CAR_INTERFACE_IMPL(ExpandableListActivity, Activity, IExpandableListActivity)

ExpandableListActivity::ExpandableListActivity()
    : mFinishedStart(FALSE)
{}

ExpandableListActivity::~ExpandableListActivity()
{}

ECode ExpandableListActivity::OnCreateContextMenu(
    /* [in] */ IContextMenu* menu,
    /* [in] */ IView* v,
    /* [in] */ IContextMenuInfo* menuInfo)
{
    return Activity::OnCreateContextMenu(menu, v, menuInfo);
}

ECode ExpandableListActivity::OnChildClick(
    /* [in] */ IExpandableListView* parent,
    /* [in] */ IView* v,
    /* [in] */ Int32 groupPosition,
    /* [in] */ Int32 childPosition,
    /* [in] */ Int64 id,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    *result = FALSE;
    return NOERROR;
}

ECode ExpandableListActivity::OnGroupCollapse(
    /* [in] */ Int32 groupPosition)
{
    return NOERROR;
}

ECode ExpandableListActivity::OnGroupExpand(
    /* [in] */ Int32 groupPosition)
{
    return NOERROR;
}

ECode ExpandableListActivity::OnRestoreInstanceState(
    /* [in] */ IBundle* state)
{
    FAIL_RETURN(EnsureList())
    return Activity::OnRestoreInstanceState(state);
}

ECode ExpandableListActivity::OnContentChanged()
{
    Activity::OnContentChanged();
    AutoPtr<IView> emptyView, listView;
    FindViewById(R::id::empty, (IView**)&emptyView);
    FindViewById(R::id::list, (IView**)&listView);
    mList = IExpandableListView::Probe(listView);
    if (mList == NULL) {
        Logger::E("ExpandableListActivity", "Your content must have a ExpandableListView"
            " whose id attribute is 'android.R.id.list'");
        return E_RUNTIME_EXCEPTION;
    }
    if (emptyView != NULL) {
        IAdapterView::Probe(mList)->SetEmptyView(emptyView);
    }

    if (mListener == NULL) {
        mListener = new InnerListener(this);
    }
    mList->SetOnChildClickListener(mListener);
    mList->SetOnGroupExpandListener(mListener);
    mList->SetOnGroupCollapseListener(mListener);

    if (mFinishedStart) {
        SetListAdapter(mAdapter);
    }
    mFinishedStart = TRUE;
    return NOERROR;
}

ECode ExpandableListActivity::SetListAdapter(
    /* [in] */ IExpandableListAdapter* adapter)
{
    {    AutoLock syncLock(this);
        FAIL_RETURN(EnsureList())
        mAdapter = adapter;
        mList->SetAdapter(adapter);
    }
    return NOERROR;
}

ECode ExpandableListActivity::GetExpandableListView(
    /* [out] */ IExpandableListView** view)
{
    FAIL_RETURN(EnsureList())
    VALIDATE_NOT_NULL(view)
    *view = mList;
    REFCOUNT_ADD(*view)
    return NOERROR;
}

ECode ExpandableListActivity::GetExpandableListAdapter(
    /* [out] */ IExpandableListAdapter** adapter)
{
    VALIDATE_NOT_NULL(adapter)
    *adapter = mAdapter;
    REFCOUNT_ADD(*adapter)
    return NOERROR;
}

ECode ExpandableListActivity::EnsureList()
{
    if (mList != NULL) {
        return NOERROR;
    }
    return SetContentView(R::layout::expandable_list_content);
}

ECode ExpandableListActivity::GetSelectedId(
    /* [out] */ Int64* result)
{
    return mList->GetSelectedId(result);
}

ECode ExpandableListActivity::GetSelectedPosition(
    /* [out] */ Int64* result)
{
    return mList->GetSelectedPosition(result);
}

ECode ExpandableListActivity::SetSelectedChild(
    /* [in] */ Int32 groupPosition,
    /* [in] */ Int32 childPosition,
    /* [in] */ Boolean shouldExpandGroup,
    /* [out] */ Boolean* result)
{
    return mList->SetSelectedChild(groupPosition, childPosition, shouldExpandGroup, result);
}

ECode ExpandableListActivity::SetSelectedGroup(
    /* [in] */ Int32 groupPosition)
{
    return mList->SetSelectedGroup(groupPosition);
}

} // namespace App
} // namespace Droid
} // namespace Elastos
