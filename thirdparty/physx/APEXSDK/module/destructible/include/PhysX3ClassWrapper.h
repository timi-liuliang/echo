/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PHYS_X3_CLASS_WRAPPER_H
#define	PHYS_X3_CLASS_WRAPPER_H

#include "NxApexDefs.h"

#if NX_SDK_VERSION_MAJOR == 3

#include "PsAllocator.h"
#include "PxRigidActor.h"
#include "PxJoint.h"
#include "PxFiltering.h"
#include "PxShape.h"
#include "PxRigidBody.h"
#include "PxRigidDynamic.h"
#include "PxConvexMesh.h"
#include "PxMat34Legacy.h"
#include "PxShapeExt.h"
#include "PxRigidBodyExt.h"
#include "NxApexReadWriteLock.h"
#include "foundation/PxSimpleTypes.h"
#include "ApexActor.h"


#define	PXFROMNXVEC3(v)	(v)
#define	NXFROMPXVEC3(v)	(v)
#define NX_INLINE	inline

typedef physx::PxVec3			NxVec3;
typedef physx::PxReal			NxReal;
typedef physx::PxU32			NxU32;
typedef physx::PxF32			NxF32;
typedef physx::PxFilterData		NxGroupsMask;
typedef physx::PxScene			NxScene;
typedef physx::PxJoint			NxJoint;
typedef physx::PxConvexMesh		NxConvexMesh;
typedef physx::PxShape			NxShape;
typedef physx::PxShape			NxConvexShape;

namespace physx
{

#define NX_MAX_REAL		PX_MAX_REAL		
static const PxI32 MAX_SHAPE_COUNT	= 256;

// define class for NxActor, NxShape, NxBox, NxSphere
NX_INLINE PxReal operator|(const PxVec3& left, const PxVec3& right)
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

PX_INLINE void PxFromNxVec3(physx::PxVec3& out, const PxVec3& in)
{
	out = in;
}

PX_INLINE void PxFromNxMat34(physx::PxMat34Legacy& out, const PxTransform& in)
{
	out	= PxMat34Legacy(in);
}
NX_INLINE NxReal distanceSquared(const NxVec3& v1, const NxVec3& v2)
	{
	NxReal dx = v1.x - v2.x;
	NxReal dy = v1.y - v2.y;
	NxReal dz = v1.z - v2.z;
	return dx * dx + dy * dy + dz * dz;
	}


enum NxBodyFlag
{
	NX_BF_KINEMATIC			= (1<<7),		//!< Enable kinematic mode for the body.
};


class NxActor : public PxActor
{
public:

	NxU32 getNbShapes()  const  
	{
		const PxRigidActor*	rigidActor	= isRigidActor();

		if (!rigidActor)
			return 0;
		SCOPED_PHYSX3_LOCK_READ(rigidActor->getScene());
		return rigidActor->getNbShapes();
	}

	bool updateMassFromShapes(NxReal density, NxReal totalMass, bool includeNonSimShapes = false)
	{
		PxRigidBody*	rigidBody	= isRigidBody();

		PX_ASSERT(density == 0.0f);
		if (rigidBody && density == 0.0f)
		{
			SCOPED_PHYSX3_LOCK_WRITE(rigidBody->getScene());
			// don't call setMass, we need update inertia and cmass at same time.
			PxRigidBodyExt::setMassAndUpdateInertia(*rigidBody, totalMass, NULL, includeNonSimShapes);

			return true;
		}
		return false;
	}

	bool readBodyFlag(NxBodyFlag bodyFlag)  const 
	{
		bool ret = false;
		if (bodyFlag != NX_BF_KINEMATIC)
		{
			PX_ALWAYS_ASSERT();
			return ret;
		}
		const PxRigidDynamic*	rigidDynamic	= isRigidDynamic();
		PxScene *scene = rigidDynamic ? rigidDynamic->getScene() : NULL;
		SCOPED_PHYSX3_LOCK_READ(scene);
		if (rigidDynamic && (rigidDynamic->getRigidDynamicFlags() & physx::PxRigidDynamicFlag::eKINEMATIC))
		{
			ret = true;
		}
		return ret;
	}

	bool raiseBodyFlag(NxBodyFlag bodyFlag) 
	{
		if (bodyFlag != NX_BF_KINEMATIC)
		{
			PX_ALWAYS_ASSERT();
			return false;
		}

		PxRigidDynamic*	rigidDynamic	= isRigidDynamic();

		if (rigidDynamic)
		{
			SCOPED_PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC, true);
			return true;
		}

		return false;
	}

	bool clearBodyFlag(NxBodyFlag bodyFlag) 
	{
		if (bodyFlag != NX_BF_KINEMATIC)
		{
			PX_ALWAYS_ASSERT();
			return false;
		}

		PxRigidDynamic*	rigidDynamic	= isRigidDynamic();

		if (rigidDynamic)
		{
			SCOPED_PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC, false);
			return true;
		}

		return false;
	}

	bool isDynamic() const 
	{
		return !!isRigidDynamic();
	}

	bool isSleeping() const
	{
		const PxRigidDynamic*	rigidDynamic	= isRigidDynamic();
		if (rigidDynamic)
		{
			SCOPED_PHYSX3_LOCK_READ(rigidDynamic->getScene());
			return rigidDynamic->isSleeping();
		}
		return false;
	}

	PxVec3 getCMassGlobalPosition()  const 
	{
		const PxRigidBody*	rigidBody		= isRigidBody();
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_READ(rigidBody->getScene());
			return (rigidBody->getGlobalPose()*rigidBody->getCMassLocalPose()).p;
		}
		return PxVec3(.0f);
	}

	PxTransform getGlobalPose  ()  const 
	{
		const PxRigidBody*	rigidBody		= isRigidBody();
		PxTransform	value;
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_READ(rigidBody->getScene());
			value	= rigidBody->getGlobalPose();
		}
		return value;
	}

	void moveKinematic(const PxTransform& destination)
	{
		PxRigidDynamic*	rigidDynamic	= isRigidDynamic();

		if (rigidDynamic)
		{
			SCOPED_PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			rigidDynamic->setKinematicTarget(destination);
		}
	}

	void wakeUp(PxReal wakeCounterValue = APEX_DEFAULT_WAKE_UP_COUNTER)  
	{
		PxRigidDynamic*	rigidDynamic	= isRigidDynamic();

		if (rigidDynamic)
		{
			SCOPED_PHYSX3_LOCK_WRITE(rigidDynamic->getScene());
			if ( !(rigidDynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC ) )
			{
				rigidDynamic->wakeUp();
				rigidDynamic->setWakeCounter(wakeCounterValue);
			}
		}
	}

	PxReal getMass()  const
	{
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_READ(rigidBody->getScene());
			return rigidBody->getMass();
		}

		return 0.0f;
	}

	PxVec3			getLinearVelocity()		const
	{
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_READ(rigidBody->getScene());
			return rigidBody->getLinearVelocity();
		}
		return PxVec3(0.0f);
	}
	void			setLinearVelocity(const PxVec3& linVel, bool autowake = true)
	{
		PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_WRITE(rigidBody->getScene());
			rigidBody->setLinearVelocity(linVel, autowake);
		}
	}
	PxVec3			getAngularVelocity()	const
	{
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_READ(rigidBody->getScene());
			return rigidBody->getAngularVelocity();
		}
		return PxVec3(0.0f);
	}
	void			setAngularVelocity(const PxVec3& angVel, bool autowake = true)
	{
		PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_WRITE(rigidBody->getScene());
			rigidBody->setAngularVelocity(angVel, autowake);
		}
	}

	PxVec3 getPointVelocity(const PxVec3& worldPosition) const
	{
		PxVec3 vel(0.0f);
		const PxRigidBody*	rigidBody	= isRigidBody();
		if (rigidBody)
		{
			SCOPED_PHYSX3_LOCK_READ(rigidBody->getScene());
			vel = PxRigidBodyExt::getVelocityAtPos(*rigidBody, worldPosition);
		}
		return vel;
	}

	NxActor() : PxActor(PxBaseFlag::eIS_RELEASABLE | PxBaseFlag::eOWNS_MEMORY) {}
};

PX_INLINE NxShape*	getShape(const NxActor& actor, NxU32 index)
{
	const PxRigidActor*	rigidActor = actor.isRigidActor();

	if (!rigidActor)
	{
		return NULL;
	}
	SCOPED_PHYSX3_LOCK_READ(rigidActor->getScene());
	PxShape* shape;
	rigidActor->getShapes(&shape, 1, index);

	return (NxShape*)shape;
}


class NxBox
	{
	public:
	/**
	\brief Constructor
	*/
	NX_INLINE NxBox()
		{
		}

	/**
	\brief Constructor

	\param _center Center of the OBB
	\param _extents Extents/radii of the obb.
	\param _rot rotation to apply to the obb.
	*/
	NX_INLINE NxBox(const physx::PxVec3& _center, const physx::PxVec3& _extents, const physx::PxMat33& _rot) : center(_center), extents(_extents), rot(_rot)
		{
		}

	/**
	\brief Destructor
	*/
	NX_INLINE ~NxBox()
		{
		}

	/**
	 \brief Setups an empty box.
	*/
	NX_INLINE void setEmpty()
		{
		center	= physx::PxVec3(0);
		extents = physx::PxVec3(-PX_MAX_REAL, -PX_MAX_REAL, -PX_MAX_REAL);
		rot		= physx::PxMat33::createIdentity();
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 \brief Tests if a point is contained within the box

	 See #NxBoxContainsPoint().

	 \param		p	[in] the world point to test
	 \return	true if inside the box
	*/
	NX_INLINE bool containsPoint(const physx::PxVec3& p) const
		{
		return NxBoxContainsPoint(*this, p);
		}

	/**
	 \brief Builds a box from AABB and a world transform.

	 See #NxCreateBox().

	 \param		aabb	[in] the aabb
	 \param		mat		[in] the world transform
	*/
	NX_INLINE void create(const NxBounds3& aabb, const NxMat34& mat)
		{
		NxCreateBox(*this, aabb, mat);
		}
#endif
	/**
	 \brief Recomputes the box after an arbitrary transform by a 4x4 matrix.

	 \param		mtx		[in] the transform matrix
	 \param		obb		[out] the transformed OBB
	*/
	/**
	 \brief Checks the box is valid.

	 \return	true if the box is valid
	*/
	NX_INLINE bool isValid() const
		{
		// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
		if (extents.x < 0.0f)	return false;
		if (extents.y < 0.0f)	return false;
		if (extents.z < 0.0f)	return false;
		return true;
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 \brief Computes the obb planes.

	 See #NxComputeBoxPlanes().

	 \param		planes	[out] 6 box planes
	 \return	true if success
	*/
	NX_INLINE bool computePlanes(NxPlane* planes) const
		{
		return NxComputeBoxPlanes(*this, planes);
		}

	/**
	 \brief Computes the obb points.

	 See #NxComputeBoxPoints().

	 \param		pts	[out] 8 box points
	 \return	true if success
	*/
	NX_INLINE bool computePoints(physx::PxVec3* pts) const
		{
		return NxComputeBoxPoints(*this, pts);
		}

	/**
	 \brief Computes vertex normals.

	 See #NxComputeBoxVertexNormals().

	 \param		pts	[out] 8 box points
	 \return	true if success
	*/
	NX_INLINE bool computeVertexNormals(physx::PxVec3* pts) const
		{
		return NxComputeBoxVertexNormals(*this, pts);
		}

	/**
	 \brief Returns edges.

	 See #NxGetBoxEdges().

	 \return	24 indices (12 edges) indexing the list returned by ComputePoints()
	*/
	NX_INLINE const NxU32* getEdges() const
		{
		return NxGetBoxEdges();
		}

		/**
		\brief Return edge axes indices.

		 See #NxgetBoxEdgeAxes().

		 \return Array of edge axes indices.
		*/

	NX_INLINE const NxI32* getEdgesAxes() const
		{
		return NxGetBoxEdgesAxes();
		}

	/**
	 \brief Returns triangles.

	 See #NxGetBoxTriangles().


	 \return 36 indices (12 triangles) indexing the list returned by ComputePoints()
	*/
	NX_INLINE const NxU32* getTriangles() const
		{
		return NxGetBoxTriangles();
		}

	/**
	 \brief Returns local edge normals.

	 See #NxGetBoxLocalEdgeNormals().

	 \return edge normals in local space
	*/
	NX_INLINE const physx::PxVec3* getLocalEdgeNormals() const
		{
		return NxGetBoxLocalEdgeNormals();
		}

	/**
	 \brief Returns world edge normal

	 See #NxComputeBoxWorldEdgeNormal().

	 \param		edge_index		[in] 0 <= edge index < 12
	 \param		world_normal	[out] edge normal in world space
	*/
	NX_INLINE void computeWorldEdgeNormal(NxU32 edge_index, physx::PxVec3& world_normal) const
		{
		NxComputeBoxWorldEdgeNormal(*this, edge_index, world_normal);
		}

	/**
	 \brief Computes a capsule surrounding the box.

	 See #NxComputeCapsuleAroundBox().

	 \param		capsule	[out] the capsule
	*/
	NX_INLINE void computeCapsule(NxCapsule& capsule) const
		{
		NxComputeCapsuleAroundBox(*this, capsule);
		}

	/**
	 \brief Checks the box is inside another box

	 See #NxIsBoxAInsideBoxB().

	 \param		box		[in] the other box
	 \return	TRUE if we're inside the other box
	*/
	NX_INLINE bool isInside(const NxBox& box) const
		{
		return NxIsBoxAInsideBoxB(*this, box);
		}
#endif
	// Accessors

		/**
		\brief Return center of box.

		\return Center of box.
		*/
	NX_INLINE const physx::PxVec3& GetCenter() const
		{
		return center;
		}

		/**
		\brief Return extents(radii) of box.

		\return Extents of box.
		*/

	NX_INLINE const physx::PxVec3& GetExtents() const
		{
		return extents;
		}

		/**
		\brief return box rotation.

		\return Box Rotation.
		*/

	NX_INLINE const physx::PxMat33& GetRot() const
		{
		return rot;
		}

/*	NX_INLINE	void GetRotatedExtents(NxMat33& extents) const
		{
		extents = mRot;
		extents.Scale(mExtents);
		}*/

	physx::PxVec3	center;
	physx::PxVec3	extents;
	physx::PxMat33	rot;
	};


/**
\brief Represents an infinite ray as an origin and direction.

The direction should be normalized.
*/
class NxRay
	{
	public:
	/**
	Constructor
	*/
	NX_INLINE NxRay()
		{
		}

	/**
	Constructor
	*/
	NX_INLINE NxRay(const NxVec3& _orig, const NxVec3& _dir) : orig(_orig), dir(_dir)
		{
		}

	/**
	Destructor
	*/
	NX_INLINE ~NxRay()
		{
		}
#ifdef FOUNDATION_EXPORTS

	NX_INLINE NxF32 distanceSquared(const NxVec3& point, NxF32* t=NULL) const
		{
		return NxComputeDistanceSquared(*this, point, t);
		}

	NX_INLINE NxF32 distance(const NxVec3& point, NxF32* t=NULL) const
		{
		return sqrtf(distanceSquared(point, t));
		}
#endif

	NxVec3	orig;	//!< Ray origin
	NxVec3	dir;	//!< Normalized direction
	};

	NX_INLINE void ComputeReflexionVector(NxVec3& reflected, const NxVec3& incoming_dir, const NxVec3& outward_normal)
	{
		reflected = incoming_dir - outward_normal * 2.0f * incoming_dir.dot(outward_normal);
	}

	NX_INLINE void ComputeReflexionVector(NxVec3& reflected, const NxVec3& source, const NxVec3& impact, const NxVec3& normal)
	{
		NxVec3 V = impact - source;
		reflected = V - normal * 2.0f * V.dot(normal);
	}

	NX_INLINE void ComputeNormalCompo(NxVec3& normal_compo, const NxVec3& outward_dir, const NxVec3& outward_normal)
	{
		normal_compo = outward_normal * outward_dir.dot(outward_normal);
	}

	NX_INLINE void ComputeTangentCompo(NxVec3& outward_dir, const NxVec3& outward_normal)
	{
		outward_dir -= outward_normal * outward_dir.dot(outward_normal);
	}

	NX_INLINE void DecomposeVector(NxVec3& normal_compo, NxVec3& tangent_compo, const NxVec3& outward_dir, const NxVec3& outward_normal)
	{
		normal_compo = outward_normal * outward_dir.dot(outward_normal);
		tangent_compo = outward_dir - normal_compo;
	}


	/**
\brief Represents a sphere defined by its center point and radius.
*/
class NxSphere
	{
	public:
	/**
	\brief Constructor
	*/
	NX_INLINE NxSphere()
		{
		}

	/**
	\brief Constructor
	*/
	NX_INLINE NxSphere(const NxVec3& _center, NxF32 _radius) : center(_center), radius(_radius)
		{
		}
#ifdef FOUNDATION_EXPORTS
	/**
	\brief Constructor
	*/
	NX_INLINE NxSphere(unsigned nb_verts, const NxVec3* verts)
		{
		NxComputeSphere(*this, nb_verts, verts);
		}
#endif
	/**
	\brief Copy constructor
	*/
	NX_INLINE NxSphere(const NxSphere& sphere) : center(sphere.center), radius(sphere.radius)
		{
		}
#ifdef FOUNDATION_EXPORTS

	/**
	\brief Union of spheres
	*/
	NX_INLINE NxSphere(const NxSphere& sphere0, const NxSphere& sphere1)
		{
		NxMergeSpheres(*this, sphere0, sphere1);
		}
#endif
	/**
	\brief Destructor
	*/
	NX_INLINE ~NxSphere()
		{
		}
#ifdef FOUNDATION_EXPORTS

	NX_INLINE NxBSphereMethod compute(unsigned nb_verts, const NxVec3* verts)
		{
		return NxComputeSphere(*this, nb_verts, verts);
		}

	NX_INLINE bool fastCompute(unsigned nb_verts, const NxVec3* verts)
		{
		return NxFastComputeSphere(*this, nb_verts, verts);
		}
#endif
	/**
	\brief Checks the sphere is valid.

	\return		true if the sphere is valid
	*/
	NX_INLINE bool IsValid() const
		{
		// Consistency condition for spheres: Radius >= 0.0f
		return radius >= 0.0f;
		}

	/**
	\brief Tests if a point is contained within the sphere.

	\param[in] p the point to test
	\return	true if inside the sphere
	*/
	NX_INLINE bool Contains(const NxVec3& p) const
		{
		return distanceSquared(center, p) <= radius*radius;
		}

	/**
	\brief Tests if a sphere is contained within the sphere.

	\param		sphere	[in] the sphere to test
	\return		true if inside the sphere
	*/
	NX_INLINE bool Contains(const NxSphere& sphere)	const
		{
		// If our radius is the smallest, we can't possibly contain the other sphere
		if (radius < sphere.radius)	return false;
		// So r is always positive or null now
		float r = radius - sphere.radius;
		return distanceSquared(center, sphere.center) <= r*r;
		}

	/**
	\brief Tests if a box is contained within the sphere.

	\param		min		[in] min value of the box
	\param		max		[in] max value of the box
	\return		true if inside the sphere
	*/
	NX_INLINE bool Contains(const NxVec3& min, const NxVec3& max) const
		{
		// I assume if all 8 box vertices are inside the sphere, so does the whole box.
		// Sounds ok but maybe there's a better way?
		NxF32 R2 = radius * radius;
		NxVec3 p;
		p.x=max.x; p.y=max.y; p.z=max.z;	if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;
		p.x=max.x; p.y=min.y;				if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;
		p.x=max.x; p.y=max.y; p.z=min.z;	if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;
		p.x=max.x; p.y=min.y;				if (distanceSquared(center, p)>=R2)	return false;
		p.x=min.x;							if (distanceSquared(center, p)>=R2)	return false;

		return true;
		}

	/**
	\brief Tests if the sphere intersects another sphere

	\param		sphere	[in] the other sphere
	\return		true if spheres overlap
	*/
	NX_INLINE bool Intersect(const NxSphere& sphere) const
		{
		NxF32 r = radius + sphere.radius;
		return distanceSquared(center, sphere.center) <= r*r;
		}

	NxVec3	center;		//!< Sphere's center
	NxF32	radius;		//!< Sphere's radius
	};
} // namespace physx
#elif NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
PX_INLINE NxShape*	getShape(NxActor& actor, NxU32 index)
{
	return actor.getShapes()[index];
}
#endif	// NX_SDK_VERSION_MAJOR == 3
#endif	// PHYS_X3_CLASS_WRAPPER_H
