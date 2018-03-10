/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_GEOM_H__
#define __EMITTER_GEOM_H__

#include "NxApex.h"
#include "NxEmitterGeoms.h"
#include "PsArray.h"
#include "PsUserAllocated.h"
#include <PsShare.h>
#include "PxMat34Legacy.h"
#include "ApexRand.h"

namespace physx
{
namespace apex
{

class NxApexRenderDebug;
class NiApexRenderDebug;

namespace emitter
{

/* Implementation base class for all NxEmitterGeom derivations */

class EmitterGeom : public physx::UserAllocated
{
public:
	/* Asset callable functions */
	virtual NxEmitterGeom*				getNxEmitterGeom() = 0;
	virtual void						destroy() = 0;

	/* ApexEmitterActor runtime access methods */
	virtual physx::PxF32				computeEmitterVolume() const = 0;
	virtual void						computeFillPositions(physx::Array<physx::PxVec3>& positions,
	        physx::Array<physx::PxVec3>& velocities,
	        const physx::PxMat34Legacy&,
	        physx::PxF32,
	        physx::PxBounds3& outBounds,
	        QDSRand& rand) const = 0;

	virtual physx::PxVec3				randomPosInFullVolume(const physx::PxMat34Legacy&, QDSRand&) const = 0;

	/* AssetPreview methods */
	virtual void                        drawPreview(physx::PxF32 scale, NxApexRenderDebug* renderDebug) const = 0;

	/* Optional override functions */
	virtual void						visualize(const physx::PxMat34Legacy&, NiApexRenderDebug&) { }

	virtual physx::PxF32				computeNewlyCoveredVolume(const physx::PxMat34Legacy&, const physx::PxMat34Legacy&, QDSRand&) const;
	virtual physx::PxVec3				randomPosInNewlyCoveredVolume(const physx::PxMat34Legacy&, const physx::PxMat34Legacy&, QDSRand&) const;

protected:
	virtual bool						isInEmitter(const physx::PxVec3& pos, const physx::PxMat34Legacy& pose) const = 0;
};

}
}
} // end namespace physx::apex

#endif
