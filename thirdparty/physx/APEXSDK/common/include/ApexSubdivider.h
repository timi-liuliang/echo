/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SUBDIVIDER_H
#define APEX_SUBDIVIDER_H

#include "NxApexDefs.h"
#include "PsUserAllocated.h"
#include "ApexRand.h"
#include "PsArray.h"
#include "PsShare.h"

#include "foundation/PxBounds3.h"

namespace physx
{
namespace apex
{

class IProgressListener;

class ApexSubdivider : public physx::UserAllocated
{
public:
	ApexSubdivider();

	void clear();

	void registerVertex(const PxVec3& v, PxU32 bitFlagPayload);
	void registerTriangle(PxU32 i0, PxU32 i1, PxU32 i2);

	void endRegistration();

	void mergeVertices(IProgressListener* progress);
	void closeMesh(IProgressListener* progress);
	void subdivide(PxU32 subdivisionGridSize, IProgressListener* progress);

	PxU32 getNumVertices() const;
	PxU32 getNumTriangles() const;

	void getVertex(PxU32 i, PxVec3& v, PxU32& bitFlagPayload) const;
	void getTriangle(PxU32 i, PxU32& i0, PxU32& i1, PxU32& i2) const;

private:
	void compress();
	void closeHole(PxU32* indices, PxU32 numIndices);
	PxF32 qualityOfTriangle(PxU32 v0, PxU32 v1, PxU32 v2) const;
	PxI32 getTriangleNr(const PxU32 v0, const PxU32 v1, const PxU32 v2) const;

	PxBounds3 mBound;

	struct SubdividerVertex
	{
		SubdividerVertex() : pos(0.0f, 0.0f, 0.0f), firstTriangle(-1), payload(0), marked(false) {}
		SubdividerVertex(const PxVec3& newPos, PxU32 bitFlagPayload) : pos(newPos), firstTriangle(-1), payload(bitFlagPayload), marked(false) {}

		PxVec3 pos;
		PxI32 firstTriangle;
		PxU32 payload;
		bool marked;
	};

	struct SubdividerVertexRef
	{
		SubdividerVertexRef() : pos(0.0f, 0.0f, 0.0f), vertexNr(0) {}
		SubdividerVertexRef(const PxVec3& p, PxU32 vNr) : pos(p), vertexNr(vNr) {}
		PX_INLINE bool operator < (const SubdividerVertexRef& vr) const
		{
			return pos.x < vr.pos.x;
		}

		PX_INLINE bool operator()(const SubdividerVertexRef& v1, const SubdividerVertexRef& v2) const
		{
			return v1 < v2;
		}

		PxVec3 pos;
		PxU32 vertexNr;
	};



	struct SubdividerEdge
	{
		void init(PxU32 newV0, PxU32 newV1, PxU32 newTriangleNr)
		{
			v0 = PxMax(newV0, newV1);
			v1 = PxMin(newV0, newV1);
			triangleNr = newTriangleNr;
		}
		PX_INLINE bool operator < (const SubdividerEdge& e) const
		{
			if (v0 < e.v0)
			{
				return true;
			}
			if (v0 > e.v0)
			{
				return false;
			}
			return (v1 < e.v1);
		}
		PX_INLINE bool operator()(const SubdividerEdge& e1, const SubdividerEdge& e2) const
		{
			return e1 < e2;
		}
		PX_INLINE bool operator == (const SubdividerEdge& e) const
		{
			return v0 == e.v0 && v1 == e.v1;
		}
		PxU32 v0, v1;
		PxU32 triangleNr;
	};

	PxI32 binarySearchEdges(const Array<SubdividerEdge>& edges, PxU32 v0, PxU32 v1, PxU32 triangleNr) const;

	struct SubdividerTriangle
	{
		void init(PxU32 v0, PxU32 v1, PxU32 v2)
		{
			vertexNr[0] = v0;
			vertexNr[1] = v1;
			vertexNr[2] = v2;
		}

		bool containsVertex(PxU32 vNr) const
		{
			return vertexNr[0] == vNr || vertexNr[1] == vNr || vertexNr[2] == vNr;
		}

		void replaceVertex(PxU32 vOld, PxU32 vNew)
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
				PX_ASSERT(0 && "replaceVertex failed");
			}
		}

		bool operator == (SubdividerTriangle& t) const
		{
			return
			    t.containsVertex(vertexNr[0]) &&
			    t.containsVertex(vertexNr[1]) &&
			    t.containsVertex(vertexNr[2]);
		}
		bool isValid() const
		{
			return (vertexNr[0] != vertexNr[1] && vertexNr[0] != vertexNr[2] && vertexNr[1] != vertexNr[2]);
		}

		PxU32 vertexNr[3];
	};

	struct TriangleList
	{
		TriangleList() : triangleNumber(0), nextTriangle(-1) {}
		TriangleList(PxU32 tNr) : triangleNumber(tNr), nextTriangle(-1) {}

		PxU32 triangleNumber;
		PxI32 nextTriangle;
	};

	Array<SubdividerVertex> mVertices;
	Array<SubdividerTriangle> mTriangles;
	PxU32 mMarkedVertices;

	QDSRand mRand;

	Array<TriangleList> mTriangleList;
	PxI32 mTriangleListEmptyElement;
	void addTriangleToVertex(PxU32 vertexNumber, PxU32 triangleNumber);
	void removeTriangleFromVertex(PxU32 vertexNumber, PxU32 triangleNumber);
	TriangleList& allocateTriangleElement();
	void freeTriangleElement(PxU32 index);
};

}
} // end namespace physx::apex

#endif
