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


#ifndef PX_FOUNDATION_PSHASH_H
#define PX_FOUNDATION_PSHASH_H

#include "Ps.h"
#include "PsBasicTemplates.h"

#ifdef PX_VC
#pragma warning(push)
#pragma warning(disable:4302)
//#pragma warning(disable:4311) // intentionally suppressing 64 bit warning message as we are taking this use case into account.
#endif

/*!
Central definition of hash functions
*/

namespace physx
{
namespace shdfnd
{
	// Hash functions

	// Thomas Wang's 32 bit mix
	// http://www.cris.com/~Ttwang/tech/inthash.htm
	PX_INLINE PxU32 hash(const PxU32 key)
	{
		PxU32 k = key;
		k += ~(k << 15);
		k ^= (k >> 10);
		k += (k << 3);
		k ^= (k >> 6);
		k += ~(k << 11);
		k ^= (k >> 16);
		return (PxU32)k;
	}

	PX_INLINE PxU32 hash(const PxI32 key)
	{
		return hash(PxU32(key));
	}

	// Thomas Wang's 64 bit mix
	// http://www.cris.com/~Ttwang/tech/inthash.htm
	PX_INLINE PxU32 hash(const PxU64 key)
	{
		PxU64 k = key;
		k += ~(k << 32);
		k ^= (k >> 22);
		k += ~(k << 13);
		k ^= (k >> 8);
		k += (k << 3);
		k ^= (k >> 15);
		k += ~(k << 27);
		k ^= (k >> 31);
		return (PxU32)(PX_MAX_U32 & k);
	}

#if defined(PX_APPLE) || (defined(PX_LINUX) && defined(PX_X64)) || defined(PX_PS4)
	// hash for size_t, to make gcc happy
	PX_INLINE PxU32 hash(const size_t key)
	{
		if(sizeof(key) == sizeof(PxU32))
			return hash(PxU32(key));
		else if (sizeof(key) == sizeof(PxU64))
			return hash(PxU64(key));
		else
			return 0;

	}
#endif

	// Hash function for pointers
	PX_INLINE PxU32 hash(const void* ptr)
	{
#if defined(PX_P64)
		return hash(PxU64(ptr));
#else
		return hash((PxU32)(PX_MAX_U32 & (size_t)ptr));
#endif
	}

	// Hash function for pairs
	template<typename F, typename S>
	PX_INLINE PxU32 hash(const Pair<F, S>& p)
	{
		PxU32 seed = 0x876543;
		PxU32 m = 1000007;
		return hash(p.second) ^ (m * (hash(p.first) ^ (m * seed)));
	}

	// hash object for hash map template parameter
	template <class Key>
	struct Hash
	{
		PxU32 operator()(const Key& k) const { return hash(k); }
		bool operator()(const Key& k0, const Key& k1) const { return k0 == k1; }
	};


	// specialization for strings
	template <>
	struct Hash<const char*>
	{
	public:
		PxU32 operator()(const char *string) const
		{
			// "DJB" string hash 
			PxU32 h = 5381;
			for(const char *ptr = string; *ptr; ptr++)
				h = ((h<<5)+h)^*ptr;
			return h;
		}
		bool operator()(const char* string0, const char* string1) const
		{
			return !strcmp(string0, string1);
		}
	};

} // namespace shdfnd
} // namespace physx

#ifdef PX_VC
#pragma warning(pop)
#endif

#endif
