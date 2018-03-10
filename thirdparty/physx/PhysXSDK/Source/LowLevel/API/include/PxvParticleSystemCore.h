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


#ifndef PXV_PARTICLE_SYSTEM_CORE_H
#define PXV_PARTICLE_SYSTEM_CORE_H

#include "PxvConfig.h"
#include "PxvParticleSystemFlags.h"
#include "particles/PxParticleReadData.h"
#include "PxBounds3.h"
#include "CmBitMap.h"
#include "PxStrideIterator.h"

namespace physx
{

class PxParticleCreationData;
typedef size_t PxvShapeHandle;
typedef size_t PxvBodyHandle;
class PxSerializationContext;

/*!
Particle system / particle fluid parameter. API + internal.
*/
struct PxvParticleSystemParameter
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	PxvParticleSystemParameter(const PxEMPTY&) : particleReadDataFlags(PxEmpty) {}
	PxvParticleSystemParameter() {}

	PxReal						restParticleDistance;
	PxReal						kernelRadiusMultiplier;
	PxReal						viscosity;
	PxReal						surfaceTension;
	PxReal						fadeInTime;
	PxU32						flags;
	PxU32						packetSizeMultiplierLog2;
	PxReal						restitution;
	PxReal						dynamicFriction;
	PxReal						staticFriction;
	PxReal						restDensity;
	PxReal						damping;
	PxReal						stiffness;
	PxReal						maxMotionDistance;
	PxReal						restOffset;
	PxReal						contactOffset;
	PxPlane						projectionPlane;
	PxParticleReadDataFlags		particleReadDataFlags;
	PxU32						noiseCounter;				// Needed for deterministic temporal noise
};


/*!
Descriptor for particle retrieval
*/
struct PxvParticleSystemStateDataDesc
{
	PxU32										maxParticles;
	PxU32										numParticles;
	PxU32										validParticleRange;
	const Cm::BitMap*							bitMap;
	PxStrideIterator<const PxVec3>				positions;
	PxStrideIterator<const PxVec3>				velocities;
	PxStrideIterator<const PxvParticleFlags>	flags;
	PxStrideIterator<const PxF32>				restOffsets;
};


/*!
Descriptor for particle retrieval: TODO
*/
struct PxvParticleSystemSimDataDesc
{
	PxStrideIterator<const PxF32> densities;				//! Particle densities
	PxStrideIterator<const PxVec3> collisionNormals;		//! Particle collision normals
	PxStrideIterator<const PxVec3> collisionVelocities;		//! Particle collision velocities
	PxStrideIterator<const PxVec3> twoWayImpluses;			//! collision impulses(for two way interaction)
	PxStrideIterator<PxvBodyHandle> twoWayBodies;			//! Colliding rigid bodies each particle (zero if no collision)
};


class PxvParticleSystemState
{
public:
    virtual ~PxvParticleSystemState() {}
	virtual	bool		addParticlesV(const PxParticleCreationData& creationData)																			= 0;
	virtual	void		removeParticlesV(PxU32 count, const PxStrideIterator<const PxU32>& indices)															= 0;
	virtual	void		removeParticlesV()																													= 0;
	
	/**
	If fullState is set, the entire particle state is read, ignoring PxParticleReadDataFlags
	*/
	virtual	void		getParticlesV(PxvParticleSystemStateDataDesc& particles, bool fullState, bool devicePtr) const										= 0;
	virtual	void		setPositionsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices, const PxStrideIterator<const PxVec3>& positions)	= 0;
	virtual	void		setVelocitiesV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices, const PxStrideIterator<const PxVec3>& velocities)	= 0;
	virtual	void		setRestOffsetsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices, const PxStrideIterator<const PxF32>& restOffsets)	= 0;
	virtual	void		addDeltaVelocitiesV(const Cm::BitMap& bufferMap, const PxVec3* buffer, PxReal multiplier)											= 0;
	
	virtual	PxBounds3	getWorldBoundsV() const																												= 0;
	virtual	PxU32		getMaxParticlesV() const																											= 0;
	virtual	PxU32		getParticleCountV()	const																											= 0;
};

}

#endif // PXV_PARTICLE_SYSTEM_CORE_H
