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

#include "elastos/droid/server/ProfileManagerService.h"
#include "Elastos.CoreLibrary.External.h"
#include "Elastos.CoreLibrary.IO.h"
#include "Elastos.Droid.App.h"
#include "Elastos.Droid.Content.h"
#include "Elastos.Droid.Os.h"
#include <elastos/droid/app/ActivityManagerNative.h>
#include <elastos/droid/os/Environment.h>
#include <elastos/droid/os/Binder.h>
#include <elastos/droid/R.h>
#include <elastos/droid/text/TextUtils.h>
#include <elastos/utility/Arrays.h>
#include <elastos/utility/logging/Logger.h>
#include <elastos/utility/logging/Slogger.h>
#include <elastos/core/AutoLock.h>
#include <elastos/core/StringBuilder.h>
#include <elastos/core/StringUtils.h>

using Elastos::Droid::App::ActivityManagerNative;
// using Elastos::Droid::App::Backup::CBackupManager;
using Elastos::Droid::App::CNotificationGroup;
using Elastos::Droid::App::CNotificationGroupHelper;
using Elastos::Droid::App::CProfileHelper;
using Elastos::Droid::App::CProfileGroup;
using Elastos::Droid::App::EIID_IIProfileManager;
using Elastos::Droid::App::INotificationGroupHelper;
using Elastos::Droid::App::IProfileHelper;
using Elastos::Droid::App::IProfileGroup;
using Elastos::Droid::App::IProfileManager;
using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::CIntentFilter;
using Elastos::Droid::Content::IBroadcastReceiver;
using Elastos::Droid::Content::IContext;
using Elastos::Droid::Content::Res::IResources;
using Elastos::Droid::Content::Res::IXmlResourceParser;
using Elastos::Droid::Os::Binder;
using Elastos::Droid::Os::CUserHandleHelper;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Droid::Os::Environment;
using Elastos::Droid::Os::IUserHandle;
using Elastos::Droid::Os::IUserHandleHelper;
using Elastos::Droid::Text::TextUtils;
using Elastos::Utility::Arrays;
using Elastos::Utility::CArrayList;
using Elastos::Utility::CHashMap;
using Elastos::Utility::CUUIDHelper;
using Elastos::Utility::Logging::Logger;
using Elastos::Utility::Logging::Slogger;
using Elastos::Utility::IArrayList;
using Elastos::Utility::ICollection;
using Elastos::Utility::IIterator;
using Elastos::Utility::IMapEntry;
using Elastos::Utility::ISet;
using Elastos::Utility::IUUID;
using Elastos::Utility::IUUIDHelper;
using Elastos::IO::CFile;
using Elastos::IO::CFileReader;
using Elastos::IO::CFileWriter;
using Elastos::IO::ICloseable;
using Elastos::IO::IFileReader;
using Elastos::IO::IFileWriter;
using Elastos::IO::IReader;
using Elastos::IO::IWriter;
using Elastos::Core::AutoLock;
using Elastos::Core::IStringBuilder;
using Elastos::Core::StringBuilder;
using Elastos::Core::StringUtils;
using Org::Xmlpull::V1::CXmlPullParserFactoryHelper;
using Org::Xmlpull::V1::IXmlPullParserFactory;
using Org::Xmlpull::V1::IXmlPullParserFactoryHelper;

namespace Elastos {
namespace Droid {
namespace Server {

const String ProfileManagerService::TAG("ProfileService");

Boolean ProfileManagerService::LOCAL_LOGV = FALSE;

const String ProfileManagerService::PERMISSION_CHANGE_SETTINGS("android.permission.WRITE_SETTINGS");

static AutoPtr<IFile> InitPROFILE_FILE()
{
    AutoPtr<IFile> _file = Environment::GetSystemSecureDirectory();
    AutoPtr<IFile> file;
    CFile::New(_file.Get(), String("profiles.xml"), (IFile**)&file);
    return file;
}

const AutoPtr<IFile> ProfileManagerService::PROFILE_FILE = InitPROFILE_FILE();

static AutoPtr<IUUID> InitWildcardUUID()
{
    AutoPtr<IUUIDHelper> uuidHelper;
    CUUIDHelper::AcquireSingleton((IUUIDHelper**)&uuidHelper);
    AutoPtr<IUUID> uuid;
    uuidHelper->FromString(String("a126d48a-aaef-47c4-baed-7f0e44aeffe5"), (IUUID**)&uuid);
    return uuid;
}

const AutoPtr<IUUID> ProfileManagerService::mWildcardUUID = InitWildcardUUID();

//--------------------------------------------------------------------------------
//                ProfileManagerService::MyBroadcastReceiver
//--------------------------------------------------------------------------------
ProfileManagerService::MyBroadcastReceiver::MyBroadcastReceiver(
    /* [in] */ ProfileManagerService* host)
    : mHost(host)
{}

ECode ProfileManagerService::MyBroadcastReceiver::OnReceive(
    /* [in] */ IContext* context,
    /* [in] */ IIntent* intent)
{
    String action;
    intent->GetAction(&action);
    if (action.Equals(IIntent::ACTION_LOCALE_CHANGED)) {
        mHost->PersistIfDirty();
        mHost->Initialize();
    }
    else if (action.Equals(IIntent::ACTION_SHUTDOWN)) {
        mHost->PersistIfDirty();
    }
    return NOERROR;
}

//--------------------------------------------------------------------------------
//                ProfileManagerService
//--------------------------------------------------------------------------------

CAR_INTERFACE_IMPL(ProfileManagerService, Object, IIProfileManager, IBinder)

ProfileManagerService::ProfileManagerService()
    : mDirty(FALSE)
{}

ECode ProfileManagerService::constructor(
    /* [in] */ IContext* context)
{
    mContext = context;
    Logger::I(TAG, " >> TODO needs CBackupManager");
    //CBackupManager::New(mContext.Get(), (IBackupManager**)&mBackupManager);
    mTriggerHelper = new ProfileTriggerHelper();
    mTriggerHelper->constructor(mContext, this);
    String str;
    context->GetString(R::string::wildcardProfile, &str);
    CNotificationGroup::New(str, R::string::wildcardProfile,
        mWildcardUUID, (INotificationGroup**)&mWildcardGroup);

    Initialize();

    AutoPtr<IIntentFilter> filter;
    CIntentFilter::New((IIntentFilter**)&filter);
    filter->AddAction(IIntent::ACTION_LOCALE_CHANGED);
    filter->AddAction(IIntent::ACTION_SHUTDOWN);
    mIntentReceiver = new MyBroadcastReceiver(this);
    AutoPtr<IIntent> intent;
    return mContext->RegisterReceiver(mIntentReceiver.Get(), filter.Get(), (IIntent**)&intent);
}

void ProfileManagerService::Initialize()
{
    Initialize(FALSE);
}

void ProfileManagerService::Initialize(
    /* [in] */ Boolean skipFile)
{
    CHashMap::New((IHashMap**)&mProfiles);
    CHashMap::New((IHashMap**)&mProfileNames);
    CHashMap::New((IHashMap**)&mGroups);
    mDirty = FALSE;
    Boolean init = skipFile;
    if (!skipFile) {
        ECode ec = LoadFromFile();
        if (FAILED(ec)) {
            init = TRUE;
        }
    }

    if (init) {
        // try {
        ECode ec = InitialiseStructure();
        if (FAILED(ec)) {
            Logger::E(TAG, "Error loading xml from resource: %08x", ec);
        }
    }
}

ECode ProfileManagerService::ResetAll()
{
    FAIL_RETURN(EnforceChangePermissions())
    Initialize(TRUE);
    return NOERROR;
}

ECode ProfileManagerService::SetActiveProfileByName(
    /* [in] */ const String& profileName,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    Boolean flag = FALSE;
    mProfileNames->ContainsKey(StringUtils::ParseCharSequence(profileName).Get(), &flag);
    if (!flag) {
        // Since profileName could not be casted into a UUID, we can call it a string.
        Logger::W(TAG, "Unable to find profile to set active, based on string: %s", profileName.string());
        *result = FALSE;
        return NOERROR;
    }

    if (LOCAL_LOGV) {
        Logger::V(TAG, "setActiveProfile(String) found profile name in mProfileNames.");
    }
    AutoPtr<IInterface> obj;
    mProfileNames->Get(StringUtils::ParseCharSequence(profileName).Get(), (IInterface**)&obj);
    AutoPtr<IUUID> uuid = IUUID::Probe(obj);
    obj = NULL;
    mProfiles->Get(uuid.Get(), (IInterface**)&obj);
    SetActiveProfile(IProfile::Probe(obj), TRUE);
    *result = TRUE;
    return NOERROR;
}

ECode ProfileManagerService::SetActiveProfile(
    /* [in] */ IParcelUuid* profileParcelUuid,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<IUUID> uuid;
    profileParcelUuid->GetUuid((IUUID**)&uuid);
    return SetActiveProfile(uuid.Get(), TRUE, result);
}

ECode ProfileManagerService::SetActiveProfile(
    /* [in] */ IUUID* profileUuid,
    /* [in] */ Boolean doInit,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    Boolean flag = FALSE;
    FAIL_RETURN(mProfiles->ContainsKey(profileUuid, &flag));
    if (!flag) {
        Logger::E(TAG, "Cannot set active profile to:  - does not exist.", TO_CSTR(profileUuid));
        *result = FALSE;
        return NOERROR;
    }

    if (LOCAL_LOGV) Logger::V(TAG, "setActiveProfile(UUID, boolean) found UUID in mProfiles.");
    AutoPtr<IInterface> obj;
    mProfiles->Get(profileUuid, (IInterface**)&obj);
    SetActiveProfile(IProfile::Probe(obj), doInit);
    *result = TRUE;
    return NOERROR;
}

void ProfileManagerService::SetActiveProfile(
    /* [in] */ IProfile* newActiveProfile,
    /* [in] */ Boolean doInit)
{
    /*
     * NOTE: Since this is not a public function, and all public functions
     * take either a string or a UUID, the active profile should always be
     * in the collection.  If writing another setActiveProfile which receives
     * a Profile object, run enforceChangePermissions, add the profile to the
     * list, and THEN add it.
     */

    if (FAILED(EnforceChangePermissions())) {
        return;
    }

    String name;
    AutoPtr<IUUID> uuid;
    newActiveProfile->GetUuid((IUUID**)&uuid);
    newActiveProfile->GetName(&name);
    Logger::D(TAG, "Set active profile to: %s - ", TO_CSTR(uuid), name.string());

    AutoPtr<IProfile> lastProfile = mActiveProfile;
    mActiveProfile = newActiveProfile;
    mDirty = TRUE;

    AutoPtr<IUserHandleHelper> uhh;
    CUserHandleHelper::AcquireSingleton((IUserHandleHelper**)&uhh);
    AutoPtr<IUserHandle> all;
    uhh->GetALL((IUserHandle**)&all);

    if (doInit) {
        if (LOCAL_LOGV) Logger::V(TAG, "setActiveProfile(Profile, boolean) - Running init");

        /*
         * We need to clear the caller's identity in order to
         * - allow the profile switch to execute actions
         *   not included in the caller's permissions
         * - broadcast INTENT_ACTION_PROFILE_SELECTED
         */
        Int64 token = Binder::ClearCallingIdentity();

        // Call profile's "doSelect"
        mActiveProfile->DoSelect(mContext.Get());

        // Notify other applications of newly selected profile.
        AutoPtr<IIntent> broadcast;
        CIntent::New(IProfileManager::INTENT_ACTION_PROFILE_SELECTED, (IIntent**)&broadcast);
        mActiveProfile->GetName(&name);
        broadcast->PutExtra(IProfileManager::EXTRA_PROFILE_NAME, name);
        uuid =  NULL;
        mActiveProfile->GetUuid((IUUID**)&uuid);
        broadcast->PutExtra(IProfileManager::EXTRA_PROFILE_UUID, TO_STR(uuid));
        lastProfile->GetName(&name);
        broadcast->PutExtra(IProfileManager::EXTRA_LAST_PROFILE_NAME, name);
        uuid = NULL;
        lastProfile->GetUuid((IUUID**)&uuid);
        broadcast->PutExtra(IProfileManager::EXTRA_LAST_PROFILE_UUID, TO_STR(uuid));

        mContext->SendBroadcastAsUser(broadcast.Get(), all.Get());
        Binder::RestoreCallingIdentity(token);
        PersistIfDirty();
    }
    else if (lastProfile != mActiveProfile && ActivityManagerNative::IsSystemReady()) {
        // Something definitely changed: notify.
        Int64 token = Binder::ClearCallingIdentity();
        AutoPtr<IIntent> broadcast;
        CIntent::New(IProfileManager::INTENT_ACTION_PROFILE_UPDATED, (IIntent**)&broadcast);
        mActiveProfile->GetName(&name);
        broadcast->PutExtra(IProfileManager::EXTRA_PROFILE_NAME, name);
        uuid = NULL;
        broadcast->PutExtra(IProfileManager::EXTRA_PROFILE_UUID, TO_STR(uuid));
        mContext->SendBroadcastAsUser(broadcast.Get(), all);
        Binder::RestoreCallingIdentity(token);
    }
}

ECode ProfileManagerService::AddProfile(
    /* [in] */ IProfile* profile,
    /* [out] */ Boolean* result)
{
    FAIL_RETURN(EnforceChangePermissions())
    AddProfileInternal(profile);
    PersistIfDirty();
    *result = TRUE;
    return NOERROR;
}

void ProfileManagerService::AddProfileInternal(
    /* [in] */ IProfile* profile)
{
    // Make sure this profile has all of the correct groups.
    AutoPtr<ICollection> values;
    mGroups->GetValues((ICollection**)&values);
    AutoPtr<IArrayList> arrayList;
    CArrayList::New((IArrayList**)&arrayList);
    Boolean flag = FALSE;
    arrayList->AddAll(values, &flag);
    Int32 size;
    arrayList->GetSize(&size);
    AutoPtr<IInterface> obj;
    AutoPtr<INotificationGroup> group;
    for (Int32 i = 0; i < size; ++i) {
        obj = NULL;
        arrayList->Get(i, (IInterface**)&obj);
        group = INotificationGroup::Probe(obj);
        EnsureGroupInProfile(profile, group.Get(), FALSE);
    }

    EnsureGroupInProfile(profile, mWildcardGroup.Get(), true);
    AutoPtr<IUUID> uuid;
    profile->GetUuid((IUUID**)&uuid);
    mProfiles->Put(uuid, profile);
    String name;
    profile->GetName(&name);
    mProfileNames->Put(StringUtils::ParseCharSequence(name).Get(), uuid.Get());
    mDirty = TRUE;
}

void ProfileManagerService::EnsureGroupInProfile(
    /* [in] */ IProfile* profile,
    /* [in] */ INotificationGroup* group,
    /* [in] */ Boolean defaultGroup)
{
    AutoPtr<IUUID> uuid;
    group->GetUuid((IUUID**)&uuid);
    AutoPtr<IProfileGroup> gp;
    profile->GetProfileGroup(uuid, (IProfileGroup**)&gp);
    if (gp != NULL) {
        return;
    }

    /* enforce a matchup between profile and notification group, which not only
     * works by UUID, but also by name for backwards compatibility */
    AutoPtr<ArrayOf<IProfileGroup*> > groups;
    profile->GetProfileGroups((ArrayOf<IProfileGroup*>**)&groups);
    AutoPtr<IProfileGroup> pg;
    Boolean flag = FALSE;
    for (Int32 i = 0; i < groups->GetLength(); ++i) {
        ((*groups)[i])->Matches(group, defaultGroup, &flag);
        if (flag) {
            return;
        }
    }

    /* didn't find any, create new group */
    group->GetUuid((IUUID**)&uuid);
    AutoPtr<IProfileGroup> pfgroup;
    CProfileGroup::New(uuid, defaultGroup, (IProfileGroup**)&pfgroup);
    profile->AddProfileGroup(pfgroup.Get());
}

ECode ProfileManagerService::GetProfileByName(
    /* [in] */ const String& profileName,
    /* [out] */ IProfile** result)
{
    VALIDATE_NOT_NULL(result);
    Boolean flag = FALSE;
    mProfiles->ContainsKey(StringUtils::ParseCharSequence(profileName).Get(), &flag);
    AutoPtr<IInterface> obj;
    AutoPtr<IUUID> uuid;
    if (flag) {
        mProfileNames->Get(StringUtils::ParseCharSequence(profileName).Get(), (IInterface**)&obj);
        uuid = IUUID::Probe(obj);
        obj = NULL;
        mProfiles->Get(uuid.Get(), (IInterface**)&obj);
        *result = IProfile::Probe(obj);
        REFCOUNT_ADD(*result);
        return NOERROR;
    }
    else {
        AutoPtr<IUUIDHelper> uudh;
        CUUIDHelper::AcquireSingleton((IUUIDHelper**)&uudh);
        uudh->FromString(profileName, (IUUID**)&uuid);
        mProfiles->ContainsKey(uuid.Get(), &flag);
        if (flag) {
            AutoPtr<IInterface> obj;
            mProfiles->Get(uuid.Get(), (IInterface**)&obj);
            *result = IProfile::Probe(obj);
            REFCOUNT_ADD(*result);
            return NOERROR;
        }
        else {
            *result = NULL;
            return NOERROR;
        }
    }
}

ECode ProfileManagerService::GetProfile(
    /* [in] */ IParcelUuid* profileParcelUuid,
    /* [out] */ IProfile** result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<IUUID> profileUuid;
    profileParcelUuid->GetUuid((IUUID**)&profileUuid);
    *result = GetProfile(profileUuid.Get());
    REFCOUNT_ADD(*result);
    return NOERROR;
}

AutoPtr<IProfile> ProfileManagerService::GetProfile(
    /* [in] */ IUUID* profileUuid)
{
    // use primary UUID first
    Boolean flag = FALSE;
    mProfiles->ContainsKey(profileUuid, &flag);
    AutoPtr<IProfile> profile;
    AutoPtr<IInterface> obj;
    if (flag) {
        mProfiles->Get(profileUuid, (IInterface**)&obj);
        profile = IProfile::Probe(obj);
        return profile;
    }
    // if no match was found: try secondary UUID
    AutoPtr<ICollection> collection;
    mProfiles->GetValues((ICollection**)&collection);
    AutoPtr<IArrayList> list;
    CArrayList::New((IArrayList**)&list);
    list->AddAll(collection.Get());
    Int32 size;
    list->GetSize(&size);
    AutoPtr<IProfile> p;
    AutoPtr<ArrayOf<IUUID*> > uuids;
    for (Int32 i = 0; i < size; ++i) {
        obj = NULL;
        list->Get(i, (IInterface**)&obj);
        p = IProfile::Probe(obj);
        uuids = NULL;
        p->GetSecondaryUuids((ArrayOf<IUUID*>**)&uuids);
        Int32 length = uuids->GetLength();
        for (Int32 j = 0; j < length; ++j) {
            if (Object::Equals(profileUuid, (*uuids)[j])) return p;
        }
    }
    // nothing found
    return NULL;
}

AutoPtr<ICollection> ProfileManagerService::GetProfileList()
{
    AutoPtr<ICollection> collection;
    mProfiles->GetValues((ICollection**)&collection);
    return collection;
}

ECode ProfileManagerService::GetProfiles(
    /* [out, callee] */ ArrayOf<IProfile*>** result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<ICollection> collection = GetProfileList();
    Int32 size;
    mProfiles->GetSize(&size);
    AutoPtr<ArrayOf<IInterface*> > inarray = ArrayOf<IInterface*>::Alloc(size);
    AutoPtr<ArrayOf<IInterface*> > outarray;
    collection->ToArray(inarray.Get(), (ArrayOf<IInterface*>**)&outarray);

    AutoPtr<ArrayOf<IProfile*> > profiles = ArrayOf<IProfile*>::Alloc(size);
    for (Int32 i = 0; i < size; ++i) {
        profiles->Set(i, IProfile::Probe((*outarray)[i]));
    }
    Arrays::Sort(profiles.Get());
    *result = profiles;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode ProfileManagerService::GetActiveProfile(
    /* [out] */ IProfile** result)
{
    VALIDATE_NOT_NULL(result);
    *result = mActiveProfile;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode ProfileManagerService::RemoveProfile(
    /* [in] */ IProfile* profile,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    FAIL_RETURN(EnforceChangePermissions())
    String name;
    profile->GetName(&name);
    AutoPtr<IInterface> obj;
    mProfileNames->Remove(StringUtils::ParseCharSequence(name).Get(), (IInterface**)&obj);
    if (obj != NULL) {
        AutoPtr<IUUID> uuid;
        profile->GetUuid((IUUID**)&uuid);
        AutoPtr<IInterface> tmpObj;
        mProfiles->Remove(uuid.Get(), (IInterface**)&tmpObj);
        if (tmpObj != NULL) {
            mDirty = TRUE;
            PersistIfDirty();
            *result = TRUE;
            return NOERROR;
        }
    }
    *result = FALSE;
    return NOERROR;
}

ECode ProfileManagerService::UpdateProfile(
    /* [in] */ IProfile* profile)
{
    FAIL_RETURN(EnforceChangePermissions())
    AutoPtr<IUUID> uuid;
    profile->GetUuid((IUUID**)&uuid);
    AutoPtr<IInterface> obj;
    mProfiles->Get(uuid.Get(), (IInterface**)&obj);
    AutoPtr<IProfile> old = IProfile::Probe(obj);

    if (old == NULL) {
        return NOERROR;
    }
    String name;
    old->GetName(&name);
    mProfileNames->Remove(StringUtils::ParseCharSequence(name).Get());
    String pname;
    profile->GetName(&pname);
    uuid = NULL;
    profile->GetUuid((IUUID**)&uuid);
    mProfileNames->Put(StringUtils::ParseCharSequence(pname).Get(), uuid.Get());
    mProfiles->Put(uuid.Get(), profile);
    /* no need to set mDirty, if the profile was actually changed,
     * it's marked as dirty by itself */
    PersistIfDirty();

    // Also update if we changed the active profile
    uuid = NULL;
    mActiveProfile->GetUuid((IUUID**)&uuid);
    AutoPtr<IUUID> puuid;
    profile->GetUuid((IUUID**)&puuid);
    if (mActiveProfile != NULL && Object::Equals(puuid, uuid)) {
        SetActiveProfile(profile, TRUE);
    }
    return NOERROR;
}

ECode ProfileManagerService::ProfileExists(
    /* [in] */ IParcelUuid* profileUuid,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<IUUID> uuid;
    profileUuid->GetUuid((IUUID**)&uuid);
    return mProfiles->ContainsKey(uuid.Get(), result);
}

ECode ProfileManagerService::ProfileExistsByName(
    /* [in] */ const String& profileName,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<ISet> set;
    mProfileNames->GetKeySet((ISet**)&set);

    if (set != NULL) {
        AutoPtr<IIterator> it;
        set->GetIterator((IIterator**)&it);
        Boolean hasNext = FALSE;
        while ((it->HasNext(&hasNext), hasNext)) {
            AutoPtr<IInterface> obj;
            it->GetNext((IInterface**)&obj);
            String str = TO_STR(obj);
            if (str.EqualsIgnoreCase(profileName)) {
                *result = TRUE;
                return NOERROR;
            }
        }
    }

    *result = FALSE;
    return NOERROR;
}

ECode ProfileManagerService::NotificationGroupExistsByName(
    /* [in] */ const String& notificationGroupName,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    AutoPtr<ICollection> values;
    mGroups->GetValues((ICollection**)&values);

    if (values != NULL) {
        AutoPtr<IIterator> it;
        values->GetIterator((IIterator**)&it);
        Boolean hasNext = FALSE;
        String name;
        while ((it->HasNext(&hasNext), hasNext)) {
            AutoPtr<IInterface> p;
            it->GetNext((IInterface**)&p);
            INotificationGroup* group = INotificationGroup::Probe(p);
            group->GetName(&name);
            if (name.EqualsIgnoreCase(notificationGroupName)) {
                *result = TRUE;
                return NOERROR;
            }
        }
    }

    *result = FALSE;
    return NOERROR;
}

ECode ProfileManagerService::GetNotificationGroups(
    /* [out] */ ArrayOf<INotificationGroup*>** result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<ICollection> values;
    mGroups->GetValues((ICollection**)&values);
    AutoPtr<ArrayOf<INotificationGroup*> > groups;
    if (values != NULL) {
        Int32 size;
        values->GetSize(&size);
        groups = ArrayOf<INotificationGroup*>::Alloc(size);
        AutoPtr<IIterator> it;
        values->GetIterator((IIterator**)&it);
        Int32 i = 0;
        Boolean hasNext;
        while (it->HasNext(&hasNext), hasNext) {
            AutoPtr<IInterface> obj;
            it->GetNext((IInterface**)&obj);
            groups->Set(i++, INotificationGroup::Probe(obj));
        }
    }
    else {
        groups = ArrayOf<INotificationGroup*>::Alloc(0);
    }

    *result = groups;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode ProfileManagerService::AddNotificationGroup(
    /* [in] */ INotificationGroup* group)
{
    FAIL_RETURN(EnforceChangePermissions())
    AddNotificationGroupInternal(group);
    PersistIfDirty();
    return NOERROR;
}

void ProfileManagerService::AddNotificationGroupInternal(
    /* [in] */ INotificationGroup* group)
{
    AutoPtr<IUUID> uuid;
    group->GetUuid((IUUID**)&uuid);
    AutoPtr<IInterface> obj;
    if ((mGroups->Put(uuid.Get(), group, (IInterface**)&obj), obj == NULL)) {
        // If the above is true, then the ProfileGroup shouldn't exist in
        // the profile. Ensure it is added.
        AutoPtr<ICollection> values;
        mProfiles->GetValues((ICollection**)&values);
        if (values != NULL) {
            AutoPtr<IIterator> it;
            values->GetIterator((IIterator**)&it);
            Boolean hasNext = FALSE;
            while ((it->HasNext(&hasNext), hasNext)) {
                AutoPtr<IInterface> p;
                it->GetNext((IInterface**)&p);
                AutoPtr<IProfile> profile = IProfile::Probe(p);
                EnsureGroupInProfile(profile.Get(), group, FALSE);
            }
        }
    }
    mDirty = TRUE;
}

ECode ProfileManagerService::RemoveNotificationGroup(
    /* [in] */ INotificationGroup* group)
{
    FAIL_RETURN(EnforceChangePermissions())
    AutoPtr<IUUID> uuid;
    group->GetUuid((IUUID**)&uuid);
    AutoPtr<IInterface> obj;
    mGroups->Remove(uuid.Get(), (IInterface**)&obj);
    mDirty |= (obj != NULL);
    // Remove the corresponding ProfileGroup from all the profiles too if
    // they use it.
    AutoPtr<ICollection> values;
    mProfiles->GetValues((ICollection**)&values);
    if (values != NULL) {
        AutoPtr<IIterator> it;
        values->GetIterator((IIterator**)&it);
        Boolean hasNext = FALSE;
        while ((it->HasNext(&hasNext), hasNext)) {
            AutoPtr<IInterface> p;
            it->GetNext((IInterface**)&p);
            AutoPtr<IProfile> profile = IProfile::Probe(p);
            profile->RemoveProfileGroup(uuid);
        }
    }

    PersistIfDirty();
    return NOERROR;
}

ECode ProfileManagerService::UpdateNotificationGroup(
    /* [in] */ INotificationGroup* group)
{
    FAIL_RETURN(EnforceChangePermissions())
    AutoPtr<IUUID> uuid;
    group->GetUuid((IUUID**)&uuid);
    AutoPtr<IInterface> obj;
    mGroups->Get(uuid, (IInterface**)&obj);
    AutoPtr<INotificationGroup> old = INotificationGroup::Probe(obj);
    if (old == NULL) {
        return NOERROR;
    }

    mGroups->Put(uuid.Get(), group);
    /* no need to set mDirty, if the group was actually changed,
     * it's marked as dirty by itself */
    PersistIfDirty();
    return NOERROR;
}

ECode ProfileManagerService::GetNotificationGroupForPackage(
    /* [in] */ const String& pkg,
    /* [out] */ INotificationGroup** result)
{
    VALIDATE_NOT_NULL(result);

    AutoPtr<ICollection> values;
    mGroups->GetValues((ICollection**)&values);

    if (values != NULL) {
        AutoPtr<IIterator> it;
        values->GetIterator((IIterator**)&it);
        Boolean hasNext = FALSE;
        Boolean flag = FALSE;
        while ((it->HasNext(&hasNext), hasNext)) {
            AutoPtr<IInterface> p;
            it->GetNext((IInterface**)&p);
            AutoPtr<INotificationGroup> group = INotificationGroup::Probe(p);
            group->HasPackage(pkg, &flag);
            if (flag) {
                *result = group;
                REFCOUNT_ADD(*result);
                return NOERROR;
            }
        }
    }

    *result = NULL;
    return NOERROR;
}

void ProfileManagerService::SettingsRestored()
{
    Initialize();
    AutoPtr<ICollection> values;
    mProfiles->GetValues((ICollection**)&values);
    if (values != NULL) {
        AutoPtr<IIterator> it;
        values->GetIterator((IIterator**)&it);
        Boolean hasNext = FALSE;
        while ((it->HasNext(&hasNext), hasNext)) {
            AutoPtr<IInterface> obj;
            it->GetNext((IInterface**)&obj);
            IProfile* p = IProfile::Probe(obj);
            p->ValidateRingtones(mContext);
        }
    }

    PersistIfDirty();
}

ECode ProfileManagerService::LoadFromFile()
{
    AutoPtr<IXmlPullParserFactoryHelper> ppf;
    CXmlPullParserFactoryHelper::AcquireSingleton((IXmlPullParserFactoryHelper**)&ppf);
    AutoPtr<IXmlPullParserFactory> xppf;
    FAIL_RETURN(ppf->NewInstance((IXmlPullParserFactory**)&xppf));
    AutoPtr<IXmlPullParser> xpp;
    FAIL_RETURN(xppf->NewPullParser((IXmlPullParser**)&xpp));
    AutoPtr<IFileReader> fr;
    FAIL_RETURN(CFileReader::New(PROFILE_FILE, (IFileReader**)&fr))
    FAIL_RETURN(xpp->SetInput(IReader::Probe(fr)));
    FAIL_RETURN(LoadXml(xpp.Get(), mContext.Get()));
    ICloseable::Probe(fr)->Close();
    PersistIfDirty();
    return NOERROR;
}

ECode ProfileManagerService::LoadXml(
    /* [in] */ IXmlPullParser* xpp,
    /* [in] */ IContext* context)
{
    Int32 event;
    xpp->Next(&event);
    String active;
    String _name;
    while (event != IXmlPullParser::END_TAG || !(xpp->GetName(&_name), _name).Equals("profiles")) {
        if (event == IXmlPullParser::START_TAG) {
            String name;
            xpp->GetName(&name);
            if (name.Equals("active")) {
                xpp->NextText(&active);
                Logger::D(TAG, "Found active: %s", active.string());
            }
            else if (name.Equals("profile")) {
                AutoPtr<IProfileHelper> phl;
                CProfileHelper::AcquireSingleton((IProfileHelper**)&phl);
                AutoPtr<IProfile> prof;
                phl->FromXml(xpp, context, (IProfile**)&prof);
                AddProfileInternal(prof.Get());
                // Failsafe if no active found
                if (active.IsNull()) {
                    AutoPtr<IUUID> uuid;
                    prof->GetUuid((IUUID**)&uuid);
                    active = TO_STR(uuid);
                }
            }
            else if (name.Equals("notificationGroup")) {
                AutoPtr<INotificationGroupHelper> ngh;
                CNotificationGroupHelper::AcquireSingleton((INotificationGroupHelper**)&ngh);
                AutoPtr<INotificationGroup> ng;
                ngh->FromXml(xpp, context, (INotificationGroup**)&ng);
                AddNotificationGroupInternal(ng);
            }
        }
        else if (event == IXmlPullParser::END_DOCUMENT) {
            Slogger::E("ProfileManagerService", "Premature end of file while reading %s",
                TO_CSTR(PROFILE_FILE));
            return E_IO_EXCEPTION;
        }
        xpp->Next(&event);
    }
    // Don't do initialisation on startup. The AudioManager doesn't exist yet
    // and besides, the volume settings will have survived the reboot.
    // try {
        // Try / catch block to detect if XML file needs to be upgraded.
        ECode ec = NOERROR;
        AutoPtr<IUUIDHelper> uudh;
        CUUIDHelper::AcquireSingleton((IUUIDHelper**)&uudh);
        AutoPtr<IUUID> uuid;
        uudh->FromString(active, (IUUID**)&uuid);
        Boolean flag = FALSE;
        ec = SetActiveProfile(uuid.Get(), FALSE, &flag);
    // } catch (IllegalArgumentException e) {
        if (ec == (ECode)E_ILLEGAL_ARGUMENT_EXCEPTION) {
            Boolean flag = FALSE;
            mProfileNames->ContainsKey(StringUtils::ParseCharSequence(active), &flag);
            if (flag) {
                AutoPtr<IInterface> obj;
                mProfileNames->Get(StringUtils::ParseCharSequence(active), (IInterface**)&obj);
                SetActiveProfile(IUUID::Probe(obj), FALSE, &flag);
            }
            else {
                // Final fail-safe: We must have SOME profile active.
                // If we couldn't select one by now, we'll pick the first in the set.
                AutoPtr<ICollection> values;
                mProfiles->GetValues((ICollection**)&values);
                if (values != NULL) {
                    AutoPtr<IIterator> it;
                    values->GetIterator((IIterator**)&it);
                    AutoPtr<IInterface> obj;
                    it->GetNext((IInterface**)&obj);
                    SetActiveProfile(IProfile::Probe(obj), FALSE);
                }
            }
            // This is a hint that we probably just upgraded the XML file. Save changes.
            mDirty = TRUE;
        }
    // }
    return NOERROR;
}

ECode ProfileManagerService::InitialiseStructure()
{
    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    AutoPtr<IXmlResourceParser> xml;
    res->GetXml(R::xml::profile_default, (IXmlResourceParser**)&xml);
    // try {
    ECode ec = LoadXml(IXmlPullParser::Probe(xml), mContext.Get());
    mDirty = TRUE;
    PersistIfDirty();
    ICloseable::Probe(xml)->Close();
    return ec;
}

String ProfileManagerService::GetXmlString()
{
    AutoPtr<StringBuilder> builder = new StringBuilder();
    builder->Append("<profiles>\n<active>");
    AutoPtr<IProfile> profile;
    GetActiveProfile((IProfile**)&profile);
    AutoPtr<IUUID> uuid;
    profile->GetUuid((IUUID**)&uuid);
    builder->Append(TextUtils::HtmlEncode(TO_STR(uuid)));
    builder->Append("</active>\n");

    AutoPtr<ICollection> values;
    mProfiles->GetValues((ICollection**)&values);
    if (values != NULL) {
        AutoPtr<IIterator> it;
        values->GetIterator((IIterator**)&it);
        Boolean hasNext = FALSE;
        while ((it->HasNext(&hasNext), hasNext)) {
            AutoPtr<IInterface> obj;
            it->GetNext((IInterface**)&obj);
            IProfile* p = IProfile::Probe(obj);
            p->GetXmlString((IStringBuilder*)builder, mContext.Get());
        }
    }

    AutoPtr<ICollection> groups;
    mGroups->GetValues((ICollection**)&groups);
    if (groups != NULL) {
        AutoPtr<IIterator> it;
        groups->GetIterator((IIterator**)&it);
        Boolean hasNext = FALSE;
        while ((it->HasNext(&hasNext), hasNext)) {
            AutoPtr<IInterface> obj;
            it->GetNext((IInterface**)&obj);
            INotificationGroup* g = INotificationGroup::Probe(obj);
            g->GetXmlString((IStringBuilder*)builder, mContext.Get());
        }
    }

    builder->Append("</profiles>\n");
    return builder->ToString();
}

ECode ProfileManagerService::GetNotificationGroup(
    /* [in] */ IParcelUuid* uuid,
    /* [out] */ INotificationGroup** result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<IUUID> _uuid, uid;
    mWildcardGroup->GetUuid((IUUID**)&_uuid);
    uuid->GetUuid((IUUID**)&uid);
    if (Object::Equals(_uuid, uid)) {
        *result = mWildcardGroup;
        REFCOUNT_ADD(*result);
        return NOERROR;
    }
    AutoPtr<IInterface> obj;
    mGroups->Get(uid, (IInterface**)&obj);
    *result = INotificationGroup::Probe(obj);
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode ProfileManagerService::ToString(
    /* [out] */ String* str)
{
    return Object::ToString(str);
}

void ProfileManagerService::PersistIfDirty()
{
    AutoLock lock(this);
    Boolean dirty = mDirty;
    Boolean hasNext = FALSE;
    Boolean flag = FALSE;
    if (!dirty) {
        AutoPtr<ICollection> profiles;
        mProfiles->GetValues((ICollection**)&profiles);
        if (profiles != NULL) {
            AutoPtr<IIterator> it;
            profiles->GetIterator((IIterator**)&it);
            while (it->HasNext(&hasNext), hasNext) {
                AutoPtr<IInterface> obj;
                it->GetNext((IInterface**)&obj);
                IProfile* profile = IProfile::Probe(obj);
                if (profile->IsDirty(&flag), flag) {
                    dirty = TRUE;
                    break;
                }
            }
        }

    }

    if (!dirty) {
        AutoPtr<ICollection> groups;
        mGroups->GetValues((ICollection**)&groups);
        if (groups != NULL) {
            AutoPtr<IIterator> it;
            groups->GetIterator((IIterator**)&it);
            while (it->HasNext(&hasNext), hasNext) {
                AutoPtr<IInterface> obj;
                it->GetNext((IInterface**)&obj);
                INotificationGroup* group = INotificationGroup::Probe(obj);
                if (group->IsDirty(&flag), flag) {
                    dirty = TRUE;
                    break;
                }
            }
        }
    }

    if (dirty) {
        // try {
        Logger::D(TAG, "Saving profile data...");
        AutoPtr<IFileWriter> fw;
        CFileWriter::New(PROFILE_FILE, (IFileWriter**)&fw);
        IWriter::Probe(fw)->Write(GetXmlString());
        ICloseable::Probe(fw)->Close();
        Logger::D(TAG, "Save completed.");
        mDirty = FALSE;

        Int64 token = Binder::ClearCallingIdentity();
        if (mBackupManager) {
            mBackupManager->DataChanged();
        }
        Binder::RestoreCallingIdentity(token);
        // } catch (Throwable e) {
            // e.printStackTrace();
        // }
    }
}

ECode ProfileManagerService::EnforceChangePermissions()
{
    return mContext->EnforceCallingOrSelfPermission(PERMISSION_CHANGE_SETTINGS,
            String("You do not have permissions to change the Profile Manager."));
}

} // Server
} // Droid
} // Elastos