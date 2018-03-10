/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "PsShare.h"
#include "NxApexEmitterAsset.h"
#include "EmitterGeomBox.h"
//#include "ApexSharedSerialization.h"
#include "NxApexRenderDebug.h"
#include "NiApexRenderDebug.h"
#include "ApexPreview.h"
#include "EmitterGeomBoxParams.h"

namespace physx
{
namespace apex
{
namespace emitter
{



EmitterGeomBox::EmitterGeomBox(NxParameterized::Interface* params)
{
	NxParameterized::Handle eh(*params);
	const NxParameterized::Definition* paramDef;
	const char* enumStr = 0;

	mGeomParams = (EmitterGeomBoxParams*)params;
	mExtents = (physx::PxVec3*)(&(mGeomParams->parameters().extents));

	//error check
	mGeomParams->getParameterHandle("emitterType", eh);
	mGeomParams->getParamEnum(eh, enumStr);
	paramDef = eh.parameterDefinition();

	mType = NxApexEmitterType::NX_ET_RATE;
	for (int i = 0; i < paramDef->numEnumVals(); ++i)
	{
		if (!strcmp(paramDef->enumVal(i), enumStr))
		{
			mType = (NxApexEmitterType::Enum)i;
			break;
		}
	}
}

NxEmitterGeom* EmitterGeomBox::getNxEmitterGeom()
{
	return this;
}

#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomBox::visualize(const physx::PxMat34Legacy& , NiApexRenderDebug&)
{
}
#else
void EmitterGeomBox::visualize(const physx::PxMat34Legacy& pose, NiApexRenderDebug& renderDebug)
{
	renderDebug.pushRenderState();
	renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::DarkGreen));
	renderDebug.debugOrientedBound(2.0f * *mExtents, pose);
	renderDebug.popRenderState();
}
#endif

#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomBox::drawPreview(physx::PxF32 , NxApexRenderDebug*) const
{
}
#else
void EmitterGeomBox::drawPreview(physx::PxF32 scale, NxApexRenderDebug* renderDebug) const
{
	renderDebug->pushRenderState();
	renderDebug->setCurrentColor(renderDebug->getDebugColor(physx::DebugColors::DarkGreen),
	                             renderDebug->getDebugColor(physx::DebugColors::DarkGreen));
	renderDebug->debugBound(-(*mExtents) * scale, *mExtents * scale);
	renderDebug->popRenderState();
}
#endif


void EmitterGeomBox::setEmitterType(NxApexEmitterType::Enum t)
{
	mType = t;

	NxParameterized::Handle eh(*mGeomParams);
	const NxParameterized::Definition* paramDef;

	//error check
	mGeomParams->getParameterHandle("emitterType", eh);
	paramDef = eh.parameterDefinition();

	mGeomParams->setParamEnum(eh, paramDef->enumVal((int)mType));
}

/* ApexEmitterActor callable methods */


physx::PxF32 EmitterGeomBox::computeEmitterVolume() const
{
	return 8.0f * mExtents->x * mExtents->y * mExtents->z;
}


/* Return percentage of new volume not covered by old volume */
physx::PxF32 EmitterGeomBox::computeNewlyCoveredVolume(
    const physx::PxMat34Legacy& oldPose,
    const physx::PxMat34Legacy& newPose,
    QDSRand& rand) const
{
	// estimate by sampling
	physx::PxU32 numSamples = 100;
	physx::PxU32 numOutsideOldVolume = 0;
	for (physx::PxU32 i = 0; i < numSamples; i++)
	{
		if (!isInEmitter(randomPosInFullVolume(newPose, rand), oldPose))
		{
			numOutsideOldVolume++;
		}
	}

	return (physx::PxF32) numOutsideOldVolume / numSamples;
}


void EmitterGeomBox::computeFillPositions(physx::Array<physx::PxVec3>& positions,
        physx::Array<physx::PxVec3>& velocities,
        const physx::PxMat34Legacy& pose,
        physx::PxF32 objRadius,
        physx::PxBounds3& outBounds,
        QDSRand&) const
{
	// we're not doing anything with the velocities array
	PX_UNUSED(velocities);

	// we don't want anything outside the emitter
	physx::PxU32 numX = (physx::PxU32)PxFloor(mExtents->x / objRadius);
	numX -= numX % 2;
	physx::PxU32 numY = (physx::PxU32)PxFloor(mExtents->y / objRadius);
	numY -= numY % 2;
	physx::PxU32 numZ = (physx::PxU32)PxFloor(mExtents->z / objRadius);
	numZ -= numZ % 2;

	for (physx::PxF32 x = -(numX * objRadius); x <= mExtents->x - objRadius; x += 2 * objRadius)
	{
		for (physx::PxF32 y = -(numY * objRadius); y <= mExtents->y - objRadius; y += 2 * objRadius)
		{
			for (physx::PxF32 z = -(numZ * objRadius); z <= mExtents->z - objRadius; z += 2 * objRadius)
			{
				positions.pushBack(pose * physx::PxVec3(x, y, z));
				outBounds.include(positions.back());
			}
		}
	}
}

/* internal methods */

physx::PxVec3 EmitterGeomBox::randomPosInFullVolume(const physx::PxMat34Legacy& pose, QDSRand& rand) const
{
	physx::PxF32 u =  rand.getScaled(-mExtents->x, mExtents->x);
	physx::PxF32 v =  rand.getScaled(-mExtents->y, mExtents->y);
	physx::PxF32 w =  rand.getScaled(-mExtents->z, mExtents->z);

	physx::PxVec3 pos(u, v, w);
	return pose * pos;
}

bool EmitterGeomBox::isInEmitter(const physx::PxVec3& pos, const physx::PxMat34Legacy& pose) const
{
	physx::PxVec3 localPos = pose % pos;

	if (localPos.x < -mExtents->x)
	{
		return false;
	}
	if (localPos.x > mExtents->x)
	{
		return false;
	}
	if (localPos.y < -mExtents->y)
	{
		return false;
	}
	if (localPos.y > mExtents->y)
	{
		return false;
	}
	if (localPos.z < -mExtents->z)
	{
		return false;
	}
	if (localPos.z > mExtents->z)
	{
		return false;
	}

	return true;
}

physx::PxVec3 EmitterGeomBox::randomPosInNewlyCoveredVolume(const physx::PxMat34Legacy& pose, const physx::PxMat34Legacy& oldPose, QDSRand& rand) const
{
	// TODO make better, this is very slow when emitter moves slowly
	// SJB: I'd go one further, this seems mildly retarted
	physx::PxVec3 pos;
	do
	{
		pos = randomPosInFullVolume(pose, rand);
	}
	while (isInEmitter(pos, oldPose));
	return pos;
}

}
}
} // namespace physx::apex
