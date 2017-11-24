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

#include "elastos/droid/bluetooth/BluetoothSocket.h"
#include "elastos/droid/bluetooth/CBluetoothDevice.h"
#include "elastos/droid/bluetooth/CBluetoothAdapter.h"
#include "elastos/droid/bluetooth/BluetoothInputStream.h"
#include "elastos/droid/bluetooth/BluetoothOutputStream.h"
#include "elastos/droid/net/CLocalSocket.h"
#include "elastos/droid/os/CParcelFileDescriptor.h"
#include "elastos/core/AutoLock.h"
#include <elastos/utility/logging/Slogger.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::Net::CLocalSocket;
using Elastos::Droid::Os::CParcelFileDescriptor;
using Elastos::IO::IFileDescriptor;
using Elastos::IO::EIID_ICloseable;
using Elastos::IO::IByteBuffer;
using Elastos::IO::IByteBufferHelper;
using Elastos::IO::CByteBufferHelper;
using Elastos::IO::ByteOrder;
using Elastos::IO::IByteOrderHelper;
using Elastos::IO::CByteOrderHelper;
using Elastos::IO::IFlushable;
using Elastos::Core::AutoLock;
using Elastos::Utility::Logging::Slogger;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Bluetooth {

const Int32 BluetoothSocket::TYPE_RFCOMM;
const Int32 BluetoothSocket::TYPE_SCO;
const Int32 BluetoothSocket::TYPE_L2CAP;
const Int32 BluetoothSocket::_EBADFD;
const Int32 BluetoothSocket::_EADDRINUSE;
const Int32 BluetoothSocket::SEC_FLAG_ENCRYPT;
const Int32 BluetoothSocket::SEC_FLAG_AUTH;
const String BluetoothSocket::TAG("BluetoothSocket");
const Boolean BluetoothSocket::DBG = Logger::IsLoggable(TAG, Logger::___DEBUG);
const Boolean BluetoothSocket::VDBG = Logger::IsLoggable(TAG, Logger::VERBOSE);
Int32 BluetoothSocket::PROXY_CONNECTION_TIMEOUT = 5000;
Int32 BluetoothSocket::SOCK_SIGNAL_SIZE = 16;

CAR_INTERFACE_IMPL(BluetoothSocket, Object, IBluetoothSocket, ICloseable)

BluetoothSocket::BluetoothSocket()
{
}

BluetoothSocket::BluetoothSocket(
    /* [in] */ Int32 type,
    /* [in] */ Int32 fd,
    /* [in] */ Boolean auth,
    /* [in] */ Boolean encrypt,
    /* [in] */ IBluetoothDevice* device,
    /* [in] */ Int32 port,
    /* [in] */ IParcelUuid* uuid)
    : mType(type)
    , mDevice(device)
    , mAuth(auth)
    , mEncrypt(encrypt)
    , mPort(port)
    , mFd(fd)
    , mSocketState(BluetoothSocket::INIT)
{
    if (type == BluetoothSocket::TYPE_RFCOMM && uuid == NULL && fd == -1) {
        if (port < 1 || port > BluetoothSocket::MAX_RFCOMM_CHANNEL) {
            // throw new IOException("Invalid RFCOMM channel: " + port);
            Slogger::E(TAG, "Invalid RFCOMM channel: %d", port);
            assert(0);
        }
    }

    if(uuid != NULL) {
        mUuid = uuid;
    }
    else {
        // Mike TODO:
        // mUuid = new ParcelUuid(new UUID(0, 0));
    }

    if (device == NULL) {
        // Server socket
        AutoPtr<IBluetoothAdapter> adapter = CBluetoothAdapter::GetDefaultAdapter();
        adapter->GetAddress(&mAddress);
    }
    else {
        // Remote socket
        device->GetAddress(&mAddress);
    }

    mInputStream = new BluetoothInputStream(this);
    mOutputStream = new BluetoothOutputStream(this);
}

BluetoothSocket::BluetoothSocket(
    /* [in] */ BluetoothSocket* s)
    : mType(s->mType)
    , mAuth(s->mAuth)
    , mEncrypt(s->mEncrypt)
    , mUuid(s->mUuid)
    , mPort(s->mPort)
    , mFd(0)
    , mServiceName(s->mServiceName)
{
    mInputStream = new BluetoothInputStream(this);
    mOutputStream = new BluetoothOutputStream(this);
}

ECode BluetoothSocket::AcceptSocket(
    /* [in] */ const String& remoteAddr,
    /* [out] */ BluetoothSocket** socket)
{
    VALIDATE_NOT_NULL(socket)

    AutoPtr<BluetoothSocket> as = new BluetoothSocket(this);
    as->mSocketState = CONNECTED;
    AutoPtr< ArrayOf<IFileDescriptor*> > fds;
    mSocket->GetAncillaryFileDescriptors((ArrayOf<IFileDescriptor*>**)&fds);
    if (DBG) Slogger::D(TAG, "socket fd passed by stack  fds: %p", fds.Get());
    if(fds == NULL || fds->GetLength() != 1) {
        Slogger::E(TAG, "fd passed from stack failed");
        as->Close();
        return E_IO_EXCEPTION;
        // throw new IOException("bt socket acept failed");
    }
    CLocalSocket::New((*fds)[0], (ILocalSocket**)&as->mSocket);
    as->mSocket->GetInputStream((IInputStream**)&as->mSocketIS);
    as->mSocket->GetOutputStream((IOutputStream**)&as->mSocketOS);
    as->mAddress = remoteAddr;

    AutoPtr<IBluetoothAdapter> adapter = CBluetoothAdapter::GetDefaultAdapter();
    adapter->GetRemoteDevice(remoteAddr, (IBluetoothDevice**)&as->mDevice);

    *socket = as;
    REFCOUNT_ADD(*socket)
    return NOERROR;
}

BluetoothSocket::BluetoothSocket(
    /* [in] */ Int32 type,
    /* [in] */ Int32 fd,
    /* [in] */ Boolean auth,
    /* [in] */ Boolean encrypt,
    /* [in] */ const String& address,
    /* [in] */ Int32 port)
{
    AutoPtr<IBluetoothDevice> device;
    CBluetoothDevice::New(address, (IBluetoothDevice**)&device);
    BluetoothSocket(type, fd, auth, encrypt, device, port, NULL);
}

BluetoothSocket::~BluetoothSocket()
{
    // try {
    Close();
    // } finally {
    //     super.finalize();
    // }
}

Int32 BluetoothSocket::GetSecurityFlags()
{
    Int32 flags = 0;
    if(mAuth) flags |= SEC_FLAG_AUTH;
    if(mEncrypt) flags |= SEC_FLAG_ENCRYPT;
    return flags;
}

ECode BluetoothSocket::GetRemoteDevice(
    /* [out] */ IBluetoothDevice** device)
{
    VALIDATE_NOT_NULL(device)
    *device = mDevice;
    REFCOUNT_ADD(*device)
    return NOERROR;
}

ECode BluetoothSocket::GetInputStream(
    /* [out] */ IInputStream** inputstream)
{
    VALIDATE_NOT_NULL(inputstream)
    *inputstream = (IInputStream*)mInputStream;
    REFCOUNT_ADD(*inputstream)
    return NOERROR;
}

ECode BluetoothSocket::GetOutputStream(
    /* [out] */ IOutputStream** outputstream)
{
    VALIDATE_NOT_NULL(outputstream)
    *outputstream = (IOutputStream*)mOutputStream;
    REFCOUNT_ADD(*outputstream)
    return NOERROR;
}

ECode BluetoothSocket::IsConnected(
    /* [out] */ Boolean* isConnected)
{
    VALIDATE_NOT_NULL(isConnected)
    *isConnected = mSocketState == CONNECTED;
    return NOERROR;
}

void BluetoothSocket::SetServiceName(
    /* [in] */ const String& name)
{
    mServiceName = name;
}

ECode BluetoothSocket::Connect()
{
    if (mDevice == NULL) {
        // throw new IOException("Connect is called on null device");
        Slogger::E(TAG, "Connect is called on null device");
        return E_IO_EXCEPTION;
    }

    // try {
    if (mSocketState == CLOSED) {
        // throw new IOException("socket closed");
        Slogger::E(TAG, "socket closed");
        return E_IO_EXCEPTION;
    }

    AutoPtr<IBluetoothAdapter> adapter = CBluetoothAdapter::GetDefaultAdapter();
    AutoPtr<IIBluetooth> bluetoothProxy = ((CBluetoothAdapter*)adapter.Get())->GetBluetoothService(NULL);
    if (bluetoothProxy == NULL) {
        // throw new IOException("Bluetooth is off");
        Slogger::E(TAG, "Bluetooth is off");
        return E_IO_EXCEPTION;
    }
    FAIL_RETURN(bluetoothProxy->ConnectSocket(mDevice, mType, mUuid, mPort,
            GetSecurityFlags(), (IParcelFileDescriptor**)&mPfd))

    {
        AutoLock lock(mLock);
        if (DBG) Slogger::D(TAG, "connect(), SocketState: %d, mPfd: %p", mSocketState, mPfd.Get());
        if (mSocketState == CLOSED) {
            // throw new IOException("socket closed");
            Slogger::E(TAG, "socket closed");
            return E_IO_EXCEPTION;
        }
        if (mPfd == NULL) {
            // throw new IOException("bt socket connect failed");
            Slogger::E(TAG, "bt socket connect failed");
            return E_IO_EXCEPTION;
        }
        AutoPtr<IFileDescriptor> fd;
        mPfd->GetFileDescriptor((IFileDescriptor**)&fd);
        CLocalSocket::New(fd, (ILocalSocket**)&mSocket);
        mSocket->GetInputStream((IInputStream**)&mSocketIS);
        mSocket->GetOutputStream((IOutputStream**)&mSocketOS);
    }
    Int32 channel;
    ReadInt32(mSocketIS, &channel);
    if (channel <= 0) {
        // throw new IOException("bt socket connect failed");
        Slogger::E(TAG, "bt socket connect failed");
        return E_IO_EXCEPTION;
    }

    mPort = channel;
    String str;
    WaitSocketSignal(mSocketIS, &str);
    AutoLock lock(mLock);
    if (mSocketState == CLOSED) {
        // throw new IOException("bt socket closed");
        Slogger::E(TAG, "bt socket closed");
        return E_IO_EXCEPTION;
    }
    mSocketState = CONNECTED;
    // } catch (RemoteException e) {
    //     Log.e(TAG, Log.getStackTraceString(new Throwable()));
    //     throw new IOException("unable to send RPC: " + e.getMessage());
    // }
    return NOERROR;
}

Int32 BluetoothSocket::BindListen()
{
    if (mSocketState == CLOSED) return _EBADFD;

    AutoPtr<IBluetoothAdapter> adapter = CBluetoothAdapter::GetDefaultAdapter();
    AutoPtr<IIBluetooth> bluetoothProxy = ((CBluetoothAdapter*)adapter.Get())->GetBluetoothService(NULL);
    if (bluetoothProxy == NULL) {
        Slogger::E(TAG, "bindListen fail, reason: bluetooth is off");
        return -1;
    }
    // try {
    if (FAILED(bluetoothProxy->CreateSocketChannel(mType, mServiceName,
            mUuid, mPort, GetSecurityFlags(), (IParcelFileDescriptor**)&mPfd))) {
        Slogger::E(TAG, "create socket channel failed");
        return -1;
    }
    // } catch (RemoteException e) {
    //     Log.e(TAG, Log.getStackTraceString(new Throwable()));
    //     return -1;
    // }

    // read out port number
    // try {
    {
        AutoLock lock(mLock);
        if (DBG) Slogger::D(TAG, "bindListen(), SocketState: %d, mPfd: %p", mSocketState, mPfd.Get());
        if(mSocketState != INIT) return _EBADFD;
        if(mPfd == NULL) return -1;
        AutoPtr<IFileDescriptor> fd;
        mPfd->GetFileDescriptor((IFileDescriptor**)&fd);
        if (DBG) Slogger::D(TAG, "bindListen(), new LocalSocket ");
        CLocalSocket::New(fd, (ILocalSocket**)&mSocket);
        if (DBG) Slogger::D(TAG, "bindListen(), new LocalSocket.getInputStream() ");
        mSocket->GetInputStream((IInputStream**)&mSocketIS);
        mSocket->GetOutputStream((IOutputStream**)&mSocketOS);
    }

    if (DBG) Slogger::D(TAG, "bindListen(), readInt mSocketIS: %p", mSocketIS.Get());
    Int32 channel;
    ReadInt32(mSocketIS, &channel);
    {
        AutoLock lock(mLock);
        if(mSocketState == INIT)
            mSocketState = LISTENING;
    }
    if (DBG) Slogger::D(TAG, "channel: %d", channel);
    if (mPort == -1) {
        mPort = channel;
    } // else ASSERT(mPort == channel)
    // } catch (IOException e) {
    //     if (mPfd != null) {
    //            try {
    //                mPfd.close();
    //            } catch (IOException e1) {
    //                Log.e(TAG, "bindListen, close mPfd: " + e1);
    //            }
    //            mPfd = null;
    //        }
    //     Log.e(TAG, "bindListen, fail to get port number, exception: " + e);
    //     return -1;
    // }
    return 0;
}

ECode BluetoothSocket::Accept(
    /* [in] */ Int32 timeout,
    /* [out] */ IBluetoothSocket** socket)
{
    VALIDATE_NOT_NULL(socket)
    *socket = NULL;

    AutoPtr<BluetoothSocket> acceptedSocket;
    if (mSocketState != LISTENING) {
        // throw new IOException("bt socket is not in listen state");
        Slogger::E(TAG, "bt socket is not in listen state");
        return E_IO_EXCEPTION;
    }
    if(timeout > 0) {
        Slogger::D(TAG, "accept() set timeout (ms):%d", timeout);
        mSocket->SetSoTimeout(timeout);
    }
    String remoteAddr;
    WaitSocketSignal(mSocketIS, &remoteAddr);
    if(timeout > 0) mSocket->SetSoTimeout(0);
    AutoLock lock(mLock);
    if (mSocketState != LISTENING) {
        // throw new IOException("bt socket is not in listen state");
        Slogger::E(TAG, "bt socket is not in listen state");
        return E_IO_EXCEPTION;
    }
    FAIL_RETURN(AcceptSocket(remoteAddr, (BluetoothSocket**)&acceptedSocket))
    *socket = (IBluetoothSocket*)acceptedSocket;
    REFCOUNT_ADD(*socket)
    //quick drop the reference of the file handle
    return NOERROR;
}

ECode BluetoothSocket::Available(
    /* [out] */ Int32* result)
{
    VALIDATE_NOT_NULL(result)
    if (VDBG) Slogger::D(TAG, "available: %p", mSocketIS.Get());
    return mSocketIS->Available(result);
}

ECode BluetoothSocket::Flush()
{
    if (mSocketOS == NULL) {
        //throw new IOException("flush is called on null OutputStream");
        Logger::E(TAG, "flush is called on null OutputStream");
        return E_IO_EXCEPTION;
    }
    if (VDBG) Logger::D(TAG, "flush: ");// + mSocketOS);
    IFlushable::Probe(mSocketOS)->Flush();
    return NOERROR;
}

ECode BluetoothSocket::Read(
    /* [in] */ ArrayOf<Byte>* b,
    /* [in] */ Int32 offset,
    /* [in] */ Int32 length,
    /* [out] */ Int32* count)
{
    if (mSocketIS == NULL) {
        //throw new IOException("read is called on null InputStream");
        Logger::E(TAG, "read is called on null InputStream");
        return E_IO_EXCEPTION;
    }
    VALIDATE_NOT_NULL(count)
    *count = 0;

    if (VDBG) Slogger::D(TAG, "read in:  %p len: %d", mSocketIS.Get(), length);
    Int32 ret;
    mSocketIS->Read(b, offset, length, &ret);
    if(ret < 0) {
        // throw new IOException("bt socket closed, read return: " + ret);
        Slogger::E(TAG, "bt socket closed, read return: %d", ret);
        return E_IO_EXCEPTION;
    }
    if (VDBG) Slogger::D(TAG, "read out:  %p ret: %d", mSocketIS.Get(), ret);
    *count = ret;
    return NOERROR;
}

ECode BluetoothSocket::Write(
    /* [in] */ ArrayOf<Byte>* b,
    /* [in] */ Int32 offset,
    /* [in] */ Int32 length,
    /* [out] */ Int32* count)
{
    if (mSocketOS == NULL) {
        //throw new IOException("write is called on null OutputStream");
        Logger::E(TAG, "write is called on null OutputStream");
        return E_IO_EXCEPTION;
    }
    VALIDATE_NOT_NULL(count)
    *count = 0;
    if (VDBG) Slogger::D(TAG, "write: %p length: %d", mSocketOS.Get(), length);
    mSocketOS->Write(b, offset, length);
    // There is no good way to confirm since the entire process is asynchronous anyway
    if (VDBG) Slogger::D(TAG, "write out: %p length: %d", mSocketOS.Get(), length);
    *count = length;
    return NOERROR;
}

ECode BluetoothSocket::Close()
{
    if (DBG) Slogger::D(TAG, "close() in, this: %p, channel: %d, state: %d", this, mPort, mSocketState);
    if(mSocketState == CLOSED) {
        return NOERROR;
    }
    else {
        AutoLock lock(mLock);
        if(mSocketState == CLOSED) return NOERROR;
        mSocketState = CLOSED;
        if (DBG) Slogger::D(TAG, "close() this: %p, channel: %d, mSocketIS: %p, mSocketOS: %p, mSocket: %p", this
                , mPort, mSocketIS.Get(), mSocketOS.Get(), mSocket.Get());
        if(mSocket != NULL) {
            if (DBG) Slogger::D(TAG, "Closing mSocket: %p", mSocket.Get());
            mSocket->ShutdownInput();
            mSocket->ShutdownOutput();
            ICloseable::Probe(mSocket)->Close();
            mSocket = NULL;
        }
        if(mPfd != NULL) {
            Slogger::E(TAG, "function DetachFd() has not been realized!!!!!!!!!!");
            // ((CParcelFileDescriptor*)mPfd.Get())->DetachFd();
            mPfd->Close();
            mPfd = NULL;
        }
    }
    return NOERROR;
}

void BluetoothSocket::RemoveChannel()
{}

Int32 BluetoothSocket::GetPort()
{
    return mPort;
}

String BluetoothSocket::ConvertAddr(
    /* [in] */ ArrayOf<byte>* addr)
{
    String str;
    str.AppendFormat(/*TODO Local.US, */"%02X:%02X:%02X:%02X:%02X:%02X",
            (*addr)[0] , (*addr)[1], (*addr)[2], (*addr)[3] , (*addr)[4], (*addr)[5]);
    return str;
}

ECode BluetoothSocket::WaitSocketSignal(
    /* [in] */ IInputStream* is,
    /* [out] */ String* addr)
{
    VALIDATE_NOT_NULL(addr);
    *addr = String(NULL);

    AutoPtr< ArrayOf<Byte> > sig = ArrayOf<Byte>::Alloc(SOCK_SIGNAL_SIZE);
    Int32 ret;
    ReadAll(is, sig, &ret);
    if (VDBG) Slogger::D(TAG, "waitSocketSignal read 16 bytes signal ret: %d", ret);
    AutoPtr<IByteBufferHelper> bufferHelper;
    CByteBufferHelper::AcquireSingleton((IByteBufferHelper**)&bufferHelper);
    AutoPtr<IByteBuffer> bb;
    bufferHelper->Wrap(sig, (IByteBuffer**)&bb);
    AutoPtr<IByteOrderHelper> helper;
    CByteOrderHelper::AcquireSingleton((IByteOrderHelper**)&helper);
    ByteOrder nativeOrder;
    helper->GetNativeOrder(&nativeOrder);
    bb->SetOrder(nativeOrder);
    Int16 tmp;
    bb->GetInt16(&tmp);
    Int32 size = tmp;
    if(size != SOCK_SIGNAL_SIZE) {
        // throw new IOException("Connection failure, wrong signal size: " + size);
        Slogger::E(TAG, "Connection failure, wrong signal size: %d", size);
        return E_IO_EXCEPTION;
    }

    AutoPtr< ArrayOf<Byte> > addrs = ArrayOf<Byte>::Alloc(6);
    bb->Get(addrs);
    Int32 channel, status;
    bb->GetInt32(&channel);
    bb->GetInt32(&status);
    String remoteAddr = ConvertAddr(addrs);
    if (VDBG) {
        Slogger::D(TAG, "waitSocketSignal: sig size: %d, remote addr: %s, channel: %d, status: %d", size
                , remoteAddr.string(), channel, status);
    }
    if(status != 0) {
        // throw new IOException("Connection failure, status: " + status);
        Slogger::E(TAG, "Connection failure, status: %d", status);
    }

    *addr = remoteAddr;
    return NOERROR;
}

ECode BluetoothSocket::ReadAll(
    /* [in] */ IInputStream* is,
    /* [in] */ ArrayOf<byte>* b,
    /* [out] */ Int32* length)
{
    VALIDATE_NOT_NULL(length)
    *length = 0;

    Int32 left = b->GetLength();
    while(left > 0) {
        Int32 ret;
        is->Read(b, b->GetLength() - left, left, &ret);
        if(ret <= 0) {
            // throw new IOException("read failed, socket might closed or timeout, read ret: " + ret);
            Slogger::E(TAG, "read failed, socket might closed or timeout, read ret: %d", ret);
            return E_IO_EXCEPTION;
        }

        left -= ret;
        if(left != 0)
            Slogger::W(TAG, "readAll() looping, read partial size: %d, expect size: %d", (b->GetLength() - left)
                    , b->GetLength());
    }
    *length = b->GetLength();
    return NOERROR;
}

ECode BluetoothSocket::ReadInt32(
    /* [in] */ IInputStream* is,
    /* [out] */ Int32* length)
{
    VALIDATE_NOT_NULL(length)
    *length = 0;

    AutoPtr< ArrayOf<Byte> > ibytes = ArrayOf<Byte>::Alloc(4);
    Int32 ret;
    FAIL_RETURN(ReadAll(is, ibytes, &ret))
    if (VDBG) Slogger::D(TAG, "inputStream.read ret: %d", ret);
    AutoPtr<IByteBufferHelper> helper;
    CByteBufferHelper::AcquireSingleton((IByteBufferHelper**)&helper);
    AutoPtr<IByteBuffer> bb;
    helper->Wrap(ibytes, (IByteBuffer**)&bb);
    AutoPtr<IByteOrderHelper> orderHelper;
    CByteOrderHelper::AcquireSingleton((IByteOrderHelper**)&orderHelper);
    ByteOrder nativeOrder;
    orderHelper->GetNativeOrder(&nativeOrder);
    bb->SetOrder(nativeOrder);
    return bb->GetInt32(length);
}

/**
 * setSocketOpt for the Buetooth Socket.
 *
 * @param optionName socket option name
 * @param optionVal  socket option value
 * @param optionLen  socket option length
 * @return -1 on immediate error,
 *               0 otherwise
 * @hide
 */
ECode BluetoothSocket::SetSocketOpt(
    /* [in] */ Int32 optionName,
    /* [in] */ ArrayOf<Byte>* optionVal,
    /* [in] */ Int32 optionLen,
    /* [out] */ Int32* result)
{
    *result = 0;
    if (mSocketState == CLOSED) {
        SLOGGERE(TAG, "socket closed")
        return E_IO_EXCEPTION;
    }
    AutoPtr<IBluetoothAdapter> apt = CBluetoothAdapter::GetDefaultAdapter();
    AutoPtr<CBluetoothAdapter> aptImpl = (CBluetoothAdapter*)apt.Get();
    AutoPtr<IIBluetooth> bluetoothProxy = aptImpl->GetBluetoothService(NULL);;
    if (bluetoothProxy == NULL) {
        SLOGGERD(TAG, "setSocketOpt fail, reason: bluetooth is off")
        *result = -1;
        return NOERROR;
    }
    // try {
        if(VDBG) SLOGGERD(TAG, "setSocketOpt(), mType: %d, mPort: %d", mType, mPort)
        if (FAILED(bluetoothProxy->SetSocketOpt(mType, mPort, optionName, optionVal, optionLen, result)))
        {
            SLOGGERE(TAG, "RemoteException: IIBluetooth bluetoothProxy->SetSocketOpt Error")
            *result = -1;
            return NOERROR;
        }
    // } catch (RemoteException e) {
    // }
    return NOERROR;
}

/**
 * getSocketOpt for the Buetooth Socket.
 *
 * @param optionName socket option name
 * @param optionVal  socket option value
 * @return -1 on immediate error,
 *               length of returned socket option otherwise
 * @hide
 */
ECode BluetoothSocket::GetSocketOpt(
    /* [in] */ Int32 optionName,
    /* [in] */ ArrayOf<Byte>* optionVal,
    /* [out] */ Int32* result)
{
    *result = 0;
    if (mSocketState == CLOSED) {
        SLOGGERE(TAG, "socket closed")
        return E_IO_EXCEPTION;
    }
    AutoPtr<IBluetoothAdapter> apt = CBluetoothAdapter::GetDefaultAdapter();
    AutoPtr<CBluetoothAdapter> aptImpl = (CBluetoothAdapter*)apt.Get();
    AutoPtr<IIBluetooth> bluetoothProxy = aptImpl->GetBluetoothService(NULL);;
    if (bluetoothProxy == NULL) {
        SLOGGERD(TAG, "setSocketOpt fail, reason: bluetooth is off")
        *result = -1;
        return NOERROR;
    }
    // try {
        if(VDBG) SLOGGERD(TAG, "GetSocketOpt(), mType: %d, mPort: %d", mType, mPort)
        if (FAILED(bluetoothProxy->GetSocketOpt(mType, mPort, optionName, optionVal, result)))
        {
            SLOGGERE(TAG, "RemoteException: IIBluetooth bluetoothProxy->GetSocketOpt Error")
            *result = -1;
            return NOERROR;
        }
    // } catch (RemoteException e) {
    // }
    return NOERROR;
}

} // Bluetooth
} // Droid
} // Elastos

