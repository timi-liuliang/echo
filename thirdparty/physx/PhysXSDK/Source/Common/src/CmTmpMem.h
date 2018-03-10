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


#ifndef PX_PHYSICS_COMMON_TMPMEM
#define PX_PHYSICS_COMMON_TMPMEM

#include "CmPhysXCommon.h"
#include "PsAllocator.h"

namespace physx
{
namespace Cm
{
	// dsequeira: we should be able to use PX_ALLOCA or Ps::InlineArray for this, but both seem slightly flawed:
	//
	//	PX_ALLOCA has non-configurable fallback threshold and uses _alloca, which means the allocation is necessarily
	//	function-scope rather than block-scope (sometimes useful, mostly not.)
	//
	//	Ps::InlineArray touches all memory on resize (a general flaw in the array class which badly needs fixing)
	//
	//	Todo: fix both the above issues.

	template<typename T, PxU32 stackLimit>
	class TmpMem
	{

	public:
		PX_FORCE_INLINE TmpMem(PxU32 size):
		  mPtr(size<=stackLimit?mStackBuf : reinterpret_cast<T*>(PX_ALLOC(size*sizeof(T), PX_DEBUG_EXP("char"))))
		{
		}

		PX_FORCE_INLINE ~TmpMem()
		{
			if(mPtr!=mStackBuf)
				PX_FREE(mPtr);
		}

		PX_FORCE_INLINE T& operator*() const
	    {
			return *mPtr;
		}

		PX_FORCE_INLINE T* operator->() const
		{
			return mPtr;
		}
	
		PX_FORCE_INLINE T& operator[](PxU32 index)
		{
			return mPtr[index];
		}

		T* getBase()
		{
			return mPtr;
		}

	private:
		T mStackBuf[stackLimit];
		T* mPtr;
	};
}

}

#endif
