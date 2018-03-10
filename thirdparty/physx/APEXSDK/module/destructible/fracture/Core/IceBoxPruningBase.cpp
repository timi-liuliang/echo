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
 *	\file		IceBoxPruning.cpp
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	You could use a complex sweep-and-prune as implemented in I-Collide.
	You could use a complex hashing scheme as implemented in V-Clip or recently in ODE it seems.
	You could use a "Recursive Dimensional Clustering" algorithm as implemented in GPG2.

	Or you could use this.
	Faster ? I don't know. Probably not. It would be a shame. But who knows ?
	Easier ? Definitely. Enjoy the sheer simplicity.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IceBoxPruningBase.h"

namespace physx
{
namespace fracture
{
namespace base
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Bipartite box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to a different set.
 *	\param		nb0		[in] number of boxes in the first set
 *	\param		bounds0	[in] list of boxes for the first set
 *	\param		nb1		[in] number of boxes in the second set
 *	\param		bounds1	[in] list of boxes for the second set
 *	\param		pairs	[out] list of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BoxPruning::bipartiteBoxPruning(const shdfnd::Array<PxBounds3> &bounds0, const shdfnd::Array<PxBounds3> &bounds1, shdfnd::Array<PxU32>& pairs, const Axes& axes)
{
	PxU32 nb0 = bounds0.size();
	PxU32 nb1 = bounds1.size();
	pairs.clear();
	// Checkings
	if(nb0 == 0 || nb1 == 0)	return false;

	// Catch axes
	PxU32 Axis0 = axes.Axis0;
	//PxU32 Axis1 = axes.Axis1;
	//PxU32 Axis2 = axes.Axis2;

	// Allocate some temporary data
	if (mMinPosBounds0.size() < nb0)
		mMinPosBounds0.resize(nb0);
	if (mMinPosBounds1.size() < nb1)
		mMinPosBounds1.resize(nb1);

	// 1) Build main lists using the primary axis
	for(PxU32 i=0;i<nb0;i++)	mMinPosBounds0[i] = bounds0[i].minimum[Axis0];
	for(PxU32 i=0;i<nb1;i++)	mMinPosBounds1[i] = bounds1[i].minimum[Axis0];

	// 2) Sort the lists
	PxU32* Sorted0 = mRS0.Sort(&mMinPosBounds0[0], nb0).GetRanks();
	PxU32* Sorted1 = mRS1.Sort(&mMinPosBounds1[0], nb1).GetRanks();

	// 3) Prune the lists
	PxU32 Index0, Index1;

	const PxU32* const LastSorted0 = &Sorted0[nb0];
	const PxU32* const LastSorted1 = &Sorted1[nb1];
	const PxU32* RunningAddress0 = Sorted0;
	const PxU32* RunningAddress1 = Sorted1;

	while(RunningAddress1<LastSorted1 && Sorted0<LastSorted0)
	{
		Index0 = *Sorted0++;

		while(RunningAddress1<LastSorted1 && mMinPosBounds1[*RunningAddress1]<mMinPosBounds0[Index0])	RunningAddress1++;

		const PxU32* RunningAddress2_1 = RunningAddress1;

		while(RunningAddress2_1<LastSorted1 && mMinPosBounds1[Index1 = *RunningAddress2_1++]<=bounds0[Index0].maximum[Axis0])
		{
			if(bounds0[Index0].intersects(bounds1[Index1]))
			{
				pairs.pushBack(Index0);
				pairs.pushBack(Index1);
			}
		}
	}

	////

	while(RunningAddress0<LastSorted0 && Sorted1<LastSorted1)
	{
		Index0 = *Sorted1++;

		while(RunningAddress0<LastSorted0 && mMinPosBounds0[*RunningAddress0]<=mMinPosBounds1[Index0])	RunningAddress0++;

		const PxU32* RunningAddress2_0 = RunningAddress0;

		while(RunningAddress2_0<LastSorted0 && mMinPosBounds0[Index1 = *RunningAddress2_0++]<=bounds1[Index0].maximum[Axis0])
		{
			if(bounds0[Index1].intersects(bounds1[Index0]))
			{
				pairs.pushBack(Index1);
				pairs.pushBack(Index0);
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Complete box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to the same set.
 *	\param		nb		[in] number of boxes
 *	\param		list	[in] list of boxes
 *	\param		pairs	[out] list of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BoxPruning::completeBoxPruning(const shdfnd::Array<PxBounds3> &bounds, shdfnd::Array<PxU32> &pairs, const Axes& axes)
{
	PxU32 nb = bounds.size();
	pairs.clear();

	// Checkings
	if(!nb)	return false;

	// Catch axes
	PxU32 Axis0 = axes.Axis0;
	//PxU32 Axis1 = axes.Axis1;
	//PxU32 Axis2 = axes.Axis2;

	// Allocate some temporary data
	if (mPosList.size() < nb)
		mPosList.resize(nb);

	// 1) Build main list using the primary axis
	for(PxU32 i=0;i<nb;i++)	mPosList[i] = bounds[i].minimum[Axis0];

	// 2) Sort the list
	PxU32* Sorted = mRS.Sort(&mPosList[0], nb).GetRanks();

	// 3) Prune the list
	const PxU32* const LastSorted = &Sorted[nb];
	const PxU32* RunningAddress = Sorted;
	PxU32 Index0, Index1;
	while(RunningAddress<LastSorted && Sorted<LastSorted)
	{
		Index0 = *Sorted++;

		while(RunningAddress<LastSorted && mPosList[*RunningAddress++]<mPosList[Index0]);

		const PxU32* RunningAddress2 = RunningAddress;

		while(RunningAddress2<LastSorted && mPosList[Index1 = *RunningAddress2++]<=bounds[Index0].maximum[Axis0])
		{
			if(Index0!=Index1)
			{
				if(bounds[Index0].intersects(bounds[Index1]))
				{
					pairs.pushBack(Index0);
					pairs.pushBack(Index1);
				}
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Brute-force versions are kept:
// - to check the optimized versions return the correct list of intersections
// - to check the speed of the optimized code against the brute-force one
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Brute-force bipartite box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to a different set.
 *	\param		nb0		[in] number of boxes in the first set
 *	\param		bounds0	[in] list of boxes for the first set
 *	\param		nb1		[in] number of boxes in the second set
 *	\param		bounds1	[in] list of boxes for the second set
 *	\param		pairs	[out] list of overlapping pairs
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BoxPruning::bruteForceBipartiteBoxTest(const shdfnd::Array<PxBounds3> &bounds0, const shdfnd::Array<PxBounds3> &bounds1, shdfnd::Array<PxU32>& pairs, const Axes& /*axes*/)
{
	PxU32 nb0 = bounds0.size();
	PxU32 nb1 = bounds1.size();
	pairs.clear();

	// Checkings
	if(!nb0 || !nb1)	return false;

	// Brute-force nb0*nb1 overlap tests
	for(PxU32 i=0;i<nb0;i++)
	{
		for(PxU32 j=0;j<nb1;j++)
		{
			if(bounds0[i].intersects(bounds1[j])) {
				pairs.pushBack(i);
				pairs.pushBack(j);
			}
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Complete box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to the same set.
 *	\param		nb		[in] number of boxes
 *	\param		list	[in] list of boxes
 *	\param		pairs	[out] list of overlapping pairs
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BoxPruning::bruteForceCompleteBoxTest(const shdfnd::Array<PxBounds3> &bounds, shdfnd::Array<PxU32> &pairs, const Axes& /*axes*/)
{
	PxU32 nb = bounds.size();
	pairs.clear();

	// Checkings
	if(!nb)	return false;

	// Brute-force n(n-1)/2 overlap tests
	for(PxU32 i=0;i<nb;i++)
	{
		for(PxU32 j=i+1;j<nb;j++)
		{
			if(bounds[i].intersects(bounds[j]))	
			{
				pairs.pushBack(i);
				pairs.pushBack(j);
			}
		}
	}
	return true;
}

}
}
}
#endif