/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_GEOM_SPHERE_H__
#define __EMITTER_GEOM_SPHERE_H__

#include "EmitterGeom.h"
#include "PsUserAllocated.h"
#include "EmitterGeomSphereParams.h"

namespace NxParameterized
{
class Interface;
};

namespace physx
{
namespace apex
{
namespace emitter
{

class EmitterGeomSphere : public NxEmitterSphereGeom, public EmitterGeom
{
public:
	EmitterGeomSphere(NxParameterized::Interface* params);

	/* Asset callable methods */
	NxEmitterGeom*				getNxEmitterGeom();
	const NxEmitterSphereGeom* 	isSphereGeom() const
	{
		return this;
	}
	NxApexEmitterType::Enum		getEmitterType() const
	{
		return mType;
	}
	void						setEmitterType(NxApexEmitterType::Enum t);
	void	                    setRadius(physx::PxF32 radius)
	{
		*mRadius = radius;
	}
	physx::PxF32	                    getRadius() const
	{
		return *mRadius;
	}
	void	                    setHemisphere(physx::PxF32 hemisphere)
	{
		*mHemisphere = hemisphere;
	}
	physx::PxF32	                    getHemisphere() const
	{
		return *mHemisphere;
	}
	void						destroy()
	{
		delete this;
	}

	/* AssetPreview methods */
	void                        drawPreview(physx::PxF32 scale, NxApexRenderDebug* renderDebug) const;

	/* Actor callable methods */
	void						visualize(const physx::PxMat34Legacy& pose, NiApexRenderDebug& renderDebug);

	void						computeFillPositions(physx::Array<physx::PxVec3>& positions,
	        physx::Array<physx::PxVec3>& velocities,
	        const physx::PxMat34Legacy&,
	        physx::PxF32,
	        physx::PxBounds3& outBounds,
	        QDSRand& rand) const;

	physx::PxF32				computeEmitterVolume() const;
	physx::PxVec3				randomPosInFullVolume(const physx::PxMat34Legacy& pose, QDSRand& rand) const;
	bool						isInEmitter(const physx::PxVec3& pos, const physx::PxMat34Legacy& pose) const;

protected:
	NxApexEmitterType::Enum		mType;
	physx::PxF32*				mRadius;
	physx::PxF32*				mHemisphere;
	EmitterGeomSphereParams*	mGeomParams;
};

}
}
} // end namespace physx::apex

#endif
