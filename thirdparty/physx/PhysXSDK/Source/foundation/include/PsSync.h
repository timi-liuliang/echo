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


#ifndef PX_FOUNDATION_PSSYNC_H
#define PX_FOUNDATION_PSSYNC_H

#include "PsAllocator.h"

namespace physx
{
namespace shdfnd
{
	/*!
	Implementation notes:
	* - Calling set() on an already signaled Sync does not change its state.
	* - Calling reset() on an already reset Sync does not change its state.
	* - Calling set() on a reset Sync wakes all waiting threads (potential for thread contention).
	* - Calling wait() on an already signaled Sync will return true immediately.
	* - NOTE: be careful when pulsing an event with set() followed by reset(), because a 
	*   thread that is not waiting on the event will miss the signal.
	*/
	class PX_FOUNDATION_API SyncImpl
	{
	public:

		static const PxU32 waitForever = 0xffffffff;

		SyncImpl();

		~SyncImpl();

		/** Wait on the object for at most the given number of ms. Returns 
		*  true if the object is signaled. Sync::waitForever will block forever 
		*  or until the object is signaled.
		*/

		bool wait(PxU32 milliseconds = waitForever);

		/** Signal the synchronization object, waking all threads waiting on it */

		void set();

		/** Reset the synchronization object */

		void reset();

		 /**
        Size of this class.
        */
		static const PxU32& getSize();
	};

	
	/*!
	Implementation notes:
	* - Calling set() on an already signaled Sync does not change its state.
	* - Calling reset() on an already reset Sync does not change its state.
	* - Calling set() on a reset Sync wakes all waiting threads (potential for thread contention).
	* - Calling wait() on an already signaled Sync will return true immediately.
	* - NOTE: be careful when pulsing an event with set() followed by reset(), because a 
	*   thread that is not waiting on the event will miss the signal.
	*/
	template <typename Alloc = ReflectionAllocator<SyncImpl> >
	class SyncT: protected Alloc
	{
	public:

		static const PxU32 waitForever = SyncImpl::waitForever;

		SyncT(const Alloc& alloc = Alloc())
			: Alloc(alloc)
		{
			mImpl = reinterpret_cast<SyncImpl*>(Alloc::allocate(SyncImpl::getSize(), __FILE__, __LINE__));
			PX_PLACEMENT_NEW(mImpl, SyncImpl)();
		}

		~SyncT()
		{
			mImpl->~SyncImpl();
			Alloc::deallocate(mImpl);
		}

		/** Wait on the object for at most the given number of ms. Returns 
		*  true if the object is signaled. Sync::waitForever will block forever 
		*  or until the object is signaled.
		*/

		bool wait(PxU32 milliseconds = SyncImpl::waitForever)	{ return mImpl->wait(milliseconds); }

		/** Signal the synchronization object, waking all threads waiting on it */

		void set()												{ mImpl->set(); }

		/** Reset the synchronization object */

		void reset()											{ mImpl->reset(); }


	private:
		class SyncImpl *mImpl;
	};

	typedef SyncT<> Sync;

} // namespace shdfnd
} // namespace physx

#endif
