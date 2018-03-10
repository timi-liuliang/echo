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


#ifndef NP_SPATIALINDEX
#define NP_SPATIALINDEX

#include "PxSpatialIndex.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"

namespace physx
{
namespace Sq
{
	class AABBPruner;
}

class NpSpatialIndex: public PxSpatialIndex, public Ps::UserAllocated
{
public:
								NpSpatialIndex();
								~NpSpatialIndex();

	virtual	PxSpatialIndexItemId	insert(PxSpatialIndexItem& item,
										   const PxBounds3& bounds);

	virtual	void					update(PxSpatialIndexItemId id,
										   const PxBounds3& bounds);

	virtual	void					remove(PxSpatialIndexItemId id);

	virtual PxBounds3				getBounds(PxSpatialIndexItemId id)				const;

	virtual void					overlap(const PxBounds3& aabb,
											PxSpatialOverlapCallback& callback)		const;

	virtual void					raycast(const PxVec3& origin, 
											const PxVec3& unitDir, 
											PxReal maxDist, 
											PxSpatialLocationCallback& callback)	const;

	virtual	void					sweep(const PxBounds3& aabb, 
										  const PxVec3& unitDir, 
										  PxReal maxDist, 
										  PxSpatialLocationCallback& callback)		const;

	virtual void					flush()	{ flushUpdates(); }
	virtual void					rebuildFull();
	virtual void					setIncrementalRebuildRate(PxU32 rate);
	virtual void					rebuildStep();
	virtual void					release();
private:
	
	// const so that we can call it from const methods
	void							flushUpdates() const;

	mutable bool			mPendingUpdates;
	Sq::AABBPruner*		mPruner;
};


}
#endif


