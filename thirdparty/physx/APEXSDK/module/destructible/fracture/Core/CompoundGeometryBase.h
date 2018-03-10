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
#ifndef COMPOUND_GEOMETRY_BASE
#define COMPOUND_GEOMETRY_BASE

#include <foundation/PxVec3.h>
#include <foundation/PxPlane.h>
#include <PsArray.h>
#include <PsUserAllocated.h>

namespace physx
{
namespace fracture
{
namespace base
{

// -----------------------------------------------------------------------------------
class CompoundGeometry : public ::physx::shdfnd::UserAllocated
{
public:
	CompoundGeometry() {}
	virtual ~CompoundGeometry() {}
	bool loadFromFile(const char *filename);
	bool saveFromFile(const char *filename);
	void clear();
	void derivePlanes();

	virtual void debugDraw(int /*maxConvexes*/ = 0) const {}

	struct Convex {			// init using CompoundGeometry::initConvex()
		int firstVert;
		int numVerts;
		int firstNormal;	// one per face index!	If not provided (see face flags) face normal is used
		int firstIndex;
		int numFaces;
		int firstPlane;
		int firstNeighbor;
		int numNeighbors;
	};

	void initConvex(Convex &c);

	shdfnd::Array<Convex> convexes;
	shdfnd::Array<PxVec3> vertices;
	shdfnd::Array<PxVec3> normals;	// one per face and vertex!
	// face size, face flags, id, id, .., face size, face flags, id ..
	shdfnd::Array<int> indices;
	shdfnd::Array<int> neighbors;

	shdfnd::Array<PxPlane> planes;  // derived for faster cuts

	enum FaceFlags {
		FF_OBJECT_SURFACE = 1,
		FF_HAS_NORMALS = 2,
		FF_INVISIBLE = 4,
		FF_NEW = 8,
	};
};

}
}
}

#endif
#endif