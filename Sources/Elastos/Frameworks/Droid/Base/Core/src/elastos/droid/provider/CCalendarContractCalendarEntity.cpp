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

#include "Elastos.Droid.Database.h"
#include "elastos/droid/content/CContentValues.h"
#include "elastos/droid/content/CEntity.h"
#include "elastos/droid/database/DatabaseUtils.h"
#include "elastos/droid/net/Uri.h"
#include "elastos/droid/provider/CCalendarContractCalendarEntity.h"
#include "elastos/droid/provider/ContactsContract.h"
#include <elastos/core/StringBuilder.h>

using Elastos::Droid::Content::CContentValues;
using Elastos::Droid::Content::CEntity;
using Elastos::Droid::Content::EIID_ICursorEntityIterator;
using Elastos::Droid::Content::EIID_IEntityIterator;
using Elastos::Droid::Content::IContentValues;
using Elastos::Droid::Content::ICursorEntityIterator;
using Elastos::Droid::Content::IEntityIterator;
using Elastos::Droid::Database::DatabaseUtils;
using Elastos::Droid::Net::Uri;
using Elastos::Utility::EIID_IIterator;
using Elastos::Core::StringBuilder;

namespace Elastos {
namespace Droid {
namespace Provider {

CAR_INTERFACE_IMPL(CCalendarContractCalendarEntity::EntityIteratorImpl, Object, ICursorEntityIterator, IEntityIterator, IIterator)

CCalendarContractCalendarEntity::EntityIteratorImpl::EntityIteratorImpl()
{
}

CCalendarContractCalendarEntity::EntityIteratorImpl::~EntityIteratorImpl()
{
}

ECode CCalendarContractCalendarEntity::EntityIteratorImpl::constructor(
    /* [in] */ ICursor* cursor)
{
    return CursorEntityIterator::constructor(cursor);
}

ECode CCalendarContractCalendarEntity::EntityIteratorImpl::GetEntityAndIncrementCursor(
    /* [in] */ ICursor* cursor,
    /* [out] */ IEntity** entity)
{
    // we expect the cursor is already at the row we need to read from

    Int32 columnIndex;
    FAIL_RETURN(cursor->GetColumnIndexOrThrow(IBaseColumns::ID, &columnIndex))
    Int64 calendarId;
    FAIL_RETURN(cursor->GetInt64(columnIndex, &calendarId))

    // Create the content value
    AutoPtr<IContentValues> cv;
    FAIL_RETURN(CContentValues::New((IContentValues**)&cv))
    FAIL_RETURN(cv->Put(IBaseColumns::ID, calendarId))

    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ACCOUNT_NAME);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ACCOUNT_TYPE);

    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, _SYNC_ID);
    DatabaseUtils::CursorInt64ToContentValuesIfPresent(cursor, cv, DIRTY);

    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC1);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC2);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC3);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC4);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC5);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC6);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC7);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC8);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC9);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendarSyncColumns::CAL_SYNC10);

    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendars::NAME);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, CALENDAR_DISPLAY_NAME);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, CALENDAR_COLOR);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, CALENDAR_ACCESS_LEVEL);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, VISIBLE);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, SYNC_EVENTS);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ICalendarContractCalendars::CALENDAR_LOCATION);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, CALENDAR_TIME_ZONE);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, OWNER_ACCOUNT);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, CAN_ORGANIZER_RESPOND);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, CAN_MODIFY_TIME_ZONE);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, MAX_REMINDERS);
    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, CAN_PARTIALLY_UPDATE);
    DatabaseUtils::CursorStringToContentValuesIfPresent(cursor, cv, ALLOWED_REMINDERS);

    DatabaseUtils::CursorInt32ToContentValuesIfPresent(cursor, cv, DELETED);

    // Create the Entity from the ContentValue
    AutoPtr<IEntity> _entity;
    FAIL_RETURN(CEntity::New(cv, (IEntity**)&_entity))

    // Set cursor to next row
    Boolean result;
    FAIL_RETURN(cursor->MoveToNext(&result))

    // Return the created Entity
    *entity = _entity;
    REFCOUNT_ADD(*entity);
    return NOERROR;
}

CAR_SINGLETON_IMPL(CCalendarContractCalendarEntity)

CAR_INTERFACE_IMPL(CCalendarContractCalendarEntity, Singleton
    , ICalendarContractCalendarEntity
    , IBaseColumns
    , ICalendarContractSyncColumns
    , ICalendarContractCalendarColumns)

ECode CCalendarContractCalendarEntity::GetCONTENT_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    StringBuilder builder;
    builder += "content://";
    builder += ICalendarContract::AUTHORITY;
    builder += "/calendar_entities";
    String str = builder.ToString();
    return Uri::Parse(str, uri);
}

ECode CCalendarContractCalendarEntity::NewEntityIterator(
    /* [in] */ ICursor* cursor,
    /* [out] */ IEntityIterator** iter)
{
    VALIDATE_NOT_NULL(iter);

    AutoPtr<EntityIteratorImpl> impl = new EntityIteratorImpl();
    impl->constructor(cursor);

    *iter = (IEntityIterator*)impl.Get();
    REFCOUNT_ADD(*iter);
    return NOERROR;
}

} //Provider
} //Droid
} //Elastos