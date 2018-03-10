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


#include "PsAllocator.h"
#include "PsAtomic.h"
#include "PsSList.h"
#include "PsThread.h"
#include <pthread.h>

#if defined(PX_APPLE_IOS)
#define USE_MUTEX
#endif

namespace physx
{
	namespace shdfnd
	{
		namespace 
		{
#if defined(USE_MUTEX)
			class ScopedMutexLock
			{
				pthread_mutex_t& mMutex;
			public:
				PX_INLINE	ScopedMutexLock(pthread_mutex_t& mutex): mMutex(mutex) 
				{
					pthread_mutex_lock(&mMutex);
				}
				
				PX_INLINE	~ScopedMutexLock() 
				{ 
					pthread_mutex_unlock(&mMutex);
				}
			};
			
			typedef ScopedMutexLock ScopedLock;
#else
			struct ScopedSpinLock
			{
				PX_FORCE_INLINE ScopedSpinLock(volatile PxI32& lock): mLock(lock)	
				{  
					while (__sync_lock_test_and_set(&mLock, 1))
					{
						// spinning without atomics is usually
						// causing less bus traffic. -> only one
						// CPU is modifying the cache line.
						while(lock)
							PxSpinLockPause();
					} 
				}
				
				PX_FORCE_INLINE ~ScopedSpinLock()									
				{  
					__sync_lock_release(&mLock);
				}
			private:
				volatile PxI32& mLock;	
			};
			
			typedef ScopedSpinLock ScopedLock;
#endif
			
			
			struct SListDetail
			{
				SListEntry* head;
#if defined(USE_MUTEX) 
				pthread_mutex_t lock;
#else
				volatile PxI32 lock;	
#endif
			};
			
			template <typename T>
			SListDetail* getDetail(T* impl)
			{
				return reinterpret_cast<SListDetail*>(impl);
			}
		}
		
		SListImpl::SListImpl()
		{
			getDetail(this)->head = NULL;
			
#if defined(USE_MUTEX)
			pthread_mutex_init(&getDetail(this)->lock, NULL);
#else
			getDetail(this)->lock = 0; // 0 == unlocked
#endif
		}
		
		SListImpl::~SListImpl()
		{
#if defined(USE_MUTEX)
			pthread_mutex_destroy(&getDetail(this)->lock); 
#endif
		}
		
		void SListImpl::push(SListEntry* entry)
		{
			ScopedLock lock(getDetail(this)->lock);
			entry->mNext = getDetail(this)->head;
			getDetail(this)->head = entry;
		}
		
		SListEntry* SListImpl::pop()
		{
			ScopedLock lock(getDetail(this)->lock);
			SListEntry* result = getDetail(this)->head;
			if( result != NULL )
				getDetail(this)->head = result->mNext;
			return result;
		}
		
		SListEntry* SListImpl::flush()
		{
			ScopedLock lock(getDetail(this)->lock);
			SListEntry* result = getDetail(this)->head;
			getDetail(this)->head = NULL;
			return result;
		}
		
		static const PxU32 gSize = sizeof(SListDetail);

		const PxU32& SListImpl::getSize() 
		{
			return gSize;
		}
		
	} // namespace shdfnd
} // namespace physx
