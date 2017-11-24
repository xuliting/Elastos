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

#include "Elastos.CoreLibrary.IO.h"
#include "CConcurrentHashMap.h"
#include "LockSupport.h"
#include "Math.h"
#include "StringBuilder.h"
#include "CBoolean.h"
#include "CAtomicInteger32.h"
#include "Arrays.h"
#include "corelibrary/Atomic.h"
#include "elastos/core/CArrayOf.h"
#include <elastos/core/AutoLock.h>
#include <elastos/core/Thread.h>
#include <cutils/atomic.h>
#include <cutils/atomic-inline.h>

using Elastos::Core::AutoLock;
using Elastos::Core::IBoolean;
using Elastos::Core::CBoolean;
using Elastos::Core::CArrayOf;
using Elastos::Core::IComparable;
using Elastos::Core::IString;
using Elastos::Core::EIID_IString;
using Elastos::Core::StringBuilder;
using Elastos::Core::Thread;
using Elastos::IO::IObjectInput;
using Elastos::IO::IObjectOutput;
using Elastos::IO::IOutputStream;
using Elastos::IO::IObjectOutputStreamPutField;
using Elastos::IO::EIID_ISerializable;
using Elastos::Utility::Concurrent::Atomic::CAtomicInteger32;
using Elastos::Utility::Concurrent::Locks::LockSupport;
using Elastos::Utility::Concurrent::Locks::EIID_IReentrantLock;

namespace Elastos {
namespace Utility {
namespace Concurrent {

static Boolean CompareAndSwapInt32(volatile int32_t* address, Int32 expect, Int32 update)
{
    // Note: android_atomic_release_cas() returns 0 on success, not failure.
    int ret = android_atomic_release_cas(expect, update, address);

    return (ret == 0);
}

static Boolean CompareAndSwapInt64(volatile int64_t* address, Int64 expect, Int64 update)
{
    // Note: android_atomic_cmpxchg() returns 0 on success, not failure.
    int ret = QuasiAtomicCas64(expect, update, address);

    return (ret == 0);
}

static Boolean CompareAndSwapObject(volatile int32_t* address, IInterface* expect, IInterface* update)
{
    // Note: android_atomic_cmpxchg() returns 0 on success, not failure.
    int ret = android_atomic_release_cas((int32_t)expect,
            (int32_t)update, address);
    if (ret == 0) {
        REFCOUNT_ADD(update)
        REFCOUNT_RELEASE(expect)
    }
    return (ret == 0);
}

static void PutOrderedInt32(volatile int32_t* address, Int32 newValue)
{
    ANDROID_MEMBAR_STORE();
    *address = newValue;
}

static void PutOrderedObject(volatile int32_t* address, IInterface* newValue)
{
    ANDROID_MEMBAR_STORE();
    IInterface* oldValue = reinterpret_cast<IInterface*>(*address);
    *address = reinterpret_cast<int32_t>(newValue);
    REFCOUNT_ADD(newValue);
    REFCOUNT_RELEASE(oldValue);
}

//===============================================================================
// CConcurrentHashMap::Node::
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::Node, Object, IMapEntry)

CConcurrentHashMap::Node::Node(
    /* [in] */ Int32 hash,
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* val,
    /* [in] */ Node* next)
    : mHash(hash)
    , mKey(key)
    , mVal(val)
    , mNext(next)
{}

ECode CConcurrentHashMap::Node::GetKey(
    /* [out] */ IInterface** key)
{
    VALIDATE_NOT_NULL(key);
    *key = mKey;
    REFCOUNT_ADD(*key);
    return NOERROR;
}

ECode CConcurrentHashMap::Node::GetValue(
    /* [out] */ IInterface** value)
{
    VALIDATE_NOT_NULL(value);
    *value = mVal;
    REFCOUNT_ADD(*value);
    return NOERROR;
}

ECode CConcurrentHashMap::Node::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode);
    Int32 hck = Object::GetHashCode(mKey);
    Int32 hcv = Object::GetHashCode(mVal.Get());
    *hashCode = hck ^ hcv;
    return NOERROR;
}

ECode CConcurrentHashMap::Node::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)

    *str = Object::ToString(mKey) + String("=") + Object::ToString(mVal.Get());
    return NOERROR;
}

ECode CConcurrentHashMap::Node::SetValue(
    /* [in] */ IInterface* valueReplacer,
    /* [out] */ IInterface** valueReplacee)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CConcurrentHashMap::Node::Equals(
    /* [in] */ IInterface* entry,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    AutoPtr<IInterface> k, v, u; AutoPtr<IMapEntry> e;
    *result = ((IMapEntry::Probe(entry) != NULL) &&
            (((e = IMapEntry::Probe(entry))->GetKey((IInterface**)&k)), k != NULL) &&
            ((e->GetValue((IInterface**)&v)), v != NULL) &&
            (k == mKey || Object::Equals(k, mKey)) &&
            (v == (u = mVal) || Object::Equals(v, u)));
    return NOERROR;
}

AutoPtr<CConcurrentHashMap::Node> CConcurrentHashMap::Node::Find(
    /* [in] */ Int32 h,
    /* [in] */ IInterface* k)
{
    AutoPtr<Node> e = this;
    if (k != NULL) {
        do {
            AutoPtr<IInterface> ek;
            if (e->mHash == h &&
                ((ek = e->mKey).Get() == k || (ek != NULL && Object::Equals(k, ek)))) {
                return e;
            }
        } while ((e = e->mNext) != NULL);
    }
    return NULL;
}


CAR_INTERFACE_IMPL(CConcurrentHashMap::ReservationNode, CConcurrentHashMap::Node, IReservationNode)

//===============================================================================
// CConcurrentHashMap::
//===============================================================================

Int32 CConcurrentHashMap::Spread(
    /* [in] */ Int32 h)
{
    return (h ^ (h >> 16)) & HASH_BITS;
}

Int32 CConcurrentHashMap::TableSizeFor(
    /* [in] */ Int32 c)
{
    Int32 n = c - 1;
    n |= ((UInt32)n) >> 1;
    n |= ((UInt32)n) >> 2;
    n |= ((UInt32)n) >> 4;
    n |= ((UInt32)n) >> 8;
    n |= ((UInt32)n) >> 16;
    return (n < 0) ? 1 : (n >= MAXIMUM_CAPACITY) ? MAXIMUM_CAPACITY : n + 1;
}

InterfaceID CConcurrentHashMap::ComparableClassFor(
    /* [in] */ IInterface* x)
{
    if (IComparable::Probe(x) != NULL) {
        if (IString::Probe(x) != NULL) { // bypass checks
            return EIID_IString;
        }
        if (IComparable::Probe(x) != NULL) {
            return EIID_IComparable;
        }
    }
    return EMUID_NULL;
}

Int32 CConcurrentHashMap::CompareComparables(
    /* [in] */ const InterfaceID& kc,
    /* [in] */ IInterface* k,
    /* [in] */ IInterface* x)
{
    Int32 res;
    return (x == NULL || (x->Probe(kc) == NULL) ? 0 :
            (IComparable::Probe(k)->CompareTo(x, &res), res));
}

AutoPtr<CConcurrentHashMap::Node> CConcurrentHashMap::TabAt(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 i)
{
    volatile int32_t* address =
            (volatile int32_t*)(tab->GetPayload() + i);
    int32_t value = android_atomic_acquire_load(address);
    return (Node*)reinterpret_cast<IMapEntry*>(value);
}

Boolean CConcurrentHashMap::CasTabAt(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 i,
    /* [in] */ Node* c,
    /* [in] */ Node* v)
{
    return CompareAndSwapObject((volatile int32_t*)(tab->GetPayload() + i),
            (IMapEntry*)c, (IMapEntry*)v);
}

void CConcurrentHashMap::SetTabAt(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 i,
    /* [in] */ Node* v)
{
    PutOrderedObject((volatile int32_t*)(tab->GetPayload() + i), (IMapEntry*)v);
}

ECode CConcurrentHashMap::PutVal(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* value,
    /* [in] */ Boolean onlyIfAbsent,
    /* [out] */ IInterface** oldValue)
{
    if (key == NULL || value == NULL) {
        *oldValue = NULL;
        return E_NULL_POINTER_EXCEPTION;
    }
    Int32 hc = Object::GetHashCode(key);
    Int32 hash = Spread(hc);
    Int32 binCount = 0;
    AutoPtr<ArrayOf<Node*> > tab = mTable;
    for (;;) {
        AutoPtr<Node> f; Int32 n, i, fh;
        if (tab == NULL || (n = tab->GetLength()) == 0) {
            tab = InitTable();
        }
        else if ((f = TabAt(tab, i = (n - 1) & hash)) == NULL) {
            AutoPtr<Node> node = new Node(hash, key, value, NULL);
            if (CasTabAt(tab, i, NULL, node)) {
                break;                   // no lock when adding to empty bin
            }
        }
        else if ((fh = f->mHash) == MOVED) {
            tab = HelpTransfer(tab, f);
        }
        else {
            AutoPtr<IInterface> oldVal;
            {
                AutoLock lock(f);
                if (TabAt(tab, i) == f) {
                    if (fh >= 0) {
                        binCount = 1;
                        for (AutoPtr<Node> e = f;; ++binCount) {
                            AutoPtr<IInterface> ek;
                            if (e->mHash == hash &&
                                ((ek = e->mKey).Get() == key ||
                                 (ek != NULL && Object::Equals(key, ek)))) {
                                oldVal = e->mVal;
                                if (!onlyIfAbsent) {
                                    e->mVal = value;
                                }
                                break;
                            }
                            AutoPtr<Node> pred = e;
                            if ((e = e->mNext) == NULL) {
                                pred->mNext = new Node(hash, key, value, NULL);
                                break;
                            }
                        }
                    }
                    else if (ITreeBin::Probe(f) != NULL) {
                        AutoPtr<Node> p;
                        binCount = 2;
                        AutoPtr<TreeBin> tb = (TreeBin*)ITreeBin::Probe(f);
                        if ((p = tb->PutTreeVal(hash, key,
                                                       value)) != NULL) {
                            oldVal = p->mVal;
                            if (!onlyIfAbsent) {
                                p->mVal = value;
                            }
                        }
                    }
                }
            }
            if (binCount != 0) {
                if (binCount >= TREEIFY_THRESHOLD) {
                    TreeifyBin(tab, i);
                }
                if (oldVal != NULL) {
                    *oldValue = oldVal;
                    REFCOUNT_ADD(*oldValue);
                    return NOERROR;
                }
                break;
            }
        }
    }
    AddCount(1LL, binCount);
    *oldValue = NULL;
    return NOERROR;
}

AutoPtr<IInterface> CConcurrentHashMap::ReplaceNode(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* value,
    /* [in] */ IInterface* cv)
{
    Int32 hc = Object::GetHashCode(key);
    Int32 hash = Spread(hc);
    for (AutoPtr<ArrayOf<Node*> > tab = mTable;;) {
        AutoPtr<Node> f; Int32 n, i, fh;
        if (tab == NULL || (n = tab->GetLength()) == 0 ||
            (f = TabAt(tab, i = (n - 1) & hash)) == NULL) {
            break;
        }
        else if ((fh = f->mHash) == MOVED) {
            tab = HelpTransfer(tab, f);
        }
        else {
            AutoPtr<IInterface> oldVal;
            Boolean validated = FALSE;
            {
                AutoLock lock(f);
                if (TabAt(tab, i) == f) {
                    if (fh >= 0) {
                        validated = TRUE;
                        for (AutoPtr<Node> e = f, pred = NULL;;) {
                            AutoPtr<IInterface> ek;
                            if (e->mHash == hash &&
                                ((ek = e->mKey).Get() == key ||
                                 (ek != NULL && Object::Equals(key, ek)))) {
                                AutoPtr<IInterface> ev = e->mVal;
                                if (cv == NULL || cv == ev ||
                                    (ev != NULL && Object::Equals(cv, ev))) {
                                    oldVal = ev;
                                    if (value != NULL) {
                                        e->mVal = value;
                                    }
                                    else if (pred != NULL) {
                                        pred->mNext = e->mNext;
                                    }
                                    else {
                                        SetTabAt(tab, i, e->mNext);
                                    }
                                }
                                break;
                            }
                            pred = e;
                            if ((e = e->mNext) == NULL) {
                                break;
                            }
                        }
                    }
                    else if (ITreeBin::Probe(f) != NULL) {
                        validated = TRUE;
                        AutoPtr<TreeBin> t = (TreeBin*)ITreeBin::Probe(f);
                        AutoPtr<TreeNode> r, p;
                        if ((r = t->mRoot) != NULL &&
                            (p = r->FindTreeNode(hash, key, EMUID_NULL)) != NULL) {
                            AutoPtr<IInterface> pv = p->mVal;
                            if (cv == NULL || cv == pv ||
                                (pv != NULL && Object::Equals(cv, pv))) {
                                oldVal = pv;
                                if (value != NULL) {
                                    p->mVal = value;
                                }
                                else if (t->RemoveTreeNode(p)) {
                                    SetTabAt(tab, i, Untreeify(t->mFirst.Get()).Get());
                                }
                            }
                        }
                    }
                }
            }
            if (validated) {
                if (oldVal != NULL) {
                    if (value == NULL) {
                        AddCount(-1LL, -1);
                    }
                    return oldVal;
                }
                break;
            }
        }
    }
    return NULL;
}

String CConcurrentHashMap::ToString()
{
    AutoPtr<ArrayOf<Node*> > t;
    Int32 f = (t = mTable) == NULL ? 0 : t->GetLength();
    AutoPtr<Traverser> it = new Traverser(t, f, 0, f);
    StringBuilder sb;
    sb.AppendChar('{');
    AutoPtr<Node> p;
    if ((p = it->Advance()) != NULL) {
        for (;;) {
            AutoPtr<IInterface> k = p->mKey;
            AutoPtr<IInterface> v = p->mVal;
            if (Object::Equals(k, TO_IINTERFACE(this))) {
                sb.Append("(this Map)");
            }
            else {
                sb.Append(k);
            }
            sb.AppendChar('=');
            if (Object::Equals(v, TO_IINTERFACE(this))) {
                sb.Append("(this Map)");
            }
            else {
                sb.Append(v);
            }
            if ((p = it->Advance()) == NULL) {
                break;
            }
            sb.AppendChar(',');
            sb.AppendChar(' ');
        }
    }
    sb.AppendChar('}');
    return sb.ToString();
}

Int64 CConcurrentHashMap::MappingCount()
{
    Int64 n = SumCount();
    return (n < 0LL) ? 0LL : n; // ignore transient negative values
}

AutoPtr<CConcurrentHashMap::KeySetView> CConcurrentHashMap::NewKeySet()
{
    AutoPtr<IBoolean> b;
    CBoolean::New(TRUE, (IBoolean**)&b);
    AutoPtr<CConcurrentHashMap> m;
    CConcurrentHashMap::NewByFriend((CConcurrentHashMap**)&m);
    AutoPtr<KeySetView> res = new KeySetView(m, b);
    return res;
}

AutoPtr<CConcurrentHashMap::KeySetView> CConcurrentHashMap::NewKeySet(
    /* [in] */ Int32 initialCapacity)
{
    AutoPtr<IBoolean> b;
    CBoolean::New(TRUE, (IBoolean**)&b);
    AutoPtr<CConcurrentHashMap> m;
    CConcurrentHashMap::NewByFriend(initialCapacity, (CConcurrentHashMap**)&m);
    AutoPtr<KeySetView> res = new KeySetView(m, b);
    return res;
}

ECode CConcurrentHashMap::KeySet(
    /* [in] */ IInterface* mappedValue,
    /* [out] */ ISet** set)
{
    if (mappedValue == NULL) {
        *set = NULL;
        return E_NULL_POINTER_EXCEPTION;
    }
    *set = new KeySetView(this, mappedValue);
    REFCOUNT_ADD(*set);
    return NOERROR;
}

//===============================================================================
// CConcurrentHashMap::ForwardingNode::
//===============================================================================

CAR_INTERFACE_IMPL(CConcurrentHashMap::ForwardingNode, CConcurrentHashMap::Node, IForwardingNode)

CConcurrentHashMap::ForwardingNode::ForwardingNode(
    /* [in] */ ArrayOf<Node*>* tab)
    : Node(MOVED, NULL, NULL, NULL)
    , mNextTable(tab)
{}

AutoPtr<CConcurrentHashMap::Node> CConcurrentHashMap::ForwardingNode::Find(
    /* [in] */ Int32 h,
    /* [in] */ IInterface* k)
{
    AutoPtr<Node> e; Int32 n;
    AutoPtr<ArrayOf<Node*> > tab = mNextTable;
    if (k != NULL && tab != NULL && (n = tab->GetLength()) > 0 &&
        (e = TabAt(tab, (n - 1) & h)) != NULL) {
        do {
            Int32 eh; AutoPtr<IInterface> ek;
            if ((eh = e->mHash) == h &&
                ((ek = e->mKey).Get() == k || (ek != NULL && Object::Equals(k, ek)))) {
                return e;
            }
            if (eh < 0) {
                return e->Find(h, k);
            }
        } while ((e = e->mNext) != NULL);
    }
    return NULL;
}

//===============================================================================
// CConcurrentHashMap::
//===============================================================================

AutoPtr<ArrayOf<CConcurrentHashMap::Node*> > CConcurrentHashMap::InitTable()
{
    AutoPtr<ArrayOf<Node*> > tab; Int32 sc;
    while ((tab = mTable) == NULL || tab->GetLength() == 0) {
        if ((sc = mSizeCtl) < 0) {
            Thread::Yield(); // lost initialization race; just spin
        }
        else if (CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, sc, -1)) {
            if ((tab = mTable) == NULL || tab->GetLength() == 0) {
                Int32 n = (sc > 0) ? sc : DEFAULT_CAPACITY;
                AutoPtr<ArrayOf<Node*> > nt = ArrayOf<Node*>::Alloc(n);
                mTable = tab = nt;
                sc = n - (((UInt32)n) >> 2);
            }
            mSizeCtl = sc;
            break;
        }
    }
    return tab;
}

void CConcurrentHashMap::AddCount(
    /* [in] */ Int64 x,
    /* [in] */ Int32 check)
{
    AutoPtr<ArrayOf<CounterCell*> > as; Int64 b, s;
    if ((as = mCounterCells) != NULL || (b = mBaseCount, s = b + x,
        !CompareAndSwapInt64((volatile int64_t*)&mBaseCount, b, s))) {
        AutoPtr<CounterHashCode> hc; AutoPtr<CounterCell> a; Int64 v; Int32 m;
        Boolean uncontended = TRUE;
        if ((hc = (CounterHashCode*)pthread_getspecific(sThreadCounterHashCode)) == NULL ||
            as == NULL || (m = as->GetLength() - 1) < 0 ||
            (a = (*as)[m & hc->mCode]) == NULL ||
            (v = a->mValue, !(uncontended =
                CompareAndSwapInt64((volatile int64_t*)&a->mValue, v, v + x)))) {
            FullAddCount(x, hc, uncontended);
            return;
        }
        if (check <= 1) {
            return;
        }
        s = SumCount();
    }
    if (check >= 0) {
        AutoPtr<ArrayOf<Node*> > tab, nt; Int32 sc;
        while (s >= (Int64)(sc = mSizeCtl) && (tab = mTable) != NULL &&
               tab->GetLength() < MAXIMUM_CAPACITY) {
            if (sc < 0) {
                if (sc == -1 || mTransferIndex <= mTransferOrigin ||
                    (nt = mNextTable) == NULL) {
                    break;
                }
                if (CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, sc, sc - 1)) {
                    Transfer(tab, nt);
                }
            }
            else if (CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, sc, -2)) {
                Transfer(tab, NULL);
            }
            s = SumCount();
        }
    }
}

AutoPtr<ArrayOf<CConcurrentHashMap::Node*> > CConcurrentHashMap::HelpTransfer(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Node* f)
{
    AutoPtr<ArrayOf<Node*> > nextTab; Int32 sc;
    if ((IForwardingNode::Probe(f) != NULL) &&
        (nextTab = ((ForwardingNode*)IForwardingNode::Probe(f))->mNextTable) != NULL) {
        if (nextTab == mNextTable && tab == mTable &&
            mTransferIndex > mTransferOrigin && (sc = mSizeCtl) < -1 &&
            CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, sc, sc - 1)) {
            Transfer(tab, nextTab);
        }
        return nextTab;
    }
    return mTable;
}

void CConcurrentHashMap::TryPresize(
    /* [in] */ Int32 size)
{
    Int32 c = (size >= (((UInt32)MAXIMUM_CAPACITY) >> 1)) ? MAXIMUM_CAPACITY :
            TableSizeFor(size + (((UInt32)size) >> 1) + 1);
    Int32 sc;
    while ((sc = mSizeCtl) >= 0) {
        AutoPtr<ArrayOf<Node*> > tab = mTable;
        Int32 n;
        if (tab == NULL || (n = tab->GetLength()) == 0) {
            n = (sc > c) ? sc : c;
            if (CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, sc, -1)) {
                if (mTable == tab) {
                    AutoPtr<ArrayOf<Node*> > nt = ArrayOf<Node*>::Alloc(n);
                    mTable = nt;
                    sc = n - (((UInt32)n) >> 2);
                }
                mSizeCtl = sc;
            }
        }
        else if (c <= sc || n >= MAXIMUM_CAPACITY) {
            break;
        }
        else if (tab == mTable &&
                CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, sc, -2)) {
            Transfer(tab, NULL);
        }
    }
}

void CConcurrentHashMap::Transfer(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ ArrayOf<Node*>* nextTab)
{
    Int32 n = tab->GetLength(), stride;
    if ((stride = (NCPU > 1) ? (((UInt32)n) >> 3) / NCPU : n) < MIN_TRANSFER_STRIDE) {
        stride = MIN_TRANSFER_STRIDE; // subdivide range
    }
    if (nextTab == NULL) {            // initiating
        // try {
        AutoPtr<ArrayOf<Node*> > nt = ArrayOf<Node*>::Alloc(n << 1);
        nextTab = nt;
        // } catch (Throwable ex) {      // try to cope with OOME
        //     mSizeCtl = Integer.MAX_VALUE;
        //     return;
        // }
        mNextTable = nextTab;
        mTransferOrigin = n;
        mTransferIndex = n;
        AutoPtr<ForwardingNode> rev = new ForwardingNode(tab);
        for (Int32 k = n; k > 0;) {    // progressively reveal ready slots
            Int32 nextk = (k > stride) ? k - stride : 0;
            for (Int32 m = nextk; m < k; ++m) {
                nextTab->Set(m, rev);
            }
            for (Int32 m = n + nextk; m < n + k; ++m) {
                nextTab->Set(m, rev);
            }
            PutOrderedInt32((volatile int32_t*)&mTransferOrigin, k = nextk);
        }
    }
    Int32 nextn = nextTab->GetLength();
    AutoPtr<ForwardingNode> fwd = new ForwardingNode(nextTab);
    Boolean advance = TRUE;
    for (Int32 i = 0, bound = 0;;) {
        Int32 nextIndex, nextBound, fh; AutoPtr<Node> f;
        while (advance) {
            if (--i >= bound) {
                advance = FALSE;
            }
            else if ((nextIndex = mTransferIndex) <= mTransferOrigin) {
                i = -1;
                advance = FALSE;
            }
            else if (CompareAndSwapInt32((volatile int32_t*)&mTransferIndex,
                    nextIndex, nextBound = (nextIndex > stride ?
                                            nextIndex - stride : 0))) {
                bound = nextBound;
                i = nextIndex - 1;
                advance = FALSE;
            }
        }
        if (i < 0 || i >= n || i + n >= nextn) {
            for (Int32 sc;;) {
                if (sc = mSizeCtl, CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, mSizeCtl, ++sc)) {
                    if (sc == -1) {
                        mNextTable = NULL;
                        mTable = nextTab;
                        mSizeCtl = (n << 1) - (((UInt32)n) >> 1);
                    }
                    return;
                }
            }
        }
        else if ((f = TabAt(tab, i)) == NULL) {
            if (CasTabAt(tab, i, NULL, fwd)) {
                SetTabAt(nextTab, i, NULL);
                SetTabAt(nextTab, i + n, NULL);
                advance = TRUE;
            }
        }
        else if ((fh = f->mHash) == MOVED) {
            advance = TRUE; // already processed
        }
        else {
            {
                AutoLock syncLock(f);
                if (TabAt(tab, i) == f) {
                    AutoPtr<Node> ln, hn;
                    if (fh >= 0) {
                        Int32 runBit = fh & n;
                        AutoPtr<Node> lastRun = f;
                        for (AutoPtr<Node> p = f->mNext; p != NULL; p = p->mNext) {
                            Int32 b = p->mHash & n;
                            if (b != runBit) {
                                runBit = b;
                                lastRun = p;
                            }
                        }
                        if (runBit == 0) {
                            ln = lastRun;
                            hn = NULL;
                        }
                        else {
                            hn = lastRun;
                            ln = NULL;
                        }
                        for (AutoPtr<Node> p = f; p != lastRun; p = p->mNext) {
                            Int32 ph = p->mHash;
                            AutoPtr<IInterface> pk = p->mKey;
                            AutoPtr<IInterface> pv = p->mVal;
                            if ((ph & n) == 0) {
                                ln = new Node(ph, pk, pv, ln);
                            }
                            else {
                                hn = new Node(ph, pk, pv, hn);
                            }
                        }
                    }
                    else if (ITreeBin::Probe(f) != NULL) {
                        AutoPtr<TreeBin> t = (TreeBin*)ITreeBin::Probe(f);
                        AutoPtr<TreeNode> lo, loTail;
                        AutoPtr<TreeNode> hi, hiTail;
                        Int32 lc = 0, hc = 0;
                        for (AutoPtr<Node> e = t->mFirst; e != NULL; e = e->mNext) {
                            Int32 h = e->mHash;
                            AutoPtr<TreeNode> p = new TreeNode(h, e->mKey, e->mVal, NULL, NULL);
                            if ((h & n) == 0) {
                                if ((p->mPrev = loTail) == NULL) {
                                    lo = p;
                                }
                                else {
                                    loTail->mNext = p;
                                }
                                loTail = p;
                                ++lc;
                            }
                            else {
                                if ((p->mPrev = hiTail) == NULL) {
                                    hi = p;
                                }
                                else {
                                    hiTail->mNext = p;
                                }
                                hiTail = p;
                                ++hc;
                            }
                        }
                        ln = (lc <= UNTREEIFY_THRESHOLD) ? Untreeify(lo) :
                            (hc != 0) ? new TreeBin(lo) : t.Get();
                        hn = (hc <= UNTREEIFY_THRESHOLD) ? Untreeify(hi) :
                            (lc != 0) ? new TreeBin(hi) : t.Get();
                    }
                    else {
                        ln = hn = NULL;
                    }
                    SetTabAt(nextTab, i, ln);
                    SetTabAt(nextTab, i + n, hn);
                    SetTabAt(tab, i, fwd);
                    advance = TRUE;
                }
            }
        }
    }
}

void CConcurrentHashMap::TreeifyBin(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 index)
{
    AutoPtr<Node> b; Int32 n, sc;
    if (tab != NULL) {
        if ((n = tab->GetLength()) < MIN_TREEIFY_CAPACITY) {
            if (tab == mTable && (sc = mSizeCtl) >= 0 &&
                CompareAndSwapInt32((volatile int32_t*)&mSizeCtl, sc, -2)) {
                Transfer(tab, NULL);
            }
        }
        else if ((b = TabAt(tab, index)) != NULL) {
            {
                AutoLock syncLock(b);
                if (TabAt(tab, index) == b) {
                    AutoPtr<TreeNode> hd, tl;
                    for (AutoPtr<Node> e = b; e != NULL; e = e->mNext) {
                        AutoPtr<TreeNode> p =
                                new TreeNode(e->mHash, e->mKey, e->mVal, NULL, NULL);
                        if ((p->mPrev = tl) == NULL) {
                            hd = p;
                        }
                        else {
                            tl->mNext = p;
                        }
                        tl = p;
                    }
                    AutoPtr<Node> n = new TreeBin(hd);
                    SetTabAt(tab, index, n);
                }
            }
        }
    }
}

AutoPtr<CConcurrentHashMap::Node> CConcurrentHashMap::Untreeify(
    /* [in] */ Node* b)
{
    AutoPtr<Node> hd, tl;
    for (AutoPtr<Node> q = b; q != NULL; q = q->mNext) {
        AutoPtr<Node> p = new Node(q->mHash, q->mKey, q->mVal, NULL);
        if (tl == NULL) {
            hd = p;
        }
        else {
            tl->mNext = p;
        }
        tl = p;
    }
    return hd;
}

//===============================================================================
// CConcurrentHashMap::TreeNode::
//===============================================================================

CAR_INTERFACE_IMPL(CConcurrentHashMap::TreeNode, CConcurrentHashMap::Node, ITreeNode)

CConcurrentHashMap::TreeNode::TreeNode(
    /* [in] */ Int32 hash,
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* val,
    /* [in] */ Node* next,
    /* [in] */ TreeNode* parent)
    : Node(hash, key, val, next)
    , mParent(parent)
{}

AutoPtr<CConcurrentHashMap::Node> CConcurrentHashMap::TreeNode::Find(
    /* [in] */ Int32 h,
    /* [in] */ IInterface* k)
{
    return FindTreeNode(h, k, EMUID_NULL);
}

AutoPtr<CConcurrentHashMap::TreeNode> CConcurrentHashMap::TreeNode::FindTreeNode(
    /* [in] */ Int32 h,
    /* [in] */ IInterface* k,
    /* [in] */ InterfaceID kc)
{
    if (k != NULL) {
        AutoPtr<TreeNode> p = this;
        do {
            Int32 ph, dir; AutoPtr<IInterface> pk; AutoPtr<TreeNode> q;
            AutoPtr<TreeNode> pl = p->mLeft, pr = p->mRight;
            if ((ph = p->mHash) > h) {
                p = pl;
            }
            else if (ph < h) {
                p = pr;
            }
            else if (((pk = p->mKey), pk.Get() == k) || (pk != NULL && Object::Equals(k, pk))) {
                return p;
            }
            else if (pl == NULL && pr == NULL) {
                break;
            }
            else if ((kc != EMUID_NULL ||
                    (kc = ComparableClassFor(k)) != EMUID_NULL) &&
                    (dir = CompareComparables(kc, k, pk)) != 0) {
                p = (dir < 0) ? pl : pr;
            }
            else if (pl == NULL) {
                p = pr;
            }
            else if (pr == NULL ||
                    (q = pr->FindTreeNode(h, k, kc)) == NULL) {
                p = pl;
            }
            else {
                return q;
            }
        } while (p != NULL);
    }
    return NULL;
}

//===============================================================================
// CConcurrentHashMap::TreeBin::
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::TreeBin, CConcurrentHashMap::Node, ITreeBin)

Int32 CConcurrentHashMap::TreeBin::WRITER = 1; // set while holding write lock
Int32 CConcurrentHashMap::TreeBin::WAITER = 2; // set when waiting for write lock
Int32 CConcurrentHashMap::TreeBin::READER = 4; // increment value for setting read lock

CConcurrentHashMap::TreeBin::TreeBin(
    /* [in] */ TreeNode* b)
    : Node(TREEBIN, NULL, NULL, NULL)
{
    mFirst = b;
    AutoPtr<TreeNode> r;
    for (AutoPtr<TreeNode> x = b, next; x != NULL; x = next) {
        next = (TreeNode*)ITreeNode::Probe(x->mNext);
        x->mLeft = x->mRight = NULL;
        if (r == NULL) {
            x->mParent = NULL;
            x->mRed = FALSE;
            r = x;
        }
        else {
            AutoPtr<IInterface> key = x->mKey;
            Int32 hash = x->mHash;
            InterfaceID kc;
            for (AutoPtr<TreeNode> p = r;;) {
                Int32 dir, ph;
                if ((ph = p->mHash) > hash) {
                    dir = -1;
                }
                else if (ph < hash) {
                    dir = 1;
                }
                else if ((kc != EMUID_NULL ||
                          (kc = ComparableClassFor(key)) != EMUID_NULL)) {
                    dir = CompareComparables(kc, key, p->mKey);
                }
                else {
                    dir = 0;
                }
                AutoPtr<TreeNode> xp = p;
                if ((p = (dir <= 0) ? p->mLeft : p->mRight) == NULL) {
                    x->mParent = xp;
                    if (dir <= 0) {
                        xp->mLeft = x;
                    }
                    else {
                        xp->mRight = x;
                    }
                    r = BalanceInsertion(r, x);
                    break;
                }
            }
        }
    }
    mRoot = r;
}

void CConcurrentHashMap::TreeBin::LockRoot()
{
    if (!CompareAndSwapInt32((volatile int32_t*)&mLockState, 0, WRITER)) {
        ContendedLock(); // offload to separate method
    }
}

void CConcurrentHashMap::TreeBin::UnlockRoot()
{
    mLockState = 0;
}

void CConcurrentHashMap::TreeBin::ContendedLock()
{
    Boolean waiting = FALSE;
    for (Int32 s;;) {
        if (((s = mLockState) & WRITER) == 0) {
            if (CompareAndSwapInt32((volatile int32_t*)&mLockState, s, WRITER)) {
                if (waiting) {
                    mWaiter = NULL;
                }
                return;
            }
        }
        else if ((s & WAITER) == 0) {
            if (CompareAndSwapInt32((volatile int32_t*)&mLockState, s, s | WAITER)) {
                waiting = TRUE;
                mWaiter = Thread::GetCurrentThread();
            }
        }
        else if (waiting) {
            LockSupport::Park((ITreeBin*)this);
        }
    }
}

AutoPtr<CConcurrentHashMap::Node> CConcurrentHashMap::TreeBin::Find(
    /* [in] */ Int32 h,
    /* [in] */ IInterface* k)
{
    if (k != NULL) {
        for (AutoPtr<Node> e = mFirst; e != NULL; e = e->mNext) {
            Int32 s; AutoPtr<IInterface> ek;
            if (((s = mLockState) & (WAITER|WRITER)) != 0) {
                if (e->mHash == h &&
                    (((ek = e->mKey), ek.Get() == k) || (ek != NULL && Object::Equals(k, ek)))) {
                    return e;
                }
            }
            else if (CompareAndSwapInt32((volatile int32_t*)&mLockState, s,
                                        s + READER)) {
                AutoPtr<TreeNode> r, p;
                p = ((r = mRoot) == NULL ? NULL :
                        r->FindTreeNode(h, k, EMUID_NULL));

                AutoPtr<IThread> w;
                Int32 ls;
                do {} while (ls = mLockState, !CompareAndSwapInt32((volatile int32_t*)&mLockState,
                            ls, ls - READER));
                if (ls == (READER|WAITER) && (w = mWaiter) != NULL) {
                    LockSupport::Unpark(w);
                }
                return p;
            }
        }
    }
    return NULL;
}

AutoPtr<CConcurrentHashMap::TreeNode> CConcurrentHashMap::TreeBin::PutTreeVal(
    /* [in] */ Int32 h,
    /* [in] */ IInterface* k,
    /* [in] */ IInterface* v)
{
    InterfaceID kc = EMUID_NULL;
    for (AutoPtr<TreeNode> p = mRoot;;) {
        Int32 dir, ph; AutoPtr<IInterface> pk; AutoPtr<TreeNode> q, pr;
        if (p == NULL) {
            mRoot = new TreeNode(h, k, v, NULL, NULL);
            mFirst = mRoot;
            break;
        }
        else if ((ph = p->mHash) > h) {
            dir = -1;
        }
        else if (ph < h) {
            dir = 1;
        }
        else if ((pk = p->mKey).Get() == k || (pk != NULL && Object::Equals(k, pk))) {
            return p;
        }
        else if ((kc == EMUID_NULL &&
                (kc = ComparableClassFor(k)) == EMUID_NULL) ||
                (dir = CompareComparables(kc, k, pk)) == 0) {
            if (p->mLeft == NULL) {
                dir = 1;
            }
            else if ((pr = p->mRight) == NULL ||
                     (q = pr->FindTreeNode(h, k, kc)) == NULL) {
                dir = -1;
            }
            else {
                return q;
            }
        }
        AutoPtr<TreeNode> xp = p;
        if ((p = (dir < 0) ? p->mLeft : p->mRight) == NULL) {
            AutoPtr<TreeNode> x, f = mFirst;
            mFirst = x = new TreeNode(h, k, v, f, xp);
            if (f != NULL) {
                f->mPrev = x;
            }
            if (dir < 0) {
                xp->mLeft = x;
            }
            else {
                xp->mRight = x;
            }
            if (!xp->mRed) {
                x->mRed = TRUE;
            }
            else {
                LockRoot();
                mRoot = BalanceInsertion(mRoot, x);
                UnlockRoot();
            }
            break;
        }
    }
    assert(CheckInvariants(mRoot));
    return NULL;
}

Boolean CConcurrentHashMap::TreeBin::RemoveTreeNode(
    /* [in] */ TreeNode* p)
{
    AutoPtr<TreeNode> next = (TreeNode*)ITreeNode::Probe(p->mNext);
    AutoPtr<TreeNode> pred = p->mPrev;  // unlink traversal pointers
    AutoPtr<TreeNode> r, rl;
    if (pred == NULL) {
        mFirst = next;
    }
    else {
        pred->mNext = next;
    }
    if (next != NULL) {
        next->mPrev = pred;
    }
    if (mFirst == NULL) {
        mRoot = NULL;
        return TRUE;
    }
    if ((r = mRoot) == NULL || r->mRight == NULL || // too small
        (rl = r->mLeft) == NULL || rl->mLeft == NULL) {
        return TRUE;
    }
    LockRoot();
    AutoPtr<TreeNode> replacement;
    AutoPtr<TreeNode> pl = p->mLeft;
    AutoPtr<TreeNode> pr = p->mRight;
    if (pl != NULL && pr != NULL) {
        AutoPtr<TreeNode> s = pr, sl;
        while ((sl = s->mLeft) != NULL) { // find successor
            s = sl;
        }
        Boolean c = s->mRed; s->mRed = p->mRed; p->mRed = c; // swap colors
        AutoPtr<TreeNode> sr = s->mRight;
        AutoPtr<TreeNode> pp = p->mParent;
        if (s == pr) { // p was s's direct parent
            p->mParent = s;
            s->mRight = p;
        }
        else {
            AutoPtr<TreeNode> sp = s->mParent;
            if ((p->mParent = sp) != NULL) {
                if (s == sp->mLeft) {
                    sp->mLeft = p;
                }
                else {
                    sp->mRight = p;
                }
            }
            if ((s->mRight = pr) != NULL) {
                pr->mParent = s;
            }
        }
        p->mLeft = NULL;
        if ((p->mRight = sr) != NULL) {
            sr->mParent = p;
        }
        if ((s->mLeft = pl) != NULL) {
            pl->mParent = s;
        }
        if ((s->mParent = pp) == NULL) {
            r = s;
        }
        else if (p == pp->mLeft) {
            pp->mLeft = s;
        }
        else {
            pp->mRight = s;
        }
        if (sr != NULL) {
            replacement = sr;
        }
        else {
            replacement = p;
        }
    }
    else if (pl != NULL) {
        replacement = pl;
    }
    else if (pr != NULL) {
        replacement = pr;
    }
    else {
        replacement = p;
    }
    if (replacement.Get() != p) {
        AutoPtr<TreeNode> pp = replacement->mParent = p->mParent;
        if (pp == NULL) {
            r = replacement;
        }
        else if (p == pp->mLeft) {
            pp->mLeft = replacement;
        }
        else {
            pp->mRight = replacement;
        }
        p->mLeft = p->mRight = p->mParent = NULL;
    }

    mRoot = (p->mRed) ? r : BalanceDeletion(r, replacement);

    if (p == replacement) {  // detach pointers
        AutoPtr<TreeNode> pp;
        if ((pp = p->mParent) != NULL) {
            if (p == pp->mLeft) {
                pp->mLeft = NULL;
            }
            else if (p == pp->mRight) {
                pp->mRight = NULL;
            }
            p->mParent = NULL;
        }
    }
    UnlockRoot();
    assert(CheckInvariants(mRoot));
    return FALSE;
}

AutoPtr<CConcurrentHashMap::TreeNode> CConcurrentHashMap::TreeBin::RotateLeft(
    /* [in] */ TreeNode* root,
    /* [in] */ TreeNode* p)
{
    AutoPtr<TreeNode> r, pp, rl;
    if (p != NULL && (r = p->mRight) != NULL) {
        if ((rl = p->mRight = r->mLeft) != NULL) {
            rl->mParent = p;
        }
        if ((pp = r->mParent = p->mParent) == NULL) {
            (root = r)->mRed = FALSE;
        }
        else if (pp->mLeft.Get() == p) {
            pp->mLeft = r;
        }
        else {
            pp->mRight = r;
        }
        r->mLeft = p;
        p->mParent = r;
    }
    return root;
}

AutoPtr<CConcurrentHashMap::TreeNode> CConcurrentHashMap::TreeBin::RotateRight(
    /* [in] */ TreeNode* root,
    /* [in] */ TreeNode* p)
{
    AutoPtr<TreeNode> l, pp, lr;
    if (p != NULL && (l = p->mLeft) != NULL) {
        if ((lr = p->mLeft = l->mRight) != NULL) {
            lr->mParent = p;
        }
        if ((pp = l->mParent = p->mParent) == NULL) {
            (root = l)->mRed = FALSE;
        }
        else if (pp->mRight.Get() == p) {
            pp->mRight = l;
        }
        else {
            pp->mLeft = l;
        }
        l->mRight = p;
        p->mParent = l;
    }
    return root;
}

AutoPtr<CConcurrentHashMap::TreeNode> CConcurrentHashMap::TreeBin::BalanceInsertion(
    /* [in] */ TreeNode* root,
    /* [in] */ TreeNode* x)
{
    x->mRed = TRUE;
    for (AutoPtr<TreeNode> xp, xpp, xppl, xppr;;) {
        if ((xp = x->mParent) == NULL) {
            x->mRed = FALSE;
            return x;
        }
        else if (!xp->mRed || (xpp = xp->mParent) == NULL) {
            return root;
        }
        if (xp == (xppl = xpp->mLeft)) {
            if ((xppr = xpp->mRight) != NULL && xppr->mRed) {
                xppr->mRed = FALSE;
                xp->mRed = FALSE;
                xpp->mRed = TRUE;
                x = xpp;
            }
            else {
                if (x == xp->mRight) {
                    root = RotateLeft(root, x = xp);
                    xpp = (xp = x->mParent) == NULL ? NULL : xp->mParent;
                }
                if (xp != NULL) {
                    xp->mRed = FALSE;
                    if (xpp != NULL) {
                        xpp->mRed = TRUE;
                        root = RotateRight(root, xpp);
                    }
                }
            }
        }
        else {
            if (xppl != NULL && xppl->mRed) {
                xppl->mRed = FALSE;
                xp->mRed = FALSE;
                xpp->mRed = TRUE;
                x = xpp;
            }
            else {
                if (x == xp->mLeft) {
                    root = RotateRight(root, x = xp);
                    xpp = (xp = x->mParent) == NULL ? NULL : xp->mParent;
                }
                if (xp != NULL) {
                    xp->mRed = FALSE;
                    if (xpp != NULL) {
                        xpp->mRed = TRUE;
                        root = RotateLeft(root, xpp);
                    }
                }
            }
        }
    }
}

AutoPtr<CConcurrentHashMap::TreeNode> CConcurrentHashMap::TreeBin::BalanceDeletion(
    /* [in] */ TreeNode* root,
    /* [in] */ TreeNode* x)
{
    for (AutoPtr<TreeNode> xp, xpl, xpr;;)  {
        if (x == NULL || x == root) {
            return root;
        }
        else if ((xp = x->mParent) == NULL) {
            x->mRed = FALSE;
            return x;
        }
        else if (x->mRed) {
            x->mRed = FALSE;
            return root;
        }
        else if ((xpl = xp->mLeft).Get() == x) {
            if ((xpr = xp->mRight) != NULL && xpr->mRed) {
                xpr->mRed = FALSE;
                xp->mRed = TRUE;
                root = RotateLeft(root, xp);
                xpr = (xp = x->mParent) == NULL ? NULL : xp->mRight;
            }
            if (xpr == NULL) {
                x = xp;
            }
            else {
                AutoPtr<TreeNode> sl = xpr->mLeft, sr = xpr->mRight;
                if ((sr == NULL || !sr->mRed) &&
                    (sl == NULL || !sl->mRed)) {
                    xpr->mRed = TRUE;
                    x = xp;
                }
                else {
                    if (sr == NULL || !sr->mRed) {
                        if (sl != NULL) {
                            sl->mRed = FALSE;
                        }
                        xpr->mRed = TRUE;
                        root = RotateRight(root, xpr);
                        xpr = (xp = x->mParent) == NULL ?
                            NULL : xp->mRight;
                    }
                    if (xpr != NULL) {
                        xpr->mRed = (xp == NULL) ? FALSE : xp->mRed;
                        if ((sr = xpr->mRight) != NULL) {
                            sr->mRed = FALSE;
                        }
                    }
                    if (xp != NULL) {
                        xp->mRed = FALSE;
                        root = RotateLeft(root, xp);
                    }
                    x = root;
                }
            }
        }
        else { // symmetric
            if (xpl != NULL && xpl->mRed) {
                xpl->mRed = FALSE;
                xp->mRed = TRUE;
                root = RotateRight(root, xp);
                xpl = (xp = x->mParent) == NULL ? NULL : xp->mLeft;
            }
            if (xpl == NULL) {
                x = xp;
            }
            else {
                AutoPtr<TreeNode> sl = xpl->mLeft, sr = xpl->mRight;
                if ((sl == NULL || !sl->mRed) &&
                    (sr == NULL || !sr->mRed)) {
                    xpl->mRed = TRUE;
                    x = xp;
                }
                else {
                    if (sl == NULL || !sl->mRed) {
                        if (sr != NULL) {
                            sr->mRed = FALSE;
                        }
                        xpl->mRed = TRUE;
                        root = RotateLeft(root, xpl);
                        xpl = (xp = x->mParent) == NULL ?
                            NULL : xp->mLeft;
                    }
                    if (xpl != NULL) {
                        xpl->mRed = (xp == NULL) ? FALSE : xp->mRed;
                        if ((sl = xpl->mLeft) != NULL) {
                            sl->mRed = FALSE;
                        }
                    }
                    if (xp != NULL) {
                        xp->mRed = FALSE;
                        root = RotateRight(root, xp);
                    }
                    x = root;
                }
            }
        }
    }
}

Boolean CConcurrentHashMap::TreeBin::CheckInvariants(
    /* [in] */ TreeNode* t)
{
    AutoPtr<TreeNode> tp = t->mParent, tl = t->mLeft, tr = t->mRight,
        tb = t->mPrev, tn = (TreeNode*)ITreeNode::Probe(t->mNext);
    if (tb != NULL && tb->mNext.Get() != t) {
        return FALSE;
    }
    if (tn != NULL && tn->mPrev.Get() != t) {
        return FALSE;
    }
    if (tp != NULL && t != tp->mLeft.Get() && t != tp->mRight.Get()) {
        return FALSE;
    }
    if (tl != NULL && (tl->mParent.Get() != t || tl->mHash > t->mHash)) {
        return FALSE;
    }
    if (tr != NULL && (tr->mParent.Get() != t || tr->mHash < t->mHash)) {
        return FALSE;
    }
    if (t->mRed && tl != NULL && tl->mRed && tr != NULL && tr->mRed) {
        return FALSE;
    }
    if (tl != NULL && !CheckInvariants(tl)) {
        return FALSE;
    }
    if (tr != NULL && !CheckInvariants(tr)) {
        return FALSE;
    }
    return TRUE;
}

//===============================================================================
// CConcurrentHashMap::Traverser::
//===============================================================================

CConcurrentHashMap::Traverser::Traverser(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 size,
    /* [in] */ Int32 index,
    /* [in] */ Int32 limit)
    : mTab(tab)
    , mIndex(index)
    , mBaseIndex(index)
    , mBaseLimit(limit)
    , mBaseSize(size)
{}

AutoPtr<CConcurrentHashMap::Node> CConcurrentHashMap::Traverser::Advance()
{
    AutoPtr<Node> e;
    if ((e = mNext) != NULL) {
        e = e->mNext;
    }
    for (;;) {
        AutoPtr<ArrayOf<Node*> > t; Int32 i, n; AutoPtr<IInterface> ek;  // must use locals in checks
        if (e != NULL) {
            return mNext = e;
        }
        if (mBaseIndex >= mBaseLimit || (t = mTab) == NULL ||
            (n = t->GetLength()) <= (i = mIndex) || i < 0) {
            return mNext = NULL;
        }
        if ((e = TabAt(t, mIndex)) != NULL && e->mHash < 0) {
            if (IForwardingNode::Probe(e) != NULL) {
                mTab = ((ForwardingNode*)IForwardingNode::Probe(e))->mNextTable;
                e = NULL;
                continue;
            }
            else if (ITreeBin::Probe(e) != NULL) {
                TreeBin* tb = (TreeBin*)ITreeBin::Probe(e);
                e = (Node*)(tb->mFirst.Get());
            }
            else {
                e = NULL;
            }
        }
        if ((mIndex += mBaseSize) >= n) {
            mIndex = ++mBaseIndex;    // visit upper slots if present
        }
    }
}

//===============================================================================
// CConcurrentHashMap::MapEntry::
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::MapEntry, Object, IMapEntry)

CConcurrentHashMap::MapEntry::MapEntry(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* val,
    /* [in] */ CConcurrentHashMap* map)
    : mKey(key)
    , mVal(val)
    , mMap(map)
{}

ECode CConcurrentHashMap::MapEntry::GetKey(
    /* [out] */ IInterface** key)
{
    VALIDATE_NOT_NULL(key)
    *key = mKey;
    REFCOUNT_ADD(*key)
    return NOERROR;
}

ECode CConcurrentHashMap::MapEntry::GetValue(
    /* [out] */ IInterface** value)
{
    VALIDATE_NOT_NULL(value)
    *value = mVal;
    REFCOUNT_ADD(*value)
    return NOERROR;
}

ECode CConcurrentHashMap::MapEntry::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode)
    Int32 hck = Object::GetHashCode(mKey);
    Int32 hcv = Object::GetHashCode(mVal);
    *hashCode = hck ^ hcv;
    return NOERROR;
}

ECode CConcurrentHashMap::MapEntry::ToString(
    /* [out] */ String* str)
{
    VALIDATE_NOT_NULL(str)

    *str = Object::ToString(mKey) + String("=") + Object::ToString(mVal);
    return NOERROR;
}

ECode CConcurrentHashMap::MapEntry::Equals(
    /* [in] */ IInterface* entry,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<IInterface> k, v; AutoPtr<IMapEntry> e;
    *result = ((IMapEntry::Probe(entry)!= NULL) &&
            ((e = IMapEntry::Probe(entry))->GetKey((IInterface**)&k), k != NULL) &&
            (e->GetValue((IInterface**)&v), v != NULL) &&
            (k == mKey || Object::Equals(k, mKey)) &&
            (v == mVal || Object::Equals(v, mVal)));
    return NOERROR;
}

ECode CConcurrentHashMap::MapEntry::SetValue(
    /* [in] */ IInterface* valueReplacer,
    /* [out] */ IInterface** valueReplacee)
{
    VALIDATE_NOT_NULL(valueReplacee)

    if (valueReplacer == NULL) return E_NULL_POINTER_EXCEPTION;
    AutoPtr<IInterface> v = mVal;
    mVal = valueReplacer;
    AutoPtr<IInterface> p;
    mMap->Put(mKey, valueReplacer, (IInterface**)&p);
    *valueReplacee = v.Get();
    REFCOUNT_ADD(*valueReplacee)
    return NOERROR;
}

//===============================================================================
// CConcurrentHashMap::CollectionView::
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::CollectionView, Object, ICollection, IIterable, ISerializable)

String CConcurrentHashMap::CollectionView::mOomeMsg = String("Required array size too large");

CConcurrentHashMap::CollectionView::CollectionView(
    /* [in] */ CConcurrentHashMap* map)
    : mMap(map)
{}

AutoPtr<CConcurrentHashMap> CConcurrentHashMap::CollectionView::GetMap()
{
    return mMap;
}

ECode CConcurrentHashMap::CollectionView::Clear()
{
    return mMap->Clear();
}

ECode CConcurrentHashMap::CollectionView::GetSize(
    /* [out] */ Int32* size)
{
    VALIDATE_NOT_NULL(size)

    return mMap->GetSize(size);
}

ECode CConcurrentHashMap::CollectionView::IsEmpty(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    return mMap->IsEmpty(result);
}

ECode CConcurrentHashMap::CollectionView::ToArray(
    /* [out, callee] */ ArrayOf<IInterface*>** array)
{
    VALIDATE_NOT_NULL(array)

    Int64 sz = mMap->MappingCount();
    if (sz > MAX_ARRAY_SIZE) {
        return E_OUT_OF_MEMORY;
    }
    Int32 n = (Int32)sz;
    AutoPtr<ArrayOf<IInterface*> > r = ArrayOf<IInterface*>::Alloc(n);
    Int32 i = 0;
    AutoPtr<IIterator> it;
    GetIterator((IIterator**)&it);
    Boolean hasNext = FALSE;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> e;
        it->GetNext((IInterface**)&e);
        if (i == n) {
            if (n >= MAX_ARRAY_SIZE) {
                return E_OUT_OF_MEMORY;
            }
            if (n >= MAX_ARRAY_SIZE - (((UInt32)MAX_ARRAY_SIZE) >> 1) - 1) {
                n = MAX_ARRAY_SIZE;
            }
            else {
                n += (((UInt32)n) >> 1) + 1;
            }
            AutoPtr< ArrayOf<IInterface*> > rr;
            Arrays::CopyOf(r, n, (ArrayOf<IInterface*>**)&rr);
            r = rr;
        }
        r->Set(i++, e);
    }

    if (i == n) {
        *array = r;
        REFCOUNT_ADD(*array)
        return NOERROR;
    }
    else {
        return Arrays::CopyOf(r, i, array);
    }
}

ECode CConcurrentHashMap::CollectionView::ToArray(
    /* [in] */ ArrayOf<IInterface*>* inArray,
    /* [out, callee] */ ArrayOf<IInterface*>** outArray)
{
    VALIDATE_NOT_NULL(outArray)

    Int64 sz = mMap->MappingCount();
    if (sz > MAX_ARRAY_SIZE) {
        return E_OUT_OF_MEMORY;
    }
    Int32 m = (Int32)sz;
    AutoPtr<ArrayOf<IInterface*> > r = (inArray->GetLength() >= m) ? inArray :
            ArrayOf<IInterface*>::Alloc(m);
    Int32 n = r->GetLength();
    Int32 i = 0;
    AutoPtr<IIterator> it;
    GetIterator((IIterator**)&it);
    Boolean hasNext = FALSE;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> e;
        it->GetNext((IInterface**)&e);
        if (i == n) {
            if (n >= MAX_ARRAY_SIZE) {
                return E_OUT_OF_MEMORY;
            }
            if (n >= MAX_ARRAY_SIZE - (((UInt32)MAX_ARRAY_SIZE) >> 1) - 1) {
                n = MAX_ARRAY_SIZE;
            }
            else {
                n += (((UInt32)n) >> 1) + 1;
            }
            AutoPtr< ArrayOf<IInterface*> > rr;
            Arrays::CopyOf(r, n, (ArrayOf<IInterface*>**)&rr);
            r = rr;
        }
        r->Set(i++, e);
    }
    if (inArray == r && i < n) {
        r->Set(i, NULL); // NULL-terminate
        *outArray = r;
        REFCOUNT_ADD(*outArray)
        return NOERROR;
    }

    if (i == n) {
        *outArray = r;
        REFCOUNT_ADD(*outArray)
        return NOERROR;
    }
    else {
        return Arrays::CopyOf(r, i, outArray);
    }
}

String CConcurrentHashMap::CollectionView::ToString()
{
    StringBuilder sb;
    sb.AppendChar('[');
    AutoPtr<IIterator> it;
    GetIterator((IIterator**)&it);
    Boolean b1 = FALSE;
    if ((it->HasNext(&b1), b1)) {
        for (;;) {
            AutoPtr<IInterface> e;
            it->GetNext((IInterface**)&e);
            if (Object::Equals(e, TO_IINTERFACE(this))) {
                sb.Append("(this Collection)");
            }
            else {
                sb.Append(e);
            }
            Boolean b2 = FALSE;
            if (!(it->HasNext(&b2), b2)) {
                break;
            }
            sb.AppendChar(',');
            sb.AppendChar(' ');
        }
    }
    sb.AppendChar(']');
    return sb.ToString();
}

ECode CConcurrentHashMap::CollectionView::ContainsAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (collection != (ICollection*)this) {
        AutoPtr<IIterator> it;
        collection->GetIterator((IIterator**)&it);
        Boolean hasNext;
        while (it->HasNext(&hasNext), hasNext) {
            AutoPtr<IInterface> obj;
            it->GetNext((IInterface**)&obj);
            Boolean b = FALSE;
            if (obj == NULL || !(Contains(obj, &b), b)) {
                *result = FALSE;
                return NOERROR;
            }
        }
    }
    *result = TRUE;
    return NOERROR;
}

ECode CConcurrentHashMap::CollectionView::RemoveAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified)

    Boolean mod = FALSE;
    AutoPtr<IIterator> it;
    GetIterator((IIterator**)&it);
    Boolean b = FALSE;
    for (; (it->HasNext(&b), b);) {
        AutoPtr<IInterface> p;
        it->GetNext((IInterface**)&p);
        Boolean b2 = FALSE;
        if ((collection->Contains(p, &b2), b2)) {
            it->Remove();
            mod = TRUE;
        }
    }
    *modified = mod;
    return NOERROR;
}

ECode CConcurrentHashMap::CollectionView::RetainAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* modified)
{
    Boolean mod = FALSE;
    AutoPtr<IIterator> it;
    GetIterator((IIterator**)&it);
    Boolean b = FALSE;
    for (;(it->HasNext(&b), b);) {
        AutoPtr<IInterface> p;
        it->GetNext((IInterface**)&p);
        Boolean b2 = FALSE;
        if (!(collection->Contains(p, &b2), b2)) {
            it->Remove();
            mod = TRUE;
        }
    }
    *modified = mod;
    return NOERROR;
}

//===============================================================================
// CConcurrentHashMap::BaseIterator
//===============================================================================
CConcurrentHashMap::BaseIterator::BaseIterator(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 size,
    /* [in] */ Int32 index,
    /* [in] */ Int32 limit,
    /* [in] */ CConcurrentHashMap* map) : Traverser(tab, size, index, limit)
{
    mMap = map;
    Advance();
}

ECode CConcurrentHashMap::BaseIterator::HasNext(
    /* [out] */ Boolean* value)
{
    VALIDATE_NOT_NULL(value)
    *value = mNext != NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::BaseIterator::HasMoreElements(
    /* [out] */ Boolean* value)
{
    VALIDATE_NOT_NULL(value)
    *value = mNext != NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::BaseIterator::Remove()
{
    AutoPtr<Node> p;
    if ((p = mLastReturned) == NULL) {
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    mLastReturned = NULL;
    mMap->ReplaceNode(p->mKey, NULL, NULL);
    return NOERROR;
}

//===============================================================================
// CConcurrentHashMap::KeyIterator
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::KeyIterator, BaseIterator, IIterator, IEnumeration)

CConcurrentHashMap::KeyIterator::KeyIterator(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 index,
    /* [in] */ Int32 size,
    /* [in] */ Int32 limit,
    /* [in] */ CConcurrentHashMap* map)
    : BaseIterator(tab, index, size, limit, map)
{
}

ECode CConcurrentHashMap::KeyIterator::GetNext(
    /* [out] */ IInterface** object)
{
    VALIDATE_NOT_NULL(object)

    AutoPtr<Node> p;
    if ((p = mNext) == NULL) {
        return E_NO_SUCH_ELEMENT_EXCEPTION;
    }
    AutoPtr<IInterface> k = p->mKey;
    mLastReturned = p;
    Advance();
    *object = k;
    REFCOUNT_ADD(*object)
    return NOERROR;
}

ECode CConcurrentHashMap::KeyIterator::GetNextElement(
    /* [out] */ IInterface** object)
{
    return GetNext(object);
}

ECode CConcurrentHashMap::KeyIterator::HasNext(
    /* [out] */ Boolean* value)
{
    return BaseIterator::HasNext(value);
}

ECode CConcurrentHashMap::KeyIterator::HasMoreElements(
    /* [out] */ Boolean* value)
{
    return BaseIterator::HasMoreElements(value);
}

ECode CConcurrentHashMap::KeyIterator::Remove()
{
    return BaseIterator::Remove();
}

//===============================================================================
// CConcurrentHashMap::ValueIterator
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::ValueIterator, BaseIterator, IIterator, IEnumeration)

CConcurrentHashMap::ValueIterator::ValueIterator(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 index,
    /* [in] */ Int32 size,
    /* [in] */ Int32 limit,
    /* [in] */ CConcurrentHashMap* map)
    : BaseIterator(tab, index, size, limit, map)
{
}

ECode CConcurrentHashMap::ValueIterator::GetNext(
    /* [out] */ IInterface** object)
{
    VALIDATE_NOT_NULL(object)

    AutoPtr<Node> p;
    if ((p = mNext) == NULL) {
        return E_NO_SUCH_ELEMENT_EXCEPTION;
    }
    AutoPtr<IInterface> v = p->mVal;
    mLastReturned = p;
    Advance();
    *object = v;
    REFCOUNT_ADD(*object)
    return NOERROR;
}

ECode CConcurrentHashMap::ValueIterator::GetNextElement(
    /* [out] */ IInterface** object)
{
    return GetNext(object);
}

ECode CConcurrentHashMap::ValueIterator::HasNext(
    /* [out] */ Boolean* value)
{
    return BaseIterator::HasNext(value);
}

ECode CConcurrentHashMap::ValueIterator::HasMoreElements(
    /* [out] */ Boolean* value)
{
    return BaseIterator::HasMoreElements(value);
}

ECode CConcurrentHashMap::ValueIterator::Remove()
{
    return BaseIterator::Remove();
}

//===============================================================================
// CConcurrentHashMap::EntryIterator
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::EntryIterator, BaseIterator, IIterator)

CConcurrentHashMap::EntryIterator::EntryIterator(
    /* [in] */ ArrayOf<Node*>* tab,
    /* [in] */ Int32 index,
    /* [in] */ Int32 size,
    /* [in] */ Int32 limit,
    /* [in] */ CConcurrentHashMap* map)
    : BaseIterator(tab, index, size, limit, map)
{
}

ECode CConcurrentHashMap::EntryIterator::GetNext(
    /* [out] */ IInterface** object)
{
    VALIDATE_NOT_NULL(object)

    AutoPtr<Node> p;
    if ((p = mNext) == NULL) {
        return E_NO_SUCH_ELEMENT_EXCEPTION;
    }
    AutoPtr<IInterface> k = p->mKey;
    AutoPtr<IInterface> v = p->mVal;
    mLastReturned = p;
    Advance();
    AutoPtr<MapEntry> res = new MapEntry(k, v, mMap);
    *object = (IInterface*)IMapEntry::Probe(res);
    REFCOUNT_ADD(*object)
    return NOERROR;
}

ECode CConcurrentHashMap::EntryIterator::HasNext(
    /* [out] */ Boolean* value)
{
    return BaseIterator::HasNext(value);
}

ECode CConcurrentHashMap::EntryIterator::Remove()
{
    return BaseIterator::Remove();
}

//===============================================================================
// CConcurrentHashMap::KeySetView
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::KeySetView, CollectionView, ISet)

CConcurrentHashMap::KeySetView::KeySetView(
    /* [in] */ CConcurrentHashMap* map,
    /* [in] */ IInterface* value) : CollectionView(map)
{
    mValue = value;
}

AutoPtr<IInterface> CConcurrentHashMap::KeySetView::GetMappedValue()
{
    return mValue;
}

ECode CConcurrentHashMap::KeySetView::Contains(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    return mMap->ContainsKey(o, result);
}

ECode CConcurrentHashMap::KeySetView::Remove(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<IInterface> outface;
    mMap->Remove(o, (IInterface**)&outface);
    *result = outface != NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::KeySetView::GetIterator(
    /* [out] */ IIterator** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ArrayOf<Node*> > t;
    AutoPtr<CConcurrentHashMap> m = mMap;
    Int32 f = (t = m->mTable) == NULL ? 0 : t->GetLength();
    AutoPtr<KeyIterator> res = new KeyIterator(t, f, 0, f, m);
    *result = (IIterator*)res.Get();
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CConcurrentHashMap::KeySetView::Add(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified)

    AutoPtr<IInterface> v;
    if ((v = mValue) == NULL) {
        return E_UNSUPPORTED_OPERATION_EXCEPTION;
    }
    AutoPtr<IInterface> oldValue;
    FAIL_RETURN(mMap->PutVal(object, v, TRUE, (IInterface**)&oldValue))
    *modified = oldValue == NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::KeySetView::AddAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified)

    Boolean added = FALSE;
    AutoPtr<IInterface> v;
    if ((v = mValue) == NULL) {
        return E_UNSUPPORTED_OPERATION_EXCEPTION;
    }

    AutoPtr<IIterator> it;
    collection->GetIterator((IIterator**)&it);
    Boolean hasNext;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> obj;
        it->GetNext((IInterface**)&obj);
        AutoPtr<IInterface> oldValue;
        FAIL_RETURN(mMap->PutVal(obj, v, TRUE, (IInterface**)&oldValue))
        if (oldValue == NULL) {
            added = TRUE;
        }
    }
    *modified = added;
    return NOERROR;
}

ECode CConcurrentHashMap::KeySetView::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode)

    Int32 h = 0;
    AutoPtr<IIterator> it;
    GetIterator((IIterator**)&it);
    Boolean hasNext;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> obj;
        it->GetNext((IInterface**)&obj);
        Int32 hc = Object::GetHashCode(obj);
        h += hc;
    }
    *hashCode = h;
    return NOERROR;
}

ECode CConcurrentHashMap::KeySetView::Equals(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ISet> c; Boolean bc1 = FALSE, bc2 = FALSE;
    *result = (ISet::Probe(object) != NULL) &&
            ((c = ISet::Probe(object), Object::Equals(object, TO_IINTERFACE(this))) ||
             ((ContainsAll(ICollection::Probe(c), &bc1), bc1) && (ICollection::Probe(c)->ContainsAll(this, &bc2), bc2)));
    return NOERROR;
}

ECode CConcurrentHashMap::KeySetView::Add(
    /* [in] */ IInterface* object)
{
    Boolean b;
    return Add(object, &b);
}

ECode CConcurrentHashMap::KeySetView::AddAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return AddAll(collection, &b);
}

ECode CConcurrentHashMap::KeySetView::Remove(
    /* [in] */ IInterface* object)
{
    Boolean b;
    return Remove(object, &b);
}

ECode CConcurrentHashMap::KeySetView::RemoveAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return CollectionView::RemoveAll(collection, &b);
}

ECode CConcurrentHashMap::KeySetView::RetainAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return CollectionView::RetainAll(collection, &b);
}

ECode CConcurrentHashMap::KeySetView::Clear()
{
    return CollectionView::Clear();
}

ECode CConcurrentHashMap::KeySetView::ContainsAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* result)
{
    return CollectionView::ContainsAll(collection, result);
}

ECode CConcurrentHashMap::KeySetView::IsEmpty(
    /* [out] */ Boolean* result)
{
    return CollectionView::IsEmpty(result);
}

ECode CConcurrentHashMap::KeySetView::RemoveAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* result)
{
    return CollectionView::RemoveAll(collection, result);
}

ECode CConcurrentHashMap::KeySetView::RetainAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* result)
{
    return CollectionView::RetainAll(collection, result);
}

ECode CConcurrentHashMap::KeySetView::GetSize(
    /* [out] */ Int32* size)
{
    return CollectionView::GetSize(size);
}

ECode CConcurrentHashMap::KeySetView::ToArray(
    /* [out, callee] */ ArrayOf<IInterface*>** result)
{
    return CollectionView::ToArray(result);
}

ECode CConcurrentHashMap::KeySetView::ToArray(
    /* [in] */ ArrayOf<IInterface*>* array,
    /* [out, callee] */ ArrayOf<IInterface*>** result)
{
    return CollectionView::ToArray(array, result);
}

//===============================================================================
// CConcurrentHashMap::ValuesView::
//===============================================================================

CConcurrentHashMap::ValuesView::ValuesView(
    /* [in] */ CConcurrentHashMap* map)
    : CollectionView(map)
{
}

ECode CConcurrentHashMap::ValuesView::Contains(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    return mMap->ContainsValue(o, result);
}

ECode CConcurrentHashMap::ValuesView::Remove(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified)

    if (object != NULL) {
        AutoPtr<IIterator> it;
        GetIterator((IIterator**)&it);
        Boolean b = FALSE;
        for (;(it->HasNext(&b), b);) {
            AutoPtr<IInterface> p;
            it->GetNext((IInterface**)&p);
            if (Object::Equals(object, p)) {
                it->Remove();
                *modified = TRUE;
                return NOERROR;
            }
        }
    }
    *modified = FALSE;
    return NOERROR;
}

ECode CConcurrentHashMap::ValuesView::GetIterator(
    /* [out] */ IIterator** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<CConcurrentHashMap> m = mMap;
    AutoPtr<ArrayOf<Node*> > t;
    Int32 f = (t = m->mTable) == NULL ? 0 : t->GetLength();
    AutoPtr<ValueIterator> res = new ValueIterator(t, f, 0, f, m);
    *result = (IIterator*)res;
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CConcurrentHashMap::ValuesView::Add(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* modified)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CConcurrentHashMap::ValuesView::AddAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* modified)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CConcurrentHashMap::ValuesView::Add(
    /* [in] */ IInterface* object)
{
    Boolean b;
    return Add(object, &b);
}

ECode CConcurrentHashMap::ValuesView::AddAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return AddAll(collection, &b);
}

ECode CConcurrentHashMap::ValuesView::Remove(
    /* [in] */ IInterface* object)
{
    Boolean b;
    return Remove(object, &b);
}

ECode CConcurrentHashMap::ValuesView::RemoveAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return CollectionView::RemoveAll(collection, &b);
}

ECode CConcurrentHashMap::ValuesView::RetainAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return CollectionView::RetainAll(collection, &b);
}

ECode CConcurrentHashMap::ValuesView::Equals(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* result)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

ECode CConcurrentHashMap::ValuesView::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    return E_UNSUPPORTED_OPERATION_EXCEPTION;
}

//===============================================================================
// CConcurrentHashMap::EntrySetView
//===============================================================================
CAR_INTERFACE_IMPL(CConcurrentHashMap::EntrySetView, CollectionView, ISet)

CConcurrentHashMap::EntrySetView::EntrySetView(
    /* [in] */ CConcurrentHashMap* map)
    : CollectionView(map)
{
}

ECode CConcurrentHashMap::EntrySetView::Contains(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (!(IMapEntry::Probe(o))) {
        *result = FALSE;
        return NOERROR;
    }
    AutoPtr<IMapEntry> e = IMapEntry::Probe(o);
    AutoPtr<IInterface> k, v, r;
    *result = (e->GetKey((IInterface**)&k), k != NULL) &&
                (mMap->Get(k, (IInterface**)&r), r != NULL) &&
                (e->GetValue((IInterface**)&v), v != NULL) &&
                (v == r || Object::Equals(v, r));
    return NOERROR;
}

ECode CConcurrentHashMap::EntrySetView::Remove(
    /* [in] */ IInterface* o,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (!(IMapEntry::Probe(o))) {
        *result = FALSE;
        return NOERROR;
    }
    AutoPtr<IMapEntry> e = IMapEntry::Probe(o);
    AutoPtr<IInterface> k, v; Boolean b = FALSE;
    *result = (e->GetKey((IInterface**)&k), k != NULL) &&
                (e->GetValue((IInterface**)&v), v != NULL) &&
                (mMap->Remove(k, v, &b), b);
    return NOERROR;
}

ECode CConcurrentHashMap::EntrySetView::GetIterator(
    /* [out] */ IIterator** result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<CConcurrentHashMap> m = mMap;
    AutoPtr<ArrayOf<Node*> > t;
    Int32 f = (t = m->mTable) == NULL ? 0 : t->GetLength();
    AutoPtr<EntryIterator> res = new EntryIterator(t, f, 0, f, m);
    *result = (IIterator*)res;
    REFCOUNT_ADD(*result)
    return NOERROR;
}

ECode CConcurrentHashMap::EntrySetView::Add(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified)

    AutoPtr<IMapEntry> e = IMapEntry::Probe(object);
    AutoPtr<IInterface> k,v;
    e->GetKey((IInterface**)&k);
    e->GetValue((IInterface**)&v);
    AutoPtr<IInterface> oldValue;
    FAIL_RETURN(mMap->PutVal(k, v, FALSE, (IInterface**)&oldValue))
    *modified = oldValue == NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::EntrySetView::AddAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified)

    Boolean added = FALSE;
    AutoPtr<IIterator> it;
    collection->GetIterator((IIterator**)&it);
    Boolean hasNext;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> obj;
        it->GetNext((IInterface**)&obj);
        AutoPtr<IMapEntry> e = IMapEntry::Probe(obj);
        Boolean b = FALSE;
        if ((Add(e, &b), b)) {
            added = TRUE;
        }
    }
    *modified = added;
    return NOERROR;
}

ECode CConcurrentHashMap::EntrySetView::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode)

    Int32 h = 0;
    AutoPtr<ArrayOf<Node*> > t;
    if ((t = mMap->mTable) != NULL) {
        AutoPtr<Traverser> it = new Traverser(t, t->GetLength(), 0, t->GetLength());
        for (AutoPtr<Node> p; (p = it->Advance()) != NULL; ) {
            Int32 hc;
            p->GetHashCode(&hc);
            h += hc;
        }
    }
    *hashCode = h;
    return NOERROR;
}

ECode CConcurrentHashMap::EntrySetView::Equals(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<ISet> c;
    Boolean bc1, bc2;
    *result = (ISet::Probe(object) != NULL) &&
                ((c = ISet::Probe(object), Object::Equals(object, TO_IINTERFACE(this))) ||
                 ((ContainsAll(ICollection::Probe(c), &bc1), bc1) && (ICollection::Probe(c)->ContainsAll(this, &bc2), bc2)));
    return NOERROR;
}

ECode CConcurrentHashMap::EntrySetView::Add(
    /* [in] */ IInterface* object)
{
    Boolean b;
    return Add(object, &b);
}

ECode CConcurrentHashMap::EntrySetView::AddAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return AddAll(collection, &b);
}

ECode CConcurrentHashMap::EntrySetView::Remove(
    /* [in] */ IInterface* object)
{
    Boolean b;
    return Remove(object, &b);
}

ECode CConcurrentHashMap::EntrySetView::RemoveAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return CollectionView::RemoveAll(collection, &b);
}

ECode CConcurrentHashMap::EntrySetView::RetainAll(
    /* [in] */ ICollection* collection)
{
    Boolean b;
    return CollectionView::RetainAll(collection, &b);
}

ECode CConcurrentHashMap::EntrySetView::Clear()
{
    return CollectionView::Clear();
}

ECode CConcurrentHashMap::EntrySetView::ContainsAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* result)
{
    return CollectionView::ContainsAll(collection, result);
}

ECode CConcurrentHashMap::EntrySetView::IsEmpty(
    /* [out] */ Boolean* result)
{
    return CollectionView::IsEmpty(result);
}

ECode CConcurrentHashMap::EntrySetView::RemoveAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* result)
{
    return CollectionView::RemoveAll(collection, result);
}

ECode CConcurrentHashMap::EntrySetView::RetainAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* result)
{
    return CollectionView::RetainAll(collection, result);
}

ECode CConcurrentHashMap::EntrySetView::GetSize(
    /* [out] */ Int32* size)
{
    return CollectionView::GetSize(size);
}

ECode CConcurrentHashMap::EntrySetView::ToArray(
    /* [out, callee] */ ArrayOf<IInterface*>** result)
{
    return CollectionView::ToArray(result);
}

ECode CConcurrentHashMap::EntrySetView::ToArray(
    /* [in] */ ArrayOf<IInterface*>* array,
    /* [out, callee] */ ArrayOf<IInterface*>** result)
{
    return CollectionView::ToArray(array, result);
}


//====================================================================
// CConcurrentHashMap::StaticInitializer
//====================================================================
static void ThreadDestructor(void* st)
{
    CConcurrentHashMap::CounterHashCode* handler = static_cast<CConcurrentHashMap::CounterHashCode*>(st);
    if (handler) {
        handler->Release();
    }
}

CConcurrentHashMap::StaticInitializer::StaticInitializer()
{
    CAtomicInteger32::New(1, (IAtomicInteger32**)&sCounterHashCodeGenerator);
    Int32 UNUSED(result) = pthread_key_create(&CConcurrentHashMap::sThreadCounterHashCode, ThreadDestructor);
    assert(result == 0);
}

//===============================================================================
// CConcurrentHashMap
//===============================================================================

const Int32 CConcurrentHashMap::MAXIMUM_CAPACITY = 1 << 30;
const Int32 CConcurrentHashMap::DEFAULT_CAPACITY = 16;
const Int32 CConcurrentHashMap::MAX_ARRAY_SIZE = Elastos::Core::Math::INT32_MAX_VALUE - 8;
const Int32 CConcurrentHashMap::DEFAULT_CONCURRENCY_LEVEL = 16;
const Float CConcurrentHashMap::LOAD_FACTOR = 0.75f;
const Int32 CConcurrentHashMap::TREEIFY_THRESHOLD = 8;
const Int32 CConcurrentHashMap::UNTREEIFY_THRESHOLD = 6;
const Int32 CConcurrentHashMap::MIN_TREEIFY_CAPACITY = 64;
const Int32 CConcurrentHashMap::MIN_TRANSFER_STRIDE = 16;
const Int32 CConcurrentHashMap::MOVED     = 0x8fffffff; // (-1) hash for forwarding nodes
const Int32 CConcurrentHashMap::TREEBIN   = 0x80000000; // hash for roots of trees
const Int32 CConcurrentHashMap::RESERVED  = 0x80000001; // hash for transient reservations
const Int32 CConcurrentHashMap::HASH_BITS = 0x7fffffff; // usable bits of normal node hash
const Int32 CConcurrentHashMap::NCPU = 4;// = Runtime.getRuntime().availableProcessors();
const AutoPtr<IAtomicInteger32> CConcurrentHashMap::sCounterHashCodeGenerator;
Int32 CConcurrentHashMap::SEED_INCREMENT = 0x61c88647;
pthread_key_t CConcurrentHashMap::sThreadCounterHashCode;
const CConcurrentHashMap::StaticInitializer sInitializer;

CAR_INTERFACE_IMPL(CConcurrentHashMap, AbstractMap, IConcurrentHashMap, IConcurrentMap, ISerializable)

CAR_OBJECT_IMPL(CConcurrentHashMap)

ECode CConcurrentHashMap::constructor()
{
    return NOERROR;
}

ECode CConcurrentHashMap::constructor(
    /* [in] */ Int32 initialCapacity)
{
    if (initialCapacity < 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    Int32 cap = ((initialCapacity >= (((UInt32)MAXIMUM_CAPACITY) >> 1)) ?
               MAXIMUM_CAPACITY :
               TableSizeFor(initialCapacity + (((UInt32)initialCapacity) >> 1) + 1));
    mSizeCtl = cap;
    return NOERROR;
}

ECode CConcurrentHashMap::constructor(
    /* [in] */ IMap* m)
{
    mSizeCtl = DEFAULT_CAPACITY;
    PutAll(m);
    return NOERROR;
}

ECode CConcurrentHashMap::constructor(
    /* [in] */ Int32 initialCapacity,
    /* [in] */ Float loadFactor,
    /* [in] */ Int32 concurrencyLevel)
{
    if (!(loadFactor > 0) || initialCapacity < 0 || concurrencyLevel <= 0) {
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }
    if (initialCapacity < concurrencyLevel) {  // Use at least as many bins
        initialCapacity = concurrencyLevel;   // as estimated threads
    }
    Int64 size = (Int64)(1.0 + (Int64)initialCapacity / loadFactor);
    Int32 cap = (size >= (Int64)MAXIMUM_CAPACITY) ?
            MAXIMUM_CAPACITY : TableSizeFor((Int32)size);
    mSizeCtl = cap;
    return NOERROR;
}

ECode CConcurrentHashMap::constructor(
    /* [in] */ Int32 initialCapacity,
    /* [in] */ Float loadFactor)
{
    return constructor(initialCapacity, loadFactor, 1);
}

ECode CConcurrentHashMap::PutIfAbsent(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* value,
    /* [out] */ IInterface** outface)
{
    VALIDATE_NOT_NULL(outface)
    return PutVal(key, value, TRUE, outface);
}

ECode CConcurrentHashMap::Remove(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* value,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    if (key == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    *result = value != NULL && ReplaceNode(key, NULL, value) != NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::Replace(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* oldValue,
    /* [in] */ IInterface* newValue,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (key == NULL || oldValue == NULL || newValue == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    *result = ReplaceNode(key, newValue, oldValue) != NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::Replace(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* value,
    /* [out] */ IInterface** result)
{
    VALIDATE_NOT_NULL(result)
    if (key == NULL || value == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    AutoPtr<IInterface> p = ReplaceNode(key, value, NULL);
    *result = p;
    REFCOUNT_ADD(*result);
    return NOERROR;
}

ECode CConcurrentHashMap::Clear()
{
    Int64 delta = 0L; // negative number of deletions
    Int32 i = 0;
    AutoPtr<ArrayOf<Node*> > tab = mTable;
    while (tab != NULL && i < tab->GetLength()) {
        Int32 fh;
        AutoPtr<Node> f = TabAt(tab, i);
        if (f == NULL) {
            ++i;
        }
        else if ((fh = f->mHash) == MOVED) {
            tab = HelpTransfer(tab, f);
            i = 0; // restart
        }
        else {
            {
                AutoLock syncLock(f);
                if (TabAt(tab, i) == f) {
                    TreeBin* tb = (TreeBin*)ITreeBin::Probe(f);
                    AutoPtr<Node> p = (fh >= 0 ? f.Get() :
                                   (tb != NULL) ? (Node*)tb->mFirst.Get() : NULL);
                    while (p != NULL) {
                        --delta;
                        p = p->mNext;
                    }
                    SetTabAt(tab, i++, NULL);
                }
            }
        }
    }
    if (delta != 0LL) {
        AddCount(delta, -1);
    }
    return NOERROR;
}

ECode CConcurrentHashMap::ContainsKey(
    /* [in] */ IInterface* key,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    AutoPtr<IInterface> value;
    Get(key, (IInterface**)&value);
    *result = value != NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::ContainsValue(
    /* [in] */ IInterface* value,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (value == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    AutoPtr<ArrayOf<Node*> > t;
    if ((t = mTable) != NULL) {
        AutoPtr<Traverser> it = new Traverser(t, t->GetLength(), 0, t->GetLength());
        for (AutoPtr<Node> p; (p = it->Advance()) != NULL; ) {
            AutoPtr<IInterface> v;
            if ((v = p->mVal).Get() == value || (v != NULL && Object::Equals(value, v))) {
                *result = TRUE;
                return NOERROR;
            }
        }
    }
    *result = FALSE;
    return NOERROR;
}

ECode CConcurrentHashMap::Contains(
    /* [in] */ IInterface* value,
    /* [out] */ Boolean* result)
{
    // BEGIN android-note
    // removed deprecation
    // END android-note
    return ContainsValue(value, result);
}

ECode CConcurrentHashMap::GetEntrySet(
    /* [out] */ ISet** entries)
{
    VALIDATE_NOT_NULL(entries)

    AutoPtr<EntrySetView> es = mEntrySet;
    AutoPtr<EntrySetView> res = (es != NULL) ? es : (mEntrySet = new EntrySetView(this));
    *entries = ISet::Probe(res);
    REFCOUNT_ADD(*entries)
    return NOERROR;
}

ECode CConcurrentHashMap::Equals(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    if (!Object::Equals(object, TO_IINTERFACE(this))) {
        if (IMap::Probe(object) == NULL) {
            *result = FALSE;
            return NOERROR;
        }
        AutoPtr<IMap> m = IMap::Probe(object);
        AutoPtr<ArrayOf<Node*> > t;
        Int32 f = (t = mTable) == NULL ? 0 : t->GetLength();
        AutoPtr<Traverser> it = new Traverser(t, f, 0, f);
        for (AutoPtr<Node> p; (p = it->Advance()) != NULL; ) {
            AutoPtr<IInterface> val = p->mVal;
            AutoPtr<IInterface> v;
            m->Get(p->mKey, (IInterface**)&v);
            if (v == NULL || (v != val && !Object::Equals(v, val))) {
                *result = FALSE;
                return NOERROR;
            }
        }

        AutoPtr<ISet> s;
        m->GetEntrySet((ISet**)&s);
        AutoPtr<IIterator> iter;
        s->GetIterator((IIterator**)&iter);
        Boolean hasNext;
        while (iter->HasNext(&hasNext), hasNext) {
            AutoPtr<IInterface> obj;
            iter->GetNext((IInterface**)&obj);
            AutoPtr<IMapEntry> e = IMapEntry::Probe(obj);
            AutoPtr<IInterface> mk, mv, v;
            if ((e->GetKey((IInterface**)&mk), mk) == NULL ||
                (e->GetValue((IInterface**)&mv), mv) == NULL ||
                (Get(mk, (IInterface**)&v), v) == NULL ||
                (mv != v && !Object::Equals(mv, v))) {
                *result = FALSE;
                return NOERROR;
            }
        }
    }
    *result = TRUE;
    return NOERROR;
}

ECode CConcurrentHashMap::Get(
    /* [in] */ IInterface* key,
    /* [out] */ IInterface** value)
{
    VALIDATE_NOT_NULL(value)

    AutoPtr<ArrayOf<Node*> > tab; AutoPtr<Node> e, p; Int32 n, eh; AutoPtr<IInterface> ek;
    Int32 hc = Object::GetHashCode(key);
    Int32 h = Spread(hc);
    if ((tab = mTable) != NULL && (n = tab->GetLength()) > 0 &&
        (e = TabAt(tab, (n - 1) & h)) != NULL) {
        if ((eh = e->mHash) == h) {
            if ((ek = e->mKey).Get() == key || (ek != NULL && Object::Equals(key, ek))) {
                *value = e->mVal;
                REFCOUNT_ADD(*value);
                return NOERROR;
            }
        }
        else if (eh < 0) {
            *value = ((p = e->Find(h, key)) != NULL) ? (p->mVal).Get() : NULL;
            REFCOUNT_ADD(*value)
            return NOERROR;
        }
        while ((e = e->mNext) != NULL) {
            if (e->mHash == h &&
                ((ek = e->mKey).Get() == key || (ek != NULL && Object::Equals(key, ek)))) {
                *value = e->mVal;
                REFCOUNT_ADD(*value);
                return NOERROR;
            }
        }
    }
    *value = NULL;
    return NOERROR;
}

ECode CConcurrentHashMap::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    VALIDATE_NOT_NULL(hashCode);
    Int32 h = 0;
    AutoPtr<ArrayOf<Node*> > t;
    if ((t = mTable) != NULL) {
        AutoPtr<Traverser> it = new Traverser(t, t->GetLength(), 0, t->GetLength());
        for (AutoPtr<Node> p; (p = it->Advance()) != NULL; ) {
            Int32 hck = Object::GetHashCode(p->mKey);
            Int32 hcv = Object::GetHashCode(p->mVal.Get());
            h += hck ^ hcv;
        }
    }
    *hashCode = h;
    return NOERROR;
}

ECode CConcurrentHashMap::IsEmpty(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    *result = SumCount() <= 0LL; // ignore transient negative values
    return NOERROR;
}

ECode CConcurrentHashMap::GetKeySet(
    /* [out] */ ISet** keySet)
{
    VALIDATE_NOT_NULL(keySet)

    AutoPtr<KeySetView> ks = mKeySet;
    AutoPtr<KeySetView> res = (ks != NULL) ? ks : (mKeySet = new KeySetView(this, NULL));
    *keySet = (ISet*)res;
    REFCOUNT_ADD(*keySet)
    return NOERROR;
}

ECode CConcurrentHashMap::Put(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* value,
    /* [out] */ IInterface** oldValue)
{
    VALIDATE_NOT_NULL(oldValue);
    return PutVal(key, value, FALSE, oldValue);
}

ECode CConcurrentHashMap::Put(
    /* [in] */ IInterface* key,
    /* [in] */ IInterface* value)
{
    AutoPtr<IInterface> oldValue;
    return PutVal(key, value, FALSE, (IInterface**)&oldValue);
}

ECode CConcurrentHashMap::PutAll(
    /* [in] */ IMap* map)
{
    Int32 s;
    map->GetSize(&s);
    TryPresize(s);
    AutoPtr<ISet> outset;
    map->GetEntrySet((ISet**)&outset);
    AutoPtr<IIterator> it;
    outset->GetIterator((IIterator**)&it);
    Boolean hasNext;
    while (it->HasNext(&hasNext), hasNext) {
        AutoPtr<IInterface> obj;
        it->GetNext((IInterface**)&obj);
        AutoPtr<IMapEntry> e = IMapEntry::Probe(obj);
        AutoPtr<IInterface> keyface;
        AutoPtr<IInterface> valueface;
        e->GetKey((IInterface**)&keyface);
        e->GetValue((IInterface**)&valueface);
        AutoPtr<IInterface> oldValue;
        PutVal(keyface, valueface, FALSE, (IInterface**)&oldValue);
    }
    return NOERROR;
}

ECode CConcurrentHashMap::Remove(
    /* [in] */ IInterface* key,
    /* [out] */ IInterface** value)
{
    VALIDATE_NOT_NULL(value)
    AutoPtr<IInterface> res = ReplaceNode(key, NULL, NULL);
    *value = res;
    REFCOUNT_ADD(*value)
    return NOERROR;
}

ECode CConcurrentHashMap::Remove(
    /* [in] */ IInterface* key)
{
    ReplaceNode(key, NULL, NULL);
    return NOERROR;
}

ECode CConcurrentHashMap::GetSize(
    /* [out] */ Int32* outsize)
{
    VALIDATE_NOT_NULL(outsize)

    Int64 n = SumCount();
    *outsize = ((n < 0L) ? 0 :
        (n > (Int64)Elastos::Core::Math::INT32_MAX_VALUE) ? Elastos::Core::Math::INT32_MAX_VALUE :
        (Int32)n);
    return NOERROR;
}

ECode CConcurrentHashMap::GetValues(
    /* [out] */ ICollection** value)
{
    VALIDATE_NOT_NULL(value)

    AutoPtr<ValuesView> vs = mValues;
    *value = (vs != NULL) ? ICollection::Probe(vs) : ICollection::Probe((mValues = new ValuesView(this)));
    REFCOUNT_ADD(*value)
    return NOERROR;
}

ECode CConcurrentHashMap::Keys(
    /* [out] */ IEnumeration** outenum)
{
    VALIDATE_NOT_NULL(outenum)

    AutoPtr<ArrayOf<Node*> > t;
    Int32 f = (t = mTable) == NULL ? 0 : t->GetLength();
    AutoPtr<KeyIterator> res = new KeyIterator(t, f, 0, f, this);
    *outenum = IEnumeration::Probe(res);
    REFCOUNT_ADD(*outenum)
    return NOERROR;
}

ECode CConcurrentHashMap::Elements(
    /* [out] */ IEnumeration** outenum)
{
    VALIDATE_NOT_NULL(outenum)

    AutoPtr<ArrayOf<Node*> > t;
    Int32 f = (t = mTable) == NULL ? 0 : t->GetLength();
    AutoPtr<ValueIterator> res = new ValueIterator(t, f, 0, f, this);
    *outenum = (IEnumeration*) res->Probe(EIID_IEnumeration);
    REFCOUNT_ADD(*outenum)
    return NOERROR;
}

ECode CConcurrentHashMap::WriteObject(
    /* [in] */ IObjectOutputStream* s)
{
    // For serialization compatibility
    // Emulate segment calculation from previous version of this class
    Int32 sshift = 0;
    Int32 ssize = 1;
    while (ssize < DEFAULT_CONCURRENCY_LEVEL) {
        ++sshift;
        ssize <<= 1;
    }
    Int32 segmentShift = 32 - sshift;
    Int32 segmentMask = ssize - 1;
    AutoPtr<IArrayOf> segments;
    CArrayOf::New(EIID_IReentrantLock, DEFAULT_CONCURRENCY_LEVEL, (IArrayOf**)&segments);
    for (Int32 i = 0; i < DEFAULT_CONCURRENCY_LEVEL; ++i) {
        segments->Set(i, (IReentrantLock*)new Segment(LOAD_FACTOR));
    }
    AutoPtr<IObjectOutputStreamPutField> field;
    s->PutFields((IObjectOutputStreamPutField**)&field);
    field->Put(String("segments"), segments);
    field->Put(String("segmentShift"), segmentShift);
    field->Put(String("segmentMask"), segmentMask);
    s->WriteFields();

    AutoPtr<ArrayOf<Node*> > t;
    if ((t = mTable) != NULL) {
        AutoPtr<Traverser> it = new Traverser(t, t->GetLength(), 0, t->GetLength());
        for (AutoPtr<Node> p; (p = it->Advance()) != NULL; ) {
            IObjectOutput::Probe(s)->WriteObject(p->mKey);
            IObjectOutput::Probe(s)->WriteObject(p->mVal);
        }
    }
    IObjectOutput::Probe(s)->WriteObject(NULL);
    IObjectOutput::Probe(s)->WriteObject(NULL);
    return NOERROR;
}

ECode CConcurrentHashMap::ReadObject(
    /* [in] */ IObjectInputStream* s)
{
    /*
     * To improve performance in typical cases, we create nodes
     * while reading, then place in table once size is known.
     * However, we must also validate uniqueness and deal with
     * overpopulated bins while doing so, which requires
     * specialized versions of putVal mechanics.
     */
    mSizeCtl = -1; // force exclusion for table construction
    s->DefaultReadObject();
    Int64 size = 0L;
    AutoPtr<Node> p;
    for (;;) {
        AutoPtr<IInterface> k, v;
        IObjectInput::Probe(s)->ReadObject((IInterface**)&k);
        IObjectInput::Probe(s)->ReadObject((IInterface**)&v);
        if (k != NULL && v != NULL) {
            Int32 hc = Object::GetHashCode(k);
            p = new Node(Spread(hc), k, v, p);
            ++size;
        }
        else {
            break;
        }
    }
    if (size == 0LL) {
        mSizeCtl = 0;
    }
    else {
        Int32 n;
        if (size >= (Int64)(((UInt32)MAXIMUM_CAPACITY) >> 1)) {
            n = MAXIMUM_CAPACITY;
        }
        else {
            Int32 sz = (Int32)size;
            n = TableSizeFor(sz + (((UInt32)sz) >> 1) + 1);
        }
        AutoPtr<ArrayOf<Node*> > tab = ArrayOf<Node*>::Alloc(n);
        Int32 mask = n - 1;
        Int64 added = 0LL;
        while (p != NULL) {
            Boolean insertAtFront;
            AutoPtr<Node> next = p->mNext, first;
            Int32 h = p->mHash, j = h & mask;
            if ((first = TabAt(tab, j)) == NULL) {
                insertAtFront = TRUE;
            }
            else {
                AutoPtr<IInterface> k = p->mKey;
                if (first->mHash < 0) {
                    AutoPtr<TreeBin> t = (TreeBin*)ITreeBin::Probe(first);
                    if (t->PutTreeVal(h, k, p->mVal) == NULL) {
                        ++added;
                    }
                    insertAtFront = FALSE;
                }
                else {
                    Int32 binCount = 0;
                    insertAtFront = TRUE;
                    AutoPtr<Node> q; AutoPtr<IInterface> qk;
                    for (q = first; q != NULL; q = q->mNext) {
                        if (q->mHash == h &&
                            ((qk = q->mKey).Get() == k ||
                             (qk != NULL && Object::Equals(k, qk)))) {
                            insertAtFront = FALSE;
                            break;
                        }
                        ++binCount;
                    }
                    if (insertAtFront && binCount >= TREEIFY_THRESHOLD) {
                        insertAtFront = FALSE;
                        ++added;
                        p->mNext = first;
                        AutoPtr<TreeNode> hd = NULL, tl = NULL;
                        for (q = p; q != NULL; q = q->mNext) {
                            AutoPtr<TreeNode> t = new TreeNode(q->mHash, q->mKey, q->mVal, NULL, NULL);
                            if ((t->mPrev = tl) == NULL) {
                                hd = t;
                            }
                            else {
                                tl->mNext = t;
                            }
                            tl = t;
                        }
                        SetTabAt(tab, j, new TreeBin(hd));
                    }
                }
            }
            if (insertAtFront) {
                ++added;
                p->mNext = first;
                SetTabAt(tab, j, p);
            }
            p = next;
        }
        mTable = tab;
        mSizeCtl = n - (((UInt32)n) >> 2);
        mBaseCount = added;
    }
    return NOERROR;
}

Int64 CConcurrentHashMap::SumCount()
{
    AutoPtr<ArrayOf<CounterCell*> > as = mCounterCells; AutoPtr<CounterCell> a;
    Int64 sum = mBaseCount;
    if (as != NULL) {
        for (Int32 i = 0; i < as->GetLength(); ++i) {
            if ((a = (*as)[i]) != NULL) {
                sum += a->mValue;
            }
        }
    }
    return sum;
}

void CConcurrentHashMap::FullAddCount(
    /* [in] */ Int64 x,
    /* [in] */ CounterHashCode* _hc,
    /* [in] */ Boolean wasUncontended)
{
    AutoPtr<CounterHashCode> hc = _hc;
    Int32 h;
    if (hc == NULL) {
        hc = new CounterHashCode();
        Int32 s;
        sCounterHashCodeGenerator->AddAndGet(SEED_INCREMENT, &s);
        h = hc->mCode = (s == 0) ? 1 : s; // Avoid zero
        pthread_setspecific(sThreadCounterHashCode, hc.Get());
        hc->AddRef();
    }
    else {
        h = hc->mCode;
    }
    Boolean collide = FALSE;                // True if last slot nonempty
    for (;;) {
        AutoPtr<ArrayOf<CounterCell*> > as; AutoPtr<CounterCell> a;
        Int32 n; Int64 v;
        if ((as = mCounterCells) != NULL && (n = as->GetLength()) > 0) {
            if ((a = (*as)[(n - 1) & h]) == NULL) {
                if (mCellsBusy == 0) {            // Try to attach new Cell
                    AutoPtr<CounterCell> r = new CounterCell(x); // Optimistic create
                    if (mCellsBusy == 0 &&
                        CompareAndSwapInt32((volatile int32_t*)&mCellsBusy, 0, 1)) {
                        Boolean created = FALSE;
                        AutoPtr<ArrayOf<CounterCell*> > rs; Int32 m, j;
                        if ((rs = mCounterCells) != NULL &&
                            (m = rs->GetLength()) > 0 &&
                            (*rs)[j = (m - 1) & h] == NULL) {
                            rs->Set(j, r);
                            created = TRUE;
                        }
                        mCellsBusy = 0;
                        if (created) {
                            break;
                        }
                        continue;           // Slot is now non-empty
                    }
                }
                collide = FALSE;
            }
            else if (!wasUncontended) {      // CAS already known to fail
                wasUncontended = TRUE;      // Continue after rehash
            }
            else if (v = a->mValue, CompareAndSwapInt64((volatile int64_t*)&a->mValue, v, v + x)) {
                break;
            }
            else if (mCounterCells != as || n >= NCPU) {
                collide = FALSE;            // At max size or stale
            }
            else if (!collide) {
                collide = TRUE;
            }
            else if (mCellsBusy == 0 &&
                    CompareAndSwapInt32((volatile int32_t*)&mCellsBusy, 0, 1)) {
                if (mCounterCells == as) {// Expand table unless stale
                    AutoPtr<ArrayOf<CounterCell*> > rs = ArrayOf<CounterCell*>::Alloc(n << 1);
                    for (Int32 i = 0; i < n; ++i) {
                        rs->Set(i, (*as)[i]);
                    }
                    mCounterCells = rs;
                }
                mCellsBusy = 0;
                collide = FALSE;
                continue;                   // Retry with expanded table
            }
            h ^= h << 13;                   // Rehash
            h ^= ((UInt32)h) >> 17;
            h ^= h << 5;
        }
        else if (mCellsBusy == 0 && mCounterCells == as &&
                CompareAndSwapInt32((volatile int32_t*)&mCellsBusy, 0, 1)) {
            Boolean init = FALSE;
            if (mCounterCells == as) {
                AutoPtr<ArrayOf<CounterCell*> > rs = ArrayOf<CounterCell*>::Alloc(2);
                rs->Set(h & 1, new CounterCell(x));
                mCounterCells = rs;
                init = TRUE;
            }
            mCellsBusy = 0;
            if (init) {
                break;
            }
        }
        else if (v = mBaseCount, CompareAndSwapInt64((volatile int64_t*)&mBaseCount, v, v + x)) {
            break;                          // Fall back on using base
        }
    }
    hc->mCode = h;                            // Record index for next time
}

} // namespace Concurrent
} // namespace Utility
} // namespace Elastos
