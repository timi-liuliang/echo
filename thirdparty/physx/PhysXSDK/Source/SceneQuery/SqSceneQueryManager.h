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


#ifndef PX_PHYSICS_SCENEQUERYMANAGER
#define PX_PHYSICS_SCENEQUERYMANAGER
/** \addtogroup physics 
@{ */

#include "PxSceneDesc.h" // PxPruningStructure
#include "SqPruner.h"
#include "CmBitMap.h"
#include "PsArray.h"
#include "GuRaycastTests.h" // need to include to cache entry table
#include "GuOverlapTests.h" // need to include to cache entry table
#include "GuSweepTests.h" // need to include cache entry table
#include "PxActor.h" // needed for offset table
#include "ScbActor.h" // needed for offset table

// threading
#include "PsSync.h"

namespace physx
{

class PxBounds3;
class NpShape;
class NpBatchQuery;

namespace Scb
{
	class Scene;
	class Actor;
	class Shape;
}

namespace Sc
{
	class ActorCore;
}

namespace Sq
{
	struct ActorShape;

	struct OffsetTable
	{
		PX_FORCE_INLINE	OffsetTable() {}
		PX_FORCE_INLINE const Scb::Actor&		convertPxActor2Scb(const PxActor& actor) const		{ return *Ps::pointerOffset<const Scb::Actor*>(&actor, pxActorToScbActor[actor.getConcreteType()]); }
		PX_FORCE_INLINE const Sc::ActorCore&	convertScbActor2Sc(const Scb::Actor& actor) const	{ return *Ps::pointerOffset<const Sc::ActorCore*>(&actor, scbToSc[actor.getScbType()]); }

		ptrdiff_t pxActorToScbActor[PxConcreteType::ePHYSX_CORE_COUNT];
		ptrdiff_t scbToSc[ScbType::TYPE_COUNT];
	};
	extern OffsetTable gOffsetTable;

	class SceneQueryManager : public Ps::UserAllocated
	{
		PX_NOCOPY(SceneQueryManager)
	public:
														SceneQueryManager(Scb::Scene& scene, const PxSceneDesc& desc);
														~SceneQueryManager();

						ActorShape*						addShape(const NpShape& shape, const PxRigidActor& actor, bool dynamic, PxBounds3* bounds=NULL);
						void							removeShape(ActorShape* shapeData);
						const PrunerPayload&			getPayload(const ActorShape* shapeData) const;
						// returns true if current combination of pruners is accepted on SPU:
						bool							canRunOnSPU(const NpBatchQuery& bq) const; 
						// sets runOnPPU (fallback) accordingly for each query type
						void							fallbackToPPUByType(const NpBatchQuery& bq, bool runOnPPU[3]) const; 
						// asserts if !canRunOnSPU()
						void							blockingSPURaycastOverlapSweep(NpBatchQuery* bq, bool runOnPPU[3]); 
						void							freeSPUTasks(NpBatchQuery* bq);
						void							checkSPUSweepHasMtdFlag(NpBatchQuery* bq); 

	public:
		PX_FORCE_INLINE	Scb::Scene&						getScene()						const	{ return mScene;							}
						PxScene*						getPxScene()					const;
		PX_FORCE_INLINE	PxU32							getDynamicTreeRebuildRateHint()	const	{ return mRebuildRateHint;					}

		PX_FORCE_INLINE	PxPruningStructure::Enum		getStaticStructure()			const	{ return mPrunerType[0];					}
		PX_FORCE_INLINE	PxPruningStructure::Enum		getDynamicStructure()			const	{ return mPrunerType[1];			}

		PX_FORCE_INLINE	const Pruner*					getStaticPruner()				const	{ return mPruners[0];						}
		PX_FORCE_INLINE	const Pruner*					getDynamicPruner()				const	{ return mPruners[1];						}

		PX_FORCE_INLINE void							invalidateStaticTimestamp()				{ mTimestamp[0]++;	}
		PX_FORCE_INLINE void							invalidateDynamicTimestamp()			{ mTimestamp[1]++;	}
		PX_FORCE_INLINE PxU32							getStaticTimestamp()			const	{ return mTimestamp[0];	}
		PX_FORCE_INLINE PxU32							getDynamicTimestamp()			const	{ return mTimestamp[1];	}

						void							preallocate(PxU32 staticShapes, PxU32 dynamicShapes);
						void							markForUpdate(Sq::ActorShape* s);
						void							setDynamicTreeRebuildRateHint(PxU32 dynTreeRebuildRateHint);
						
						void							flushUpdates();
						void							forceDynamicTreeRebuild(bool rebuildStaticStructure, bool rebuildDynamicStructure);

		// Force a rebuild of the aabb/loose octree etc to allow raycasting on multiple threads.
						void							processSimUpdates();

						void							shiftOrigin(const PxVec3& shift);

						Gu::GeomRaycastTableEntry7&		mCachedRaycastFuncs;
						const Gu::GeomSweepFuncs&		mCachedSweepFuncs;
						Gu::GeomOverlapTableEntry7*		mCachedOverlapFuncs;

	private:
						Pruner*							mPruners[2];	// 0 = static, 1 = dynamic
						PxU32							mTimestamp[2];

						Cm::BitMap						mDirtyMap[2];
						Ps::Array<ActorShape*>			mDirtyList;

						PxPruningStructure::Enum		mPrunerType[2];
						PxU32							mRebuildRateHint;

						Scb::Scene&						mScene;

						// threading
						shdfnd::Mutex					mSceneQueryLock;  // to make sure only one query updates the dirty pruner structure if multiple queries run in parallel

						void							flushShapes();
		PX_FORCE_INLINE	bool							updateObject(PxU32 index, PrunerHandle handle);
		PX_FORCE_INLINE void							processActiveShapes(ActorShape** PX_RESTRICT shapes, PxU32 nb);
		
		static PxU32 getPrunerIndex(const ActorShape* ref)		{ return static_cast<PxU32>(reinterpret_cast<size_t>(ref))&1;				}
		static PxU32 getPrunerHandle(const ActorShape* ref)		{ return static_cast<PxU32>(reinterpret_cast<size_t>(ref))>>2;				}

		// ensure there is always one set bit, so there is never confusion with null ptr
		static ActorShape* createRef(PxU32 index, PxU32 handle)	{ return reinterpret_cast<ActorShape*>(size_t((handle<<2) | 2 | index));	}

		static Pruner*									createPruner(PxPruningStructure::Enum type);
	};

} // namespace Sq

}

/** @} */
#endif
