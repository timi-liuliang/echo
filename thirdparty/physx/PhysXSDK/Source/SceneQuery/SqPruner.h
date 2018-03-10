/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef SQ_PRUNER_H
#define SQ_PRUNER_H

#include "GuSphere.h"
#include "GuBoxConversion.h"
#include "GuGeomUtilsInternal.h"
#include "GuConvexMesh.h"
#include "CmSPU.h"
#include "PxQueryReport.h" // for PxAgain
#include "GuSPUHelpers.h" // for GU_FETCH_CONVEX_DATA
#include "GuConvexUtilsInternal.h" // for computeOBBAroundConvex
#include "CmMemFetch.h"
#include "GuGeometryUnion.h"

namespace physx
{

using namespace Cm;

namespace Sq
{

typedef PxU32 PrunerHandle;
static const PrunerHandle INVALID_PRUNERHANDLE = 0xFFffFFff;
static const PxReal SQ_PRUNER_INFLATION = 1.01f; // pruner test shape inflation (not narrow phase shape)

class ShapeData
{
public:

	ShapeData(const PxGeometry& g, const PxTransform& t, PxReal inflation)
		: mWorldTransform(t), mOriginalPxGeom(g)
	{

		// a copy is needed not to destroy the input geometry
		// some types are converted to OBBs for queries
		mPrunerVolume.set(g);

		// BucketPruner uses AABB as cullBox
		PxBounds3 aabb;
		mPrunerVolume.computeBounds(aabb, mWorldTransform, inflation, NULL);
		mPrunerWorldRot33 = PxMat33(mWorldTransform.q); // precompute the 3x3 rotation matrix

		// grow the AABB by SQ_PRUNER_INFLATION factor for overlap/sweep/GJK accuracy
		mPrunerInflatedAABB = PxBounds3::centerExtents(aabb.getCenter(), aabb.getExtents()*SQ_PRUNER_INFLATION);
		
		PxGeometryType::Enum theType = g.getType();
		switch(theType)
		{
			case PxGeometryType::eBOX:
			{
				// AP: pruners are now responsible for growing the bounds
				// reasons:
				// 1. precision is pruner specific
				// 2. growing the bounds is cheap
				// 3. in the future when we eliminate Gu types we can hopefully avoid the shape duplication
				Gu::Box& box = reinterpret_cast<Gu::Box&>(mNPBox); // store the narrow phase Gu version copy
				box.center = mWorldTransform.p;
				box.extents = mPrunerVolume.get<PxBoxGeometry>().halfExtents;
				box.rot = mPrunerWorldRot33;
			}
			break;
			case PxGeometryType::eCAPSULE:
			{
				Gu::Capsule& dstWorldCapsule = reinterpret_cast<Gu::Capsule&>(mNPCapsule); // store a narrow phase version copy
				Gu::getCapsule(dstWorldCapsule, mPrunerVolume.get<PxCapsuleGeometry>(), mWorldTransform);

				// compute PxBoxGeometry pruner geom around input capsule geom; transform remains unchanged
				PxBoxGeometry newBoxPrunerVolume;
				Gu::computePxBoxAroundCapsule(static_cast<const PxCapsuleGeometry&>(g), newBoxPrunerVolume);
				mPrunerVolume.set(newBoxPrunerVolume);
			}
			break;
			case PxGeometryType::eCONVEXMESH:
			{
				const PxConvexMeshGeometry& convexGeom = mPrunerVolume.get<PxConvexMeshGeometryLL>();
				GU_FETCH_CONVEX_DATA(convexGeom); // needs to be done for SPU before we compute OBB around it
				Gu::Box prunerBox;
				computeOBBAroundConvex(prunerBox, convexGeom, cm, mWorldTransform);
				mPrunerWorldRot33 = prunerBox.rot;

				// AP: pruners are now responsible for growing the OBB by 1% for overlap/sweep/GJK accuracy
				mPrunerVolume.set(PxBoxGeometry(prunerBox.extents)); // AP scaffold looks like room for optimization here
				mWorldTransform = prunerBox.getTransform();
			}
			break;
			case PxGeometryType::eSPHERE:
			{
				// no need to grow the Sphere
				reinterpret_cast<Gu::Sphere &>(mNPSphere) = Gu::Sphere(mWorldTransform.p, mPrunerVolume.get<PxSphereGeometry>().radius);
			}
			break;
			case PxGeometryType::ePLANE:
			case PxGeometryType::eTRIANGLEMESH:
			case PxGeometryType::eHEIGHTFIELD:
			case PxGeometryType::eGEOMETRY_COUNT:
			case PxGeometryType::eINVALID:
			default:
				PX_ALWAYS_ASSERT_MESSAGE("PhysX internal error: Invalid shape in ShapeData contructor.");
		}
	}

#if 0
	ShapeData(const PxBounds3& aabb)
		:	mWorldTransform(aabb.getCenter()),
			mPrunerInflatedAABB(aabb)
	{
		mPrunerVolume.set(PxBoxGeometry(aabb.getExtents()));
	}
#endif
	
	PX_FORCE_INLINE const PxGeometry&	getPrunerGeometry()	const				{ return mPrunerVolume.getGeometry(); }
	PX_FORCE_INLINE const PxBoxGeometry&getPrunerBoxGeom() const				{ return mPrunerVolume.get<PxBoxGeometry>(); }

	PX_FORCE_INLINE const PxTransform&	getPrunerWorldTransform()	const		{ return mWorldTransform; }
	PX_FORCE_INLINE const PxBounds3&	getPrunerInflatedWorldAABB() const		{ return mPrunerInflatedAABB; }
	PX_FORCE_INLINE const PxMat33&		getPrunerWorldRot33() const				{ return mPrunerWorldRot33; }

	PX_FORCE_INLINE const PxCapsuleGeometry& getNPPxCapsule() const // NP for narrow phase
	{
		PX_ASSERT(mOriginalPxGeom.getType() == PxGeometryType::eCAPSULE);
		return static_cast<const PxCapsuleGeometry&>(mOriginalPxGeom);
	}
	PX_FORCE_INLINE const Gu::Sphere&	getNPGuSphere() const
	{
		PX_ASSERT(mOriginalPxGeom.getType() == PxGeometryType::eSPHERE);
		const Gu::Sphere& sphere = reinterpret_cast<const Gu::Sphere&>(mNPSphere);
		return sphere;
	}
	PX_FORCE_INLINE const Gu::Box&		getNPGuBox() const
	{
		PX_ASSERT(mOriginalPxGeom.getType() == PxGeometryType::eBOX);
		const Gu::Box& box = reinterpret_cast<const Gu::Box&>(mNPBox);
		return box;
	}
	PX_FORCE_INLINE const Gu::Capsule&	getNPGuCapsule() const
	{
		PX_ASSERT(mOriginalPxGeom.getType() == PxGeometryType::eCAPSULE);
		const Gu::Capsule& cap = reinterpret_cast<const Gu::Capsule&>(mNPCapsule);
		return cap;
	}

	const PxGeometry& getOriginalPxGeom() const { return mOriginalPxGeom; }

	PX_NOCOPY(ShapeData)
private:
	Gu::GeometryUnion	mPrunerVolume; // these Px shapes are used for pruners. This volume encloses but can differ from the original shape
	PxTransform			mWorldTransform; // unchanged copy of original transform (AP scaffold do we need a copy?) for the above Px shape
	PxMat33				mPrunerWorldRot33; // precomputed PxMat33 version of mWorldTransform.q only valid for pruner shape
	PxBounds3			mPrunerInflatedAABB; // precomputed AABB for the pruner shape
	const PxGeometry&	mOriginalPxGeom; // original geometry passed to the top level query by the user

	// these union Gu shapes are only precomputed for narrow phase (not pruners), can be different from mPrunerVolume
	// so need separate storage
	union
	{
		PxU8 mNPBox[sizeof(Gu::Box)];
		PxU8 mNPCapsule[sizeof(Gu::Capsule)];
		PxU8 mNPSphere[sizeof(Gu::Sphere)];
	};
};

struct PrunerPayload
{
	size_t data[2];

	PX_FORCE_INLINE	bool operator == (const PrunerPayload& other) const
	{
		return (data[0] == other.data[0]) && (data[1] == other.data[1]);
	}
};

struct PrunerCallback
{
	virtual PxAgain invoke(PxReal& distance, const PrunerPayload* payload, PxU32 nb=1) = 0;
    virtual ~PrunerCallback() {}
};

class Pruner : public Ps::UserAllocated
{
public:


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	\brief		Adds objects to the pruner.
	 *	\param		results		[out]	an array for resulting handles
	 *  \param		bounds		[in]	an array of bounds
	 *  \param		userData	[in]	an array of object data
	 *  \param		count		[in]	the number of objects in the arrays
	 *
	 *	\return		true if success, false if internal allocation failed. The first failing add results in a INVALID_PRUNERHANDLE.
	 *
	 *  Handles are usable as indices. Each handle is either be a recycled handle returned by the client via removeObjects(),
	 *  or a fresh handle that is either zero, or one greater than the last fresh handle returned.
	 *
	 *	Objects and bounds in the arrays have the same number of elements and ordering. 
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool						addObjects(PrunerHandle* results, const PxBounds3* bounds, const PrunerPayload* userData, PxU32 count = 1) = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Removes objects from the pruner.
	 *	\param		handles		[in]	the objects to remove
	 *  \param		count		[in]	the number of objects to remove
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void						removeObjects(const PrunerHandle* handles, PxU32 count = 1) = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Updates objects with new bounds.
	 *	\param		handles		[in]	the objects to update
	 *  \param		newBounds	[in]	updated bounds 
	 *  \param		count		[in]	the number of objects to update
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void						updateObjects(const PrunerHandle* handles, const PxBounds3* newBounds, PxU32 count = 1) = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Makes the queries consistent with previous changes.
	 *	This function must be called before starting queries on an updated Pruner and assert otherwise.
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void						commit() = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Query functions
	 *  
	 *	Note: return value may disappear if PrunerCallback contains the necessary information
	 *			currently it is still used for the dynamic pruner internally (to decide if added objects must be queried)
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual	PxAgain raycast(const PxVec3& origin, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const = 0;
	virtual	PxAgain	overlap(const ShapeData& queryVolume, PrunerCallback&) const = 0;
	virtual	PxAgain	sweep(const ShapeData& queryVolume, const PxVec3& unitDir, PxReal& inOutDistance, PrunerCallback&) const = 0;


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Retrieve the object data associated with the handle
	 *	
	 *	\param	handle		The handle returned by addObjects()
	 *
	 *	\return				A reference to the object data
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual const PrunerPayload&		getPayload(const PrunerHandle&) const = 0;




	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Preallocate space 
	 *	
	 *	\param	entries		the number of entries to preallocate space for
	 *
	 *	\return				A reference to the object data
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void						preallocate(PxU32 entries) = 0;


	// shift the origin of the pruner objects
	virtual void						shiftOrigin(const PxVec3& shift) = 0;

	virtual								~Pruner() {}

	// additional 'internal' interface		
	virtual	void						visualize(Cm::RenderOutput&, PxU32) const {}
};
}

}

#endif // SQ_PRUNER_H
