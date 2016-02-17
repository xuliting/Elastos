
#ifndef __ELASTOS_CORE_AUTOLOCK_H__
#define __ELASTOS_CORE_AUTOLOCK_H__

#include <elastos/core/Object.h>

using Elastos::Core::EIID_ISynchronize;

namespace Elastos {
namespace Core {

class AutoLock
{
public:
    AutoLock(
        /* [in] */ ISynchronize * obj)
        : mLocked(TRUE)
    {
        assert(obj != NULL);
        mSyncObj = obj;
        ASSERT_SUCCEEDED(mSyncObj->Lock());
    }

    AutoLock(
        /* [in] */ IInterface * obj)
        : mLocked(TRUE)
    {
        assert(obj != NULL);
        mSyncObj = (ISynchronize *)obj->Probe(EIID_ISynchronize);
        assert(mSyncObj != NULL);
        ASSERT_SUCCEEDED(mSyncObj->Lock());
    }

    AutoLock(
        /* [in] */ IObject * obj)
        : mLocked(TRUE)
    {
        assert(obj != NULL);
        mSyncObj = (ISynchronize *)obj->Probe(EIID_ISynchronize);
        assert(mSyncObj != NULL);
        ASSERT_SUCCEEDED(mSyncObj->Lock());
    }

    AutoLock(
        /* [in] */ Object * obj)
        : mLocked(TRUE)
    {
        mSyncObj = (ISynchronize *)obj->Probe(EIID_ISynchronize);
        assert(mSyncObj != NULL);
        ASSERT_SUCCEEDED(mSyncObj->Lock());
    }

    AutoLock(
        /* [in] */ Object & obj)
        : mLocked(TRUE)
    {
        mSyncObj = (ISynchronize *)obj.Probe(EIID_ISynchronize);
        assert(mSyncObj != NULL);
        ASSERT_SUCCEEDED(mSyncObj->Lock());
    }

    ~AutoLock()
    {
        assert(mSyncObj != NULL);
        ASSERT_SUCCEEDED(mSyncObj->Unlock());
    }

    // report the state of locking when used as a boolean
    inline operator Boolean () const
    {
        return mLocked;
    }

    // unlock
    inline void SetUnlock()
    {
        mLocked = FALSE;
    }

private:
    AutoLock(const AutoLock&);

private:
    ISynchronize* mSyncObj;
    Boolean mLocked;
};

} // namespace Core
} // namespace Elastos

#ifndef synchronized
#ifdef __GNUC__
#define synchronized(obj)  for(Elastos::Core::AutoLock tmpLockObj##__COUNTER__(obj); tmpLockObj##__COUNTER__; tmpLockObj##__COUNTER__.SetUnlock())
#else
#define synchronized(obj)  for(Elastos::Core::AutoLock tmpLockObj##__LINE__(obj); tmpLockObj##__LINE__; tmpLockObj##__LINE__.SetUnlock())
#endif
#endif

using Elastos::Core::AutoLock;

#endif //__ELASTOS_CORE_AUTOLOCK_H__