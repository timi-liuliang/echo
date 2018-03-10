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


#include "PxsParticleSystemSim.h"
#include "PxsContext.h"
#include "PxsParticleShape.h"
#include "CmEventProfiler.h"

//----------------------------------------------------------------------------//

// Standard value for particle resolution
#define PXN_FLUID_REST_PARTICLE_PER_UNIT_STD 10.0f

// Macros to clamp restitution and adhesion (particle collision) to values that give stable results.
#define DYNAMIC_FRICTION_CLAMP 0.001f
#define RESTITUTION_CLAMP 0.05f

#define CLAMP_DYNAMIC_FRICTION(t)	PxClamp(t, DYNAMIC_FRICTION_CLAMP, 1.0f)
#define CLAMP_RESTITUTION(t)		PxClamp(t, 0.0f, 1.0f - RESTITUTION_CLAMP)

using namespace physx;
	
//----------------------------------------------------------------------------//

PxvParticleSystemState& PxsParticleSystemSim::getParticleStateV()
{
	PX_ASSERT(mParticleState);
	return *mParticleState;	
}

//----------------------------------------------------------------------------//
		
void PxsParticleSystemSim::getSimParticleDataV(PxvParticleSystemSimDataDesc& simParticleData, bool) const
{
	simParticleData.densities			= PxStrideIterator<const PxF32>();
	simParticleData.collisionNormals	= PxStrideIterator<const PxVec3>();
	simParticleData.collisionVelocities	= PxStrideIterator<const PxVec3>();
	simParticleData.twoWayImpluses		= PxStrideIterator<const PxVec3>();
	simParticleData.twoWayBodies		= PxStrideIterator<PxvBodyHandle>();	
	
	if (mParticleState->getParticleCount() > 0 && mSimulated)
	{
		if (mParameter->particleReadDataFlags & PxParticleReadDataFlag::eDENSITY_BUFFER)
			simParticleData.densities		= PxStrideIterator<const PxF32>(&mParticleState->getParticleBuffer()->density, sizeof(PxsFluidParticle));
		
		if (mParameter->particleReadDataFlags & PxParticleReadDataFlag::eCOLLISION_NORMAL_BUFFER)
			simParticleData.collisionNormals = PxStrideIterator<const PxVec3>(mTransientBuffer, sizeof(PxVec3));
		
		if (mParameter->particleReadDataFlags & PxParticleReadDataFlag::eCOLLISION_VELOCITY_BUFFER)
			simParticleData.collisionVelocities = PxStrideIterator<const PxVec3>(mCollisionVelocities);

		if(mFluidTwoWayData)
		{
			simParticleData.twoWayImpluses	= PxStrideIterator<const PxVec3>(&mFluidTwoWayData->impulse, sizeof(PxsFluidTwoWayData));
			simParticleData.twoWayBodies	= PxStrideIterator<PxvBodyHandle>(reinterpret_cast<PxvBodyHandle*>(&mFluidTwoWayData->body), sizeof(PxsFluidTwoWayData));	
		}
	}
}

//----------------------------------------------------------------------------//

/**
Will be called from HL twice per step. Once after the shape update (at the start of the frame) has been executed, 
and once after the particle pipeline has finished.
*/
void PxsParticleSystemSim::getShapesUpdateV(PxvParticleShapeUpdateResults& updateResults) const
{
	PX_ASSERT(mIsSimulated);

	updateResults.destroyedShapeCount = mNumDeletedParticleShapes;
	updateResults.destroyedShapes = mCreatedDeletedParticleShapes;

	updateResults.createdShapeCount = mNumCreatedParticleShapes;
	updateResults.createdShapes = mCreatedDeletedParticleShapes + mNumDeletedParticleShapes;
}

//----------------------------------------------------------------------------//

PxBaseTask& PxsParticleSystemSim::schedulePacketShapesUpdate(const PxvParticleShapesUpdateInput& input, PxBaseTask& continuation)
{
	mPacketShapesFinalizationTask.setContinuation(&continuation);
	mPacketShapesUpdateTask.setContinuation(&mPacketShapesFinalizationTask);
	mPacketShapesFinalizationTask.removeReference();
	mPacketShapesUpdateTaskInput = input;
	return mPacketShapesUpdateTask;
}

//----------------------------------------------------------------------------//

PxBaseTask& PxsParticleSystemSim::scheduleDynamicsUpdate(PxBaseTask& continuation)
{
	if (mParameter->flags & PxvInternalParticleSystemFlag::eSPH)
	{
		mDynamicsUpdateTask.setContinuation(&continuation);
		return mDynamicsUpdateTask;
	}
	else
	{
		continuation.addReference();
		return continuation;
	}
}

//----------------------------------------------------------------------------//

PxBaseTask& PxsParticleSystemSim::scheduleCollisionUpdate(PxBaseTask& continuation)
{
	mCollisionFinalizationTask.setContinuation(&continuation);
	mCollisionUpdateTask.setContinuation(&mCollisionFinalizationTask);
	mCollisionFinalizationTask.removeReference();
	return mCollisionUpdateTask;
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::spatialHashUpdateSections(PxBaseTask* continuation)
{
	PX_ASSERT(mParameter->flags & PxvInternalParticleSystemFlag::eSPH);

	// Split each packet into sections and reorder particles of a packet according to these sections
#ifdef PX_PS3
	PxU32 numSpus = mContext.getSceneParamInt(PxPS3ConfigParam::eSPU_FLUID_HASH);
	mSpatialHash->mNumReorderSPU = numSpus;
#endif
	mSpatialHash->updatePacketSections(mPacketParticlesIndices, mParticleState->getParticleBuffer(), continuation);
}

void PxsParticleSystemSim::packetShapesUpdate(PxBaseTask* )
{
	PX_ASSERT(mIsSimulated);
	PX_ASSERT(mSpatialHash);

	// Init parameters for tracking of new/deleted fluid shapes
	mNumCreatedParticleShapes = 0;
	mNumDeletedParticleShapes = 0;

	if (mParticleState->getValidParticleRange() > 0)
	{
		if (!mPacketParticlesIndices)
			mPacketParticlesIndices = reinterpret_cast<PxU32*>(mAlign16.allocate(mParticleState->getMaxParticles()*sizeof(PxU32), __FILE__, __LINE__));

		PxBaseTask* cont;
		if (mParameter->flags & PxvInternalParticleSystemFlag::eSPH)
		{
			cont = &mSpatialHashUpdateSectionsTask;
			mSpatialHashUpdateSectionsTask.setContinuation(&mPacketShapesFinalizationTask);
		}
		else
		{
			cont = &mPacketShapesFinalizationTask;
			cont->addReference();
		}

		// Hash particles to packets and reorder particle indices
#ifdef PX_PS3
		PxU32 numSpus = mContext.getSceneParamInt(PxPS3ConfigParam::eSPU_FLUID_HASH);
		mSpatialHash->mNumHashSPU = numSpus;
#endif
		mSpatialHash->updatePacketHash(mNumPacketParticlesIndices, mPacketParticlesIndices, mParticleState->getParticleBuffer(), 
			mParticleState->getParticleMap(), mParticleState->getValidParticleRange(), cont);
	}
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::packetShapesFinalization(PxBaseTask*)
{
	// - Find for each packet shape the related packet and adjust the mapping.
	// - Track created / deleted packets.
	remapShapesToPackets(mPacketShapesUpdateTaskInput.shapes, mPacketShapesUpdateTaskInput.shapeCount);

	//release the shapes, since their ownership was tranferred to us.
	if (mPacketShapesUpdateTaskInput.shapes)
		PX_FREE(mPacketShapesUpdateTaskInput.shapes);
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::dynamicsUpdate(PxBaseTask* continuation)
{
	PX_ASSERT(mParameter->flags & PxvInternalParticleSystemFlag::eSPH);
	PX_ASSERT(mIsSimulated);
	PX_ASSERT(mSpatialHash);
	PX_ASSERT(continuation);

	if (mNumPacketParticlesIndices > 0)
	{
		updateDynamicsParameter();

		if (mParameter->flags & PxvInternalParticleSystemFlag::eSPH)
		{
			mDynamics.updateSph(*continuation);
	}
}
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::collisionUpdate(PxBaseTask* continuation)
{
	PX_ASSERT(mIsSimulated);
	PX_ASSERT(mSpatialHash);
	PX_ASSERT(mCollisionUpdateTaskInput.contactManagerStream);
	PX_ASSERT(continuation);

	updateCollisionParameter();

	mParticleState->getWorldBounds().setEmpty();

	mCollision.updateCollision(mCollisionUpdateTaskInput.contactManagerStream, *continuation);
	mCollision.updateOverflowParticles();
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::collisionFinalization(PxBaseTask* )
{
	PX_FREE(mCollisionUpdateTaskInput.contactManagerStream);
	mCollisionUpdateTaskInput.contactManagerStream = NULL;

	mSimulated = true;
	
	//clear shape update
	mNumDeletedParticleShapes = 0;
	mNumCreatedParticleShapes = 0;
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::setExternalAccelerationV(const PxVec3& v)
{
	mExternalAcceleration = v;
}

//----------------------------------------------------------------------------//

const PxVec3& PxsParticleSystemSim::getExternalAccelerationV() const
{
	return mExternalAcceleration;
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::setSimulationTimeStepV(PxReal value)
{
	PX_ASSERT(value >= 0.0f);

	mSimulationTimeStep = value;
}

//----------------------------------------------------------------------------//

PxReal PxsParticleSystemSim::getSimulationTimeStepV() const
{
	return mSimulationTimeStep;
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::setSimulatedV(bool isSimulated)
{
	mIsSimulated = isSimulated;
	if(!isSimulated)
		clearParticleConstraints();
}

//----------------------------------------------------------------------------//

Ps::IntBool PxsParticleSystemSim::isSimulatedV() const
{
	return mIsSimulated;
}

//----------------------------------------------------------------------------//

PxsParticleSystemSim::PxsParticleSystemSim(PxsContext* context, PxU32 index) :
	mContext(*context),
	mParticleState(NULL),
	mSimulated(false),
	mFluidTwoWayData(NULL),
	mCreatedDeletedParticleShapes(NULL),
	mPacketParticlesIndices(NULL),
	mNumPacketParticlesIndices(0),
	mOpcodeCacheBuffer(NULL),
	mTransientBuffer(NULL),
	mCollisionVelocities(NULL),
	mDynamics(*this),
	mCollision(*this),
	mIndex(index),
	mPacketShapesUpdateTask(this, "PxsParticleSystemSim.packetShapesUpdate"),
	mPacketShapesFinalizationTask(this, "PxsParticleSystemSim.packetShapesFinalization"),
	mDynamicsUpdateTask(this, "PxsParticleSystemSim.dynamicsUpdate"),
	mCollisionUpdateTask(this, "PxsParticleSystemSim.collisionUpdate"),
	mCollisionFinalizationTask(this, "PxsParticleSystemSim.collisionFinalization"),
	mSpatialHashUpdateSectionsTask(this, "PxsParticleSystemSim.spatialHashUpdateSections")
{
}

//----------------------------------------------------------------------------//

PxsParticleSystemSim::~PxsParticleSystemSim()
{
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::init(PxsParticleData& particleData, const PxvParticleSystemParameter& parameter)
{
	mParticleState = &particleData;
	mParticleState->clearSimState();
	mParameter = &parameter;
	mSimulationTimeStep = 0.0f;
	mExternalAcceleration = PxVec3(0);
	mPacketParticlesIndices = NULL;

	initializeParameter();

	PxU32 maxParticles = mParticleState->getMaxParticles();

	// Initialize buffers
	mConstraintBuffers.constraint0Buf = reinterpret_cast<PxsFluidConstraint*>(mAlign16.allocate(maxParticles*sizeof(PxsFluidConstraint), __FILE__, __LINE__));
	mConstraintBuffers.constraint1Buf = reinterpret_cast<PxsFluidConstraint*>(mAlign16.allocate(maxParticles*sizeof(PxsFluidConstraint), __FILE__, __LINE__));
	if (mParameter->flags & PxParticleBaseFlag::eCOLLISION_WITH_DYNAMIC_ACTORS)
	{
		mConstraintBuffers.constraint0DynamicBuf = reinterpret_cast<PxsFluidConstraintDynamic*>(mAlign16.allocate(maxParticles*sizeof(PxsFluidConstraintDynamic), __FILE__, __LINE__));
		mConstraintBuffers.constraint1DynamicBuf = reinterpret_cast<PxsFluidConstraintDynamic*>(mAlign16.allocate(maxParticles*sizeof(PxsFluidConstraintDynamic), __FILE__, __LINE__));
	}
	else
	{
		mConstraintBuffers.constraint0DynamicBuf = NULL;
		mConstraintBuffers.constraint1DynamicBuf = NULL;
	}

	if ((mParameter->flags & PxParticleBaseFlag::eCOLLISION_TWOWAY)
		&& (mParameter->flags & PxParticleBaseFlag::eCOLLISION_WITH_DYNAMIC_ACTORS))
		mFluidTwoWayData = reinterpret_cast<PxsFluidTwoWayData*>(mAlign16.allocate(maxParticles*sizeof(PxsFluidTwoWayData), __FILE__, __LINE__));


#if defined(PX_CHECKED)
	{
		PxU32 numWords = maxParticles*sizeof(PxsFluidConstraint) >> 2;
		for (PxU32 i = 0; i < numWords; ++i)
		{
			reinterpret_cast<PxU32*>(mConstraintBuffers.constraint0Buf)[i] = 0xDEADBEEF;
			reinterpret_cast<PxU32*>(mConstraintBuffers.constraint1Buf)[i] = 0xDEADBEEF;
		}
	}
#endif

	if (mParameter->flags & PxParticleBaseFlag::ePER_PARTICLE_COLLISION_CACHE_HINT)
	{
		mOpcodeCacheBuffer = reinterpret_cast<PxsFluidParticleOpcodeCache*>(mAlign16.allocate(maxParticles*sizeof(PxsFluidParticleOpcodeCache), __FILE__, __LINE__));
#if defined(PX_CHECKED)
		//sschirm: avoid reading uninitialized mGeom in PxsFluidParticleOpcodeCache::read in assert statement
		PxMemZero(mOpcodeCacheBuffer, maxParticles*sizeof(PxsFluidParticleOpcodeCache));
#endif
	}

	if ((mParameter->flags & PxvInternalParticleSystemFlag::eSPH) || (mParameter->particleReadDataFlags & PxParticleReadDataFlag::eCOLLISION_NORMAL_BUFFER))
		mTransientBuffer = reinterpret_cast<PxVec3*>(mAlign16.allocate(maxParticles*sizeof(PxVec3), __FILE__, __LINE__));

	if (mParameter->particleReadDataFlags & PxParticleReadDataFlag::eCOLLISION_VELOCITY_BUFFER)
		mCollisionVelocities = reinterpret_cast<PxVec3*>(mAlign16.allocate(maxParticles*sizeof(PxVec3), __FILE__, __LINE__));

	mCreatedDeletedParticleShapes = reinterpret_cast<PxvParticleShape**>(PX_ALLOC(2*PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE*sizeof(PxvParticleShape*), PX_DEBUG_EXP("PxvParticleShape*")));
	mNumCreatedParticleShapes = 0;
	mNumDeletedParticleShapes = 0;

	// Create object for spatial hashing.
	mSpatialHash = reinterpret_cast<PxsFluidSpatialHash*>(PX_ALLOC(sizeof(PxsFluidSpatialHash), PX_DEBUG_EXP("PxsFluidSpatialHash")));
	if(mSpatialHash)
	{
		new(mSpatialHash) PxsFluidSpatialHash(
			PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE, 
			mDynamics.getParameter().cellSizeInv,			
			mParameter->packetSizeMultiplierLog2,
			(mParameter->flags & PxvInternalParticleSystemFlag::eSPH) != 0);
	}
#ifdef PX_PS3	
	mDynamics.init((mParameter->flags & PxvInternalParticleSystemFlag::eSPH) != 0);

	PxU32 numSpus = mContext.getSceneParamInt(PxPS3ConfigParam::eSPU_FLUID_HASH);
	mSpatialHash->mNumHashSPU = numSpus;
	mSpatialHash->mNumReorderSPU = numSpus;
#endif

	mCollisionUpdateTaskInput.contactManagerStream = NULL;

	// Make sure we start deactivated.
	mSimulated = false;
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::clear()
{
	mDynamics.clear();

	if (mSpatialHash)
	{
		mSpatialHash->~PxsFluidSpatialHash();
		PX_FREE(mSpatialHash);
		mSpatialHash = NULL;
	}

	// Free particle buffers
	mAlign16.deallocate(mConstraintBuffers.constraint0Buf);
	mConstraintBuffers.constraint0Buf = NULL;

	mAlign16.deallocate(mConstraintBuffers.constraint1Buf);
	mConstraintBuffers.constraint1Buf = NULL;

	if(mConstraintBuffers.constraint0DynamicBuf)
	{
		mAlign16.deallocate(mConstraintBuffers.constraint0DynamicBuf);
		mConstraintBuffers.constraint0DynamicBuf = NULL;
	}

	if(mConstraintBuffers.constraint1DynamicBuf)
	{
		mAlign16.deallocate(mConstraintBuffers.constraint1DynamicBuf);
		mConstraintBuffers.constraint1DynamicBuf = NULL;
	}

	if (mOpcodeCacheBuffer)
	{
		mAlign16.deallocate(mOpcodeCacheBuffer);
		mOpcodeCacheBuffer = NULL;
	}

	if (mTransientBuffer)
	{
		mAlign16.deallocate(mTransientBuffer);
		mTransientBuffer = NULL;
	}

	if (mCollisionVelocities)
	{
		mAlign16.deallocate(mCollisionVelocities);
		mCollisionVelocities = NULL;
	}

	if (mCreatedDeletedParticleShapes)
	{
		PX_FREE(mCreatedDeletedParticleShapes);
		mCreatedDeletedParticleShapes = NULL;
	}

	if (mPacketParticlesIndices)
	{
		mAlign16.deallocate(mPacketParticlesIndices);
		mPacketParticlesIndices = NULL;
	}
	mNumPacketParticlesIndices = 0;

	if (mFluidTwoWayData)
	{
		mAlign16.deallocate(mFluidTwoWayData);
		mFluidTwoWayData = NULL;		
	}

	mSimulated = false;

	if (mParticleState)
	{
		mParticleState->release();
		mParticleState = NULL;
	}
}

//----------------------------------------------------------------------------//

PxsParticleData* PxsParticleSystemSim::obtainParticleState()
{
	PX_ASSERT(mParticleState);
	PxsParticleData* tmp = mParticleState;
	mParticleState = NULL;
	return tmp;
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::remapShapesToPackets(PxvParticleShape*const* shapes, PxU32 numShapes)
{
	PX_ASSERT(mNumCreatedParticleShapes == 0);
	PX_ASSERT(mNumDeletedParticleShapes == 0);

	if (mParticleState->getValidParticleRange() > 0)
	{
		PX_ASSERT(mSpatialHash);
		
		Cm::BitMap mappedFluidPackets;		// Marks the fluid packets that are mapped to a fluid shape.
		mappedFluidPackets.clear(PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE);

		// Find for each shape the corresponding packet. If it does not exist the shape has to be deleted.
		for(PxU32 i=0; i < numShapes; i++)
		{
			PxsParticleShape* shape = static_cast<PxsParticleShape*>(shapes[i]);

			PxU32 hashIndex;
			const PxsParticleCell* particlePacket = mSpatialHash->findCell(hashIndex, shape->getPacketCoordinates());
			if (particlePacket)
			{
				shape->setFluidPacket(particlePacket);

				// Mark packet as mapped.
				mappedFluidPackets.set(hashIndex);
			}
			else
			{
				mCreatedDeletedParticleShapes[mNumDeletedParticleShapes++] = shape;
			}
		}

		// Check for each packet whether it is mapped to a fluid shape. If not, a new shape must be created.
		const PxsParticleCell* fluidPackets = mSpatialHash->getPackets();
		PX_ASSERT((mappedFluidPackets.getWordCount() << 5) >= PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE);
		for(PxU32 p=0; p < PXS_PARTICLE_SYSTEM_PACKET_HASH_SIZE; p++)
		{
			if ((!mappedFluidPackets.test(p)) && (fluidPackets[p].numParticles != PX_INVALID_U32))
			{
				PxsParticleShape* shape = mContext.createFluidShape(this, &fluidPackets[p]);
				if (shape)
				{
					mCreatedDeletedParticleShapes[mNumDeletedParticleShapes + mNumCreatedParticleShapes++] = shape;
				}
			}
		}
	}
	else
	{
		// Release all shapes.
		for(PxU32 i=0; i < numShapes; i++)
		{
			PxsParticleShape* shape = static_cast<PxsParticleShape*>(shapes[i]);
			mCreatedDeletedParticleShapes[mNumDeletedParticleShapes++] = shape;
		}
	}
}

//----------------------------------------------------------------------------//
// Body Shape Reference Invalidation
//----------------------------------------------------------------------------//

/**
Removes all BodyShape references. 
Only the info in the PxsFluidParticle (constraint0Info, constraint1Info) need 
to be cleared, since they are checked before copying references from the constraints 
to the PxsFluidTwoWayData, where it is finally used for dereferencing.
*/
void PxsParticleSystemSim::clearParticleConstraints()
{
	PxsFluidParticle* particleBuffer = mParticleState->getParticleBuffer();
	Cm::BitMap::Iterator it(mParticleState->getParticleMap());
	for (PxU32 particleIndex = it.getNext(); particleIndex != Cm::BitMap::Iterator::DONE; particleIndex = it.getNext())
	{
		PxsFluidParticle& particle = particleBuffer[particleIndex];
		particle.flags.low &= (PxU16)~PxvInternalParticleFlag::eANY_CONSTRAINT_VALID;
	}
}

//----------------------------------------------------------------------------//

/**
Updates shape transform hash from context and removes references to a rigid body that was deleted.
*/
void PxsParticleSystemSim::removeInteractionV(const PxvParticleShape& particleShape, PxvShapeHandle shape, PxvBodyHandle body, bool isDynamic, bool isDyingRb, bool)
{
	const PxsShapeCore* pxsShape = (const PxsShapeCore*)(shape);
	const PxsParticleShape& pxsParticleShape = static_cast<const PxsParticleShape&>(particleShape);

	if (isDyingRb)
	{
		if (isDynamic)
		{
			if (mFluidTwoWayData)
			{
				//just call when packets cover the same particles when constraints where 
				//generated (which is the case with isDyingRb).
				removeTwoWayRbReferences(pxsParticleShape, (const PxsBodyCore*)body);
			}
		}
		else if (mOpcodeCacheBuffer && pxsShape->geometry.getType() == PxGeometryType::eTRIANGLEMESH)
		{
			//just call when packets cover the same particles when cache was used last (must be the last simulation step, 
			//since the cache gets invalidated after one step not being used).
			setCollisionCacheInvalid(pxsParticleShape, pxsShape->geometry);	
		}
	}
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::onRbShapeChangeV(const PxvParticleShape& particleShape, PxvShapeHandle shape, PxvBodyHandle /*body*/)
{
	const PxsShapeCore* pxsShape = (const PxsShapeCore*)(shape);
	const PxsParticleShape& pxsParticleShape = static_cast<const PxsParticleShape&>(particleShape);	
	
	if (mOpcodeCacheBuffer && pxsShape->geometry.getType() == PxGeometryType::eTRIANGLEMESH)
	{
		//just call when packets cover the same particles when cache was used last (must be the last simulation step, 
		//since the cache gets invalidated after one step not being used).
		setCollisionCacheInvalid(pxsParticleShape, pxsShape->geometry);
	}
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::passCollisionInputV(PxvParticleCollisionUpdateInput input) 
{ 
	PX_ASSERT(mCollisionUpdateTaskInput.contactManagerStream == NULL); 
	mCollisionUpdateTaskInput = input; 
}

//----------------------------------------------------------------------------//

/**
Removes specific PxsShapeCore references from particles belonging to a certain shape. 
The constraint data itself needs to be accessed, because it's assumed that if there 
is only one constraint, it's in the slot 1 of the constraint pair. 

Should only be called when packets cover the same particles when constraints where generated!
*/
void PxsParticleSystemSim::removeTwoWayRbReferences(const PxsParticleShape& particleShape, const PxsBodyCore* rigidBody)
{
	PX_ASSERT(mFluidTwoWayData);
	PX_ASSERT(mConstraintBuffers.constraint0DynamicBuf);
	PX_ASSERT(mConstraintBuffers.constraint1DynamicBuf);
	PX_ASSERT(rigidBody);
	PX_ASSERT(particleShape.getFluidPacket());
	const PxsParticleCell* packet = particleShape.getFluidPacket();
	PxsFluidParticle* particleBuffer = mParticleState->getParticleBuffer();

	PxU32 endIndex = packet->firstParticle + packet->numParticles;
	for (PxU32 i = packet->firstParticle; i < endIndex; ++i)
	{
		//update particles for shapes that have been deleted!
		PxU32 particleIndex = mPacketParticlesIndices[i];
		PxsFluidParticle& particle = particleBuffer[particleIndex];
		
		//we need to skip invalid particles 
		//it may be that a particle has been deleted prior to the deletion of the RB
		//it may also be that a particle has been re-added to the same index, in which case
		//the particle.flags.low will have been overwritten
		if (!(particle.flags.api & PxParticleFlag::eVALID))
			continue;

		if (!(particle.flags.low & PxvInternalParticleFlag::eANY_CONSTRAINT_VALID))
			continue;

		PxsFluidConstraint& c0 = mConstraintBuffers.constraint0Buf[particleIndex];
		PxsFluidConstraint& c1 = mConstraintBuffers.constraint1Buf[particleIndex];
		PxsFluidConstraintDynamic& cd0 = mConstraintBuffers.constraint0DynamicBuf[particleIndex];
		PxsFluidConstraintDynamic& cd1 = mConstraintBuffers.constraint1DynamicBuf[particleIndex];
		
		if((PxsBodyCore*)rigidBody == cd1.twoWayBody)
		{	
			particle.flags.low &= PxU16(~(PxvInternalParticleFlag::eCONSTRAINT_1_VALID | PxvInternalParticleFlag::eCONSTRAINT_1_DYNAMIC));
		}

		if((PxsBodyCore*)rigidBody == cd0.twoWayBody)
		{
			if (!(particle.flags.low & PxvInternalParticleFlag::eCONSTRAINT_1_VALID))
			{
				particle.flags.low &= PxU16(~(PxvInternalParticleFlag::eCONSTRAINT_0_VALID | PxvInternalParticleFlag::eCONSTRAINT_0_DYNAMIC));
			}
			else
			{
				c0 = c1;
				cd0 = cd1;
				particle.flags.low &= PxU16(~(PxvInternalParticleFlag::eCONSTRAINT_1_VALID | PxvInternalParticleFlag::eCONSTRAINT_1_DYNAMIC));
			}
		}	
	}
}

//----------------------------------------------------------------------------//

/**
Should only be called when packets cover the same particles when cache was used last.
I.e. after the last collision update and before the next shape update.
It's ok if particles where replaced or removed from the corresponding packet intervalls, 
since the cache updates will not do any harm for those.
*/
void PxsParticleSystemSim::setCollisionCacheInvalid(const PxsParticleShape& particleShape, const Gu::GeometryUnion& geometry)
{
	PX_ASSERT(mOpcodeCacheBuffer);
	PX_ASSERT(particleShape.getFluidPacket());
	const PxsParticleCell* packet = particleShape.getFluidPacket();
	PxsFluidParticle* particleBuffer = mParticleState->getParticleBuffer();

	PxU32 endIndex = packet->firstParticle + packet->numParticles;
	for (PxU32 i = packet->firstParticle; i < endIndex; ++i)
	{
		//update particles for shapes that have been deleted!
		PxU32 particleIndex = mPacketParticlesIndices[i];
		PxsFluidParticle& particle = particleBuffer[particleIndex];

		if ((particle.flags.low & PxvInternalParticleFlag::eGEOM_CACHE_MASK) != 0)
		{
			PxsFluidParticleOpcodeCache& cache = mOpcodeCacheBuffer[particleIndex];
			if (cache.getGeometry() == &geometry)
				particle.flags.low &= ~PxU16(PxvInternalParticleFlag::eGEOM_CACHE_MASK);
		}
	}
}


//----------------------------------------------------------------------------//

void PxsParticleSystemSim::initializeParameter()
{
	const PxvParticleSystemParameter& parameter = *mParameter;

	PxsFluidDynamicsParameters& dynamicsParams = mDynamics.getParameter();

	//initialize dynamics parameter
	{
		PxReal restParticlesDistance		= parameter.restParticleDistance;
		PxReal restParticlesDistanceStd		= 1.0f / PXN_FLUID_REST_PARTICLE_PER_UNIT_STD;
		PxReal restParticlesDistance3		= restParticlesDistance * restParticlesDistance * restParticlesDistance;
		PxReal restParticlesDistanceStd3	= restParticlesDistanceStd * restParticlesDistanceStd * restParticlesDistanceStd;
		PX_UNUSED(restParticlesDistance3);

		dynamicsParams.initialDensity				=	parameter.restDensity;
		dynamicsParams.particleMassStd				=	dynamicsParams.initialDensity * restParticlesDistanceStd3;
		dynamicsParams.cellSize						=	parameter.kernelRadiusMultiplier * restParticlesDistance;
		dynamicsParams.cellSizeInv					=   1.0f / dynamicsParams.cellSize;
		dynamicsParams.cellSizeSq					=	dynamicsParams.cellSize * dynamicsParams.cellSize;
		dynamicsParams.packetSize					=   dynamicsParams.cellSize * (1 << parameter.packetSizeMultiplierLog2);
		PxReal radiusStd					= parameter.kernelRadiusMultiplier * restParticlesDistanceStd;
		PxReal radius2Std					= radiusStd*radiusStd;
		PxReal radius6Std					= radius2Std*radius2Std*radius2Std;
		PxReal radius9Std					= radius6Std*radius2Std*radiusStd;
		PxReal wPoly6ScalarStd				= 315.0f / (64.0f * PxPi * radius9Std);
		PxReal wSpikyGradientScalarStd		= 1.5f * 15.0f / (PxPi * radius6Std);

		dynamicsParams.radiusStd					=	radiusStd;
		dynamicsParams.radiusSqStd					=	radius2Std;
		dynamicsParams.densityMultiplierStd			=	wPoly6ScalarStd * dynamicsParams.particleMassStd;
		dynamicsParams.stiffMulPressureMultiplierStd=	wSpikyGradientScalarStd * dynamicsParams.particleMassStd * parameter.stiffness;
		dynamicsParams.selfDensity					=	dynamicsParams.densityMultiplierStd * radius2Std * radius2Std * radius2Std;
		dynamicsParams.scaleToStd					=	restParticlesDistanceStd / restParticlesDistance;
		dynamicsParams.scaleSqToStd					=	dynamicsParams.scaleToStd * dynamicsParams.scaleToStd;
		dynamicsParams.scaleToWorld					=	1.0f / dynamicsParams.scaleToStd;
		dynamicsParams.packetMultLog				=	parameter.packetSizeMultiplierLog2;
			
		PxReal densityRestOffset			= (dynamicsParams.initialDensity - dynamicsParams.selfDensity);
		dynamicsParams.densityNormalizationFactor	=	(densityRestOffset > 0.0f) ? (1.0f / densityRestOffset) : 0.0f;

		updateDynamicsParameter();
	}

	PxsFluidCollisionParameters& collisionParams =	mCollision.getParameter();
	
	//initialize collision parameter: these partially depend on dynamics parameters!
	{
		collisionParams.cellSize					=	dynamicsParams.cellSize;
		collisionParams.cellSizeInv					=	dynamicsParams.cellSizeInv;
		collisionParams.packetMultLog				=	parameter.packetSizeMultiplierLog2;
		collisionParams.packetMult					=	PxU32(1 << parameter.packetSizeMultiplierLog2);
		collisionParams.packetSize					=	dynamicsParams.packetSize;
		collisionParams.restOffset					=	parameter.restOffset;
		collisionParams.contactOffset				=	parameter.contactOffset;
		PX_ASSERT(collisionParams.contactOffset >= collisionParams.restOffset);
		collisionParams.maxMotionDistance			=	parameter.maxMotionDistance;
		collisionParams.collisionRange				=	collisionParams.maxMotionDistance + collisionParams.contactOffset + PXS_PARTICLE_SYSTEM_COLLISION_SLACK;
		updateCollisionParameter();
	}
}

//----------------------------------------------------------------------------//

PX_FORCE_INLINE PxF32 computeDampingFactor(PxF32 damping, PxF32 timeStep)
{
	PxF32 dampingDt = damping * timeStep;
	if (dampingDt < 1.0f)
		return 1.0f - dampingDt;
	else
		return 0.0f;
}

void PxsParticleSystemSim::updateDynamicsParameter()
{
	const PxvParticleSystemParameter& parameter = *mParameter;
	PxsFluidDynamicsParameters& dynamicsParams = mDynamics.getParameter();

	PxReal restParticlesDistanceStd		= 1.0f / PXN_FLUID_REST_PARTICLE_PER_UNIT_STD;
	PxReal radiusStd					= parameter.kernelRadiusMultiplier * restParticlesDistanceStd;
	PxReal radius2Std					= radiusStd*radiusStd;
	PxReal radius6Std					= radius2Std*radius2Std*radius2Std;
	
	dynamicsParams.viscosityMultiplierStd		=	computeViscosityMultiplier(parameter.viscosity, dynamicsParams.particleMassStd, radius6Std);
}

//----------------------------------------------------------------------------//

void PxsParticleSystemSim::updateCollisionParameter()
{
	const PxvParticleSystemParameter& parameter = *mParameter;
	PxsFluidCollisionParameters& collisionParams =	mCollision.getParameter();

	collisionParams.dampingDtComp				=	computeDampingFactor(parameter.damping, mSimulationTimeStep);
	collisionParams.externalAcceleration		=	mExternalAcceleration;

	collisionParams.projectionPlane.n			=	parameter.projectionPlane.n;
	collisionParams.projectionPlane.d			=	parameter.projectionPlane.d;
	collisionParams.timeStep					=	mSimulationTimeStep;
	collisionParams.invTimeStep					=	(mSimulationTimeStep>0.0f)?1.0f / mSimulationTimeStep:0.0f;
	
	collisionParams.restitution					=	CLAMP_RESTITUTION(parameter.restitution);
	collisionParams.dynamicFriction				=	CLAMP_DYNAMIC_FRICTION(parameter.dynamicFriction);
	collisionParams.staticFrictionSqr			=   parameter.staticFriction*parameter.staticFriction;
	collisionParams.temporalNoise				=	(parameter.noiseCounter*parameter.noiseCounter*4999879) & 0xffff;
	collisionParams.flags						=	parameter.flags;	
}

//----------------------------------------------------------------------------//
