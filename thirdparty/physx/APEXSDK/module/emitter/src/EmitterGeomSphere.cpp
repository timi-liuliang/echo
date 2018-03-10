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
#include "EmitterGeomSphere.h"
//#include "ApexSharedSerialization.h"
#include "NxApexRenderDebug.h"
#include "NiApexRenderDebug.h"
#include "ApexPreview.h"
#include "EmitterGeomSphereParams.h"

namespace physx
{
namespace apex
{
namespace emitter
{



EmitterGeomSphere::EmitterGeomSphere(NxParameterized::Interface* params)
{
	NxParameterized::Handle eh(*params);
	const NxParameterized::Definition* paramDef;
	const char* enumStr = 0;

	mGeomParams = (EmitterGeomSphereParams*)params;
	mRadius = &(mGeomParams->parameters().radius);
	mHemisphere = &(mGeomParams->parameters().hemisphere);

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

NxEmitterGeom* EmitterGeomSphere::getNxEmitterGeom()
{
	return this;
}


#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomSphere::visualize(const physx::PxMat34Legacy& , NiApexRenderDebug&)
{
}
#else
void EmitterGeomSphere::visualize(const physx::PxMat34Legacy& pose, NiApexRenderDebug& renderDebug)
{
	const physx::PxF32 radius = *mRadius;
	const physx::PxF32 radiusSquared = radius * radius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	renderDebug.pushRenderState();
	renderDebug.setPose(pose.toPxTransform());
	renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::DarkGreen));
	renderDebug.debugSphere(physx::PxVec3(0.0f), *mRadius);
	if(hemisphere > 0.0f) 
	{
		renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::DarkPurple));
		physx::PxMat44 circlePose = physx::PxMat44::createIdentity();
		circlePose.setPosition(physx::PxVec3(0.0f, sphereCapBaseHeight, 0.0f));
		renderDebug.debugOrientedCircle(sphereCapBaseRadius, 3, circlePose);
		renderDebug.debugLine(circlePose.getPosition(), circlePose.getPosition() + physx::PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		for(physx::PxF32 t = 0.0f; t < 2 * physx::PxPi; t += physx::PxPi / 3)
		{
			physx::PxVec3 offset(physx::PxSin(t) * sphereCapBaseRadius, 0.0f, physx::PxCos(t) * sphereCapBaseRadius);
			renderDebug.debugLine(circlePose.getPosition() + offset, circlePose.getPosition() + physx::PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		}
	}
	renderDebug.popRenderState();
}
#endif

#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomSphere::drawPreview(physx::PxF32 , NxApexRenderDebug*) const
{
}
#else
void EmitterGeomSphere::drawPreview(physx::PxF32 scale, NxApexRenderDebug* renderDebug) const
{
	const physx::PxF32 radius = *mRadius;
	const physx::PxF32 radiusSquared = radius * radius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	renderDebug->pushRenderState();
	renderDebug->setCurrentColor(renderDebug->getDebugColor(physx::DebugColors::DarkGreen),
	                             renderDebug->getDebugColor(physx::DebugColors::DarkGreen));
	renderDebug->debugSphere(physx::PxVec3(0.0f), *mRadius * scale);
	if(hemisphere > 0.0f)
	{
		renderDebug->setCurrentColor(renderDebug->getDebugColor(physx::DebugColors::DarkPurple));
		physx::PxMat44 circlePose = physx::PxMat44::createIdentity();
		circlePose.setPosition(physx::PxVec3(0.0f, sphereCapBaseHeight, 0.0f));
		renderDebug->debugOrientedCircle(sphereCapBaseRadius, 3, circlePose);
		renderDebug->debugLine(circlePose.getPosition(), circlePose.getPosition() + physx::PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		for(physx::PxF32 t = 0.0f; t < 2 * physx::PxPi; t += physx::PxPi / 3)
		{
			physx::PxVec3 offset(physx::PxSin(t) * sphereCapBaseRadius, 0.0f, physx::PxCos(t) * sphereCapBaseRadius);
			renderDebug->debugLine(circlePose.getPosition() + offset, circlePose.getPosition() + physx::PxVec3(0.0f, radius - sphereCapBaseHeight, 0.0f));
		}
	}
	renderDebug->popRenderState();
}
#endif

void EmitterGeomSphere::setEmitterType(NxApexEmitterType::Enum t)
{
	mType = t;

	NxParameterized::Handle eh(*mGeomParams);
	const NxParameterized::Definition* paramDef;

	//error check
	mGeomParams->getParameterHandle("emitterType", eh);
	paramDef = eh.parameterDefinition();

	mGeomParams->setParamEnum(eh, paramDef->enumVal((int)mType));
}

physx::PxF32 EmitterGeomSphere::computeEmitterVolume() const
{
	PX_ASSERT(*mHemisphere >= 0.0f);
	PX_ASSERT(*mHemisphere <= 1.0f);
	physx::PxF32 radius = *mRadius;
	physx::PxF32 hemisphere = 2 * radius * (*mHemisphere);
	bool moreThanHalf = true;
	if (hemisphere > radius)
	{
		hemisphere -= radius;
		moreThanHalf = false;
	}
	const physx::PxF32 halfSphereVolume = 2.0f / 3.0f * PxPi * radius * radius * radius;
	const physx::PxF32 sphereCapVolume = 1.0f / 3.0f * PxPi * hemisphere * hemisphere * (3 * radius - hemisphere);
	if (moreThanHalf)
	{
		return halfSphereVolume + sphereCapVolume;
	}
	else
	{
		return sphereCapVolume;
	}
}

physx::PxVec3 EmitterGeomSphere::randomPosInFullVolume(const physx::PxMat34Legacy& pose, QDSRand& rand) const
{
	PX_ASSERT(*mHemisphere >= 0.0f);
	PX_ASSERT(*mHemisphere <= 1.0f);

	const physx::PxF32 radius = *mRadius;
	const physx::PxF32 radiusSquared = radius * radius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	const physx::PxF32 horizontalExtents = hemisphere < 0.5f ? radius : sphereCapBaseRadius;
	/* bounding box for a sphere cap */
	const physx::PxBounds3 boundingBox(PxVec3(-horizontalExtents, sphereCapBaseHeight, -horizontalExtents),
									   PxVec3(horizontalExtents, radius, horizontalExtents));

	physx::PxVec3 pos;
	do
	{
		pos = rand.getScaled(boundingBox.minimum, boundingBox.maximum);
	}
	while (pos.magnitudeSquared() > radiusSquared);

	return pose * pos;
}


bool EmitterGeomSphere::isInEmitter(const physx::PxVec3& pos, const physx::PxMat34Legacy& pose) const
{
	const physx::PxVec3 localPos = pose % pos;
	const physx::PxF32 radius = *mRadius;
	const physx::PxF32 radiusSquared = radius * radius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	const physx::PxF32 horizontalExtents = hemisphere < 0.5f ? radius : sphereCapBaseRadius;
	/* bounding box for a sphere cap */
	const physx::PxBounds3 boundingBox(PxVec3(-horizontalExtents, sphereCapBaseHeight, -horizontalExtents),
	                                   PxVec3(horizontalExtents, radius, horizontalExtents));

	bool isInSphere = localPos.magnitudeSquared() <= radiusSquared;
	bool isInBoundingBox = boundingBox.contains(localPos);

	return isInSphere & isInBoundingBox;
}


void EmitterGeomSphere::computeFillPositions(physx::Array<physx::PxVec3>& positions,
        physx::Array<physx::PxVec3>& velocities,
        const physx::PxMat34Legacy& pose,
        physx::PxF32 objRadius,
        physx::PxBounds3& outBounds,
        QDSRand&) const
{
	// we're not doing anything with the velocities array
	PX_UNUSED(velocities);
	const physx::PxF32 radius = *mRadius;
	const physx::PxF32 radiusSquared = radius * radius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	const physx::PxF32 horizontalExtents = hemisphere < 0.5f ? radius : sphereCapBaseRadius;

	physx::PxU32 numX = (physx::PxU32)PxFloor(horizontalExtents / objRadius);
	numX -= numX % 2;
	physx::PxU32 numY = (physx::PxU32)PxFloor((radius - sphereCapBaseHeight) / objRadius);
	numY -= numY % 2;
	physx::PxU32 numZ = (physx::PxU32)PxFloor(horizontalExtents / objRadius);
	numZ -= numZ % 2;

	const physx::PxF32 radiusMinusObjRadius = radius - objRadius;
	const physx::PxF32 radiusMinusObjRadiusSquared = radiusMinusObjRadius * radiusMinusObjRadius;
	for (physx::PxF32 x = -(numX * objRadius); x <= radiusMinusObjRadius; x += 2 * objRadius)
	{
		for (physx::PxF32 y = sphereCapBaseHeight; y <= radiusMinusObjRadius; y += 2 * objRadius)
		{
			for (physx::PxF32 z = -(numZ * objRadius); z <= radiusMinusObjRadius; z += 2 * objRadius)
			{
				const physx::PxVec3 p(x, y, z);
				if (p.magnitudeSquared() < radiusMinusObjRadiusSquared)
				{
					positions.pushBack(pose * physx::PxVec3(x, y, z));
					outBounds.include(positions.back());
				}
			}
		}
	}
}

}
}
} // namespace physx::apex
