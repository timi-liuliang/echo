/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_FOUNDATION_PSMUTEX_H
#define PX_FOUNDATION_PSMUTEX_H

#include "PsAllocator.h"

/*
 * This <new> inclusion is a best known fix for gcc 4.4.1 error:
 * Creating object file for apex/src/PsAllocator.cpp ...
 * In file included from apex/include/PsFoundation.h:30,
 *                from apex/src/PsAllocator.cpp:26:
 * apex/include/PsMutex.h: In constructor  'physx::shdfnd::MutexT<Alloc>::MutexT(const Alloc&)':
 * apex/include/PsMutex.h:92: error: no matching function for call to 'operator new(unsigned int,
 * physx::shdfnd::MutexImpl*&)'
 * <built-in>:0: note: candidates are: void* operator new(unsigned int)
 */
#include <new>

namespace physx
{
namespace shdfnd
{
    class PX_FOUNDATION_API MutexImpl
    {
    public:

        /**
        The constructor for Mutex creates a mutex. It is initially unlocked.
        */
        MutexImpl();

        /**
        The destructor for Mutex deletes the mutex.
        */
        ~MutexImpl();

        /**
        Acquire (lock) the mutex. If the mutex is already locked
        by another thread, this method blocks until the mutex is
        unlocked.
        */
        void lock();

        /**
        Acquire (lock) the mutex. If the mutex is already locked
        by another thread, this method returns false without blocking.
        */
        bool trylock();

        /**
        Release (unlock) the mutex.
        */
        void unlock();

        /**
        Size of this class.
        */
		static const PxU32& getSize();
    };

	template <typename Alloc = ReflectionAllocator<MutexImpl> >
	class MutexT : protected Alloc
	{
		PX_NOCOPY(MutexT)
	public:

		class ScopedLock
		{
			MutexT<Alloc>& mMutex;
			PX_NOCOPY(ScopedLock)
		public:
			PX_INLINE	ScopedLock(MutexT<Alloc> &mutex): mMutex(mutex) { mMutex.lock(); }
			PX_INLINE	~ScopedLock() { mMutex.unlock(); }
		};

		/**
		The constructor for Mutex creates a mutex. It is initially unlocked.
		*/
		MutexT(const Alloc& alloc = Alloc())
			: Alloc(alloc)
		{
			mImpl = (MutexImpl*)Alloc::allocate(MutexImpl::getSize(), __FILE__, __LINE__);
			PX_PLACEMENT_NEW(mImpl, MutexImpl)();
		}

		/**
		The destructor for Mutex deletes the mutex.
		*/
		~MutexT()
		{
			mImpl->~MutexImpl();
			Alloc::deallocate(mImpl);
		}

		/**
		Acquire (lock) the mutex. If the mutex is already locked
		by another thread, this method blocks until the mutex is
		unlocked.
		*/
		void lock()		const	{ mImpl->lock(); }

		/**
		Acquire (lock) the mutex. If the mutex is already locked
		by another thread, this method returns false without blocking,
		returns true if lock is successfully acquired
		*/
		bool trylock()	const	{ return mImpl->trylock(); }

		/**
		Release (unlock) the mutex, the calling thread must have 
		previously called lock() or method will error
		*/
		void unlock()	const	{ mImpl->unlock(); }

	private:
		MutexImpl* mImpl;
	};

    class PX_FOUNDATION_API ReadWriteLock
    {
		PX_NOCOPY(ReadWriteLock)
    public:

		ReadWriteLock();
        ~ReadWriteLock();

        void lockReader();
        void lockWriter();

        void unlockReader();
        void unlockWriter();

    private:
        class ReadWriteLockImpl*    mImpl;
    };

	class ScopedReadLock
	{
		PX_NOCOPY(ScopedReadLock)
	public:
		PX_INLINE	ScopedReadLock(ReadWriteLock& lock): mLock(lock)	{			mLock.lockReader(); 	}
		PX_INLINE	~ScopedReadLock()									{			mLock.unlockReader();	}

	private:
		ReadWriteLock& mLock;
	};

	class ScopedWriteLock
	{
		PX_NOCOPY(ScopedWriteLock)
	public:
		PX_INLINE	ScopedWriteLock(ReadWriteLock& lock): mLock(lock)	{		mLock.lockWriter(); 	}
		PX_INLINE	~ScopedWriteLock()									{		mLock.unlockWriter();	}

	private:
		ReadWriteLock& mLock;
	};

	typedef MutexT<> Mutex;

	/*
	 * Use this type of lock for mutex behaviour that must operate on SPU and PPU
	 * On non-PS3 platforms, it is implemented using Mutex
	 */
#ifndef PX_PS3
	class AtomicLock
	{
		Mutex mMutex;
		PX_NOCOPY(AtomicLock)

	public:
		AtomicLock()
		{
		}

		bool lock()
		{
			mMutex.lock();
			return true;
		}

		bool trylock()
		{
			return mMutex.trylock();
		}

		bool unlock()
		{
			mMutex.unlock();
			return true;
		}
	};

	class AtomicLockCopy
	{
		AtomicLock* pLock;

	public:
		AtomicLockCopy() : pLock(NULL)
		{
		}
		
		AtomicLockCopy& operator = (AtomicLock& lock)
		{
			pLock = &lock;
			return *this;
		}

		bool lock()
		{
			return pLock->lock();
		}

		bool trylock()
		{
			return pLock->trylock();
		}

		bool unlock()
		{
			return pLock->unlock();
		}

	};
#else
	struct AtomicLockImpl
	{
		PX_ALIGN(128, PxU32 m_Lock);
		PxI32 m_LockId;
		PxU32 m_LockCount;

		AtomicLockImpl();
	};
	class AtomicLock
	{
		friend class AtomicLockCopy;
		AtomicLockImpl* m_pImpl;
public:

		AtomicLock();

		~AtomicLock();

		bool lock();

		bool trylock();

		bool unlock();
	};


	// if an AtomicLock is copied and then the copy goes out of scope, it'll delete the atomic primitive
	// (just a 128-byte aligned int) and cause a crash when it tries to delete it again
	// This class just uses the atomic primitive without releasing it in the end.

	class AtomicLockCopy
	{
		AtomicLockImpl* m_pImpl;
public:

		AtomicLockCopy() : m_pImpl(NULL)
		{
		}

		AtomicLockCopy(const AtomicLock& lock) : m_pImpl(lock.m_pImpl)
		{
		}

		~AtomicLockCopy()
		{
		}

		AtomicLockCopy& operator = (const AtomicLock& lock)
		{
			m_pImpl = lock.m_pImpl;
			return *this;
		}

		bool lock();

		bool trylock();

		bool unlock();
	};
#endif

#ifndef PX_PS3

	class AtomicRwLock
	{
		ReadWriteLock m_Lock;
		PX_NOCOPY(AtomicRwLock)

	public:

		AtomicRwLock()
		{
		}

		void lockReader()
		{
			m_Lock.lockReader();
		}
        void lockWriter()
		{
			m_Lock.lockWriter();
		}

		bool tryLockReader()
		{
			//Todo - implement this
			m_Lock.lockReader();
			return true;
		}

        void unlockReader()
		{
			m_Lock.unlockReader();
		}
        void unlockWriter()
		{
			m_Lock.unlockWriter();
		}
	};
#else

	struct AtomicRwLockImpl
	{
		PX_ALIGN(128, volatile PxU32 m_Lock);
		PX_ALIGN(128, volatile PxU32 m_ReadCounter);
		PxI32 m_LockId;
		PxU32 m_LockCount;

		AtomicRwLockImpl();
	};

	class AtomicRwLock
	{
		PX_NOCOPY(AtomicRwLock)
		AtomicRwLockImpl* m_pImpl;
	public:

		AtomicRwLock();

		~AtomicRwLock();

		void lockReader();

		bool tryLockReader();

        void lockWriter();

        void unlockReader();

        void unlockWriter();
	};

#endif

	class ScopedAtomicLock
	{
		PX_INLINE	ScopedAtomicLock(AtomicLock& lock): mLock(lock)	{			mLock.lock(); 	}
		PX_INLINE	~ScopedAtomicLock()								{			mLock.unlock();	}

		PX_NOCOPY(ScopedAtomicLock)
	private:
		AtomicLock& mLock;
	};

} // namespace shdfnd
} // namespace physx

#endif
