/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_PARAMETERIZED_TRAITS_H
#define NX_PARAMETERIZED_TRAITS_H

/*!
\file
\brief NxParameterized traits class
*/

#include <string.h>
#include "foundation/PxAssert.h"

#include "NxApexUsingNamespace.h"

namespace NxParameterized
{

PX_PUSH_PACK_DEFAULT

class Traits;
class Interface;

/**
\brief Interface class for NxParameterized factories
*/
class Factory
{
public:

	/**
	\brief Creates NxParameterized object of class
	*/
	virtual ::NxParameterized::Interface *create( Traits *paramTraits ) = 0;

	/**
	\brief Finishes initialization of inplace-deserialized objects (vptr and stuff)
	*/
	virtual ::NxParameterized::Interface *finish( Traits *paramTraits, void *obj, void *buf, physx::PxI32 *refCount ) = 0;

	/**
	\brief Returns name of class whose objects are created by factory
	*/
	virtual const char * getClassName() = 0;

	/**
	\brief Returns version of class whose objects are created by factory
	*/
	virtual physx::PxU32 getVersion() = 0;

	/**
	\brief Returns memory alignment required for objects of class
	*/
	virtual physx::PxU32 getAlignment() = 0;

	/**
	\brief Returns checksum of class whose objects are created by factory
	*/
	virtual const physx::PxU32 * getChecksum( physx::PxU32 &bits ) = 0;
};

/**
\brief Interface class for legacy object conversions
*/
class Conversion
{
public:
	virtual ~Conversion() {}

	/**
	\brief Initialize object using data from legacy object
	\param [in] legacyObj legacy object to be converter
	\param [in] obj destination object
	\return true if conversion succeeded, false otherwise
	\warning You may assume that all nested references were already converted.
	*/
	virtual bool operator()(::NxParameterized::Interface &legacyObj, ::NxParameterized::Interface &obj) = 0;

	/**
	\brief Release converter and any memory allocations associated with it
	*/
	virtual void release() = 0;
};

/**
\brief Interface class for user traits

This class is a collection of loosely-related functions provided by application or framework
and used by NxParameterized library to do memory allocation, object creation, user notification, etc.
*/
class Traits
{
public:
	virtual ~Traits() {}

	/**
	\brief Register NxParameterized class factory
	*/
	virtual void registerFactory( ::NxParameterized::Factory & factory ) = 0;

	/**
	\brief Remove NxParameterized class factory for current version of class
	\return Removed factory or NULL if it is not found
	*/
	virtual ::NxParameterized::Factory *removeFactory( const char * className ) = 0;

	/**
	\brief Remove NxParameterized class factory for given version of class
	\return Removed factory or NULL if it is not found
	*/
	virtual ::NxParameterized::Factory *removeFactory( const char * className, physx::PxU32 version ) = 0;

	/**
	\brief Checks whether any class factory is registered
	*/
	virtual bool doesFactoryExist(const char* className) = 0;

	/**
	\brief Checks whether class factory for given version is registered
	*/
	virtual bool doesFactoryExist(const char* className, physx::PxU32 version) = 0;

	/**
	\brief Create object of NxParameterized class
	
	Most probably this just calls Factory::create on appropriate factory.
	*/
	virtual ::NxParameterized::Interface * createNxParameterized( const char * name ) = 0;

	/**
	\brief Create object of NxParameterized class
	
	Most probably this just calls Factory::create on appropriate factory.
	*/
	virtual ::NxParameterized::Interface * createNxParameterized( const char * name, physx::PxU32 ver ) = 0;

	/**
	\brief Finish construction of inplace object of NxParameterized class
	
	Most probably this just calls Factory::finish using appropriate factory.
	*/
	virtual ::NxParameterized::Interface * finishNxParameterized( const char * name, void *obj, void *buf, physx::PxI32 *refCount ) = 0;

	/**
	\brief Finish construction of inplace object of NxParameterized class
	
	Most probably this just calls Factory::finish using appropriate factory.
	*/
	virtual ::NxParameterized::Interface * finishNxParameterized( const char * name, physx::PxU32 ver, void *obj, void *buf, physx::PxI32 *refCount ) = 0;

	/**
	\brief Get version of class which is currently used
	*/
	virtual physx::PxU32 getCurrentVersion(const char *className) const = 0;

	/**
	\brief Get memory alignment required for objects of class
	*/
	virtual physx::PxU32 getAlignment(const char *className, physx::PxU32 classVersion) const = 0;

	/**
	\brief Register converter for legacy version of class
	*/
	virtual void registerConversion(const char * /*className*/, physx::PxU32 /*from*/, physx::PxU32 /*to*/, Conversion & /*conv*/) {}

	/**
	\brief Remove converter for legacy version of class
	*/
	virtual ::NxParameterized::Conversion *removeConversion(const char * /*className*/, physx::PxU32 /*from*/, physx::PxU32 /*to*/) { return 0; }

	/**
	\brief Update legacy object (most probably using appropriate registered converter)
	\param [in] legacyObj legacy object to be converted
	\param [in] obj destination object
	\return True if conversion was successful, false otherwise
	\warning Note that update is intrusive - legacyObj may be modified as a result of update
	*/
	virtual bool updateLegacyNxParameterized(::NxParameterized::Interface &legacyObj, ::NxParameterized::Interface &obj)
	{
		PX_UNUSED(&legacyObj);
		PX_UNUSED(&obj);

		return false;
	}

	/**
	\brief Get a list of the NxParameterized class type names

	\param [in] names buffer for names
	\param [out] outCount minimal required length of buffer
	\param [in] inCount length of buffer
	\return False if 'inCount' is not large enough to contain all of the names, true otherwise
	
	\warning The memory for the strings returned is owned by the traits class
	         and should only be read, not written or freed.
	*/
	virtual bool getNxParameterizedNames( const char ** names, physx::PxU32 &outCount, physx::PxU32 inCount) const = 0;

		/**
	\brief Get a list of versions of particular NxParameterized class

	\param [in] className Name of the class
	\param [in] versions buffer for versions
	\param [out] outCount minimal required length of buffer
	\param [in] inCount length of buffer
	\return False if 'inCount' is not large enough to contain all of version names, true otherwise
	
	\warning The memory for the strings returned is owned by the traits class
	         and should only be read, not written or freed.
	*/
	virtual bool getNxParameterizedVersions(const char* className, physx::PxU32* versions, physx::PxU32 &outCount, physx::PxU32 inCount) const = 0;

	/**
	\brief Increment reference counter
	*/
	virtual physx::PxI32 incRefCount(physx::PxI32 *refCount) = 0;

	/**
	\brief Decrement reference counter
	*/
	virtual physx::PxI32 decRefCount(physx::PxI32 *refCount) = 0;

	/**
	\brief Called when inplace object is destroyed
	*/
	virtual void onInplaceObjectDestroyed(void * /*buf*/, ::NxParameterized::Interface * /*obj*/) {}

	/**
	\brief Called when all inplace objects are destroyed
	*/
	virtual void onAllInplaceObjectsDestroyed(void *buf) { free(buf); }

	/**
	\brief Allocate memory with default alignment of 8
	*/
	virtual void *alloc(physx::PxU32 nbytes) = 0;

	/**
	\brief Allocate aligned memory
	*/
	virtual void *alloc(physx::PxU32 nbytes, physx::PxU32 align) = 0;

	/**
	\brief Deallocate memory
	*/
	virtual void free(void *buf) = 0;

	/**
	\brief Copy string
	*/
	virtual char *strdup(const char *str)
	{
		if( !str )
			return NULL;

		physx::PxU32 strLen = (physx::PxU32)strlen(str) + 1;
		char *retStr = (char *)this->alloc(strLen, 1);
		
		PX_ASSERT( retStr );
		
		if( NULL != retStr )
#if defined(PX_WINDOWS) || defined(PX_HAS_SECURE_STRCPY)
			strcpy_s( retStr, strLen, str );
#else
			strncpy(retStr, str, strLen);
#endif
		return retStr;
	}

 	/**
	\brief Release copied string
	*/
	virtual void strfree(char *str)
	{
		if( NULL != str )
			this->free( str );
	}

 	/**
	\brief Warns user
	*/
	virtual void traitsWarn(const char * /*msg*/) const {}

	/**
	\brief Adapter for allocator classes in PxAlloctor.h
	*/
	class Allocator
	{
		::NxParameterized::Traits *mTraits;

	public:

		/**
		\brief Constructor
		*/
		Allocator(Traits *traits): mTraits(traits) {}

		/**
		\brief Allocate memory
		*/
		void *allocate(size_t size)
		{
			return allocate(size, __FILE__, __LINE__);
		}

		/**
		\brief Allocate memory
		*/
		void *allocate(size_t size, const char * /*filename*/, int /*line*/)
		{
			PX_ASSERT( static_cast<physx::PxU32>(size) == size );
			return mTraits->alloc(static_cast<physx::PxU32>(size));
		}

		/**
		\brief Release memory
		*/
		void deallocate(void *ptr)
		{
			return mTraits->free(ptr);
		}
	};
};

/**
\brief Trivial implementation of Traits which redirects all calls to another traits object.
This class provides default implementation of Traits so that overloaded classes could limit themselves to implementing
only necessary methods.
*/
class WrappedTraits: public Traits
{
	::NxParameterized::Traits *wrappedTraits;

protected:
	/**
	\brief Allows derived classes to access core traits object
	*/
	::NxParameterized::Traits *getWrappedTraits()
	{
		return wrappedTraits;
	}

public:
	/**
	\brief Constructor
	\param [in] wrappedTraits all calls will be delegated to this Traits object
	\note Usually wrappedTraits_ will be an APEX Traits object
	*/
	WrappedTraits( ::NxParameterized::Traits *wrappedTraits );

	/// Calls wrapped Traits object
	void registerFactory( ::NxParameterized::Factory & factory );

	/// Calls wrapped Traits object
	::NxParameterized::Factory *removeFactory( const char * className );

	/// Calls wrapped Traits object
	::NxParameterized::Factory *removeFactory( const char * className, physx::PxU32 version );

	/// Calls wrapped Traits object
	::NxParameterized::Interface * createNxParameterized( const char * name );

	/// Calls wrapped Traits object
	::NxParameterized::Interface * createNxParameterized( const char * name, physx::PxU32 ver );

	/// Calls wrapped Traits object
	::NxParameterized::Interface * finishNxParameterized( const char * name, void *obj, void *buf, physx::PxI32 *refCount );

	/// Calls wrapped Traits object
	::NxParameterized::Interface * finishNxParameterized( const char * name, physx::PxU32 ver, void *obj, void *buf, physx::PxI32 *refCount );

	/// Calls wrapped Traits object
	physx::PxU32 getCurrentVersion(const char *className) const;

	/// Calls wrapped Traits object
	physx::PxU32 getAlignment(const char *className, physx::PxU32 classVersion) const;

	/// Calls wrapped Traits object
	void registerConversion(const char *className, physx::PxU32 from, physx::PxU32 to, Conversion &conv);

	/// Calls wrapped Traits object
	::NxParameterized::Conversion *removeConversion(const char *className, physx::PxU32 from, physx::PxU32 to);

	/// Calls wrapped Traits object
	bool updateLegacyNxParameterized(::NxParameterized::Interface &legacyObj, ::NxParameterized::Interface &obj);

	/// Calls wrapped Traits object
	bool getNxParameterizedNames( const char ** names, physx::PxU32 &outCount, physx::PxU32 inCount) const;

	/// Calls wrapped Traits object
	physx::PxI32 incRefCount(physx::PxI32 *refCount);

	/// Calls wrapped Traits object
	physx::PxI32 decRefCount(physx::PxI32 *refCount);

	/// Calls wrapped Traits object
	void onInplaceObjectDestroyed(void *buf, ::NxParameterized::Interface *obj);

	/// Calls wrapped Traits object
	void onAllInplaceObjectsDestroyed(void *buf);

	/// Calls wrapped Traits object
	void *alloc(physx::PxU32 nbytes);

	/// Calls wrapped Traits object
	void *alloc(physx::PxU32 nbytes, physx::PxU32 align);

	/// Calls wrapped Traits object
	void free(void *buf);

	/// Calls wrapped Traits object
	char *strdup(const char *str);

	/// Calls wrapped Traits object
	void strfree(char *str);

	/// Calls wrapped Traits object
	void traitsWarn(const char *msg) const;
};

PX_POP_PACK

}; // namespace NxParameterized

#endif // NX_PARAMETERIZED_TRAITS_H
