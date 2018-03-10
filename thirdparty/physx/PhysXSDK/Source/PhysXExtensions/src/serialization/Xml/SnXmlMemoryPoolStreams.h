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
#ifndef PX_XML_MEMORY_POOL_STREAMS_H
#define PX_XML_MEMORY_POOL_STREAMS_H

#include "SnXmlMemoryPool.h"
#include "CmPhysXCommon.h"
#include "PxTransform.h"
#include "PxIO.h"

#pragma warning(push)

namespace physx {

	template<typename TDataType>
	struct XmlDefaultValue
	{
		bool force_compile_error;
	};


#define XML_DEFINE_DEFAULT_VALUE(type, defVal )		\
	template<>											\
	struct XmlDefaultValue<type>						\
	{													\
		type getDefaultValue() { return type(defVal); }	\
	};

	XML_DEFINE_DEFAULT_VALUE(PxU8, 0)
	XML_DEFINE_DEFAULT_VALUE(PxI8, 0)
	XML_DEFINE_DEFAULT_VALUE(PxU16, 0)
	XML_DEFINE_DEFAULT_VALUE(PxI16, 0)
	XML_DEFINE_DEFAULT_VALUE(PxU32, 0)
	XML_DEFINE_DEFAULT_VALUE(PxI32, 0)
	XML_DEFINE_DEFAULT_VALUE(PxU64, 0)
	XML_DEFINE_DEFAULT_VALUE(PxI64, 0)
	XML_DEFINE_DEFAULT_VALUE(PxF32, 0)
	XML_DEFINE_DEFAULT_VALUE(PxF64, 0)

#undef XML_DEFINE_DEFAULT_VALUE

	template<>											
	struct XmlDefaultValue<PxVec3>						
	{													
		PxVec3 getDefaultValue() { return PxVec3( 0,0,0 ); }	
	};
	
	template<>											
	struct XmlDefaultValue<PxTransform>						
	{													
		PxTransform getDefaultValue() { return PxTransform(PxIdentity); }	
	};

	template<>											
	struct XmlDefaultValue<PxQuat>	
	{													
		PxQuat getDefaultValue() { return PxQuat(PxIdentity); }	
	};

/** 
 *	Mapping of PxOutputStream to a memory pool manager.
 *	Allows write-then-read semantics of a set of
 *	data.  Can safely write up to 4GB of data; then you
 *	will silently fail...
 */

template<typename TAllocatorType>
struct MemoryBufferBase : public PxOutputStream, public PxInputStream
{
	TAllocatorType* mManager;
	mutable PxU32	mWriteOffset;
	mutable PxU32	mReadOffset;
	PxU8*	mBuffer;
	PxU32	mCapacity;


	MemoryBufferBase( TAllocatorType* inManager )
		: mManager( inManager )
		, mWriteOffset( 0 )
		, mReadOffset( 0 )
		, mBuffer( NULL )
		, mCapacity( 0 )
	{
	}
	virtual						~MemoryBufferBase()
	{
		mManager->deallocate( mBuffer );
	}
	PxU8* releaseBuffer()
	{
		clear();
		mCapacity = 0;
		PxU8* retval(mBuffer);
		mBuffer = NULL;
		return retval;
	}
	void clear()
	{
		mWriteOffset = mReadOffset = 0;
	}

	virtual PxU32 read(void* dest, PxU32 count)
	{
		bool fits = ( mReadOffset + count ) <= mWriteOffset;
		PX_ASSERT( fits );
		if ( fits )
		{
			memcpy( dest, mBuffer + mReadOffset, count );
			mReadOffset += count;
			return count;
		}
		return 0;
	}

	inline void checkCapacity( PxU32 inNewCapacity )
	{
		if ( mCapacity < inNewCapacity )
		{
			PxU32 newCapacity = 32;
			while( newCapacity < inNewCapacity )
				newCapacity = newCapacity << 1;

			PxU8* newData( mManager->allocate( newCapacity ) );
			if ( mWriteOffset )
				memcpy( newData, mBuffer, mWriteOffset );
			mManager->deallocate( mBuffer );
			mBuffer = newData;
			mCapacity = newCapacity;
		}
	}

	virtual PxU32 write(const void* src, PxU32 count)
	{
		checkCapacity( mWriteOffset + count );
		memcpy( mBuffer + mWriteOffset, src, count );
		mWriteOffset += count;
		return count;
	}
};

class MemoryBuffer : public MemoryBufferBase<CMemoryPoolManager >
{
public:
	MemoryBuffer( CMemoryPoolManager* inManager ) : MemoryBufferBase<CMemoryPoolManager >( inManager ) {}
};

}

#pragma warning(pop)
#endif
