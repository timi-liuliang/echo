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

#ifndef SQ_PRUNINGPOOL_H
#define SQ_PRUNINGPOOL_H

#include "SqPruner.h"

namespace physx
{
namespace Sq
{
	// This class is designed to maintain a two way mapping between pair(PrunerPayload,AABB) and PrunerHandle
	// Internally there's also an index for handles (AP: can be simplified?)
	// This class effectively stores bounded pruner payloads, returns a PrunerHandle and allows O(1)
	// access to them using a PrunerHandle
	// Supported operations are add, remove, update bounds
	class PruningPool 
	{

		public:

							PruningPool();
#ifdef __SPU__
			PX_FORCE_INLINE ~PruningPool() {}
#else
			virtual			~PruningPool();
#endif

			virtual	const PrunerPayload&	getPayload(const PrunerHandle& h) const { return mObjects[getIndex(h)]; }	
			virtual void					shiftOrigin(const PxVec3& shift);

					PrunerHandle			addObject(const PxBounds3& worldAABB, const PrunerPayload& payload);

					// this function will swap the last object with the hole formed by removed PrunerHandle object
					// and return the removed last object's index in the pool
					PxU32					removeObject(PrunerHandle h);
				PX_FORCE_INLINE void		updateObject(PrunerHandle h, const PxBounds3& worldAABB)
											{
												mWorldBoxes[getIndex(h)] = worldAABB;
											}

		// Data access
		PX_FORCE_INLINE	PxU32				getIndex(PrunerHandle h)const	{ return mHandleToIndex[h];	}
		PX_FORCE_INLINE	PrunerPayload*		getObjects()			const	{ return mObjects;		}
		PX_FORCE_INLINE	PxU32				getNbActiveObjects()	const	{ return mNbObjects;	}
		PX_FORCE_INLINE	const PxBounds3*	getCurrentWorldBoxes()	const	{ return mWorldBoxes;	}
		PX_FORCE_INLINE	PxBounds3*			getCurrentWorldBoxes()			{ return mWorldBoxes;	}

		PX_FORCE_INLINE	const PxBounds3&	getWorldAABB(PrunerHandle h) const
											{
												return getWorldAABBbyIndex(getIndex(h));
											}
						void				preallocate(PxU32 entries);
	protected:
		PX_FORCE_INLINE	const PxBounds3&	getWorldAABBbyIndex(PxU32 index) const
											{
												// return cached box
												#ifdef __SPU__
													static PxU8 PX_ALIGN(16,worldBoxBuffer[(sizeof(PxBounds3)+31)&~15]);
													PxBounds3* worldBox = Cm::memFetchAsync<PxBounds3>(worldBoxBuffer,(Cm::MemFetchPtr)&mWorldBoxes[index],sizeof(PxBounds3),1);
													Cm::memFetchWait(1);
													return *worldBox;
												#else
													return mWorldBoxes[index];
												#endif
											}


				PxU32						mNbObjects;		//!< Current number of objects
				PxU32						mMaxNbObjects;	//!< Max. number of objects (capacity for mWorldBoxes, mObjects)

				//!< these arrays are parallel
				PxBounds3*					mWorldBoxes;	//!< List of world boxes, stores mNbObjects, capacity=mMaxNbObjects
				PrunerPayload*				mObjects;		//!< List of objects, stores mNbObjects, capacity=mMaxNbObjects

	private:			
				PxU32*						mHandleToIndex;	//!< Maps from PrunerHandle to internal index (payload index in mObjects)
				PxU32*						mIndexToHandle; //!< Inverse map from objectIndex to PrunerHandle
				PxU32						mFirstFreshHandle; //!< Maximum handle index allocated so far

				// this is an index, not a handle. this is the head of a list of holes formed in mHandleToIndex
				// by removed handles
				// the rest of the list is stored in holes in mHandleToIndex (in place)
				PxU32						mHandleFreeList;

				void						resize(PxU32 newCapacity);


	};

} // namespace Sq

}

#endif // SQ_PRUNINGPOOL_H
