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


#ifndef PX_PHYSICS_SC_OBJECT_ID_TRACKER
#define PX_PHYSICS_SC_OBJECT_ID_TRACKER

#include "CmPhysXCommon.h"
#include "CmIDPool.h"
#include "CmBitMap.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Sc
{

	class ObjectIDTracker : public Ps::UserAllocated
	{
		PX_NOCOPY(ObjectIDTracker)
	public:
		ObjectIDTracker() : mPendingReleasedIDs(PX_DEBUG_EXP("objectIDTrackerIDs")) {}

		PX_INLINE PxU32			createID()						{ return mIDPool.getNewID();								}
		PX_INLINE void			releaseID(PxU32 id)				
		{ 
			markIDAsDeleted(id); 
			mPendingReleasedIDs.pushBack(id);	
		}
		PX_INLINE Ps::IntBool	isDeletedID(PxU32 id)	const	{ return mDeletedIDsMap.boundedTest(id);					}
		PX_FORCE_INLINE PxU32	getDeletedIDCount()		const	{ return mPendingReleasedIDs.size();						}
		PX_INLINE void			clearDeletedIDMap()				{ mDeletedIDsMap.clear();									}
		PX_INLINE void			resizeDeletedIDMap(PxU32 id, PxU32 numIds)	
		{ 
			mDeletedIDsMap.resize(id); 
			mPendingReleasedIDs.reserve(numIds);
		}
		PX_INLINE void			processPendingReleases()
		{
			for(PxU32 i=0; i < mPendingReleasedIDs.size(); i++)
			{
				 mIDPool.freeID(mPendingReleasedIDs[i]);
			}
			mPendingReleasedIDs.clear();
		}
		PX_INLINE void reset()
		{
			processPendingReleases();
			mPendingReleasedIDs.reset();

			// Don't free stuff in IDPool, we still need the list of free IDs

			// And it does not seem worth freeing the memory of the bitmap
		}

	private:
		PX_INLINE void markIDAsDeleted(PxU32 id) { PX_ASSERT(!isDeletedID(id)); mDeletedIDsMap.growAndSet(id); }


	private:
		Cm::IDPool					mIDPool;
		Cm::BitMap					mDeletedIDsMap;
		Ps::Array<PxU32>			mPendingReleasedIDs;  // Buffer for released IDs to make sure newly created objects do not re-use these IDs immediately
	};

}
}

#endif
