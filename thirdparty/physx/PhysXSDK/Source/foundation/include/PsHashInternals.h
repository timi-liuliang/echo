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


#ifndef PX_FOUNDATION_PSHASHINTERNALS_H
#define PX_FOUNDATION_PSHASHINTERNALS_H

#include "PsBasicTemplates.h"
#include "PsArray.h"
#include "PsBitUtils.h"
#include "PsHash.h"
#include "foundation/PxMemory.h"

#ifdef PX_VC
	#pragma warning(push)
	#pragma warning(disable:4127) // conditional expression is constant
#endif
namespace physx
{
namespace shdfnd
{
	namespace internal
	{
		template <class Entry,
				  class Key,
				  class HashFn,
				  class GetKey,
				  class Allocator,
				  bool compacting>
		class HashBase : private Allocator
		{
			void init(PxU32 initialTableSize, float loadFactor)
			{
				mBuffer = NULL;
				mEntries = NULL;
				mEntriesNext = NULL;
				mHash = NULL;
				mEntriesCapacity = 0;
				mHashSize = 0;
				mLoadFactor = loadFactor;
				mFreeList = (PxU32)EOL;
				mTimestamp = 0;
				mEntriesCount = 0;

				if(initialTableSize)
					reserveInternal(initialTableSize);
			}

		public:
			typedef Entry EntryType;

			HashBase(PxU32 initialTableSize = 64, float loadFactor = 0.75f)
			:	Allocator(PX_DEBUG_EXP("hashBase"))
			{
				init(initialTableSize, loadFactor);
			}

			HashBase(PxU32 initialTableSize, float loadFactor, const Allocator& alloc)
			:	Allocator(alloc)
			{
				init(initialTableSize, loadFactor);
			}

			HashBase(const Allocator& alloc)
			:	Allocator(alloc)
			{
				init(64, 0.75f);
			}

			~HashBase()
			{
				destroy(); //No need to clear()

				if(mBuffer)
					Allocator::deallocate(mBuffer);
			}

			static const PxU32 EOL = 0xffffffff;

			PX_INLINE Entry* create(const Key& k, bool& exists)
			{
				PxU32 h=0;
				if(mHashSize)
				{
					h = hash(k);
					PxU32 index = mHash[h];
					while(index!=EOL && !HashFn()(GetKey()(mEntries[index]), k))
						index = mEntriesNext[index];
					exists = index!=EOL;
					if(exists)
						return mEntries + index;
				} else
					exists = false;

				if(freeListEmpty())
				{
					grow();
					h = hash(k);
				}

				PxU32 entryIndex = freeListGetNext();

				mEntriesNext[entryIndex] = mHash[h];
				mHash[h] = entryIndex;

				mEntriesCount++;
				mTimestamp++;

				return mEntries + entryIndex;
			}

			PX_INLINE const Entry* find(const Key& k) const
			{
				if(!mHashSize)
					return NULL;

				PxU32 h = hash(k);
				PxU32 index = mHash[h];
				while(index!=EOL && !HashFn()(GetKey()(mEntries[index]), k))
					index = mEntriesNext[index];
				return index != EOL ? mEntries + index : 0;
			}

			PX_INLINE bool erase(const Key& k)
			{
				if(!mHashSize)
					return false;

				PxU32 h = hash(k);
				PxU32* ptr = mHash + h;
				while(*ptr!=EOL && !HashFn()(GetKey()(mEntries[*ptr]), k))
					ptr = mEntriesNext + *ptr;

				if(*ptr == EOL)
					return false;

				PxU32 index = *ptr;
				*ptr = mEntriesNext[index];

				mEntries[index].~Entry();

				mEntriesCount--;
				mTimestamp++;

				if(compacting && index!=mEntriesCount)
					replaceWithLast(index);

				freeListAdd(index);

				return true;
			}

			PX_INLINE PxU32 size() const
			{ 
				return mEntriesCount; 
			}

			PX_INLINE PxU32 capacity() const
			{ 
				return mHashSize; 
			}		
			
			void clear()
			{
				if(!mHashSize || mEntriesCount == 0)
					return;

				destroy();

				PxMemSet(mHash, EOL, mHashSize * sizeof(PxU32));

				const PxU32 sizeMinus1 = mEntriesCapacity - 1;
				for(PxU32 i = 0;i<sizeMinus1;i++)
				{
					prefetchLine(mEntriesNext + i, 128);
					mEntriesNext[i] = i+1;
				}
				mEntriesNext[mEntriesCapacity-1] = (PxU32)EOL;
				mFreeList = 0;
				mEntriesCount = 0;
			}

			void reserve(PxU32 size)
			{
				if(size>mHashSize)
					reserveInternal(size);
			}

			PX_INLINE const Entry* getEntries() const
			{
				return mEntries;
			}

			PX_INLINE Entry* insertUnique(const Key& k)
			{
				PX_ASSERT(find(k) == NULL);
				PxU32 h = hash(k);
				
				PxU32 entryIndex = freeListGetNext();

				mEntriesNext[entryIndex] = mHash[h];
				mHash[h] = entryIndex;

				mEntriesCount++;
				mTimestamp++;

				return mEntries + entryIndex;
			}

		private:

			void destroy()
			{
				for(PxU32 i = 0;i<mHashSize;i++)
				{				
					for(PxU32 j = mHash[i]; j != EOL; j = mEntriesNext[j])
						mEntries[j].~Entry();
				}
			}

			template <typename HK, typename GK, class A, bool comp> 
			PX_NOINLINE void copy(const HashBase<Entry,Key,HK,GK,A,comp>& other);
			
			// free list management - if we're coalescing, then we use mFreeList to hold
			// the top of the free list and it should always be equal to size(). Otherwise,
			// we build a free list in the next() pointers.

			PX_INLINE void freeListAdd(PxU32 index)
			{
				if(compacting)
				{
					mFreeList--;
					PX_ASSERT(mFreeList == mEntriesCount);
				}
				else
				{
					mEntriesNext[index] = mFreeList;
					mFreeList = index;
				}
			}

			PX_INLINE void freeListAdd(PxU32 start, PxU32 end)
			{
				if(!compacting)
				{
					for(PxU32 i = start; i<end-1; i++)	// add the new entries to the free list
						mEntriesNext[i] = i+1;

					//link in old free list
					mEntriesNext[end-1] = mFreeList;
					PX_ASSERT(mFreeList != end-1);
					mFreeList = start;
				}
				else if(mFreeList==EOL)					// don't reset the free ptr for the compacting hash unless it's empty
					mFreeList = start;
			}

			PX_INLINE PxU32 freeListGetNext()
			{
				PX_ASSERT(!freeListEmpty());
				if(compacting)
				{
					PX_ASSERT(mFreeList == mEntriesCount);
					return mFreeList++;
				}
				else
				{
					PxU32 entryIndex = mFreeList;
					mFreeList = mEntriesNext[mFreeList];
					return entryIndex;
				}
			}

			PX_INLINE bool freeListEmpty()	const
			{
				if(compacting)
					return mEntriesCount == mEntriesCapacity;
				else
					return mFreeList == EOL;
			}

			PX_INLINE void replaceWithLast(PxU32 index)
			{
				PX_PLACEMENT_NEW(mEntries + index, Entry)(mEntries[mEntriesCount]);
				mEntries[mEntriesCount].~Entry();
				mEntriesNext[index] = mEntriesNext[mEntriesCount];

				PxU32 h = hash(GetKey()(mEntries[index]));
				PxU32* ptr;
				for(ptr = mHash + h; *ptr!=mEntriesCount; ptr = mEntriesNext + *ptr)
					PX_ASSERT(*ptr!=EOL);
				*ptr = index;
			}

			PX_INLINE PxU32 hash(const Key& k, PxU32 hashSize) const
			{
				return HashFn()(k)&(hashSize-1);
			}

			PX_INLINE PxU32 hash(const Key& k) const
			{
				return hash(k, mHashSize);
			}

			void reserveInternal(PxU32 size)
			{
				if(!isPowerOfTwo(size))
					size = nextPowerOfTwo(size);

				PX_ASSERT(!(size&(size-1)));
				
				// decide whether iteration can be done on the entries directly
				bool resizeCompact = compacting || freeListEmpty();

				// define new table sizes
				PxU32 oldEntriesCapacity = mEntriesCapacity;
				PxU32 newEntriesCapacity = PxU32(float(size)*mLoadFactor);
				PxU32 newHashSize = size;
				
				// allocate new common buffer and setup pointers to new tables
				PxU8* newBuffer;
				PxU32* newHash;
				PxU32* newEntriesNext;
				Entry* newEntries;
				{
					PxU32 newHashByteOffset = 0;
					PxU32 newEntriesNextBytesOffset = newHashByteOffset + newHashSize*sizeof(PxU32);
					PxU32 newEntriesByteOffset = newEntriesNextBytesOffset + newEntriesCapacity*sizeof(PxU32);
					newEntriesByteOffset += (16 - (newEntriesByteOffset & 15)) & 15;
					PxU32 newBufferByteSize = newEntriesByteOffset + newEntriesCapacity*sizeof(Entry);
					
					newBuffer = (PxU8*)Allocator::allocate(newBufferByteSize, __FILE__, __LINE__);
					PX_ASSERT(newBuffer);

					newHash = (PxU32*)(newBuffer + newHashByteOffset);
					newEntriesNext = (PxU32*)(newBuffer + newEntriesNextBytesOffset);
					newEntries = (Entry*)(newBuffer + newEntriesByteOffset);
				}

				// initialize new hash table
				PxMemSet(newHash, PxU32(EOL), newHashSize * sizeof(PxU32));
				
				// iterate over old entries, re-hash and create new entries
				if (resizeCompact)
				{
					// check that old free list is empty - we don't need to copy the next entries
					PX_ASSERT(compacting || mFreeList == EOL);

					for(PxU32 index=0; index<mEntriesCount; ++index)
					{
						PxU32 h = hash(GetKey()(mEntries[index]), newHashSize);
						newEntriesNext[index] = newHash[h];
						newHash[h] = index;

						PX_PLACEMENT_NEW(newEntries+index, Entry)(mEntries[index]);
						mEntries[index].~Entry();
					}
				}
				else
				{
					// copy old free list, only required for non compact resizing
					PxMemCopy(newEntriesNext, mEntriesNext, mEntriesCapacity*sizeof(PxU32));
					
					for (PxU32 bucket = 0; bucket < mHashSize; bucket++)
					{
						PxU32 index = mHash[bucket];
						while (index != EOL)
						{
							PxU32 h = hash(GetKey()(mEntries[index]), newHashSize);
							newEntriesNext[index] = newHash[h];
							PX_ASSERT(index != newHash[h]);

							newHash[h] = index;

							PX_PLACEMENT_NEW(newEntries+index, Entry)(mEntries[index]);
							mEntries[index].~Entry();

							index = mEntriesNext[index];
						}
					}
				}

				//swap buffer and pointers
				Allocator::deallocate(mBuffer);
				mBuffer = newBuffer;
				mHash = newHash;
				mHashSize = newHashSize;
				mEntriesNext = newEntriesNext;
				mEntries = newEntries;
				mEntriesCapacity = newEntriesCapacity;

				freeListAdd(oldEntriesCapacity, newEntriesCapacity);
			}

			void grow()
			{
				PX_ASSERT((mFreeList == EOL) || (compacting && (mEntriesCount == mEntriesCapacity)));

				PxU32 size = mHashSize==0 ? 16 : mHashSize*2;
				reserve(size);
			}

			PxU8*					mBuffer;
			Entry*					mEntries; 
			PxU32*					mEntriesNext;	// same size as mEntries
			PxU32*					mHash;
			PxU32					mEntriesCapacity;
			PxU32					mHashSize;
			float					mLoadFactor;
			PxU32					mFreeList;
			PxU32					mTimestamp;
			PxU32					mEntriesCount;			// number of entries

		public:

			class Iter
			{
			public:
				PX_INLINE Iter(HashBase& b): mBucket(0), mEntry((PxU32)b.EOL), mTimestamp(b.mTimestamp), mBase(b)
				{
					if(mBase.mEntriesCapacity>0)
					{
						mEntry = mBase.mHash[0];
						skip();
					}
				}

				PX_INLINE void check() const		{ PX_ASSERT(mTimestamp == mBase.mTimestamp);	}
				PX_INLINE Entry operator*()	const	{ check(); return mBase.mEntries[mEntry];		}
				PX_INLINE Entry* operator->() const	{ check(); return mBase.mEntries + mEntry;		}
				PX_INLINE Iter operator++()			{ check(); advance(); return *this;				}
				PX_INLINE Iter operator++(int)		{ check(); Iter i = *this; advance(); return i;	}
				PX_INLINE bool done() const			{ check(); return mEntry == mBase.EOL;			}

			private:
				PX_INLINE void advance()			{ mEntry = mBase.mEntriesNext[mEntry]; skip();		    }
				PX_INLINE void skip()
				{
					while(mEntry==mBase.EOL) 
					{ 
						if(++mBucket == mBase.mHashSize)
							break;
						mEntry = mBase.mHash[mBucket];
					}
				}

				Iter& operator=(const Iter&);


				PxU32 mBucket;
				PxU32 mEntry;
				PxU32 mTimestamp;
				HashBase &mBase;
			};
		};
		
		template <class Entry,
				  class Key,
				  class HashFn,
				  class GetKey,
				  class Allocator,
				  bool compacting>
		template <typename HK, typename GK, class A, bool comp> 
		PX_NOINLINE void HashBase<Entry,Key,HashFn,GetKey,Allocator,compacting>::copy(const HashBase<Entry,Key,HK,GK,A,comp>& other)
		{
			reserve(other.mEntriesCount);

			for(PxU32 i = 0;i < other.mEntriesCount;i++)
			{
				for(PxU32 j = other.mHash[i]; j != EOL; j = other.mEntriesNext[j])
				{
					const Entry &otherEntry = other.mEntries[j];

					bool exists;
					Entry *newEntry = create(GK()(otherEntry), exists);
					PX_ASSERT(!exists);

					PX_PLACEMENT_NEW(newEntry, Entry)(otherEntry);
				}
			}
		}

		template <class Key, 
				  class HashFn, 
				  class Allocator = Allocator,
				  bool Coalesced = false>
		class HashSetBase
		{
			PX_NOCOPY(HashSetBase)
		public:
			struct GetKey { PX_INLINE const Key& operator()(const Key& e) {	return e; }	};

			typedef HashBase<Key, Key, HashFn, GetKey, Allocator, Coalesced> BaseMap;
			typedef typename BaseMap::Iter Iterator;

			HashSetBase(PxU32 initialTableSize, 
						float loadFactor,
						const Allocator& alloc):	mBase(initialTableSize,loadFactor,alloc)	{}

			HashSetBase(const Allocator& alloc):	mBase(64,0.75f,alloc)	{}

			HashSetBase(PxU32 initialTableSize = 64,
						float loadFactor = 0.75f):	mBase(initialTableSize,loadFactor)	{}

			bool insert(const Key& k)
			{
				bool exists;
				Key* e = mBase.create(k,exists);
				if(!exists)
					PX_PLACEMENT_NEW(e, Key)(k);
				return !exists;
			}

			PX_INLINE bool		contains(const Key& k)	const	{	return mBase.find(k)!=0;		}
			PX_INLINE bool		erase(const Key& k)				{	return mBase.erase(k);			}
			PX_INLINE PxU32		size()					const	{	return mBase.size();			}
			PX_INLINE PxU32		capacity()				const	{	return mBase.capacity();		}
			PX_INLINE void		reserve(PxU32 size)				{	mBase.reserve(size);			}
			PX_INLINE void		clear()							{	mBase.clear();					}
		protected:
			BaseMap mBase;

		};

		template <class Key, 
			  class Value,
			  class HashFn, 
			  class Allocator = Allocator >

		class HashMapBase
		{
			PX_NOCOPY(HashMapBase)
		public:
			typedef Pair<const Key,Value> Entry;

			struct GetKey 
			{ 
				PX_INLINE const Key& operator()(const Entry& e) 
				{ 
					return e.first; 
				}	
			};

			typedef HashBase<Entry, Key, HashFn, GetKey, Allocator, true> BaseMap;
			typedef typename BaseMap::Iter Iterator;

			HashMapBase(PxU32 initialTableSize, float loadFactor, const Allocator& alloc):	mBase(initialTableSize,loadFactor,alloc)	{}

			HashMapBase(const Allocator &alloc):	mBase(64,0.75f,alloc)	{}

			HashMapBase(PxU32 initialTableSize = 64, float loadFactor = 0.75f):	mBase(initialTableSize,loadFactor)	{}

			bool insert(const Key/*&*/ k, const Value/*&*/ v)
			{
				bool exists;
				Entry* e = mBase.create(k,exists);
				if(!exists)
					PX_PLACEMENT_NEW(e, Entry)(k,v);
				return !exists;
			}

			Value& operator [](const Key& k)
			{
				bool exists;
				Entry* e = mBase.create(k, exists);
				if(!exists)
					PX_PLACEMENT_NEW(e, Entry)(k,Value());
		
				return e->second;
			}

			PX_INLINE const Entry*	find(const Key& k)		const	{	return mBase.find(k);			}
			PX_INLINE bool			erase(const Key& k)				{	return mBase.erase(k);			}
			PX_INLINE PxU32			size()					const	{	return mBase.size();			}
			PX_INLINE PxU32			capacity()				const	{	return mBase.capacity();		}
			PX_INLINE Iterator		getIterator()					{	return Iterator(mBase);			}
			PX_INLINE void			reserve(PxU32 size)				{	mBase.reserve(size);			}
			PX_INLINE void			clear()							{	mBase.clear();					}

		protected:
			BaseMap mBase;
		};

	}

} // namespace shdfnd
} // namespace physx

#ifdef PX_VC
	#pragma warning(pop)
#endif
#endif
