/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_IOFX_ASSET_H
#define NX_IOFX_ASSET_H

#include "NxApex.h"
#include "NxModifierDefs.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#define NX_IOFX_AUTHORING_TYPE_NAME "IOFX"

class NxModifier;
class ApexActor;

/**
	IOFX Asset public interface. Used to define the way the visual parameters are created
	from physical parameters of a particle
*/
class NxIofxAsset : public NxApexAsset, public NxApexContext
{
public:
	///get the name of the material used for sprite-based particles visualization
	virtual const char*							getSpriteMaterialName() const = 0;

	///get the number of different mesh assets used for mesh-based particles visualization
	virtual physx::PxU32						getMeshAssetCount() const = 0;
	///get the name of one of the mesh assets used for mesh-based particles visualization
	/// \param index mesh asset internal index
	virtual const char*							getMeshAssetName(physx::PxU32 index = 0) const = 0;
	///get the weight of one of the mesh assets used for mesh-based particles visualization. Can be any value; not normalized.
	/// \param index mesh asset internal index
	virtual physx::PxU32						getMeshAssetWeight(physx::PxU32 index = 0) const = 0;

	///get the list of spawn modifiers
	virtual const NxModifier*					getSpawnModifiers(physx::PxU32& outCount) const = 0;
	///get the list of continuous modifiers
	virtual const NxModifier*					getContinuousModifiers(physx::PxU32& outCount) const = 0;

	///get the biggest possible scale given the current spawn- and continuous modifiers
	///note that some modifiers depend on velocity, so the scale can get arbitrarily large.
	/// \param maxVelocity this value defines what the highest expected velocity is to compute the upper bound
	virtual physx::PxF32						getScaleUpperBound(physx::PxF32 maxVelocity) const = 0;

	///the IOFX asset needs to inform other actors when it is released
	/// \note only for internal use
	virtual void								addDependentActor(ApexActor* actor) = 0;
};


class NxIofxAssetAuthoring : public NxApexAssetAuthoring
{
};

PX_POP_PACK

}
} // namespace physx::apex

#endif // NX_IOFX_ASSET_H
