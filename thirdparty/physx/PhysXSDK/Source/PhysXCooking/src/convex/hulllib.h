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

#ifndef HULL_LIB_H
#define HULL_LIB_H


/*----------------------------------------------------------------------
		Copyright (c) 2004 Open Dynamics Framework Group
					www.physicstools.org
		All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, are permitted provided
		that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, this list of conditions
		and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

		Neither the name of the Open Dynamics Framework Group nor the names of its contributors may
		be used to endorse or promote products derived from this software without specific prior written permission.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES,
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE INTEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
		LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
		IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
		THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

#include "hulllibArray.h"

namespace physx
{

class HullResult
{
public:
	HullResult()
	{
		mPolygons			= true;
		mNumOutputVertices	= 0;
		mOutputVertices		= 0;
		mNumFaces			= 0;
		mNumIndices			= 0;
		mIndices			= 0;
	}
	bool	mPolygons;				// true if indices represents polygons, false indices are triangles
	PxU32	mNumOutputVertices;		// number of vertices in the output hull
	float*	mOutputVertices;		// array of vertices, 3 floats each x,y,z
	PxU32	mNumFaces;				// the number of faces produced
	PxU32	mNumIndices;			// the total number of indices
	PxU32*	mIndices;				// pointer to indices.

// If triangles, then indices are array indexes into the vertex list.
// If polygons, indices are in the form (number of points in face) (p1, p2, p3, ..) etc..
};

enum HullFlag
{
	QF_TRIANGLES		= (1<<0),	// report results as triangles, not polygons.
	QF_REVERSE_ORDER	= (1<<1),	// reverse order of the triangle indices.
	QF_SKIN_WIDTH		= (1<<2),	// extrude hull based on this skin width
	QF_DEFAULT			= 0
};

class HullDesc
{
public:
	HullDesc()
	{
		mFlags			= QF_DEFAULT;
		mVcount			= 0;
		mVertices		= 0;
		mVertexStride	= 0;
		mNormalEpsilon	= 0.001f;
		mMaxVertices	= 4096;		// maximum number of points to be considered for a convex hull.
		mSkinWidth		= 0.01f;	// default is one centimeter
	}

	HullDesc(HullFlag flag, PxU32 vcount, const float* vertices, PxU32 stride)
	{
		mFlags			= flag;
		mVcount			= vcount;
		mVertices		= vertices;
		mVertexStride	= stride;
		mNormalEpsilon	= 0.001f;
		mMaxVertices	= 4096;
		mSkinWidth		= 0.01f;	// default is one centimeter
	}

	PX_FORCE_INLINE bool HasHullFlag(HullFlag flag)	const
	{
		return (mFlags & flag)!=0;
	}

	PX_FORCE_INLINE	void SetHullFlag(HullFlag flag)
	{
		mFlags |= flag;
	}

	PX_FORCE_INLINE	void ClearHullFlag(HullFlag flag)
	{
		mFlags &= ~flag;
	}

	PxU32			mFlags;				// flags to use when generating the convex hull.
	PxU32			mVcount;			// number of vertices in the input point cloud
	const float*	mVertices;			// the array of vertices.
	PxU32			mVertexStride;		// the stride of each vertex, in bytes.
	float			mNormalEpsilon;		// the epsilon for removing duplicates.  This is a normalized value, if normalized bit is on.
	float			mSkinWidth;			// additional skinWidth used for inflation code path
	float			mAreaTestEpsilon;	// the epsilon used for area test during initial hull computation
	PxU32			mMaxVertices;		// maximum number of vertices to be considered for the hull!
};

enum HullError
{
	QE_OK = 0,		// success!
	QE_FAIL,		// failed.
	QE_VERTEX_LIMIT_REACHED,		// vertex limit reached fallback.
	QE_AREA_TEST_FAIL// area test failed - failed to create simplex
};

// This class is used when converting a convex hull into a triangle mesh.
class ConvexHullVertex
{
public:
	float	mPos[3];
	float	mNormal[3];
	float	mTexel[2];
};

// A virtual interface to receive the triangles from the convex hull.
class ConvexHullTriangleInterface
{
public:
    virtual ~ConvexHullTriangleInterface() {}
	virtual void ConvexHullTriangle(const ConvexHullVertex& v1, const ConvexHullVertex& v2, const ConvexHullVertex& v3) = 0;
};

class Tri;
class float3;
class Plane;
class PHullResult;

class HullLibrary
{
public:
					HullLibrary();
					~HullLibrary();

		HullError	CreateConvexHull(	const HullDesc& desc,	// describes the input request
										HullResult& result);	// contains the resulst

		HullError	ReleaseResult(HullResult& result);	// release memory allocated for this result, we are done with it.

		HullError	CreateTriangleMesh(HullResult& answer, ConvexHullTriangleInterface* iface);

	private:
		hullLibArray::Array<Tri*>* mTris;

		void		b2bfix(Tri* s, Tri* t);
		void		removeb2b(Tri* s, Tri* t);
		void		checkit(Tri* t)	const;
		void		extrude(Tri* t0, int v);
		Tri*		extrudable(float epsilon)	const;
		HullError	calchullgen(float3* verts, int verts_count, int vlimit, float areaEpsilon);
		HullError	calchull(float3* verts, int verts_count, int*& tris_out, int& tris_count,float3*& verts_out, int& verts_count_out, int vlimit, float areaEpsilon);
		HullError	expandhull(float3* verts, int verts_count,float3*& verts_out, int& verts_count_out, float inflate = 0.0f);
		HullError	calchullpbev(float3* verts, int verts_count, int vlimit, hullLibArray::Array<Plane>& planes, float bevangle);
		HullError	ComputeHull(PxU32 vcount, const float* vertices, PHullResult& result, PxU32 vlimit, float inflate, float areaEpsilon);
		HullError	overhullv(	float3* verts, int verts_count, int maxplanes,
								float3*& verts_out, int& verts_count_out, int*& faces_out, int& faces_count_out, float inflate, float bevangle, int vlimit, float areaEpsilon);
};

}

#endif
