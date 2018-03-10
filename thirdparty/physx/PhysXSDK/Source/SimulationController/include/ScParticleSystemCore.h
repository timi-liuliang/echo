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


#ifndef PX_PHYSICS_SCP_PARTICLE_SYSTEM_CORE
#define PX_PHYSICS_SCP_PARTICLE_SYSTEM_CORE

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "PsArray.h"
#include "CmBitMap.h"
#include "ScActorCore.h"
#include "PxFiltering.h"
#include "PxvParticleSystemCore.h"
#include "particles/PxParticleBaseFlag.h"
#include "particles/PxParticleFluidReadData.h"

namespace physx
{

class PxParticleCreationData;
class PxParticleBase;
class PxvObjectFactory;
class PxsParticleData;
class PxvParticleSystemState;
class PxBaseTask;

#if PX_SUPPORT_GPU_PHYSX
class PxParticleDeviceExclusiveAccess;
#endif

#define MAX_PARTICLES_PER_PARTICLE_SYSTEM			PXD_PARTICLE_SYSTEM_PARTICLE_LIMIT

#define SPH_KERNEL_RADIUS_MULT						2.0f
#define SPH_REST_DENSITY							1000.0f

namespace Sc
{

	class ParticleSystemSim;

	class ParticleSystemCore : public ActorCore
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
								ParticleSystemCore(const PxEMPTY&) :	ActorCore(PxEmpty), mSimulationFilterData(PxEmpty), mLLParameter(PxEmpty) {}
		void					exportExtraData(PxSerializationContext& stream);
		void					importExtraData(PxDeserializationContext& context);
		static void				getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
		ParticleSystemCore(const PxActorType::Enum&, PxU32, bool);
								~ParticleSystemCore();

		//---------------------------------------------------------------------------------
		// External API
		//---------------------------------------------------------------------------------
		PxParticleBase*			getPxParticleBase();

		PxReal					getStiffness()									const;
		void 					setStiffness(PxReal);

		PxReal					getViscosity()									const;
		void 					setViscosity(PxReal);

		PxReal					getDamping()									const;
		void 					setDamping(PxReal);

		PxReal					getParticleMass()								const;
		void					setParticleMass(PxReal);

		PxReal					getRestitution()								const;
		void 					setRestitution(PxReal);

		PxReal					getDynamicFriction()							const;
		void 					setDynamicFriction(PxReal);

		PxReal					getStaticFriction()								const;
		void 					setStaticFriction(PxReal);

		const PxFilterData&		getSimulationFilterData()						const;
		void					setSimulationFilterData(const PxFilterData& data);
		void					resetFiltering();

		PxParticleBaseFlags		getFlags()										const;
		void					setFlags(PxParticleBaseFlags);
		PxU32					getInternalFlags()								const;
		void					setInternalFlags(PxParticleBaseFlags flags);
		void					notifyCpuFallback();

		PxParticleReadDataFlags	getParticleReadDataFlags()						const;
		void					setParticleReadDataFlags(PxParticleReadDataFlags);

		PxU32 					getMaxParticles()								const;

		PxReal					getMaxMotionDistance()							const;
		void					setMaxMotionDistance(PxReal);
		PxReal					getRestOffset()									const;
		void					setRestOffset(PxReal);
		PxReal					getContactOffset()								const;
		void					setContactOffset(PxReal);
		PxReal					getGridSize()									const;
		void					setGridSize(PxReal);
		PxReal					getRestParticleDistance()						const;
		void					setRestParticleDistance(PxReal);

		//---------------------------------------------------------------------------------------------------------------------------//

		bool 					createParticles(const PxParticleCreationData& creationData);
		void					releaseParticles(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices);
		void					releaseParticles();
		void					setPositions(PxU32 numParticles, const PxStrideIterator<const PxU32>& indexBuffer, const PxStrideIterator<const PxVec3>& positionBuffer);
		void					setVelocities(PxU32 numParticles, const PxStrideIterator<const PxU32>& indexBuffer, const PxStrideIterator<const PxVec3>& velocityBuffer);
		void					setRestOffsets(PxU32 numParticles, const PxStrideIterator<const PxU32>& indexBuffer, const PxStrideIterator<const PxF32>& restOffsetBuffer);
		void					addDeltaVelocities(const Cm::BitMap& bufferMap, const PxVec3* buffer, PxReal multiplier);

		void					getParticleReadData(PxParticleFluidReadData& readData) const;
		
		PxU32					getParticleCount() const;
		const Cm::BitMap&		getParticleMap() const;
		PxBounds3				getWorldBounds() const;			

		//---------------------------------------------------------------------------------------------------------------------------//

		const PxVec3&			getExternalAcceleration()						const;
		void 					setExternalAcceleration(const PxVec3&);

		const PxPlane&			getProjectionPlane()							const;
		void 					setProjectionPlane(const PxPlane& plane);

#if PX_SUPPORT_GPU_PHYSX
		void					enableDeviceExclusiveModeGpu();
		PxParticleDeviceExclusiveAccess*
								getDeviceExclusiveAccessGpu()					const;
		bool					isGpu()											const;
#endif

	public:
		// non-DDI methods:
		PxvParticleSystemParameter&		getLowLevelParameter() { return mLLParameter; }
		ParticleSystemSim*				getSim() const;
		PxsParticleData*				obtainStandaloneData();
		void							returnStandaloneData(PxsParticleData* stateBuffer);

		void							onOriginShift(const PxVec3& shift);

	private:
		PxvParticleSystemState&			getParticleState();
		const PxvParticleSystemState&	getParticleState() const;

	private:
		PxsParticleData*			mStandaloneData;
		PxFilterData				mSimulationFilterData; // The filter data

		// External acceleration is set every frame for the LL sim object
		PxVec3						mExternalAcceleration;

		// Used for two way interaction executed in HL
		PxReal						mParticleMass;

		// Merged particleSystem/particleFluid, API + internal parameter 
		PxvParticleSystemParameter	mLLParameter;
	};

} // namespace Sc

}

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PX_PHYSICS_SCP_PARTICLE_SYSTEM_CORE
