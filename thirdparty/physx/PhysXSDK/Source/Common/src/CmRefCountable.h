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


#ifndef PX_PHYSICS_COMMON_REFCOUNTABLE
#define PX_PHYSICS_COMMON_REFCOUNTABLE

#include "CmPhysXCommon.h"
#include "PsAtomic.h"
#include "PxAssert.h"
#include "PxMetaData.h"

namespace physx
{
	class PxOutputStream;

namespace Cm
{

	// simple thread-safe reference count
	// when the ref count is zero, the object is in an undefined state (pending delete)

	class RefCountable
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
		RefCountable(const PxEMPTY&) : mRefCount(1)	{}
		static	void	getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
		explicit RefCountable(PxU32 initialCount = 1)
			: mRefCount((PxI32)initialCount)
		{
			PX_ASSERT(mRefCount!=0);
		}

		virtual ~RefCountable() {}

		/**
		Calls 'delete this;'. It needs to be overloaded for classes also deriving from 
		PxBase and call 'Cm::deletePxBase(this);' instead.
		*/
		virtual	void onRefCountZero()
		{
			delete this;
		}

		void incRefCount()
		{
			physx::shdfnd::atomicIncrement(&mRefCount);
			// value better be greater than 1, or we've created a ref to an undefined object
			PX_ASSERT(mRefCount>1);
		}

		void decRefCount()
		{
			PX_ASSERT(mRefCount>0);
			if(physx::shdfnd::atomicDecrement(&mRefCount) == 0)
				onRefCountZero();
		}

		PX_FORCE_INLINE PxU32 getRefCount() const
		{
			return (PxU32)mRefCount;
		}
	private:
		volatile PxI32 mRefCount;
	};


} // namespace Cm

}

#endif
