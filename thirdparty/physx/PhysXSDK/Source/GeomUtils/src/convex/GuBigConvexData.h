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

#ifndef GU_BIG_CONVEX_DATA_H
#define GU_BIG_CONVEX_DATA_H

#include "PxSimpleTypes.h"

namespace physx
{

class BigConvexDataBuilder;
class PxcHillClimb;
class BigConvexData;

// Data

namespace Gu
{

struct Valency
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	PxU16		mCount;
	PxU16		mOffset;
};
PX_COMPILE_TIME_ASSERT(sizeof(Gu::Valency) == 4);

struct BigConvexRawData
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	// Support vertex map
	PxU16		mSubdiv;		// "Gaussmap" subdivision
	PxU16		mNbSamples;		// Total #samples in gaussmap PT: this is not even needed at runtime!

	PxU8*		mSamples;
	PX_FORCE_INLINE const PxU8*	getSamples2()	const
	{
		return mSamples + mNbSamples;
	}
	//~Support vertex map

	// Valencies data
	PxU32			mNbVerts;		//!< Number of vertices
	PxU32			mNbAdjVerts;	//!< Total number of adjacent vertices  ### PT: this is useless at runtime and should not be stored here
	Gu::Valency*	mValencies;		//!< A list of mNbVerts valencies (= number of neighbors)
	PxU8*			mAdjacentVerts;	//!< List of adjacent vertices
	//~Valencies data
};
#if defined(PX_P64)
PX_COMPILE_TIME_ASSERT(sizeof(Gu::BigConvexRawData) == 40);
#else
PX_COMPILE_TIME_ASSERT(sizeof(Gu::BigConvexRawData) == 24);
#endif

} // namespace Gu

}

#endif
