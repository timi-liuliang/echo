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


#ifndef GRB_ENVENTS_H_
#define GRB_ENVENTS_H_

#include "PsArray.h"
#include "PxMat33.h"

#if PX_SUPPORT_GPU_PHYSX
#	define USE_GRB_INTEROP	1
#else
#	define USE_GRB_INTEROP	0
#endif
#define GRB_INTEROP_PTR	PxU64
#define TO_GRB_INTEROP_PTR( pointer ) static_cast<GRB_INTEROP_PTR>(reinterpret_cast<size_t>(pointer))

namespace physx
{
#if (USE_GRB_INTEROP == 1)

class PxMaterial;
class PxConvexMesh;
class PxAggregate;
class PxBroadPhase;
struct PxBroadPhaseRegion;
class PxScene;
class PxActor;
class PxShape;
class PxRigidDynamic;
class PxRigidStatic;
class PxGeometry;
struct PxFilterData;


namespace GrbInterop3
{
	class StackAllocator;
	struct PoolSet;
	struct SceneAddRigidStaticEvent;
	struct SceneAddRigidDynamicEvent;
	struct SceneAddRigidActorsEvent;
	struct SceneAddAggregateEvent;
	struct SceneAddBroadphaseRegionEvent;
	struct SceneUpdateMaterialEvent;
	struct SceneFetchResultsEvent;
	struct ActorCreateShapeEvent;
	struct ActorAttachShapeEvent;
	struct ShapeSetGeometryEvent;
	struct MaterialDesc;
}

//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
//Class for communicating the details of a PhysX API call to GRB


class GrbInteropEvent3
{

private:
	GrbInteropEvent3& operator=(const GrbInteropEvent3&);

public:
	enum Type
	{
		//PxScene events
		PxSceneSetGravity,
		PxSceneAddRigidStatic,
		PxSceneAddRigidDynamic,
		PxSceneAddRigidActors,
		PxSceneRemoveActor,
		PxSceneAddAggregate,
		PxSceneRemoveAggregate,
		PxSceneAddBroadphaseRegion,
		PxSceneRemoveBroadphaseRegion,
		PxSceneCreateActor,
		PxSceneFetchResults,
		PxSceneUpdateMaterial,
		PxSceneRemoveMaterial,
		PxSceneRelease,


		//PxActor events
		PxActorCreateShape,
		PxActorAttachShape,
		PxActorDetachShape,
		PxActorSetFlags,
		PxActorSetDominanceGroup,
		PxActorSetName,

		PxActorSetRigidBodyFlags,
		PxActorSetContactReportThreshold,


		//PxShapeEvents
		PxShapeSetGeometry,
		PxShapeSetLocalPose,
		PxShapeSetSimulationFilterData,
		PxShapeSetQueryFilterData,
		PxShapeResetFiltering,
		PxShapeSetMaterials,
		PxShapeSetContactOffset,
		PxShapeSetRestOffset,
		PxShapeSetFlags,
		PxShapeSetName,
		PxShapeRelease,


		//PxMaterial events
		/*
		PxMaterialLoadFromDesc,
		PxMaterialSetRestitution,
		PxMaterialSetDynamicFriction,
		PxMaterialSetStaticFriction,
		PxMaterialSetRestitutionCombineMode,
		PxMaterialSetFrictionCombineMode,
		*/


		//PxAggregate events
		PxAggregateAddActor,
		PxAggregateRelease,


		ForceSizeTo32Bits = 0xFFFFFFFF
	};

	//PxScene events

	GrbInteropEvent3(GrbInterop3::StackAllocator &stackAlloc): mStackAlloc(stackAlloc) {}

	typedef GrbInterop3::StackAllocator StAlloc;
	// PxScene
	GrbInteropEvent3(StAlloc& sa, Type t, const PxVec3 & vec);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxRigidStatic * rigidStatic);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxRigidDynamic * rigidDynamic);
	GrbInteropEvent3(StAlloc& sa, Type t, PxActor *const* actors, PxU32 actorsCount);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxAggregate * aggregate);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxAggregate * aggregate, PxU32);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxBroadPhaseRegion * bpRegion, bool populateRegion);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxMaterial * material);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxMaterial * material, PxU32);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor * actor);
	GrbInteropEvent3(StAlloc& sa, Type t, GrbInterop3::PoolSet & pools, PxScene & scene);
	GrbInteropEvent3(StAlloc& sa, Type t, PxScene * scene);
	GrbInteropEvent3(StAlloc& sa, Type t, PxU32 pxu32);

	// PxActor
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor* actor, const PxShape* shape, PxU32 creationFlag);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor* actor, const PxShape * shape);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor* actor, PxU8 pxu8);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor* actor, PxU16 pxu16);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor* actor, PxU32 pxu32);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor* actor, PxReal real);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxActor* actor, const char * name);

	// PxShape
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, const PxGeometry&);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, PxMaterial*const* materials, PxU32 materialCount);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, PxU8 pxu8);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, PxU16 pxu16);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, PxU32 pxu32);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, PxReal real);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, const PxTransform & trans);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, const PxFilterData & simFilterData);
	GrbInteropEvent3(StAlloc& sa, Type t, const PxShape* shape, const char * name);

	// PxAggregate
	GrbInteropEvent3(StAlloc &sa, Type t, const PxAggregate * aggregate, PxActor * actor);


	//inheriting all events from GrbInteropEvent3 in order to use a virtual destructor was more lines of code
	void release()
	{
		//switch(type)
		//{
		//default:
		//	break;
		//}
	}

	StAlloc &mStackAlloc;
	const void *	object;
	Type			type;

	PxVec3 &					vec3()							{ return *(PxVec3*)arg.buf; }
	PxMat33 &					mat33()							{ return *(PxMat33*)arg.buf; }
	PxQuat &					quat()							{ return *(PxQuat*)arg.buf; }
	PxTransform &				transform()						{ return *(PxTransform*)arg.buf; }
	PxFilterData &				filterData()					{ return *(PxFilterData*)arg.buf; }

	union
	{
		PxU8	buf[sizeof(PxMat33)];															// buf should have sizeof of the largest element

		char * name;																			// Clone, do read in GRB

		const char *	charPtr;																// Not a clone, do not read in GRB
		GRB_INTEROP_PTR shapePtr;
		GRB_INTEROP_PTR actorPtr;
		GRB_INTEROP_PTR materialPtr;
		GRB_INTEROP_PTR aggregatePtr;

		PxU8	pxu8;																			// u8, u16, u32 - internal GRB types
		PxU16	pxu16;
		PxU32	pxu32;
		PxU64	pxu64;
		PxReal	real;

		struct
		{
			struct GrbInterop3::MaterialDesc * materials;
			PxU32 materialsCount;
		} setMaterialsPair;

		GrbInterop3::SceneAddRigidStaticEvent *		sceneAddRigidStaticEventPtr;								// OK to dereference
		GrbInterop3::SceneAddRigidDynamicEvent *		sceneAddRigidDynamicEventPtr;								// OK to dereference
		GrbInterop3::SceneAddRigidActorsEvent *		sceneAddRigidActorsEventPtr;								// OK to dereference
		GrbInterop3::SceneAddAggregateEvent *			sceneAddAggregateEventPtr;									// OK to dereference
		GrbInterop3::SceneAddBroadphaseRegionEvent *	sceneAddBroadphaseRegionEventPtr;							// OK to dereference
		GrbInterop3::SceneUpdateMaterialEvent *		sceneUpdateMaterialEventPtr;								// OK to dereference
		GrbInterop3::SceneFetchResultsEvent *			sceneFetchResultsEventPtr;									// OK to dereference
		GrbInterop3::ActorCreateShapeEvent *			actorCreateShapeEventPtr;									// OK to dereference
		GrbInterop3::ActorAttachShapeEvent *			actorAttachShapeEventPtr;									// OK to dereference
		GrbInterop3::ShapeSetGeometryEvent *			shapeSetGeometryEventPtr;									// OK to dereference
	} arg;
};



PxU32				grbGetNbEventStreams(PxScene& scene);
GrbInterop3::StackAllocator&		
					grbGetEventStreamAlloc(PxScene& scene, PxU32 streamIndex);
void				grbSendEvent(PxScene& scene, const GrbInteropEvent3&, PxU32 streamIndex);

#endif	// #if (USE_GRB_INTEROP == 1)
}


#if (USE_GRB_INTEROP == 1)
#	define GRB_EVENT(scene, e, ...)																	\
	{																								\
																									\
	if (scene)																						\
	{																								\
		PxU32 nbEventStreams = grbGetNbEventStreams(*scene);										\
		for (PxU32 i = 0; i < nbEventStreams; i++)													\
			grbSendEvent(*scene, GrbInteropEvent3(grbGetEventStreamAlloc(*scene, i), __VA_ARGS__), i);	\
	}																								\
}
#else
#	define GRB_EVENT(scene, e, ...)
#endif


#endif
