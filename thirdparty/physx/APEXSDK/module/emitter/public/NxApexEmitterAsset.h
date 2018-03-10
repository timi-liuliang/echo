/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_EMITTER_ASSET_H
#define NX_APEX_EMITTER_ASSET_H

#include "NxApex.h"
#include "NxEmitterGeoms.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define NX_APEX_EMITTER_AUTHORING_TYPE_NAME "ApexEmitterAsset"

class NxApexEmitterActor;
class NxApexEmitterPreview;
class NxEmitterLodParamDesc;

///APEX Emitter asset. Emits particles within some shape.
class NxApexEmitterAsset : public NxApexAsset
{
protected:
	virtual ~NxApexEmitterAsset() {}

public:
	/**
	\brief Returns the explicit geometry for the all actors based on this asset if the asset is explicit, NULL otherwise
	*/
	virtual NxEmitterExplicitGeom* isExplicitGeom() = 0;

	/**
	\brief Returns the geometry used for the all actors based on this asset
	*/
	virtual const NxEmitterGeom*	getGeom() const = 0;

	///Gets IOFX asset name that is used to visualize partices of this emitter
	virtual const char* 			getInstancedObjectEffectsAssetName(void) const = 0;
	///Gets IOS asset name that is used to simulate partices of this emitter
	virtual const char* 			getInstancedObjectSimulatorAssetName(void) const = 0;
	///Gets IOS asset class name that is used to simulate partices of this emitter
	virtual const char* 			getInstancedObjectSimulatorTypeName(void) const = 0;

	/* Scalable parameter */
	virtual const NxRange<physx::PxF32> & getDensityRange() const = 0; ///< Gets the range used to choose the density of particles
	/* Scalable parameter */
	virtual const NxRange<physx::PxF32> & getRateRange() const = 0;	///< Gets the range used to choose the emission rate

	/* Noise parameter */
	virtual const NxRange<physx::PxVec3> & getVelocityRange() const = 0; ///< Gets the range used to choose the velocity of particles
	/// Gets the range used to choose the lifetime of particles
	virtual const NxRange<physx::PxF32> & getLifetimeRange() const = 0;   /* Noise parameter */
	/// For an explicit emitter, Max Samples is ignored.  For shaped emitters, it is the maximum number of objects spawned in a step.
	virtual physx::PxU32                   getMaxSamples() const = 0;

	/**
	\brief Gets the emitter duration in seconds
	\note If NxApexEmitterActor::startEmit() is called with persistent=true, then this duration is ignored.
	*/
	virtual physx::PxF32				getEmitDuration() const = 0;

	///Gets LOD settings for this asset
	virtual const NxEmitterLodParamDesc& getLodParamDesc() const = 0;

};

///APEX Emitter Asset Authoring. Used to create APEX Emitter assets.
class NxApexEmitterAssetAuthoring : public NxApexAssetAuthoring
{
protected:
	virtual ~NxApexEmitterAssetAuthoring() {}
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_EMITTER_ASSET_H
