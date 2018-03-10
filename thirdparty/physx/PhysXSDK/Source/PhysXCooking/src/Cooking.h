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


#include "PxCooking.h"
#include "PsUserAllocated.h"

namespace physx
{
class TriangleMeshBuilder;

class Cooking: public PxCooking, public Ps::UserAllocated
{
public:
									Cooking(const PxCookingParams& params): mParams(params) {}

	virtual void					release();
	virtual void					setParams(const PxCookingParams& params);
	virtual const PxCookingParams&	getParams();
	virtual bool					platformMismatch();
	virtual bool					cookTriangleMesh(const PxTriangleMeshDesc& desc, PxOutputStream& stream);
	virtual PxTriangleMesh*			createTriangleMesh(const PxTriangleMeshDesc& desc, PxPhysicsInsertionCallback& insertionCallback);
	virtual bool					validateTriangleMesh(const PxTriangleMeshDesc& desc);

	virtual bool					cookConvexMesh(const PxConvexMeshDesc& desc, PxOutputStream& stream, PxConvexMeshCookingResult::Enum* condition);
	virtual bool					computeHullPolygons(const PxSimpleTriangleMesh& mesh, PxAllocatorCallback& inCallback,PxU32& nbVerts, PxVec3*& vertices,
											PxU32& nbIndices, PxU32*& indices, PxU32& nbPolygons, PxHullPolygon*& hullPolygons);
	virtual bool					cookHeightField(const PxHeightFieldDesc& desc, PxOutputStream& stream);
	virtual PxHeightField*			createHeightField(const PxHeightFieldDesc& desc, PxPhysicsInsertionCallback& insertionCallback);

private:
	bool							loadTriangleMeshDesc(const PxTriangleMeshDesc& desc, TriangleMeshBuilder& meshBuilder);

private:
			PxCookingParams			mParams;
};

}
