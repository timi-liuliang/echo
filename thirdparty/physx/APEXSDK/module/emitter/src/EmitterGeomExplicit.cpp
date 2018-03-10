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
#include "PsSort.h"
#include "PsMathUtils.h"
#include "PsHashSet.h"
#include "NxApexEmitterAsset.h"
#include "EmitterGeomExplicit.h"
//#include "ApexSharedSerialization.h"
#include "NxApexRenderDebug.h"
#include "NiApexRenderDebug.h"
#include "ApexPreview.h"
#include "EmitterGeomExplicitParams.h"
#include "NxParamArray.h"

namespace physx
{
namespace apex
{
namespace emitter
{

PX_INLINE static void GetNormals(const physx::PxVec3& n, PxVec3& t1, PxVec3& t2)
{
	physx::PxVec3 nabs(physx::PxAbs(n.x), physx::PxAbs(n.y), physx::PxAbs(n.z));

	t1 = nabs.x <= nabs.y && nabs.x <= nabs.z
	     ? physx::PxVec3(0, -n.z, n.y)
	     : nabs.y <= nabs.x && nabs.y <= nabs.z ? physx::PxVec3(-n.z, 0, n.x)
	     : physx::PxVec3(-n.y, n.x, 0);
	t1.normalize();

	t2 = n.cross(t1);
	t2.normalize();
}

EmitterGeomExplicit::EmitterGeomExplicit(NxParameterized::Interface* params)
{
	mGeomParams = (EmitterGeomExplicitParams*)params;

	mDistance = mGeomParams->distance;
	if (mDistance < PX_EPS_F32)
	{
		mInvDistance = PX_MAX_F32;
	}
	else
	{
		mInvDistance = 1 / mDistance;
	}

	// get points
	mPoints.resize((physx::PxU32)mGeomParams->points.positions.arraySizes[0]);
	if (!mPoints.empty())
	{
		PX_COMPILE_TIME_ASSERT(sizeof(PointParams) == sizeof(mGeomParams->points.positions.buf[0]));
		memcpy(&mPoints[0], mGeomParams->points.positions.buf, mPoints.size() * sizeof(PointParams));
	}

	// get point velocities
	for (physx::PxU32 i = 0; i < (physx::PxU32)mGeomParams->points.velocities.arraySizes[0]; i++)
	{
		mVelocities.pushBack(mGeomParams->points.velocities.buf[i]);
	}

	// get spheres
	mSpheres.resize((physx::PxU32)mGeomParams->spheres.positions.arraySizes[0]);
	if (!mSpheres.empty())
	{
		PX_COMPILE_TIME_ASSERT(sizeof(SphereParams) == sizeof(mGeomParams->spheres.positions.buf[0]));
		memcpy(&mSpheres[0], mGeomParams->spheres.positions.buf, mSpheres.size() * sizeof(SphereParams));
	}

	// get sphere velocities
	for (physx::PxI32 i = 0; i < mGeomParams->spheres.velocities.arraySizes[0]; i++)
	{
		mSphereVelocities.pushBack(mGeomParams->spheres.velocities.buf[i]);
	}

	// get ellipsoids
	mEllipsoids.resize((physx::PxU32)mGeomParams->ellipsoids.positions.arraySizes[0]);
	if (!mEllipsoids.empty())
	{
		PX_COMPILE_TIME_ASSERT(sizeof(EllipsoidParams) == sizeof(mGeomParams->ellipsoids.positions.buf[0]));
		memcpy(&mEllipsoids[0], mGeomParams->ellipsoids.positions.buf, mEllipsoids.size() * sizeof(EllipsoidParams));
	}

	// get ellipsoid velocities
	for (physx::PxI32 i = 0; i < mGeomParams->ellipsoids.velocities.arraySizes[0]; i++)
	{
		mEllipsoidVelocities.pushBack(mGeomParams->ellipsoids.velocities.buf[i]);
	}

	updateCollisions();
}

EmitterGeomExplicit::EmitterGeomExplicit() :
	mDistance(0.0f),
	mInvDistance(PX_MAX_F32),
	mGeomParams(NULL)
{}

NxEmitterGeom* EmitterGeomExplicit::getNxEmitterGeom()
{
	return this;
}

#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomExplicit::visualize(const physx::PxMat34Legacy& , NiApexRenderDebug&)
{
}
#else
void EmitterGeomExplicit::visualize(const physx::PxMat34Legacy& pose, NiApexRenderDebug& renderDebug)
{
	renderDebug.pushRenderState();

	renderDebug.setCurrentColor(renderDebug.getDebugColor(physx::DebugColors::DarkGreen));

	renderDebug.setPose(pose);

	physx::PxVec3 pos;
	for (physx::PxU32 i = 0; i < mPoints.size(); ++i)
	{
		renderDebug.debugPoint(pos, 0.01f);
	}

	for (physx::PxU32 i = 0; i < mSpheres.size(); ++i)
	{
		renderDebug.debugSphere(mSpheres[i].center, mSpheres[i].radius);
	}

	for (physx::PxU32 i = 0; i < mEllipsoids.size(); ++i)
	{
		physx::PxVec3 n = mEllipsoids[i].normal, t1, t2;
		GetNormals(n, t1, t2);

		physx::PxMat44 unitSphereToEllipsoid(
		    mEllipsoids[i].radius * t1,
		    mEllipsoids[i].radius * t2,
		    mEllipsoids[i].polarRadius * n,
		    mEllipsoids[i].center
		);

		renderDebug.setPose(pose * unitSphereToEllipsoid);

		renderDebug.debugSphere(physx::PxVec3(0.0f), 1.0f);
	}

	renderDebug.popRenderState();
}
#endif

#ifdef WITHOUT_DEBUG_VISUALIZE
void EmitterGeomExplicit::drawPreview(physx::PxF32 , NxApexRenderDebug*) const
{
}
#else
void EmitterGeomExplicit::drawPreview(physx::PxF32 scale, NxApexRenderDebug* renderDebug) const
{
	renderDebug->pushRenderState();
	renderDebug->setCurrentColor(renderDebug->getDebugColor(physx::DebugColors::DarkGreen),
	                             renderDebug->getDebugColor(physx::DebugColors::DarkGreen));

	physx::PxVec3 pos;
	for (physx::PxU32 i = 0 ; i < mPoints.size() ; i++)
	{
		pos = mPoints[i].position;
		renderDebug->debugPoint(pos, scale);
	}

	for (physx::PxU32 i = 0; i < mSpheres.size(); ++i)
	{
		renderDebug->debugSphere(mSpheres[i].center, mSpheres[i].radius * scale);
	}

	for (physx::PxU32 i = 0; i < mEllipsoids.size(); ++i)
	{
		physx::PxVec3 n = mEllipsoids[i].normal, t1, t2;
		GetNormals(n, t1, t2);

		physx::PxMat44 unitSphereToEllipsoid(
		    mEllipsoids[i].radius * t1,
		    mEllipsoids[i].radius * t2,
		    mEllipsoids[i].polarRadius * n,
		    mEllipsoids[i].center
		);

		renderDebug->setPose(unitSphereToEllipsoid);

		renderDebug->debugSphere(physx::PxVec3(0.0f), scale);
	}

	renderDebug->popRenderState();
}
#endif

/* ApexEmitterActor callable methods */

void EmitterGeomExplicit::updateAssetPoints()
{
	if (!mGeomParams)
	{
		return;
	}

	// just copy the position and velocity lists to mGeomParams

	NxParameterized::Handle h(*mGeomParams);
	NxParameterized::ErrorType pError;

	// Update positions

	pError = mGeomParams->getParameterHandle("points.positions", h);
	PX_ASSERT(pError == NxParameterized::ERROR_NONE);

	pError = h.resizeArray((physx::PxI32)mPoints.size());
	PX_ASSERT(pError == NxParameterized::ERROR_NONE);

	PX_COMPILE_TIME_ASSERT(sizeof(PointParams) == sizeof(mGeomParams->points.positions.buf[0]));
	memcpy(&mGeomParams->points.positions.buf[0], &mPoints[0], mPoints.size());

	// velocities may or may not exist, handle this
	pError = mGeomParams->getParameterHandle("velocities", h);
	if (pError == NxParameterized::ERROR_NONE)
	{
		pError = mGeomParams->resizeArray(h, (physx::PxI32)mVelocities.size());
		PX_ASSERT(pError == NxParameterized::ERROR_NONE);

		pError = h.setParamVec3Array(&mVelocities[0], (physx::PxI32)mVelocities.size());
		PX_ASSERT(pError == NxParameterized::ERROR_NONE);
	}
}

void EmitterGeomExplicit::addParticleList(
    physx::PxU32 count,
    const PointParams* params,
    const physx::PxVec3* velocities)
{
	if (velocities)
	{
		mVelocities.resize(mPoints.size(), physx::PxVec3(0.0f));
	}

	for (physx::PxU32 i = 0 ; i < count ; i++)
	{
		mPoints.pushBack(*params++);

		if (velocities)
		{
			mVelocities.pushBack(*velocities++);
		}
	}

	updateCollisions();
	updateAssetPoints();
}

void EmitterGeomExplicit::addParticleList(physx::PxU32 count,
        const physx::PxVec3* positions,
        const physx::PxVec3* velocities)
{
	if (velocities)
	{
		mVelocities.resize(mPoints.size(), physx::PxVec3(0.0f));
	}

	for (physx::PxU32 i = 0 ; i < count ; i++)
	{
		PointParams params = { positions[i], false };
		mPoints.pushBack(params);

		if (velocities)
		{
			mVelocities.pushBack(velocities[i]);
		}
	}

	updateCollisions();
	updateAssetPoints();
}

void EmitterGeomExplicit::addParticleList(physx::PxU32 count,
		const PointListData& data)
{
	if (data.velocityStart != NULL)
	{
		mVelocities.resize(mPoints.size(), physx::PxVec3(0.0f));
	}
	if (data.userDataStart != NULL)
	{
		mPointsUserData.resize(mPoints.size(), 0);
	}

	const physx::PxU8* positionCurr = static_cast<const physx::PxU8*>(data.positionStart);
	const physx::PxU8* velocityCurr = static_cast<const physx::PxU8*>(data.velocityStart);
	const physx::PxU8* userDataCurr = static_cast<const physx::PxU8*>(data.userDataStart);

	for (physx::PxU32 i = 0 ; i < count ; i++)
	{
		const physx::PxVec3& position = *reinterpret_cast<const physx::PxVec3*>(positionCurr);
		positionCurr += data.positionStrideBytes;

		PointParams params = { position, false };
		mPoints.pushBack(params);

		if (velocityCurr != NULL)
		{
			const physx::PxVec3& velocity = *reinterpret_cast<const physx::PxVec3*>(velocityCurr);
			velocityCurr += data.velocityStrideBytes;

			mVelocities.pushBack(velocity);
		}
		if (userDataCurr != NULL)
		{
			const physx::PxU32 userData = *reinterpret_cast<const physx::PxU32*>(userDataCurr);
			userDataCurr += data.userDataStrideBytes;

			mPointsUserData.pushBack(userData);
		}
	}

	updateCollisions();
	updateAssetPoints();
}

void EmitterGeomExplicit::addSphereList(
    physx::PxU32 count,
    const SphereParams*  params,
    const physx::PxVec3* velocities)
{
	if (velocities)
	{
		mSphereVelocities.resize(mSpheres.size(), physx::PxVec3(0.0f));
	}

	for (physx::PxU32 i = 0 ; i < count ; i++)
	{
		mSpheres.pushBack(*params++);

		if (velocities)
		{
			mSphereVelocities.pushBack(*velocities++);
		}
	}

	updateCollisions();

	// just copy the position and velocity lists to mGeomParams
	if (!mGeomParams)
	{
		return;
	}

	NxParameterized::Handle h(*mGeomParams);
	NxParameterized::ErrorType pError;

	// Update positions

	pError = mGeomParams->getParameterHandle("spheres.positions", h);
	PX_ASSERT(pError == NxParameterized::ERROR_NONE);

	pError = h.resizeArray((physx::PxI32)mSpheres.size());
	PX_ASSERT(pError == NxParameterized::ERROR_NONE);

	PX_COMPILE_TIME_ASSERT(sizeof(SphereParams) == sizeof(mGeomParams->spheres.positions.buf[0]));
	memcpy(mGeomParams->spheres.positions.buf, &mSpheres[0], mSpheres.size() * sizeof(SphereParams));

	// velocities may or may not exist, handle this
	pError = mGeomParams->getParameterHandle("spheres.velocities", h);
	if (pError == NxParameterized::ERROR_NONE)
	{
		pError = mGeomParams->resizeArray(h, (physx::PxI32)mSphereVelocities.size());
		PX_ASSERT(pError == NxParameterized::ERROR_NONE);

		h.setParamVec3Array(&mSphereVelocities[0], (physx::PxI32)mSphereVelocities.size());
	}
}

void EmitterGeomExplicit::addEllipsoidList(
    physx::PxU32 count,
    const EllipsoidParams*  params,
    const physx::PxVec3* velocities)
{
	if (velocities)
	{
		mEllipsoidVelocities.resize(mEllipsoids.size(), physx::PxVec3(0.0f));
	}

	for (physx::PxU32 i = 0 ; i < count ; i++)
	{
		mEllipsoids.pushBack(*params++);

		if (velocities)
		{
			mEllipsoidVelocities.pushBack(*velocities++);
		}
	}

	updateCollisions();

	// just copy the position and velocity lists to mGeomParams
	if (!mGeomParams)
	{
		return;
	}

	NxParameterized::Handle h(*mGeomParams);
	NxParameterized::ErrorType pError;

	// Update positions

	pError = mGeomParams->getParameterHandle("ellipsoids.positions", h);
	PX_ASSERT(pError == NxParameterized::ERROR_NONE);

	pError = h.resizeArray((physx::PxI32)mEllipsoids.size());
	PX_ASSERT(pError == NxParameterized::ERROR_NONE);

	PX_COMPILE_TIME_ASSERT(sizeof(EllipsoidParams) == sizeof(mGeomParams->ellipsoids.positions.buf[0]));
	memcpy(mGeomParams->ellipsoids.positions.buf, &mEllipsoids[0], mEllipsoids.size() * sizeof(EllipsoidParams));

	// velocities may or may not exist, handle this
	pError = mGeomParams->getParameterHandle("ellipsoids.velocities", h);
	if (pError == NxParameterized::ERROR_NONE)
	{
		pError = mGeomParams->resizeArray(h, (physx::PxI32)mEllipsoidVelocities.size());
		PX_ASSERT(pError == NxParameterized::ERROR_NONE);

		h.setParamVec3Array(&mEllipsoidVelocities[0], (physx::PxI32)mEllipsoidVelocities.size());
	}
}

void EmitterGeomExplicit::getParticleList(
    const PointParams* &params,
    physx::PxU32& numPoints,
    const physx::PxVec3* &velocities,
    physx::PxU32& numVelocities) const
{
	numPoints = mPoints.size();
	params = numPoints ? &mPoints[0] : 0;

	numVelocities = mVelocities.size();
	velocities = numVelocities ? &mVelocities[0] : 0;
}

void EmitterGeomExplicit::getSphereList(
    const SphereParams*  &params,
    physx::PxU32& numSpheres,
    const physx::PxVec3* &velocities,
    physx::PxU32& numVelocities) const
{
	numSpheres = mSpheres.size();
	params = numSpheres ? &mSpheres[0] : 0;

	numVelocities = mSphereVelocities.size();
	velocities = numVelocities ? &mSphereVelocities[0] : 0;
}

void EmitterGeomExplicit::getEllipsoidList(
    const EllipsoidParams*  &params,
    physx::PxU32& numEllipsoids,
    const physx::PxVec3* &velocities,
    physx::PxU32& numVelocities) const
{
	numEllipsoids = mEllipsoids.size();
	params = numEllipsoids ? &mEllipsoids[0] : 0;

	numVelocities = mSphereVelocities.size();
	velocities = numVelocities ? &mSphereVelocities[0] : 0;
}

#define EPS 0.001f

bool EmitterGeomExplicit::isInside(const physx::PxVec3& x, physx::PxU32 shapeIdx) const
{
	if (!mBboxes[shapeIdx].contains(x))
	{
		return false;
	}

	physx::PxU32 nspheres = mSpheres.size(),
	             nellipsoids = mEllipsoids.size();

	if (shapeIdx < nspheres)  // Sphere
	{
		const SphereParams& sphere = mSpheres[shapeIdx];
		return physx::PxAbs((x - sphere.center).magnitudeSquared() - sphere.radius * sphere.radius) < EPS;
	}
	else if (shapeIdx < nspheres + nellipsoids)  // Ellipsoid
	{
		const EllipsoidParams& ellipsoid = mEllipsoids[shapeIdx - nspheres];

		physx::PxVec3 y(x - ellipsoid.center);

		physx::PxVec3 n(ellipsoid.normal);
		n.normalize();

		physx::PxF32 normalProj = y.dot(n);

		physx::PxVec3 planeProj = y - normalProj * n;
		physx::PxF32 planeProjMag2 = planeProj.magnitudeSquared();

		physx::PxF32 R2 = ellipsoid.radius * ellipsoid.radius,
		             Rp2 = ellipsoid.polarRadius * ellipsoid.polarRadius;

		if (planeProjMag2 > R2 + EPS)
		{
			return false;
		}

		return normalProj * normalProj < EPS + Rp2 * (1.0f - 1.0f / R2 * planeProjMag2);
	}
	else // Point
	{
		return (x - mPoints[shapeIdx - nspheres - nellipsoids].position).magnitudeSquared() < EPS;
	}
}

void EmitterGeomExplicit::AddParticle(
    physx::Array<physx::PxVec3>& positions, physx::Array<physx::PxVec3>& velocities,
    physx::Array<physx::PxU32>* userDataArrayPtr,
    const physx::PxMat34Legacy& pose, physx::PxF32 cutoff, physx::PxBounds3& outBounds,
	const physx::PxVec3& pos, const physx::PxVec3& vel, physx::PxU32 userData,
    physx::PxU32 srcShapeIdx,
    QDSRand& rand) const
{
	physx::PxF32 r = rand.getScaled(0, 1);
	if (r > cutoff)
	{
		return;
	}

	physx::PxVec3 randPos = pos + physx::PxVec3(
	                            rand.getScaled(-mDistance, mDistance),
	                            rand.getScaled(-mDistance, mDistance),
	                            rand.getScaled(-mDistance, mDistance));

	// Check collisions
	for (physx::PxU32 i = 0; i < mCollisions[srcShapeIdx].next; ++i)
	{
		physx::PxU32 collisionShapeIdx = mCollisions[srcShapeIdx].shapeIndices[i];

		// If colliding body was already processed and point lies in this body
		if (collisionShapeIdx < srcShapeIdx && isInside(pos, collisionShapeIdx))
		{
			return;
		}
	}

	/* if absolute positions are submitted, pose will be at origin */
	if (pose.isIdentity())
	{
		positions.pushBack(randPos);
		velocities.pushBack(vel);
	}
	else
	{
		positions.pushBack(pose * randPos);
		velocities.pushBack(pose.M * vel);
	}

	if (userDataArrayPtr)
	{
		userDataArrayPtr->pushBack(userData);
	}

	outBounds.include(positions.back());
}

void EmitterGeomExplicit::computeFillPositions(
    physx::Array<physx::PxVec3>& positions,
    physx::Array<physx::PxVec3>& velocities,
	physx::Array<physx::PxU32>* userDataArrayPtr,
    const physx::PxMat34Legacy& pose,
    physx::PxF32 density,
    physx::PxBounds3& outBounds,
    QDSRand& rand) const
{
	physx::PxU32 shapeIdx = 0; // Global shape index

	for (physx::PxU32 i = 0; i < mPoints.size(); ++i, ++shapeIdx)
	{
		const physx::PxU32 pointUserData = i < mPointsUserData.size() ? mPointsUserData[i] : 0;

		AddParticle(
		    positions, velocities, userDataArrayPtr,
		    pose, density, outBounds,
		    mPoints[i].position, i >= mVelocities.size() ? physx::PxVec3(0.0f) : mVelocities[i], pointUserData,
		    shapeIdx,
		    rand
		);
	}

	for (physx::PxU32 i = 0; i < mSpheres.size(); ++i, ++shapeIdx)
	{
		physx::PxVec3 c = mSpheres[i].center;
		physx::PxF32 R = mSpheres[i].radius;

		const physx::PxU32 sphereUserData = 0;

		physx::PxF32 RR = R * R;

		physx::PxI32 nx = (physx::PxI32)(R * mInvDistance);

		for (physx::PxI32 j = -nx; j < nx; ++j)
		{
			physx::PxF32 x = j * mDistance,
			             xx = x * x;

			physx::PxF32 yext = physx::PxSqrt(physx::PxMax(0.0f, RR - xx));
			physx::PxI32 ny = (physx::PxI32)(yext * mInvDistance);

			for (physx::PxI32 k = -ny; k < ny; ++k)
			{
				physx::PxF32 y = k * mDistance;

				physx::PxF32 zext = physx::PxSqrt(physx::PxMax(0.0f, RR - xx - y * y));
				physx::PxI32 nz = (physx::PxI32)(zext * mInvDistance);

				for (physx::PxI32 l = -nz; l < nz; ++l)
				{
					physx::PxF32 z = l * mDistance;

					AddParticle(
					    positions, velocities, userDataArrayPtr,
					    pose, density, outBounds,
					    c + physx::PxVec3(x, y, z),
					    i >= mSphereVelocities.size() ? physx::PxVec3(0.0f) : mSphereVelocities[i], sphereUserData,
					    shapeIdx,
					    rand
					);
				} //l
			} //k
		} //j
	} //i

	for (physx::PxU32 i = 0; i < mEllipsoids.size(); ++i, ++shapeIdx)
	{
		physx::PxVec3 c = mEllipsoids[i].center,
		              n = mEllipsoids[i].normal;

		const physx::PxU32 ellipsoidUserData = 0;

		// TODO: precompute this stuff?

		physx::PxF32 R = mEllipsoids[i].radius,
		             Rp = mEllipsoids[i].polarRadius;

		physx::PxVec3 t1, t2;
		GetNormals(n, t1, t2);

		physx::PxF32 RR = R * R,
		             Rp_R = physx::PxAbs(Rp / R);

		physx::PxI32 nx = (physx::PxI32)(R * mInvDistance);
		for (physx::PxI32 j = -nx; j <= nx ; ++j)
		{
			physx::PxF32 x = j * mDistance,
			             xx = x * x;

			physx::PxF32 yext = physx::PxSqrt(physx::PxMax(0.0f, RR - xx));
			physx::PxI32 ny = (physx::PxI32)(yext * mInvDistance);

			for (physx::PxI32 k = -ny; k <= ny; ++k)
			{
				physx::PxF32 y = k * mDistance;

				physx::PxF32 zext = Rp_R * physx::PxSqrt(physx::PxMax(0.0f, RR - xx - y * y));
				physx::PxI32 nz = (physx::PxI32)(zext * mInvDistance);

				physx::PxI32 lmax = Rp < 0 ? 0 : nz; // Check for half-ellipsoid
				for (physx::PxI32 l = -nz; l <= lmax; ++l)
				{
					physx::PxF32 z = l * mDistance;

					AddParticle(
					    positions, velocities, userDataArrayPtr,
					    pose, density, outBounds,
					    c + x * t1 + y * t2 + z * n,
					    i >= mEllipsoidVelocities.size() ? physx::PxVec3(0.0f) : mEllipsoidVelocities[i], ellipsoidUserData,
					    shapeIdx,
					    rand
					);
				} //l
			} //k
		} //j
	} //i
}

struct EndPoint
{
	physx::PxF32	val;

	physx::PxU32	boxIdx;
	bool			isLeftPoint;

	EndPoint(physx::PxF32 v, physx::PxU32 idx, bool left)
		: val(v), boxIdx(idx), isLeftPoint(left) {}

	PX_INLINE static bool isLessThan(const EndPoint& p1, const EndPoint& p2)
	{
		if (p1.val < p2.val)
		{
			return true;
		}

		// Adjacent shapes do intersect
		if (p1.val == p2.val)
		{
			return p1.isLeftPoint && !p2.isLeftPoint;
		}

		return false;
	}
};

void EmitterGeomExplicit::updateCollisions()
{
	physx::PxU32 npoints = mPoints.size(),
	             nspheres = mSpheres.size(),
	             nellipsoids = mEllipsoids.size();

	mCollisions.reset();

	physx::PxU32 ntotal = npoints + nspheres + nellipsoids;
	if (!ntotal)
	{
		return;
	}

	mBboxes.resize(ntotal);
	mCollisions.resize(ntotal);

	physx::Array<bool> doDetectOverlaps(ntotal);

	// TODO: use custom axis instead of Ox
	// E.g. (sx, sy, sz) or simply choose the best of Ox, Oy, Oz

	for (physx::PxU32 i = 0; i < nspheres; ++i)
	{
		physx::PxVec3 r3(mSpheres[i].radius);

		mBboxes[i] = physx::PxBounds3(mSpheres[i].center - r3, mSpheres[i].center + r3);
		doDetectOverlaps[i] = mSpheres[i].doDetectOverlaps;
	}

	for (physx::PxU32 i = 0; i < nellipsoids; ++i)
	{
		physx::PxVec3 r3(mEllipsoids[i].radius, mEllipsoids[i].radius, mEllipsoids[i].polarRadius);
		physx::PxBounds3 localBox(-r3, r3);

		physx::PxVec3 n = mEllipsoids[i].normal;
		n.normalize();

		PxMat33 T = physx::rotFrom2Vectors(physx::PxVec3(0, 0, 1), n);

		if(!localBox.isValid())
		{
			localBox.setEmpty();
		}

		localBox = physx::PxBounds3::transformFast(T, localBox);
		localBox.minimum += mEllipsoids[i].center;
		localBox.maximum += mEllipsoids[i].center;

		mBboxes[nspheres + i] = localBox;
		doDetectOverlaps[nspheres + i] = mEllipsoids[i].doDetectOverlaps;
	}

	for (physx::PxU32 i = 0; i < npoints; ++i)
	{
		mBboxes[nspheres + nellipsoids + i] = physx::PxBounds3(mPoints[i].position, mPoints[i].position);
		doDetectOverlaps[nspheres + nellipsoids + i] = mPoints[i].doDetectOverlaps;
	}

	physx::Array<EndPoint> endPoints;
	endPoints.reserve(2 * ntotal);
	for (physx::PxU32 i = 0; i < ntotal; ++i)
	{
		endPoints.pushBack(EndPoint(mBboxes[i].minimum.x, i, true));
		endPoints.pushBack(EndPoint(mBboxes[i].maximum.x, i, false));
	}

	physx::sort(&endPoints[0], endPoints.size(), EndPoint::isLessThan);

	physx::HashSet<physx::PxU32> openedBoxes;
	for (physx::PxU32 i = 0; i < endPoints.size(); ++i)
	{
		physx::PxU32 boxIdx = endPoints[i].boxIdx;

		if (endPoints[i].isLeftPoint)
		{
			for (physx::HashSet<physx::PxU32>::Iterator j = openedBoxes.getIterator(); !j.done(); ++j)
			{
				if (!doDetectOverlaps[boxIdx] || !doDetectOverlaps[*j])
				{
					break;
				}

				if (!mBboxes[*j].intersects(mBboxes[boxIdx]))
				{
					continue;
				}

				mCollisions[boxIdx].pushBack(*j);
				mCollisions[*j].pushBack(boxIdx);
			} //j

			openedBoxes.insert(boxIdx);
		}
		else
		{
			PX_ASSERT(openedBoxes.contains(boxIdx));
			openedBoxes.erase(boxIdx);
		}
	} //i
}

}
}
} // namespace physx::apex
