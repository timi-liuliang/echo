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


#ifndef __GRB_SCENE_EVENT_DESCS_H__
#define __GRB_SCENE_EVENT_DESCS_H__

#include "PxScene.h"
#include "PxConvexMesh.h"
#include "PxAggregate.h"
#include "PxBroadPhase.h"
#include "PxRigidStatic.h"
#include "PxRigidDynamic.h"
#include "PxMaterial.h"
#include "PxShape.h"
#include "PxGeometryHelpers.h"
#include "geometry/PxGeometry.h"
#include "geometry/PxSphereGeometry.h"
#include "geometry/PxBoxGeometry.h"
#include "geometry/PxCapsuleGeometry.h"
#include "geometry/PxConvexMeshGeometry.h"
#include "geometry/PxPlaneGeometry.h"
#include "geometry/PxHeightFieldGeometry.h"
#include "geometry/PxTriangleMeshGeometry.h"
#include "convex/GuConvexMesh.h"
#include "mesh/GuTriangleMesh.h"
#include "hf/GuHeightField.h"
#include "GrbEvents.h"
#include "GrbStackAllocator.h"




#define GRB_INTEROP_RIGID_STATIC_DESC	1
#define GRB_INTEROP_RIGID_DYNAMIC_DESC	2

namespace physx
{
	
namespace GrbInterop3
{

struct MaterialDesc
{
	GRB_INTEROP_PTR			ptr;
	PxReal					dynamicFriction;				
	PxReal					staticFriction;					
	PxReal					restitution;					
	PxMaterialFlags			flags;							
	PxCombineMode::Enum		frictionCombineMode;
	PxCombineMode::Enum		restitutionCombineMode;
};

struct ShapeDesc
{
	GRB_INTEROP_PTR			ptr;
	PxFilterData			simFilterData;
	PxFilterData			queryFilterData;
	PxShapeFlags			flags;
	PxReal					contactOffset;
	PxReal					restOffset;
	bool					isExclusive;
	PxTransform				localPose;
	PxGeometryHolder		geometry;
	MaterialDesc *			materialDescs;
	PxU32					materialsCount;
};

struct RigidStaticDesc
{
	GRB_INTEROP_PTR			ptr;
	PxTransform				globalPose;
	ShapeDesc *				shapeDescs;
	char *					name;
	PxU32					shapesCount;
	PxActorFlags			actorFlags;
	PxDominanceGroup		dominanceGroup;
	PxClientID				clientID;
	PxU8					type;
};

struct RigidDynamicDesc : public RigidStaticDesc
{
	PxVec3					linearVelocity;
	PxVec3					angularVelocity;
	PxReal					linearDamping;
	PxReal					angularDamping;
	PxReal					contactReportThreshold;
	PxRigidBodyFlags		rigidBodyFlags;
};

struct AggregateDesc
{
	GRB_INTEROP_PTR			ptr;
	GRB_INTEROP_PTR *		actorPtrs;
	PxU32					actorsCount;
};

struct BroadphaseRegionDesc
{
	GRB_INTEROP_PTR			ptr;
	PxBroadPhaseRegion		desc;
	bool					populateRegion;
};


class ClonedTriangleMesh;
class ClonedHeightField;
void clonePhysXGeometry(PxGeometryHolder & clone, const PxGeometryHolder & orig, StackAllocator &stackAlloc);
void clonePhysXName(char * & cloned, const char * orig, StackAllocator & stackAlloc);
void clonePhysXMaterial(MaterialDesc & materialDesc, const PxMaterial * orig);
MaterialDesc * clonePhysXMaterials(PxU32 materialsCount, PxMaterial * const * origMaterials, StackAllocator & stackAlloc);
void clonePhysxShape(ShapeDesc & shapeDesc, const PxShape * shape, StackAllocator & stackAlloc);
void clonePhysXRigidStatic(RigidStaticDesc & rigidStaticDesc, const PxRigidStatic * rigidStatic, StackAllocator & stackAlloc);
void clonePhysXRigidDynamic(RigidDynamicDesc & rigidDynamicDesc, const PxRigidDynamic * rigidDynamic, StackAllocator & stackAlloc);
void releaseShapeDesc(ShapeDesc & shapeDesc);


// avoroshilov: deal with allocators
struct SceneAddRigidStaticEvent
{
	SceneAddRigidStaticEvent() {}

	SceneAddRigidStaticEvent(const PxRigidStatic * rigidStatic, StackAllocator &stackAlloc)
	{
		clonePhysXRigidStatic(rigidStaticDesc, rigidStatic, stackAlloc);
	}

	~SceneAddRigidStaticEvent()
	{
		for (PxU32 i = 0; i < rigidStaticDesc.shapesCount; ++i)
		{
			releaseShapeDesc(rigidStaticDesc.shapeDescs[i]);
		}
	}

	GrbInterop3::RigidStaticDesc rigidStaticDesc;
};

struct SceneAddRigidDynamicEvent
{
	SceneAddRigidDynamicEvent() {}

	SceneAddRigidDynamicEvent(const PxRigidDynamic * rigidDynamic, StackAllocator &stackAlloc)
	{
		clonePhysXRigidDynamic(rigidDynamicDesc, rigidDynamic, stackAlloc);
	}

	~SceneAddRigidDynamicEvent()
	{
		for (PxU32 i = 0; i < rigidDynamicDesc.shapesCount; ++i)
		{
			releaseShapeDesc(rigidDynamicDesc.shapeDescs[i]);
		}
	}

	GrbInterop3::RigidDynamicDesc rigidDynamicDesc;
};

struct SceneAddRigidActorsEvent
{
	SceneAddRigidActorsEvent() {}

	SceneAddRigidActorsEvent(PxActor *const* actors, PxU32 numActors, StackAllocator &stackAlloc)
	{
		numRigidActors = 0;
		rigidActorDescs = (GrbInterop3::RigidStaticDesc **)stackAlloc.allocate(numActors * sizeof(GrbInterop3::RigidStaticDesc *));

		for (PxU32 i = 0; i < numActors; ++i)
		{
			PxActorType::Enum actorType = actors[i]->getType();
			if (actorType == PxActorType::eRIGID_STATIC)
			{
				GrbInterop3::RigidStaticDesc * rigidStaticDesc = (GrbInterop3::RigidStaticDesc *)stackAlloc.allocate(sizeof(GrbInterop3::RigidStaticDesc));

				clonePhysXRigidStatic(*rigidStaticDesc, static_cast<PxRigidStatic *>(actors[i]), stackAlloc);

				rigidActorDescs[numRigidActors] = rigidStaticDesc;
				++numRigidActors;
			}
			else
			if (actorType == PxActorType::eRIGID_DYNAMIC)
			{
				GrbInterop3::RigidDynamicDesc * rigidDynamicDesc = (GrbInterop3::RigidDynamicDesc *)stackAlloc.allocate(sizeof(GrbInterop3::RigidDynamicDesc));

				clonePhysXRigidDynamic(*rigidDynamicDesc, static_cast<PxRigidDynamic *>(actors[i]), stackAlloc);

				rigidActorDescs[numRigidActors] = static_cast<GrbInterop3::RigidStaticDesc *>(rigidDynamicDesc);
				++numRigidActors;
			}
		}
	}

	~SceneAddRigidActorsEvent()
	{
		for (PxU32 j = 0; j < numRigidActors; ++j)
		{
			GrbInterop3::RigidStaticDesc * curDesc = rigidActorDescs[j];
			for (PxU32 i = 0; i < curDesc->shapesCount; ++i)
			{
				releaseShapeDesc(curDesc->shapeDescs[i]);
			}
		}
	}

	GrbInterop3::RigidStaticDesc **	rigidActorDescs;
	PxU32									numRigidActors;
};

struct SceneAddAggregateEvent
{
	SceneAddAggregateEvent() {}

	SceneAddAggregateEvent(const PxAggregate * aggregate, StackAllocator &stackAlloc)
	{
		aggregateDesc.actorsCount = aggregate->getNbActors();
		aggregateDesc.actorPtrs = (GRB_INTEROP_PTR *)stackAlloc.allocate(aggregateDesc.actorsCount * sizeof(GRB_INTEROP_PTR));

		PxActor * userBuf[5];
		for (PxU32 actorCnt = 0; actorCnt < aggregateDesc.actorsCount; actorCnt += 5)
		{
			PxU32 numActorsRemaining = PxMin<PxU32>(5, aggregateDesc.actorsCount - actorCnt);
			aggregate->getActors(userBuf, 5, actorCnt);

			for (PxU32 i = 0; i < numActorsRemaining; ++i)
				aggregateDesc.actorPtrs[actorCnt + i] = TO_GRB_INTEROP_PTR(userBuf[i]);
		}
	}

	GrbInterop3::AggregateDesc aggregateDesc;
};

struct SceneAddBroadphaseRegionEvent
{
	SceneAddBroadphaseRegionEvent() {}

	SceneAddBroadphaseRegionEvent(const PxBroadPhaseRegion * bpRegion, bool populateRegion, StackAllocator& /*stackAlloc*/ )
	{
		bpRegionDesc.ptr = TO_GRB_INTEROP_PTR(bpRegion);
		bpRegionDesc.desc = *bpRegion;
		bpRegionDesc.populateRegion = populateRegion;
	}

	GrbInterop3::BroadphaseRegionDesc bpRegionDesc;
};

struct SceneUpdateMaterialEvent
{
	SceneUpdateMaterialEvent() {}

	SceneUpdateMaterialEvent(const PxMaterial * material, StackAllocator& /*stackAlloc*/)
	{
		clonePhysXMaterial(materialDesc, material);
	}

	GrbInterop3::MaterialDesc materialDesc;
};




//-----------------------------------------------------------------------------
struct ActorCreateShapeEvent
{
	ActorCreateShapeEvent(
		const PxActor * actor,
		const PxShape * shape,
		StackAllocator &stackAlloc
		)
	{
		pxActorPtr = TO_GRB_INTEROP_PTR(actor);
		clonePhysxShape(shapeDesc, shape, stackAlloc);
	}

	GrbInterop3::ShapeDesc	shapeDesc;
	GRB_INTEROP_PTR					pxActorPtr;
};
//-----------------------------------------------------------------------------
struct ActorAttachShapeEvent
{
	ActorAttachShapeEvent(
		const PxActor * actor,
		const PxShape * shape,
		StackAllocator &stackAlloc
		)
	{
		pxActorPtr = TO_GRB_INTEROP_PTR(actor);
		clonePhysxShape(shapeDesc, shape, stackAlloc);
	}

	GrbInterop3::ShapeDesc	shapeDesc;
	GRB_INTEROP_PTR					pxActorPtr;
};
//-----------------------------------------------------------------------------
struct ShapeSetGeometryEvent
{
	ShapeSetGeometryEvent(
		const PxShape * shape,
		StackAllocator &stackAlloc
		)
	{
		pxShapePtr = TO_GRB_INTEROP_PTR(shape);
		clonePhysXGeometry(geometry, shape->getGeometry(), stackAlloc);
	}

	PxGeometryHolder				geometry;
	GRB_INTEROP_PTR					pxShapePtr;
};


struct SceneFetchResultsEvent : public physx::shdfnd::UserAllocated
{
	struct ActorUpdateData
	{
		PxTransform		pose;
		PxVec3			linVel;
		PxVec3			angVel;
		GRB_INTEROP_PTR	actor;
	};

	void set(PxScene & scene);
	void clear()				{	updates.clear();	}		// note this doesn't actually free any memory	
	bool isEmpty()				{	return updates.begin() == updates.end();	}

	shdfnd::Array<ActorUpdateData> updates;
};
struct MaterialDesc;



struct PoolSet : public Ps::UserAllocated
{
	GrbInteropEvent3Pool<GrbInterop3::SceneFetchResultsEvent>	sceneFetchResultsEventPool;
};

}
}

#endif
