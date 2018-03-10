/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef SERIALIZER_COMMON_H
#define SERIALIZER_COMMON_H

#include <stdio.h> // FILE

#include "PsShare.h"
#include "PsArray.h"
#include "PsHashMap.h"

#include "NxParameterized.h"
#include "NxParameterizedTraits.h"
#include "NxSerializer.h"

#define ENABLE_DEBUG_ASSERTS 0

#if ENABLE_DEBUG_ASSERTS
#	define DEBUG_ASSERT(x) PX_ASSERT(x)
#else
#	define DEBUG_ASSERT(x)
#endif
#define DEBUG_ALWAYS_ASSERT() DEBUG_ASSERT(0)

#define NX_ERR_CHECK_RETURN(x) { Serializer::ErrorType err = x; if( Serializer::ERROR_NONE != err ) { DEBUG_ALWAYS_ASSERT(); return err; } }
#define NX_BOOL_ERR_CHECK_RETURN(x, err) { if( !(x) ) { DEBUG_ALWAYS_ASSERT(); return err; } }
#define NX_PARAM_ERR_CHECK_RETURN(x, err) { if( NxParameterized::ERROR_NONE != (NxParameterized::ErrorType)(x) ) { DEBUG_ALWAYS_ASSERT(); return err; } }

#define NX_ERR_CHECK_WARN_RETURN(x, ...) { \
	Serializer::ErrorType err = x; \
	if( Serializer::ERROR_NONE != err ) { \
		NX_PARAM_TRAITS_WARNING(mTraits, ##__VA_ARGS__); \
		DEBUG_ALWAYS_ASSERT(); \
		return err; \
	} \
}

#define NX_BOOL_ERR_CHECK_WARN_RETURN(x, err, ...) { \
	if( !(x) ) { \
		NX_PARAM_TRAITS_WARNING(mTraits, ##__VA_ARGS__); \
		DEBUG_ALWAYS_ASSERT(); \
		return err; \
	} \
}

#define NX_PARAM_ERR_CHECK_WARN_RETURN(x, err, ...) { \
	if( NxParameterized::ERROR_NONE != (NxParameterized::ErrorType)(x) ) \
	{ \
		NX_PARAM_TRAITS_WARNING(mTraits, ##__VA_ARGS__); \
		DEBUG_ALWAYS_ASSERT(); \
		return err; \
	} \
}

namespace NxParameterized
{

bool UpgradeLegacyObjects(Serializer::DeserializedData &data, bool &isUpdated, Traits *t);
Interface *UpgradeObject(Interface &obj, bool &isUpdated, Traits *t);

//This is used for releasing resources (I wish we had generic smart ptrs...)
class Releaser
{
	FILE *mFile;

	void *mBuf;
	Traits *mTraits;

	Interface *mObj;

	Releaser(const Releaser &) {}

public:

	Releaser()
	{
		reset();
	}

	Releaser(void *buf, Traits *traits)
	{
		reset(buf, traits);
	}

	Releaser(FILE *file)
	{
		reset(file);
	}

	Releaser(Interface *obj)
	{
		reset(obj);
	}

	void reset()
	{
		mFile = 0;
		mBuf = 0;
		mTraits = 0;
		mObj = 0;
	}

	void reset(Interface *obj)
	{
		reset();

		mObj = obj;
	}

	void reset(FILE *file)
	{
		reset();

		mFile = file;
	}

	void reset(void *buf, Traits *traits)
	{
		reset();

		mBuf = buf;
		mTraits = traits;
	}

	~Releaser()
	{
		if( mBuf )
			mTraits->free(mBuf);

		if( mFile )
			fclose(mFile);

		if( mObj )
			mObj->destroy();
	}
};

void *serializerMemAlloc(physx::PxU32 size, Traits *traits);
void serializerMemFree(void *mem, Traits *traits);

// Checksum for some classes is invalid
bool DoIgnoreChecksum(const Interface &obj);

} // namespace NxParameterized

#endif
