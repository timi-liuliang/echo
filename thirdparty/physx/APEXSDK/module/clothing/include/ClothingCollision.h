/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_COLLISION_H
#define CLOTHING_COLLISION_H

#include "ApexInterface.h"
#include "ApexSDKHelpers.h"
#include "NxClothingCollision.h"
#include "ApexRWLockable.h"

#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{
namespace clothing
{

class ClothingActor;


class ClothingCollision : public ApexResource, public NxApexResource, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingCollision(NxResourceList& list, ClothingActor& owner);

	/* NxApexResource */
	virtual void release();

	PxU32 getListIndex() const
	{
		return m_listIndex;
	}
	void setListIndex(NxResourceList& list, PxU32 index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	void destroy();

	void setId(PxI32 id)
	{
		mId = id;
	}
	PxI32 getId() const
	{
		return mId;
	}

	virtual NxClothingPlane* isPlane() { NX_READ_ZONE(); return NULL;}
	virtual NxClothingConvex* isConvex() { NX_READ_ZONE(); return NULL;}
	virtual NxClothingSphere* isSphere() { NX_READ_ZONE(); return NULL;}
	virtual NxClothingCapsule* isCapsule() { NX_READ_ZONE(); return NULL;}
	virtual NxClothingTriangleMesh* isTriangleMesh() { NX_READ_ZONE(); return NULL;}

protected:

	ClothingActor& mOwner;
	bool mInRelease;

	PxI32 mId;

private:
	ClothingCollision& operator=(const ClothingCollision&);
};


/************************************************************************/
// ClothingPlane
/************************************************************************/
class ClothingPlane : public NxClothingPlane, public ClothingCollision
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingPlane(NxResourceList& list, ClothingActor& owner, const PxPlane& plane) :
		ClothingCollision(list, owner),
		mPlane(plane),
		mRefCount(0)
	{
	}

	virtual NxClothingCollisionType::Enum getType() const
	{
		NX_READ_ZONE();
		return NxClothingCollisionType::Plane;
	}
	virtual NxClothingPlane* isPlane() { return this; }
	virtual NxClothingConvex* isConvex() { return NULL; }
	virtual NxClothingSphere* isSphere() { return NULL; }
	virtual NxClothingCapsule* isCapsule() { return NULL; }
	virtual NxClothingTriangleMesh* isTriangleMesh() { return NULL; }

	virtual void release()
	{
		if (mRefCount > 0)
		{
			APEX_DEBUG_WARNING("Cannot release NxClothingPlane that is referenced by a NxClothingConvex. Release convex first.");
			return;
		}

		ClothingCollision::release();
	}

	virtual void setPlane(const PxPlane& plane);
	virtual PxPlane& getPlane()
	{
		return mPlane;
	}

	void incRefCount()
	{
		++mRefCount;
	}
	void decRefCount()
	{
		PX_ASSERT(mRefCount > 0);
		--mRefCount;
	}
	virtual PxU32 getRefCount() const
	{
		return (physx::PxU32)mRefCount;
	}

protected:
	PxPlane mPlane;

	PxI32 mRefCount;
};



/************************************************************************/
// ClothingConvex
/************************************************************************/
class ClothingConvex : public NxClothingConvex, public ClothingCollision
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingConvex(NxResourceList& list, ClothingActor& owner, NxClothingPlane** planes, PxU32 numPlanes) :
		ClothingCollision(list, owner)
	{
		mPlanes.resize(numPlanes);
		for (PxU32 i = 0; i < numPlanes; ++i)
		{
			mPlanes[i] = DYNAMIC_CAST(ClothingPlane*)(planes[i]);
			mPlanes[i]->incRefCount();
		}
	}

	virtual NxClothingCollisionType::Enum getType() const
	{
		return NxClothingCollisionType::Convex;
	}
	virtual NxClothingPlane* isPlane() { return NULL; }
	virtual NxClothingConvex* isConvex() { return this; }
	virtual NxClothingSphere* isSphere() { return NULL; }
	virtual NxClothingCapsule* isCapsule() { return NULL; }
	virtual NxClothingTriangleMesh* isTriangleMesh() { return NULL; }

	virtual void release()
	{
		for (PxU32 i = 0; i < mPlanes.size(); ++i)
		{
			mPlanes[i]->decRefCount();
		}

		ClothingCollision::release();
	}

	virtual void releaseWithPlanes()
	{
		for (PxU32 i = 0; i < mPlanes.size(); ++i)
		{
			mPlanes[i]->decRefCount();
			mPlanes[i]->release();
		}

		ClothingCollision::release();
	}

	virtual PxU32 getNumPlanes()
	{
		return mPlanes.size();
	}

	virtual NxClothingPlane** getPlanes()
	{
		return (NxClothingPlane**)&mPlanes[0];
	}

protected:
	Array<ClothingPlane*> mPlanes;
};




/************************************************************************/
// ClothingSphere
/************************************************************************/
class ClothingSphere : public NxClothingSphere, public ClothingCollision
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingSphere(NxResourceList& list, ClothingActor& owner, const PxVec3& position, PxF32 radius) :
		ClothingCollision(list, owner),
		mPosition(position),
		mRadius(radius),
		mRefCount(0)
	{
	}

	virtual NxClothingCollisionType::Enum getType() const
	{
		return NxClothingCollisionType::Sphere;
	}
	virtual NxClothingPlane* isPlane() { return NULL; }
	virtual NxClothingConvex* isConvex() { return NULL; }
	virtual NxClothingSphere* isSphere() { return this; }
	virtual NxClothingCapsule* isCapsule() { return NULL; }
	virtual NxClothingTriangleMesh* isTriangleMesh() { return NULL; }

	virtual void release()
	{
		if (mRefCount > 0)
		{
			APEX_DEBUG_WARNING("Cannot release NxClothingSphere that is referenced by an NxClothingCapsule. Release capsule first.");
			return;
		}

		ClothingCollision::release();
	}
	
	virtual void setPosition(const PxVec3& position);
	virtual const PxVec3& getPosition() const
	{
		return mPosition;
	}

	virtual void setRadius(PxF32 radius);
	virtual PxF32 getRadius() const
	{
		return mRadius;
	}

	void incRefCount()
	{
		++mRefCount;
	}
	void decRefCount()
	{
		PX_ASSERT(mRefCount > 0);
		--mRefCount;
	}
	virtual PxU32 getRefCount() const
	{
		return mRefCount;
	}

protected:
	PxVec3 mPosition;
	PxF32 mRadius;

	PxU32 mRefCount;
};




/************************************************************************/
// ClothingCapsule
/************************************************************************/
class ClothingCapsule : public NxClothingCapsule, public ClothingCollision
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingCapsule(NxResourceList& list, ClothingActor& owner, NxClothingSphere& sphere1, NxClothingSphere& sphere2) :
		ClothingCollision(list, owner)
	{
		mSpheres[0] = (DYNAMIC_CAST(ClothingSphere*)(&sphere1));
		mSpheres[0]->incRefCount();

		mSpheres[1] = (DYNAMIC_CAST(ClothingSphere*)(&sphere2));
		mSpheres[1]->incRefCount();
	}

	virtual NxClothingCollisionType::Enum getType() const
	{
		return NxClothingCollisionType::Capsule;
	}
	virtual NxClothingPlane* isPlane() { return NULL; }
	virtual NxClothingConvex* isConvex() { return NULL; }
	virtual NxClothingSphere* isSphere() { return NULL; }
	virtual NxClothingCapsule* isCapsule() { return this; }
	virtual NxClothingTriangleMesh* isTriangleMesh() { return NULL; }

	virtual void release()
	{
		mSpheres[0]->decRefCount();
		mSpheres[1]->decRefCount();

		ClothingCollision::release();
	}

	virtual void releaseWithSpheres()
	{
		mSpheres[0]->decRefCount();
		mSpheres[1]->decRefCount();

		mSpheres[0]->release();
		mSpheres[1]->release();

		ClothingCollision::release();
	}

	virtual NxClothingSphere** getSpheres()
	{
		return (NxClothingSphere**)mSpheres;
	}

protected:
	ClothingSphere* mSpheres[2];
};




/************************************************************************/
// ClothingTriangleMesh
/************************************************************************/

struct ClothingTriangle
{
	PxVec3 v[3];
	PxU32 id;

	bool operator<(const ClothingTriangle& other) const
	{
		return id < other.id;
	}
};

class ClothingTriangleMesh : public NxClothingTriangleMesh, public ClothingCollision
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingTriangleMesh(NxResourceList& list, ClothingActor& owner) :
		ClothingCollision(list, owner),
		mPose(PxMat44::createIdentity())
	{
	}

	virtual NxClothingCollisionType::Enum getType() const
	{
		return NxClothingCollisionType::TriangleMesh;
	}
	virtual NxClothingPlane* isPlane() { return NULL; }
	virtual NxClothingConvex* isConvex() { return NULL; }
	virtual NxClothingSphere* isSphere() { return NULL; }
	virtual NxClothingCapsule* isCapsule() { return NULL; }
	virtual NxClothingTriangleMesh* isTriangleMesh() { return this; }

	virtual PxU32 lockTriangles(const PxU32** ids, const PxVec3** triangles);
	virtual PxU32 lockTrianglesWrite(const PxU32** ids, PxVec3** triangles);
	virtual void unlockTriangles();

	virtual void addTriangle(PxU32 id, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2);
	virtual void addTriangles(const PxU32* ids, const PxVec3* triangleVertices, PxU32 numTriangles);

	virtual void removeTriangle(PxU32 id);
	virtual void removeTriangles(const PxU32* ids, PxU32 numTriangles);
	virtual void clearTriangles();

	virtual void release()
	{
		ClothingCollision::release();
	}

	virtual void setPose(PxMat44 pose);
	virtual const PxMat44& getPose() const
	{
		return mPose;
	}

	void update(const PxTransform& tm, const shdfnd::Array<PxVec3>& allTrianglesOld, shdfnd::Array<PxVec3>& allTrianglesOldTemp, shdfnd::Array<PxVec3>& allTriangles);

protected:
	void sortAddAndRemoves();

	PxMat44 mPose;
	Array<PxVec3> mTriangles;
	Array<PxU32> mIds;

	Array<PxU32> mRemoved;
	Array<ClothingTriangle> mAddedTriangles;

	Mutex mLock;
};


}
} // namespace apex
} // namespace physx


#endif // CLOTHING_COLLISION_H