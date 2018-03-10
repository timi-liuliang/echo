/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RW_LOCKABLE_H
#define APEX_RW_LOCKABLE_H

#include "NxApexRWLockable.h"
#include "PsThread.h"
#include "PsMutex.h"
#include "PsHashMap.h"

namespace physx
{
namespace apex
{

struct ThreadReadWriteCount 
{
	ThreadReadWriteCount() : value(0) {}
	union {
		struct {
			PxU8 readDepth;			// depth of re-entrant reads
			PxU8 writeDepth;		// depth of re-entrant writes 
			PxU8 readLockDepth;		// depth of read-locks
			PxU8 writeLockDepth;	// depth of write-locks
		} counters;
		PxU32 value;
	};
};

class ApexRWLockable : public NxApexRWLockable
{
public:
	ApexRWLockable();
	virtual ~ApexRWLockable();

	virtual void acquireReadLock(const char *fileName, const PxU32 lineno) const;
	virtual void acquireWriteLock(const char *fileName, const PxU32 lineno)const;
	virtual void releaseReadLock(void) const;
	virtual void releaseWriteLock(void) const;
	virtual physx::PxU32 getReadWriteErrorCount() const;
	bool startWrite(bool allowReentry);
	void stopWrite(bool allowReentry);
	physx::shdfnd::Thread::Id getCurrentWriter() const;

	bool startRead() const;
	void stopRead() const;

	void setEnabled(bool);
	bool isEnabled() const;
private:
	bool											mEnabled;
	mutable volatile physx::shdfnd::Thread::Id		mCurrentWriter;
	mutable physx::shdfnd::ReadWriteLock			mRWLock;
	volatile PxI32									mConcurrentWriteCount;
	mutable volatile PxI32							mConcurrentReadCount;
	mutable volatile PxI32							mConcurrentErrorCount;
	physx::shdfnd::Mutex							mDataLock;
	typedef physx::shdfnd::HashMap<physx::shdfnd::ThreadImpl::Id, ThreadReadWriteCount> DepthsHashMap_t;
	mutable DepthsHashMap_t							mData;
};

#define APEX_RW_LOCKABLE_BOILERPLATE								\
	virtual void acquireReadLock(const char *fileName, const PxU32 lineno) const \
	{																\
		ApexRWLockable::acquireReadLock(fileName, lineno);					\
	}																\
	virtual void acquireWriteLock(const char *fileName, const PxU32 lineno) const\
	{																\
		ApexRWLockable::acquireWriteLock(fileName, lineno);			\
	}																\
	virtual void releaseReadLock(void)									const\
	{																\
		ApexRWLockable::releaseReadLock();								\
	}																\
	virtual void releaseWriteLock(void)									const\
	{																\
		ApexRWLockable::releaseWriteLock();								\
	}																\
	virtual physx::PxU32 getReadWriteErrorCount() const				\
	{																\
		return ApexRWLockable::getReadWriteErrorCount();			\
	}																\
	bool startWrite(bool allowReentry)								\
	{																\
		return ApexRWLockable::startWrite(allowReentry);			\
	}																\
	void stopWrite(bool allowReentry)								\
	{																\
		ApexRWLockable::stopWrite(allowReentry);					\
	}																\
	bool startRead() const											\
	{																\
		return ApexRWLockable::startRead();							\
	}																\
	void stopRead() const											\
	{																\
		ApexRWLockable::stopRead();									\
	}																\

class ApexRWLockableScopedDisable
{
public:
	ApexRWLockableScopedDisable(NxApexRWLockable*);
	~ApexRWLockableScopedDisable();
private:
	ApexRWLockableScopedDisable(const ApexRWLockableScopedDisable&);
	ApexRWLockableScopedDisable& operator=(const ApexRWLockableScopedDisable&);

	ApexRWLockable* mLockable;
};

#define APEX_RW_LOCKABLE_SCOPED_DISABLE(lockable) ApexRWLockableScopedDisable __temporaryDisable(lockable);

}
}

#endif // APEX_RW_LOCKABLE_H
