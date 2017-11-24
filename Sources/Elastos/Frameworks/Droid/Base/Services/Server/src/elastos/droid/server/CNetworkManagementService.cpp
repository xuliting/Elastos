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

#include "elastos/droid/server/CNetworkManagementService.h"
#include "elastos/droid/server/FgThread.h"
#include "elastos/droid/server/NativeDaemonEvent.h"
#include "elastos/droid/server/Watchdog.h"
#include "elastos/droid/server/net/LockdownVpnTracker.h"
#include <elastos/droid/Manifest.h>
#include <elastos/droid/os/SystemClock.h>
#include <Elastos.CoreLibrary.h>
#include <Elastos.Droid.Wifi.h>
#include <cutils/log.h>
#include <elastos/core/AutoLock.h>
#include <elastos/core/StringBuffer.h>
#include <elastos/core/StringUtils.h>
#include <elastos/droid/Manifest.h>
#include <elastos/droid/net/IpPrefix.h>
#include <elastos/droid/os/Binder.h>
#include <elastos/droid/os/Looper.h>
#include <elastos/droid/os/Process.h>
#include <elastos/droid/os/ServiceManager.h>
#include <elastos/droid/os/SystemClock.h>
#include <elastos/droid/os/SystemProperties.h>
#include <elastos/droid/server/NetworkManagementSocketTagger.h>
#include "elastos/droid/R.h"
#include <elastos/core/CoreUtils.h>
#include <elastos/utility/Arrays.h>
#include <elastos/utility/logging/Logger.h>
#include <elastos/utility/logging/Slogger.h>

#include <elastos/core/AutoLock.h>
using Elastos::Core::AutoLock;
using Elastos::Droid::App::IService;
using Elastos::Droid::Content::IContext;
using Elastos::Droid::Content::Res::IResources;
using Elastos::Droid::Internal::Net::CNetworkStatsFactory;
using Elastos::Droid::Internal::Utility::IPreconditions;
using Elastos::Droid::Internal::Utility::CPreconditions;
using Elastos::Droid::Manifest;
using Elastos::Droid::Net::CConnectivityManager;
using Elastos::Droid::Net::CConnectivityManagerHelper;
using Elastos::Droid::Net::CIpPrefix;
using Elastos::Droid::Net::IConnectivityManager;
using Elastos::Droid::Net::IConnectivityManagerHelper;
using Elastos::Droid::Net::IIpPrefix;
using Elastos::Droid::Net::ITrafficStats;
using Elastos::Droid::Os::CHandler;
using Elastos::Droid::Os::CRemoteCallbackList;
using Elastos::Droid::Os::CServiceManager;
using Elastos::Droid::Os::Binder;
using Elastos::Droid::Os::CSystemProperties;
using Elastos::Droid::Os::EIID_IBinder;
using Elastos::Droid::Os::EIID_IINetworkManagementService;
using Elastos::Droid::Os::IBatteryStats;
using Elastos::Droid::Os::IPowerManager;
using Elastos::Droid::Os::IPowerManagerWakeLock;
using Elastos::Droid::Os::IServiceManager;
using Elastos::Droid::Os::ISystemProperties;
using Elastos::Droid::Os::Looper;
using Elastos::Droid::Os::Process;
using Elastos::Droid::Os::ServiceManager;
using Elastos::Droid::Os::SystemClock;
using Elastos::Droid::Os::SystemProperties;
using Elastos::Droid::R;
using Elastos::Droid::Server::Net::LockdownVpnTracker;
using Elastos::Droid::Telephony::ITelephonyManager;
using Elastos::Droid::Telephony::ISubscriptionManager;
using Elastos::Droid::Utility::CSparseBooleanArray;
using Elastos::Core::CObject;
using Elastos::Core::CoreUtils;
using Elastos::Core::CString;
using Elastos::Core::CSystem;
using Elastos::Core::CThread;
using Elastos::Core::ICharSequence;
using Elastos::Core::ISystem;
using Elastos::Core::StringBuffer;
using Elastos::Core::StringUtils;
using Elastos::IO::CFile;
using Elastos::IO::IBuffer;
using Elastos::IO::IDataInput;
using Elastos::IO::IInputStream;
using Elastos::IO::IReader;
using Elastos::Net::CInetAddressHelper;
using Elastos::Net::CURI;
using Elastos::Net::IInetAddressHelper;
using Elastos::Utility::Arrays;
using Elastos::Utility::CArrayList;
using Elastos::Utility::CHashMap;
using Elastos::Utility::CStringTokenizer;
using Elastos::Utility::IArrayList;
using Elastos::Utility::IIterator;
using Elastos::Utility::IMapEntry;
using Elastos::Utility::IStringTokenizer;
using Elastos::Utility::Logging::Logger;
using Elastos::Utility::Logging::Slogger;
using Elastos::Utility::IIterable;

namespace Elastos {
namespace Droid {
namespace Server {

//=============================================================================
// CNetworkManagementService::IdleTimerParams
//=============================================================================
CNetworkManagementService::IdleTimerParams::IdleTimerParams(
    /* [in] */ Int32 timeout,
    /* [in] */ Int32 type)
    : mTimeout(timeout)
    , mType(type)
    , mNetworkCount(1)
{}

//=============================================================================
// CNetworkManagementService::NetdCallbackReceiver::ReceiverRunnable
//=============================================================================
CAR_INTERFACE_IMPL(CNetworkManagementService::NetdCallbackReceiver::ReceiverRunnable, Object, IRunnable)

CNetworkManagementService::NetdCallbackReceiver::ReceiverRunnable::ReceiverRunnable(
    /* [in] */ CNetworkManagementService* host)
    : mHost(host)
{}

ECode CNetworkManagementService::NetdCallbackReceiver::ReceiverRunnable::Run()
{
    mHost->PrepareNativeDaemon();
    return NOERROR;
}

//=============================================================================
// CNetworkManagementService::NetdCallbackReceiver
//=============================================================================
CAR_INTERFACE_IMPL(CNetworkManagementService::NetdCallbackReceiver, Object, INativeDaemonConnectorCallbacks)

CNetworkManagementService::NetdCallbackReceiver::NetdCallbackReceiver(
    /* [in] */ CNetworkManagementService* host)
    : mHost(host)
{}

ECode CNetworkManagementService::NetdCallbackReceiver::OnDaemonConnected()
{
    // event is dispatched from internal NDC thread, so we prepare the
    // daemon back on main thread.
    if (mHost->mConnectedSignal != NULL) {
        mHost->mConnectedSignal->CountDown();
        mHost->mConnectedSignal = NULL;
    }
    else {
        AutoPtr<IRunnable> task = new ReceiverRunnable(mHost);
        Boolean result;
        mHost->mFgHandler->Post(task, &result);
    }
    return NOERROR;
}

ECode CNetworkManagementService::NetdCallbackReceiver::OnCheckHoldWakeLock(
    /* [in] */ Int32 code,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    *result = code == NetdResponseCode::InterfaceClassActivity;
    return NOERROR;
}

ECode CNetworkManagementService::NetdCallbackReceiver::OnEvent(
    /* [in] */ Int32 code,
    /* [in] */ const String& raw,
    /* [in] */ ArrayOf<String>* cooked,
    /* [out] */ Boolean* result)
{
    String errorMessage;
    errorMessage.AppendFormat("Invalid event from daemon (%s)", raw.string());
    switch (code) {
    case NetdResponseCode::InterfaceChange:
        /*
         * a network interface change occured
         * Format: "NNN Iface added <name>"
         *         "NNN Iface removed <name>"
         *         "NNN Iface changed <name> <up/down>"
         *         "NNN Iface linkstatus <name> <up/down>"
         */
        if (cooked->GetLength() < 4 || !(*cooked)[1].Equals("Iface")) {
            Logger::E(TAG, "%s", errorMessage.string());
            return E_ILLEGAL_STATE_EXCEPTION;
        }
        if ((*cooked)[2].Equals("added")) {
            mHost->NotifyInterfaceAdded((*cooked)[3]);
            *result = TRUE;
            return NOERROR;
        }
        else if ((*cooked)[2].Equals("removed")) {
            mHost->NotifyInterfaceRemoved((*cooked)[3]);
            *result = TRUE;
            return NOERROR;
        }
        else if ((*cooked)[2].Equals("changed") && cooked->GetLength() == 5) {
            mHost->NotifyInterfaceStatusChanged((*cooked)[3], (*cooked)[4].Equals("up"));
            *result = TRUE;
            return NOERROR;
        }
        else if ((*cooked)[2].Equals("linkstate") && cooked->GetLength() == 5) {
            mHost->NotifyInterfaceLinkStateChanged((*cooked)[3], (*cooked)[4].Equals("up"));
            *result = TRUE;
            return NOERROR;
        }
        Logger::E(TAG, "%s", errorMessage.string());
        return E_ILLEGAL_STATE_EXCEPTION;
        // break;
    case NetdResponseCode::BandwidthControl:
        /*
         * Bandwidth control needs some attention
         * Format: "NNN limit alert <alertName> <ifaceName>"
         */
        if (cooked->GetLength() < 5 || !(*cooked)[1].Equals("limit")) {
            Logger::E(TAG, "%s", errorMessage.string());
            return E_ILLEGAL_STATE_EXCEPTION;
        }
        if ((*cooked)[2].Equals("alert")) {
            mHost->NotifyLimitReached((*cooked)[3], (*cooked)[4]);
            *result = TRUE;
            return NOERROR;
        }
        Logger::E(TAG, "%s", errorMessage.string());
        return E_ILLEGAL_STATE_EXCEPTION;
        // break;
    case NetdResponseCode::InterfaceMessage: {
        /*
         * An message arrived in network interface.
         * Format: "NNN IfaceMessage <3>AP-STA-CONNECTED 00:08:22:64:9d:84
         */
        if (cooked->GetLength() < 3 || !(*cooked)[1].Equals("IfaceMessage")) {
            // throw new IllegalStateException(errorMessage);
            return E_ILLEGAL_STATE_EXCEPTION;
        }
        Slogger::D(TAG, "onEvent: %d", raw.string());
        if(!((*cooked)[4]).IsNull()) {
            String str = (*cooked)[3];
            str += " ";
            str += (*cooked)[4];
            mHost->NotifyInterfaceMessage(str);
        }
        else {
            mHost->NotifyInterfaceMessage((*cooked)[3]);
        }
        return TRUE;
        // break;
    }
    case NetdResponseCode::InterfaceClassActivity:
    {
        /*
         * An network interface class state changed (active/idle)
         * Format: "NNN IfaceClass <active/idle> <label>"
         */
        if (cooked->GetLength() < 4 || !(*cooked)[1].Equals("IfaceClass")) {
            Logger::E(TAG, "%s", errorMessage.string());
            return E_ILLEGAL_STATE_EXCEPTION;
        }
        Int64 timestampNanos = 0;
        if (cooked->GetLength() == 5) {
            // try {
            timestampNanos = StringUtils::ParseInt64((*cooked)[4]);
            // } catch(NumberFormatException ne) {}
        } else {
            timestampNanos = SystemClock::GetElapsedRealtimeNanos();
        }
        Boolean isActive = (*cooked)[2].Equals("active");
        mHost->NotifyInterfaceClassActivity(StringUtils::ParseInt32((*cooked)[3]),
                isActive ? IDataConnectionRealTimeInfo::DC_POWER_STATE_HIGH
                : IDataConnectionRealTimeInfo::DC_POWER_STATE_LOW, timestampNanos, FALSE);
        *result = TRUE;
        return NOERROR;
        // break;
    }
    case NetdResponseCode::InterfaceAddressChange:
    {
        /*
         * A network address change occurred
         * Format: "NNN Address updated <addr> <iface> <flags> <scope>"
         *         "NNN Address removed <addr> <iface> <flags> <scope>"
         */
        if (cooked->GetLength() < 7 || !(*cooked)[1].Equals("Address")) {
            Logger::E(TAG, "%s", errorMessage.string());
            return E_ILLEGAL_STATE_EXCEPTION;
        }

        String iface = (*cooked)[4];
        AutoPtr<ILinkAddress> address;
        // try {
        ECode ec;
        do {
            Int32 flags;
            ec = StringUtils::Parse((*cooked)[5], &flags);
            if (FAILED(ec)) break;
            Int32 scope;
            ec = StringUtils::Parse((*cooked)[6], &scope);
            if (FAILED(ec)) break;
            ec = CLinkAddress::New((*cooked)[3], flags, scope, (ILinkAddress**)&address);
        } while(FALSE);
        if (FAILED(ec)) {
        // } catch(NumberFormatException e) {     // Non-numeric lifetime or scope.
            if ((ECode)E_NUMBER_FORMAT_EXCEPTION == ec) {
                Logger::E(TAG, "%s %d", errorMessage.string(), ec);
                return E_ILLEGAL_STATE_EXCEPTION;
            }
        // } catch(IllegalArgumentException e) {  // Malformed/invalid IP address.
            else if ((ECode)E_ILLEGAL_ARGUMENT_EXCEPTION == ec) {
                Logger::E(TAG, "%s %d", errorMessage.string(), ec);
                return E_ILLEGAL_STATE_EXCEPTION;
            }
            else
                return ec;
        // }
        }

        if ((*cooked)[2].Equals("updated")) {
            mHost->NotifyAddressUpdated(iface, address);
        } else {
            mHost->NotifyAddressRemoved(iface, address);
        }
        *result = TRUE;
        return NOERROR;
        // break;
    }
    case NetdResponseCode::InterfaceDnsServerInfo:
    {
        /*
         * Information about available DNS servers has been received.
         * Format: "NNN DnsInfo servers <interface> <lifetime> <servers>"
         */
        Int64 lifetime;  // Actually a 32-bit unsigned integer.

        if (cooked->GetLength() == 6 &&
            (*cooked)[1].Equals("DnsInfo") &&
            (*cooked)[2].Equals("servers")) {
            // try {
            ECode ec = StringUtils::Parse((*cooked)[4], &lifetime);
            // } catch (NumberFormatException e) {
            if (FAILED(ec)) {
                if ((ECode)E_NUMBER_FORMAT_EXCEPTION == ec) {
                    Logger::E(TAG, "%s", errorMessage.string());
                    return E_ILLEGAL_STATE_EXCEPTION;
                }
                return ec;
            }
            // }
            AutoPtr<ArrayOf<String> > servers;
            StringUtils::Split((*cooked)[5], ",", (ArrayOf<String>**)&servers);
            mHost->NotifyInterfaceDnsServerInfo((*cooked)[3], lifetime, servers);
        }
        *result = TRUE;
        return NOERROR;
        // break;
    }
    case NetdResponseCode::RouteChange:
    {
        /*
         * A route has been updated or removed.
         * Format: "NNN Route <updated|removed> <dst> [via <gateway] [dev <iface>]"
         */
        if (!(*cooked)[1].Equals("Route") || cooked->GetLength() < 6) {
            Logger::E(TAG, "%s", errorMessage.string());
            return E_ILLEGAL_STATE_EXCEPTION;
        }

        String via = String(NULL);
        String dev = String(NULL);
        Boolean valid = TRUE;
        for (Int32 i = 4; (i + 1) < cooked->GetLength() && valid; i += 2) {
            if ((*cooked)[i].Equals("dev")) {
                if (dev == String(NULL)) {
                    dev = (*cooked)[i+1];
                } else {
                    valid = FALSE;  // Duplicate interface.
                }
            } else if ((*cooked)[i].Equals("via")) {
                if (via == String(NULL)) {
                    via = (*cooked)[i+1];
                } else {
                    valid = FALSE;  // Duplicate gateway.
                }
            } else {
                valid = FALSE;      // Unknown syntax.
            }
        }
        if (valid) {
            // try {
            ECode ec;
            do {
                // InetAddress.parseNumericAddress(null) inexplicably returns ::1.
                AutoPtr<IInetAddressHelper> helper;
                CInetAddressHelper::AcquireSingleton((IInetAddressHelper**)&helper);
                AutoPtr<IInetAddress> gateway;
                if (via != String(NULL)) {
                    if (FAILED(ec = helper->ParseNumericAddress(via, (IInetAddress**)&gateway))) break;
                }
                AutoPtr<IIpPrefix> ipPrefix;
                ec = CIpPrefix::New((*cooked)[3], (IIpPrefix**)&ipPrefix);
                if (FAILED(ec)) break;
                AutoPtr<IRouteInfo> route;
                ec = CRouteInfo::New(ipPrefix, gateway, dev, (IRouteInfo**)&route);
                if (FAILED(ec)) break;
                ec = mHost->NotifyRouteChange((*cooked)[2], route);
                if (FAILED(ec)) break;
                *result = TRUE;
                return NOERROR;
            } while(FALSE);
            // } catch (IllegalArgumentException e) {}
            if (FAILED(ec)) {
                if ((ECode)E_ILLEGAL_ARGUMENT_EXCEPTION != ec)
                    return ec;
            }
        }
        Logger::E(TAG, "%s", errorMessage.string());
        return E_ILLEGAL_STATE_EXCEPTION;
        // break;
    }
    default: break;
    }
    *result = FALSE;
    return NOERROR;
}

//=============================================================================
// CNetworkManagementService::InnerSub_PhoneStateListener
//=============================================================================
CNetworkManagementService::InnerSub_PhoneStateListener::InnerSub_PhoneStateListener(
    /* [in] */ CNetworkManagementService* host)
    : mHost(host)
{}

ECode CNetworkManagementService::InnerSub_PhoneStateListener::OnDataConnectionRealTimeInfoChanged(
    /* [in] */ IDataConnectionRealTimeInfo* dcRtInfo)
{
    if (CNetworkManagementService::DBG) Slogger::D(CNetworkManagementService::TAG,
            "onDataConnectionRealTimeInfoChanged: %s", TO_CSTR(dcRtInfo));
    Int32 state;
    dcRtInfo->GetDcPowerState(&state);
    Int64 time;
    dcRtInfo->GetTime(&time);
    mHost->NotifyInterfaceClassActivity(IConnectivityManager::TYPE_MOBILE,
            state, time, TRUE);
    return NOERROR;
}

//=============================================================================
// CNetworkManagementService::TimerRunnable
//=============================================================================
CAR_INTERFACE_IMPL(CNetworkManagementService::TimerRunnable, Object, IRunnable)

CNetworkManagementService::TimerRunnable::TimerRunnable(
    /* [in] */ CNetworkManagementService* host,
    /* [in] */ Int32 type)
    : mHost(host)
    , mType(type)
{}

ECode CNetworkManagementService::TimerRunnable::Run()
{
    return mHost->NotifyInterfaceClassActivity(mType,
            IDataConnectionRealTimeInfo::DC_POWER_STATE_HIGH,
            SystemClock::GetElapsedRealtimeNanos(), FALSE);
}

//=============================================================================
// CNetworkManagementService::RemoveRunnable
//=============================================================================
CAR_INTERFACE_IMPL(CNetworkManagementService::RemoveRunnable, Object, IRunnable)

CNetworkManagementService::RemoveRunnable::RemoveRunnable(
    /* [in] */ CNetworkManagementService* host,
    /* [in] */ IdleTimerParams* params)
    : mHost(host)
    , mParams(params)
{}

ECode CNetworkManagementService::RemoveRunnable::Run()
{
    return mHost->NotifyInterfaceClassActivity(mParams->mType,
            IDataConnectionRealTimeInfo::DC_POWER_STATE_LOW,
            SystemClock::GetElapsedRealtimeNanos(), FALSE);
}

//=============================================================================
// CNetworkManagementService
//=============================================================================
const String CNetworkManagementService::LIMIT_GLOBAL_ALERT("globalAlert");
const String CNetworkManagementService::TAG("NetworkManagementService");
const Boolean CNetworkManagementService::DBG = FALSE;
const String CNetworkManagementService::NETD_TAG("NetdConnector");
const String CNetworkManagementService::NETD_SOCKET_NAME("netd");
const Int32 CNetworkManagementService::MAX_UID_RANGES_PER_COMMAND = 10;
const Int32 CNetworkManagementService::DAEMON_MSG_MOBILE_CONN_REAL_TIME_INFO = 1;

CAR_OBJECT_IMPL(CNetworkManagementService)

CAR_INTERFACE_IMPL(CNetworkManagementService, Object, IBinder, IINetworkManagementService)

CNetworkManagementService::CNetworkManagementService()
    : mBandwidthControlEnabled(FALSE)
    , mFirewallEnabled(FALSE)
    , mMobileActivityFromRadio(FALSE)
    , mLastPowerStateFromRadio(IDataConnectionRealTimeInfo::DC_POWER_STATE_LOW)
    , mNetworkActive(FALSE)
{
}

ECode CNetworkManagementService::constructor(
    /* [in] */ IContext* context,
    /* [in] */ const String& socket)
{
    mContext = context;

    CCountDownLatch::New(1, (ICountDownLatch**)&mConnectedSignal);
    CRemoteCallbackList::New((IRemoteCallbackList**)&mObservers);
    CNetworkStatsFactory::New((INetworkStatsFactory**)&mStatsFactory);
    CRemoteCallbackList::New((IRemoteCallbackList**)&mNetworkActivityListeners);

    CHashMap::New((IHashMap**)&mActiveQuotas);
    CHashMap::New((IHashMap**)&mActiveAlerts);
    CSparseBooleanArray::New((ISparseBooleanArray**)&mUidRejectOnQuota);
    CHashMap::New((IHashMap**)&mActiveIdleTimers);

    // make sure this is on the same looper as our NativeDaemonConnector for sync purposes
    AutoPtr<ILooper> looper;
    FgThread::Get()->GetLooper((ILooper**)&looper);
    CHandler::New(looper, (IHandler**)&mFgHandler);

    AutoPtr<ISystemProperties> sysProp;
    CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
    String value;
    sysProp->Get(String("ro.product.device"), &value);
    if (value.Equals("simulator")) {
        mConnector = NULL;
        mThread = NULL;
        mDaemonHandler = NULL;
        mPhoneStateListener = NULL;
        return NOERROR;
    }

    // Don't need this wake lock, since we now have a time stamp for when
    // the network actually went inactive.  (It might be nice to still do this,
    // but I don't want to do it through the power manager because that pollutes the
    // battery stats history with pointless noise.)
    AutoPtr<IInterface> obj;
    context->GetSystemService(IContext::POWER_SERVICE, (IInterface**)&obj);
    AutoPtr<IPowerManager> pm = IPowerManager::Probe(obj);
    AutoPtr<IPowerManagerWakeLock> wl;
    pm->NewWakeLock(IPowerManager::PARTIAL_WAKE_LOCK, NETD_TAG, (IPowerManagerWakeLock**)&wl);

    //Waiting for NativeDaemonConnector
    mConnector = new NativeDaemonConnector(
             new NetdCallbackReceiver(this), socket, 10, NETD_TAG, 160, wl, looper);

    CThread::New(mConnector, NETD_TAG, (IThread**)&mThread);

    CHandler::New(looper, (IHandler**)&mDaemonHandler);

    //Waiting for PhoneStateListener
    AutoPtr<InnerSub_PhoneStateListener> psl = new InnerSub_PhoneStateListener(this);
    psl->constructor(ISubscriptionManager::DEFAULT_SUB_ID, looper);
    mPhoneStateListener = psl;

    obj = NULL;
    context->GetSystemService(IContext::TELEPHONY_SERVICE, (IInterface**)&obj);
    AutoPtr<ITelephonyManager> tm = ITelephonyManager::Probe(obj);
    if (tm != NULL) {
        tm->Listen(mPhoneStateListener,
                IPhoneStateListener::LISTEN_DATA_CONNECTION_REAL_TIME_INFO);
    }

    // Add ourself to the Watchdog monitors.
    Watchdog::GetInstance()->AddMonitor(this);
    return NOERROR;
}

ECode CNetworkManagementService::Create(
    /* [in] */ IContext* context,
    /* [in] */ const String& socket,
    /* [out] */ IINetworkManagementService** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<CNetworkManagementService> service;
    CNetworkManagementService::NewByFriend(context, socket, (CNetworkManagementService**)&service);
    AutoPtr<ICountDownLatch> connectedSignal = service->mConnectedSignal;
    if (DBG) Slogger::D(TAG, "Creating NetworkManagementService");
    service->mThread->Start();
    if (DBG) Slogger::D(TAG, "Awaiting socket connection");
    connectedSignal->Await();
    if (DBG) Slogger::D(TAG, "Connected");
    *result = IINetworkManagementService::Probe(service);
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CNetworkManagementService::Create(
    /* [in] */ IContext* context,
    /* [out] */ IINetworkManagementService** result)
{
    return Create(context, NETD_SOCKET_NAME, result);
}

void CNetworkManagementService::SystemReady()
{
    PrepareNativeDaemon();
    if (DBG) Slogger::D(TAG, "Prepared");
}

ECode CNetworkManagementService::RegisterObserver(
    /* [in] */ IINetworkManagementEventObserver* observer)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
        Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG));
    Boolean result;
    return mObservers->Register(observer, &result);
}

ECode CNetworkManagementService::UnregisterObserver(
    /* [in] */ IINetworkManagementEventObserver* observer)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
        Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG));
    Boolean result;
    return mObservers->Unregister(observer, &result);
}

ECode CNetworkManagementService::NotifyInterfaceStatusChanged(
    /* [in] */ const String& iface,
    /* [in] */ Boolean up)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    // try {
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->InterfaceStatusChanged(iface, up);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyInterfaceLinkStateChanged(
    /* [in] */ const String& iface,
    /* [in] */ Boolean up)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    // try {
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->InterfaceLinkStateChanged(iface, up);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyInterfaceAdded(
    /* [in] */ const String& iface)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->InterfaceAdded(iface);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyInterfaceRemoved(
    /* [in] */ const String& iface)
{
    // netd already clears out quota and alerts for removed ifaces; update
    // our sanity-checking state.
    mActiveAlerts->Remove(StringUtils::ParseCharSequence(iface));
    mActiveQuotas->Remove(StringUtils::ParseCharSequence(iface));

    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->InterfaceRemoved(iface);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyLimitReached(
    /* [in] */ const String& limitName,
    /* [in] */ const String& iface)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->LimitReached(limitName, iface);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyInterfaceClassActivity(
    /* [in] */ Int32 type,
    /* [in] */ Int32 powerState,
    /* [in] */ Int64 tsNanos,
    /* [in] */ Boolean fromRadio)
{
    Boolean isMobile;
    AutoPtr<IConnectivityManagerHelper> helper;
    CConnectivityManagerHelper::AcquireSingleton((IConnectivityManagerHelper**)&helper);
    helper->IsNetworkTypeMobile(type, &isMobile);
    if (isMobile) {
        if (!fromRadio) {
            if (mMobileActivityFromRadio) {
                // If this call is not coming from a report from the radio itself, but we
                // have previously received reports from the radio, then we will take the
                // power state to just be whatever the radio last reported.
                powerState = mLastPowerStateFromRadio;
            }
        }
        else {
            mMobileActivityFromRadio = true;
        }
        if (mLastPowerStateFromRadio != powerState) {
            mLastPowerStateFromRadio = powerState;
            // try {
            AutoPtr<IIBatteryStats> bstats;
            GetBatteryStats((IIBatteryStats**)&bstats);
            ECode ec = bstats->NoteMobileRadioPowerState(powerState, tsNanos);
            // } catch (RemoteException e) {
            if (FAILED(ec)) {
                if ((ECode)E_REMOTE_EXCEPTION != ec)
                    return ec;
            }
            // }
        }
    }

    Boolean isActive = powerState == IDataConnectionRealTimeInfo::DC_POWER_STATE_MEDIUM
            || powerState == IDataConnectionRealTimeInfo::DC_POWER_STATE_HIGH;

    ECode ec = NOERROR;
    if (!isMobile || fromRadio || !mMobileActivityFromRadio) {
        // Report the change in data activity.  We don't do this if this is a change
        // on the mobile network, that is not coming from the radio itself, and we
        // have previously seen change reports from the radio.  In that case only
        // the radio is the authority for the current state.
        Int32 length;
        mObservers->BeginBroadcast(&length);
        // try {
            for (Int32 i = 0; i < length; i++) {
                // try {
                AutoPtr<IInterface> obj;
                mObservers->GetBroadcastItem(i, (IInterface**)&obj);
                AutoPtr<IINetworkManagementEventObserver> observer = IINetworkManagementEventObserver::Probe(obj);
                ec = observer->InterfaceClassDataActivityChanged(StringUtils::ToString(type), isActive, tsNanos);
                // } catch (RemoteException e) {
                // } catch (RuntimeException e) {
                // }
                if (FAILED(ec)) {
                    if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                        continue;
                    }
                    break;
                }
            }
        // } finally {
        mObservers->FinishBroadcast();
        // }
        if(FAILED(ec)) return ec;
    }

    Boolean report = FALSE;
    {
        AutoLock syncLock(mIdleTimerLock);
        Boolean empty;
        if (mActiveIdleTimers->IsEmpty(&empty), empty) {
            // If there are no idle timers, we are not monitoring activity, so we
            // are always considered active.
            isActive = TRUE;
        }
        if (mNetworkActive != isActive) {
            mNetworkActive = isActive;
            report = isActive;
        }
    }
    if (report) {
        ReportNetworkActive();
    }
    return NOERROR;
}

ECode CNetworkManagementService::NotifyInterfaceMessage(
    /* [in] */ const String& message)
{
    Int32 length = 0;
    mObservers->BeginBroadcast(&length);
    for (Int32 i = 0; i < length; i++) {
        AutoPtr<IInterface> p;
        mObservers->GetBroadcastItem(i, (IInterface**)&p);
        AutoPtr<IINetworkManagementEventObserver> _p = IINetworkManagementEventObserver::Probe(p);
        _p->InterfaceMessageRecevied(message);
    }
    mObservers->FinishBroadcast();
    return NOERROR;
}

ECode CNetworkManagementService::NotifyAddressUpdated(
    /* [in] */ const String& iface,
    /* [in] */ ILinkAddress* address)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    // try {
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->AddressUpdated(iface, address);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyAddressRemoved(
        /* [in] */ const String& iface,
        /* [in] */ ILinkAddress* address)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    // try {
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->AddressRemoved(iface, address);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyInterfaceDnsServerInfo(
        /* [in] */ const String& iface,
        /* [in] */ Int64 lifetime,
        /* [in] */ ArrayOf<String>* addresses)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    // try {
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        ec = observer->InterfaceDnsServerInfo(iface, lifetime,
                        addresses);
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::NotifyRouteChange(
        /* [in] */ const String& action,
        /* [in] */ IRouteInfo* route)
{
    Int32 length;
    mObservers->BeginBroadcast(&length);
    ECode ec = NOERROR;
    // try {
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        mObservers->GetBroadcastItem(i, (IInterface**)&obj);
        IINetworkManagementEventObserver* observer = IINetworkManagementEventObserver::Probe(obj);
        if (action.Equals("updated")) {
            ec = observer->RouteUpdated(route);
        }
        else {
            ec = observer->RouteRemoved(route);
        }
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
        // }
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec) {
                continue;
            }
            break;
        }
    }
    // } finally {
    mObservers->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::PrepareNativeDaemon()
{
    mBandwidthControlEnabled = FALSE;

    // only enable bandwidth control when support exists
    Boolean hasKernelSupport;
    AutoPtr<IFile> files;
    CFile::New(String("/proc/net/xt_qtaguid/ctrl"), (IFile**)&files);
    files->Exists(&hasKernelSupport);
    if (hasKernelSupport) {
        Slogger::D(TAG, "enabling bandwidth control");
        // try {
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
        args->Set(0, CoreUtils::Convert(String("enable")));
        AutoPtr<NativeDaemonEvent> event;
        //Waiting for NativeDaemonConnector
        mConnector->Execute(String("bandwidth"), args, (NativeDaemonEvent**)&event);
        mBandwidthControlEnabled = TRUE;
        // } catch (NativeDaemonConnectorException e) {
        //     Log.wtf(TAG, "problem enabling bandwidth controls", e);
        // }
    }
    else {
        Slogger::D(TAG, "not enabling bandwidth control");
    }

    AutoPtr<ISystemProperties> sysProp;
    CSystemProperties::AcquireSingleton((ISystemProperties**)&sysProp);
    sysProp->Set(INetworkManagementSocketTagger::PROP_QTAGUID_ENABLED,
            mBandwidthControlEnabled ? String("1") : String("0"));

    if (mBandwidthControlEnabled) {
        // try {
        AutoPtr<IIBatteryStats> stats;
        GetBatteryStats((IIBatteryStats**)&stats);
        ECode ec = stats->NoteNetworkStatsEnabled();
        // } catch (RemoteException e) {
        if (FAILED(ec)) {
            if ((ECode)E_REMOTE_EXCEPTION != ec)
                return ec;
        }
        // }
    }

    // push any existing quota or UID rules
    {
        AutoLock syncLock(mQuotaLock);
        Int32 size;
        mActiveQuotas->GetSize(&size);
        if (size > 0) {
            Slogger::D(TAG, "pushing %d active quota rules", size);
            AutoPtr<IHashMap> activeQuotas = mActiveQuotas;
            mActiveQuotas = NULL;
            CHashMap::New((IHashMap**)&mActiveQuotas);
            AutoPtr<ISet> entries;
            activeQuotas->GetEntrySet((ISet**)&entries);
            AutoPtr<IIterator> it;
            entries->GetIterator((IIterator**)&it);
            Boolean hasNext;
            while (it->HasNext(&hasNext), hasNext) {
                AutoPtr<IInterface> obj;
                it->GetNext((IInterface**)&obj);
                AutoPtr<IMapEntry> entry = IMapEntry::Probe(obj);
                AutoPtr<IInterface> key, value;
                entry->GetKey((IInterface**)&key);
                entry->GetValue((IInterface**)&value);
                Int64 quota;
                IInteger64::Probe(value)->GetValue(&quota);
                SetInterfaceQuota(Object::ToString(key), quota);
            }
        }

        mActiveAlerts->GetSize(&size);
        if (size > 0) {
            Slogger::D(TAG, "pushing %d active quota rules", size);
            AutoPtr<IHashMap> activeAlerts = mActiveAlerts;
            mActiveAlerts = NULL;
            CHashMap::New((IHashMap**)&mActiveAlerts);
            AutoPtr<ISet> entries;
            activeAlerts->GetEntrySet((ISet**)&entries);
            AutoPtr<IIterator> it;
            entries->GetIterator((IIterator**)&it);
            Boolean hasNext;
            while (it->HasNext(&hasNext), hasNext) {
                AutoPtr<IInterface> obj;
                it->GetNext((IInterface**)&obj);
                AutoPtr<IMapEntry> entry = IMapEntry::Probe(obj);
                AutoPtr<IInterface> key, value;
                entry->GetKey((IInterface**)&key);
                entry->GetValue((IInterface**)&value);
                Int64 alert;
                IInteger64::Probe(value)->GetValue(&alert);
                SetInterfaceAlert(Object::ToString(key), alert);
            }
        }

        mUidRejectOnQuota->GetSize(&size);
        if (size > 0) {
            Slogger::D(TAG, "pushing %d active uid rules", size);
            AutoPtr<ISparseBooleanArray> uidRejectOnQuota = mUidRejectOnQuota;
            mUidRejectOnQuota = NULL;
            CSparseBooleanArray::New((ISparseBooleanArray**)&mUidRejectOnQuota);
            uidRejectOnQuota->GetSize(&size);
            for (Int32 i = 0; i < size; i++) {
                Int32 key;
                Boolean value;
                uidRejectOnQuota->KeyAt(i, &key);
                uidRejectOnQuota->ValueAt(i, &value);
                SetUidNetworkRules(key, value);
            }
        }
    }

    // TODO: Push any existing firewall state
    SetFirewallEnabled(mFirewallEnabled || LockdownVpnTracker::IsEnabled());
    return NOERROR;
}

ECode CNetworkManagementService::ListInterfaces(
    /* [out, callee] */ ArrayOf<String>** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
     // try {
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("list")));
    AutoPtr< ArrayOf<NativeDaemonEvent*> > events;
    mConnector->ExecuteForList(String("interface"), args, (ArrayOf<NativeDaemonEvent*>**)&events);
    AutoPtr< ArrayOf<String> > resArray = NativeDaemonEvent::FilterMessageList(
            *events, NetdResponseCode::InterfaceListResult);
    *result = resArray;
    REFCOUNT_ADD(*result);
     // } catch (NativeDaemonConnectorException e) {
     //     throw e.rethrowAsParcelableException();
     // }
     return NOERROR;
}

ECode CNetworkManagementService::AddUpstreamV6Interface(
    /* [in] */ const String& iface)
{
    mContext->EnforceCallingOrSelfPermission(
            Manifest::permission::ACCESS_NETWORK_STATE, String("NetworkManagementService"));

    Slogger::D(TAG, "addUpstreamInterface(%s)", iface.string());

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("interface")));
    args->Set(1, CoreUtils::Convert(String("add_upstream")));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
            String("tether"), args);
    cmd->AppendArg(CoreUtils::Convert(iface));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::RemoveUpstreamV6Interface(
    /* [in] */ const String& iface)
{
    mContext->EnforceCallingOrSelfPermission(
            Manifest::permission::ACCESS_NETWORK_STATE, String("NetworkManagementService"));

    Slogger::D(TAG, "removeUpstreamInterface(%s)", iface.string());

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("interface")));
    args->Set(1, CoreUtils::Convert(String("remove_upstream")));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
            String("tether"), args);
    cmd->AppendArg(CoreUtils::Convert(iface));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::GetInterfaceConfig(
    /* [in] */ const String& iface,
    /* [out] */ IInterfaceConfiguration** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr<NativeDaemonEvent> event;
     // try {
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("getcfg")));
    args->Set(1, CoreUtils::Convert(iface));
    //Waiting for NativeDaemonConnector
    FAIL_RETURN(mConnector->Execute(String("interface"), args, (NativeDaemonEvent**)&event));
     // } catch (NativeDaemonConnectorException e) {
     //     throw e.rethrowAsParcelableException();
     // }

    FAIL_RETURN(event->CheckCode(NetdResponseCode::InterfaceGetCfgResult));
     // Rsp: 213 xx:xx:xx:xx:xx:xx yyy.yyy.yyy.yyy zzz flag1 flag2 flag3

    AutoPtr<IStringTokenizer> st;
    CStringTokenizer::New(event->GetMessage(), (IStringTokenizer**)&st);

    AutoPtr<IInterfaceConfiguration> cfg;
    // try {
    CInterfaceConfiguration::New((IInterfaceConfiguration**)&cfg);
    String delim(" "), nextToken;
    st->GetNextToken(delim, &nextToken);
    cfg->SetHardwareAddress(nextToken);
    AutoPtr<IInetAddress> addr;
    Int32 prefixLength = 0;
    // try {
    st->GetNextToken(&nextToken);
    NetworkUtils::NumericToInetAddress(nextToken, (IInetAddress**)&addr);
    // } catch (IllegalArgumentException iae) {
    //     Slog.e(TAG, "Failed to parse ipaddr", iae);
    // }

    // try {
    st->GetNextToken(&nextToken);
    prefixLength = atoi(nextToken);
    // } catch (NumberFormatException nfe) {
    //     Slog.e(TAG, "Failed to parse prefixLength", nfe);
    // }

    AutoPtr<ILinkAddress> linkAddr;
    CLinkAddress::New(addr, prefixLength, (ILinkAddress**)&linkAddr);
    cfg->SetLinkAddress(linkAddr);
    Boolean hasNext;
    while (st->HasMoreTokens(&hasNext), hasNext) {
        st->GetNextToken(&nextToken);
        cfg->SetFlag(nextToken);
    }
    // } catch (NoSuchElementException nsee) {
    //     throw new IllegalStateException("Invalid response from daemon: " + event);
    // }
    *result = cfg;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CNetworkManagementService::SetInterfaceConfig(
    /* [in] */ const String& iface,
    /* [in] */ IInterfaceConfiguration* cfg)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    AutoPtr<ILinkAddress> linkAddr;
    cfg->GetLinkAddress((ILinkAddress**)&linkAddr);
    AutoPtr<IInetAddress> inetAddr;
    if (linkAddr == NULL || (linkAddr->GetAddress((IInetAddress**)&inetAddr), inetAddr == NULL)) {
        Logger::E(TAG, "NULL LinkAddress given");
        return E_ILLEGAL_STATE_EXCEPTION;
    }

    String hostAddr;
    inetAddr->GetHostAddress(&hostAddr);
    Int32 prefixLength;
    linkAddr->GetPrefixLength(&prefixLength);

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(4);
    args->Set(0, CoreUtils::Convert(String("setcfg")));
    args->Set(1, CoreUtils::Convert(iface));
    args->Set(2, CoreUtils::Convert(hostAddr));
    args->Set(3, CoreUtils::Convert(prefixLength));
    //Waiting for NativeDaemonConnector
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
            String("interface"), args);

    AutoPtr<IIterable> flags;
    cfg->GetFlags((IIterable**)&flags);
    AutoPtr<IIterator> iter;
    flags->GetIterator((IIterator**)&iter);
    Boolean hasNext;
    while(iter->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> obj;
        iter->GetNext((IInterface**)&obj);
        AutoPtr<ICharSequence> flag = ICharSequence::Probe(obj);
        cmd->AppendArg(flag);
    }

    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::SetInterfaceDown(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    AutoPtr<IInterfaceConfiguration> ifcg;
    FAIL_RETURN(GetInterfaceConfig(iface, (IInterfaceConfiguration**)&ifcg));
    ifcg->SetInterfaceDown();
    SetInterfaceConfig(iface, ifcg);
    return NOERROR;
}

ECode CNetworkManagementService::SetInterfaceUp(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    AutoPtr<IInterfaceConfiguration> ifcg;
    FAIL_RETURN(GetInterfaceConfig(iface, (IInterfaceConfiguration**)&ifcg));
    ifcg->SetInterfaceUp();
    SetInterfaceConfig(iface, ifcg);
    return NOERROR;
}

ECode CNetworkManagementService::SetInterfaceIpv6PrivacyExtensions(
    /* [in] */ const String& iface,
    /* [in] */ Boolean enable)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("ipv6privacyextensions")));
    args->Set(1, CoreUtils::Convert(iface));
    args->Set(2, CoreUtils::Convert(enable ? String("enable") : String("disable")));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("interface"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::ClearInterfaceAddresses(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("clearaddrs")));
    args->Set(1, CoreUtils::Convert(iface));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("interface"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::EnableIpv6(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("ipv6")));
    args->Set(1, CoreUtils::Convert(iface));
    args->Set(2, CoreUtils::Convert(String("enable")));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("interface"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::DisableIpv6(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("ipv6")));
    args->Set(1, CoreUtils::Convert(iface));
    args->Set(2, CoreUtils::Convert(String("disable")));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("interface"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::AddRoute(
    /* [in] */ Int32 netId,
    /* [in] */ IRouteInfo* route)
{
    return ModifyRoute(String("add"), StringUtils::ToString(netId), route);
}

ECode CNetworkManagementService::RemoveRoute(
    /* [in] */ Int32 netId,
    /* [in] */ IRouteInfo* route)
{
    return ModifyRoute(String("remove"), StringUtils::ToString(netId), route);
}

ECode CNetworkManagementService::ModifyRoute(
    /* [in] */ const String& action,
    /* [in] */ const String& netId,
    /* [in] */ IRouteInfo* route)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);

    args->Set(0, CoreUtils::Convert(String("route")));
    args->Set(1, CoreUtils::Convert(action));
    args->Set(2, CoreUtils::Convert(netId));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(String("network"), args);

    // // create triplet: interface dest-ip-addr/prefixlength gateway-ip-addr
    String routeInterface;
    route->GetInterface(&routeInterface);
    cmd->AppendArg(CoreUtils::Convert(routeInterface));
    AutoPtr<IIpPrefix> ipPrefix;
    route->GetDestination((IIpPrefix**)&ipPrefix);
    cmd->AppendArg(CoreUtils::Convert(Object::ToString(ipPrefix)));

    Int32 type;
    route->GetType(&type);
    if (DBG) {
        String ss;
        IObject::Probe(route)->ToString(&ss);
        Logger::E("NetworkManagementService", "line:%d, func:%s, type:%d route:%s\n", __LINE__, "ModifyRoute", type, ss.string());
    }
    switch (type) {
        case IRouteInfo::RTN_UNICAST:
        {
            Boolean hasGateway;
            route->HasGateway(&hasGateway);
            if (hasGateway) {
                AutoPtr<IInetAddress> netAddress;
                route->GetGateway((IInetAddress**)&netAddress);
                String hostAddress;
                netAddress->GetHostAddress(&hostAddress);
                cmd->AppendArg(CoreUtils::Convert(hostAddress));
            }
            break;
        }
        case IRouteInfo::RTN_UNREACHABLE:
        {
            cmd->AppendArg(CoreUtils::Convert(String("unreachable")));
            break;
        }
        case IRouteInfo::RTN_THROW:
        {
            cmd->AppendArg(CoreUtils::Convert(String("throw")));
            break;
        }
    }

    // try {
    AutoPtr<NativeDaemonEvent> event;
    ECode ec = mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    // // } catch (NativeDaemonConnectorException e) {
    if (FAILED(ec)) {
         if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
             return E_ILLEGAL_STATE_EXCEPTION;
         else
             return ec;
    // }
    }
    return NOERROR;
}

// @Override
ECode CNetworkManagementService::SetMtu(
    /* [in] */ const String& iface,
    /* [in] */ Int32 mtu)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    AutoPtr<NativeDaemonEvent> event;
    // try {
    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("setmtu")));
    args->Set(1, CoreUtils::Convert(iface));
    args->Set(2, CoreUtils::Convert(mtu));

    ECode ec = mConnector->Execute(String("interface"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    if (FAILED(ec)) {
        if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
            return E_ILLEGAL_STATE_EXCEPTION;
        return ec;
    }
    // // }
    return NOERROR;
}

ECode CNetworkManagementService::ReadRouteList(
    /* [in] */ const String& filename,
    /* [out, callee] */ List<String>** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    AutoPtr<IFileInputStream> fstream;
    CFileInputStream::New(filename, (IFileInputStream**)&fstream);

    AutoPtr< List<String> > strList = new List<String>();
    AutoPtr<IDataInputStream> in;
    CDataInputStream::New(IInputStream::Probe(fstream), (IDataInputStream**)&in);
    AutoPtr<IBufferedReader> br;
    AutoPtr<IInputStreamReader> isReader;
    CInputStreamReader::New(IInputStream::Probe(in), (IInputStreamReader**)&isReader);
    CBufferedReader::New(IReader::Probe(isReader), (IBufferedReader**)&br);
    String s;

    // throw away the title line
    br->ReadLine(&s);
    while (!s.IsNullOrEmpty()) {
        strList->PushBack(s);
        br->ReadLine(&s);
    }
    *result = strList;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CNetworkManagementService::GetRoutes(
    /* [in] */ const String& interfaceName,
    /* [out, callee] */ ArrayOf<IRouteInfo*>** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    List<AutoPtr<IRouteInfo> > routes;

    // v4 routes listed as:
    // iface dest-addr gateway-addr flags refcnt use metric netmask mtu window IRTT
    AutoPtr< List<String> > strList;
    ReadRouteList(String("/proc/net/route"), (List<String>**)&strList);
    List<String>::Iterator iter;
    for (iter = strList->Begin(); iter != strList->End(); ++iter) {
        AutoPtr<ArrayOf<String> > fields;
        StringUtils::Split(*iter, String("\t"), (ArrayOf<String>**)&fields);

        if (fields->GetLength() > 7) {
            String iface = (*fields)[0];

            if (interfaceName.Equals(iface)) {
                String dest = (*fields)[1];
                String gate = (*fields)[2];
                String flags = (*fields)[3]; // future use?
                String mask = (*fields)[7];
                // try {
                // address stored as a hex string, ex: 0014A8C0
                AutoPtr<IInetAddress> destAddr;
                NetworkUtils::IntToInetAddress((Int32)StringUtils::ParseInt64(dest, 16), (IInetAddress**)&destAddr);
                Int32 prefixLength;
                NetworkUtils::NetmaskIntToPrefixLength((Int32)StringUtils::ParseInt64(mask, 16), &prefixLength);
                AutoPtr<ILinkAddress> linkAddress;
                CLinkAddress::New(destAddr, prefixLength, (ILinkAddress**)&linkAddress);

                // address stored as a hex string, ex 0014A8C0
                AutoPtr<IInetAddress> gatewayAddr;
                NetworkUtils::IntToInetAddress((Int32)StringUtils::ParseInt64(gate, 16), (IInetAddress**)&gatewayAddr);

                AutoPtr<IRouteInfo> route;
                CRouteInfo::New(linkAddress, gatewayAddr, (IRouteInfo**)&route);
                routes.PushBack(route);
                // } catch (Exception e) {
                //     Logger::E(TAG, "Error parsing route " + s + " : " + e);
                //     continue;
                // }
            }
        }
    }

    strList = NULL;

    // v6 routes listed as:
    // dest-addr prefixlength ?? ?? gateway-addr ?? ?? ?? ?? iface
    ReadRouteList(String("/proc/net/ipv6_route"), (List<String>**)&strList);
    for (iter = strList->Begin(); iter != strList->End(); ++iter) {
        AutoPtr<ArrayOf<String> > fields;
        StringUtils::Split(*iter, String("\\s+"), (ArrayOf<String>**)&fields);

        if (fields->GetLength() > 9) {
            String iface = (*fields)[9].Trim();
            if (interfaceName.Equals(iface)) {
                String dest = (*fields)[0];
                String prefix = (*fields)[1];
                String gate = (*fields)[4];

//                try {
                // prefix length stored as a hex string, ex 40
                Int32 prefixLength = StringUtils::ParseInt32(prefix, 16);

                // address stored as a 32 char hex string
                // ex fe800000000000000000000000000000

                AutoPtr<IInetAddress> destAddr;
                NetworkUtils::HexToInet6Address(dest, (IInetAddress**)&destAddr);

                AutoPtr<ILinkAddress> linkAddress;
                CLinkAddress::New(destAddr, prefixLength, (ILinkAddress**)&linkAddress);

                AutoPtr<IInetAddress> gateAddr;
                NetworkUtils::HexToInet6Address(gate, (IInetAddress**)&gateAddr);

                AutoPtr<IRouteInfo> route;
                CRouteInfo::New(linkAddress, gateAddr, (IRouteInfo**)&route);
                routes.PushBack(route);
//                } catch (Exception e) {
//                    Logger::E(TAG, "Error parsing route " + s + " : " + e);
//                    continue;
//                }
            }
        }
    }

    List<AutoPtr<IRouteInfo> >::Iterator iter2;
    AutoPtr< ArrayOf<IRouteInfo*> > resArray = ArrayOf<IRouteInfo*>::Alloc(routes.GetSize());
    iter2 = routes.Begin();
    for (Int32 i = 0; iter2 != routes.End(); ++iter2) {
        resArray->Set(i++, *iter2);
    }
    *result = resArray;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CNetworkManagementService::Shutdown()
{
    // TODO: remove from aidl if nobody calls externally
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::SHUTDOWN/*SHUTDOWN*/, TAG));
     Slogger::D(TAG, "Shutting down");
     return NOERROR;
}

ECode CNetworkManagementService::GetIpForwardingEnabled(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("status")));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("ipfwd"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }

     // 211 Forwarding enabled
     FAIL_RETURN(event->CheckCode(NetdResponseCode::IpFwdStatusResult));
     *result = event->GetMessage().EndWith("enabled");
     return NOERROR;
}

ECode CNetworkManagementService::SetIpForwardingEnabled(
    /* [in] */ Boolean enable)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(enable ? String("enable") : String("disable")));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("ipfwd"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::StartTethering(
    /* [in] */ ArrayOf<String>* dhcpRange)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
     // cmd is "tether start first_start first_stop second_start second_stop ..."
     // an odd number of addrs will fail

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("start")));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
        String("tether"), args);
    for (Int32 i = 0; i < dhcpRange->GetLength(); i++) {
         cmd->AppendArg(CoreUtils::Convert((*dhcpRange)[i]));
    }

    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::StopTethering()
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("stop")));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("tether"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::IsTetheringStarted(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("status")));

    AutoPtr<NativeDaemonEvent> event;
    // try {
    mConnector->Execute(String("tether"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }

    // 210 Tethering services started
    FAIL_RETURN(event->CheckCode(NetdResponseCode::TetherStatusResult));
    *result = event->GetMessage().EndWith("started");
    return NOERROR;
}

ECode CNetworkManagementService::TetherInterface(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("interface")));
    args->Set(1, CoreUtils::Convert(String("add")));
    args->Set(2, CoreUtils::Convert(iface));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    ECode ec = mConnector->Execute(String("tether"), args, (NativeDaemonEvent**)&event);
    // // } catch (NativeDaemonConnectorException e) {
    if (FAILED(ec)) {
         if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
             return E_ILLEGAL_STATE_EXCEPTION;
         return ec;
    }
    // // }
    AutoPtr<IList> routes;
    CArrayList::New((IList**)&routes);
        // The RouteInfo constructor truncates the LinkAddress to a network prefix, thus making it
        // suitable to use as a route destination.
    AutoPtr<IInterfaceConfiguration> interfaceConf;
    GetInterfaceConfig(iface, (IInterfaceConfiguration**)&interfaceConf);
    AutoPtr<ILinkAddress> address;
    interfaceConf->GetLinkAddress((ILinkAddress**)&address);
    AutoPtr<IRouteInfo> routeInfo;
    CRouteInfo::New(address, NULL, iface, (IRouteInfo**)&routeInfo);
    routes->Add(routeInfo);
    AddInterfaceToLocalNetwork(iface, routes);
    return NOERROR;
}

ECode CNetworkManagementService::UntetherInterface(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
        Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("interface")));
    args->Set(1, CoreUtils::Convert(String("remove")));
    args->Set(2, CoreUtils::Convert(iface));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    ECode ec = mConnector->Execute(String("tether"), args, (NativeDaemonEvent**)&event);
    // // } catch (NativeDaemonConnectorException e) {
    if (FAILED(ec)) {
         if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
             return E_ILLEGAL_STATE_EXCEPTION;
         return ec;
    }
    // // }
    RemoveInterfaceFromLocalNetwork(iface);
    return NOERROR;
}

ECode CNetworkManagementService::ListTetheredInterfaces(
    /* [out, callee] */ ArrayOf<String>** result)
{
    VALIDATE_NOT_NULL(result);
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("interface")));
    args->Set(1, CoreUtils::Convert(String("list")));
    // try {
    AutoPtr<ArrayOf<NativeDaemonEvent*> > events;
    mConnector->ExecuteForList(String("tether"), args, (ArrayOf<NativeDaemonEvent*>**)&events);
    AutoPtr< ArrayOf<String> > resArray = NativeDaemonEvent::FilterMessageList(*events, NetdResponseCode::TetherInterfaceListResult);
    *result = resArray;
    REFCOUNT_ADD(*result);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::SetDnsForwarders(
    /* [in] */ INetwork* network,
    /* [in] */ ArrayOf<String>* dns)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    Int32 netId = IConnectivityManager::NETID_UNSET;
    if (network != NULL) {
        network->GetNetId(&netId);
    }
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("dns")));
    args->Set(1, CoreUtils::Convert(String("set")));
    args->Set(2, CoreUtils::Convert(netId));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
             String("tether"), args);
    for (Int32 i = 0; i < dns->GetLength(); i++) {
        AutoPtr<IInetAddress> inetAddr;
        NetworkUtils::NumericToInetAddress((*dns)[i], (IInetAddress**)&inetAddr);
        String str;
        inetAddr->GetHostAddress(&str);
        cmd->AppendArg(CoreUtils::Convert(str));
    }
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::GetDnsForwarders(
    /* [out, callee] */ ArrayOf<String>** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("dns")));
    args->Set(1, CoreUtils::Convert(String("list")));
    // try {
    AutoPtr<ArrayOf<NativeDaemonEvent*> > events;
    mConnector->ExecuteForList(String("tether"), args, (ArrayOf<NativeDaemonEvent*>**)&events);
    AutoPtr< ArrayOf<String> > resArray = NativeDaemonEvent::FilterMessageList(*events, NetdResponseCode::TetherDnsFwdTgtListResult);
    *result = resArray;
    REFCOUNT_ADD(*result);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::ModifyNat(
    /* [in] */ const String& action,
    /* [in] */ const String& internalInterface,
    /* [in] */ const String& externalInterface)
{
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(action));
    args->Set(1, CoreUtils::Convert(internalInterface));
    args->Set(2, CoreUtils::Convert(externalInterface));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
         String("nat"), args);

    AutoPtr<INetworkInterface> internalNetworkInterface;
    AutoPtr<INetworkInterfaceHelper> helper;
    CNetworkInterfaceHelper::AcquireSingleton((INetworkInterfaceHelper**)&helper);
    helper->GetByName(internalInterface, (INetworkInterface**)&internalNetworkInterface);

    if (internalNetworkInterface == NULL) {
       AutoPtr<ICharSequence> cseq;
       CString::New(String("0"), (ICharSequence**)&cseq);
       cmd->AppendArg(cseq);
    } else {
       // Don't touch link-local routes, as link-local addresses aren't routable,
       // kernel creates link-local routes on all interfaces automatically
       AutoPtr<IList> list;
       internalNetworkInterface->GetInterfaceAddresses((IList**)&list);
       AutoPtr<IList> interfaceAddresses;
       ExcludeLinkLocal(list, (IList**)&interfaceAddresses);
       Int32 size;
       interfaceAddresses->GetSize(&size);
       AutoPtr<IInteger32> i32;
       CInteger32::New(size, (IInteger32**)&i32);
       cmd->AppendArg(i32);
       AutoPtr<IIterator> emu;
       FAIL_RETURN(interfaceAddresses->GetIterator((IIterator**)&emu));
       Boolean hasNext;
       while (emu->HasNext(&hasNext), hasNext ) {
           AutoPtr<IInterface> obj;
           emu->GetNext((IInterface**)&obj);
           AutoPtr<IInterfaceAddress> ia = IInterfaceAddress::Probe(obj);
           AutoPtr<IInetAddress> addrIn;
           AutoPtr<IInetAddress> addrOut;
           Int16 prefixLen;
           ia->GetAddress((IInetAddress**)&addrIn);
           ia->GetNetworkPrefixLength(&prefixLen);
           NetworkUtils::GetNetworkPart(addrIn, (Int32)prefixLen, (IInetAddress**)&addrOut);
           String str;
           addrOut->GetHostAddress(&str);
           AutoPtr<ICharSequence> c0;
           CString::New(str, (ICharSequence**)&c0);
           AutoPtr<ICharSequence> c1;
           CString::New(String("/"), (ICharSequence**)&c1);
           AutoPtr<IInteger16> i2;
           CInteger16::New(prefixLen, (IInteger16**)&i2);
           cmd->AppendArg(c0);
           cmd->AppendArg(c1);
           cmd->AppendArg(i2);
       }
    }
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::EnableNat(
    /* [in] */ const String& internalInterface,
    /* [in] */ const String& externalInterface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    return ModifyNat(String("enable"), internalInterface, externalInterface);
}

ECode CNetworkManagementService::DisableNat(
    /* [in] */ const String& internalInterface,
    /* [in] */ const String& externalInterface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    return ModifyNat(String("disable"), internalInterface, externalInterface);
}

ECode CNetworkManagementService::ListTtys(
    /* [out, callee] */ ArrayOf<String>** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
         String("list_ttys"), NULL);
    AutoPtr<ArrayOf<NativeDaemonEvent*> > events;
    mConnector->ExecuteForList(cmd, (ArrayOf<NativeDaemonEvent*>**)&events);
    AutoPtr< ArrayOf<String> > resArray = NativeDaemonEvent::FilterMessageList(*events, NetdResponseCode::TtyListResult);
    *result = resArray;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CNetworkManagementService::AttachPppd(
    /* [in] */ const String& tty,
    /* [in] */ const String& localAddr,
    /* [in] */ const String& remoteAddr,
    /* [in] */ const String& dns1Addr,
    /* [in] */ const String& dns2Addr)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr<IInetAddress> inetAddr;
    String str1, str2, str3, str4;
    NetworkUtils::NumericToInetAddress(localAddr, (IInetAddress**)&inetAddr);
    inetAddr->GetHostAddress(&str1);
    inetAddr = NULL;
    NetworkUtils::NumericToInetAddress(remoteAddr, (IInetAddress**)&inetAddr);
    inetAddr->GetHostAddress(&str2);
    inetAddr = NULL;
    NetworkUtils::NumericToInetAddress(dns1Addr, (IInetAddress**)&inetAddr);
    inetAddr->GetHostAddress(&str3);
    inetAddr = NULL;
    NetworkUtils::NumericToInetAddress(dns2Addr, (IInetAddress**)&inetAddr);
    inetAddr->GetHostAddress(&str4);

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(6);
    args->Set(0, CoreUtils::Convert(String("attach")));
    args->Set(1, CoreUtils::Convert(tty));
    args->Set(2, CoreUtils::Convert(str1));
    args->Set(3, CoreUtils::Convert(str2));
    args->Set(4, CoreUtils::Convert(str3));
    args->Set(5, CoreUtils::Convert(str4));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("pppd"), args, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::DetachPppd(
    /* [in] */ const String& tty)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("detach")));
    args->Set(1, CoreUtils::Convert(tty));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("pppd"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::StartAccessPoint(
    /* [in] */ IWifiConfiguration* wifiConfig,
    /* [in] */ const String& wlanIface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    // try {
    AutoPtr<IResources> res;
    mContext->GetResources((IResources**)&res);
    Boolean b = FALSE;
    res->GetBoolean(R::bool_::config_wifiApFirmwareReload, &b);
    if (b) {
        WifiFirmwareReload(wlanIface, String("AP"));
    }
    if (wifiConfig == NULL) {
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
        args->Set(0, CoreUtils::Convert(String("set")));
        args->Set(1, CoreUtils::Convert(wlanIface));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("softap"), args, (NativeDaemonEvent**)&event);
    } else {
        String strSSID;
        wifiConfig->GetSSID(&strSSID);
        String strPreShareKey;
        wifiConfig->GetPreSharedKey(&strPreShareKey);
        AutoPtr<NativeDaemonConnector::SensitiveArg> sensitiveArg =
            new NativeDaemonConnector::SensitiveArg(CoreUtils::Convert(strPreShareKey));
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(7);
        args->Set(0, StringUtils::ParseCharSequence(String("set")).Get());
        args->Set(1, StringUtils::ParseCharSequence(wlanIface).Get());
        args->Set(2, StringUtils::ParseCharSequence(strSSID).Get());
        args->Set(3, StringUtils::ParseCharSequence(String("broadcast")).Get());
        args->Set(4, StringUtils::ParseCharSequence(String("6")).Get());
        args->Set(5, StringUtils::ParseCharSequence(GetSecurityType(wifiConfig)).Get());
        args->Set(6, TO_IINTERFACE(sensitiveArg));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("softap"), args, (NativeDaemonEvent**)&event);
    }
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("startap")));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("softap"), args, (NativeDaemonEvent**)&event);
    return NOERROR;
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
}

String CNetworkManagementService::GetSecurityType(
    /* [in] */ IWifiConfiguration* wifiConfig)
{
    Int32 num;
    wifiConfig->GetAuthType(&num);
    switch (num) {
        case IWifiConfigurationKeyMgmt::WPA_PSK:
        return String("wpa-psk");
        case IWifiConfigurationKeyMgmt::WPA2_PSK:
        return String("wpa2-psk");
        default:
        return String("open");
    }
}

ECode CNetworkManagementService::WifiFirmwareReload(
    /* [in] */ const String& wlanIface,
    /* [in] */ const String& mode)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("fwreload")));
    args->Set(1, CoreUtils::Convert(wlanIface));
    args->Set(2, CoreUtils::Convert(mode));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("softap"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::StopAccessPoint(
    /* [in] */ const String& wlanIface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("stopap")));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("softap"), args, (NativeDaemonEvent**)&event);
    WifiFirmwareReload(wlanIface, String("STA"));
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::SetAccessPoint(
    /* [in] */ IWifiConfiguration* wifiConfig,
    /* [in] */ const String& wlanIface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

     if (wifiConfig == NULL) {
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
        args->Set(0, CoreUtils::Convert(String("set")));
        args->Set(1, CoreUtils::Convert(wlanIface));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("softap"), args, (NativeDaemonEvent**)&event);
     }
     else {
        String ssid;
        wifiConfig->GetSSID(&ssid);
        String presharedKey;
        wifiConfig->GetPreSharedKey(&presharedKey);

        AutoPtr<NativeDaemonConnector::SensitiveArg> sensitiveArg =
            new NativeDaemonConnector::SensitiveArg(CoreUtils::Convert(presharedKey));
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(7);
        args->Set(0, StringUtils::ParseCharSequence(String("set")).Get());
        args->Set(1, StringUtils::ParseCharSequence(wlanIface).Get());
        args->Set(2, StringUtils::ParseCharSequence(ssid).Get());
        args->Set(3, StringUtils::ParseCharSequence(String("broadcast")).Get());
        args->Set(4, StringUtils::ParseCharSequence(String("6")).Get());
        args->Set(5, StringUtils::ParseCharSequence(GetSecurityType(wifiConfig)).Get());
        args->Set(6, TO_IINTERFACE(sensitiveArg));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("softap"), args, (NativeDaemonEvent**)&event);
    }
    return NOERROR;
}

ECode CNetworkManagementService::AddIdleTimer(
    /* [in] */ const String& iface,
    /* [in] */ Int32 timeout,
    /* [in] */ Int32 type)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    if (DBG) Slogger::D(TAG, "Adding idletimer");

    {    AutoLock syncLock(mIdleTimerLock);
        AutoPtr<IInterface> value;
        mActiveIdleTimers->Get(StringUtils::ParseCharSequence(iface), (IInterface**)&value);
        AutoPtr<IdleTimerParams> params = (IdleTimerParams*)IObject::Probe(value);
        if (params != NULL) {
            // the interface already has idletimer, update network count
            params->mNetworkCount++;
            return NOERROR;
        }
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(4);
        args->Set(0, CoreUtils::Convert(String("add")));
        args->Set(1, CoreUtils::Convert(iface));
        args->Set(2, CoreUtils::Convert(StringUtils::ToString(timeout)));
        args->Set(3, CoreUtils::Convert(StringUtils::ToString(type)));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("idletimer"), args, (NativeDaemonEvent**)&event);
        AutoPtr<IdleTimerParams> timer = new IdleTimerParams(timeout, type);
        mActiveIdleTimers->Put(StringUtils::ParseCharSequence(iface), TO_IINTERFACE(timer));

        // Networks start up.
        AutoPtr<IConnectivityManagerHelper> helper;
        CConnectivityManagerHelper::AcquireSingleton((IConnectivityManagerHelper**)&helper);
        Boolean isNetworkTypeMobile;
        helper->IsNetworkTypeMobile(type, &isNetworkTypeMobile);
        if (isNetworkTypeMobile) {
            mNetworkActive = FALSE;
        }
        Boolean tmp;
        mDaemonHandler->Post(new TimerRunnable(this, type), &tmp);
    }

    return NOERROR;
}

ECode CNetworkManagementService::RemoveIdleTimer(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    if (DBG) Slogger::D(TAG, "Removing idletimer");

    {    AutoLock syncLock(mIdleTimerLock);
        AutoPtr<IInterface> value;
        mActiveIdleTimers->Get(StringUtils::ParseCharSequence(iface), (IInterface**)&value);
        AutoPtr<IdleTimerParams> params = (IdleTimerParams*)IObject::Probe(value);
        if (params == NULL || --(params->mNetworkCount) > 0) {
            return NOERROR;
        }
        AutoPtr<ICharSequence> cseq0;
        CString::New(String("remove"), (ICharSequence**)&cseq0);
        AutoPtr<ICharSequence> cseq1;
        CString::New(iface, (ICharSequence**)&cseq1);
        AutoPtr<ICharSequence> cseq2;
        CString::New(StringUtils::ToString(params->mTimeout), (ICharSequence**)&cseq2);
        AutoPtr<ICharSequence> cseq3;
        CString::New(StringUtils::ToString(params->mType), (ICharSequence**)&cseq3);
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(4);
        args->Set(0, CoreUtils::Convert(String("remove")));
        args->Set(1, CoreUtils::Convert(iface));
        args->Set(2, CoreUtils::Convert(StringUtils::ToString(params->mTimeout)));
        args->Set(3, CoreUtils::Convert(StringUtils::ToString(params->mType)));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("idletimer"), args, (NativeDaemonEvent**)&event);
        mActiveIdleTimers->Remove(StringUtils::ParseCharSequence(iface));
        Boolean tmp;
        mDaemonHandler->Post(new RemoveRunnable(this, params), &tmp);
    }
    return NOERROR;
}

ECode CNetworkManagementService::GetNetworkStatsSummaryDev(
    /* [out] */ INetworkStats** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    // try {
    ECode ec = mStatsFactory->ReadNetworkStatsSummaryDev(result);
    // } catch (IOException e) {
    if ((ECode)E_IO_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::GetNetworkStatsSummaryXt(
    /* [out] */ INetworkStats** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    // try {
    ECode ec = mStatsFactory->ReadNetworkStatsSummaryXt(result);
    // } catch (IOException e) {
    if ((ECode)E_IO_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::GetNetworkStatsDetail(
    /* [out] */ INetworkStats** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    // try {
    ECode ec = mStatsFactory->ReadNetworkStatsDetail(INetworkStats::UID_ALL, NULL, INetworkStats::TAG_ALL, NULL, result);
    // } catch (IOException e) {
    if ((ECode)E_IO_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::SetInterfaceQuota(
    /* [in] */ const String& iface,
    /* [in] */ Int64 quotaBytes)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    // silently discard when control disabled
    // TODO: eventually migrate to be always enabled
    if (!mBandwidthControlEnabled)
        return NOERROR;

    {    AutoLock syncLock(mQuotaLock);
        Boolean isContainsKey;
        mActiveQuotas->ContainsKey(StringUtils::ParseCharSequence(iface), &isContainsKey);
        if (isContainsKey) {
            Logger::E(TAG, "iface %s already has quota", iface.string());
            return E_ILLEGAL_STATE_EXCEPTION;
        }

        // TODO: support quota shared across interfaces
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
        args->Set(0, CoreUtils::Convert(String("setiquota")));
        args->Set(1, CoreUtils::Convert(iface));
        args->Set(2, CoreUtils::Convert(quotaBytes));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("bandwidth"), args, (NativeDaemonEvent**)&event);
        mActiveQuotas->Put(StringUtils::ParseCharSequence(iface), CoreUtils::Convert(quotaBytes));
    }
    return NOERROR;
}

ECode CNetworkManagementService::RemoveInterfaceQuota(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    // silently discard when control disabled
    // TODO: eventually migrate to be always enabled
    if (!mBandwidthControlEnabled)
        return NOERROR;

    {    AutoLock syncLock(mQuotaLock);
        Boolean isContainsKey;
        mActiveQuotas->ContainsKey(StringUtils::ParseCharSequence(iface), &isContainsKey);
        if (isContainsKey) {
            // TODO: eventually consider throwing
            return NOERROR;
        }
        mActiveQuotas->Remove(StringUtils::ParseCharSequence(iface));
        mActiveAlerts->Remove(StringUtils::ParseCharSequence(iface));
        // TODO: support quota shared across interfaces
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
        args->Set(0, CoreUtils::Convert(String("removeiquota")));
        args->Set(1, CoreUtils::Convert(iface));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("bandwidth"), args, (NativeDaemonEvent**)&event);
    }
    return NOERROR;
}

ECode CNetworkManagementService::SetInterfaceAlert(
    /* [in] */ const String& iface,
    /* [in] */ Int64 alertBytes)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    // silently discard when control disabled
    // TODO: eventually migrate to be always enabled
    if (!mBandwidthControlEnabled)
        return NOERROR;

    // quick sanity check
    Boolean isContainsKey;
    mActiveQuotas->ContainsKey(StringUtils::ParseCharSequence(iface), &isContainsKey);
    if (isContainsKey) {
        Logger::E(TAG, "setting alert requires existing quota on iface");
        return E_ILLEGAL_STATE_EXCEPTION;
    }

    {    AutoLock syncLock(mQuotaLock);
        Boolean isContainsKey;
        mActiveAlerts->ContainsKey(StringUtils::ParseCharSequence(iface), &isContainsKey);
        if (isContainsKey) {
            Logger::E(TAG, "iface %s already has alert", iface.string());
            return E_ILLEGAL_STATE_EXCEPTION;
        }
         // TODO: support alert shared across interfaces
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
        args->Set(0, CoreUtils::Convert(String("setinterfacealert")));
        args->Set(1, CoreUtils::Convert(iface));
        args->Set(2, CoreUtils::Convert(alertBytes));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("bandwidth"), args, (NativeDaemonEvent**)&event);
        mActiveAlerts->Put(StringUtils::ParseCharSequence(iface), CoreUtils::Convert(alertBytes));
    }
    return NOERROR;
}

ECode CNetworkManagementService::RemoveInterfaceAlert(
    /* [in] */ const String& iface)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    // silently discard when control disabled
    // TODO: eventually migrate to be always enabled
    if (!mBandwidthControlEnabled)
        return NOERROR;

    {    AutoLock syncLock(mQuotaLock);
        Boolean isContainsKey;
        mActiveAlerts->ContainsKey(StringUtils::ParseCharSequence(iface), &isContainsKey);
        if (isContainsKey) {
            // TODO: eventually consider throwing
            return NOERROR;
        }

        // TODO: support alert shared across interfaces
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
        args->Set(0, CoreUtils::Convert(String("removeinterfacealert")));
        args->Set(1, CoreUtils::Convert(iface));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("bandwidth"), args, (NativeDaemonEvent**)&event);
        mActiveAlerts->Remove(StringUtils::ParseCharSequence(iface));
    }
    return NOERROR;
}

ECode CNetworkManagementService::SetGlobalAlert(
    /* [in] */ Int64 alertBytes)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    // silently discard when control disabled
    // TODO: eventually migrate to be always enabled
    if (!mBandwidthControlEnabled)
        return NOERROR;

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("setglobalalert")));
    args->Set(1, CoreUtils::Convert(alertBytes));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("bandwidth"), args, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::SetUidNetworkRules(
    /* [in] */ Int32 uid,
    /* [in] */ Boolean rejectOnQuotaInterfaces)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    // silently discard when control disabled
    // TODO: eventually migrate to be always enabled
    if (!mBandwidthControlEnabled)
        return NOERROR;

    {    AutoLock syncLock(mQuotaLock);
        Boolean oldRejectOnQuota;
        mUidRejectOnQuota->Get(uid, &oldRejectOnQuota);
        if (oldRejectOnQuota == rejectOnQuotaInterfaces) {
            // TODO: eventually consider throwing
            return NOERROR;
        }

        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
        args->Set(0, CoreUtils::Convert(String("setglobalalert")));
        args->Set(1, CoreUtils::Convert(rejectOnQuotaInterfaces ? String("addnaughtyapps") : String("removenaughtyapps")));
        args->Set(2, CoreUtils::Convert(uid));
        AutoPtr<NativeDaemonEvent> event;
        mConnector->Execute(String("bandwidth"), args, (NativeDaemonEvent**)&event);

        if (rejectOnQuotaInterfaces) {
            mUidRejectOnQuota->Put(uid, TRUE);
        } else {
            mUidRejectOnQuota->Delete(uid);
        }
    }
    return NOERROR;
}

ECode CNetworkManagementService::IsBandwidthControlEnabled(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = FALSE;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    *result = mBandwidthControlEnabled;
    return NOERROR;
}

ECode CNetworkManagementService::GetNetworkStatsUidDetail(
    /* [in] */ Int32 uid,
    /* [out] */ INetworkStats** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));
    // try {
    ECode ec = mStatsFactory->ReadNetworkStatsDetail(uid, NULL, INetworkStats::TAG_ALL, NULL, result);
    // } catch (IOException e) {
    if ((ECode)E_IO_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::GetNetworkStatsTethering(
    /* [out] */ INetworkStats** result)
{
    VALIDATE_NOT_NULL(result);
    *result = NULL;

    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
        Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr<INetworkStats> stats;
    CNetworkStats::New(SystemClock::GetElapsedRealtime(), 1, (INetworkStats**)&stats);
    ECode ec;
        // try {
    do {
        AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
        args->Set(0, CoreUtils::Convert(String("gettetherstats")));
        AutoPtr<ArrayOf<NativeDaemonEvent*> > events;
        ec = mConnector->ExecuteForList(
                 String("bandwidth"), args, (ArrayOf<NativeDaemonEvent*>**)&events);
        if (FAILED(ec)) break;
        for (Int32 i = 0; i < events->GetLength(); ++i) {
            AutoPtr<NativeDaemonEvent> event = (*events)[i];
            if (event->GetCode() != NetdResponseCode::TetheringStatsListResult) continue;

            // 114 ifaceIn ifaceOut rx_bytes rx_packets tx_bytes tx_packets
            AutoPtr<IStringTokenizer> tok;
            CStringTokenizer::New(event->GetMessage(), (IStringTokenizer**)&tok);
            // try {
            do {
                String nextToken;
                tok->GetNextToken(&nextToken);
                String ifaceOut;
                tok->GetNextToken(&ifaceOut);

                AutoPtr<INetworkStatsEntry> entry;
                CNetworkStatsEntry::New((INetworkStatsEntry**)&entry);
                entry->SetIface(ifaceOut);
                entry->SetUid(ITrafficStats::UID_TETHERING);
                entry->SetSet(INetworkStats::SET_DEFAULT);
                entry->SetTag(INetworkStats::TAG_NONE);
                tok->GetNextToken(&nextToken);
                Int64 i64;
                ec = StringUtils::Parse(nextToken, &i64);
                if (FAILED(ec)) break;
                entry->SetRxBytes(i64);
                tok->GetNextToken(&nextToken);
                ec = StringUtils::Parse(nextToken, &i64);
                if (FAILED(ec)) break;
                entry->SetRxPackets(i64);
                tok->GetNextToken(&nextToken);
                ec = StringUtils::Parse(nextToken, &i64);
                if (FAILED(ec)) break;
                entry->SetTxBytes(i64);
                tok->GetNextToken(&nextToken);
                ec = StringUtils::Parse(nextToken, &i64);
                if (FAILED(ec)) break;
                entry->SetTxPackets(i64);
                stats->CombineValues(entry);
            } while(FALSE);
            if (FAILED(ec)) {
                // } catch (NoSuchElementException e) {
                if ((ECode)E_NO_SUCH_ELEMENT_EXCEPTION == ec) {
                    Logger::E(TAG, "problem parsing tethering stats: %s", TO_CSTR(event));
                    return E_ILLEGAL_STATE_EXCEPTION;
                }
                // } catch (NumberFormatException e) {
                else if ((ECode)E_NUMBER_FORMAT_EXCEPTION == ec) {
                    Logger::E(TAG, "problem parsing tethering stats: %s", TO_CSTR(event));
                    return E_ILLEGAL_STATE_EXCEPTION;
                }
                else
                    return ec;
                // }
            }
        }
    } while(FALSE);
    // } catch (NativeDaemonConnectorException e) {
    if (FAILED(ec)) {
        if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
            return E_ILLEGAL_STATE_EXCEPTION;
        return ec;
    }
    // }
    *result = stats;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CNetworkManagementService::SetDnsServersForNetwork(
    /* [in] */ Int32 netId,
    /* [in] */ ArrayOf<String>* servers,
    /* [in] */ const String& domains)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, StringUtils::ParseCharSequence(String("setnetdns")));
    args->Set(1, StringUtils::ParseCharSequence(StringUtils::ToString(netId)));
    args->Set(2, StringUtils::ParseCharSequence((domains.IsNull() ? String("") : domains)));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(
        String("resolver"), args);

    for (Int32 i = 0; i < servers->GetLength(); i++) {
        AutoPtr<IInetAddress> addr;
        NetworkUtils::NumericToInetAddress((*servers)[i], (IInetAddress**)&addr);
        Boolean bol;
        addr->IsAnyLocalAddress(&bol);
        if (bol == FALSE) {
            String str;
            addr->GetHostAddress(&str);
            AutoPtr<ICharSequence> cseq;
            CString::New(str, (ICharSequence**)&cseq);
            cmd->AppendArg(cseq);
        }
    }

    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(cmd, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::AddVpnUidRanges(
    /* [in] */ Int32 netId,
    /* [in] */ ArrayOf<IUidRange*>* ranges)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);
    AutoPtr<ArrayOf<IInterface*> > argv = ArrayOf<IInterface*>::Alloc(3 + MAX_UID_RANGES_PER_COMMAND);
    argv->Set(0, StringUtils::ParseCharSequence(String("users")));
    argv->Set(1, StringUtils::ParseCharSequence(String("add")));
    argv->Set(2, StringUtils::ParseCharSequence(StringUtils::ToString(netId)));
    Int32 argc = 3;
    // Avoid overly long commands by limiting number of UID ranges per command.
    for (Int32 i = 0; i < ranges->GetLength(); i++) {
        argv->Set(argc++, (*ranges)[i]);
        if (i == (ranges->GetLength() - 1) || argc == argv->GetLength()) {
            // try {
            ECode ec;
            do {
                AutoPtr<ArrayOf<IInterface*> > subArray;
                ec = Arrays::CopyOf(argv, argc, (ArrayOf<IInterface*>**)&subArray);
                if (FAILED(ec)) break;

                AutoPtr<NativeDaemonEvent> event;
                ec = mConnector->Execute(String("network"), subArray, (NativeDaemonEvent**)&event);
                if (FAILED(ec)) break;
            } while(FALSE);
            // } catch (NativeDaemonConnectorException e) {
            if (FAILED(ec)) {
                if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
                    return E_ILLEGAL_STATE_EXCEPTION;
                return ec;
            }
            // }
            argc = 3;
        }
    }
    return NOERROR;
}

ECode CNetworkManagementService::RemoveVpnUidRanges(
    /* [in] */ Int32 netId,
    /* [in] */ ArrayOf<IUidRange*>* ranges)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);
    AutoPtr<ArrayOf<IInterface*> > argv = ArrayOf<IInterface*>::Alloc(3 + MAX_UID_RANGES_PER_COMMAND);
    argv->Set(0, StringUtils::ParseCharSequence(String("users")));
    argv->Set(1, StringUtils::ParseCharSequence(String("remove")));
    argv->Set(2, StringUtils::ParseCharSequence(StringUtils::ToString(netId)));
    Int32 argc = 3;
    // Avoid overly long commands by limiting number of UID ranges per command.
    for (Int32 i = 0; i < ranges->GetLength(); i++) {
        argv->Set(argc++, (*ranges)[i]);
        if (i == (ranges->GetLength() - 1) || argc == argv->GetLength()) {
            // try {
            ECode ec;
            do {
                AutoPtr<ArrayOf<IInterface*> > subArray;
                ec = Arrays::CopyOf(argv, argc, (ArrayOf<IInterface*>**)&subArray);
                if (FAILED(ec)) break;
                AutoPtr<NativeDaemonEvent> event;
                ec = mConnector->Execute(String("network"), subArray, (NativeDaemonEvent**)&event);
                if (FAILED(ec)) break;
            } while(FALSE);
            // } catch (NativeDaemonConnectorException e) {
            if (FAILED(ec)) {
                if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
                    return E_ILLEGAL_STATE_EXCEPTION;
                return ec;
            }
            // }
            argc = 3;
        }
    }
    return NOERROR;
}

ECode CNetworkManagementService::FlushNetworkDnsCache(
    /* [in] */ Int32 netId)
{
    FAIL_RETURN(mContext->EnforceCallingOrSelfPermission(
            Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL/*CONNECTIVITY_INTERNAL*/, TAG));

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("flushnet")));
    args->Set(1, CoreUtils::Convert(StringUtils::ToString(netId)));
    // try {
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("resolver"), args, (NativeDaemonEvent**)&event);
    // } catch (NativeDaemonConnectorException e) {
    //     throw e.rethrowAsParcelableException();
    // }
    return NOERROR;
}

ECode CNetworkManagementService::SetFirewallEnabled(
    /* [in] */ Boolean enabled)
{
    FAIL_RETURN(EnforceSystemUid());

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(enabled ? String("enable") : String("disable")));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("firewall"), args, (NativeDaemonEvent**)&event);
    mFirewallEnabled = enabled;
    return NOERROR;
}

ECode CNetworkManagementService::IsFirewallEnabled(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = FALSE;

    FAIL_RETURN(EnforceSystemUid());
    *result = mFirewallEnabled;
    return NOERROR;
}

ECode CNetworkManagementService::SetFirewallInterfaceRule(
    /* [in] */ const String& iface,
    /* [in] */ Boolean allow)
{
    EnforceSystemUid();
//     Preconditions::CheckState(mFirewallEnabled);
    String rule = allow ? String("allow") : String("deny");

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("set_interface_rule")));
    args->Set(1, CoreUtils::Convert(iface));
    args->Set(2, CoreUtils::Convert(rule));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("firewall"), args, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::SetFirewallEgressSourceRule(
    /* [in] */ const String& addr,
    /* [in] */ Boolean allow)
{
    FAIL_RETURN(EnforceSystemUid());
//     Preconditions::CheckState(mFirewallEnabled);
    String rule = allow ? String("allow") : String("deny");

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("set_egress_source_rule")));
    args->Set(1, CoreUtils::Convert(addr));
    args->Set(2, CoreUtils::Convert(rule));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("firewall"), args, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::SetFirewallEgressDestRule(
    /* [in] */ const String& addr,
    /* [in] */ Int32 port,
    /* [in] */ Boolean allow)
{
    FAIL_RETURN(EnforceSystemUid());
    AutoPtr<IPreconditions> preconditions;
    CPreconditions::AcquireSingleton((IPreconditions**)&preconditions);
    ECode ec = preconditions->CheckState(mFirewallEnabled);
    if (FAILED(ec)) {
        Logger::E(TAG, "mFirewallEnabled is not expected, mFirewallEnabled:%d", mFirewallEnabled);
        return ec;
    }
    String rule = allow ? String("allow") : String("deny");

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(4);
    args->Set(0, CoreUtils::Convert(String("set_egress_dest_rule")));
    args->Set(1, CoreUtils::Convert(addr));
    args->Set(2, CoreUtils::Convert(port));
    args->Set(3, CoreUtils::Convert(rule));
    AutoPtr<NativeDaemonEvent> event;
    mConnector->Execute(String("firewall"), args, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::SetFirewallUidRule(
    /* [in] */ Int32 uid,
    /* [in] */ Boolean allow)
{
    FAIL_RETURN(EnforceSystemUid());
    // Preconditions::CheckState(mFirewallEnabled);
    String rule = allow ? String("allow") : String("deny");

    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, CoreUtils::Convert(String("set_uid_rule")));
    args->Set(1, CoreUtils::Convert(uid));
    args->Set(2, CoreUtils::Convert(rule));
    AutoPtr<NativeDaemonEvent> event;

    mConnector->Execute(String("firewall"), args, (NativeDaemonEvent**)&event);
    return NOERROR;
}

ECode CNetworkManagementService::EnforceSystemUid()
{
    Int32 uid = Binder::GetCallingUid();
    if (uid != IProcess::SYSTEM_UID) {
        //throw new SecurityException("Only available to AID_SYSTEM");
        return E_SECURITY_EXCEPTION;
    }
    return NOERROR;
}

ECode CNetworkManagementService::StartClatd(
        /* [in] */ const String& interfaceName)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("start")));
    args->Set(1, CoreUtils::Convert(interfaceName));
    AutoPtr<NativeDaemonEvent> event;
    ECode ec = mConnector->Execute(String("clatd"), args, (NativeDaemonEvent**)&event);
    // // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    //}
    return ec;
}

ECode CNetworkManagementService::StopClatd()
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    args->Set(0, CoreUtils::Convert(String("stop")));
    AutoPtr<NativeDaemonEvent> event;
    ECode ec = mConnector->Execute(String("clatd"), args, (NativeDaemonEvent**)&event);
    // // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // // }
    return ec;
}

ECode CNetworkManagementService::IsClatdStarted(
        /* [out] */ Boolean* result)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    AutoPtr<NativeDaemonEvent> event;
    // try {
    //AutoPtr<ICharSequence> cseq;
    //CString::New(String("status"), (ICharSequence**)&cseq);
    //AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(1);
    //args->Set(0, cseq);
    ECode ec = mConnector->Execute(String("clatd"), String("status"), (NativeDaemonEvent**)&event);
    // // } catch (NativeDaemonConnectorException e) {
    if (FAILED(ec)) {
        if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
            return E_ILLEGAL_STATE_EXCEPTION;
        return ec;
    }
    // // }

    event->CheckCode(NetdResponseCode::ClatdStatusResult);
    *result = event->GetMessage().EndWith("started");
    return NOERROR;
}

ECode CNetworkManagementService::RegisterNetworkActivityListener(
        /* [in] */ INetworkActivityListener* listener)
{
    Boolean tmp;
    return mNetworkActivityListeners->Register(listener, &tmp);
}

ECode CNetworkManagementService::UnregisterNetworkActivityListener(
        /* [in] */ INetworkActivityListener* listener)
{
    Boolean tmp;
    return mNetworkActivityListeners->Unregister(listener, &tmp);
}

ECode CNetworkManagementService::ExcludeLinkLocal(
    /* [in] */ IList* addresses,
    /* [out] */ IList** result)
{
    Int32 size;
    addresses->GetSize(&size);
    AutoPtr<IArrayList> filtered;
    CArrayList::New(size, (IArrayList**)&filtered);
    AutoPtr<IIterator> it;
    addresses->GetIterator((IIterator**)&it);
    Boolean hasNext;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> obj;
        it->GetNext((IInterface**)&obj);
        IInterfaceAddress* ia = IInterfaceAddress::Probe(obj);
        AutoPtr<IInetAddress> address;
        ia->GetAddress((IInetAddress**)&address);
        Boolean linkLocalAddress;
        if (address->IsLinkLocalAddress(&linkLocalAddress), !linkLocalAddress) {
            filtered->Add(ia);
        }
    }
    *result = IList::Probe(filtered);
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CNetworkManagementService::ReportNetworkActive()
{
    Int32 length;
    mNetworkActivityListeners->BeginBroadcast(&length);
    // try {
    ECode ec = NOERROR;
    for (Int32 i = 0; i < length; i++) {
        // try {
        AutoPtr<IInterface> obj;
        do {
            ec = mNetworkActivityListeners->GetBroadcastItem(i, (IInterface**)&obj);
            if (FAILED(ec)) break;
            ec = INetworkActivityListener::Probe(obj)->OnNetworkActive();
        } while(FALSE);
        if (FAILED(ec)) {
        // } catch (RemoteException e) {
        // } catch (RuntimeException e) {
            if ((ECode)E_REMOTE_EXCEPTION == ec || (ECode)E_RUNTIME_EXCEPTION == ec)
                continue;
            break;
        // }
        }
    }
    // } finally {
    mNetworkActivityListeners->FinishBroadcast();
    // }
    return ec;
}

ECode CNetworkManagementService::IsNetworkActive(
    /* [out] */ Boolean* result)
{
    {
        AutoLock syncLock(mNetworkActivityListeners);
        Boolean empty;
        *result = mNetworkActive || (mActiveIdleTimers->IsEmpty(&empty), empty);
    }
    return NOERROR;
}

ECode CNetworkManagementService::Monitor()
{
    if (mConnector != NULL) {
        mConnector->Monitor();
    }
    return NOERROR;
}

void CNetworkManagementService::Dump(
    /* [in] */ IFileDescriptor* fd,
    /* [in] */ IPrintWriter* pw,
    /* [in] */ ArrayOf<String>* args)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::DUMP, TAG);

    pw->Println(String("NetworkManagementService NativeDaemonConnector Log:"));
    mConnector->Dump(fd, pw, args);
    pw->Println();

    pw->Print(String("Bandwidth control enabled: "));
    pw->Println(mBandwidthControlEnabled);
    pw->Print(String("mMobileActivityFromRadio="));
    pw->Print(mMobileActivityFromRadio);
    pw->Print(String(" mLastPowerStateFromRadio="));
    pw->Println(mLastPowerStateFromRadio);
    pw->Print(String("mNetworkActive="));
    pw->Println(mNetworkActive);

    {
        AutoLock syncLock(mQuotaLock);
        pw->Print(String("Active quota ifaces: "));
        pw->Println(Object::ToString(mActiveQuotas));
        pw->Print(String("Active alert ifaces: "));
        pw->Println(Object::ToString(mActiveAlerts));
    }

    {
        AutoLock syncLock(mUidRejectOnQuota);
        pw->Print(String("UID reject on quota ifaces: ["));
        Int32 size;
        mUidRejectOnQuota->GetSize(&size);
        for (Int32 i = 0; i < size; i++) {
            Int32 key;
            mUidRejectOnQuota->KeyAt(i, &key);
            pw->Print(key);
            if (i < size - 1) pw->Print(String(","));
        }
        pw->Println(String("]"));
    }

    {
        AutoLock syncLock(mIdleTimerLock);
        pw->Println(String("Idle timers:"));
        AutoPtr<ISet> entries;
        mActiveIdleTimers->GetEntrySet((ISet**)&entries);
        AutoPtr<IIterator> it;
        entries->GetIterator((IIterator**)&it);
        Boolean hasNext;
        while (it->HasNext(&hasNext), hasNext) {
            AutoPtr<IInterface> obj;
            it->GetNext((IInterface**)&obj);
            AutoPtr<IMapEntry> ent = IMapEntry::Probe(obj);
            pw->Print(String("  "));
            obj = NULL;
            ent->GetKey((IInterface**)&obj);
            pw->Print(obj);
            pw->Println(String(":"));
            obj = NULL;
            ent->GetValue((IInterface**)&obj);
            AutoPtr<IdleTimerParams> params = (IdleTimerParams*)IObject::Probe(obj);
            pw->Print(String("    timeout="));
            pw->Print(params->mTimeout);
            pw->Print(String(" type="));
            pw->Print(params->mType);
            pw->Print(String(" networkCount="));
            pw->Println(params->mNetworkCount);
        }
    }

    pw->Print(String("Firewall enabled: "));
    pw->Println(mFirewallEnabled);
}

ECode CNetworkManagementService::GetBatteryStats(
    /* [out] */ IIBatteryStats** result)
{
    {
        AutoLock syncLock(this);
        if (mBatteryStats != NULL) {
            *result = mBatteryStats;
            REFCOUNT_ADD(*result);
            return NOERROR;
        }
        AutoPtr<IServiceManager> helper;
        CServiceManager::AcquireSingleton((IServiceManager**)&helper);
        AutoPtr<IInterface> service;
        helper->GetService(IBatteryStats::SERVICE_NAME, (IInterface**)&service);
        mBatteryStats = IIBatteryStats::Probe(service);
        *result = mBatteryStats;
        REFCOUNT_ADD(*result);
        return NOERROR;
    }
}

ECode CNetworkManagementService::CreatePhysicalNetwork(
    /* [in] */ Int32 netId)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr< ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("create")));
    args->Set(1, CoreUtils::Convert(netId));
    AutoPtr<NativeDaemonEvent> event;
    ECode ec = mConnector->Execute(String("network"), args, (NativeDaemonEvent**)&event);
    // // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
         return E_ILLEGAL_STATE_EXCEPTION;
    //}
    return ec;
}

ECode CNetworkManagementService::CreateVirtualNetwork(
    /* [in] */ Int32 netId,
    /* [in] */ Boolean hasDNS,
    /* [in] */ Boolean secure)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(5);
    args->Set(0, StringUtils::ParseCharSequence(String("create")));
    args->Set(1, StringUtils::ParseCharSequence(StringUtils::ToString(netId)));
    args->Set(2, StringUtils::ParseCharSequence(String("vpn")));
    if (hasDNS)
        args->Set(3, StringUtils::ParseCharSequence(String("1")));
    else
        args->Set(3, StringUtils::ParseCharSequence(String("0")));
    if (secure)
        args->Set(3, StringUtils::ParseCharSequence(String("1")));
    else
        args->Set(3, StringUtils::ParseCharSequence(String("0")));
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec = mConnector->Execute(String("network"), args, (NativeDaemonEvent**)&tmp);
    // // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
         return E_ILLEGAL_STATE_EXCEPTION;
    // // }
    return ec;
}

ECode CNetworkManagementService::RemoveNetwork(
    /* [in] */ Int32 netId)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, CoreUtils::Convert(String("destroy")));
    args->Set(1, CoreUtils::Convert(netId));
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec = mConnector->Execute(String("network"), args, (NativeDaemonEvent**)&tmp);
    // // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
         return E_ILLEGAL_STATE_EXCEPTION;
    // // }
    return ec;
}

ECode CNetworkManagementService::AddInterfaceToNetwork(
    /* [in] */ const String& iface,
    /* [in] */ Int32 netId)
{
    return ModifyInterfaceInNetwork(String("add"), StringUtils::ToString(netId), iface);
}

ECode CNetworkManagementService::RemoveInterfaceFromNetwork(
    /* [in] */ const String& iface,
    /* [in] */ Int32 netId)
{
    return ModifyInterfaceInNetwork(String("remove"), StringUtils::ToString(netId), iface);
}

ECode CNetworkManagementService::AddLegacyRouteForNetId(
    /* [in] */ Int32 netId,
    /* [in] */ IRouteInfo* routeInfo,
    /* [in] */ Int32 uid)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(5);
    args->Set(0, CoreUtils::Convert(String("route")));
    args->Set(1, CoreUtils::Convert(String("legacy")));
    args->Set(2, CoreUtils::Convert(uid));
    args->Set(3, CoreUtils::Convert(String("add")));
    args->Set(4, CoreUtils::Convert(netId));
    AutoPtr<NativeDaemonConnector::Command> cmd = new NativeDaemonConnector::Command(String("network"), args);

    // create triplet: interface dest-ip-addr/prefixlength gateway-ip-addr
    AutoPtr<ILinkAddress> la;
    routeInfo->GetDestinationLinkAddress((ILinkAddress**)&la);
    String itf;
    routeInfo->GetInterface(&itf);
    cmd->AppendArg(CoreUtils::Convert(itf));
    AutoPtr<IInetAddress> address;
    la->GetAddress((IInetAddress**)&address);
    String host;
    address->GetHostAddress(&host);
    Int32 length;
    la->GetPrefixLength(&length);
    cmd->AppendArg(CoreUtils::Convert(host + "/" + StringUtils::ToString(length)));
    Boolean hasGateway;
    if (routeInfo->HasGateway(&hasGateway), hasGateway) {
        address = NULL;
        routeInfo->GetGateway((IInetAddress**)&address);
        address->GetHostAddress(&host);
        cmd->AppendArg(CoreUtils::Convert(host));
    }

    // try {
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec;
    ec = mConnector->Execute(cmd, (NativeDaemonEvent**)&tmp);
    // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::SetDefaultNetId(
    /* [in] */ Int32 netId)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(3);
    args->Set(0, StringUtils::ParseCharSequence(String("default")));
    args->Set(1, StringUtils::ParseCharSequence(String("set")));
    AutoPtr<IInteger32> i32;
    CInteger32::New(netId, (IInteger32**)&i32);
    args->Set(2, i32);
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec;
    ec = mConnector->Execute(String("network"), args, (NativeDaemonEvent**)&tmp);
    // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::ClearDefaultNetId()
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr<ArrayOf<IInterface*> > args = ArrayOf<IInterface*>::Alloc(2);
    args->Set(0, StringUtils::ParseCharSequence(String("default")));
    args->Set(1, StringUtils::ParseCharSequence(String("clear")));
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec;
    ec = mConnector->Execute(String("network"), args, (NativeDaemonEvent**)&tmp);
    // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::SetPermission(
    /* [in] */ const String& permission,
    /* [in] */ ArrayOf<Int32>* uids)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    AutoPtr<ArrayOf<IInterface*> > argv = ArrayOf<IInterface*>::Alloc(4 + MAX_UID_RANGES_PER_COMMAND);
    argv->Set(0, StringUtils::ParseCharSequence(String("permission")));
    argv->Set(1, StringUtils::ParseCharSequence(String("user")));
    argv->Set(2, StringUtils::ParseCharSequence(String("set")));
    argv->Set(3, StringUtils::ParseCharSequence(permission));
    Int32 argc = 4;
    // Avoid overly long commands by limiting number of UIDs per command.
    for (Int32 i = 0; i < uids->GetLength(); ++i) {
        AutoPtr<IInteger32> i32;
        CInteger32::New((*uids)[i], (IInteger32**)&i32);
        argv->Set(argc++, i32);
        if (i == uids->GetLength() - 1 || argc == argv->GetLength()) {
            // try {
            AutoPtr<NativeDaemonEvent> tmp;
            AutoPtr<ArrayOf<IInterface*> >  newArgv;
            Arrays::CopyOf(argv, argc, (ArrayOf<IInterface*>**)&newArgv);
            ECode ec;
            ec = mConnector->Execute(String("network"), newArgv, (NativeDaemonEvent**)&tmp);
            // } catch (NativeDaemonConnectorException e) {
            if (FAILED(ec)) {
                if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
                    return E_ILLEGAL_STATE_EXCEPTION;
                return ec;
            }
            // }
            argc = 4;
        }
    }
    return NOERROR;
}

ECode CNetworkManagementService::ClearPermission(
    /* [in] */ ArrayOf<Int32>* uids)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    AutoPtr<ArrayOf<IInterface*> > argv = ArrayOf<IInterface*>::Alloc(3 + MAX_UID_RANGES_PER_COMMAND);
    argv->Set(0, StringUtils::ParseCharSequence(String("permission")));
    argv->Set(1, StringUtils::ParseCharSequence(String("user")));
    argv->Set(2, StringUtils::ParseCharSequence(String("clear")));
    Int32 argc = 3;
    // Avoid overly long commands by limiting number of UIDs per command.
    for (Int32 i = 0; i < uids->GetLength(); ++i) {
        AutoPtr<IInteger32> i32;
        CInteger32::New((*uids)[i], (IInteger32**)&i32);
        argv->Set(argc++, i32);
        if (i == uids->GetLength() - 1 || argc == argv->GetLength()) {
            // try {
            AutoPtr<ArrayOf<IInterface*> >  newArgv;
            Arrays::CopyOf(argv, argc, (ArrayOf<IInterface*>**)&newArgv);

            AutoPtr<NativeDaemonEvent> event;
            ECode ec;
            ec = mConnector->Execute(String("network"), newArgv, (NativeDaemonEvent**)&event);
            // } catch (NativeDaemonConnectorException e) {
            if (FAILED(ec)) {
                if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
                    return E_ILLEGAL_STATE_EXCEPTION;
                return ec;
            }
            // }
            argc = 3;
        }
    }
    return NOERROR;
}

ECode CNetworkManagementService::AllowProtect(
    /* [in] */ Int32 uid)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr<ArrayOf<IInterface*> > argv = ArrayOf<IInterface*>::Alloc(3);
    argv->Set(0, StringUtils::ParseCharSequence(String("protect")));
    argv->Set(1, StringUtils::ParseCharSequence(String("allow")));
    argv->Set(2, StringUtils::ParseCharSequence(StringUtils::ToString(uid)));
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec;
    ec = mConnector->Execute(String("network"), argv, (NativeDaemonEvent**)&tmp);
    // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::DenyProtect(
    /* [in] */ Int32 uid)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);

    // try {
    AutoPtr<ArrayOf<IInterface*> > argv = ArrayOf<IInterface*>::Alloc(3);
    argv->Set(0, StringUtils::ParseCharSequence(String("protect")));
    argv->Set(1, StringUtils::ParseCharSequence(String("deny")));
    argv->Set(2, StringUtils::ParseCharSequence(StringUtils::ToString(uid)));
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec;
    ec = mConnector->Execute(String("network"), argv, (NativeDaemonEvent**)&tmp);
    // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::AddInterfaceToLocalNetwork(
    /* [in] */ const String& iface,
    /* [in] */ IList* routes)
{
    ModifyInterfaceInNetwork(String("add"), String("local"), iface);

    AutoPtr<IIterator> it;
    routes->GetIterator((IIterator**)&it);
    Boolean hasNext;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> route;
        it->GetNext((IInterface**)&route);
        Boolean defaultRoute;
        IRouteInfo::Probe(route)->IsDefaultRoute(&defaultRoute);
        if (!defaultRoute) {
            ModifyRoute(String("add"), String("local"), IRouteInfo::Probe(route));
        }
    }
    return NOERROR;
}

ECode CNetworkManagementService::RemoveInterfaceFromLocalNetwork(
    /* [in] */ const String& iface)
{
    return ModifyInterfaceInNetwork(String("remove"), String("local"), iface);
}

ECode CNetworkManagementService::ModifyInterfaceInNetwork(
    /* [in] */ const String& action,
    /* [in] */ const String& netId,
    /* [in] */ const String& iface)
{
    mContext->EnforceCallingOrSelfPermission(Elastos::Droid::Manifest::permission::CONNECTIVITY_INTERNAL, TAG);
    // try {
    AutoPtr<ArrayOf<IInterface*> > argv = ArrayOf<IInterface*>::Alloc(4);
    argv->Set(0, StringUtils::ParseCharSequence(String("interface")));
    argv->Set(1, StringUtils::ParseCharSequence(action));
    argv->Set(2, StringUtils::ParseCharSequence(netId));
    argv->Set(3, StringUtils::ParseCharSequence(iface));
    AutoPtr<NativeDaemonEvent> tmp;
    ECode ec;
    ec = mConnector->Execute(String("network"), argv, (NativeDaemonEvent**)&tmp);
    // } catch (NativeDaemonConnectorException e) {
    if ((ECode)E_NATIVE_DAEMON_CONNECTOR_EXCEPTION == ec)
        return E_ILLEGAL_STATE_EXCEPTION;
    // }
    return ec;
}

ECode CNetworkManagementService::ToString(
    /* [out] */ String* str)
{
    return IObject::Probe(TO_IINTERFACE(this))->ToString(str);
}

} // namespace Server
} // namepsace Droid
} // namespace Elastos
