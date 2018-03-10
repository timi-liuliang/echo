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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "PsIntrinsics.h"
#include "GuRevisitedRadixBuffered.h"
#include "GuRevisitedRadix.h"

using namespace physx;
using namespace Gu;

RadixSortBuffered::RadixSortBuffered()
: RadixSort()
{
}

RadixSortBuffered::~RadixSortBuffered()
{
	// Release everything
	if(mDeleteRanks)
	{
		PX_FREE_AND_RESET(mRanks2);
		PX_FREE_AND_RESET(mRanks);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Resizes the inner lists.
 *	\param		nb	[in] new size (number of dwords)
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RadixSortBuffered::Resize(PxU32 nb)
{
	if(mDeleteRanks)
	{
		// Free previously used ram
		PX_FREE_AND_RESET(mRanks2);
		PX_FREE_AND_RESET(mRanks);

		// Get some fresh one
		mRanks	= (PxU32*)PX_ALLOC(sizeof(PxU32)*nb, PX_DEBUG_EXP("RadixSortBuffered:mRanks"));
		mRanks2	= (PxU32*)PX_ALLOC(sizeof(PxU32)*nb, PX_DEBUG_EXP("RadixSortBuffered:mRanks2"));
	}

	return true;
}

PX_INLINE void RadixSortBuffered::CheckResize(PxU32 nb)
{
	PxU32 CurSize = CURRENT_SIZE;
	if(nb!=CurSize)
	{
		if(nb>CurSize)	Resize(nb);
		mCurrentSize = nb;
		INVALIDATE_RANKS;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main sort routine.
 *	This one is for integer values. After the call, mRanks contains a list of indices in sorted order, i.e. in the order you may process your data.
 *	\param		input	[in] a list of integer values to sort
 *	\param		nb		[in] number of values to sort, must be < 2^31
 *	\param		hint	[in] RADIX_SIGNED to handle negative values, RADIX_UNSIGNED if you know your input buffer only contains positive values
 *	\return		Self-Reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSortBuffered& RadixSortBuffered::Sort(const PxU32* input, PxU32 nb, RadixHint hint)
{
	// Checkings
	if(!input || !nb || nb&0x80000000)	return *this;

	// Resize lists if needed
	CheckResize(nb);

	//Set histogram buffers.
	PxU32 histogram[1024];
	PxU32* links[256];
	mHistogram1024=histogram;
	mLinks256=links;

	RadixSort::Sort(input,nb,hint);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main sort routine.
 *	This one is for floating-point values. After the call, mRanks contains a list of indices in sorted order, i.e. in the order you may process your data.
 *	\param		input			[in] a list of floating-point values to sort
 *	\param		nb				[in] number of values to sort, must be < 2^31
 *	\return		Self-Reference
 *	\warning	only sorts IEEE floating-point values
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSortBuffered& RadixSortBuffered::Sort(const float* input2, PxU32 nb)
{
	// Checkings
	if(!input2 || !nb || nb&0x80000000)	return *this;

	// Resize lists if needed
	CheckResize(nb);

	//Set histogram buffers.
	PxU32 histogram[1024];
	PxU32* links[256];
	mHistogram1024=histogram;
	mLinks256=links;

	RadixSort::Sort(input2,nb);
	return *this;
}

