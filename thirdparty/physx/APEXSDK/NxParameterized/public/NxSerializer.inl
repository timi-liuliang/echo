/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxParameterized.h"
#include "NxParameterizedTraits.h"

namespace NxParameterized
{

/*!
\brief Serializer::SerializePlatform and Serializer::DeserializedData inline implementation
*/

/**
\brief Check binary compatibility of compiler versions
*/
PX_INLINE bool DoCompilerVersMatch(SerializePlatform::CompilerType t, physx::PxU32 v1, physx::PxU32 v2)
{
	PX_UNUSED(t);
	if( SerializePlatform::ANY_VERSION == v1 || SerializePlatform::ANY_VERSION == v2 )
		return true;
	
	//In future we should distinguish compiler versions which have different ABI
	//but now we are optimistic

	return true;
}

/**
\brief Check binary compatibility of OS versions
*/
PX_INLINE bool DoOsVersMatch(SerializePlatform::OsType t, physx::PxU32 v1, physx::PxU32 v2)
{
	PX_UNUSED(t);
	if( SerializePlatform::ANY_VERSION == v1 || SerializePlatform::ANY_VERSION == v2 )
		return true;
	
	return true; //See comment for doCompilerVersMatch
}

PX_INLINE SerializePlatform::SerializePlatform()
	: archType(ARCH_LAST),
	compilerType(COMP_LAST),
	compilerVer(ANY_VERSION),
	osType(OS_LAST),
	osVer(ANY_VERSION)
{}

PX_INLINE SerializePlatform::SerializePlatform(ArchType archType_, CompilerType compType_, physx::PxU32 compVer_, OsType osType_, physx::PxU32 osVer_)
	: archType(archType_),
	compilerType(compType_),
	compilerVer(compVer_),
	osType(osType_),
	osVer(osVer_)
{}

PX_INLINE bool SerializePlatform::operator ==(const SerializePlatform &p) const
{
	return archType == p.archType
		&& compilerType == p.compilerType
		&& osType == p.osType
		&& DoCompilerVersMatch(compilerType, compilerVer, p.compilerVer)
		&& DoOsVersMatch(osType, osVer, p.osVer);
}

PX_INLINE bool SerializePlatform::operator !=(const SerializePlatform &p) const
{
	return !(*this == p);
}

template<typename T, int bufSize> PX_INLINE Serializer::DeserializedResults<T, bufSize>::DeserializedResults(): objs(0), nobjs(0), traits(0) {}

template<typename T, int bufSize> PX_INLINE Serializer::DeserializedResults<T, bufSize>::DeserializedResults(const Serializer::DeserializedResults<T, bufSize> &data)
{
	*this = data;
}

template<typename T, int bufSize> PX_INLINE Serializer::DeserializedResults<T, bufSize> &Serializer::DeserializedResults<T, bufSize>::operator =(const Serializer::DeserializedResults<T, bufSize> &rhs)
{
	if( this == &rhs )
		return *this;

	init(rhs.traits, rhs.objs, rhs.nobjs);
	return *this;
}

template<typename T, int bufSize> PX_INLINE void Serializer::DeserializedResults<T, bufSize>::clear()
{
	if ( objs && objs != buf ) //Memory was allocated?
	{
		PX_ASSERT(traits);
		traits->free(objs);
	}
}

template<typename T, int bufSize> PX_INLINE Serializer::DeserializedResults<T, bufSize>::~DeserializedResults()
{
	clear();
}

template<typename T, int bufSize> PX_INLINE void Serializer::DeserializedResults<T, bufSize>::init(Traits *traits_, T *objs_, physx::PxU32 nobjs_)
{
	init(traits_, nobjs_);
	::memcpy(objs, objs_, nobjs * sizeof(T));
}

template<typename T, int bufSize> PX_INLINE void Serializer::DeserializedResults<T, bufSize>::init(Traits *traits_, physx::PxU32 nobjs_)
{
	clear();

	traits = traits_;
	nobjs = nobjs_;

	//Allocate memory if buf is too small
	objs = nobjs <= bufSize
		? buf
		: (T *)traits->alloc(nobjs * sizeof(T));
}

template<typename T, int bufSize> PX_INLINE physx::PxU32 Serializer::DeserializedResults<T, bufSize>::size() const
{
	return nobjs;
}

template<typename T, int bufSize> PX_INLINE T &Serializer::DeserializedResults<T, bufSize>::operator[](physx::PxU32 i)
{
	PX_ASSERT( i < nobjs );
	return objs[i];
}

template<typename T, int bufSize> PX_INLINE const T &Serializer::DeserializedResults<T, bufSize>::operator[](physx::PxU32 i) const
{
	PX_ASSERT( i < nobjs );
	return objs[i];
}

template<typename T, int bufSize> PX_INLINE void Serializer::DeserializedResults<T, bufSize>::getObjects(T *outObjs)
{
	::memcpy(outObjs, objs, nobjs * sizeof(T));
}

template<typename T, int bufSize> PX_INLINE void Serializer::DeserializedResults<T, bufSize>::releaseAll()
{
	for(physx::PxU32 i = 0; i < nobjs; ++i)
	{
		if (objs[i]) 
		{
			objs[i]->destroy(); // FIXME What should we do with buf. And should we delete T* obj? 
		}
	}
}

} // namespace NxParameterized
