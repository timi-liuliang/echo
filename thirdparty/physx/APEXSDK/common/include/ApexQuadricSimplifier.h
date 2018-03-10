/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_QUADRIC_SIMPLIFIER_H__
#define __APEX_QUADRIC_SIMPLIFIER_H__

#include "PsShare.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace apex
{

class ApexQuadricSimplifier : public physx::UserAllocated
{
public:
	ApexQuadricSimplifier();

	~ApexQuadricSimplifier();
	void clear();

	// registration
	void registerVertex(const physx::PxVec3& pos);
	void registerTriangle(physx::PxU32 v0, physx::PxU32 v1, physx::PxU32 v2);
	bool endRegistration(bool mergeCloseVertices, IProgressListener* progress);

	// manipulation
	physx::PxU32 simplify(physx::PxU32 subdivision, physx::PxI32 maxSteps, physx::PxF32 maxError, IProgressListener* progress);

	// accessors
	physx::PxU32 getNumVertices() const
	{
		return mVertices.size();
	}
	physx::PxU32 getNumDeletedVertices() const
	{
		return mNumDeletedVertices;
	}

	bool getVertexPosition(physx::PxU32 vertexNr, physx::PxVec3& pos) const
	{
		PX_ASSERT(vertexNr < mVertices.size());
		if (mVertices[vertexNr]->bDeleted == 1)
		{
			return false;
		}

		pos = mVertices[vertexNr]->pos;
		return true;
	}
	physx::PxI32 getTriangleNr(physx::PxU32 v0, physx::PxU32 v1, physx::PxU32 v2) const;
	physx::PxU32 getNumTriangles() const
	{
		return mTriangles.size() - mNumDeletedTriangles;
	}
	bool getTriangle(physx::PxU32 i, physx::PxU32& v0, physx::PxU32& v1, physx::PxU32& v2) const;

private:

	class Quadric
	{
	public:
		void zero()
		{
			a00 = 0.0f;
			a01 = 0.0f;
			a02 = 0.0f;
			a03 = 0.0f;
			a11 = 0.0f;
			a12 = 0.0f;
			a13 = 0.0f;
			a22 = 0.0f;
			a23 = 0.0f;
			a33 = 0.0f;
		}

		// generate quadric from plane
		void setFromPlane(const physx::PxVec3& v0, const physx::PxVec3& v1, const physx::PxVec3& v2)
		{
			physx::PxVec3 n = (v1 - v0).cross(v2 - v0);
			n.normalize();
			float d = -n.dot(v0);
			a00 = n.x * n.x;
			a01 = n.x * n.y;
			a02 = n.x * n.z;
			a03 = n.x * d;
			a11 = n.y * n.y;
			a12 = n.y * n.z;
			a13 = n.y * d;
			a22 = n.z * n.z;
			a23 = n.z * d;
			a33 = d * d;
		}

		Quadric operator +(const Quadric& q) const
		{
			Quadric sum;
			sum.a00 = a00 + q.a00;
			sum.a01 = a01 + q.a01;
			sum.a02 = a02 + q.a02;
			sum.a03 = a03 + q.a03;
			sum.a11 = a11 + q.a11;
			sum.a12 = a12 + q.a12;
			sum.a13 = a13 + q.a13;
			sum.a22 = a22 + q.a22;
			sum.a23 = a23 + q.a23;
			sum.a33 = a33 + q.a33;
			return sum;
		}

		void operator +=(const Quadric& q)
		{
			a00 += q.a00;
			a01 += q.a01;
			a02 += q.a02;
			a03 += q.a03;
			a11 += q.a11;
			a12 += q.a12;
			a13 += q.a13;
			a22 += q.a22;
			a23 += q.a23;
			a33 += q.a33;
		}

		physx::PxF32 outerProduct(const physx::PxVec3& v)
		{
			return a00 * v.x * v.x + 2.0f * a01 * v.x * v.y + 2.0f * a02 * v.x * v.z + 2.0f * a03 * v.x +
			       a11 * v.y * v.y + 2.0f * a12 * v.y * v.z + 2.0f * a13 * v.y +
			       a22 * v.z * v.z + 2.0f * a23 * v.z + a33;
		}
	private:
		physx::PxF32 a00, a01, a02, a03;
		physx::PxF32      a11, a12, a13;
		physx::PxF32           a22, a23;
		physx::PxF32                a33;

	};

	struct QuadricVertex : public physx::UserAllocated
	{
		QuadricVertex(const physx::PxVec3& newPos)
		{
			pos = newPos;
			q.zero();
			bDeleted = 0;
			bReferenced = 0;
			bBorder = 0;
		}
		void removeEdge(physx::PxI32 edgeNr);
		void addTriangle(physx::PxI32 triangleNr);
		void removeTriangle(physx::PxI32 triangleNr);
		physx::PxVec3 pos;
		Quadric  q;
		physx::Array<physx::PxU32> mEdges;
		physx::Array<physx::PxU32> mTriangles;
		physx::PxU32 bDeleted : 1;
		physx::PxU32 bReferenced : 1;
		physx::PxU32 bBorder : 1;
	};

	struct QuadricEdge
	{
		void init(physx::PxI32 v0, physx::PxI32 v1)
		{
			vertexNr[0] = (physx::PxU32)physx::PxMin(v0, v1);
			vertexNr[1] = (physx::PxU32)physx::PxMax(v0, v1);
			cost = -1.0f;
			lengthSquared = -1.0f;
			ratio = -1.0f;
			heapPos = -1;
			border = false;
			deleted = false;
		}
		bool operator < (QuadricEdge& e) const
		{
			if (vertexNr[0] < e.vertexNr[0])
			{
				return true;
			}
			if (vertexNr[0] > e.vertexNr[0])
			{
				return false;
			}
			return vertexNr[1] < e.vertexNr[1];
		}
		bool operator == (QuadricEdge& e) const
		{
			return vertexNr[0] == e.vertexNr[0] && vertexNr[1] == e.vertexNr[1];
		}
		physx::PxU32 otherVertex(physx::PxU32 vNr) const
		{
			if (vertexNr[0] == vNr)
			{
				return vertexNr[1];
			}
			else
			{
				PX_ASSERT(vertexNr[1] == vNr);
				return vertexNr[0];
			}
		}
		void replaceVertex(physx::PxU32 vOld, physx::PxU32 vNew)
		{
			if (vertexNr[0] == vOld)
			{
				vertexNr[0] = vNew;
			}
			else if (vertexNr[1] == vOld)
			{
				vertexNr[1] = vNew;
			}
			else
			{
				PX_ASSERT(0);
			}
			if (vertexNr[0] > vertexNr[1])
			{
				unsigned v = vertexNr[0];
				vertexNr[0] = vertexNr[1];
				vertexNr[1] = v;
			}
		}
		physx::PxU32 vertexNr[2];
		physx::PxF32 cost;
		physx::PxF32 lengthSquared;
		physx::PxF32 ratio;
		physx::PxI32 heapPos;
		bool border;
		bool deleted;
	};

	struct QuadricTriangle
	{
		void init(physx::PxU32 v0, physx::PxU32 v1, physx::PxU32 v2)
		{
			vertexNr[0] = v0;
			vertexNr[1] = v1;
			vertexNr[2] = v2;
			deleted = false;
		}
		bool containsVertex(physx::PxU32 vNr) const
		{
			return vertexNr[0] == vNr || vertexNr[1] == vNr || vertexNr[2] == vNr;
		}
		physx::PxU32 otherVertex(physx::PxU32 v0, physx::PxU32 v1)
		{
			if (vertexNr[0] != v0 && vertexNr[0] != v1)
			{
				PX_ASSERT(v0 == vertexNr[1] || v0 == vertexNr[2]);
				PX_ASSERT(v1 == vertexNr[1] || v1 == vertexNr[2]);
				return vertexNr[0];
			}
			else if (vertexNr[1] != v0 && vertexNr[1] != v1)
			{
				PX_ASSERT(v0 == vertexNr[0] || v0 == vertexNr[2]);
				PX_ASSERT(v1 == vertexNr[0] || v1 == vertexNr[2]);
				return vertexNr[1];
			}
			else
			{
				PX_ASSERT(vertexNr[2] != v0 && vertexNr[2] != v1);
				PX_ASSERT(v0 == vertexNr[0] || v0 == vertexNr[1]);
				PX_ASSERT(v1 == vertexNr[0] || v1 == vertexNr[1]);
				return vertexNr[2];
			}
		}
		void replaceVertex(physx::PxU32 vOld, physx::PxU32 vNew)
		{
			if (vertexNr[0] == vOld)
			{
				vertexNr[0] = vNew;
			}
			else if (vertexNr[1] == vOld)
			{
				vertexNr[1] = vNew;
			}
			else if (vertexNr[2] == vOld)
			{
				vertexNr[2] = vNew;
			}
			else
			{
				PX_ASSERT(0);
			}
		}
		bool operator == (QuadricTriangle& t) const
		{
			return t.containsVertex(vertexNr[0]) &&
			       t.containsVertex(vertexNr[1]) &&
			       t.containsVertex(vertexNr[2]);
		}
		physx::PxU32 vertexNr[3];
		bool deleted;
	};

	struct QuadricVertexRef
	{
		void init(const physx::PxVec3& p, physx::PxI32 vNr)
		{
			pos = p;
			vertexNr = (physx::PxU32)vNr;
		}
		bool operator < (const QuadricVertexRef& vr)
		{
			return pos.x < vr.pos.x;
		}
		physx::PxVec3 pos;
		physx::PxU32 vertexNr;
	};


	void computeCost(QuadricEdge& edge);
	bool legalCollapse(QuadricEdge& edge, physx::PxF32 maxLength);
	void collapseEdge(QuadricEdge& edge);
	void quickSortEdges(physx::PxI32 l, physx::PxI32 r);
	void quickSortVertexRefs(physx::PxI32 l, physx::PxI32 r);
	void mergeVertices();

	bool heapElementSmaller(QuadricEdge* e0, QuadricEdge* e1);
	void heapUpdate(physx::PxU32 i);
	void heapSift(physx::PxU32 i);
	void heapRemove(physx::PxU32 i, bool append);
	void testHeap();
	void testMesh();

	physx::PxBounds3 mBounds;

	physx::Array<QuadricVertex*>  mVertices;
	physx::Array<QuadricEdge>      mEdges;
	physx::Array<QuadricTriangle>  mTriangles;
	physx::Array<QuadricEdge*>    mHeap;
	physx::Array<QuadricVertexRef> mVertexRefs;

	physx::PxU32 mNumDeletedTriangles;
	physx::PxU32 mNumDeletedVertices;
	physx::PxU32 mNumDeletedHeapElements;
};

}
} // end namespace physx::apex

#endif