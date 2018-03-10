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

#include "GuCubeIndex.h"
#include "CmPhysXCommon.h"
#include "PsMathUtils.h"
#include "PsFPU.h"

namespace physx
{

/*
It's pretty straightforwards in concept (though the execution in hardware is
a bit crufty and complex). You use a 3D texture coord to look up a texel in
a cube map. First you find which of the axis has the largest value (i.e.
X,Y,Z), and then the sign of that axis decides which face you are going to
use. Which is why the faces are called +X, -X, +Y, -Y, +Z, -Z - after their
principle axis. Then you scale the vector so that the largest value is +/-1.
Then use the other two as 2D coords to look up your texel (with a 0.5 scale
& offset).

For example, vector (0.4, -0.2, -0.5). Largest value is the Z axis, and it's
-ve, so we're reading from the -Z map. Scale so that this Z axis is +/-1,
and you get the vector (0.8, -0.4, -1.0). So now use the other two values to
look up your texel. So we look up texel (0.8, -0.4). The scale & offset move
the -1->+1 range into the usual 0->1 UV range, so we actually look up texel
(0.9, 0.3). The filtering is extremely complex, especially where three maps
meet, but that's a hardware problem :-)
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Cubemap lookup function.
 *
 *	To transform returned uvs into mapping coordinates :
 *	u += 1.0f;	u *= 0.5f;
 *	v += 1.0f;	v *= 0.5f;
 *
 *	\fn			CubemapLookup(const PxVec3& direction, float& u, float& v)
 *	\param		direction	[in] a direction vector
 *	\param		u			[out] impact coordinate on the unit cube, in [-1,1]
 *	\param		v			[out] impact coordinate on the unit cube, in [-1,1]
 *	\return		cubemap texture index
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CubeIndex CubemapLookup(const PxVec3& direction, float& u, float& v)
//{
//	PxU32 Index2, Index3;
//	const PxU32 Index = Ps::closestAxis(direction, Index2, Index3);	// 0, 1, 2
//	const float Coeff = 1.0f / fabsf(direction[Index]);
//	const float tmp = direction[Index];
//	const PxU32 Sign = PX_IR(tmp)>>31;
//	u = direction[Index2] * Coeff;
//	v = direction[Index3] * Coeff;
//	return CubeIndex(Sign|(Index+Index));
//}

CubeIndex CubemapLookup(const PxVec3& direction, float& u, float& v)
{
	const PxF32 absPx = PxAbs(direction.x);
	const PxF32 absNy = PxAbs(direction.y);
	const PxF32 absNz = PxAbs(direction.z);

	PxU32 Index1 = 0;	//x biggest axis
	PxU32 Index2 = 1;
	PxU32 Index3 = 2;
	if( (absNy > absPx) & (absNy > absNz))
	{
		//y biggest
		Index2 = 2;
		Index3 = 0;
		Index1 = 1;
	}
	else if(absNz > absPx)
	{
		//z biggest
		Index2 = 0;
		Index3 = 1;
		Index1 = 2;
	}

	const float Coeff = 1.0f / fabsf(direction[Index1]);
	//This code is going to go mem -> reg -> mem -> reg
	/*const float tmp = direction[Index];
	const PxU32 Sign = PX_IR(tmp)>>31;*/
	//This won't; it'll go from mem -> reg. Shouldn't LHS
	const PxU32 Sign = PX_IR(direction[Index1])>>31;

	u = direction[Index2] * Coeff;
	v = direction[Index3] * Coeff;
	return CubeIndex(Sign|(Index1+Index1));
}

}
