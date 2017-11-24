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

#include "elastos/droid/provider/CTelephonySms.h"
#include "elastos/droid/provider/Telephony.h"
#include "Elastos.Droid.Net.h"

namespace Elastos {
namespace Droid {
namespace Provider {

CAR_INTERFACE_IMPL(CTelephonySms, Singleton, ITelephonySms, IBaseColumns, ITelephonyTextBasedSmsColumns);
CAR_SINGLETON_IMPL(CTelephonySms);

ECode CTelephonySms::GetDefaultSmsPackage(
    /* [in] */ IContext* context,
    /* [out] */ String* smsPackage)
{
    return Telephony::Sms::GetDefaultSmsPackage(context, smsPackage);
}

/**
 * Return cursor for table query.
 * @hide
 */
ECode CTelephonySms::Query(
    /* [in] */ IContentResolver* cr,
    /* [in] */ ArrayOf<String>* projection,
    /* [out] */ ICursor** cursor)
{
    return Telephony::Sms::Query(cr, projection, cursor);
}

/**
 * Return cursor for table query.
 * @hide
 */
ECode CTelephonySms::Query(
    /* [in] */ IContentResolver* cr,
    /* [in] */ ArrayOf<String>* projection,
    /* [in] */ const String& where,
    /* [in] */ const String& orderBy,
    /* [out] */ ICursor** cursor)
{
    return Telephony::Sms::Query(cr, projection, where, orderBy, cursor);
}

/**
 * Add an SMS to the given URI.
 *
 * @param resolver the content resolver to use
 * @param uri the URI to add the message to
 * @param address the address of the sender
 * @param body the body of the message
 * @param subject the pseudo-subject of the message
 * @param date the timestamp for the message
 * @param read TRUE if the message has been read, FALSE if not
 * @param deliveryReport TRUE if a delivery report was requested, FALSE if not
 * @return the URI for the new message
 * @hide
 */
ECode CTelephonySms::AddMessageToUri(
    /* [in] */ IContentResolver* resolver,
    /* [in] */ IUri* uri,
    /* [in] */ const String& address,
    /* [in] */ const String& body,
    /* [in] */ const String& subject,
    /* [in] */ Int64 date,
    /* [in] */ Boolean read,
    /* [in] */ Boolean deliveryReport,
    /* [out] */ IUri** _uri)
{
    return Telephony::Sms::AddMessageToUri(resolver, uri, address, body, subject, date, read, deliveryReport, _uri);
}

/**
 * Add an SMS to the given URI.
 *
 * @param resolver the content resolver to use
 * @param uri the URI to add the message to
 * @param address the address of the sender
 * @param body the body of the message
 * @param subject the psuedo-subject of the message
 * @param date the timestamp for the message
 * @param read TRUE if the message has been read, FALSE if not
 * @param deliveryReport TRUE if a delivery report was requested, FALSE if not
 * @param subId the sub_id which the message belongs to
 * @return the URI for the new message
 * @hide
 */
ECode CTelephonySms::AddMessageToUri(
    /* [in] */ Int64 subId,
    /* [in] */ IContentResolver* resolver,
    /* [in] */ IUri* uri,
    /* [in] */ const String& address,
    /* [in] */ const String& body,
    /* [in] */ const String& subject,
    /* [in] */ Int64 date,
    /* [in] */ Boolean read,
    /* [in] */ Boolean deliveryReport,
    /* [out] */ IUri** _uri)
{
    return Telephony::Sms::AddMessageToUri(subId, resolver, uri, address, body, subject, date, read, deliveryReport, _uri);
}

/**
 * Add an SMS to the given URI with the specified thread ID.
 *
 * @param resolver the content resolver to use
 * @param uri the URI to add the message to
 * @param address the address of the sender
 * @param body the body of the message
 * @param subject the pseudo-subject of the message
 * @param date the timestamp for the message
 * @param read TRUE if the message has been read, FALSE if not
 * @param deliveryReport TRUE if a delivery report was requested, FALSE if not
 * @param threadId the thread_id of the message
 * @return the URI for the new message
 * @hide
 */
ECode CTelephonySms::AddMessageToUri(
    /* [in] */ IContentResolver* resolver,
    /* [in] */ IUri* uri,
    /* [in] */ const String& address,
    /* [in] */ const String& body,
    /* [in] */ const String& subject,
    /* [in] */ Int64 date,
    /* [in] */ Boolean read,
    /* [in] */ Boolean deliveryReport,
    /* [in] */ Int64 threadId,
    /* [out] */ IUri** _uri)
{
    return Telephony::Sms::AddMessageToUri(resolver, uri, address, body, subject, date, read, deliveryReport, threadId, _uri);
}

/**
 * Add an SMS to the given URI with thread_id specified.
 *
 * @param resolver the content resolver to use
 * @param uri the URI to add the message to
 * @param address the address of the sender
 * @param body the body of the message
 * @param subject the psuedo-subject of the message
 * @param date the timestamp for the message
 * @param read TRUE if the message has been read, FALSE if not
 * @param deliveryReport TRUE if a delivery report was requested, FALSE if not
 * @param threadId the thread_id of the message
 * @param subId the sub_id which the message belongs to
 * @return the URI for the new message
 * @hide
 */
ECode CTelephonySms::AddMessageToUri(
    /* [in] */ Int64 subId,
    /* [in] */ IContentResolver* resolver,
    /* [in] */ IUri* uri,
    /* [in] */ const String& address,
    /* [in] */ const String& body,
    /* [in] */ const String& subject,
    /* [in] */ Int64 date,
    /* [in] */ Boolean read,
    /* [in] */ Boolean deliveryReport,
    /* [in] */ Int64 threadId,
    /* [out] */ IUri** _uri)
{
    return Telephony::Sms::AddMessageToUri(subId, resolver, uri, address, body, subject, date, read, deliveryReport, threadId, _uri);
}

/**
 * Add an SMS to the given URI with priority specified.
 *
 * @param resolver the content resolver to use
 * @param uri the URI to add the message to
 * @param address the address of the sender
 * @param body the body of the message
 * @param subject the psuedo-subject of the message
 * @param date the timestamp for the message
 * @param read TRUE if the message has been read, FALSE if not
 * @param deliveryReport TRUE if a delivery report was requested, FALSE if not
 * @param threadId the thread_id of the message
 * @param subId the sub_id which the message belongs to
 * @param priority the priority of the message
 * @return the URI for the new message
 * @hide
 */
ECode CTelephonySms::AddMessageToUri(
    /* [in] */ Int64 subId,
    /* [in] */ IContentResolver* resolver,
    /* [in] */ IUri* uri,
    /* [in] */ const String& address,
    /* [in] */ const String& body,
    /* [in] */ const String& subject,
    /* [in] */ Int64 date,
    /* [in] */ Boolean read,
    /* [in] */ Boolean deliveryReport,
    /* [in] */ Int64 threadId,
    /* [in] */ Int32 priority,
    /* [out] */ IUri** _uri)
{
    return Telephony::Sms::AddMessageToUri(subId, resolver, uri, address, body, subject, date, read, deliveryReport, threadId, priority, _uri);
}

/**
 * Move a message to the given folder.
 *
 * @param context the context to use
 * @param uri the message to move
 * @param folder the folder to move to
 * @return TRUE if the operation succeeded
 * @hide
 */
ECode CTelephonySms::MoveMessageToFolder(
    /* [in] */ IContext* context,
    /* [in] */ IUri* uri,
    /* [in] */ Int32 folder,
    /* [in] */ Int32 error,
    /* [out] */ Boolean* result)
{
    return Telephony::Sms::MoveMessageToFolder(context, uri, folder, error, result);
}

/**
 * Returns TRUE iff the Folder (message type) identifies an
 * outgoing message.
 * @hide
 */
ECode CTelephonySms::IsOutgoingFolder(
    /* [in] */ Int32 messageType,
    /* [out] */ Boolean* result)
{
    return Telephony::Sms::IsOutgoingFolder(messageType, result);
}

ECode CTelephonySms::GetCONTENT_URI(
    /* [out] */ IUri** uri)
{
    VALIDATE_NOT_NULL(uri);
    *uri = Telephony::Sms::CONTENT_URI;
    REFCOUNT_ADD(*uri);
    return NOERROR;
}

} // namespace Provider
} // namespace Droid
} // namespace Elastos
