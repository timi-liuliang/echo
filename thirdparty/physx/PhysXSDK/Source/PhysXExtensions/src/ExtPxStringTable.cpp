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


#include "PxAllocatorCallback.h"
#include "PxStringTableExt.h"
#include "PxProfileFoundationWrapper.h" //tools for using a custom allocator
#include "PsString.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"

namespace physx
{
	using namespace physx::profile;

	class PxStringTableImpl : public PxStringTable, public Ps::UserAllocated
	{
		typedef ProfileHashMap<const char*, PxU32> THashMapType;
		FoundationWrapper mWrapper;
		THashMapType mHashMap;
	public:

		PxStringTableImpl( PxAllocatorCallback& inAllocator )
			: mWrapper ( inAllocator )
			, mHashMap ( mWrapper )
		{
		}

		virtual ~PxStringTableImpl()
		{
			for ( THashMapType::Iterator iter = mHashMap.getIterator();
				iter.done() == false;
				++iter )
				PX_PROFILE_DELETE( mWrapper, const_cast<char*>( iter->first ) );
			mHashMap.clear();
		}


		virtual const char* allocateStr( const char* inSrc )
		{
			if ( inSrc == NULL )
				inSrc = "";
			const THashMapType::Entry* existing( mHashMap.find( inSrc ) );
			if ( existing == NULL )
			{
				size_t len( strlen( inSrc ) );
				len += 1;
				char* newMem = (char*)mWrapper.getAllocator().allocate( len, "PxStringTableImpl: const char*", __FILE__, __LINE__ );
				physx::string::strcpy_s( newMem, len, inSrc );
				mHashMap.insert( newMem, 1 );
				return newMem;
			}
			else
			{
				++const_cast<THashMapType::Entry*>(existing)->second;
				return existing->first;
			}
		}

		/**
		 *	Release the string table and all the strings associated with it.
		 */
		virtual void release()
		{
			PX_PROFILE_DELETE( mWrapper.getAllocator(), this );
		}
	};

	PxStringTable& PxStringTableExt::createStringTable( PxAllocatorCallback& inAllocator )
	{
		return *PX_PROFILE_NEW( inAllocator, PxStringTableImpl )( inAllocator );
	}
}
