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


#ifndef PX_PHYSICS_COMMON_IO
#define PX_PHYSICS_COMMON_IO

#include "PxIO.h"
#include "PxMemory.h"
#include "CmPhysXCommon.h"
#include "PxAssert.h"
#include "PxSerialFramework.h"
#include "CmUtils.h"

namespace physx
{

	// wrappers for IO classes so that we can add extra functionality (byte counting, buffering etc)

namespace Cm
{

class InputStreamReader
{
public:

	InputStreamReader(PxInputStream& stream) : mStream(stream) {	}
	PxU32	read(void* dest, PxU32 count)	
	{	
		PxU32 readLength = mStream.read(dest, count);

		// zero the buffer if we didn't get all the data
		if(readLength<count)
			PxMemZero(reinterpret_cast<PxU8*>(dest)+readLength, count-readLength);
	
		return readLength;
	}

	template <typename T> T get()			
	{		
		T val;	
		PxU32 length = mStream.read(&val, sizeof(T));
		PX_ASSERT(length == sizeof(T));
		PX_UNUSED(length);
		return val; 
	}


protected:
	PxInputStream &mStream;
private:
	InputStreamReader& operator=(const InputStreamReader&);
};


class InputDataReader : public InputStreamReader
{
public:
	InputDataReader(PxInputData& data) : InputStreamReader(data) {}
	InputDataReader &operator=(const InputDataReader &);

	PxU32	length() const					{		return getData().getLength();		}
	void	seek(PxU32 offset)				{		getData().seek(offset);				}
	PxU32	tell()							{		return getData().tell();			}

private:
	PxInputData& getData()					{		return static_cast<PxInputData&>(mStream); }
	const PxInputData& getData() const		{		return static_cast<const PxInputData&>(mStream); }
};


class OutputStreamWriter
{
public:

	PX_INLINE OutputStreamWriter(PxOutputStream& stream) 
	:	mStream(stream)
	,	mCount(0)
	{}

	PX_INLINE	PxU32	write(const void* src, PxU32 offset)		
	{		
		PxU32 count = mStream.write(src, offset);
		mCount += count;
		return count;
	}

	PX_INLINE	PxU32	getStoredSize()
	{
		return mCount;
	}

	template<typename T> void put(const T& val)	
	{		
		PxU32 length = write(&val, sizeof(T));		
		PX_ASSERT(length == sizeof(T));
		PX_UNUSED(length);
	}

private:

	OutputStreamWriter& operator=(const OutputStreamWriter&);
	PxOutputStream& mStream;
	PxU32 mCount;
};



}
}

#endif
