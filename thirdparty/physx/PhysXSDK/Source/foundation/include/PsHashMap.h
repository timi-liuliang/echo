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


#ifndef PX_FOUNDATION_PSHASHMAP_H
#define PX_FOUNDATION_PSHASHMAP_H

#include "PsHashInternals.h"

// TODO: make this doxy-format
//
// This header defines two hash maps. Hash maps
// * support custom initial table sizes (rounded up internally to power-of-2)
// * support custom static allocator objects
// * auto-resize, based on a load factor (i.e. a 64-entry .75 load factor hash will resize 
//                                        when the 49th element is inserted)
// * are based on open hashing
// * have O(1) contains, erase
//
// Maps have STL-like copying semantics, and properly initialize and destruct copies of objects
// 
// There are two forms of map: coalesced and uncoalesced. Coalesced maps keep the entries in the
// initial segment of an array, so are fast to iterate over; however deletion is approximately
// twice as expensive.
//
// HashMap<T>:
//		bool			insert(const Key& k, const Value& v)	O(1) amortized (exponential resize policy)
//		Value &			operator[](const Key& k)				O(1) for existing objects, else O(1) amortized
//		const Entry *	find(const Key& k);						O(1)
//		bool			erase(const T& k);						O(1)
//		PxU32			size();									constant
//		void			reserve(PxU32 size);					O(MAX(currentOccupancy,size))
//		void			clear();								O(currentOccupancy) (with zero constant for objects without destructors) 
//      Iterator		getIterator();
//
// operator[] creates an entry if one does not exist, initializing with the default constructor.
// CoalescedHashMap<T> does not support getInterator, but instead supports
// 		const Key *getEntries();
//
// Use of iterators:
// 
// for(HashMap::Iterator iter = test.getIterator(); !iter.done(); ++iter)
//			myFunction(iter->first, iter->second);

namespace physx
{
namespace shdfnd
{
	template <class Key,
			  class Value,
			  class HashFn = Hash<Key>,
			  class Allocator = Allocator >
	class HashMap: public internal::HashMapBase<Key, Value, HashFn, Allocator>
	{
	public:

		typedef internal::HashMapBase<Key, Value, HashFn, Allocator> HashMapBase;
		typedef typename HashMapBase::Iterator Iterator;

		HashMap(PxU32 initialTableSize = 64, float loadFactor = 0.75f):	HashMapBase(initialTableSize,loadFactor) {}
		HashMap(PxU32 initialTableSize, float loadFactor, const Allocator& alloc): HashMapBase(initialTableSize,loadFactor,alloc) {}
		HashMap(const Allocator& alloc): HashMapBase(64,0.75f,alloc) {}
		Iterator getIterator() { return Iterator(HashMapBase::mBase); }
	};

	template <class Key, 
			  class Value,
			  class HashFn = Hash<Key>, 
			  class Allocator = Allocator >
	class CoalescedHashMap: public internal::HashMapBase<Key, Value, HashFn, Allocator>
	{
	public:
		typedef internal::HashMapBase<Key, Value, HashFn, Allocator> HashMapBase;

		CoalescedHashMap(PxU32 initialTableSize = 64, float loadFactor = 0.75f): HashMapBase(initialTableSize,loadFactor){}
		const Pair<const Key,Value>* getEntries() const { return HashMapBase::mBase.getEntries(); }
	};

} // namespace shdfnd
} // namespace physx

#endif
