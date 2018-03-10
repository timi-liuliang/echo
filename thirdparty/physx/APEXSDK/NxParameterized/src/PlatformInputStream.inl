/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

template<typename T> PX_INLINE Serializer::ErrorType PlatformInputStream::read(T &x, bool doAlign)
{
	if( doAlign )
		align(mTargetParams.getAlignment<T>());

	if( mCurParams.getSize<T>() != mTargetParams.getSize<T>() )
	{
		PX_ALWAYS_ASSERT();
		return Serializer::ERROR_INVALID_PLATFORM;
	}

	mStream.read(&x, sizeof(T));

	if( mCurParams.endian != mTargetParams.endian )
		SwapBytes((char *)&x, sizeof(T), GetDataType<T>::value);

	return Serializer::ERROR_NONE;
}

//Deserialize array of primitive type (slow path)
template<typename T> PX_INLINE Serializer::ErrorType PlatformInputStream::readSimpleArraySlow(Handle &handle)
{
	physx::PxI32 n;
	handle.getArraySize(n);

	align(mTargetParams.getAlignment<T>());

	for(physx::PxI32 i = 0; i < n; ++i)
	{
		handle.set(i);

		T val;
		NX_ERR_CHECK_RETURN( read(val) );
		handle.setParam<T>(val);

		handle.popIndex();
	}

	return Serializer::ERROR_NONE;
}

template<> PX_INLINE Serializer::ErrorType PlatformInputStream::readSimpleArraySlow<physx::PxMat34Legacy>(Handle &)
{
	PX_ASSERT(0);
	return Serializer::ERROR_UNKNOWN;
}

template<typename T> PX_INLINE Serializer::ErrorType PlatformInputStream::readSimpleArray(Handle &handle)
{
	if( mTargetParams.getSize<T>() == mCurParams.getSize<T>()
			&& mTargetParams.getSize<T>() >= mTargetParams.getAlignment<T>()
			&& mCurParams.getSize<T>() >= mCurParams.getAlignment<T>() ) //No gaps between array elements on both platforms
	{
		//Fast path

		physx::PxI32 n;
		handle.getArraySize(n);

		align(mTargetParams.getAlignment<T>());

		const physx::PxI32 elemSize = (physx::PxI32)sizeof(T);

		if( mStream.tellRead() + elemSize * n >= mStream.getFileLength() )
		{
			DEBUG_ALWAYS_ASSERT();
			return Serializer::ERROR_INVALID_INTERNAL_PTR;
		}

		char *p = (char *)mTraits->alloc((physx::PxU32)elemSize * n);
		mStream.read(p, (physx::PxU32)elemSize * n);

		if( mCurParams.endian != mTargetParams.endian )
		{
			char *elem = p;
			for(physx::PxI32 i = 0; i < n; ++i)
			{
				SwapBytes(elem, elemSize, GetDataType<T>::value);
				elem += elemSize;
			}
		}

		handle.setParamArray<T>((const T *)p, n);

		mTraits->free(p);
	}
	else
	{
		return readSimpleArraySlow<T>(handle);
	}

	return Serializer::ERROR_NONE;
}

template<> PX_INLINE Serializer::ErrorType PlatformInputStream::readSimpleArray<physx::PxMat34Legacy>(Handle &)
{
	PX_ASSERT(0);
	return Serializer::ERROR_UNKNOWN;
}
