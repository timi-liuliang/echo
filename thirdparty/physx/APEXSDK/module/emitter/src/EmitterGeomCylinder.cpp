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
#include "EmitterGeomCylinder.h"
//#include "ApexSharedSerialization.h"
#include "NxApexRenderDebug.h"
#include "NiApexRenderDebug.h"
#include "ApexPreview.h"
#include "EmitterGeomCylinderParams.h"

namespace physx
{
namespace apex
{
namespace emitter
{



EmitterGeomCylinder::EmitterGeomCylinder(NxParameterized::Interface* params)
{
	NxParameterized::Handle eh(*params);
	const NxParameterized::Definition* paramDef;
	const char* enumStr = 0;

	mGeomParams = (EmitterGeomCylinderParams*)params;
	mRadius = &(mGeomParams->parameters().radius);
	mHeight = &(mGeomParams->parameters().height);

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

NxEmitterGeom* EmitterGeomCylinder::getNxEmitterGeom()
{
	return this;
}


#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomCylinder::visualize(const physx::PxMat34Legacy& , NiApexRenderDebug&)
{
}
#else
void EmitterGeomCylinder::visualize(const physx::PxMat34Legacy& pose, NiApexRenderDebug& renderDebug)
{
	physx::PxVec3 p0, p1;
	p0 = physx::PxVec3(0.0f, -*mHeight / 2.0f, 0.0f);
	p1 = physx::PxVec3(0.0f, *mHeight / 2.0f, 0.0f);

	pose.multiply(p0, p0);
	pose.multiply(p1, p1);

	renderDebug.pushRenderState();
	renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::DarkGreen));

	renderDebug.debugCylinder(p0, p1, *mRadius);
	renderDebug.popRenderState();
}
#endif

#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomCylinder::drawPreview(physx::PxF32 , NxApexRenderDebug*) const
{
}
#else
void EmitterGeomCylinder::drawPreview(physx::PxF32 scale, NxApexRenderDebug* renderDebug) const
{
	physx::PxVec3 p0, p1;
	// where should we put this thing???
	p0 = physx::PxVec3(0.0f, -*mHeight / 2.0f, 0.0f);
	p1 = physx::PxVec3(0.0f, *mHeight / 2.0f, 0.0f);

	renderDebug->pushRenderState();
	renderDebug->setCurrentColor(renderDebug->getDebugColor(physx::DebugColors::DarkGreen),
	                             renderDebug->getDebugColor(physx::DebugColors::DarkGreen));

	renderDebug->debugCylinder(p0, p1, *mRadius * scale);
	renderDebug->popRenderState();
}
#endif

void EmitterGeomCylinder::setEmitterType(NxApexEmitterType::Enum t)
{
	mType = t;

	NxParameterized::Handle eh(*mGeomParams);
	const NxParameterized::Definition* paramDef;

	//error check
	mGeomParams->getParameterHandle("emitterType", eh);
	paramDef = eh.parameterDefinition();

	mGeomParams->setParamEnum(eh, paramDef->enumVal((int)mType));
}

physx::PxF32 EmitterGeomCylinder::computeEmitterVolume() const
{
	return (*mHeight) * (*mRadius) * (*mRadius) * physx::PxPi;
}


physx::PxVec3 EmitterGeomCylinder::randomPosInFullVolume(const physx::PxMat34Legacy& pose, QDSRand& rand) const
{
	physx::PxVec3 pos;

	physx::PxF32 u, v, w;
	do
	{
		u = rand.getNext();
		w = rand.getNext();
	}
	while (u * u + w * w > 1.0f);

	v = *mHeight / 2 * rand.getNext();

	pos = physx::PxVec3(u * (*mRadius), v, w * (*mRadius));

#if _DEBUG
	physx::PxMat34Legacy tmpPose;
	tmpPose.id();
	PX_ASSERT(isInEmitter(pos, tmpPose));
#endif

	return pose * pos;
}


bool EmitterGeomCylinder::isInEmitter(const physx::PxVec3& pos, const physx::PxMat34Legacy& pose) const
{
	physx::PxVec3 localPos = pose % pos;

	if (localPos.x < -*mRadius)
	{
		return false;
	}
	if (localPos.x > *mRadius)
	{
		return false;
	}
	if (localPos.y < -*mHeight / 2.0f)
	{
		return false;
	}
	if (localPos.y > *mHeight / 2.0f)
	{
		return false;
	}
	if (localPos.z < -*mRadius)
	{
		return false;
	}
	if (localPos.z > *mRadius)
	{
		return false;
	}

	return true;
}


void EmitterGeomCylinder::computeFillPositions(physx::Array<physx::PxVec3>& positions,
        physx::Array<physx::PxVec3>& velocities,
        const physx::PxMat34Legacy& pose,
        physx::PxF32 objRadius,
        physx::PxBounds3& outBounds,
        QDSRand&) const
{
	// we're not doing anything with the velocities array
	PX_UNUSED(velocities);

	PxF32 halfHeight = *mHeight / 2;

	physx::PxU32 numX = (physx::PxU32)PxFloor(*mRadius / objRadius);
	numX -= numX % 2;
	physx::PxU32 numY = (physx::PxU32)PxFloor(halfHeight / objRadius);
	numY -= numY % 2;
	physx::PxU32 numZ = (physx::PxU32)PxFloor(*mRadius / objRadius);
	numZ -= numZ % 2;

	for (physx::PxF32 x = -(numX * objRadius); x <= *mRadius - objRadius; x += 2 * objRadius)
	{
		for (physx::PxF32 y = -(numY * objRadius); y <= halfHeight - objRadius; y += 2 * objRadius)
		{
			for (physx::PxF32 z = -(numZ * objRadius); z <= *mRadius - objRadius; z += 2 * objRadius)
			{
				if (x * x + z * z < (*mRadius - objRadius) * (*mRadius - objRadius))
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
