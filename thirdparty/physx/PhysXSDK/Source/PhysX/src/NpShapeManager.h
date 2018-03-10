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


#ifndef PX_PHYSICS_NP_SHAPE_MANAGER
#define PX_PHYSICS_NP_SHAPE_MANAGER

#include "NpShape.h"
#include "CmPtrTable.h"

#if PX_ENABLE_DEBUG_VISUALIZATION
#include "CmRenderOutput.h"
#endif

namespace physx
{

namespace Sq
{
	class SceneQueryManager;
	struct ActorShape;
}

class NpShapeManager : public Ps::UserAllocated
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
	static			void			getBinaryMetaData(PxOutputStream& stream);
					NpShapeManager(const PxEMPTY&);
					void			exportExtraData(PxSerializationContext& stream);
					void			importExtraData(PxDeserializationContext& context);
//~PX_SERIALIZATION
									NpShapeManager();
									~NpShapeManager();

	PX_FORCE_INLINE	PxU32			getNbShapes()	const	{ return mShapes.getCount(); }

	PxU32			getShapes(PxShape** buffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	PX_FORCE_INLINE	NpShape* const*	getShapes()		const	{ return reinterpret_cast<NpShape*const*>(mShapes.getPtrs());	}
	PX_FORCE_INLINE	Sq::ActorShape*const *
									getSceneQueryData()	const { return reinterpret_cast<Sq::ActorShape*const*>(mSceneQueryData.getPtrs()); }

					void			attachShape(NpShape& shape, PxRigidActor& actor);
					void			detachShape(NpShape& s, PxRigidActor &actor, bool wakeOnLostTouch);
					bool			shapeIsAttached(NpShape& s) const;
					void			detachAll(NpScene *scene);

					void			teardownSceneQuery(Sq::SceneQueryManager& sqManager, const NpShape& shape);
					void			setupSceneQuery(Sq::SceneQueryManager& sqManager, const PxRigidActor& actor, const NpShape& shape);

					void			setupSceneQueryInBatch(Sq::SceneQueryManager& sqManager, const PxRigidActor& actor, const NpShape& shape, PxU32 index, bool isDynamic, PxBounds3* bounds);

					PX_FORCE_INLINE void setSceneQueryData(PxU32 index, Sq::ActorShape* data)	
					{
						// for batch insertion optimization; be careful not to violate the invariant that the ActorShape matches the shape and the shape is SQ, which we can't check here.
						// because we don't have the SQ manager
						PX_ASSERT(index<getNbShapes());
						mSceneQueryData.getPtrs()[index] = data;
					}

					void			setupAllSceneQuery(const PxRigidActor& actor);
					void			teardownAllSceneQuery(Sq::SceneQueryManager& sqManager);
					void			markAllSceneQueryForUpdate(Sq::SceneQueryManager& shapeManager);
					
					Sq::ActorShape*
									findSceneQueryData(const NpShape& shape);

					PxBounds3		getWorldBounds(const PxRigidActor&) const;

					void			clearShapesOnRelease(Scb::Scene& s, PxRigidActor&);
					void			releaseExclusiveUserReferences();

#if PX_ENABLE_DEBUG_VISUALIZATION
public:
					void			visualize(Cm::RenderOutput& out, NpScene* scene, const PxRigidActor& actor);
#endif

					// for batching
					const Cm::PtrTable&
									getShapeTable() const 		{	return mShapes; }



protected:
					Sq::ActorShape**	getSqDataInternal()	{ return reinterpret_cast<Sq::ActorShape**>(mSceneQueryData.getPtrs()); }
					void			setupSceneQuery(Sq::SceneQueryManager& sqManager, const PxRigidActor& actor, PxU32 index);
					void			teardownSceneQuery(Sq::SceneQueryManager& sqManager, PxU32 index);
					bool			checkSqActorShapeInternal();


					Cm::PtrTable	mShapes;
					Cm::PtrTable	mSceneQueryData;	// 1-1 correspondence with shapes - TODO: allocate on scene insertion or combine with the shape array for better caching
};

}

#endif
