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


#ifndef PX_PHYSICS_CM_COLLECTION
#define PX_PHYSICS_CM_COLLECTION

#include "CmPhysXCommon.h"
#include "PxCollection.h"
#include "PsHashMap.h"
#include "PsHashSet.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Cm
{	
	template <class Key, 
			  class Value,
			  class HashFn = Ps::Hash<Key>, 
			  class Allocator = Ps::Allocator >
	class CollectionHashMap : public Ps::CoalescedHashMap< Key, Value, HashFn, Allocator>
	{
		typedef Ps::internal::HashMapBase< Key, Value, HashFn, Allocator> MapBase;	
		typedef Ps::Pair<const Key,Value> EntryData;

		public:
			CollectionHashMap(PxU32 initialTableSize = 64, float loadFactor = 0.75f):
			    Ps::CoalescedHashMap< Key, Value, HashFn, Allocator>(initialTableSize,loadFactor) {}

			void insertUnique(const Key& k, const Value& v)
			{
				PX_PLACEMENT_NEW(MapBase::mBase.insertUnique(k), EntryData)(k,v);
			}
	};

	
	
	class Collection : public PxCollection, public Ps::UserAllocated
	{
	public:
		typedef CollectionHashMap<PxBase*, PxSerialObjectId> ObjectToIdMap;
		typedef CollectionHashMap<PxSerialObjectId, PxBase*> IdToObjectMap;
					
		virtual void						add(PxBase& object, PxSerialObjectId ref);
		virtual	void						remove(PxBase& object);	
		virtual bool						contains(PxBase& object) const;
		virtual void						addId(PxBase& object, PxSerialObjectId id);
		virtual void						removeId(PxSerialObjectId id);
		virtual PxBase*						find(PxSerialObjectId ref) const;
		virtual void						add(PxCollection& collection);
		virtual void						remove(PxCollection& collection);		
		virtual	PxU32						getNbObjects() const;
		virtual PxBase&						getObject(PxU32 index) const;
		virtual	PxU32						getObjects(PxBase** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

		virtual PxU32						getNbIds() const;		
		virtual PxSerialObjectId			getId(const PxBase& object) const;
		virtual	PxU32						getIds(PxSerialObjectId* userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

		void								release() { PX_DELETE(this); }


		// Only for internal use. Bypasses virtual calls, specialized behaviour.
		PX_INLINE	void		            internalAdd(PxBase* s, PxSerialObjectId id = PX_SERIAL_OBJECT_ID_INVALID)				{ mObjects.insertUnique(s, id);	                   }
		PX_INLINE	PxU32		            internalGetNbObjects()		 const	{ return mObjects.size();								               }
		PX_INLINE	PxBase*		            internalGetObject(PxU32 i)	 const	{ PX_ASSERT(i<mObjects.size());	return mObjects.getEntries()[i].first; }
		PX_INLINE	const ObjectToIdMap::Entry*	internalGetObjects() const  { return mObjects.getEntries(); 			                           }
			
		IdToObjectMap					    mIds;
		ObjectToIdMap                       mObjects;
		
	};
}
}

#endif
