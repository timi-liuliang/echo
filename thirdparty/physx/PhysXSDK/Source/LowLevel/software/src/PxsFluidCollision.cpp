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


#include "PxsFluidCollision.h"
#include "PxsFluidConfig.h"
#include "PxsFluidConstants.h"
#include "PxsParticleSystemSim.h"
#include "PxsParticleShape.h"
#include "PxsContext.h"
#include "PxsRigidBody.h"
#include "PxsBodyTransformVault.h"
#include "PxsFluidCollisionHelper.h"
#include "GuConvexMeshData.h"
#include "CmFlushPool.h"

//Enable tuner profiling.
#ifdef PX_PS3
#include "CellTimerMarker.h"
#endif

namespace physx
{

class PxsFluidCollisionTask : public Cm::Task
{
public:
	PxsFluidCollisionTask(PxsFluidCollision& context, PxU32 taskDataIndex) :
	  mCollisionContext(context),
		  mTaskDataIndex(taskDataIndex)
	  {}

	  virtual void runInternal()
	  {
		  mCollisionContext.processShapeListWithFilter(mTaskDataIndex);
	  }

	  virtual const char* getName() const
	  {
		  return "PxsFluidCollision.fluidCollision";
	  }

private:
	PxsFluidCollisionTask& operator=(const PxsFluidCollisionTask&);
	PxsFluidCollision&	mCollisionContext;
	PxU32 mTaskDataIndex; 
};

/* 
how to support dominance-driven one/two-way collision (search for 'todo dominance'):
- add 2-bit flag to PxsBodyShapeRef which stores the dominance matrix values
- store this flag when creating the shape ref in updateFluidBodyContactPair()
- use this flag when copying impulse to collData.shapeImpulse
*/
PxsFluidCollision::PxsFluidCollision(PxsParticleSystemSim& particleSystem): 
mParticleSystem(particleSystem),
mMergeTask(this, "PxsFluidCollision.mergeResults")
{
}

PxsFluidCollision::~PxsFluidCollision()
{
}

void PX_FORCE_INLINE PxsFluidCollision::addTempW2STransform(TaskData& taskData, const PxsParticleStreamContactManager& cm)
{
	PxsW2STransformTemp& cmTemp = taskData.tempContactManagers.insert();

	if(cm.isDynamic)
	{
		const PxsBodyCore* bodyCore = static_cast<const PxsBodyCore*>(cm.rigidCore);
		cmTemp.w2sOld = cm.shapeCore->transform.transformInv(bodyCore->body2Actor).transform(cm.w2sOld->getInverse());
		cmTemp.w2sNew = cm.shapeCore->transform.transformInv(bodyCore->body2Actor).transform(bodyCore->body2World.getInverse());
	}
	else
	{	
		const PxTransform tmp = cm.shapeCore->transform.getInverse() * cm.rigidCore->body2World.getInverse();		
		cmTemp.w2sOld = tmp;
		cmTemp.w2sNew = tmp;
	}	
}

void PxsFluidCollision::updateCollision(const PxU8* contactManagerStream, 
										PxBaseTask& continuation)
{
	mMergeTask.setContinuation(&continuation);
	PxU32 maxTasks = PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION;
	PxU32 packetParticleIndicesCount = mParticleSystem.mNumPacketParticlesIndices;
	
#ifdef PX_PS3
	startTimerMarker(ePARTICLEUPDATECOLLISION);

	//for now we force the ppu fallback if collision velocities are required.
	const PxU32 numSPUs = mParticleSystem.mCollisionVelocities != NULL ? 
		0 : mParticleSystem.getContext().getSceneParamInt(PxPS3ConfigParam::eSPU_FLUID_COLLISION);

	if(numSPUs > 0)
	{
		PX_ASSERT(numSPUs < PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION); //need one for ppu fallback
		maxTasks = numSPUs;
	}
#endif
	//would be nice to get available thread count to decide on task decomposition
	//mParticleSystem.getContext().getTaskManager().getCpuDispatcher();

	// use number of particles for task decomposition

	PxU32 targetParticleCountPerTask = PxMax(PxU32(packetParticleIndicesCount / maxTasks), PxU32(PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_COLLISION));
	PxsParticleContactManagerStreamReader cmStreamReader(contactManagerStream);
	PxsParticleContactManagerStreamIterator cmStreamEnd = cmStreamReader.getEnd();
	PxsParticleContactManagerStreamIterator cmStream = cmStreamReader.getBegin();
	PxsParticleContactManagerStreamIterator cmStreamLast;

	PxU32 numTasks = 0;
	for (PxU32 i = 0; i < PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION; ++i)
	{
		TaskData& taskData = mTaskData[i]; 
		taskData.bounds.setEmpty();

		// if this is the last interation, we need to gather all remaining packets
		if (i == maxTasks - 1)
			targetParticleCountPerTask = 0xffffffff;

		cmStreamLast = cmStream;
		PxU32 currentParticleCount = 0;

		while (currentParticleCount < targetParticleCountPerTask && cmStream != cmStreamEnd)
		{
			PxsParticleStreamShape streamShape;
			cmStream.getNext(streamShape);
			const PxsParticleShape* particleShape = static_cast<const PxsParticleShape*>(streamShape.particleShape);
			currentParticleCount += particleShape->getFluidPacket()->numParticles;
		}

		if (currentParticleCount > 0)
		{
			PX_ASSERT(cmStreamLast != cmStream);
			taskData.packetBegin = cmStreamLast;
			taskData.packetEnd = cmStream;
			numTasks++;
		}
	}
	PX_ASSERT(cmStream == cmStreamEnd);

	// spawn tasks
#ifdef PX_PS3
	if(numSPUs > 0 && numTasks > 0)
	{
		const bool readNormals = mParticleSystem.mParameter->particleReadDataFlags & PxParticleReadDataFlag::eCOLLISION_NORMAL_BUFFER;
		void* ptr = mParticleSystem.getContext().getTaskPool().allocate(sizeof(CellFluidCollisionTask));
		CellFluidCollisionTask* task = PX_PLACEMENT_NEW(ptr, CellFluidCollisionTask)(
			&mParams, mParticleSystem.mParticleState->getParticleBuffer(), mParticleSystem.mPacketParticlesIndices,
			mParticleSystem.mFluidTwoWayData, &mParticleSystem.mConstraintBuffers, mParticleSystem.mOpcodeCacheBuffer,
			mParticleSystem.mParticleState->getMaxParticles(), packetParticleIndicesCount, mParticleSystem.mParticleState->getRestOffsetBuffer(),
			mParticleSystem.mTransientBuffer, readNormals);

		task->setSpuCount(numTasks);

		for (PxU32 i=0; i < numTasks; ++i)
		{
			task->setArgs(&mTaskData[i].bounds, i, mTaskData[i].packetBegin.getStream(), mTaskData[i].packetEnd.getStream());
		}

		task->setContinuation(&mMergeTask);
		task->removeReference();
	}
	else
#endif
	{
		for (PxU32 i = 0; i < numTasks; ++i)
		{
			void* ptr = mParticleSystem.getContext().getTaskPool().allocate(sizeof(PxsFluidCollisionTask));
			PxsFluidCollisionTask* task = PX_PLACEMENT_NEW(ptr, PxsFluidCollisionTask)(*this, i);	
			task->setContinuation(&mMergeTask);		
			task->removeReference();
		}
	}

#ifdef PX_PS3
	if(numSPUs > 0)
	{
		TaskData& fallbackTask = mTaskData[PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION-1]; 
		fallbackTask.packetBegin = cmStreamReader.getBegin();
		fallbackTask.packetEnd = cmStreamReader.getEnd();
		processShapeListWithFilter(PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION-1, PXS_FLUID_MAX_CM_PER_FLUID_SHAPE_SPU+1);

		stopTimerMarker(ePARTICLEUPDATECOLLISION);
	}
#endif

	mMergeTask.removeReference();
}

void PxsFluidCollision::updateOverflowParticles()
{
	//if no particles are present, the hash shouldn't be accessed, as it hasn't been updated. 
	if (mParticleSystem.mParticleState->getValidParticleRange() > 0)
	{
		const PxsParticleCell& overflowCell = mParticleSystem.mSpatialHash->getPackets()[PXS_PARTICLE_SYSTEM_OVERFLOW_INDEX];
		PxsFluidParticle* particles = mParticleSystem.mParticleState->getParticleBuffer();
		PxU32* indices = mParticleSystem.mPacketParticlesIndices;
		for (PxU32 i = overflowCell.firstParticle; i < overflowCell.firstParticle + overflowCell.numParticles; i++)
		{
			PxU32 index = indices[i];
			PxsFluidParticle& particle = particles[index];
			PX_ASSERT((particle.flags.api & PxParticleFlag::eSPATIAL_DATA_STRUCTURE_OVERFLOW) != 0);

			//update velocity and position
			//world bounds are not updated for overflow particles, to make it more consistent with GPU.
			{
				PxVec3 acceleration = mParams.externalAcceleration;
				integrateParticleVelocity(particle, mParams.maxMotionDistance, acceleration, mParams.dampingDtComp, mParams.timeStep);
	
				particle.position = particle.position + particle.velocity*mParams.timeStep;

				// adapted from updateParticle(...) in PxsFluidCollisionHelper.h
				bool projection =  (mParams.flags & PxParticleBaseFlag::ePROJECT_TO_PLANE) != 0;
				if (projection)
				{
					const PxReal dist = mParams.projectionPlane.n.dot(particle.velocity);
					particle.velocity = particle.velocity - ( mParams.projectionPlane.n * dist);		
					particle.position = mParams.projectionPlane.project(particle.position);
				}
				PX_ASSERT(particle.position.isFinite());
			}
		}
	}
}

void PxsFluidCollision::processShapeListWithFilter(PxU32 taskDataIndex,const PxU32 skipNum)
{
	TaskData& taskData = mTaskData[taskDataIndex];

	PxsParticleContactManagerStreamIterator it = taskData.packetBegin;
	while (it != taskData.packetEnd)
	{
		PxsParticleStreamShape streamShape;
		it.getNext(streamShape);

		if(streamShape.numContactManagers < skipNum)
			continue;

		const PxsParticleShape* particleShape = static_cast<const PxsParticleShape*>(streamShape.particleShape);
		PX_ASSERT(particleShape);
		PX_UNUSED(particleShape);

		// Collect world to shape space transforms for all colliding rigid body shapes
		taskData.tempContactManagers.clear();		
		for (PxU32 i = 0; i < streamShape.numContactManagers; i++)
		{
			const PxsParticleStreamContactManager& cm = streamShape.contactManagers[i];
			addTempW2STransform(taskData, cm);
		}

		updateFluidShapeCollision(mParticleSystem.mParticleState->getParticleBuffer(), mParticleSystem.mFluidTwoWayData, 
			mParticleSystem.mTransientBuffer, mParticleSystem.mCollisionVelocities,
			mParticleSystem.mConstraintBuffers,	mParticleSystem.mOpcodeCacheBuffer,	taskData.bounds, mParticleSystem.mPacketParticlesIndices,
			mParticleSystem.mParticleState->getRestOffsetBuffer(), taskData.tempContactManagers.begin(), streamShape);
	}	
}

void PxsFluidCollision::mergeResults(PxBaseTask* /*continuation*/)
{
	PxBounds3& worldBounds = mParticleSystem.mParticleState->getWorldBounds();
	for (PxU32 i = 0; i < PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION; ++i)
		worldBounds.include(mTaskData[i].bounds);
}

void PxsFluidCollision::updateFluidShapeCollision(PxsFluidParticle* particles, 
												  PxsFluidTwoWayData* fluidTwoWayData, 
												  PxVec3* transientBuf,
												  PxVec3* collisionVelocities,
												  PxsFluidConstraintBuffers& constraintBufs,												 
												  PxsFluidParticleOpcodeCache* opcodeCache,												 
												  PxBounds3& worldBounds,
												  const PxU32* fluidShapeParticleIndices, 
												  const PxF32* restOffsets,
												  const PxsW2STransformTemp* w2sTransforms,
												  const PxsParticleStreamShape& streamShape)
{
	const PxsParticleShape& particleShape = *static_cast<const PxsParticleShape*>(streamShape.particleShape);
	PX_ASSERT(particleShape.getFluidPacket());

	const PxsParticleCell& packet = *particleShape.getFluidPacket();

	PxU32 numParticles = packet.numParticles;
	PxU32 firstParticleIndex = packet.firstParticle;
	const PxU32* packetParticleIndices = fluidShapeParticleIndices + firstParticleIndex;
	const PxU32 numParticlesPerSubpacket = PXS_FLUID_SUBPACKET_PARTICLE_LIMIT_COLLISION;

	PX_ALLOCA(particlesSp, PxsFluidParticle, numParticlesPerSubpacket);
	PxF32 restOffsetsSp[numParticlesPerSubpacket];
	
	const PxU32 numHashBuckets = PXS_FLUID_LOCAL_HASH_SIZE_MESH_COLLISION;

	PxU32 hashMemCount = numHashBuckets * sizeof(PxsParticleCell) + numParticlesPerSubpacket * sizeof(PxU32);
	PxU32 cacheMemCount = numParticlesPerSubpacket * sizeof(PxsFluidParticleOpcodeCache);
	PX_ALLOCA(shareMem, PxU8, PxMax(hashMemCount, cacheMemCount));
	
	PxsFluidParticleOpcodeCache* perParticleCacheSp = NULL;
	PxsFluidLocalCellHash localCellHash;
	PxVec3 packetCorner;

	if(opcodeCache)	
		perParticleCacheSp = (PxsFluidParticleOpcodeCache*)(shareMem.mPointer);
	else
	{
		// Make sure the number of hash buckets is a power of 2 (requirement for the used hash function)
		PX_ASSERT((((numHashBuckets-1)^numHashBuckets)+1) == (2*numHashBuckets));
		PX_ASSERT(numHashBuckets > numParticlesPerSubpacket);
		// Set the buffers for the local cell hash
		localCellHash.particleIndices = (PxU32*)(shareMem.mPointer);
		localCellHash.hashEntries = (PxsParticleCell*)(shareMem.mPointer + numParticlesPerSubpacket*sizeof(PxU32));
		packetCorner = PxVec3(PxReal(packet.coords.x), PxReal(packet.coords.y), PxReal(packet.coords.z)) * mParams.packetSize;
	}

	// Divide the packet into subpackets that fit into local memory of processing unit.
	PxU32 particlesRemainder = (numParticles-1) % numParticlesPerSubpacket + 1;

	PxU32 numProcessedParticles = 0;
	PxU32 numParticlesSp = particlesRemainder;	// We start with the smallest subpacket, i.e., the subpacket which does not reach its particle limit.
	while (numProcessedParticles < numParticles)
	{
		const PxU32* particleIndicesSp = packetParticleIndices + numProcessedParticles;

		//load particles (constraints are loaded on demand so far)
		for (PxU32 p=0; p < numParticlesSp; p++)
		{
			PxU32 particleIndex = particleIndicesSp[p];
			particlesSp[p] = particles[particleIndex];			
		}

		if (restOffsets)
		{
			for (PxU32 p=0; p < numParticlesSp; p++)
			{
				PxU32 particleIndex = particleIndicesSp[p];
				restOffsetsSp[p] = restOffsets[particleIndex];
			}
		}
		else
		{
			for (PxU32 p=0; p < numParticlesSp; p++)
				restOffsetsSp[p] = mParams.restOffset;
		}

		updateSubPacket(particlesSp, fluidTwoWayData, transientBuf, collisionVelocities, constraintBufs, perParticleCacheSp, opcodeCache,  
			localCellHash, worldBounds, packetCorner, particleIndicesSp, numParticlesSp, streamShape.contactManagers,
			w2sTransforms, streamShape.numContactManagers, restOffsetsSp);

		// store particles back
		for (PxU32 p=0; p < numParticlesSp; p++)
		{
			PxU32 particleIndex = particleIndicesSp[p];			
			particles[particleIndex] = particlesSp[p];		
		}		

		// Invalidate cached local cell hash
		localCellHash.isHashValid = false;

		numProcessedParticles += numParticlesSp;
		numParticlesSp = numParticlesPerSubpacket;
	}
}

PX_FORCE_INLINE void PxsFluidCollision::updateSubPacket(PxsFluidParticle* particlesSp,
										PxsFluidTwoWayData* fluidTwoWayData, 
										PxVec3* transientBuf,
										PxVec3* collisionVelocities,
										PxsFluidConstraintBuffers& constraintBufs,										
										PxsFluidParticleOpcodeCache* perParticleCacheLocal,
										PxsFluidParticleOpcodeCache* perParticleCacheGlobal,
										PxsFluidLocalCellHash& localCellHash,
										PxBounds3& worldBounds,
										const PxVec3& packetCorner,
										const PxU32* particleIndicesSp,
										const PxU32 numParticlesSp,
										const PxsParticleStreamContactManager* contactManagers,
										const PxsW2STransformTemp* w2sTransforms,
										const PxU32 numContactManagers,
										const PxF32* restOffsetsSp)								
{	
	PxsParticleCollData*  collDataSp = (PxsParticleCollData*)PX_ALLOC(numParticlesSp*sizeof(PxsParticleCollData), PX_DEBUG_EXP("PxsParticleCollData"));
	for (PxU32 p=0; p < numParticlesSp; p++)
	{
		const PxU32 particleIndex = particleIndicesSp[p];
		PxsFluidParticle& particle = particlesSp[p];
		PX_ASSERT(particle.position.isFinite() && particle.velocity.isFinite());
		PxsParticleCollData& collData = collDataSp[p];
		Ps::prefetchLine(&collData);
		collData.c0 = &constraintBufs.constraint0Buf[particleIndex];
		collData.c1 = &constraintBufs.constraint1Buf[particleIndex];
		Ps::prefetchLine(collData.c0);
		Ps::prefetchLine(collData.c1);
		const PxVec3 particleOldVel = particle.velocity;

		//integrate velocity
		{
			PxVec3 acceleration = mParams.externalAcceleration;
			if (mParams.flags & PxvInternalParticleSystemFlag::eSPH)
				acceleration += transientBuf[particleIndex];
			
			integrateParticleVelocity(particle, mParams.maxMotionDistance, acceleration, mParams.dampingDtComp, mParams.timeStep);
		}

		PxVec3 c0Velocity(0.0f);
		PxVec3 c1Velocity(0.0f);
		const PxsBodyCore* c0TwoWayBody = NULL;
		const PxsBodyCore* c1TwoWayBody = NULL;
		if( particle.flags.low & PxvInternalParticleFlag::eCONSTRAINT_0_DYNAMIC)
		{
			c0Velocity = constraintBufs.constraint0DynamicBuf[particleIndex].velocity;
			if(fluidTwoWayData)
				c0TwoWayBody = constraintBufs.constraint0DynamicBuf[particleIndex].twoWayBody;
		}

		if(particle.flags.low & PxvInternalParticleFlag::eCONSTRAINT_1_DYNAMIC)
		{
			c1Velocity = constraintBufs.constraint1DynamicBuf[particleIndex].velocity;
			if(fluidTwoWayData)
				c1TwoWayBody = constraintBufs.constraint1DynamicBuf[particleIndex].twoWayBody;
		}		
		
		initCollDataAndApplyConstraints(collData, particle, particleOldVel, restOffsetsSp[p], c0Velocity, c1Velocity,
			c0TwoWayBody, c1TwoWayBody, particleIndex, mParams);

		collData.particleFlags.low &= PxU16(~(PxvInternalParticleFlag::eCONSTRAINT_0_VALID | PxvInternalParticleFlag::eCONSTRAINT_1_VALID |
			PxvInternalParticleFlag::eCONSTRAINT_0_DYNAMIC | PxvInternalParticleFlag::eCONSTRAINT_1_DYNAMIC));	
	}

	//
	// Collide with dynamic shapes
	
	PxU32 numDynamicShapes = 0;
	for (PxU32 i = 0; i < numContactManagers; i++)
	{
		const PxsParticleStreamContactManager& cm = contactManagers[i];
		if (!cm.isDynamic)
			continue;		

		updateFluidBodyContactPair(particlesSp, numParticlesSp, collDataSp, constraintBufs, 
			perParticleCacheLocal, localCellHash, packetCorner, cm, w2sTransforms[i]);

		numDynamicShapes++;
	}
	
	PxF32 maxMotionDistanceSqr = mParams.maxMotionDistance * mParams.maxMotionDistance;

	if (numDynamicShapes > 0)
	{
		bool isTwoWay = (mParams.flags & PxParticleBaseFlag::eCOLLISION_TWOWAY) != 0;
		for (PxU32 p=0; p < numParticlesSp; p++)
		{
			PxsParticleCollData& collData = collDataSp[p];
			collisionResponse(collData, isTwoWay, false, mParams);			
			clampToMaxMotion(collData.newPos, collData.oldPos, mParams.maxMotionDistance, maxMotionDistanceSqr); 
			collData.flags &= ~PXS_FLUID_COLL_FLAG_CC;
			collData.flags &= ~PXS_FLUID_COLL_FLAG_DC;
			collData.flags |= PXS_FLUID_COLL_FLAG_RESET_SNORMAL; 
			collData.surfacePos = PxVec3(0);
			//we need to keep the dynamic surface velocity for providing collision velocities in finalization 
			//collData.surfaceVel = PxVec3(0);
			collData.ccTime = 1.0f;
		}
	}

	//
	// Collide with static shapes
	// (Static shapes need to be processed after dynamic shapes to avoid that dynamic shapes push
	//  particles into static shapes)
	//

	bool loadedCache = false;
	for (PxU32 i = 0; i < numContactManagers; i++)
	{
		const PxsParticleStreamContactManager& cm = contactManagers[i];
		if (cm.isDynamic)
			continue;	

		const Gu::GeometryUnion& shape = cm.shapeCore->geometry;		
		if( perParticleCacheLocal && (!loadedCache) && (shape.getType() == PxGeometryType::eTRIANGLEMESH))
		{
			for (PxU32 p=0; p < numParticlesSp; p++)
			{
				PxU32 particleIndex = particleIndicesSp[p];
				perParticleCacheLocal[p] = perParticleCacheGlobal[particleIndex];
			}
			loadedCache = true;
		}				

		updateFluidBodyContactPair(particlesSp, numParticlesSp, collDataSp, constraintBufs,  
			perParticleCacheLocal, localCellHash, packetCorner, cm, w2sTransforms[i]);
	}

	if(loadedCache)
	{
		for (PxU32 p=0; p < numParticlesSp; p++)
		{
			PxU32 particleIndex = particleIndicesSp[p];
			perParticleCacheGlobal[particleIndex] = perParticleCacheLocal[p];
		}
	}
	
	for (PxU32 p=0; p < numParticlesSp; p++)
	{
		PxsParticleCollData& collData = collDataSp[p];
		PxsFluidParticle& particle = particlesSp[p];

		collisionResponse(collData, false, true, mParams);

		// Clamp new particle position to maximum motion.
		clampToMaxMotion(collData.newPos, collData.oldPos, mParams.maxMotionDistance, maxMotionDistanceSqr); 

		// Update particle
		updateParticle(particle, collData, (mParams.flags & PxParticleBaseFlag::ePROJECT_TO_PLANE) != 0, mParams.projectionPlane, worldBounds);
	}

	if(transientBuf)
	{
		for (PxU32 p=0; p < numParticlesSp; p++)
		{
			PxsParticleCollData& collData = collDataSp[p];
			transientBuf[collData.origParticleIndex] = collData.surfaceNormal;
		}
	}

	if (collisionVelocities)
	{
		for (PxU32 p=0; p < numParticlesSp; p++)
		{
			PxsParticleCollData& collData = collDataSp[p];
			PxVec3 collisionVelocity = particlesSp[p].velocity - collData.surfaceVel;
			collisionVelocities[collData.origParticleIndex] = collisionVelocity;
		}
	}

	if(fluidTwoWayData)
	{
		for (PxU32 p=0; p < numParticlesSp; p++)
		{
			PxsParticleCollData& collData = collDataSp[p];
			PX_ASSERT(!collData.twoWayBody || ( particlesSp[p].flags.api & PxParticleFlag::eCOLLISION_WITH_DYNAMIC));
			fluidTwoWayData[collData.origParticleIndex].body = collData.twoWayBody;
			fluidTwoWayData[collData.origParticleIndex].impulse = collData.twoWayImpulse;	
		}
	}

	PX_FREE(collDataSp);
}

void PxsFluidCollision::updateFluidBodyContactPair(const PxsFluidParticle* particles, 
												   PxU32 numParticles,
												   PxsParticleCollData* particleCollData, 
												   PxsFluidConstraintBuffers& constraintBufs,												
												   PxsFluidParticleOpcodeCache* opcodeCacheLocal,
												   PxsFluidLocalCellHash& localCellHash,
												   const PxVec3& packetCorner,
												   const PxsParticleStreamContactManager& contactManager,
												   const PxsW2STransformTemp& w2sTransform)
{
	PX_ASSERT(particles);
	PX_ASSERT(particleCollData);

	bool isStaticMeshType = false;

	const Gu::GeometryUnion& shape = contactManager.shapeCore->geometry;
	const PxsBodyCore* body = contactManager.isDynamic ? static_cast<const PxsBodyCore*>(contactManager.rigidCore) : NULL;

	const PxTransform& world2Shape = w2sTransform.w2sNew;
	const PxTransform& world2ShapeOld = w2sTransform.w2sOld;
	const PxTransform shape2World = world2Shape.getInverse();

	for (PxU32 p=0; p < numParticles; p++)
	{
		PxsParticleCollData& collData = particleCollData[p];

		collData.localFlags = (collData.flags & PXS_FLUID_COLL_FLAG_CC);		
		// Transform position from world to shape space
		collData.localNewPos = world2Shape.transform(collData.newPos);
		collData.localOldPos = world2ShapeOld.transform(collData.oldPos);
		collData.c0 = constraintBufs.constraint0Buf + collData.origParticleIndex;
		collData.c1 = constraintBufs.constraint1Buf + collData.origParticleIndex;	
		collData.localSurfaceNormal = PxVec3(0.0f);
		collData.localSurfacePos = PxVec3(0.0f);
	}

	switch (shape.getType())
	{
	case PxGeometryType::eSPHERE :
		{
			collideWithSphere(particleCollData, numParticles, shape, mParams.contactOffset);
			break;
		}
	case PxGeometryType::ePLANE :
		{
			collideWithPlane(particleCollData, numParticles, shape, mParams.contactOffset);
			break;
		}
	case PxGeometryType::eCAPSULE :
		{
			collideWithCapsule(particleCollData, numParticles, shape, mParams.contactOffset);
			break;
		}
	case PxGeometryType::eBOX :
		{
			collideWithBox(particleCollData, numParticles, shape, mParams.contactOffset);
			break;
		}
	case PxGeometryType::eCONVEXMESH :
		{
			const PxConvexMeshGeometryLL& convexShapeData = shape.get<const PxConvexMeshGeometryLL>();
			const Gu::ConvexHullData* convexHullData = convexShapeData.hullData;
			PX_ASSERT(convexHullData);

			PX_ALLOCA(scaledPlanesBuf, PxPlane, convexHullData->mNbPolygons);
			collideWithConvex(scaledPlanesBuf, particleCollData, numParticles, shape, mParams.contactOffset);
			break;
		}
	case PxGeometryType::eTRIANGLEMESH :
		{
			if(opcodeCacheLocal)
			{
				collideWithStaticMesh(numParticles, particleCollData, opcodeCacheLocal,
				shape, world2Shape, shape2World, mParams.cellSize, mParams.collisionRange, mParams.contactOffset);
			}
			else
			{
				// Compute cell hash if needed
				if (!localCellHash.isHashValid)
				{
					PX_ALLOCA(hashKeyArray, PxU16, numParticles * sizeof(PxU16));  //save the hashkey for reorder
					PX_ASSERT(hashKeyArray);
					computeLocalCellHash(localCellHash, hashKeyArray, particles, numParticles, packetCorner, mParams.cellSizeInv);
				}

				collideCellsWithStaticMesh(particleCollData, localCellHash, 
					shape, world2Shape, shape2World, mParams.cellSize, mParams.collisionRange,
					mParams.contactOffset, packetCorner);
			}
			isStaticMeshType = true;
			break;
		}
	case PxGeometryType::eHEIGHTFIELD :
		{
			collideWithStaticHeightField(particleCollData, numParticles, shape, mParams.contactOffset, shape2World);
			isStaticMeshType = true;
			break;
		}
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default :
		PX_ASSERT(0);
	}

	if (isStaticMeshType)
	{
		for (PxU32 p=0; p < numParticles; p++)
		{
			PxsParticleCollData& collData = particleCollData[p];	
			updateCollDataStaticMesh(collData, shape2World, mParams.timeStep);
		}
	}
	else if (body)
	{
		for (PxU32 p=0; p < numParticles; p++)
		{
			PxsParticleCollData& collData = particleCollData[p];		
			PxsFluidConstraintDynamic cdTemp;
			PxsFluidConstraintDynamic& c0Dynamic = constraintBufs.constraint0DynamicBuf ? constraintBufs.constraint0DynamicBuf[collData.origParticleIndex] : cdTemp;
			PxsFluidConstraintDynamic& c1Dynamic = constraintBufs.constraint1DynamicBuf ? constraintBufs.constraint1DynamicBuf[collData.origParticleIndex] : cdTemp;
			c0Dynamic.setEmpty();
			c1Dynamic.setEmpty();
			updateCollDataDynamic(collData, body->body2World, body->linearVelocity, body->angularVelocity,
				body, shape2World, mParams.timeStep, c0Dynamic, c1Dynamic);
		}
	}
	else
	{
		for (PxU32 p=0; p < numParticles; p++)
		{
			PxsParticleCollData& collData = particleCollData[p];
							
			updateCollDataStatic(collData, shape2World, mParams.timeStep);
		}
	}

	if (contactManager.isDrain)
	{
		for (PxU32 p=0; p < numParticles; p++)
		{
			PxsParticleCollData& collData = particleCollData[p];
		
			if ((collData.localFlags & PXS_FLUID_COLL_FLAG_L_ANY) != 0)
			{
				collData.particleFlags.api |= PxParticleFlag::eCOLLISION_WITH_DRAIN;
			}
		}
	}
}

}
