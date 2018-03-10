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

#ifndef GU_PROJECTION_H
#define GU_PROJECTION_H

#include "PxVec3.h"

namespace physx
{
namespace Gu
{

using namespace Ps::aos;

// A bunch of function for quickly projecting a set of points along a direction and 
// finding the minimum and maximum etc.

#ifndef PX_VMX

class Projection
{
public: 

	static PX_FORCE_INLINE void pointsOnDir(const PxVec3& dir, const PxVec3* verts, PxU32 numVerts, PxReal& _min, PxReal& _max)
	{
		// PT: prevents aliasing
		PxReal minimum = PX_MAX_REAL;
		PxReal maximum = -PX_MAX_REAL;

		while(numVerts--)
		{
			const PxReal dp = (*verts++).dot(dir);
			minimum = physx::intrinsics::selectMin(minimum, dp);
			maximum = physx::intrinsics::selectMax(maximum, dp);
		}

		_min = minimum;
		_max = maximum;
	}

	static PX_FORCE_INLINE PxU32 pointsOnDirMax(const PxVec3& dir, const PxVec3* verts, PxU32 numVerts)
	{
		PX_ASSERT(numVerts >= 1);

		PxReal bestValue = (*verts).dot(dir);
		PxU32 bestIndex = 0;

		for(PxU32 i = 1; i < numVerts; i++)
		{
			const PxReal dp = verts[i].dot(dir);
			if(dp > bestValue) 
			{
				bestValue = dp;
				bestIndex = i;
			}
		}

		return bestIndex;
	}

private:
	Projection(){}
};

#else

class Projection
{
public:

	// Note: in terms of instructions we execute ~twice as many but it takes much less time due to
	// better pipelining. (at least up to ~100)

	static PX_FORCE_INLINE void pointsOnDir(const Vec3V& dir4, const PxVec3* PX_RESTRICT verts, PxU32 numVerts, FloatV& min4, FloatV& max4)
	{
		//split to eliminate dependance within loop.
		FloatV min4_1 = FMax();
		FloatV max4_1 = FNegMax();
		FloatV min4_2 = min4_1;
		FloatV max4_2 = max4_1;
		FloatV min4_3 = min4_1;
		FloatV max4_3 = max4_1;
		FloatV min4_4 = min4_1;
		FloatV max4_4 = max4_1;

		const PxU32 numRolled = numVerts & 0x3;

		for(PxU32 i=0; i<numRolled; i++)
		{
			const FloatV dp = V3Dot(dir4, V3LoadU(*verts));

			min4_1 = FMin(min4_1, dp);
			max4_1 = FMax(max4_1, dp);

			verts++;
		}

		const PxU32 numUnRolled = numVerts & ~0x3;
		const PxVec3* PX_RESTRICT verts_end = verts + numUnRolled;

		while(verts<verts_end)
		{
			const Vec3V verts_0 = V3LoadU(verts[0]);
			const Vec3V verts_1 = V3LoadU(verts[1]);
			const Vec3V verts_2 = V3LoadU(verts[2]);
			const Vec3V verts_3 = V3LoadU(verts[3]);

			const FloatV dp_1 = V3Dot(dir4, verts_0);
			const FloatV dp_2 = V3Dot(dir4, verts_1);
			const FloatV dp_3 = V3Dot(dir4, verts_2);
			const FloatV dp_4 = V3Dot(dir4, verts_3);

			min4_1 = FMin(min4_1, dp_1);
			max4_1 = FMax(max4_1, dp_1);

			min4_2 = FMin(min4_2, dp_2);
			max4_2 = FMax(max4_2, dp_2);
			
			min4_3 = FMin(min4_3, dp_3);
			max4_3 = FMax(max4_3, dp_3);
			
			min4_4 = FMin(min4_4, dp_4);
			max4_4 = FMax(max4_4, dp_4);

			verts+=4;
		}

		min4_1 = FMin(min4_1, min4_2);
		min4_3 = FMin(min4_3, min4_4);
		min4 = FMin(min4_1, min4_3);

		max4_1 = FMax(max4_1, max4_2);
		max4_3 = FMax(max4_3, max4_4);
		max4 = FMax(max4_1, max4_3);
	}

	static PX_FORCE_INLINE void pointsOnDir(const PxVec3& dir, const PxVec3* PX_RESTRICT verts, PxU32 numVerts,
								PxReal& minimum, PxReal& maximum)
	{
		Vec3V dir4 = V3LoadU(dir);
		FloatV min4, max4;

		pointsOnDir(dir4, verts, numVerts, min4, max4);

		minimum=FStore(min4);
		maximum=FStore(max4);

		PX_ASSERT(minimum <= maximum);
	}

	static PX_FORCE_INLINE PxU32 pointsOnDirMax(const PxVec3& dir, const PxVec3* PX_RESTRICT verts, PxU32 numVerts)
	{
		PX_ASSERT(numVerts >= 1);

		/*
		PxReal bestValue = (*verts).dot(dir);
		PxU32 bestIndex = 0;
		*/

		Vec3V dir4 = V3LoadU(dir);
		FloatV bestValue4 = V3Dot(dir4,V3LoadU(*verts));
		VecI32V bestIndex4 = VecI32V_Zero();
		PX_ALIGN(16, PxI32 bestIndex[4]);
		verts++;

		/*for (PxU32 i = 1; i < numVerts; i++)
		{
			PxReal dp = verts[i].dot(dir);
			if(dp > bestValue) 
			{
				bestValue = dp;
				bestIndex = i;
			}
		}*/
		const VecI32V iOne = I4Load(1);
		VecI32V i4 = iOne;
		for (PxU32 i = 1; i < numVerts; i++)
		{
			FloatV dp = V3Dot(dir4,V3LoadU(*verts));
			BoolV mask = FIsGrtr(dp,bestValue4);
			bestValue4 = FSel(mask, dp, bestValue4);
			bestIndex4 = V4I32Sel(mask, i4, bestIndex4);
			verts++;
			i4 = VecI32V_Add(i4, iOne);
		}

		/*maxProjection = bestValue;
		return bestIndex;*/

		I4StoreA(bestIndex4, bestIndex);

		return (PxU32)bestIndex[0];
	}

private:
	Projection(){}
};

#endif

}
}

#endif
