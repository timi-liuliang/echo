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
#ifndef PX_PXSTREAMOPERATORS_H
#define PX_PXSTREAMOPERATORS_H

#include "PxVec3.h"
#include "PxTransform.h"
#include "PxBounds3.h"
#include "PxFiltering.h"

namespace physx
{

	static inline PxU32 strLen( const char* inStr )
	{
		return inStr ? (PxU32)strlen(inStr) : 0;
	}

	inline PxOutputStream& operator << ( PxOutputStream& ioStream, const char* inString )
	{
		if ( inString && *inString )
		{
			ioStream.write( inString, strLen(inString) );
		}
		return ioStream;
	}

	template<typename TDataType>
	inline PxOutputStream& toStream( PxOutputStream& ioStream, const char* inFormat, const TDataType inData )
	{
		char buffer[128] = { 0 };
		sprintf( buffer, inFormat, inData );
		ioStream << buffer;
		return ioStream;
	}

	struct endl_obj {};
	//static endl_obj endl;

	inline PxOutputStream& operator << ( PxOutputStream& ioStream, bool inData ) { ioStream << (inData ? "true" : "false"); return ioStream; }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxI32 inData ) { return toStream( ioStream, "%d",  inData ); }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxU16 inData ) {	return toStream( ioStream, "%u", (PxU32)inData ); }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxU8 inData ) {	return toStream( ioStream, "%u", (PxU32)inData ); }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, char inData ) {	return toStream( ioStream, "%c", inData ); }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxU32 inData ) {	return toStream( ioStream, "%u", inData ); }
#if defined(PX_GNUC) || defined(PX_GHS)
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxU64 inData ) {	return toStream( ioStream, "%llu", inData ); }
#else
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxU64 inData ) {	return toStream( ioStream, "%I64u", inData ); }
#endif
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, const void* inData ) { return ioStream << PX_PROFILE_POINTER_TO_U64( inData ); }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxF32 inData ) { return toStream( ioStream, "%g", (PxF64)inData ); }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, PxF64 inData ) { return toStream( ioStream, "%g", inData ); }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, endl_obj) { return ioStream << "\n"; }
	inline PxOutputStream& operator << ( PxOutputStream& ioStream, const PxVec3& inData ) 
	{ 
		ioStream << inData[0];
		ioStream << " ";
		ioStream << inData[1];
		ioStream << " ";
		ioStream << inData[2];
		return ioStream;
	}

	inline PxOutputStream& operator << ( PxOutputStream& ioStream, const PxQuat& inData ) 
	{
		ioStream << inData.x;
		ioStream << " ";
		ioStream << inData.y;
		ioStream << " ";
		ioStream << inData.z;
		ioStream << " ";
		ioStream << inData.w;
		return ioStream;
	}

	inline PxOutputStream& operator << ( PxOutputStream& ioStream, const PxTransform& inData ) 
	{
		ioStream << inData.q;
		ioStream << " ";
		ioStream << inData.p;
		return ioStream;
	}

	inline PxOutputStream& operator << ( PxOutputStream& ioStream, const PxBounds3& inData )
	{
		ioStream << inData.minimum;
		ioStream << " ";
		ioStream << inData.maximum;
		return ioStream;
	}

	inline PxOutputStream& operator << ( PxOutputStream& ioStream, const PxFilterData& inData )
	{
		ioStream << inData.word0 << " " << inData.word1 << " " << inData.word2 << " " << inData.word3;
		return ioStream;
	}

	inline PxOutputStream& operator << ( PxOutputStream& ioStream, struct PxMetaDataPlane& inData )
	{
		ioStream << inData.normal;
		ioStream << " ";
		ioStream << inData.distance;
		return ioStream;
	}
}

#endif
