/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLESTRUCTURE_H__
#define __DESTRUCTIBLESTRUCTURE_H__

#include "NxApex.h"
#include "NxApexReadWriteLock.h"
#include "ApexInterface.h"
#include "DestructibleAssetProxy.h"

#include "PsMutex.h"
#include "NxFromPx.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxSphere.h"

#elif NX_SDK_VERSION_MAJOR == 3
#include "PxShape.h"
#include "PxRigidActor.h"
#include <PxRigidDynamic.h>
#include <PxRigidBodyExt.h>
#include <PxShapeExt.h>
#include <PhysX3ClassWrapper.h>
#endif

#ifndef USE_CHUNK_RWLOCK
#define USE_CHUNK_RWLOCK 0
#endif

namespace physx
{
namespace apex
{
namespace destructible
{

class ModuleDestructible;
class DestructibleScene;
typedef class DestructibleStructureStressSolver StressSolver;

#define ENFORCE(condition) extern char unusableName[(condition)?1:-1]
#define GET_OFFSET(Class, Member) physx::PxU64(&(static_cast<Class*>(0)->Member))
struct CachedChunk : public NxApexChunkTransformUnit
{
    CachedChunk(physx::PxU32 chunkIndex_, physx::PxMat44 chunkPose_)
    {
        chunkIndex = chunkIndex_;
        chunkPosition = chunkPose_.getPosition();
        chunkOrientation = physx::PxQuat(physx::PxMat33(chunkPose_.column0.getXYZ(), chunkPose_.column1.getXYZ(), chunkPose_.column2.getXYZ()));
#if defined WIN32
        ENFORCE(GET_OFFSET(CachedChunk, chunkIndex)			== GET_OFFSET(NxApexChunkTransformUnit, chunkIndex));
        ENFORCE(GET_OFFSET(CachedChunk, chunkPosition)		== GET_OFFSET(NxApexChunkTransformUnit, chunkPosition));
        ENFORCE(GET_OFFSET(CachedChunk, chunkOrientation)	== GET_OFFSET(NxApexChunkTransformUnit, chunkOrientation));
        ENFORCE(static_cast<physx::PxU64>(sizeof(*this))	== static_cast<physx::PxU64>(sizeof(NxApexChunkTransformUnit)));
#endif // WIN32
    }
    ~CachedChunk() {}
private:
    CachedChunk();
};
typedef CachedChunk ControlledChunk;
#undef GET_OFFSET
#undef ENFORCE

struct SyncDamageEventCoreDataParams : public NxDamageEventCoreData
{
	SyncDamageEventCoreDataParams()
		:
		destructibleID(0xFFFFFFFF)
	{
		NxDamageEventCoreData::chunkIndexInAsset = 0;
		NxDamageEventCoreData::damage = 0.0f;
		NxDamageEventCoreData::radius = 0.0f;
		NxDamageEventCoreData::position = physx::PxVec3(0.0f);
	}

	physx::PxU32	destructibleID;			// The ID of the destructible actor that is being damaged.
};

struct FractureEvent
{
	FractureEvent() : chunkIndexInAsset(0xFFFFFFFF), destructibleID(0xFFFFFFFF), flags(0), impactDamageActor(NULL), appliedDamageUserData(NULL), deletionWeight(0.0f), damageFraction(1.0f) {}

	enum Flag
	{
		DamageFromImpact =	(1U << 0),
        CrumbleChunk =      (1U << 1),
		DeleteChunk =		(1U << 2),

		SyncDirect	= (1U << 24),		// fracture event is directly sync-ed
		SyncDerived = (1U << 25),		// fracture event is a derivative of a sync-ed damage event
		Manual		= (1U << 26),		// fracture event is manually invoked by the user
		Snap		= (1U << 27),		// fracture event is generated from the destructible stress solver
		Forced		= (1U << 28),
		Silent		= (1U << 29),
		Virtual		= (1U << 30),

		Invalid =	(1U << 31)
	};

	physx::PxVec3			position;					// The position of a single fracture event.
	physx::PxU32			chunkIndexInAsset;			// The chunk index which is being fractured.
	physx::PxVec3			impulse;					// The impulse vector to apply for this fracture event.
	physx::PxU32			destructibleID;				// The ID of the destructible actor that is being damaged.
	physx::PxU32			flags;						// Bit flags describing behavior of this fracture event.
	physx::PxVec3			hitDirection;				// The direction vector this damage being applied to this fracture event.
#if NX_SDK_VERSION_MAJOR == 2
	NxActor const*			impactDamageActor;			// Other PhysX actor that caused damage to NxApexDamageEventReportData.
#elif NX_SDK_VERSION_MAJOR == 3
	physx::PxActor const*	impactDamageActor;			// Other PhysX actor that caused damage to NxApexDamageEventReportData.
#endif
	void*					appliedDamageUserData;		// User data from applyDamage or applyRadiusDamage.
	physx::PxF32			deletionWeight;				// A weighting factor for probabilistic deletion
	physx::PxF32			damageFraction;				// Calculated from damage spread functions, it's good to store this for later use (e.g. impulse scaling)
};

enum ChunkState
{
	ChunkVisible =			    0x01,
	ChunkDynamic =			    0x02,
    //ChunkControlled =         0x04,   // chunk behavior is not locally-determined //unused

	ChunkTemp0  =			    0x10,   // chunk state has been cached
	ChunkTemp1  =			    0x20,   // chunk exists
	ChunkTemp2  =			    0x40,   // chunk is visible
	ChunkTemp3  =			    0x80,   // chunk is dynamic
	ChunkTempMask =			    0xF0,
};

enum ChunkFlag
{
	ChunkCrumbled =				0x01,
	ChunkBelowSupportDepth =	0x02,
	ChunkExternallySupported =	0x04,
	ChunkWorldSupported =		0x08,
	ChunkMissingChild =			0x20,
	ChunkRuntime =				0x80,
    //ChunkGraphical =            0x80,   // chunk has no attached NxShape //unused
};

#if NX_SDK_VERSION_MAJOR == 3
/* A struct for adding forces to actors after they are added to the scene */
struct ActorForceAtPosition
{
	ActorForceAtPosition() : force(0.0f), pos(0.0f), mode(PxForceMode::eFORCE), wakeup(true), usePosition(true) {}

	ActorForceAtPosition(const PxVec3& _force, const PxVec3& _pos, PxForceMode::Enum _mode, bool _wakeup, bool _usePosition)
		: force(_force)
		, pos(_pos)
		, mode(_mode)
		, wakeup(_wakeup) 
		, usePosition(_usePosition)
	{}

	PxVec3 force;
	PxVec3 pos;
	PxForceMode::Enum mode;
	bool wakeup;
	bool usePosition;
};
#endif

class DestructibleStructure : public physx::UserAllocated
{
public:

	enum
	{
		InvalidID =				0xFFFFFFFF,
		InvalidChunkIndex =		0xFFFFFFFF
	};

	struct Chunk
	{
		physx::PxU32			destructibleID;				// The GUID of the destructible actor this chunk is associated with.
		physx::PxU32			reportID;					// A GUID to report state about this chunk
		physx::PxU16			indexInAsset;				// The index into the master asset for this destructible
		physx::PxU8				state;						// bit flags controlling the current 'state' of this chunk.
		physx::PxU8				flags;						// Overall Chunk flags
		physx::PxF32			damage;						// How damaged this chunk is.
		NxSphere				localSphere;				// A local bounding sphere for this chunk.
		const ControlledChunk *	controlledChunk;			// Chunk data given by user
#if USE_CHUNK_RWLOCK
		physx::ReadWriteLock*	lock;
#endif

		physx::PxVec3			localOffset;				// If this chunk is instanced, this may be non-zero.  It needs to be stored somewhere in case we use 
															// the transform of a parent chunk which has a different offset.  Actually, this can all be looked up
															// through a chain of indirection, but I'm storing it here for efficiency.
		physx::PxI32			visibleAncestorIndex;		// Index (in structure) of this chunks' visible ancestor, if any.  If none exists, it's InvalidChunkIndex.

		physx::PxU32			islandID;					// The GUID of the actor associated with the chunk. Used for island reconstruction.

	private:
		physx::Array<NxShape*> 	shapes;						// The rigid body shapes for this chunk.

	public:

		Chunk() {}
		Chunk(const Chunk& other)
		{
			*this = other;
		}

		Chunk&	operator = (const Chunk& other)
		{
			destructibleID			=	other.destructibleID;
			reportID				=	other.reportID;
			indexInAsset			=	other.indexInAsset;
			state					=	other.state;
			flags					=	other.flags;
			damage					=	other.damage;
			localSphere				=	other.localSphere;
			controlledChunk			=	other.controlledChunk;
#if USE_CHUNK_RWLOCK
#error USE_CONTROL_RWLOCK non-zero, but lock is not supported in assignment operator and copy constructor
#endif
			localOffset				=	other.localOffset;
			visibleAncestorIndex	=	other.visibleAncestorIndex;
			islandID				=	other.islandID;
			shapes					=	physx::Array<NxShape*>(other.shapes);
			return *this;
		}

		physx::PxU32	getShapeCount() const
		{
			return shapes.size();
		}

		const NxShape*	getShape(physx::PxU32 shapeIndex) const
		{
			return shapeIndex < shapes.size() ? shapes[shapeIndex] : NULL;
		}

		NxShape*	getShape(physx::PxU32 shapeIndex)
		{
			return shapeIndex < shapes.size() ? shapes[shapeIndex] : NULL;
		}

		bool	isFirstShape(const NxShape* shape) const
		{
			return shapes.size() ? shapes[0] == shape : false;
		}

		void	setShapes(NxShape* const* newShapes, physx::PxU32 shapeCount)
		{
			shapes.resize(shapeCount);
			for (physx::PxU32 i = 0; i < shapeCount; ++i)
			{
				shapes[i] = newShapes[i];
			}
			visibleAncestorIndex = InvalidChunkIndex;
		}

		void	clearShapes()
		{
			shapes.reset();
			visibleAncestorIndex = InvalidChunkIndex;
		}

		bool	isDestroyed() const
		{
			return shapes.empty() && visibleAncestorIndex == (physx::PxI32)InvalidChunkIndex;
		}

		friend class DestructibleStructure;
	};

#if USE_CHUNK_RWLOCK
	class ChunkScopedReadLock : public physx::ScopedReadLock
	{
	public:
		ChunkScopedReadLock(Chunk& chunk) : physx::ScopedReadLock(*chunk.lock) {}
	};

	class ChunkScopedWriteLock : public physx::ScopedWriteLock
	{
	public:
		ChunkScopedWriteLock(Chunk& chunk) : physx::ScopedWriteLock(*chunk.lock) {}
	};
#endif

	DestructibleScene* 			dscene;						// The scene that this destructible structure belongs to
	Array<DestructibleActor*>	destructibles;				// The array of destructible actors associated with this destructible structure
	Array<Chunk>				chunks;						// The array of chunks associated with this structure.
	Array<PxU32>				supportDepthChunks;			//
	Array<PxU32>				overlaps;
	Array<PxU32>				firstOverlapIndices;		// Size = chunks.size()+1, firstOverlapsIndices[chunks.size()] = overlaps.size()
	PxU32						ID;							// The unique GUID associated with this destructible structure
	PxU32						supportDepthChunksNotExternallySupportedCount;
	bool						supportInvalid;
	NxActor*					actorForStaticChunks;
	StressSolver *				stressSolver;
	
	typedef HashMap<NxActor*, physx::PxU32> ActorToIslandMap;
	typedef HashMap<physx::PxU32, NxActor*>	IslandToActorMap;
	// As internal, cache-type containers, these structures do not affect external state
	mutable ActorToIslandMap	actorToIsland;
	mutable IslandToActorMap	islandToActor;

	DestructibleStructure(DestructibleScene* inScene, physx::PxU32 inID);
	~DestructibleStructure();

	bool			addActors(const physx::Array<class DestructibleActor*>& destructiblesToAdd);
	bool			removeActor(DestructibleActor* destructibleToRemove);
	void			setSupportInvalid(bool supportIsInvalid);

	void			updateIslands();
	void			tickStressSolver(PxF32 deltaTime);
	void			visualizeSupport(NiApexRenderDebug* debugRender);

	physx::PxU32	damageChunk(Chunk& chunk, const physx::PxVec3& position, const physx::PxVec3& direction, bool fromImpact, physx::PxF32 damage, physx::PxF32 damageRadius,
								physx::Array<FractureEvent> outputs[], physx::PxU32& possibleDeleteChunks, physx::PxF32& totalDeleteChunkRelativeDamage,
								physx::PxU32& maxDepth, physx::PxU32 depth, physx::PxU16 stopDepth, physx::PxF32 padding);
	void			fractureChunk(const FractureEvent& fractureEvent);
#if APEX_RUNTIME_FRACTURE
	void			runtimeFractureChunk(const FractureEvent& fractureEvent, Chunk& chunk);
#endif
	void			crumbleChunk(const FractureEvent& fractureEvent, Chunk& chunk, const physx::PxVec3* impulse = NULL);	// Add an impulse - used when actor is static
	void			addDust(Chunk& chunk);
	void			removeChunk(Chunk& chunk);
	void			separateUnsupportedIslands();
	void			createDynamicIsland(const physx::Array<physx::PxU32>& indices);
	void			calculateExternalSupportChunks();
	void			buildSupportGraph();
	void			invalidateBounds(const physx::PxBounds3* bounds, physx::PxU32 boundsCount);
	void			postBuildSupportGraph();
	void			evaluateForHitChunkList(const physx::Array<physx::PxU32> & chunkIndices) const;

	physx::PxMat44	getActorForStaticChunksPose()
	{
		if (NULL != actorForStaticChunks)
		{
#if NX_SDK_VERSION_MAJOR == 2
			NxMat34 pose34 = actorForStaticChunks->getGlobalPose();
			PxMat44 pose44;
			PxFromNxMat34(pose44, pose34);
			return pose44;
#elif NX_SDK_VERSION_MAJOR == 3
			return physx::PxMat44(actorForStaticChunks->getGlobalPose());
#endif
		}
		else
		{
			return physx::PxMat44::createIdentity();
		}
	}

	physx::Array<NxShape*>&	getChunkShapes(Chunk& chunk)
	{
		return (chunk.visibleAncestorIndex == (physx::PxI32)InvalidChunkIndex) ? chunk.shapes : chunks[(physx::PxU32)chunk.visibleAncestorIndex].shapes;
	}

	const physx::Array<NxShape*>&	getChunkShapes(const Chunk& chunk) const
	{
		return (chunk.visibleAncestorIndex == (physx::PxI32)InvalidChunkIndex) ? chunk.shapes : chunks[(physx::PxU32)chunk.visibleAncestorIndex].shapes;
	}

	NxActor* getChunkActor(Chunk& chunk);

	const NxActor* getChunkActor(const Chunk& chunk) const;

	bool chunkIsSolitary(Chunk& chunk)
	{
		NxActor* actor = getChunkActor(chunk);
		return (actor == NULL) ? false : (getChunkShapes(chunk).size() == actor->getNbShapes());
	}

	Chunk*	getRootChunk(Chunk& chunk)
	{
		if (chunk.isDestroyed())
		{
			return NULL;
		}
		return chunk.visibleAncestorIndex == (physx::PxI32)InvalidChunkIndex ? &chunk : &chunks[(physx::PxU32)chunk.visibleAncestorIndex];
	}

	physx::PxMat34Legacy getChunkLocalPose(const Chunk& chunk) const;

	void    setChunkGlobalPose(Chunk& chunk, physx::PxMat34Legacy pose)
	{
		physx::Array<NxShape*>& shapes = getChunkShapes(chunk);
		PX_ASSERT(!shapes.empty());
#if NX_SDK_VERSION_MAJOR == 2
		NxMat34 nxpose;
		for (physx::PxU32 i = 0; i < shapes.size(); ++i)
		{
			const NxMat34 shapeLocalPose = shapes[i]->getLocalPose();
			NxMat34 inverseShapeLocalPose;
			shapeLocalPose.getInverse(inverseShapeLocalPose);

			physx::PxMat34Legacy inverseRelativePose;
			PxFromNxMat34(inverseRelativePose, inverseShapeLocalPose);

			physx::PxMat34Legacy newGlobalPose;
			newGlobalPose.multiply(pose, inverseRelativePose);

			NxFromPxMat34(nxpose, newGlobalPose);
			shapes[i]->getActor().setGlobalPose(nxpose);
		}
#elif NX_SDK_VERSION_MAJOR == 3
		for (physx::PxU32 i = 0; i < shapes.size(); ++i)
		{
			const PxTransform shapeLocalPose = shapes[i]->getLocalPose();
			const PxTransform inverseShapeLocalPose = shapeLocalPose.getInverse();
			PxTransform newGlobalPose = pose.toPxTransform() * inverseShapeLocalPose;

			shapes[i]->getActor()->setGlobalPose(newGlobalPose);
		}
#endif
	}

	physx::PxMat34Legacy getChunkActorPose(const Chunk& chunk) const;

	physx::PxMat34Legacy getChunkGlobalPose(const Chunk& chunk) const
	{
		physx::PxMat34Legacy globalPose;
		globalPose.multiply(getChunkActorPose(chunk), getChunkLocalPose(chunk));
		return globalPose;
	}

	physx::PxTransform getChunkLocalTransform(const Chunk& chunk) const
	{
		const physx::Array<NxShape*>* shapes;
		physx::PxVec3 offset;
		if (chunk.visibleAncestorIndex == (physx::PxI32)InvalidChunkIndex)
		{
			shapes = &chunk.shapes;
			offset = physx::PxVec3(0.0f);
		}
		else
		{
			shapes = &chunks[(physx::PxU32)chunk.visibleAncestorIndex].shapes;
			offset = chunk.localOffset - chunks[(physx::PxU32)chunk.visibleAncestorIndex].localOffset;
		}

		PX_ASSERT(!shapes->empty());
		physx::PxTransform transform;
		if (!shapes->empty() && NULL != (*shapes)[0])
		{
#if NX_SDK_VERSION_MAJOR == 2
			PxMat44 localPose;
			PxFromNxMat34(localPose, (*shapes)[0]->getLocalPose());
			transform = PxTransform(localPose);
#else
			transform = (*shapes)[0]->getLocalPose();
#endif
		}
		else
		{
			transform = physx::PxTransform(PxVec3(0, 0, 0), PxQuat(0, 0, 0, 1));
		}
		transform.p += offset;
		return transform;
	}

	physx::PxTransform getChunkActorTransform(const Chunk& chunk) const
	{
		const physx::Array<NxShape*>& shapes = getChunkShapes(chunk);
		PX_ASSERT(!shapes.empty());
		if (!shapes.empty() && NULL != shapes[0])
		{
			// All shapes should have the same actor
#if NX_SDK_VERSION_MAJOR == 2
			PxMat44 globalPose;
			PxFromNxMat34(globalPose, shapes[0]->getActor().getGlobalPose());
			return PxTransform(globalPose);
#else
			return shapes[0]->getActor()->getGlobalPose();
#endif
		}
		else
		{
			return PxTransform(PxVec3(0, 0, 0), PxQuat(0, 0, 0, 1));
		}
	}

	physx::PxTransform getChunkGlobalTransform(const Chunk& chunk) const
	{
		return getChunkActorTransform(chunk).transform(getChunkLocalTransform(chunk));
	}

	void    addChunkImpluseForceAtPos(Chunk& chunk, const physx::PxVec3& impulse, const physx::PxVec3& position, bool wakeup = true);

	physx::PxVec3	getChunkWorldCentroid(const Chunk& chunk) const
	{
		return getChunkGlobalPose(chunk) * PXFROMNXVEC3(chunk.localSphere.center);
	}

	// This version saves a little time on consoles (saving a recalcualtion of the chunk global pose
	PX_INLINE physx::PxVec3	getChunkWorldCentroid(const Chunk& chunk, const physx::PxMat34Legacy& chuckGlobalPose)
	{
		return chuckGlobalPose * PXFROMNXVEC3(chunk.localSphere.center);
	}

	physx::PxU32	newNxActorIslandReference(Chunk& chunk, NxActor& nxActor);
	void			removeNxActorIslandReferences(NxActor& nxActor) const;

	physx::PxU32	getSupportDepthChunkIndices(physx::PxU32* const OutChunkIndices, physx::PxU32  MaxOutIndices) const
	{
		PX_ASSERT( supportDepthChunksNotExternallySupportedCount <= supportDepthChunks.size() );

		physx::PxU32 chunkNum = 0;
		for ( ; chunkNum < supportDepthChunksNotExternallySupportedCount && chunkNum < MaxOutIndices; ++chunkNum )
		{
			physx::PxU32 chunkIndex = supportDepthChunks[chunkNum];
			Chunk const& chunk = chunks[chunkIndex];
			OutChunkIndices[chunkNum] = chunk.indexInAsset;
		}

		return chunkNum;
	}
};

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLESTRUCTURE_H__
