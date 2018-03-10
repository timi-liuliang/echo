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


#ifndef PXS_FLUID_COLLISION_H
#define PXS_FLUID_COLLISION_H


#include "PsBitUtils.h"
#include "PxsFluidConfig.h"
#include "PxTransform.h"
#include "PxsFluidCollisionData.h"
#include "PxsFluidCollisionMethods.h"
#include "PxsFluidParticle.h"
#include "PxsFluidTwoWayData.h"
#include "PxsFluidCollisionParameters.h"
#include "PsAlloca.h"
#include "PsAlignedMalloc.h"
#include "CmTask.h"
#include "PxsParticleContactManagerStream.h"

#ifdef PX_PS3
#include "CellFluidCollisionTask.h"
#endif

namespace physx
{

class PxsFluid;
class PxsParticleShape;
class PxsRigidBody;
class PxsBodyTransformVault;
struct PxsW2STransformTemp;
class PxBaseTask;

class PxsFluidCollision
{
public:

							PxsFluidCollision(class PxsParticleSystemSim& particleSystem);
							~PxsFluidCollision();

		void				updateCollision(const PxU8* contactManagerStream,
							PxBaseTask& continuation);

		//Update position and velocity of particles that have PxParticleFlag::eSPATIAL_DATA_STRUCTURE_OVERFLOW set.
		void				updateOverflowParticles();
											
		PX_FORCE_INLINE PxsFluidCollisionParameters& getParameter() { return mParams; }

private:
		typedef Ps::Array<PxsW2STransformTemp, shdfnd::AlignedAllocator<16, Ps::ReflectionAllocator<PxsW2STransformTemp> > > TempContactManagerArray;
		struct TaskData 
		{
			TempContactManagerArray tempContactManagers;
			PxsParticleContactManagerStreamIterator packetBegin;
			PxsParticleContactManagerStreamIterator packetEnd;
			PxBounds3 bounds;
		};
		
private:
		void				processShapeListWithFilter(PxU32 taskDataIndex,const PxU32 skipNum = 0);
		void				mergeResults(PxBaseTask* continuation);


		void				updateFluidShapeCollision(PxsFluidParticle* particles,
													  PxsFluidTwoWayData* fluidTwoWayData,
													  PxVec3* transientBuf,
													  PxVec3* collisionVelocities,
													  PxsFluidConstraintBuffers& constraintBufs,	
													  PxsFluidParticleOpcodeCache* opcodeCache,													
													  PxBounds3& worldBounds,													 
													  const PxU32* fluidShapeParticleIndices,
													  const PxF32* restOffsets,
													  const PxsW2STransformTemp* w2sTransforms,
													  const PxsParticleStreamShape& streamShape);

		PX_FORCE_INLINE void updateSubPacket(PxsFluidParticle* particlesSp,											
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
											 const PxF32* restOffsetsSp);														

		void				updateFluidBodyContactPair(	const PxsFluidParticle* particles,
														PxU32 numParticles,
														PxsParticleCollData* particleCollData,
														PxsFluidConstraintBuffers& constraintBufs,													
														PxsFluidParticleOpcodeCache* perParticleCacheLocal,
														PxsFluidLocalCellHash& localCellHash,
														const PxVec3& packetCorner,
														const PxsParticleStreamContactManager& contactManager,
														const PxsW2STransformTemp& w2sTransform);
			
		void PX_FORCE_INLINE addTempW2STransform(TaskData& taskData, const PxsParticleStreamContactManager& cm);

private:
		PxsFluidCollision& operator=(const PxsFluidCollision&);
		PxsFluidCollisionParameters	mParams;
		PxsParticleSystemSim& mParticleSystem;
		TaskData mTaskData[PXS_FLUID_NUM_PACKETS_PARALLEL_COLLISION];
				
		typedef Cm::DelegateTask<PxsFluidCollision, &PxsFluidCollision::mergeResults> MergeTask;
		MergeTask mMergeTask;
		friend class PxsFluidCollisionTask;
#ifdef PX_PS3
		friend class CellFluidCollisionTask;
#endif
};

}

#endif // PXS_FLUID_COLLISION_H
