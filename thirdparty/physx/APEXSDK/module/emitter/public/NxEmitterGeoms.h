/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_EMITTER_GEOMS_H
#define NX_EMITTER_GEOMS_H

#include "NxApex.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

///Emitter type
struct NxApexEmitterType
{
	enum Enum
	{
		NX_ET_RATE = 0,			///< use emittion rate to calculate the number of particles
		NX_ET_DENSITY_ONCE,		///< emit density particles at once
		NX_ET_DENSITY_BRUSH,	///< brush behavior: particles are emitted proportionally to the newly occupied vlume
		NX_ET_FILL,				///< fill the volume of the emitter with particles
		NX_ET_UNDEFINED,
	};
};

class NxEmitterBoxGeom;
class NxEmitterSphereGeom;
class NxEmitterSphereShellGeom;
class NxEmitterCylinderGeom;
class NxEmitterExplicitGeom;

///Base class for all emitter shapes
class NxEmitterGeom
{
public:
	///Sets the type of the emitter
	virtual void								setEmitterType(NxApexEmitterType::Enum) = 0;
	///Gets the type of the emitter
	virtual NxApexEmitterType::Enum				getEmitterType() const = 0;

	///If it is a box, cast to box class, return NULL otherwise
	virtual const NxEmitterBoxGeom* 			isBoxGeom() const
	{
		return NULL;
	}
	///If it is a sphere, cast to sphere class, return NULL otherwise
	virtual const NxEmitterSphereGeom* 			isSphereGeom() const
	{
		return NULL;
	}
	///If it is a sphere shell, cast to sphere shell class, return NULL otherwise
	virtual const NxEmitterSphereShellGeom* 	isSphereShellGeom() const
	{
		return NULL;
	}
	///If it is a cylinder shell, cast to cylinder class, return NULL otherwise
	virtual const NxEmitterCylinderGeom* 		isCylinderGeom() const
	{
		return NULL;
	}
	///If it is an explicit geom, cast to explicit geom class, return NULL otherwise
	virtual const NxEmitterExplicitGeom* 		isExplicitGeom() const
	{
		return NULL;
	}
};

///Sphere shell shape for an emitter. It's a shape formed by the difference of two cocentered spheres.
class NxEmitterSphereShellGeom : public NxEmitterGeom
{
public:
	virtual void setRadius(physx::PxF32 radius) = 0;
	virtual physx::PxF32 getRadius() const = 0;

	virtual void setShellThickness(physx::PxF32 thickness) = 0;
	virtual physx::PxF32 getShellThickness() const = 0;
};

///Spherical shape for an emitter.
class NxEmitterSphereGeom : public NxEmitterGeom
{
public:
	virtual void	setRadius(physx::PxF32 radius) = 0;
	virtual physx::PxF32	getRadius() const = 0;
};

///Cylindrical shape for an emitter.
class NxEmitterCylinderGeom : public NxEmitterGeom
{
public:
	virtual void	setRadius(physx::PxF32 radius) = 0;
	virtual physx::PxF32	getRadius() const = 0;

	virtual void	setHeight(physx::PxF32 height) = 0;
	virtual physx::PxF32	getHeight() const = 0;
};

///Box shape for an emitter.
class NxEmitterBoxGeom : public NxEmitterGeom
{
public:
	virtual void	setExtents(const physx::PxVec3& extents) = 0;
	virtual physx::PxVec3	getExtents() const = 0;
};
///Explicit geometry. Coordinates of each particle are given explicitly.
class NxEmitterExplicitGeom : public NxEmitterGeom
{
public:

	struct PointParams
	{
		physx::PxVec3 position;
		bool doDetectOverlaps;
	};

	struct SphereParams
	{
		physx::PxVec3 center;
		physx::PxF32 radius;
		bool doDetectOverlaps;
	};

	struct EllipsoidParams
	{
		physx::PxVec3 center;
		physx::PxF32 radius;
		physx::PxVec3 normal;
		physx::PxF32 polarRadius;
		bool doDetectOverlaps;
	};

	///Remove all shapes
	virtual void    resetParticleList() = 0;

	/**
	\brief Add particles to geometry to be emitted
	\param [in] count - number of particles being added by this call
	\param [in] params must be specified.  When emitted, these relative positions are added to emitter actor position
	\param [in] velocities if NULL, the geometry's velocity list will be padded with zero velocities and the asset's velocityRange will be used for velocity
	*/
	virtual void    addParticleList(physx::PxU32 count,
	                                const PointParams* params,
	                                const physx::PxVec3* velocities = 0) = 0;

	virtual void    addParticleList(physx::PxU32 count,
	                                const physx::PxVec3* positions,
	                                const physx::PxVec3* velocities = 0) = 0;

	struct PointListData
	{
		const void* positionStart;
		physx::PxU32 positionStrideBytes;
		const void* velocityStart;
		physx::PxU32 velocityStrideBytes;
		const void* userDataStart;
		physx::PxU32 userDataStrideBytes;
	};
	virtual void    addParticleList(physx::PxU32 count, const PointListData& data) = 0;


	virtual void    addSphereList(physx::PxU32 count,
	                              const SphereParams* params,
	                              const physx::PxVec3* velocities = 0) = 0;

	virtual void    addEllipsoidList(physx::PxU32 count,
	                                 const EllipsoidParams*  params,
	                                 const physx::PxVec3* velocities = 0) = 0;

	///Access the list of shapes
	virtual void	getParticleList(const PointParams* &params,
	                                physx::PxU32& numPoints,
	                                const physx::PxVec3* &velocities,
	                                physx::PxU32& numVelocities) const = 0;

	virtual void	getSphereList(const SphereParams* &params,
	                              physx::PxU32& numSpheres,
	                              const physx::PxVec3* &velocities,
	                              physx::PxU32& numVelocities) const = 0;

	virtual void	getEllipsoidList(const EllipsoidParams* &params,
	                                 physx::PxU32& numEllipsoids,
	                                 const physx::PxVec3* &velocities,
	                                 physx::PxU32& numVelocities) const = 0;

	///Get the number of points
	virtual physx::PxU32   getParticleCount() const = 0;

	///Get the position of point
	virtual physx::PxVec3  getParticlePos(physx::PxU32 index) const = 0;

	///Get the number of spheres
	virtual physx::PxU32   getSphereCount() const = 0;

	///Get the center of the sphere
	virtual physx::PxVec3 getSphereCenter(physx::PxU32 index) const = 0;

	///Get the radius of the sphere
	virtual physx::PxF32 getSphereRadius(physx::PxU32 index) const = 0;

	///Get the number of ellipsoids
	virtual physx::PxU32   getEllipsoidCount() const = 0;

	///Get the center of the ellipsoid
	virtual physx::PxVec3 getEllipsoidCenter(physx::PxU32 index) const = 0;

	///Get the radius of the ellipsoid
	virtual physx::PxF32 getEllipsoidRadius(physx::PxU32 index) const = 0;

	///Get the normal of the ellipsoid
	virtual physx::PxVec3 getEllipsoidNormal(physx::PxU32 index) const = 0;

	///Get the polar radius of the ellipsoid
	virtual physx::PxF32 getEllipsoidPolarRadius(physx::PxU32 index) const = 0;

	///Get average distance between particles
	virtual physx::PxF32 getDistance() const = 0;
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_EMITTER_GEOMS_H
