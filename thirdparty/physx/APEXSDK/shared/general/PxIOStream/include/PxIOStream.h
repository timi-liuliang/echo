/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_IOSTREAM_H
#define PX_IOSTREAM_H

/*!
\file
\brief PxIOStream class
*/
#include "Ps.h"
#include "PxFileBuf.h"
#include "foundation/PxVec3.h"
#include "foundation/PxQuat.h"
#include <PxMat33Legacy.h>
#include <PxMat34Legacy.h>
#include "foundation/PxBounds3.h"
#include <string.h>
#include <stdlib.h>
#include "PsString.h"
#include "PxAsciiConversion.h"

#define safePrintf physx::string::sprintf_s

PX_PUSH_PACK_DEFAULT

namespace physx
{
	namespace general_PxIOStream2
	{

/**
\brief A wrapper class for physx::PxFileBuf that provides both binary and ASCII streaming capabilities
*/
class PxIOStream
{
	static const physx::PxU32 MAX_STREAM_STRING = 1024;
public:
	/**
	\param [in] stream the physx::PxFileBuf through which all reads and writes will be performed
	\param [in] streamLen the length of the input data stream when de-serializing
	*/
	PxIOStream(physx::PxFileBuf &stream,physx::PxU32 streamLen) : mBinary(true), mStreamLen(streamLen), mStream(stream) { };
	~PxIOStream(void) { };

	/**
	\brief Set the stream to binary or ASCII

	\param [in] state if true, stream is binary, if false, stream is ASCII

	If the stream is binary, stream access is passed straight through to the respecitve 
	physx::PxFileBuf methods.  If the stream is ASCII, all stream reads and writes are converted to
	human readable ASCII.
	*/
	PX_INLINE void setBinary(bool state) { mBinary = state; }
	PX_INLINE bool getBinary() { return mBinary; }

	PX_INLINE PxIOStream& operator<<(bool v);
	PX_INLINE PxIOStream& operator<<(char c);
	PX_INLINE PxIOStream& operator<<(physx::PxU8 v);
	PX_INLINE PxIOStream& operator<<(physx::PxI8 v);

	PX_INLINE PxIOStream& operator<<(const char *c);
	PX_INLINE PxIOStream& operator<<(physx::PxI64 v);
	PX_INLINE PxIOStream& operator<<(physx::PxU64 v);
	PX_INLINE PxIOStream& operator<<(physx::PxF64 v);
	PX_INLINE PxIOStream& operator<<(physx::PxF32 v);
	PX_INLINE PxIOStream& operator<<(physx::PxU32 v);
	PX_INLINE PxIOStream& operator<<(physx::PxI32 v);
	PX_INLINE PxIOStream& operator<<(physx::PxU16 v);
	PX_INLINE PxIOStream& operator<<(physx::PxI16 v);
	PX_INLINE PxIOStream& operator<<(const physx::PxVec3 &v);
	PX_INLINE PxIOStream& operator<<(const physx::PxQuat &v);
	PX_INLINE PxIOStream& operator<<(const physx::PxMat33Legacy &v);
	PX_INLINE PxIOStream& operator<<(const physx::PxMat34Legacy &v);
	PX_INLINE PxIOStream& operator<<(const physx::PxBounds3 &v);

	PX_INLINE PxIOStream& operator>>(const char *&c);
	PX_INLINE PxIOStream& operator>>(bool &v);
	PX_INLINE PxIOStream& operator>>(char &c);
	PX_INLINE PxIOStream& operator>>(physx::PxU8 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxI8 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxI64 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxU64 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxF64 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxF32 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxU32 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxI32 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxU16 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxI16 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxVec3 &v);
	PX_INLINE PxIOStream& operator>>(physx::PxQuat &v);
	PX_INLINE PxIOStream& operator>>(physx::PxMat33Legacy &v);
	PX_INLINE PxIOStream& operator>>(physx::PxMat34Legacy &v);
	PX_INLINE PxIOStream& operator>>(physx::PxBounds3 &v);

	physx::PxU32 getStreamLen(void) const { return mStreamLen; }

	physx::PxFileBuf& getStream(void) { return mStream; }

	PX_INLINE void storeString(const char *c,bool zeroTerminate=false);

private:
	PxIOStream& operator=( const PxIOStream& );


	bool      mBinary; // true if we are serializing binary data.  Otherwise, everything is assumed converted to ASCII
	physx::PxU32     mStreamLen; // the length of the input data stream when de-serializing.
	physx::PxFileBuf &mStream;
	char			mReadString[MAX_STREAM_STRING]; // a temp buffer for streaming strings on input.
};

#include "PxIOStream.inl" // inline methods...

	}; // end of namespace
	using namespace general_PxIOStream2;
}; // end of physx namespace

PX_POP_PACK

#endif // PX_IOSTREAM_H
