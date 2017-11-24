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
#include "CConcurrentLinkedQueue.h"
#include "CArrayList.h"
#include <Math.h>
#include <cutils/atomic.h>
#include <cutils/atomic-inline.h>

using Elastos::Core::Math;
using Elastos::IO::IObjectInput;
using Elastos::IO::IObjectOutput;
using Elastos::IO::EIID_ISerializable;
using Elastos::Utility::IArrayList;
using Elastos::Utility::CArrayList;

namespace Elastos {
namespace Utility {
namespace Concurrent {

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

static Boolean CompareAndSwapObject(volatile int32_t* address, Object* expect, Object* update)
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

static void PutOrderedObject(volatile int32_t* address, Object* newValue)
{
    ANDROID_MEMBAR_STORE();
    Object* oldValue = reinterpret_cast<Object*>(*address);
    *address = reinterpret_cast<int32_t>(newValue);
    REFCOUNT_ADD(newValue);
    REFCOUNT_RELEASE(oldValue);
}

//====================================================================
// CConcurrentLinkedQueue::Node::
//====================================================================
CConcurrentLinkedQueue::Node::Node(
    /* [in] */ IInterface* item)
    : mItem(item)
{
//    UNSAFE.putObject(this, itemOffset, item);
}

Boolean CConcurrentLinkedQueue::Node::CasItem(
    /* [in] */ IInterface* cmp,
    /* [in] */ IInterface* val)
{
    return CompareAndSwapObject((volatile int32_t*)&mItem, cmp, val);
}

void CConcurrentLinkedQueue::Node::LazySetNext(
    /* [in] */ Node* val)
{
    PutOrderedObject((volatile int32_t*)&mNext, (Object*)val);
}

Boolean CConcurrentLinkedQueue::Node::CasNext(
    /* [in] */ Node* cmp,
    /* [in] */ Node* val)
{
    return CompareAndSwapObject((volatile int32_t*)&mNext, (Object*)cmp, (Object*)val);
}


//====================================================================
// CConcurrentLinkedQueue::Itr::
//====================================================================
CAR_INTERFACE_IMPL(CConcurrentLinkedQueue::Itr, Object, IIterator)

CConcurrentLinkedQueue::Itr::Itr(
    /* [in] */ CConcurrentLinkedQueue* owner)
{
    Advance();
    mOwner = owner;
}

AutoPtr<IInterface> CConcurrentLinkedQueue::Itr::Advance()
{
    mLastRet = mNextNode;
    AutoPtr<IInterface> x = mNextItem;

    AutoPtr<Node> pred, p;
    if (mNextNode == NULL) {
        p = mOwner->First();
        pred = NULL;
    }
    else {
        pred = mNextNode;
        p = mOwner->Succ(mNextNode);
    }

    for (;;) {
        if (p == NULL) {
            mNextNode = NULL;
            mNextItem = NULL;
            return x;
        }
        AutoPtr<IInterface> item = p->mItem;
        if (item != NULL) {
            mNextNode = p;
            mNextItem = item;
            return x;
        }
        else {
            // skip over nulls
            AutoPtr<Node> next = mOwner->Succ(p);
            if (pred != NULL && next != NULL)
                pred->CasNext(p, next);
            p = next;
        }
    }
}

ECode CConcurrentLinkedQueue::Itr::HasNext(
    /* [out] */ Boolean* result)
{
    *result = mNextNode != NULL;
    return NOERROR;
}

ECode CConcurrentLinkedQueue::Itr::GetNext(
    /* [out] */ IInterface** object)
{
    if (mNextNode == NULL) {
        return E_NO_SUCH_ELEMENT_EXCEPTION;
    }
    AutoPtr<IInterface> p = Advance();
    *object = p.Get();
    REFCOUNT_ADD(*object);
    return NOERROR;
}

ECode CConcurrentLinkedQueue::Itr::Remove()
{
    AutoPtr<Node> l = mLastRet;
    if (l == NULL) {
        return E_ILLEGAL_STATE_EXCEPTION;
    }
    // rely on a future traversal to relink.
    l->mItem = NULL;
    mLastRet = NULL;
    return NOERROR;
}


//====================================================================
// CConcurrentLinkedQueue::
//====================================================================
CAR_INTERFACE_IMPL(CConcurrentLinkedQueue, AbstractQueue, IConcurrentLinkedQueue, ISerializable)

CAR_OBJECT_IMPL(CConcurrentLinkedQueue)

ECode CConcurrentLinkedQueue::constructor()
{
    mHead = mTail = new Node(NULL);
    return NOERROR;
}

ECode CConcurrentLinkedQueue::constructor(
    /* [in] */ ICollection* c)
{
    AutoPtr<Node> h, t;
    AutoPtr<IIterator> it;
    (IIterable::Probe(c))->GetIterator((IIterator**)&it);
    Boolean isflag = FALSE;
    while ((it->HasNext(&isflag), isflag)) {
        AutoPtr<IInterface> e;
        it->GetNext((IInterface**)&e);
        CheckNotNull(e);
        AutoPtr<Node> newNode = new Node(e);
        if (h == NULL) {
            h = t = newNode;
        }
        else {
            t->LazySetNext(newNode);
            t = newNode;
        }
    }
    if (h == NULL) {
        h = t = new Node(NULL);
    }
    mHead = h;
    mTail = t;
    return NOERROR;
}

ECode CConcurrentLinkedQueue::Add(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* modified)
{
    return Offer(object, modified);
}

void CConcurrentLinkedQueue::UpdateHead(
    /* [in] */ Node* h,
    /* [in] */ Node* p)
{
    if (h != p && CasHead(h, p)) {
        h->LazySetNext(h);
    }
}

AutoPtr<CConcurrentLinkedQueue::Node> CConcurrentLinkedQueue::Succ(
    /* [in] */ Node* p)
{
    AutoPtr<Node> next = p->mNext;
    return (p == next) ? mHead : next;
}

ECode CConcurrentLinkedQueue::Offer(
    /* [in] */ IInterface* e,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    CheckNotNull(e);
    AutoPtr<Node> newNode = new Node(e);

    for (AutoPtr<Node> t = mTail, p = t;;) {
        AutoPtr<Node> q = p->mNext;
        if (q == NULL) {
            // p is last node
            if (p->CasNext(NULL, newNode)) {
                // Successful CAS is the linearization point
                // for e to become an element of this queue,
                // and for newNode to become "live".
                if (p != t) { // hop two nodes at a time
                    CasTail(t, newNode);  // Failure is OK.
                }
                *result = TRUE;
                return NOERROR;
            }
            // Lost CAS race to another thread; re-read next
        }
        else if (p == q) {
            // We have fallen off list.  If tail is unchanged, it
            // will also be off-list, in which case we need to
            // jump to head, from which all live nodes are always
            // reachable.  Else the new tail is a better bet.
            p = (t != (t = mTail)) ? t : mHead;
        }
        else {
            // Check for tail updates after two hops.
            p = (p != t && t != (t = mTail)) ? t : q;
        }
    }
}

ECode CConcurrentLinkedQueue::Poll(
    /* [out] */ IInterface** e)
{
    VALIDATE_NOT_NULL(e);
    *e = NULL;

    RESTARTFROMHEAD:
    for (;;) {
        for (AutoPtr<Node> h = mHead, p = h, q;;) {
            AutoPtr<IInterface> item = p->mItem;

            if (item != NULL && p->CasItem(item, NULL)) {
                // Successful CAS is the linearization point
                // for item to be removed from this queue.
                if (p != h) { // hop two nodes at a time
                    UpdateHead(h, ((q = p->mNext) != NULL) ? q : p);
                }
                *e = item;
                REFCOUNT_ADD(*e);
                return NOERROR;
            }
            else if ((q = p->mNext) == NULL) {
                UpdateHead(h, p);
                return NOERROR;
            }
            else if (p == q) {
                goto RESTARTFROMHEAD;
            }
            else {
                p = q;
            }
        }
    }
    return NOERROR;
}

ECode CConcurrentLinkedQueue::Peek(
    /* [out] */ IInterface** e)
{
    VALIDATE_NOT_NULL(e);
    *e = NULL;

    RESTARTFROMHEAD:
    for (;;) {
        for (AutoPtr<Node> h = mHead, p = h, q;;) {
            AutoPtr<IInterface> item = p->mItem;
            if (item != NULL || (q = p->mNext) == NULL) {
                UpdateHead(h, p);
                *e = item;
                REFCOUNT_ADD(*e);
                return NOERROR;
            }
            else if (p == q) {
                goto RESTARTFROMHEAD;
            }
            else {
                p = q;
            }
        }
    }
    return NOERROR;
}

AutoPtr<CConcurrentLinkedQueue::Node> CConcurrentLinkedQueue::First()
{
    RESTARTFROMHEAD:
    for (;;) {
        for (AutoPtr<Node> h = mHead, p = h, q;;) {
            Boolean hasItem = (p->mItem != NULL);
            if (hasItem || (q = p->mNext) == NULL) {
                UpdateHead(h, p);
                return hasItem ? p : NULL;
            }
            else if (p == q) {
                goto RESTARTFROMHEAD;
            }
            else {
                p = q;
            }
        }
    }
    return NULL;
}

ECode CConcurrentLinkedQueue::IsEmpty(
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = First() == NULL;
    return NOERROR;
}

ECode CConcurrentLinkedQueue::GetSize(
    /* [out] */ Int32* size)
{
    VALIDATE_NOT_NULL(size);
    Int32 count = 0;
    for (AutoPtr<Node> p = First(); p != NULL; p = Succ(p)) {
        if (p->mItem != NULL) {
            // Collection.size() spec says to max out
            if (++count == Elastos::Core::Math::INT32_MAX_VALUE) {
                break;
            }
        }
    }
    *size = count;
    return NOERROR;
}

ECode CConcurrentLinkedQueue::Contains(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    if (object == NULL) {
        *result = FALSE;
        return NOERROR;
    }
    for (AutoPtr<Node> p = First(); p != NULL; p = Succ(p)) {
        AutoPtr<IInterface> item = p->mItem;
        if (item != NULL && Object::Equals(object, item)) {
            *result = TRUE;
            return NOERROR;
        }
    }
    *result = FALSE;
    return NOERROR;
}

ECode CConcurrentLinkedQueue::Remove(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified);
    if (object == NULL) {
        *modified = FALSE;
        return NOERROR;
    }
    AutoPtr<Node> pred;
    for (AutoPtr<Node> p = First(); p != NULL; p = Succ(p)) {
        AutoPtr<IInterface> item = p->mItem;
        if (item != NULL &&
            Object::Equals(object, item) &&
            p->CasItem(item, NULL)) {
            AutoPtr<Node> next = Succ(p);
            if (pred != NULL && next != NULL) {
                pred->CasNext(p, next);
            }
            *modified = TRUE;
            return NOERROR;
        }
        pred = p;
    }
    *modified = FALSE;
    return NOERROR;
}

ECode CConcurrentLinkedQueue::AddAll(
    /* [in] */ ICollection* collection,
    /* [out] */ Boolean* modified)
{
    VALIDATE_NOT_NULL(modified);
    if (collection == (ICollection*)this) {
        // As historically specified in AbstractQueue#addAll
        return E_ILLEGAL_ARGUMENT_EXCEPTION;
    }

    // Copy c into a private chain of Nodes
    AutoPtr<Node> beginningOfTheEnd, last;
    AutoPtr<IIterator> it;
    IIterable::Probe(collection)->GetIterator((IIterator**)&it);
    Boolean isflag = FALSE;
    while ((it->HasNext(&isflag), isflag)) {
        AutoPtr<IInterface> e;
        it->GetNext((IInterface**)&e);
        CheckNotNull(e);
        AutoPtr<Node> newNode = new Node(e);
        if (beginningOfTheEnd == NULL) {
            beginningOfTheEnd = last = newNode;
        }
        else {
            last->LazySetNext(newNode);
            last = newNode;
        }
    }
    if (beginningOfTheEnd == NULL) {
        *modified = FALSE;
        return NOERROR;
    }

    // Atomically append the chain at the tail of this collection
    for (AutoPtr<Node> t = mTail, p = t;;) {
        AutoPtr<Node> q = p->mNext;
        if (q == NULL) {
            // p is last node
            if (p->CasNext(NULL, beginningOfTheEnd)) {
                // Successful CAS is the linearization point
                // for all elements to be added to this queue.
                if (!CasTail(t, last)) {
                    // Try a little harder to update tail,
                    // since we may be adding many elements.
                    t = mTail;
                    if (last->mNext == NULL)
                        CasTail(t, last);
                }
                *modified = TRUE;
                return NOERROR;
            }
            // Lost CAS race to another thread; re-read next
        }
        else if (p == q) {
            // We have fallen off list.  If tail is unchanged, it
            // will also be off-list, in which case we need to
            // jump to head, from which all live nodes are always
            // reachable.  Else the new tail is a better bet.
            p = (t != (t = mTail)) ? t : mHead;
        }
        else {
            // Check for tail updates after two hops.
            p = (p != t && t != (t = mTail)) ? t : q;
        }
    }
    return NOERROR;
}

ECode CConcurrentLinkedQueue::ToArray(
    /* [out, callee] */ ArrayOf<IInterface*>** array)
{
    VALIDATE_NOT_NULL(array);
    // Use ArrayList to deal with resizing.
    AutoPtr<ICollection> al;
    CArrayList::New((ICollection**)&al);
    for (AutoPtr<Node> p = First(); p != NULL; p = Succ(p)) {
        AutoPtr<IInterface> item = p->mItem;
        if (item != NULL) {
            Boolean b = FALSE;
            al->Add(item, &b);
        }
    }
    return al->ToArray(array);
}

ECode CConcurrentLinkedQueue::ToArray(
    /* [in] */ ArrayOf<IInterface*>* inArray,
    /* [out, callee] */ ArrayOf<IInterface*>** outArray)
{
    VALIDATE_NOT_NULL(outArray);
    // try to use sent-in array
    Int32 k = 0;
    AutoPtr<Node> p;
    for (p = First(); p != NULL && k < inArray->GetLength(); p = Succ(p)) {
        AutoPtr<IInterface> item = p->mItem;
        if (item != NULL) {
            inArray->Set(k, item);
            k++;
        }
    }
    if (p == NULL) {
        if (k < inArray->GetLength())
            inArray->Set(k, NULL);
        *outArray = inArray;
        REFCOUNT_ADD(*outArray);
        return NOERROR;
    }

    // If won't fit, use ArrayList version
    AutoPtr<ICollection> al;
    CArrayList::New((ICollection**)&al);
    for (AutoPtr<Node> q = First(); q != NULL; q = Succ(q)) {
        AutoPtr<IInterface> item = q->mItem;
        if (item != NULL) {
            Boolean b = FALSE;
            al->Add(item, &b);
        }
    }
    return al->ToArray(inArray, outArray);
}

ECode CConcurrentLinkedQueue::GetIterator(
    /* [out] */ IIterator** iterator)
{
    VALIDATE_NOT_NULL(iterator);
    AutoPtr<IIterator> p = (IIterator*)new Itr(this);
    *iterator = p;
    REFCOUNT_ADD(*iterator);
    return NOERROR;
}

void CConcurrentLinkedQueue::WriteObject(
    /* [in] */ IObjectOutputStream* s)
{
    // Write out any hidden stuff
    s->DefaultWriteObject();

    // Write out all elements in the proper order.
    for (AutoPtr<Node> p = First(); p != NULL; p = Succ(p)) {
        AutoPtr<IInterface> item = p->mItem;
        if (item != NULL) {
            IObjectOutput::Probe(s)->WriteObject(item);
        }
    }

    // Use trailing null as sentinel
    IObjectOutput::Probe(s)->WriteObject(NULL);
}

void CConcurrentLinkedQueue::ReadObject(
    /* [in] */ IObjectInputStream* s)
{
    s->DefaultReadObject();

    // Read in elements until trailing null sentinel found
    AutoPtr<Node> h, t;
    AutoPtr<IInterface> item;
    while (IObjectInput::Probe(s)->ReadObject((IInterface**)&item), item != NULL) {
        AutoPtr<Node> newNode = new Node(item);
        if (h == NULL) {
            h = t = newNode;
        }
        else {
            t->LazySetNext(newNode);
            t = newNode;
        }
    }
    if (h == NULL) {
        h = t = new Node(NULL);
    }
    mHead = h;
    mTail = t;
}

ECode CConcurrentLinkedQueue::CheckNotNull(
    /* [in] */ IInterface* v)
{
    if (v == NULL) {
        return E_NULL_POINTER_EXCEPTION;
    }
    return NOERROR;
}

Boolean CConcurrentLinkedQueue::CasTail(
    /* [in] */ Node* cmp,
    /* [in] */ Node* val)
{
    return CompareAndSwapObject((volatile int32_t*)&mTail, (Object*)cmp, (Object*)val);
}

Boolean CConcurrentLinkedQueue::CasHead(
    /* [in] */ Node* cmp,
    /* [in] */ Node* val)
{
    return CompareAndSwapObject((volatile int32_t*)&mHead, (Object*)cmp, (Object*)val);
}

ECode CConcurrentLinkedQueue::Equals(
    /* [in] */ IInterface* object,
    /* [out] */ Boolean* result)
{
    return E_NO_SUCH_METHOD_EXCEPTION;
}

ECode CConcurrentLinkedQueue::GetHashCode(
    /* [out] */ Int32* hashCode)
{
    return E_NO_SUCH_METHOD_EXCEPTION;
}

} // namespace Concurrent
} // namespace Utility
} // namespace Elastos
