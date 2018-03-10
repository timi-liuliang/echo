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

#ifndef GU_TRIANGLE32_H
#define GU_TRIANGLE32_H

#include "CmPhysXCommon.h"
#include "PxVec3.h"

namespace physx
{
namespace Gu
{
	/**
	\brief Structure used to store indices for a triangles points. T is either PxU32 or PxU16

	*/

	template <class T>
	struct TriangleT// : public Ps::UserAllocated
		{
		PX_INLINE	TriangleT()							{}
		PX_INLINE	TriangleT(T a, T b, T c)	{ v[0] = a; v[1] = b; v[2] = c; }
		template <class TX>
		PX_INLINE	TriangleT(const TriangleT<TX>& other) { v[0] = other[0]; v[1] = other[1]; v[2] = other[2]; }
		PX_INLINE	T& operator[](T i)				{ return v[i]; }
		template<class TX>//any type of TriangleT<>, possibly with different T
		PX_INLINE	TriangleT<T>& operator=(const TriangleT<TX>& i)	{ v[0]=i[0]; v[1]=i[1]; v[2]=i[2]; return *this; }
		PX_INLINE	const T& operator[](T i) const	{ return v[i]; }

		void	flip()
		{
			Ps::swap(v[1], v[2]);
		}

		PX_INLINE void center(const PxVec3* verts, PxVec3& center)	const
		{
			const PxVec3& p0 = verts[v[0]];
			const PxVec3& p1 = verts[v[1]];
			const PxVec3& p2 = verts[v[2]];
			center = (p0+p1+p2)*0.33333333333333333333f;
		}

		float area(const PxVec3* verts)	const
		{
			const PxVec3& p0 = verts[v[0]];
			const PxVec3& p1 = verts[v[1]];
			const PxVec3& p2 = verts[v[2]];
			return ((p0-p1).cross(p0-p2)).magnitude() * 0.5f;
		}

		PxU8	findEdge(T vref0, T vref1)	const
		{
					if(v[0]==vref0 && v[1]==vref1)	return 0;
			else	if(v[0]==vref1 && v[1]==vref0)	return 0;
			else	if(v[0]==vref0 && v[2]==vref1)	return 1;
			else	if(v[0]==vref1 && v[2]==vref0)	return 1;
			else	if(v[1]==vref0 && v[2]==vref1)	return 2;
			else	if(v[1]==vref1 && v[2]==vref0)	return 2;
			return 0xff;
		}

		// counter clock wise order
		PxU8	findEdgeCCW(T vref0, T vref1)	const
		{
			if(v[0]==vref0 && v[1]==vref1)	return 0;
			else	if(v[0]==vref1 && v[1]==vref0)	return 0;
			else	if(v[0]==vref0 && v[2]==vref1)	return 2;
			else	if(v[0]==vref1 && v[2]==vref0)	return 2;
			else	if(v[1]==vref0 && v[2]==vref1)	return 1;
			else	if(v[1]==vref1 && v[2]==vref0)	return 1;
			return 0xff;
		}

		bool	replaceVertex(T oldref, T newref)
		{
					if(v[0]==oldref)	{ v[0] = newref; return true; }
			else	if(v[1]==oldref)	{ v[1] = newref; return true; }
			else	if(v[2]==oldref)	{ v[2] = newref; return true; }
			return false;
		}

		bool isDegenerate()	const
		{
			if(v[0]==v[1])	return true;
			if(v[1]==v[2])	return true;
			if(v[2]==v[0])	return true;
			return false;
		}

		PX_INLINE void denormalizedNormal(const PxVec3* verts, PxVec3& normal)	const
		{
			const PxVec3& p0 = verts[v[0]];
			const PxVec3& p1 = verts[v[1]];
			const PxVec3& p2 = verts[v[2]];
			normal = ((p2 - p1).cross(p0 - p1));
		}

		T v[3];	//vertex indices
	};
}

}

#endif
