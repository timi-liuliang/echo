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

#ifndef GU_CUBE_INDEX_H
#define GU_CUBE_INDEX_H

#include "PxVec3.h"
#include "CmPhysXCommon.h"

namespace physx
{

	enum CubeIndex
	{
		CUBE_RIGHT,
		CUBE_LEFT,
		CUBE_TOP,
		CUBE_BOTTOM,
		CUBE_FRONT,
		CUBE_BACK,

		CUBE_FORCE_DWORD	= 0x7fffffff
	};

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
	CubeIndex		CubemapLookup(const PxVec3& direction, float& u, float& v);

	PX_INLINE PxU32 ComputeCubemapOffset(const PxVec3& dir, PxU32 subdiv)
	{
		float u,v;
		const CubeIndex CI = CubemapLookup(dir, u, v);

		// Remap to [0, subdiv[
		const float Coeff = 0.5f * float(subdiv-1);
		u += 1.0f;	u *= Coeff;
		v += 1.0f;	v *= Coeff;

		// Compute offset
		return PxU32(CI)*(subdiv*subdiv) + PxU32(u)*subdiv + PxU32(v);
	}

	//PX_INLINE PxU32 ComputeCubemapNearestOffset(const PxVec3& dir, PxU32 subdiv)
	//{
	//	float u,v;
	//	const CubeIndex CI = CubemapLookup(dir, u, v);

	//	// Remap to [0, subdiv[
	//	const float Coeff = 0.5f * float(subdiv-1);
	//	u += 1.0f;	u *= Coeff;
	//	v += 1.0f;	v *= Coeff;

	//	// Round to nearest
	//	PxU32 ui = PxU32(u);
	//	PxU32 vi = PxU32(v);
	//	const float du = u - float(ui);
	//	const float dv = v - float(vi);
	//	if(du>0.5f)	ui++;
	//	if(dv>0.5f)	vi++;

	//	// Compute offset
	//	return PxU32(CI)*(subdiv*subdiv) + ui*subdiv + vi;
	//}

	PX_INLINE PxU32 ComputeCubemapNearestOffset(const PxVec3& dir, PxU32 subdiv)
	{
		float u,v;
		const CubeIndex CI = CubemapLookup(dir, u, v);

		// Remap to [0, subdiv]
		const float Coeff = 0.5f * float(subdiv-1);
		u += 1.0f;	u *= Coeff;
		v += 1.0f;	v *= Coeff;

		// Compute offset
		return PxU32(CI)*(subdiv*subdiv) + PxU32(u + 0.5f)*subdiv + PxU32(v + 0.5f);
	}

}

#endif
