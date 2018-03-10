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


#ifndef PX_PHYSICS_SCP_SHAPESIM
#define PX_PHYSICS_SCP_SHAPESIM

#include "ScElementSim.h"
#include "ScShapeCore.h"
#include "CmPtrTable.h"

namespace physx
{

	class PxsTransformCache;
namespace Gu
{
	class TriangleMesh;
	class HeightField;
}

/** Simulation object corresponding to a shape core object. This object is created when
    a ShapeCore object is added to the simulation, and destroyed when it is removed
*/

struct PxsRigidCore;

namespace Sc
{

	class RigidSim;
	class ShapeCore;
	class Scene;
	class BodySim;
	class StaticSim;

	PX_ALIGN_PREFIX(16)
	class ShapeSim : public ElementSim
	{
		ShapeSim &operator=(const ShapeSim &);
	public:

		// passing in a pointer for the shape to output its bounds allows us not to have to compute them twice.
		// A neater way to do this would be to ask the AABB Manager for the bounds after the shape has been 
		// constructed, but there is currently no spec for what the AABBMgr is allowed to do with the bounds, 
		// hence better not to assume anything.

														ShapeSim(RigidSim&, const ShapeCore& core, PxsRigidBody* atom, PxBounds3* outBounds = NULL);
		virtual											~ShapeSim();

		// ElementSim
		virtual			void							getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const;
		//~ElementSim

						void							reinsertBroadPhase();

						Scene&							getScene()				const;
		PX_FORCE_INLINE	const ShapeCore&				getCore()				const	{ return mCore; }

						// TODO: compile time coupling

		PX_INLINE		PxGeometryType::Enum			getGeometryType()		const	{ return mCore.getGeometryType();	}

		// This is just for getting a reference for the user, so we cast away const-ness

		PX_INLINE		PxShape*						getPxShape()			const	{ return const_cast<PxShape*>(mCore.getPxShape());	}
		
		PX_INLINE		PxReal							getRestOffset()			const	{ return mCore.getRestOffset();		}
		PX_INLINE		PxU32							getFlags()				const	{ return mCore.getFlags();			}
		PX_INLINE		PxReal							getContactOffset()		const	{ return mCore.getContactOffset();	}

						RigidSim&						getRbSim()				const;
						BodySim*						getBodySim()			const;

						PxsRigidCore&					getPxsRigidCore()			const;
						bool							actorIsDynamic()			const;

						void							onFilterDataChange();
						void							onRestOffsetChange();
						void							onFlagChange(PxShapeFlags oldFlags);
						void							onResetFiltering();
						void							onVolumeChange();
						void							onTransformChange();
						void							onMaterialChange();  // remove when material properties are gone from PxcNpWorkUnit

						PxTransform						getAbsPose()						const;

						void							createTransformCache(PxsTransformCache& cache);
						void							destroyTransformCache(PxsTransformCache& cache);

						void							createLowLevelVolume(PxU32 group, const PxBounds3& bounds, PxU32 aggregateID, AABBMgrId aabbMgrId);
						bool							destroyLowLevelVolume();

		PX_FORCE_INLINE	PxU32							getID()								const	{ return mId;		}
		PX_FORCE_INLINE	PxU32							getTransformCacheID()				const	{ return mTransformCacheId;		}

						void							removeFromBroadPhase(bool wakeOnLostTouch);

	private:
						PxU32							mId;
						PxU32							mTransformCacheId;
						
						const ShapeCore&				mCore;

					PxBounds3							computeWorldBounds(const ShapeCore& core, const RigidSim& actor, PxBounds3* uninflatedBounds)				const;
		PX_FORCE_INLINE	void							internalAddToBroadPhase();
		PX_FORCE_INLINE	void							internalRemoveFromBroadPhase();
	}
	PX_ALIGN_SUFFIX(16);

#if !defined(PX_P64)
	PX_COMPILE_TIME_ASSERT(32==sizeof(Sc::ShapeSim)); // after removing bounds from shapes
	PX_COMPILE_TIME_ASSERT((sizeof(Sc::ShapeSim) % 16) == 0); // aligned mem bounds are better for prefetching
#endif

} // namespace Sc

}

#endif
