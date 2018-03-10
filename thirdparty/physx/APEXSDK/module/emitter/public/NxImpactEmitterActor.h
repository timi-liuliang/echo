/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_IMPACT_EMITTER_ACTOR_H
#define NX_IMPACT_EMITTER_ACTOR_H

#include "NxApex.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

class NxImpactEmitterAsset;
class NxApexRenderVolume;

///Impact emitter actor.  Emits particles at impact places
class NxImpactEmitterActor : public NxApexActor
{
protected:
	virtual ~NxImpactEmitterActor() {}

public:
	///Gets the pointer to the underlying asset
	virtual NxImpactEmitterAsset* 	getEmitterAsset() const = 0;

	/**
	\brief Registers an impact in the queue

	\param hitPos impact position
	\param hitDir impact direction
	\param surfNorm normal of the surface that is hit by the impact
	\param setID - id for the event set which should be spawned. Specifies the behavior. \sa NxImpactEmitterAsset::querySetID

	*/
	virtual void registerImpact(const physx::PxVec3& hitPos, const physx::PxVec3& hitDir, const physx::PxVec3& surfNorm, physx::PxU32 setID) = 0;

	///Emitted particles are injected to specified render volume on initial frame.
	///Set to NULL to clear the preferred volume.
	virtual void setPreferredRenderVolume(NxApexRenderVolume* volume) = 0;
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_IMPACT_EMITTER_ACTOR_H
