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

#include "NpSpatialIndex.h"
#include "PsFPU.h"
#include "SqAABBPruner.h"

namespace physx
{

NpSpatialIndex::NpSpatialIndex()
: mPendingUpdates(false)
{
	mPruner = PX_NEW(Sq::AABBPruner)(true);
}

NpSpatialIndex::~NpSpatialIndex()
{
	PX_DELETE(mPruner);
}

PxSpatialIndexItemId NpSpatialIndex::insert(PxSpatialIndexItem& item,
										    const PxBounds3& bounds)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN_VAL(bounds.isValid(),		"PxSpatialIndex::insert: bounds are not valid.", PX_SPATIAL_INDEX_INVALID_ITEM_ID);

	Sq::PrunerHandle output;
	Sq::PrunerPayload payload;
	payload.data[0] = reinterpret_cast<size_t>(&item);
	mPruner->addObjects(&output, &bounds, &payload);
	mPendingUpdates = true;
	return output;
}
	
void NpSpatialIndex::update(PxSpatialIndexItemId id,
							const PxBounds3& bounds)
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN(bounds.isValid(),		"PxSpatialIndex::update: bounds are not valid.");

	mPruner->updateObjects(&id, &bounds);
	mPendingUpdates = true;
}

void NpSpatialIndex::remove(PxSpatialIndexItemId id)
{
	PX_SIMD_GUARD;

	mPruner->removeObjects(&id);
	mPendingUpdates = true;
}

PxBounds3 NpSpatialIndex::getBounds(PxSpatialIndexItemId /*id*/) const
{
	return PxBounds3();
}

namespace
{
	struct OverlapCallback: public Sq::PrunerCallback
	{
		OverlapCallback(PxSpatialOverlapCallback& callback) : mUserCallback(callback) {}

		virtual PxAgain invoke(PxReal& /*distance*/, const Sq::PrunerPayload* userData, PxU32 /*nb*/)
		{
			PxSpatialIndexItem& item = *reinterpret_cast<PxSpatialIndexItem*>(userData->data[0]);
			return mUserCallback.onHit(item);
		}

		PxSpatialOverlapCallback &mUserCallback;
	private:
		OverlapCallback& operator=(const OverlapCallback&);
	};

	struct LocationCallback: public Sq::PrunerCallback
	{
		LocationCallback(PxSpatialLocationCallback& callback) : mUserCallback(callback) {}

		virtual PxAgain invoke(PxReal& distance, const Sq::PrunerPayload* userData, PxU32 /*nb*/)
		{
			PxReal oldDistance = distance, shrunkDistance = distance;
			PxSpatialIndexItem& item = *reinterpret_cast<PxSpatialIndexItem*>(userData->data[0]);
			PxAgain result = mUserCallback.onHit(item, distance, shrunkDistance);

			if(shrunkDistance>distance)
				Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "PxSpatialLocationCallback: distance may not be extended.");

			if(!result)
				return false;

			distance = PxMin(oldDistance, distance);
			return true;
		}

		PxSpatialLocationCallback& mUserCallback;

	private:
		LocationCallback& operator=(const LocationCallback&);
	};
}

void NpSpatialIndex::flushUpdates() const
{
	if(mPendingUpdates)
		mPruner->commit();
	mPendingUpdates = false;
}

void NpSpatialIndex::overlap(const PxBounds3& aabb, 
							 PxSpatialOverlapCallback& callback) const
{
	PX_SIMD_GUARD;
	PX_CHECK_AND_RETURN(aabb.isValid(),			"PxSpatialIndex::overlap: aabb is not valid.");

	flushUpdates();
	OverlapCallback cb(callback);
	PxBoxGeometry boxGeom(aabb.getExtents());
	PxTransform xf(aabb.getCenter());
	Sq::ShapeData shapeData(boxGeom, xf, 0.0f); // temporary rvalue not compatible with PX_NOCOPY 
	mPruner->overlap(shapeData, cb);
}

void NpSpatialIndex::raycast(const PxVec3& origin, 
							 const PxVec3& unitDir, 
							 PxReal maxDist, 
							 PxSpatialLocationCallback& callback) const
{
	PX_SIMD_GUARD;

	PX_CHECK_AND_RETURN(origin.isFinite(),								"PxSpatialIndex::raycast: origin is not valid.");
	PX_CHECK_AND_RETURN(unitDir.isFinite() && unitDir.isNormalized(),	"PxSpatialIndex::raycast: unitDir is not valid.");
	PX_CHECK_AND_RETURN(maxDist > 0.0f,									"PxSpatialIndex::raycast: distance must be positive");

	flushUpdates();
	LocationCallback cb(callback);
	mPruner->raycast(origin, unitDir, maxDist, cb);
}

void NpSpatialIndex::sweep(const PxBounds3& aabb, 
						   const PxVec3& unitDir, 
						   PxReal maxDist, 
						   PxSpatialLocationCallback& callback) const
{
	PX_SIMD_GUARD;

	PX_CHECK_AND_RETURN(aabb.isValid(),									"PxSpatialIndex::sweep: aabb is not valid.");
	PX_CHECK_AND_RETURN(unitDir.isFinite() && unitDir.isNormalized(),	"PxSpatialIndex::sweep: unitDir is not valid.");
	PX_CHECK_AND_RETURN(maxDist > 0.0f,									"PxSpatialIndex::sweep: distance must be positive");

	flushUpdates();
	LocationCallback cb(callback);
	PxBoxGeometry boxGeom(aabb.getExtents());
	PxTransform xf(aabb.getCenter());
	Sq::ShapeData shapeData(boxGeom, xf, 0.0f); // temporary rvalue not compatible with PX_NOCOPY 
	mPruner->sweep(shapeData, unitDir, maxDist, cb);
}


void NpSpatialIndex::rebuildFull()
{
	PX_SIMD_GUARD;

	mPruner->purge();
	mPruner->commit();
	mPendingUpdates = false;
}

void NpSpatialIndex::setIncrementalRebuildRate(PxU32 rate)
{
	mPruner->setRebuildRateHint(rate);
}

void NpSpatialIndex::rebuildStep()
{
	PX_SIMD_GUARD;
	mPruner->buildStep();
	mPendingUpdates = true;
}

void NpSpatialIndex::release()
{
	delete this;
}


PxSpatialIndex* PxCreateSpatialIndex()
{
	return PX_NEW(NpSpatialIndex)();
}

}
