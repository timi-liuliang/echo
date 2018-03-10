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

#ifndef GU_RADIX_SORT_H
#define GU_RADIX_SORT_H

#include "CmPhysXCommon.h"
#include "PxSimpleTypes.h"
#include "PxPhysXCommonConfig.h"
#include "PxAssert.h"

namespace physx
{
namespace Gu
{

	enum RadixHint
	{
		RADIX_SIGNED,		//!< Input values are signed
		RADIX_UNSIGNED,		//!< Input values are unsigned

		RADIX_FORCE_DWORD = 0x7fffffff
	};

#define INVALIDATE_RANKS	mCurrentSize|=0x80000000
#define VALIDATE_RANKS		mCurrentSize&=0x7fffffff
#define CURRENT_SIZE		(mCurrentSize&0x7fffffff)
#define INVALID_RANKS		(mCurrentSize&0x80000000)

	class PX_PHYSX_COMMON_API RadixSort
	{
		public:
										RadixSort();
		virtual							~RadixSort();
		// Sorting methods
						RadixSort&		Sort(const PxU32* input, PxU32 nb, RadixHint hint=RADIX_SIGNED);
						RadixSort&		Sort(const float* input, PxU32 nb);

		//! Access to results. mRanks is a list of indices in sorted order, i.e. in the order you may further process your data
		PX_FORCE_INLINE	const PxU32*	GetRanks()			const	{ return mRanks;		}

		//! mIndices2 gets trashed on calling the sort routine, but otherwise you can recycle it the way you want.
		PX_FORCE_INLINE	PxU32*			GetRecyclable()		const	{ return mRanks2;		}

		//! Returns the total number of calls to the radix sorter.
		PX_FORCE_INLINE	PxU32			GetNbTotalCalls()	const	{ return mTotalCalls;	}
		//! Returns the number of eraly exits due to temporal coherence.
		PX_FORCE_INLINE	PxU32			GetNbHits()			const	{ return mNbHits;		}

						bool			SetBuffers(PxU32* ranks0, PxU32* ranks1, PxU32* histogram1024, PxU32** links256);
		private:
										RadixSort(const RadixSort& object);
										RadixSort& operator=(const RadixSort& object);
		protected:
						PxU32			mCurrentSize;		//!< Current size of the indices list
						PxU32*			mRanks;				//!< Two lists, swapped each pass
						PxU32*			mRanks2;
						PxU32*			mHistogram1024;
						PxU32**			mLinks256;
		// Stats
						PxU32			mTotalCalls;		//!< Total number of calls to the sort routine
						PxU32			mNbHits;			//!< Number of early exits due to coherence

						// Stack-radix
						bool			mDeleteRanks;		//!<
	};

	#define StackRadixSort(name, ranks0, ranks1)	\
		RadixSort name;								\
		PxU32 histogramBuffer[1024];				\
		PxU32* linksBuffer[256];					\
		name.SetBuffers(ranks0, ranks1, histogramBuffer, linksBuffer);
}

}

#endif // GU_RADIX_SORT_H
