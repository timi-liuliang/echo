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

#include "GuGJKSimplex.h"

namespace physx
{
namespace Gu
{

	using namespace Ps::aos;

	static Vec3V getClosestPtPointTriangle(Vec3V* PX_RESTRICT Q, const BoolVArg bIsOutside4, PxU32* indices, PxU32& size)
	{
		const BoolV bTrue = BTTTT();
		FloatV bestSqDist = FMax();
		
		PxU32 _indices[3] = {0, 1, 2};

		Vec3V result = V3Zero();
	
		if(BAllEq(BGetX(bIsOutside4), bTrue))
		{
			//use the original indices, size, v and w
			result = closestPtPointTriangleBaryCentric(Q[0], Q[1], Q[2], indices, size);
			bestSqDist = V3Dot(result, result);
		}

		if(BAllEq(BGetY(bIsOutside4), bTrue))
		{

			PxU32 _size = 3;
			_indices[0] = 0; _indices[1] = 2; _indices[2] = 3; 
			const Vec3V q = closestPtPointTriangleBaryCentric(Q[0], Q[2], Q[3],  _indices, _size);

			const FloatV sqDist = V3Dot(q, q);
			const BoolV con = FIsGrtr(bestSqDist, sqDist);
			if(BAllEq(con, bTrue))
			{
				result = q;
				bestSqDist = sqDist;

				indices[0] = _indices[0];
				indices[1] = _indices[1];
				indices[2] = _indices[2];

				size = _size;
			}
		}

		if(BAllEq(BGetZ(bIsOutside4), bTrue))
		{
			PxU32 _size = 3;
			
			_indices[0] = 0; _indices[1] = 3; _indices[2] = 1; 

			const Vec3V q = closestPtPointTriangleBaryCentric(Q[0], Q[3], Q[1], _indices, _size);
			const FloatV sqDist = V3Dot(q, q);
			const BoolV con = FIsGrtr(bestSqDist, sqDist);
			if(BAllEq(con, bTrue))
			{
				result = q;
				bestSqDist = sqDist;

				indices[0] = _indices[0];
				indices[1] = _indices[1];
				indices[2] = _indices[2];

				size = _size;
			}

		}

		if(BAllEq(BGetW(bIsOutside4), bTrue))
		{
	

			PxU32 _size = 3;
			_indices[0] = 1; _indices[1] = 3; _indices[2] = 2; 
			const Vec3V q = closestPtPointTriangleBaryCentric(Q[1], Q[3], Q[2], _indices, _size);

			const FloatV sqDist = V3Dot(q, q);
			const BoolV con = FIsGrtr(bestSqDist, sqDist);

			if(BAllEq(con, bTrue))
			{
				result = q;
				bestSqDist = sqDist;

				indices[0] = _indices[0];
				indices[1] = _indices[1];
				indices[2] = _indices[2];

				size = _size;
			}
		}

		return result;
	}

	PX_NOALIAS Vec3V closestPtPointTetrahedron(Vec3V* PX_RESTRICT Q, Vec3V* PX_RESTRICT A, Vec3V* PX_RESTRICT B, PxU32& size)
	{
		
		const FloatV eps = FLoad(1e-4f);
		const Vec3V a = Q[0];
		const Vec3V b = Q[1];
		const Vec3V c = Q[2];  
		const Vec3V d = Q[3];
		const BoolV bFalse = BFFFF();

		//degenerated
		const Vec3V ab = V3Sub(b, a);
		const Vec3V ac = V3Sub(c, a);
		const Vec3V n = V3Normalize(V3Cross(ab, ac));
		const FloatV signDist = V3Dot(n, V3Sub(d, a));
		if(FAllGrtr(eps, FAbs(signDist)))
		{
			size = 3;
			return closestPtPointTriangle(Q, A, B, size);
		}

		const BoolV bIsOutside4 = PointOutsideOfPlane4(a, b, c, d);

		if(BAllEq(bIsOutside4, bFalse))
		{
			//All inside
			return V3Zero();
		}

		PxU32 indices[3] = {0, 1, 2};
		
		const Vec3V closest = getClosestPtPointTriangle(Q, bIsOutside4, indices, size);

		const Vec3V q0 = Q[indices[0]]; const Vec3V q1 = Q[indices[1]]; const Vec3V q2 = Q[indices[2]];
		const Vec3V a0 = A[indices[0]]; const Vec3V a1 = A[indices[1]]; const Vec3V a2 = A[indices[2]];
		const Vec3V b0 = B[indices[0]]; const Vec3V b1 = B[indices[1]]; const Vec3V b2 = B[indices[2]];
		Q[0] = q0; Q[1] = q1; Q[2] = q2;
		A[0] = a0; A[1] = a1; A[2] = a2;
		B[0] = b0; B[1] = b1; B[2] = b2; 

		return closest;
	}

	PX_NOALIAS Vec3V closestPtPointTetrahedron(Vec3V* PX_RESTRICT Q, Vec3V* PX_RESTRICT A, Vec3V* PX_RESTRICT B, PxI32* PX_RESTRICT aInd,  PxI32* PX_RESTRICT bInd, PxU32& size)
	{
		
		const FloatV eps = FLoad(1e-4f);
		const Vec3V zeroV = V3Zero();
		
		const Vec3V a = Q[0];
		const Vec3V b = Q[1];
		const Vec3V c = Q[2];
		const Vec3V d = Q[3];
		const BoolV bFalse = BFFFF();

		//degenerated
		const Vec3V ab = V3Sub(b, a);
		const Vec3V ac = V3Sub(c, a);
		const Vec3V n = V3Normalize(V3Cross(ab, ac));
		const FloatV signDist = V3Dot(n, V3Sub(d, a));
		if(FAllGrtr(eps, FAbs(signDist)))
		{
			size = 3;
			return closestPtPointTriangle(Q, A, B, aInd, bInd, size);
		}

		const BoolV bIsOutside4 = PointOutsideOfPlane4(a, b, c, d);

		if(BAllEq(bIsOutside4, bFalse))
		{
			//All inside
			return zeroV;
		}

		PxU32 indices[3] = {0, 1, 2};
		const Vec3V closest = getClosestPtPointTriangle(Q, bIsOutside4, indices, size);

		const Vec3V q0 = Q[indices[0]]; const Vec3V q1 = Q[indices[1]]; const Vec3V q2 = Q[indices[2]];
		const Vec3V a0 = A[indices[0]]; const Vec3V a1 = A[indices[1]]; const Vec3V a2 = A[indices[2]];
		const Vec3V b0 = B[indices[0]]; const Vec3V b1 = B[indices[1]]; const Vec3V b2 = B[indices[2]];
		const PxI32 _aInd0 = aInd[indices[0]]; const PxI32 _aInd1 = aInd[indices[1]]; const PxI32 _aInd2 = aInd[indices[2]];
		const PxI32 _bInd0 = bInd[indices[0]]; const PxI32 _bInd1 = bInd[indices[1]]; const PxI32 _bInd2 = bInd[indices[2]];
		Q[0] = q0; Q[1] = q1; Q[2] = q2;
		A[0] = a0; A[1] = a1; A[2] = a2;
		B[0] = b0; B[1] = b1; B[2] = b2; 
		aInd[0] = _aInd0; aInd[1] = _aInd1; aInd[2] = _aInd2;
		bInd[0] = _bInd0; bInd[1] = _bInd1; bInd[2] = _bInd2;

		return closest;
	}
}

}
