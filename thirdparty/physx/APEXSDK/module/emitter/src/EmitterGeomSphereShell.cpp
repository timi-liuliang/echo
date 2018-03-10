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
#include "EmitterGeomSphereShell.h"
//#include "ApexSharedSerialization.h"
#include "NxApexRenderDebug.h"
#include "NiApexRenderDebug.h"
#include "ApexPreview.h"
#include "EmitterGeomSphereShellParams.h"

namespace physx
{
namespace apex
{
namespace emitter
{



EmitterGeomSphereShell::EmitterGeomSphereShell(NxParameterized::Interface* params)
{
	NxParameterized::Handle eh(*params);
	const NxParameterized::Definition* paramDef;
	const char* enumStr = 0;

	mGeomParams = (EmitterGeomSphereShellParams*)params;
	mRadius = &(mGeomParams->parameters().radius);
	mShellThickness = &(mGeomParams->parameters().shellThickness);
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

NxEmitterGeom* EmitterGeomSphereShell::getNxEmitterGeom()
{
	return this;
}


#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomSphereShell::visualize(const physx::PxMat34Legacy& , NiApexRenderDebug&)
{
}
#else
void EmitterGeomSphereShell::visualize(const physx::PxMat34Legacy& pose, NiApexRenderDebug& renderDebug)
{
	renderDebug.pushRenderState();

	renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::DarkGreen));

	// outer sphere
	renderDebug.setPose(pose.toPxTransform());
	renderDebug.debugSphere(physx::PxVec3(0.0f), *mRadius);

	// intter sphere
	renderDebug.debugSphere(physx::PxVec3(0.0f), *mRadius + *mShellThickness);

	const physx::PxF32 radius = *mRadius + *mShellThickness;
	const physx::PxF32 radiusSquared = radius * radius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	// cone depicting the hemisphere
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
void EmitterGeomSphereShell::drawPreview(physx::PxF32 , NxApexRenderDebug*) const
{
}
#else
void EmitterGeomSphereShell::drawPreview(physx::PxF32 scale, NxApexRenderDebug* renderDebug) const
{
	renderDebug->pushRenderState();
	renderDebug->setCurrentColor(renderDebug->getDebugColor(physx::DebugColors::Yellow),
	                             renderDebug->getDebugColor(physx::DebugColors::Yellow));

	renderDebug->debugSphere(physx::PxVec3(0.0f), *mRadius * scale);

	renderDebug->setCurrentColor(renderDebug->getDebugColor(physx::DebugColors::DarkGreen),
	                             renderDebug->getDebugColor(physx::DebugColors::DarkGreen));

	renderDebug->debugSphere(physx::PxVec3(0.0f), (*mRadius + *mShellThickness) * scale);

	const physx::PxF32 radius = *mRadius + *mShellThickness;
	const physx::PxF32 radiusSquared = radius * radius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -radius + 2 * radius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
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

void EmitterGeomSphereShell::setEmitterType(NxApexEmitterType::Enum t)
{
	mType = t;

	NxParameterized::Handle eh(*mGeomParams);
	const NxParameterized::Definition* paramDef;

	//error check
	mGeomParams->getParameterHandle("emitterType", eh);
	paramDef = eh.parameterDefinition();

	mGeomParams->setParamEnum(eh, paramDef->enumVal((int)mType));
}

physx::PxF32 EmitterGeomSphereShell::computeEmitterVolume() const
{
	const physx::PxF32 radius = *mRadius;
	const physx::PxF32 bigRadius = *mRadius + *mShellThickness;
	physx::PxF32 hemisphere = 2 * radius * (*mHemisphere);
	physx::PxF32 bigHemisphere = 2 * bigRadius * (*mHemisphere);

	bool moreThanHalf = true;

	if (hemisphere > radius)
	{
		hemisphere -= radius;
		bigHemisphere -= bigRadius;
		moreThanHalf = false;
	}

	const physx::PxF32 volumeBigSphere = 4.0f / 3.0f * PxPi * bigRadius * bigRadius * bigRadius;
	const physx::PxF32 volumeSmallSphere = 4.0f / 3.0f * PxPi * *mRadius * *mRadius * *mRadius;
	const physx::PxF32 halfSphereShellVolume = (volumeBigSphere - volumeSmallSphere) / 2.0f;

	const physx::PxF32 bigCapVolume = 1.0f / 3.0f * PxPi * bigHemisphere * bigHemisphere * (3 * bigRadius - bigHemisphere);
	const physx::PxF32 smallCapVolume = 1.0f / 3.0f * PxPi * hemisphere * hemisphere * (3 * radius - hemisphere);

	const physx::PxF32 sphereShellCapVolume = bigCapVolume - smallCapVolume;

	if (moreThanHalf)
	{
		return halfSphereShellVolume + sphereShellCapVolume;
	}
	else
	{
		return sphereShellCapVolume;
	}
}


physx::PxVec3 EmitterGeomSphereShell::randomPosInFullVolume(const physx::PxMat34Legacy& pose, QDSRand& rand) const
{
	physx::PxF32 hemisphere = 2.0f * *mHemisphere - 1.0f;

	bool moreThanHalf = true;

	if (*mHemisphere > 0.5f)
	{
		moreThanHalf = false;
	}

	// There are two cases here - 1-st for hemisphere cut above the center of the sphere
	// and 2-nd for hemisphere cut below the center of the sphere.
	// The reason for this is that in case very high hemisphere cut is set, so the area
	// of the actual emitter is very small in compare to the whole sphere emitter, it would take too
	// much time [on average] to generate suitable point using randomPointOnUnitSphere
	// function, so in this case it is more efficient to use another method.
	// in case we have at least half of the sphere shell present the randomPointOnUnitSphere should
	// be sufficient.
	physx::PxVec3 pos;
	if(!moreThanHalf)
	{
		// 1-st case :
		// * generate random unit vector within a cone
		// * clamp to big radius
		const physx::PxF32 sphereCapBaseHeight = -1.0f + 2 * (*mHemisphere);
		const physx::PxF32 phi = rand.getScaled(0.0f, physx::PxTwoPi);
		const physx::PxF32 cos_theta = sphereCapBaseHeight;
		const physx::PxF32 z = rand.getScaled(cos_theta, 1.0f);
		const physx::PxF32 oneMinusZSquared = physx::PxSqrt(1.0f - z * z);
		pos = physx::PxVec3(oneMinusZSquared * physx::PxCos(phi), z, oneMinusZSquared * physx::PxSin(phi));
	}
	else
	{
		// 2-nd case :
		// * get random pos on unit sphere, until its height is above hemisphere cut
		do
		{
			pos = randomPointOnUnitSphere(rand);
		} while(pos.y < hemisphere);
	}

	// * add negative offset withing the thickness
	// * solve edge case [for the 1-st case] - regenerate offset from the previous step
	// in case point is below hemisphere cut	

	physx::PxVec3 tmp;
	const physx::PxF32 sphereCapBaseHeight = -(*mRadius + *mShellThickness) + 2 * (*mRadius + *mShellThickness) * (*mHemisphere);
	do
	{
		physx::PxF32 thickness = rand.getScaled(0, *mShellThickness);
		tmp = pos * (*mRadius + *mShellThickness - thickness);
	} while(tmp.y < sphereCapBaseHeight);

	pos = tmp;
	pos += pose.t;

	return pos;
}


bool EmitterGeomSphereShell::isInEmitter(const physx::PxVec3& pos, const physx::PxMat34Legacy& pose) const
{
	physx::PxVec3 localPos = pose % pos;
	const physx::PxF32 sphereCapBaseHeight = -(*mRadius + *mShellThickness) + 2 * (*mRadius + *mShellThickness) * (*mHemisphere);
	physx::PxF32 d2 = localPos.x * localPos.x + localPos.y * localPos.y + localPos.z * localPos.z;
	bool isInBigSphere = d2 < (*mRadius + *mShellThickness) * (*mRadius + *mShellThickness);
	bool isInSmallSphere = d2 < *mRadius * *mRadius;
	bool higherThanHemisphereCut = pos.y > sphereCapBaseHeight;
	return isInBigSphere && !isInSmallSphere && higherThanHemisphereCut;
}


void EmitterGeomSphereShell::computeFillPositions(physx::Array<physx::PxVec3>& positions,
        physx::Array<physx::PxVec3>& velocities,
        const physx::PxMat34Legacy& pose,
        physx::PxF32 objRadius,
        physx::PxBounds3& outBounds,
        QDSRand&) const
{
	const physx::PxF32 bigRadius = *mRadius + *mShellThickness;
	const physx::PxF32 radiusSquared = bigRadius * bigRadius;
	const physx::PxF32 hemisphere = *mHemisphere;
	const physx::PxF32 sphereCapBaseHeight = -bigRadius + 2 * bigRadius * hemisphere;
	const physx::PxF32 sphereCapBaseRadius = physx::PxSqrt(radiusSquared - sphereCapBaseHeight * sphereCapBaseHeight);
	const physx::PxF32 horizontalExtents = hemisphere < 0.5f ? bigRadius : sphereCapBaseRadius;

	// we're not doing anything with the velocities array
	PX_UNUSED(velocities);

	// we don't want anything outside the emitter
	physx::PxU32 numX = (physx::PxU32)PxFloor(horizontalExtents / objRadius);
	numX -= numX % 2;
	physx::PxU32 numY = (physx::PxU32)PxFloor((bigRadius - sphereCapBaseHeight) / objRadius);
	numY -= numY % 2;
	physx::PxU32 numZ = (physx::PxU32)PxFloor(horizontalExtents / objRadius);
	numZ -= numZ % 2;

	for (physx::PxF32 x = -(numX * objRadius); x <= bigRadius - objRadius; x += 2 * objRadius)
	{
		for (physx::PxF32 y = -(numY * objRadius); y <= bigRadius - objRadius; y += 2 * objRadius)
		{
			for (physx::PxF32 z = -(numZ * objRadius); z <= bigRadius - objRadius; z += 2 * objRadius)
			{
				const physx::PxF32 magnitudeSquare = physx::PxVec3(x, y, z).magnitudeSquared();
				if ((magnitudeSquare > (*mRadius + objRadius) * (*mRadius + objRadius)) &&
				        (magnitudeSquare < (bigRadius - objRadius) * (bigRadius - objRadius)))
				{
					positions.pushBack(pose * physx::PxVec3(x, y, z));
					outBounds.include(positions.back());
				}
			}
		}
	}
}


physx::PxVec3 EmitterGeomSphereShell::randomPointOnUnitSphere(QDSRand& rand) const
{
	// uniform distribution on the sphere around pos (Cook, Marsaglia Method. TODO: is other method cheaper?)
	physx::PxF32 x0, x1, x2, x3, div;
	do
	{
		x0 = rand.getNext();
		x1 = rand.getNext();
		x2 = rand.getNext();
		x3 = rand.getNext();
		div = x0 * x0 + x1 * x1 + x2 * x2 + x3 * x3;
	}
	while (div >= 1.0f);

	// coordinates on unit sphere
	physx::PxF32 x = 2 * (x1 * x3 + x0 * x2) / div;
	physx::PxF32 y = 2 * (x2 * x3 - x0 * x1) / div;
	physx::PxF32 z = (x0 * x0 + x3 * x3 - x1 * x1 - x2 * x2) / div;

	return physx::PxVec3(x, y, z);
}

}
}
} // namespace physx::apex
