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

#include "Elastos.Droid.Location.h"
#include "Elastos.Droid.Provider.h"
#include "elastos/droid/settings/wifi/WifiSettings.h"
#include "elastos/droid/settings/CSettingsActivity.h"
#include "elastos/droid/settings/search/SearchIndexableRaw.h"
#include "elastos/droid/settings/wifi/CWpsDialog.h"
#include "elastos/droid/settings/wifi/CWriteWifiConfigToNfcDialog.h"
#include "elastos/droid/settings/wifi/CAccessPoint.h"
#include "elastos/droid/view/LayoutInflater.h"
#include "elastos/droid/os/Build.h"
#include "elastos/droid/R.h"
#include "../R.h"
#include <elastos/core/CoreUtils.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Settings::Search::SearchIndexableRaw;
using Elastos::Droid::Settings::Search::ISearchIndexableRaw;
using Elastos::Droid::Settings::Search::EIID_IIndexable;
using Elastos::Droid::Settings::Search::EIID_IIndexableSearchIndexProvider;

using Elastos::Droid::App::IActivity;
using Elastos::Droid::App::IAlertDialog;
using Elastos::Droid::App::CActivityManagerHelper;
using Elastos::Droid::App::IActivityManagerHelper;
using Elastos::Droid::Content::IBroadcastReceiver;
using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::CIntentFilter;
using Elastos::Droid::Content::ISharedPreferences;
using Elastos::Droid::Content::Res::IResourcesTheme;
using Elastos::Droid::Content::Res::IResources;
using Elastos::Droid::Content::Res::ITypedArray;
using Elastos::Droid::Content::EIID_IDialogInterfaceOnClickListener;
using Elastos::Droid::Location::ILocationManager;
using Elastos::Droid::Net::IConnectivityManager;
using Elastos::Droid::Net::INetworkInfo;
using Elastos::Droid::Net::INetworkScoreManager;
using Elastos::Droid::Net::INetworkScorerAppManager;
using Elastos::Droid::Net::CNetworkScorerAppManager;
using Elastos::Droid::Net::NetworkInfoDetailedState_NONE;
using Elastos::Droid::Net::NetworkInfoDetailedState_OBTAINING_IPADDR;
using Elastos::Droid::Os::Build;
using Elastos::Droid::Os::CBundle;
using Elastos::Droid::Os::IUserHandle;
using Elastos::Droid::Preference::IPreferenceGroup;
using Elastos::Droid::Provider::ISettingsGlobal;
using Elastos::Droid::Provider::CSettingsGlobal;
using Elastos::Droid::Provider::ISettingsSecure;
using Elastos::Droid::Provider::CSettingsSecure;
using Elastos::Droid::View::EIID_IViewOnClickListener;
using Elastos::Droid::View::LayoutInflater;
using Elastos::Droid::View::ILayoutInflater;
using Elastos::Droid::Widget::IAdapterContextMenuInfo;
using Elastos::Droid::Widget::IButton;
using Elastos::Droid::Widget::CToastHelper;
using Elastos::Droid::Widget::IToastHelper;
using Elastos::Droid::Widget::IToast;
using Elastos::Droid::Wifi::EIID_IWifiManagerActionListener;
using Elastos::Droid::Wifi::IScanResult;
using Elastos::Droid::Wifi::IWpsInfo;
using Elastos::Core::CoreUtils;
using Elastos::Utility::IArrayList;
using Elastos::Utility::CArrayList;
using Elastos::Utility::ICollection;
using Elastos::Utility::CCollections;
using Elastos::Utility::ICollections;
using Elastos::Utility::CHashMap;
using Elastos::Utility::IIterator;
using Elastos::Utility::Concurrent::Atomic::CAtomicBoolean;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Settings {
namespace Wifi {

const Int32 WifiSettings::WIFI_DIALOG_ID;
const Int32 WifiSettings::MENU_ID_WPS_PBC;
const Int32 WifiSettings::MENU_ID_ADD_NETWORK;
const Int32 WifiSettings::WPS_PBC_DIALOG_ID;
Int32 WifiSettings::mVerboseLogging = 0;

AutoPtr<IIndexableSearchIndexProvider> WifiSettings::SEARCH_INDEX_DATA_PROVIDER;

const String WifiSettings::TAG("WifiSettings");
const Int32 WifiSettings::REQUEST_ENABLE_WIFI_ASSISTANT = 1;
const Int32 WifiSettings::MENU_ID_WPS_PIN;
const Int32 WifiSettings::MENU_ID_SAVED_NETWORK;
const Int32 WifiSettings::MENU_ID_ADVANCED;
const Int32 WifiSettings::MENU_ID_SCAN;
const Int32 WifiSettings::MENU_ID_CONNECT;
const Int32 WifiSettings::MENU_ID_FORGET;
const Int32 WifiSettings::MENU_ID_MODIFY;
const Int32 WifiSettings::MENU_ID_WRITE_NFC;
const String WifiSettings::KEY_ASSISTANT_DISMISS_PLATFORM("assistant_dismiss_platform");
const Int32 WifiSettings::WPS_PIN_DIALOG_ID;
const Int32 WifiSettings::WRITE_NFC_DIALOG_ID;
const Int32 WifiSettings::WIFI_RESCAN_INTERVAL_MS = 10 * 1000;
const String WifiSettings::SAVE_DIALOG_EDIT_MODE("edit_mode");
const String WifiSettings::SAVE_DIALOG_ACCESS_POINT_STATE("wifi_ap_state");
Boolean WifiSettings::mSavedNetworksExist;
const String WifiSettings::EXTRA_ENABLE_NEXT_ON_CONNECT("wifi_enable_next_on_connect");

AutoPtr<IIndexableSearchIndexProvider> WifiSettings::GetSEARCH_INDEX_DATA_PROVIDER()
{
    if (SEARCH_INDEX_DATA_PROVIDER == NULL) {
        SEARCH_INDEX_DATA_PROVIDER = new WifiSettings::MyBaseSearchIndexProvider();
    }

    return SEARCH_INDEX_DATA_PROVIDER;
}

//===============================================================================
//                  WifiSettings::MyBaseSearchIndexProvider
//===============================================================================

WifiSettings::MyBaseSearchIndexProvider::MyBaseSearchIndexProvider()
{}

WifiSettings::MyBaseSearchIndexProvider::~MyBaseSearchIndexProvider()
{}

ECode WifiSettings::MyBaseSearchIndexProvider::GetRawDataToIndex(
    /* [in] */ IContext* context,
    /* [in] */ Boolean enabled,
    /* [out] */ IList** list)//List<SearchIndexableRaw>
{
    VALIDATE_NOT_NULL(list);

    AutoPtr<IList> result;
    CArrayList::New((IList**)&result);
    AutoPtr<IResources> res;
    context->GetResources((IResources**)&res);

    // Add fragment title
    AutoPtr<SearchIndexableRaw> data = new SearchIndexableRaw();
    data->constructor(context);
    res->GetString(R::string::wifi_settings, &data->mTitle);
    res->GetString(R::string::wifi_settings, &data->mScreenTitle);
    res->GetString(R::string::keywords_wifi, &data->mKeywords);
    result->Add((ISearchIndexableRaw*)data);

    // Add available Wi-Fi access points
    AutoPtr<IInterface> obj;
    context->GetSystemService(IContext::WIFI_SERVICE, (IInterface**)&obj);
    IWifiManager* wifiManager = IWifiManager::Probe(obj);
    AutoPtr<IList> ap = ConstructAccessPoints(context, wifiManager,
            NULL, NetworkInfoDetailedState_NONE);
    ICollection* accessPoints = ICollection::Probe(ap);
    AutoPtr<IIterator> iter;
    accessPoints->GetIterator((IIterator**)&iter);
    Boolean hasNext;
    while (iter->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> object;
        iter->GetNext((IInterface**)&object);
        IAccessPoint* accessPoint = IAccessPoint::Probe(object);

        // We are indexing only the saved Wi-Fi networks.
        if (((CAccessPoint*)accessPoint)->GetConfig() == NULL) continue;
        data = NULL;
        data = new SearchIndexableRaw();
        data->constructor(context);
        AutoPtr<ICharSequence> cs;
        IPreference::Probe(accessPoint)->GetTitle((ICharSequence**)&cs);
        cs->ToString(&data->mTitle);
        res->GetString(R::string::wifi_settings, &data->mScreenTitle);
        data->mEnabled = enabled;
        result->Add((ISearchIndexableRaw*)data);
    }
    *list = result;
    REFCOUNT_ADD(*list);
    return NOERROR;
}

//===============================================================================
//                  WifiSettings::Multimap
//===============================================================================

WifiSettings::Multimap::Multimap()
{
    CHashMap::New((IHashMap**)&mStore);
}

WifiSettings::Multimap::~Multimap()
{}

AutoPtr<IList> WifiSettings::Multimap::GetAll(
    /* [in] */ const String& key)
{
    AutoPtr<IInterface> obj;
    mStore->Get(CoreUtils::Convert(key), (IInterface**)&obj);
    IList* values = IList::Probe(obj);
    if (values != NULL) {
        return values;
    }
    AutoPtr<ICollections> coll;
    CCollections::AcquireSingleton((ICollections**)&coll);
    AutoPtr<IList> list;
    coll->GetEmptyList((IList**)&list);
    return list;
}

void WifiSettings::Multimap::Put(
    /* [in] */ const String& key,
    /* [in] */ IAccessPoint* val)
{
    AutoPtr<IInterface> obj;
    mStore->Get(CoreUtils::Convert(key), (IInterface**)&obj);
    AutoPtr<IList> curVals = IList::Probe(obj);
    if (curVals == NULL) {
        CArrayList::New(3, (IList**)&curVals);
        mStore->Put(CoreUtils::Convert(key), curVals);
    }
    curVals->Add(val);
}

//===============================================================================
//                  WifiSettings::Scanner
//===============================================================================

WifiSettings::Scanner::Scanner(
    /* [in] */ WifiSettings* wifiSettings)
    : mRetry(0)
    , mWifiSettings(wifiSettings)
{}

WifiSettings::Scanner::~Scanner()
{}

void WifiSettings::Scanner::Resume()
{
    Boolean res;
    if (HasMessages(0, &res), !res) {
        SendEmptyMessage(0, &res);
    }
}

void WifiSettings::Scanner::ForceScan()
{
    RemoveMessages(0);
    Boolean res;
    SendEmptyMessage(0, &res);
}

void WifiSettings::Scanner::Pause()
{
    mRetry = 0;
    RemoveMessages(0);
}

ECode WifiSettings::Scanner::HandleMessage(
    /* [in] */ IMessage* message)
{
    Boolean res;
    if (mWifiSettings->mWifiManager->StartScan(&res), res) {
        mRetry = 0;
    }
    else if (++mRetry >= 3) {
        mRetry = 0;
        AutoPtr<IActivity> activity;
        mWifiSettings->GetActivity((IActivity**)&activity);
        if (activity != NULL) {
            AutoPtr<IToastHelper> helper;
            CToastHelper::AcquireSingleton((IToastHelper**)&helper);
            AutoPtr<IToast> toast;
            helper->MakeText(IContext::Probe(activity),
                    R::string::wifi_fail_to_scan,
                    IToast::LENGTH_LONG, (IToast**)&toast);
            toast->Show();
        }
        return NOERROR;
    }
    return SendEmptyMessageDelayed(0, WIFI_RESCAN_INTERVAL_MS, &res);
}

//===============================================================================
//                  WifiSettings::MyBroadcastReceiver
//===============================================================================

WifiSettings::MyBroadcastReceiver::MyBroadcastReceiver(
    /* [in] */ WifiSettings* host)
    : mHost(host)
{}

WifiSettings::MyBroadcastReceiver::~MyBroadcastReceiver()
{}

ECode WifiSettings::MyBroadcastReceiver::OnReceive(
    /* [in] */ IContext* context,
    /* [in] */ IIntent* intent)
{
    mHost->HandleEvent(intent);
    return NOERROR;
}

//===============================================================================
//                  WifiSettings::WifiManagerActionListener
//===============================================================================

CAR_INTERFACE_IMPL(WifiSettings::WifiManagerActionListener, Object, IWifiManagerActionListener);

WifiSettings::WifiManagerActionListener::WifiManagerActionListener(
    /* [in] */ WifiSettings* host,
    /* [in] */ Int32 id)
    : mHost(host)
    , mId(id)
{}

WifiSettings::WifiManagerActionListener::~WifiManagerActionListener()
{}

ECode WifiSettings::WifiManagerActionListener::OnSuccess()
{
    return NOERROR;
}

ECode WifiSettings::WifiManagerActionListener::OnFailure(
    /* [in] */ Int32 reason)
{
    AutoPtr<IActivity> activity;
    mHost->GetActivity((IActivity**)&activity);

    if (activity != NULL) {
        AutoPtr<IToastHelper> helper;
        CToastHelper::AcquireSingleton((IToastHelper**)&helper);
        AutoPtr<IToast> toast;
        switch (mId) {
            case 0:
                helper->MakeText(IContext::Probe(activity),
                        R::string::wifi_failed_connect_message,
                        IToast::LENGTH_SHORT, (IToast**)&toast);
                break;
            case 1:
                helper->MakeText(IContext::Probe(activity),
                        R::string::wifi_failed_save_message,
                        IToast::LENGTH_SHORT, (IToast**)&toast);
                break;
            case 2:
                helper->MakeText(IContext::Probe(activity),
                        R::string::wifi_failed_forget_message,
                        IToast::LENGTH_SHORT, (IToast**)&toast);
                break;
        }
        toast->Show();
    }
    return NOERROR;
}

//===============================================================================
//                  WifiSettings::ViewOnClickListener
//===============================================================================

CAR_INTERFACE_IMPL(WifiSettings::ViewOnClickListener, Object, IViewOnClickListener);

WifiSettings::ViewOnClickListener::ViewOnClickListener(
    /* [in] */ WifiSettings* host,
    /* [in] */ Int32 id)
    : mHost(host)
    , mId(id)
{}

WifiSettings::ViewOnClickListener::~ViewOnClickListener()
{}

ECode WifiSettings::ViewOnClickListener::OnClick(
    /* [in] */ IView* v)
{
    switch (mId) {
        case 0: {
            AutoPtr<IIntent> intent;
            CIntent::New((IIntent**)&intent);
            String configurationActivityClassName;
            mHost->mWifiAssistantApp->GetConfigurationActivityClassName(&configurationActivityClassName);
            String packageName;
            mHost->mWifiAssistantApp->GetPackageName(&packageName);
            if (!configurationActivityClassName.IsNull()) {
                // App has a custom configuration activity; launch that.
                // This custom activity will be responsible for launching the system
                // dialog.
                intent->SetClassName(packageName, configurationActivityClassName);
            }
            else {
                // Fall back on the system dialog.
                intent->SetAction(INetworkScoreManager::ACTION_CHANGE_ACTIVE);
                intent->PutExtra(INetworkScoreManager::EXTRA_PACKAGE_NAME,
                        packageName);
            }
            mHost->StartActivityForResult(intent, REQUEST_ENABLE_WIFI_ASSISTANT);
            break;
        }
        case 1: {
            mHost->DisableWifiAssistantCardUntilPlatformUpgrade();
            AutoPtr<IListView> listView;
            mHost->GetListView((IListView**)&listView);
            Boolean res;
            listView->RemoveHeaderView(mHost->mWifiAssistantCard, &res);
            mHost->mWifiAssistantApp = NULL;
            break;
        }
    }
    return NOERROR;
}

//===============================================================================
//                  WifiSettings
//===============================================================================

CAR_INTERFACE_IMPL(WifiSettings, RestrictedSettingsFragment,
        IDialogInterfaceOnClickListener, IIndexable);

WifiSettings::WifiSettings()
    : mLastState(NetworkInfoDetailedState_NONE)
    , mEnableNextOnConnection(FALSE)
    , mDlgEdit(FALSE)
{
    CAtomicBoolean::New(FALSE, (IAtomicBoolean**)&mConnected);
}

WifiSettings::~WifiSettings()
{}

ECode WifiSettings::constructor()
{
    RestrictedSettingsFragment::constructor(IUserManager::DISALLOW_CONFIG_WIFI);
    CIntentFilter::New((IIntentFilter**)&mFilter);
    mFilter->AddAction(IWifiManager::WIFI_STATE_CHANGED_ACTION);
    mFilter->AddAction(IWifiManager::SCAN_RESULTS_AVAILABLE_ACTION);
    mFilter->AddAction(IWifiManager::NETWORK_IDS_CHANGED_ACTION);
    mFilter->AddAction(IWifiManager::SUPPLICANT_STATE_CHANGED_ACTION);
    mFilter->AddAction(IWifiManager::CONFIGURED_NETWORKS_CHANGED_ACTION);
    mFilter->AddAction(IWifiManager::LINK_CONFIGURATION_CHANGED_ACTION);
    mFilter->AddAction(IWifiManager::NETWORK_STATE_CHANGED_ACTION);
    mFilter->AddAction(IWifiManager::RSSI_CHANGED_ACTION);

    mReceiver = new MyBroadcastReceiver(this);

    mScanner = new Scanner(this);
    mScanner->constructor();

    return NOERROR;
}

ECode WifiSettings::OnActivityCreated(
    /* [in] */ IBundle* savedInstanceState)
{
    RestrictedSettingsFragment::OnActivityCreated(savedInstanceState);

    AutoPtr<IInterface> obj = GetSystemService(IContext::WIFI_SERVICE);
    mWifiManager = IWifiManager::Probe(obj);

    mConnectListener = new WifiManagerActionListener(this, 0);

    mSaveListener = new WifiManagerActionListener(this, 1);

    mForgetListener = new WifiManagerActionListener(this, 2);

    if (savedInstanceState != NULL) {
        savedInstanceState->GetBoolean(SAVE_DIALOG_EDIT_MODE, &mDlgEdit);
        Boolean res;
        if (savedInstanceState->ContainsKey(SAVE_DIALOG_ACCESS_POINT_STATE, &res), res) {
            mAccessPointSavedState = NULL;
            savedInstanceState->GetBundle(SAVE_DIALOG_ACCESS_POINT_STATE,
                    (IBundle**)&mAccessPointSavedState);
        }
    }

    // if we're supposed to enable/disable the Next button based on our current connection
    // state, start it off in the right state
    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    AutoPtr<IIntent> intent;
    activity->GetIntent((IIntent**)&intent);
    intent->GetBooleanExtra(EXTRA_ENABLE_NEXT_ON_CONNECT, FALSE, &mEnableNextOnConnection);

    if (mEnableNextOnConnection) {
        if (HasNextButton()) {
            AutoPtr<IInterface> object;
            IContext::Probe(activity)->GetSystemService(IContext::CONNECTIVITY_SERVICE,
                    (IInterface**)&object);
            IConnectivityManager* connectivity = IConnectivityManager::Probe(object);
            if (connectivity != NULL) {
                AutoPtr<INetworkInfo> info;
                connectivity->GetNetworkInfo(IConnectivityManager::TYPE_WIFI,
                        (INetworkInfo**)&info);
                Boolean isConnected;
                info->IsConnected(&isConnected);
                ChangeNextButtonState(isConnected);
            }
        }
    }

    FAIL_RETURN(AddPreferencesFromResource(R::xml::wifi_settings));

    PrepareWifiAssistantCard();

    mEmptyView = InitEmptyView();
    AutoPtr<IListView> listView;
    GetListView((IListView**)&listView);
    RegisterForContextMenu(IView::Probe(listView));
    SetHasOptionsMenu(TRUE);
    return NOERROR;
}

ECode WifiSettings::OnActivityResult(
    /* [in] */ Int32 requestCode,
    /* [in] */ Int32 resultCode,
    /* [in] */ IIntent* resultData)
{
    if (requestCode == REQUEST_ENABLE_WIFI_ASSISTANT) {
        if (resultCode == IActivity::RESULT_OK) {
            DisableWifiAssistantCardUntilPlatformUpgrade();
            AutoPtr<IListView> listView;
            GetListView((IListView**)&listView);
            Boolean res;
            listView->RemoveHeaderView(mWifiAssistantCard, &res);
            mWifiAssistantApp = NULL;
        }
    }
    else {
        RestrictedSettingsFragment::OnActivityResult(requestCode, resultCode, resultData);
    }
    return NOERROR;
}

ECode WifiSettings::OnDestroyView()
{
    RestrictedSettingsFragment::OnDestroyView();

    if (mWifiEnabler != NULL) {
        mWifiEnabler->TeardownSwitchBar();
    }
    return NOERROR;
}

ECode WifiSettings::OnStart()
{
    RestrictedSettingsFragment::OnStart();

    // On/off switch is hidden for Setup Wizard (returns NULL)
    mWifiEnabler = CreateWifiEnabler();
    return NOERROR;
}

AutoPtr<WifiEnabler> WifiSettings::CreateWifiEnabler()
{
    AutoPtr<IActivity> _activity;
    GetActivity((IActivity**)&_activity);
    CSettingsActivity* activity = (CSettingsActivity*)ISettingsActivity::Probe(_activity);
    AutoPtr<ISwitchBar> switchBar;
    activity->GetSwitchBar((ISwitchBar**)&switchBar);
    AutoPtr<WifiEnabler> abler = new WifiEnabler(IContext::Probe(activity), switchBar);
    return abler;
}

ECode WifiSettings::OnResume()
{
    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    RestrictedSettingsFragment::OnResume();
    if (mWifiEnabler != NULL) {
        mWifiEnabler->Resume(IContext::Probe(activity));
    }

    AutoPtr<IIntent> stickyIntent;
    IContext::Probe(activity)->RegisterReceiver((IBroadcastReceiver*)mReceiver,
            mFilter, (IIntent**)&stickyIntent);
    UpdateAccessPoints();
    return NOERROR;
}

ECode WifiSettings::OnPause()
{
    RestrictedSettingsFragment::OnPause();
    if (mWifiEnabler != NULL) {
        mWifiEnabler->Pause();
    }

    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    IContext::Probe(activity)->UnregisterReceiver((IBroadcastReceiver*)mReceiver);
    mScanner->Pause();
    return NOERROR;
}

ECode WifiSettings::OnCreateOptionsMenu(
    /* [in] */ IMenu* menu,
    /* [in] */ IMenuInflater* inflater)
{
    // If the user is not allowed to configure wifi, do not show the menu.
    if (IsUiRestricted()) return NOERROR;

    AddOptionsMenuItems(menu);
    return RestrictedSettingsFragment::OnCreateOptionsMenu(menu, inflater);
}

void WifiSettings::AddOptionsMenuItems(
    /* [in] */ IMenu* menu)
{
    Logger::D("WifiSettings", " >> enter AddOptionsMenuItems ");
    Boolean wifiIsEnabled;
    mWifiManager->IsWifiEnabled(&wifiIsEnabled);
    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    AutoPtr<IResourcesTheme> theme;
    IContext::Probe(activity)->GetTheme((IResourcesTheme**)&theme);

    AutoPtr< ArrayOf<Int32> > args = ArrayOf<Int32>::Alloc(2);
    (*args)[0] = R::attr::ic_menu_add;
    (*args)[1] = R::attr::ic_wps;

    AutoPtr<ITypedArray> ta;
    theme->ObtainStyledAttributes(args, (ITypedArray**)&ta);
    AutoPtr<IDrawable> drawable;
    ta->GetDrawable(0, (IDrawable**)&drawable);
    AutoPtr<IMenuItem> item;
    menu->Add(IMenu::NONE, MENU_ID_ADD_NETWORK, 0, R::string::wifi_add_network,
            (IMenuItem**)&item);
    item->SetIcon(drawable);
    item->SetEnabled(wifiIsEnabled);
    item->SetShowAsAction(IMenuItem::SHOW_AS_ACTION_NEVER);
    if (mSavedNetworksExist) {
        AutoPtr<IMenuItem> itemOther;
        menu->Add(IMenu::NONE, MENU_ID_SAVED_NETWORK, 0,
                R::string::wifi_saved_access_points_label, (IMenuItem**)&itemOther);
        itemOther->SetIcon(drawable);
        itemOther->SetEnabled(wifiIsEnabled);
        itemOther->SetShowAsAction(IMenuItem::SHOW_AS_ACTION_NEVER);
    }
    item = NULL;
    menu->Add(IMenu::NONE, MENU_ID_SCAN, 0, R::string::menu_stats_refresh,
            (IMenuItem**)&item);
    item->SetEnabled(wifiIsEnabled);
    item->SetShowAsAction(IMenuItem::SHOW_AS_ACTION_NEVER);
    item = NULL;
    menu->Add(IMenu::NONE, MENU_ID_ADVANCED, 0, R::string::wifi_menu_advanced,
            (IMenuItem**)&item);
    item->SetShowAsAction(IMenuItem::SHOW_AS_ACTION_NEVER);

    ta->Recycle();
    Logger::D("WifiSettings", " << leave AddOptionsMenuItems ");
}

ECode WifiSettings::OnSaveInstanceState(
    /* [in] */ IBundle* outState)
{
    RestrictedSettingsFragment::OnSaveInstanceState(outState);

    // If the dialog is showing, save its state.
    Boolean res;
    if (mDialog != NULL && (mDialog->IsShowing(&res), res)) {
        outState->PutBoolean(SAVE_DIALOG_EDIT_MODE, mDlgEdit);
        if (mDlgAccessPoint != NULL) {
            mAccessPointSavedState = NULL;
            CBundle::New((IBundle**)&mAccessPointSavedState);
            ((CAccessPoint*)mDlgAccessPoint.Get())->SaveWifiState(mAccessPointSavedState);
            outState->PutBundle(SAVE_DIALOG_ACCESS_POINT_STATE, mAccessPointSavedState);
        }
    }
    return NOERROR;
}

ECode WifiSettings::OnOptionsItemSelected(
    /* [in] */ IMenuItem* item,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    // If the user is not allowed to configure wifi, do not handle menu selections.
    if (IsUiRestricted()) {
        *result = FALSE;
        return NOERROR;
    }

    Int32 id;
    item->GetItemId(&id);
    Boolean res;
    switch (id) {
        case MENU_ID_WPS_PBC:
            RestrictedSettingsFragment::ShowDialog(WPS_PBC_DIALOG_ID);
            *result = TRUE;
            return NOERROR;
            /*
        case MENU_ID_P2P:
            if (GetActivity() instanceof SettingsActivity) {
                ((SettingsActivity) GetActivity()).StartPreferencePanel(
                        WifiP2pSettings.class->GetCanonicalName(),
                        NULL,
                        R::string::wifi_p2p_settings_title, NULL,
                        this, 0);
            } else {
                StartFragment(this, WifiP2pSettings.class->GetCanonicalName(),
                        R::string::wifi_p2p_settings_title, -1, NULL);
            }
            return TRUE;
            */
        case MENU_ID_WPS_PIN:
            RestrictedSettingsFragment::ShowDialog(WPS_PIN_DIALOG_ID);
            *result = TRUE;
            return NOERROR;
        case MENU_ID_SCAN:
            if (mWifiManager->IsWifiEnabled(&res), res) {
                mScanner->ForceScan();
            }
            *result = TRUE;
            return NOERROR;
        case MENU_ID_ADD_NETWORK:
            if (mWifiManager->IsWifiEnabled(&res), res) {
                OnAddNetworkPressed();
            }
            *result = TRUE;
            return NOERROR;
        case MENU_ID_SAVED_NETWORK: {
                AutoPtr<IActivity> activity;
                GetActivity((IActivity**)&activity);
                if (ISettingsActivity::Probe(activity) != NULL) {
                    ((CSettingsActivity*)ISettingsActivity::Probe(activity))->StartPreferencePanel(
                            String("Elastos.Droid.Settings.Wifi.CSavedAccessPointsWifiSettings"), NULL,
                            R::string::wifi_saved_access_points_titlebar, NULL, this, 0);
                }
                else {
                    StartFragment(this, String("Elastos.Droid.Settings.Wifi.CSavedAccessPointsWifiSettings"),
                            R::string::wifi_saved_access_points_titlebar,
                            -1 /* Do not request a result */, NULL);
                }
                *result = TRUE;
                return NOERROR;
            }
        case MENU_ID_ADVANCED: {
                AutoPtr<IActivity> activity;
                GetActivity((IActivity**)&activity);
                if (ISettingsActivity::Probe(activity) != NULL) {
                    ((CSettingsActivity*)ISettingsActivity::Probe(activity))->StartPreferencePanel(
                            String("Elastos.Droid.Settings.Wifi.CAdvancedWifiSettings"), NULL,
                            R::string::wifi_advanced_titlebar, NULL, this, 0);
                }
                else {
                    StartFragment(this, String("Elastos.Droid.Settings.Wifi.CAdvancedWifiSettings"),
                            R::string::wifi_advanced_titlebar, -1 /* Do not request a results */,
                            NULL);
                }
                *result = TRUE;
                return NOERROR;
            }
    }
    return RestrictedSettingsFragment::OnOptionsItemSelected(item, result);
}

ECode WifiSettings::OnCreateContextMenu(
    /* [in] */ IContextMenu* menu,
    /* [in] */ IView* view,
    /* [in] */ IContextMenuInfo* info)
{
    if (IAdapterContextMenuInfo::Probe(info) != NULL) {
        AutoPtr<IListView> listView;
        GetListView((IListView**)&listView);
        Int32 position;
        IAdapterContextMenuInfo::Probe(info)->GetPosition(&position);
        AutoPtr<IInterface> obj;
        IAdapterView::Probe(listView)->GetItemAtPosition(position, (IInterface**)&obj);
        IPreference* preference = IPreference::Probe(obj);

        if (IAccessPoint::Probe(preference) != NULL) {
            mSelectedAccessPoint = IAccessPoint::Probe(preference);
            CAccessPoint* selectedAccessPoint = (CAccessPoint*)mSelectedAccessPoint.Get();
            menu->SetHeaderTitle(CoreUtils::Convert(selectedAccessPoint->mSsid));
            if (selectedAccessPoint->GetLevel() != -1
                    && selectedAccessPoint->GetState() == NetworkInfoDetailedState_NONE) {
                AutoPtr<IMenuItem> item;
                IMenu::Probe(menu)->Add(IMenu::NONE, MENU_ID_CONNECT, 0, R::string::wifi_menu_connect,
                        (IMenuItem**)&item);
            }
            if (selectedAccessPoint->mNetworkId != IWifiConfiguration::INVALID_NETWORK_ID) {
                AutoPtr<IActivityManagerHelper> helper;
                CActivityManagerHelper::AcquireSingleton((IActivityManagerHelper**)&helper);
                Int32 user;
                helper->GetCurrentUser(&user);
                if (user == IUserHandle::USER_OWNER) {
                    AutoPtr<IMenuItem> item;
                    IMenu::Probe(menu)->Add(IMenu::NONE, MENU_ID_FORGET, 0, R::string::wifi_menu_forget,
                            (IMenuItem**)&item);
                }

                AutoPtr<IMenuItem> menuItem;
                IMenu::Probe(menu)->Add(IMenu::NONE, MENU_ID_MODIFY, 0, R::string::wifi_menu_modify,
                        (IMenuItem**)&menuItem);

                if (selectedAccessPoint->mSecurity != AccessPoint::SECURITY_NONE) {
                    // Only allow writing of NFC tags for password-protected networks.
                    AutoPtr<IMenuItem> item;
                    IMenu::Probe(menu)->Add(IMenu::NONE, MENU_ID_WRITE_NFC, 0, R::string::wifi_menu_write_to_nfc,
                            (IMenuItem**)&item);
                }
            }
        }
    }
    return NOERROR;
}

ECode WifiSettings::OnContextItemSelected(
    /* [in] */ IMenuItem* item,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    if (mSelectedAccessPoint == NULL) {
        return RestrictedSettingsFragment::OnContextItemSelected(item, result);
    }

    CAccessPoint* selectedAccessPoint = (CAccessPoint*)mSelectedAccessPoint.Get();

    Int32 id;
    item->GetItemId(&id);
    switch (id) {
        case MENU_ID_CONNECT: {
            if (selectedAccessPoint->mNetworkId != IWifiConfiguration::INVALID_NETWORK_ID) {
                Connect(selectedAccessPoint->mNetworkId);
            }
            else if (selectedAccessPoint->mSecurity == AccessPoint::SECURITY_NONE) {
                /** Bypass dialog for unsecured networks */
                selectedAccessPoint->GenerateOpenNetworkConfig();
                Connect(selectedAccessPoint->GetConfig());
            }
            else {
                ShowDialog(mSelectedAccessPoint, TRUE);
            }
            *result = TRUE;
            return NOERROR;
        }
        case MENU_ID_FORGET: {
            mWifiManager->Forget(selectedAccessPoint->mNetworkId, mForgetListener);
            *result = TRUE;
            return NOERROR;
        }
        case MENU_ID_MODIFY: {
            ShowDialog(mSelectedAccessPoint, TRUE);
            *result = TRUE;
            return NOERROR;
        }
        case MENU_ID_WRITE_NFC:
            RestrictedSettingsFragment::ShowDialog(WRITE_NFC_DIALOG_ID);
            *result = TRUE;
            return NOERROR;

    }
    return RestrictedSettingsFragment::OnContextItemSelected(item, result);
}

ECode WifiSettings::OnPreferenceTreeClick(
    /* [in] */ IPreferenceScreen* screen,
    /* [in] */ IPreference* preference,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);

    if (IAccessPoint::Probe(preference)) {
        mSelectedAccessPoint = IAccessPoint::Probe(preference);
        CAccessPoint* selectedAccessPoint = (CAccessPoint*)mSelectedAccessPoint.Get();
        /** Bypass dialog for unsecured, unsaved networks */
        if (selectedAccessPoint->mSecurity == AccessPoint::SECURITY_NONE &&
                selectedAccessPoint->mNetworkId == IWifiConfiguration::INVALID_NETWORK_ID) {
            selectedAccessPoint->GenerateOpenNetworkConfig();
            if (!mSavedNetworksExist) {
                mSavedNetworksExist = TRUE;
                AutoPtr<IActivity> activity;
                GetActivity((IActivity**)&activity);
                activity->InvalidateOptionsMenu();
            }
            Connect(selectedAccessPoint->GetConfig());
        }
        else {
            ShowDialog(mSelectedAccessPoint, FALSE);
        }
    }
    else {
        return RestrictedSettingsFragment::OnPreferenceTreeClick(screen, preference, result);
    }
    *result = TRUE;
    return NOERROR;
}

void WifiSettings::ShowDialog(
    /* [in] */ IAccessPoint* accessPoint,
    /* [in] */ Boolean edit)
{
    if (mDialog != NULL) {
        RemoveDialog(WIFI_DIALOG_ID);
        mDialog = NULL;
    }

    // Save the access point and edit mode
    mDlgAccessPoint = accessPoint;
    mDlgEdit = edit;

    RestrictedSettingsFragment::ShowDialog(WIFI_DIALOG_ID);
}

ECode WifiSettings::OnCreateDialog(
    /* [in] */ Int32 dialogId,
    /* [out] */ IDialog** dialog)
{
    VALIDATE_NOT_NULL(dialog);

    switch (dialogId) {
        case WIFI_DIALOG_ID: {
            AutoPtr<IAccessPoint> ap = mDlgAccessPoint; // For manual launch
            AutoPtr<IActivity> activity;
            GetActivity((IActivity**)&activity);
            if (ap == NULL) { // For re-launch from saved state
                if (mAccessPointSavedState != NULL) {
                    CAccessPoint::New(IContext::Probe(activity),
                            mAccessPointSavedState, (IAccessPoint**)&ap);
                    // For repeated orientation changes
                    mDlgAccessPoint = ap;
                    // Reset the saved access point data
                    mAccessPointSavedState = NULL;
                }
            }
            // If it's NULL, fine, it's for Add Network
            mSelectedAccessPoint = ap;
            mDialog = NULL;
            CWifiDialog::NewByFriend(IContext::Probe(activity), this, ap, mDlgEdit, (CWifiDialog**)&mDialog);
            AutoPtr<IAlertDialog> alertDialog = (IAlertDialog*)mDialog.Get();
            *dialog = IDialog::Probe(alertDialog);
            REFCOUNT_ADD(*dialog);
            return NOERROR;
        }
        case WPS_PBC_DIALOG_ID: {
            AutoPtr<IActivity> activity;
            GetActivity((IActivity**)&activity);
            return CWpsDialog::New(IContext::Probe(activity), IWpsInfo::PBC, dialog);
        }
        case WPS_PIN_DIALOG_ID: {
            AutoPtr<IActivity> activity;
            GetActivity((IActivity**)&activity);
            return CWpsDialog::New(IContext::Probe(activity), IWpsInfo::DISPLAY, dialog);
        }
        case WRITE_NFC_DIALOG_ID: {
            if (mSelectedAccessPoint != NULL) {
                AutoPtr<IActivity> activity;
                GetActivity((IActivity**)&activity);
                mWifiToNfcDialog = NULL;
                CWriteWifiConfigToNfcDialog::New(IContext::Probe(activity),
                        mSelectedAccessPoint, mWifiManager, (IDialog**)&mWifiToNfcDialog);
                *dialog = mWifiToNfcDialog;
                REFCOUNT_ADD(*dialog);
                return NOERROR;
            }
        }

    }
    return RestrictedSettingsFragment::OnCreateDialog(dialogId, dialog);
}

void WifiSettings::UpdateAccessPoints()
{
    // Safeguard from some delayed event handling
    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    if (activity == NULL) return;

    if (IsUiRestricted()) {
        AddMessagePreference(R::string::wifi_empty_list_user_restricted);
        return;
    }
    Int32 wifiState;
    mWifiManager->GetWifiState(&wifiState);

    //when we update the screen, check if verbose logging has been turned on or off
    mWifiManager->GetVerboseLoggingLevel(&mVerboseLogging);

    switch (wifiState) {
        case IWifiManager::WIFI_STATE_ENABLED: {
            // AccessPoints are automatically sorted with TreeSet.
            AutoPtr<IList> ap = ConstructAccessPoints(IContext::Probe(activity),
                    mWifiManager, mLastInfo, mLastState);
            ICollection* accessPoints = ICollection::Probe(ap);
            AutoPtr<IPreferenceScreen> screen;
            GetPreferenceScreen((IPreferenceScreen**)&screen);
            IPreferenceGroup* screenGroup = IPreferenceGroup::Probe(screen);
            screenGroup->RemoveAll();
            Int32 size;
            if ((accessPoints->GetSize(&size), size) == 0) {
                AddMessagePreference(R::string::wifi_empty_list_wifi_on);
            }

            AutoPtr<IListView> listView;
            GetListView((IListView**)&listView);
            Boolean res;
            listView->RemoveHeaderView(mWifiAssistantCard, &res);
            if (mWifiAssistantApp != NULL) {
                listView->AddHeaderView(mWifiAssistantCard);
            }

            AutoPtr<IIterator> iter;
            accessPoints->GetIterator((IIterator**)&iter);
            Boolean hasNext;
            while (iter->HasNext(&hasNext), hasNext) {
                AutoPtr<IInterface> obj;
                iter->GetNext((IInterface**)&obj);
                IAccessPoint* accessPoint = IAccessPoint::Probe(obj);
                // Ignore access points that are out of range.
                if (((CAccessPoint*)accessPoint)->GetLevel() != -1) {
                    screenGroup->AddPreference(IPreference::Probe(accessPoint), &res);
                }
            }
            break;
        }

        case IWifiManager::WIFI_STATE_ENABLING: {
            AutoPtr<IPreferenceScreen> screen;
            GetPreferenceScreen((IPreferenceScreen**)&screen);
            IPreferenceGroup::Probe(screen)->RemoveAll();
            break;
        }

        case IWifiManager::WIFI_STATE_DISABLING:
            AddMessagePreference(R::string::wifi_stopping);
            break;

        case IWifiManager::WIFI_STATE_DISABLED:
            SetOffMessage();
            break;
    }
}

AutoPtr<INetworkScorerAppData> WifiSettings::GetWifiAssistantApp(
    /* [in] */ IContext* context)
{
    AutoPtr<INetworkScorerAppManager> appManager;
    CNetworkScorerAppManager::AcquireSingleton((INetworkScorerAppManager**)&appManager);
    AutoPtr<ICollection> scorers;
    appManager->GetAllValidScorers(context, (ICollection**)&scorers);

    Boolean res;
    if (scorers->IsEmpty(&res), res) {
        return NULL;
    }

    // TODO: b/13780935 - Implement proper scorer selection. Rather than pick the first
    // scorer on the system, we should allow the user to select one.
    AutoPtr<IIterator> iter;
    scorers->GetIterator((IIterator**)&iter);
    AutoPtr<IInterface> obj;
    iter->GetNext((IInterface**)&obj);
    INetworkScorerAppData* appData = INetworkScorerAppData::Probe(obj);
    return appData;
}

void WifiSettings::PrepareWifiAssistantCard()
{
    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    if (IWifiPickerActivity::Probe(activity) != NULL) {
        return;
    }

    AutoPtr<INetworkScorerAppManager> appManager;
    CNetworkScorerAppManager::AcquireSingleton((INetworkScorerAppManager**)&appManager);
    AutoPtr<INetworkScorerAppData> appData;
    appManager->GetActiveScorer(IContext::Probe(activity), (INetworkScorerAppData**)&appData);
    if (appData != NULL) {
        // A scorer is already enabled; don't show the card.
        return;
    }

    AutoPtr<ICollection> scorers;
    appManager->GetAllValidScorers(IContext::Probe(activity), (ICollection**)&scorers);
    Boolean res;
    if (scorers->IsEmpty(&res), res) {
        // No scorers are available to enable; don't show the card.
        return;
    }

    AutoPtr<IPreferenceScreen> screen;
    GetPreferenceScreen((IPreferenceScreen**)&screen);
    AutoPtr<ISharedPreferences> sharedPreferences;
    IPreference::Probe(screen)->GetSharedPreferences((ISharedPreferences**)&sharedPreferences);
    Int32 lastDismissPlatform;
    sharedPreferences->GetInt32(KEY_ASSISTANT_DISMISS_PLATFORM, 0, &lastDismissPlatform);

    if (Build::VERSION::SDK_INT <= lastDismissPlatform) {
        // User has dismissed the Wi-Fi assistant card on this SDK release. Suppress the card
        // until the next major platform upgrade.
        return;
    }

    // TODO: b/13780935 - Implement proper scorer selection. Rather than pick the first
    // scorer on the system, we should allow the user to select one.
    AutoPtr<IIterator> iter;
    scorers->GetIterator((IIterator**)&iter);
    AutoPtr<IInterface> obj;
    iter->GetNext((IInterface**)&obj);
    mWifiAssistantApp = INetworkScorerAppData::Probe(obj);

    if (mWifiAssistantCard == NULL) {
        AutoPtr<IListView> listView;
        GetListView((IListView**)&listView);
        AutoPtr<ILayoutInflater> inflater;
        LayoutInflater::From(IContext::Probe(activity), (ILayoutInflater**)&inflater);
        inflater->Inflate(R::layout::wifi_assistant_card,
                IViewGroup::Probe(listView), FALSE, (IView**)&mWifiAssistantCard);
        AutoPtr<IView> view;
        mWifiAssistantCard->FindViewById(R::id::setup, (IView**)&view);
        IButton* setup = IButton::Probe(view);
        view = NULL;
        mWifiAssistantCard->FindViewById(R::id::no_thanks_button, (IView**)&view);
        IButton* noThanks = IButton::Probe(view);
        view = NULL;
        mWifiAssistantCard->FindViewById(R::id::wifi_assistant_text, (IView**)&view);
        ITextView* assistantText = ITextView::Probe(view);
        AutoPtr<ICharSequence> scorerName;
        mWifiAssistantApp->GetScorerName((ICharSequence**)&scorerName);
        AutoPtr<IResources> resources;
        GetResources((IResources**)&resources);
        AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
        args->Set(0, scorerName);
        String str;
        resources->GetString(R::string::wifi_assistant_title_message, args, &str);
        assistantText->SetText(CoreUtils::Convert(str));

        if (setup != NULL && noThanks != NULL) {
            AutoPtr<ViewOnClickListener> listener1 = new ViewOnClickListener(this, 0);
            IView::Probe(setup)->SetOnClickListener((IViewOnClickListener*)listener1);

            AutoPtr<ViewOnClickListener> listener2 = new ViewOnClickListener(this, 1);
            IView::Probe(noThanks)->SetOnClickListener((IViewOnClickListener*)listener2);
        }
    }
}

void WifiSettings::DisableWifiAssistantCardUntilPlatformUpgrade()
{
    AutoPtr<IPreferenceScreen> screen;
    GetPreferenceScreen((IPreferenceScreen**)&screen);
    AutoPtr<ISharedPreferences> sharedPreferences;
    IPreference::Probe(screen)->GetSharedPreferences((ISharedPreferences**)&sharedPreferences);
    AutoPtr<ISharedPreferencesEditor> editor;
    sharedPreferences->Edit((ISharedPreferencesEditor**)&editor);
    editor->PutInt32(KEY_ASSISTANT_DISMISS_PLATFORM, Build::VERSION::SDK_INT);
    editor->Apply();
}

AutoPtr<ITextView> WifiSettings::InitEmptyView()
{
    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    AutoPtr<IView> view;
    activity->FindViewById(Elastos::Droid::R::id::empty, (IView**)&view);
    ITextView* emptyView = ITextView::Probe(view);
    AutoPtr<IListView> listView;
    GetListView((IListView**)&listView);
    IAdapterView::Probe(listView)->SetEmptyView(view);
    return emptyView;
}

void WifiSettings::SetOffMessage()
{
    if (mEmptyView != NULL) {
        mEmptyView->SetText(R::string::wifi_empty_list_wifi_off);
        AutoPtr<IActivity> activity;
        GetActivity((IActivity**)&activity);
        AutoPtr<IContentResolver> resolver;
        IContext::Probe(activity)->GetContentResolver((IContentResolver**)&resolver);

        AutoPtr<ISettingsGlobal> global;
        CSettingsGlobal::AcquireSingleton((ISettingsGlobal**)&global);
        Int32 data;
        if ((global->GetInt32(resolver,
                ISettingsGlobal::WIFI_SCAN_ALWAYS_AVAILABLE, 0, &data), data) == 1) {
            mEmptyView->Append(CoreUtils::Convert("\n\n"));
            Int32 resId;
            AutoPtr<ISettingsSecure> secure;
            CSettingsSecure::AcquireSingleton((ISettingsSecure**)&secure);
            Boolean res;
            if (secure->IsLocationProviderEnabled(
                    resolver, ILocationManager::NETWORK_PROVIDER, &res), res) {
                resId = R::string::wifi_scan_notify_text_location_on;
            }
            else {
                resId = R::string::wifi_scan_notify_text_location_off;
            }
            AutoPtr<ICharSequence> charSeq;
            GetText(resId, (ICharSequence**)&charSeq);
            mEmptyView->Append(charSeq);
        }
    }
    AutoPtr<IPreferenceScreen> screen;
    GetPreferenceScreen((IPreferenceScreen**)&screen);
    IPreferenceGroup::Probe(screen)->RemoveAll();
}

void WifiSettings::AddMessagePreference(
    /* [in] */ Int32 messageId)
{
    if (mEmptyView != NULL) mEmptyView->SetText(messageId);
    AutoPtr<IPreferenceScreen> screen;
    GetPreferenceScreen((IPreferenceScreen**)&screen);
    IPreferenceGroup::Probe(screen)->RemoveAll();
}

AutoPtr<IList> WifiSettings::ConstructAccessPoints(
    /* [in] */ IContext* context,
    /* [in] */ IWifiManager* wifiManager,
    /* [in] */ IWifiInfo* lastInfo,
    /* [in] */ NetworkInfoDetailedState lastState)
{
    AutoPtr<IList> accessPoints;
    CArrayList::New((IList**)&accessPoints);
    /** Lookup table to more quickly update AccessPoints by only considering objects with the
     * correct SSID.  Maps SSID -> List of AccessPoints with the given SSID.  */
    AutoPtr<Multimap> apMap = new Multimap();

    AutoPtr<IList> configs;
    wifiManager->GetConfiguredNetworks((IList**)&configs);
    if (configs != NULL) {
        // Update "Saved Networks" menu option.
        Int32 size;
        configs->GetSize(&size);
        if (mSavedNetworksExist != (size > 0)) {
            mSavedNetworksExist = !mSavedNetworksExist;
            if (IActivity::Probe(context) != NULL) {
                IActivity::Probe(context)->InvalidateOptionsMenu();
            }
        }

        for (Int32 i = 0; i < size; i++) {
            AutoPtr<IInterface> obj;
            configs->Get(i, (IInterface**)&obj);
            IWifiConfiguration* config = IWifiConfiguration::Probe(obj);
            Boolean selfAdded;
            Int32 numAssociation;
            if ((config->GetSelfAdded(&selfAdded), selfAdded) &&
                    (config->GetNumAssociation(&numAssociation), numAssociation) == 0) {
                continue;
            }

            AutoPtr<IAccessPoint> accessPoint;
            CAccessPoint::New(context, config, (IAccessPoint**)&accessPoint);
            if (lastInfo != NULL && lastState != NetworkInfoDetailedState_NONE) {
                ((CAccessPoint*)accessPoint.Get())->Update(lastInfo, lastState);
            }
            accessPoints->Add(accessPoint);
            apMap->Put(((CAccessPoint*)accessPoint.Get())->mSsid, accessPoint);
        }
    }

    AutoPtr<IList> results;
    wifiManager->GetScanResults((IList**)&results);
    if (results != NULL) {
        Int32 size;
        results->GetSize(&size);
        for (Int32 i = 0; i < size; i++) {
            AutoPtr<IInterface> obj;
            results->Get(i, (IInterface**)&obj);
            IScanResult* result = IScanResult::Probe(obj);
            assert(result != NULL);

            // Ignore hidden and ad-hoc networks.
            String SSID;
            result->GetSSID(&SSID);
            String capabilities;
            if (SSID.IsNullOrEmpty() ||
                (result->GetCapabilities(&capabilities), capabilities).Contains("[IBSS]")) {
                continue;
            }

            Boolean found = FALSE;
            AutoPtr<IList> list = apMap->GetAll(SSID);
            Int32 len;
            list->GetSize(&len);
            for (Int32 j = 0; j < len; j++) {
                AutoPtr<IInterface> object;
                list->Get(j, (IInterface**)&object);
                CAccessPoint* accessPoint = (CAccessPoint*)IAccessPoint::Probe(object);
                if (accessPoint->Update(result)) {
                    found = TRUE;
                }
            }

            if (!found) {
                AutoPtr<IAccessPoint> accessPoint;
                CAccessPoint::New(context, result, (IAccessPoint**)&accessPoint);
                accessPoints->Add(accessPoint);
                apMap->Put(((CAccessPoint*)accessPoint.Get())->mSsid, accessPoint);
            }
        }
    }

    // Pre-sort accessPoints to speed preference insertion
    AutoPtr<ICollections> coll;
    CCollections::AcquireSingleton((ICollections**)&coll);
    coll->Sort(accessPoints);
    return accessPoints;
}

void WifiSettings::HandleEvent(
    /* [in] */ IIntent* intent)
{
    String action;
    intent->GetAction(&action);
    if (IWifiManager::WIFI_STATE_CHANGED_ACTION.Equals(action)) {
        Int32 value;
        intent->GetInt32Extra(IWifiManager::EXTRA_WIFI_STATE,
                IWifiManager::WIFI_STATE_UNKNOWN, &value);
        UpdateWifiState(value);
    }
    else if (IWifiManager::SCAN_RESULTS_AVAILABLE_ACTION.Equals(action) ||
            IWifiManager::CONFIGURED_NETWORKS_CHANGED_ACTION.Equals(action) ||
            IWifiManager::LINK_CONFIGURATION_CHANGED_ACTION.Equals(action)) {
            UpdateAccessPoints();
    }
    else if (IWifiManager::NETWORK_STATE_CHANGED_ACTION.Equals(action)) {
        AutoPtr<IParcelable> parcel;
        intent->GetParcelableExtra(
                IWifiManager::EXTRA_NETWORK_INFO, (IParcelable**)&parcel);
        INetworkInfo* info = INetworkInfo::Probe(parcel);
        Boolean res;
        info->IsConnected(&res);
        mConnected->Set(res);
        ChangeNextButtonState(res);
        UpdateAccessPoints();
        NetworkInfoDetailedState state;
        info->GetDetailedState(&state);
        UpdateConnectionState(state);
    }
    else if (IWifiManager::RSSI_CHANGED_ACTION.Equals(action)) {
        UpdateConnectionState(NetworkInfoDetailedState_NONE);
    }
}

void WifiSettings::UpdateConnectionState(
    /* [in] */ NetworkInfoDetailedState state)
{
    /* sticky broadcasts can call this when wifi is disabled */
    Boolean res;
    if (mWifiManager->IsWifiEnabled(&res), !res) {
        mScanner->Pause();
        return;
    }

    if (state == NetworkInfoDetailedState_OBTAINING_IPADDR) {
        mScanner->Pause();
    }
    else {
        mScanner->Resume();
    }

    mLastInfo = NULL;
    mWifiManager->GetConnectionInfo((IWifiInfo**)&mLastInfo);
    if (state != NetworkInfoDetailedState_NONE) {
        mLastState = state;
    }

    AutoPtr<IPreferenceScreen> screen;
    GetPreferenceScreen((IPreferenceScreen**)&screen);
    Int32 count;
    IPreferenceGroup::Probe(screen)->GetPreferenceCount(&count);
    for (Int32 i = count - 1; i >= 0; --i) {
        // Maybe there's a WifiConfigPreference
        AutoPtr<IPreference> preference;
        IPreferenceGroup::Probe(screen)->GetPreference(i, (IPreference**)&preference);
        if (IAccessPoint::Probe(preference) != NULL) {
            IAccessPoint* accessPoint = IAccessPoint::Probe(preference);
            ((CAccessPoint*)accessPoint)->Update(mLastInfo, mLastState);
        }
    }
}

void WifiSettings::UpdateWifiState(
    /* [in] */ Int32 state)
{
    AutoPtr<IActivity> activity;
    GetActivity((IActivity**)&activity);
    if (activity != NULL) {
        activity->InvalidateOptionsMenu();
    }

    switch (state) {
        case IWifiManager::WIFI_STATE_ENABLED:
            mScanner->Resume();
            return; // not break, to avoid the call to Pause() below

        case IWifiManager::WIFI_STATE_ENABLING:
            AddMessagePreference(R::string::wifi_starting);
            break;

        case IWifiManager::WIFI_STATE_DISABLED:
            SetOffMessage();
            break;
    }

    mLastInfo = NULL;
    mLastState = NetworkInfoDetailedState_NONE;
    mScanner->Pause();
}

void WifiSettings::ChangeNextButtonState(
    /* [in] */ Boolean enabled)
{
    if (mEnableNextOnConnection && HasNextButton()) {
        AutoPtr<IButton> button = GetNextButton();
        IView::Probe(button)->SetEnabled(enabled);
    }
}

ECode WifiSettings::OnClick(
    /* [in] */ IDialogInterface* dialogInterface,
    /* [in] */ Int32 button)
{
    if (button == CWifiDialog::BUTTON_FORGET && mSelectedAccessPoint != NULL) {
        Forget();
    }
    else if (button == CWifiDialog::BUTTON_SUBMIT) {
        if (mDialog != NULL) {
            AutoPtr<IWifiConfigController> controller;
            mDialog->GetController((IWifiConfigController**)&controller);
            Submit(controller);
        }
    }
    return NOERROR;
}

void WifiSettings::Submit(
    /* [in] */ IWifiConfigController* configController)
{
    AutoPtr<IWifiConfiguration> config = ((WifiConfigController*)configController)->GetConfig();

    Int32 networkId;
    if (config == NULL) {
        if (mSelectedAccessPoint != NULL
                && ((CAccessPoint*)mSelectedAccessPoint.Get())->mNetworkId != IWifiConfiguration::INVALID_NETWORK_ID) {
            Connect(((CAccessPoint*)mSelectedAccessPoint.Get())->mNetworkId);
        }
    }
    else if (config->GetNetworkId(&networkId), networkId != IWifiConfiguration::INVALID_NETWORK_ID) {
        if (mSelectedAccessPoint != NULL) {
            mWifiManager->Save(config, mSaveListener);
        }
    }
    else {
        if (((WifiConfigController*)configController)->IsEdit()) {
            mWifiManager->Save(config, mSaveListener);
        }
        else {
            Connect(config);
        }
    }

    Boolean res;
    if (mWifiManager->IsWifiEnabled(&res), res) {
        mScanner->Resume();
    }
    UpdateAccessPoints();
}

void WifiSettings::Forget()
{
    CAccessPoint* selectedAccessPoint = (CAccessPoint*)mSelectedAccessPoint.Get();
    if (selectedAccessPoint->mNetworkId == IWifiConfiguration::INVALID_NETWORK_ID) {
        // Should not happen, but a monkey seems to trigger it
        Logger::E(TAG, "Failed to forget invalid network %s", TO_CSTR(selectedAccessPoint->GetConfig()));
        return;
    }

    mWifiManager->Forget(selectedAccessPoint->mNetworkId, mForgetListener);

    Boolean res;
    if (mWifiManager->IsWifiEnabled(&res), res) {
        mScanner->Resume();
    }
    UpdateAccessPoints();

    // We need to rename/replace "Next" button in wifi setup context.
    ChangeNextButtonState(FALSE);
}

void WifiSettings::Connect(
    /* [in] */ IWifiConfiguration* config)
{
    mWifiManager->Connect(config, mConnectListener);
}

void WifiSettings::Connect(
    /* [in] */ Int32 networkId)
{
    mWifiManager->Connect(networkId, mConnectListener);
}

void WifiSettings::RefreshAccessPoints()
{
    Boolean res;
    if (mWifiManager->IsWifiEnabled(&res), res) {
        mScanner->Resume();
    }

    AutoPtr<IPreferenceScreen> screen;
    GetPreferenceScreen((IPreferenceScreen**)&screen);
    IPreferenceGroup::Probe(screen)->RemoveAll();
}

void WifiSettings::OnAddNetworkPressed()
{
    // No exact access point is selected.
    mSelectedAccessPoint = NULL;
    ShowDialog(NULL, TRUE);
}

Int32 WifiSettings::GetAccessPointsCount()
{
    Boolean wifiIsEnabled;
    mWifiManager->IsWifiEnabled(&wifiIsEnabled);
    if (wifiIsEnabled) {
        AutoPtr<IPreferenceScreen> screen;
        GetPreferenceScreen((IPreferenceScreen**)&screen);
        Int32 count;
        IPreferenceGroup::Probe(screen)->GetPreferenceCount(&count);
        return count;
    }
    else {
        return 0;
    }
}

void WifiSettings::PauseWifiScan()
{
    Boolean res;
    if (mWifiManager->IsWifiEnabled(&res), res) {
        mScanner->Pause();
    }
}

void WifiSettings::ResumeWifiScan()
{
    Boolean res;
    if (mWifiManager->IsWifiEnabled(&res), res) {
        mScanner->Resume();
    }
}

Int32 WifiSettings::GetHelpResource()
{
    return R::string::help_url_wifi;
}

} // namespace Wifi
} // namespace Settings
} // namespace Droid
} // namespace Elastos
