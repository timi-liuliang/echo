/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __EMITTER_GEOM_EXPLICIT_H__
#define __EMITTER_GEOM_EXPLICIT_H__

#include "EmitterGeom.h"
#include "PsUserAllocated.h"
#include "EmitterGeomExplicitParams.h"
#include "ApexRand.h"

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


class EmitterGeomExplicit : public NxEmitterExplicitGeom, public EmitterGeom
{
	void updateAssetPoints();

public:
	EmitterGeomExplicit(NxParameterized::Interface* params);
	EmitterGeomExplicit();

	virtual ~EmitterGeomExplicit() {}

	/* Asset callable methods */
	NxEmitterGeom*				getNxEmitterGeom();
	const NxEmitterExplicitGeom* isExplicitGeom() const
	{
		return this;
	}
	NxApexEmitterType::Enum		getEmitterType() const
	{
		return NxApexEmitterType::NX_ET_FILL;
	}
	void						setEmitterType(NxApexEmitterType::Enum) {}   // PX_ASSERT(t == NxApexEmitterType::NX_ET_FILL);
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
	        const physx::PxMat34Legacy& pose,
	        physx::PxF32 density,
	        physx::PxBounds3& outBounds,
	        QDSRand& rand) const
	{
		computeFillPositions(positions, velocities, NULL, pose, density, outBounds, rand);
	}

	void						computeFillPositions(physx::Array<physx::PxVec3>& positions,
	        physx::Array<physx::PxVec3>& velocities,
			physx::Array<physx::PxU32>* userDataArrayPtr,
	        const physx::PxMat34Legacy&,
	        physx::PxF32,
	        physx::PxBounds3& outBounds,
	        QDSRand& rand) const;

	/* Stubs */
	physx::PxF32				computeNewlyCoveredVolume(const physx::PxMat34Legacy&, const physx::PxMat34Legacy&) const
	{
		return 0.0f;
	}
	physx::PxF32				computeEmitterVolume() const
	{
		return 0.0f;
	}
	physx::PxVec3				randomPosInFullVolume(const physx::PxMat34Legacy&, QDSRand&) const
	{
		return physx::PxVec3(0.0f, 0.0f, 0.0f);
	}
	physx::PxVec3				randomPosInNewlyCoveredVolume(const physx::PxMat34Legacy& , const physx::PxMat34Legacy&, QDSRand&) const
	{
		return physx::PxVec3(0.0f, 0.0f, 0.0f);
	}
	bool						isInEmitter(const physx::PxVec3&, const physx::PxMat34Legacy&) const
	{
		return false;
	}

	void						resetParticleList()
	{
		mPoints.clear();
		mVelocities.clear();
		mPointsUserData.clear();

		mSpheres.clear();
		mSphereVelocities.clear();

		mEllipsoids.clear();
		mEllipsoidVelocities.clear();
	}

	void						addParticleList(physx::PxU32 count,
	        const PointParams* params,
	        const physx::PxVec3* velocities = 0);

	void						addParticleList(physx::PxU32 count,
	        const physx::PxVec3* positions,
	        const physx::PxVec3* velocities = 0);

	void						addParticleList(physx::PxU32 count,
			const PointListData& data);

	void						addSphereList(physx::PxU32 count,
	        const SphereParams* params,
	        const physx::PxVec3* velocities = 0);

	void						addEllipsoidList(physx::PxU32 count,
	        const EllipsoidParams* params,
	        const physx::PxVec3* velocities = 0);

	void						getParticleList(const PointParams* &params,
	        physx::PxU32& numPoints,
	        const physx::PxVec3* &velocities,
	        physx::PxU32& numVelocities) const;

	void						getSphereList(const SphereParams* &params,
	        physx::PxU32& numSpheres,
	        const physx::PxVec3* &velocities,
	        physx::PxU32& numVelocities) const;

	void						getEllipsoidList(const EllipsoidParams* &params,
	        physx::PxU32& numEllipsoids,
	        const physx::PxVec3* &velocities,
	        physx::PxU32& numVelocities) const;

	physx::PxU32				getParticleCount() const
	{
		return mPoints.size();
	}
	physx::PxVec3				getParticlePos(physx::PxU32 index) const
	{
		return mPoints[ index ].position;
	}

	physx::PxU32				getSphereCount() const
	{
		return mSpheres.size();
	}
	physx::PxVec3				getSphereCenter(physx::PxU32 index) const
	{
		return mSpheres[ index ].center;
	}
	physx::PxF32				getSphereRadius(physx::PxU32 index) const
	{
		return mSpheres[ index ].radius;
	}

	physx::PxU32				getEllipsoidCount() const
	{
		return mEllipsoids.size();
	}
	physx::PxVec3				getEllipsoidCenter(physx::PxU32 index) const
	{
		return mEllipsoids[ index ].center;
	}
	physx::PxF32				getEllipsoidRadius(physx::PxU32 index) const
	{
		return mEllipsoids[ index ].radius;
	}
	physx::PxVec3				getEllipsoidNormal(physx::PxU32 index) const
	{
		return mEllipsoids[ index ].normal;
	}
	physx::PxF32				getEllipsoidPolarRadius(physx::PxU32 index) const
	{
		return mEllipsoids[ index ].polarRadius;
	}

	physx::PxF32				getDistance() const
	{
		return mDistance;
	}

protected:
	mutable QDSRand					mRand;

#	define MAX_COLLISION_SHAPES 5

	struct CollisionList
	{
		physx::PxU32 shapeIndices[MAX_COLLISION_SHAPES];
		physx::PxU32 next;

		PX_INLINE CollisionList(): next(0) {}

		PX_INLINE void pushBack(physx::PxU32 shapeIdx)
		{
			if (next >= MAX_COLLISION_SHAPES)
			{
				PX_ASSERT(0 && "Too many colliding shapes in explicit emitter");
				return;
			}

			shapeIndices[next++] = shapeIdx;
		}
	};

	// Collision table holds indices of shapes which collide with some shape
	// Shape indexing: firstly go spheres, then ellipsoids, then points
	physx::Array<CollisionList>		mCollisions;
	physx::Array<physx::PxBounds3>	mBboxes;

	void							updateCollisions();
	bool							isInside(const physx::PxVec3& x, physx::PxU32 shapeIdx) const;

	void AddParticle(physx::Array<physx::PxVec3>& positions, physx::Array<physx::PxVec3>& velocities,
	                 physx::Array<physx::PxU32>* userDataArrayPtr,
	                 const physx::PxMat34Legacy& pose, physx::PxF32 cutoff, physx::PxBounds3& outBounds,
	                 const physx::PxVec3& pos, const physx::PxVec3& vel, physx::PxU32 userData,
	                 physx::PxU32 srcShapeIdx,
	                 QDSRand& rand) const;

	physx::Array<SphereParams>		mSpheres;
	physx::Array<physx::PxVec3>		mSphereVelocities;

	physx::Array<EllipsoidParams>	mEllipsoids;
	physx::Array<physx::PxVec3>		mEllipsoidVelocities;

	physx::Array<PointParams>		mPoints;
	physx::Array<physx::PxVec3>		mVelocities;
	physx::Array<physx::PxU32>		mPointsUserData;

	physx::PxF32					mDistance, mInvDistance;

	EmitterGeomExplicitParams*		mGeomParams;
};

}
}
} // end namespace physx::apex

#endif