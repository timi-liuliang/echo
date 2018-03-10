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


#ifndef PXS_THRESHOLDTABLE_H
#define PXS_THRESHOLDTABLE_H

#include "PxcThresholdStreamElement.h"

namespace physx
{

class PxsRigidBody;

class PxsThresholdStream : public Ps::Array<PxcThresholdStreamElement>
{
public:

	PxsThresholdStream(): Ps::Array<PxcThresholdStreamElement>(PX_DEBUG_EXP("ThresholdStream")) {}

	void append(PxsThresholdStream& other)
	{
		PxU32 s = size(), os = other.size();
		reserve((s + os + 63) & ~63);
		resize(s + os);
		PxMemCopy(begin()+s, other.begin(), os*sizeof(PxcThresholdStreamElement));
	}
};

class PxsThresholdTable
{
public:

	PxsThresholdTable()
		:	mBuffer(NULL),
			mHash(NULL),
			mHashSize(0),
			mHashCapactiy(0),
			mPairs(NULL),
			mNexts(NULL),
			mPairsSize(0),
			mPairsCapacity(0)
	{
	}

	~PxsThresholdTable()
	{
		if(mBuffer) PX_FREE(mBuffer);
	}

	void build(const PxsThresholdStream& stream)
	{
		//Handle the case of an empty stream.
		if(0==stream.size())
		{
			mPairsSize=0;
			mPairsCapacity=0;
			mHashSize=0;
			mHashCapactiy=0;
			if(mBuffer) PX_FREE(mBuffer);
			mBuffer = NULL;
			return;
		}

		//Realloc/resize if necessary.
		const PxU32 pairsCapacity = stream.size();
		const PxU32 hashCapacity = pairsCapacity*2+1;
		if((pairsCapacity > mPairsCapacity) || (pairsCapacity < (mPairsCapacity >> 2)))
		{
			if(mBuffer) PX_FREE(mBuffer);
			const PxU32 pairsByteSize = sizeof(Pair)*pairsCapacity;
			const PxU32 nextsByteSize = sizeof(PxU32)*pairsCapacity;
			const PxU32 hashByteSize = sizeof(PxU32)*hashCapacity;
			const PxU32 totalByteSize = pairsByteSize + nextsByteSize + hashByteSize;
			mBuffer = (PxU8*)PX_ALLOC(totalByteSize, PX_DEBUG_EXP("PxThresholdStream"));

			PxU32 offset = 0;
			mPairs = (Pair*)(mBuffer + offset);
			offset += pairsByteSize;
			mNexts = (PxU32*)(mBuffer + offset);
			offset += nextsByteSize;
			mHash = (PxU32*)(mBuffer + offset);
			offset += hashByteSize;
			PX_ASSERT(totalByteSize == offset);

			mPairsCapacity = pairsCapacity;
			mHashCapactiy = hashCapacity;
		}


		//Set each entry of the hash table to 0xffffffff
		PxMemSet(mHash, 0xff, sizeof(PxU32)*hashCapacity);

		//Init the sizes of the pairs array and hash array.
		mPairsSize = 0;
		mHashSize = hashCapacity;

		PxU32* PX_RESTRICT hashes = mHash;
		PxU32* PX_RESTRICT nextIndices = mNexts;
		Pair* PX_RESTRICT pairs = mPairs;

		//Add all the pairs from the stream.
		PxU32 pairsSize = 0;
		for(PxU32 i = 0; i < pairsCapacity; i++)
		{
			const PxcThresholdStreamElement& element = stream[i];
			const PxsRigidBody* PX_RESTRICT body0 = element.body0;
			const PxsRigidBody* PX_RESTRICT body1 = element.body1;
			const PxF32 force = element.normalForce;
			PX_ASSERT(body0 < body1);

			const PxU32 hashKey = computeHashKey(body0, body1, hashCapacity);

			//Get the index of the first pair found that resulted in a hash that matched hashKey.
			PxU32 prevPairIndex = hashKey;
			PxU32 pairIndex = hashes[hashKey];

			//Search through all pairs found that resulted in a hash that matched hashKey.
			//Search until the exact same body pair is found.
			//Increment the accumulated force if the exact same body pair is found.
			while(NO_INDEX != pairIndex)
			{
				Pair& pair = pairs[pairIndex];
				const PxU32 thresholdStreamIndex = pair.thresholdStreamIndex;
				PX_ASSERT(thresholdStreamIndex < stream.size());
				const PxcThresholdStreamElement& otherElement = stream[thresholdStreamIndex];
				if(body0 == otherElement.body0 && body1==otherElement.body1)
				{	
					pair.accumulatedForce += force;
					prevPairIndex = NO_INDEX;
					pairIndex = NO_INDEX;
					break;
				}
				prevPairIndex = pairIndex;
				pairIndex = nextIndices[pairIndex];
			}

			//Add a new pair if the exact same body pair wasn't found.
			if(NO_INDEX != prevPairIndex && NO_INDEX != pairIndex)
			{
				nextIndices[prevPairIndex] = pairsSize;
				nextIndices[pairsSize] = NO_INDEX;
				Pair& newPair = pairs[pairsSize];
				newPair.thresholdStreamIndex = i;
				newPair.accumulatedForce = force;
				pairsSize++;
			}
			else if(NO_INDEX != prevPairIndex && NO_INDEX == pairIndex)
			{
				hashes[hashKey] = pairsSize;
				nextIndices[pairsSize] = NO_INDEX;
				Pair& newPair = pairs[pairsSize];
				newPair.thresholdStreamIndex = i;
				newPair.accumulatedForce = force;
				pairsSize++;
			}
		}
		mPairsSize = pairsSize;
	}

	bool check(const PxsThresholdStream& stream, const PxsRigidBody* PX_RESTRICT body0, const PxsRigidBody* PX_RESTRICT body1, PxReal dt)
	{
		PxU32* PX_RESTRICT hashes = mHash;
		PxU32* PX_RESTRICT nextIndices = mNexts;
		Pair* PX_RESTRICT pairs = mPairs;

		const PxsRigidBody* b0 = PxMin(body0, body1);
		const PxsRigidBody* b1 = PxMax(body0, body1);

		PxU32 hashKey = computeHashKey(b0, b1, mHashSize);

		PxU32 pairIndex = hashes[hashKey];
		while(NO_INDEX != pairIndex)
		{
			Pair& pair = pairs[pairIndex];
			const PxU32 thresholdStreamIndex = pair.thresholdStreamIndex;
			PX_ASSERT(thresholdStreamIndex < stream.size());
			const PxcThresholdStreamElement& otherElement = stream[thresholdStreamIndex];
			if(otherElement.body0==b0 && otherElement.body1==b1)
				return (pair.accumulatedForce > (otherElement.threshold * dt));
			pairIndex = nextIndices[pairIndex];
		}
		return false;
	}


private:

	PX_FORCE_INLINE	PxU64 ptrToPxU64(const void* p) const 
	{ 
		return PxU64(reinterpret_cast<size_t>(p)); 
	}

	PX_FORCE_INLINE PxU32 computeHashKey(const PxsRigidBody* body0, const PxsRigidBody* body1, const PxU32 hashCapacity) const
	{
		return (Ps::hash(ptrToPxU64(body0)<<32 | ptrToPxU64(body1)) % hashCapacity);
	}

	static const PxU32 NO_INDEX = 0xffffffff;

	struct Pair 
	{	
		PxU32			thresholdStreamIndex;
		PxReal			accumulatedForce;
		//PxU32			next;		// hash key & next ptr
	};

	PxU8*					mBuffer;

	PxU32*					mHash;
	PxU32					mHashSize;
	PxU32					mHashCapactiy;

	Pair*					mPairs;
	PxU32*					mNexts;
	PxU32					mPairsSize;
	PxU32					mPairsCapacity;
};

}

#endif //PXS_THRESHOLDTABLE_H
