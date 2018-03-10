/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for box pruning.
 *	\file		IceBoxPruning.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEBOXPRUNING_BASE_H__
#define __ICEBOXPRUNING_BASE_H__

//#include "vector"
#include <PsArray.h>
#include "IceRevisitedRadixBase.h"
#include "PxVec3.h"
#include "PxBounds3.h"
#include <PsUserAllocated.h>

namespace physx
{
namespace fracture
{
namespace base
{

	struct Axes
	{
		void set(PxU32 a0, PxU32 a1, PxU32 a2) {
			Axis0 = a0; Axis1 = a1; Axis2 = a2; 
		}
		PxU32	Axis0;
		PxU32	Axis1;
		PxU32	Axis2;
	};

	class BoxPruning : public ::physx::shdfnd::UserAllocated 
	{
	public:
		// Optimized versions
		bool completeBoxPruning(const shdfnd::Array<PxBounds3> &bounds, shdfnd::Array<PxU32> &pairs, const Axes& axes);
		bool bipartiteBoxPruning(const shdfnd::Array<PxBounds3> &bounds0, const shdfnd::Array<PxBounds3> &bounds1, shdfnd::Array<PxU32>& pairs, const Axes& axes);

		// Brute-force versions
		bool bruteForceCompleteBoxTest(const shdfnd::Array<PxBounds3> &bounds, shdfnd::Array<PxU32> &pairs, const Axes& axes);
		bool bruteForceBipartiteBoxTest(const shdfnd::Array<PxBounds3> &bounds0, const shdfnd::Array<PxBounds3> &bounds1, shdfnd::Array<PxU32>& pairs, const Axes& axes);

	protected:
		shdfnd::Array<float> mMinPosBounds0;
		shdfnd::Array<float> mMinPosBounds1;
		shdfnd::Array<float> mPosList;
		RadixSort mRS0, mRS1;	
		RadixSort mRS;
	};

}
}
}

#endif // __ICEBOXPRUNING_H__
#endif
