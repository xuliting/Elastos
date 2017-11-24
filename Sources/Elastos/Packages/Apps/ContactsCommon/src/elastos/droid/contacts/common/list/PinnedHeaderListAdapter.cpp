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

#include "PinnedHeaderListAdapter.h"

namespace Elastos{
namespace Apps{
namespace Contacts {
namespace Common {
namespace List {

CAR_INTERFACE_IMPL(PinnedHeaderListAdapter, Object, IPinnedHeaderListAdapter, IPinnedHeaderAdapter)

PinnedHeaderListAdapter::~PinnedHeaderListAdapter
{}

ECode PinnedHeaderListAdapter::constructor(
    /* [in] */ IContext* context)
{
    assert(0 && "TODO");
    // return CompositeCursorAdapter::constructor(context);
    return NOERROR;
}

ECode PinnedHeaderListAdapter::constructor(
    /* [in] */ IContext* context,
    /* [in] */ Int32 initialCapacity)
{
    assert(0 && "TODO");
    // return CompositeCursorAdapter::constructor(context, initialCapacity);
    return NOERROR;
}

ECode PinnedHeaderListAdapter::GetPinnedPartitionHeadersEnabled(
    /* [out] */ Boolean* result)
{
    VALUE_NOT_NULL(result);

    *result = mPinnedPartitionHeadersEnabled;
    return NOERROR;
}

ECode PinnedHeaderListAdapter::SetPinnedPartitionHeadersEnabled(
    /* [in] */ Boolean flag)
{
    mPinnedPartitionHeadersEnabled = flag;
    return NOERROR;
}

ECode PinnedHeaderListAdapter::GetPinnedHeaderCount(
    /* [out] */ Int32* count)
{
    VALUE_NOT_NULL(count);

    if (mPinnedPartitionHeadersEnabled) {
        assert(0 && "TODO");
        // *count = GetPartitionCount();
    } else {
        *count =  0;
    }

    return NOERROR;
}

Boolean PinnedHeaderListAdapter::IsPinnedPartitionHeaderVisible(
    /* [in] */ Int32 partition)
{
    assert(0 && "TODO");
    Boolean enabled;
    GetPinnedPartitionHeadersEnabled(&enabled);
    // return enabled && HasHeader(partition)
    //             && !IsPartitionEmpty(partition);
    return FALSE;
}

ECode PinnedHeaderListAdapter::GetPinnedHeaderView(
    /* [in] */ Int32 partition,
    /* [in] */ IView* convertView,
    /* [in] */ IViewGroup* parent,
    /* [out] */ IView** result)
{
    VALUE_NOT_NULL(result);

    assert(0 && "TODO");
    // if (HasHeader(partition)) {
    //     AutoPtr<IView> view;
    //     if (convertView != NULL) {
    //         AutoPtr<IInterface> tag;
    //         convertView->GetTag((IInterface**)&tag);
    //         IInteger32* headerType = IInteger32::Probe(tag);
    //         if (headerType != NULL && CoreUtils::Unbox(headerType) == PARTITION_HEADER_TYPE) {
    //             view = convertView;
    //         }
    //     }
    //     if (view == NULL) {
    //         AutoPtr<IContext> context;
    //         GetContext((IContext**)&context);
    //         NewHeaderView(context, partition, NULL, parent, (IView**)&view);
    //         view->SetTag(PARTITION_HEADER_TYPE);
    //         view->SetFocusable(FALSE);
    //         view->SetEnabled(FALSE);
    //     }
    //     AutoPtr<ICursor> cursor;
    //     GetCursor(partition, (ICursor**)&cursor);
    //     BindHeaderView(view, partition, cursor);
    //     Int32 layoutDirection;
    //     parent->GetLayoutDirection(&layoutDirection);
    //     view->SetLayoutDirection(layoutDirection);
    //     *result = view;
    //     REFCOUNT_ADD(*result);
    // }
    // else {
    //     *result = NULL;
    // }

    return NOERROR;
}

ECode PinnedHeaderListAdapter::ConfigurePinnedHeaders(
    /* [in] */ IPinnedHeaderListView* listView)
{
    Boolean result;
    GetPinnedPartitionHeadersEnabled(&result);
    if (!result) {
        return NOERROR;
    }

    Int32 size;
    assert(0 && "TODO");
    // GetPartitionCount(&size);

    // Cache visibility bits, because we will need them several times later on
    if (mHeaderVisibility == NULL || mHeaderVisibility.GetLength() != size) {
        mHeaderVisibility = ArrayOf<Boolean>::Alloc(size);
    }
    for (Int32 i = 0; i < size; i++) {
        Boolean visible = IsPinnedPartitionHeaderVisible(i);
        (*mHeaderVisibility)[i] = visible;
        if (!visible) {
            listView->SetHeaderInvisible(i, TRUE);
        }
    }

    Int32 headerViewsCount;
    listView->GetHeaderViewsCount(&headerViewsCount);

    // Starting at the top, find and pin headers for partitions preceding the visible one(s)
    Int32 maxTopHeader = -1;
    Int32 topHeaderHeight = 0;
    for (Int32 i = 0; i < size; i++) {
        if ((*mHeaderVisibility)[i]) {
            Int32 result;
            listView->GetPositionAt(topHeaderHeight, &result);
            Int32 position = result - headerViewsCount;
            Int32 partition = GetPartitionForPosition(position);
            if (i > partition) {
                break;
            }

            listView->SetHeaderPinnedAtTop(i, topHeaderHeight, FALSE);
            Int32 height;
            listView->GetPinnedHeaderHeight(i, &height);
            topHeaderHeight += height;
            maxTopHeader = i;
        }
    }

    // Starting at the bottom, find and pin headers for partitions following the visible one(s)
    Int32 maxBottomHeader = size;
    Int32 bottomHeaderHeight = 0;
    Int32 listHeight;
    listView->GetHeight(&listHeight);
    for (Int32 i = size; --i > maxTopHeader;) {
        if ((*mHeaderVisibility)[i]) {
            Int32 result;
            listView->GetPositionAt(listHeight - bottomHeaderHeight, &result)
            Int32 position = result - headerViewsCount;
            if (position < 0) {
                break;
            }

            Int32 partition = GetPartitionForPosition(position - 1);
            if (partition == -1 || i <= partition) {
                break;
            }

            Int32 height;
            listView->GetPinnedHeaderHeight(i, &height);
            bottomHeaderHeight += height;

            listView->SetHeaderPinnedAtBottom(i, listHeight - bottomHeaderHeight, FALSE);
            maxBottomHeader = i;
        }
    }

    // Headers in between the top-pinned and bottom-pinned should be hidden
    for (Int32 i = maxTopHeader + 1; i < maxBottomHeader; i++) {
        if ((*mHeaderVisibility)[i]) {
            // listView->SetHeaderInvisible(i, IsPartitionEmpty(i));
        }
    }

    return NOERROR;
}

ECode PinnedHeaderListAdapter::GetScrollPositionForHeader(
    /* [in] */ Int32 viewIndex,
    /* [out] */ Int32* result)
{
    assert(0 && "TODO");
    // return getPositionForPartition(viewIndex);
    return NOERROR;
}

} // List
} // Common
} // Contacts
} // Apps
} // Elastos
