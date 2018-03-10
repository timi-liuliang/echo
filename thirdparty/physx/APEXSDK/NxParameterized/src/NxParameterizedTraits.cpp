/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include <PsHashMap.h>
#include <PsHash.h>
#include <PsShare.h>
#include <PsUserAllocated.h>

#include "NxParameterized.h"
#include "NxParameterizedTraits.h"
#include "NxTraitsInternal.h"
#include "SerializerCommon.h"

namespace NxParameterized
{

class DefaultParamConversion: public Conversion
{
	Traits *mTraits;
	RefConversionMode::Enum mRefMode;

	typedef physx::HashMap<const char *, physx::PxU32, physx::Hash<const char *>, Traits::Allocator> MapName2Ver;
	MapName2Ver mapName2Ver;

	bool copy(Handle &legacyHandle, Handle &handle)
	{
		const Definition *pdLegacy = legacyHandle.parameterDefinition(),
			*pd = handle.parameterDefinition();

		if( pdLegacy->type() != pd->type() )
			return true;

		switch( pdLegacy->type() )
		{
		case TYPE_STRUCT:
			for(physx::PxI32 i = 0; i < pdLegacy->numChildren(); ++i)
			{
				legacyHandle.set(i);

				physx::PxI32 j = -1;
				if( pd->child(pdLegacy->child(i)->name(), j) )
				{
					handle.set(j);
					if( !copy(legacyHandle, handle) )
					{
						char longName[128];
						legacyHandle.getLongName(longName, sizeof(longName));
						NX_PARAM_TRAITS_WARNING(
							mTraits,
							"%s: failed to copy legacy data",
							longName
						);

						return false;
					}
					handle.popIndex();
				}

				legacyHandle.popIndex();
			}
			break;

		case TYPE_ARRAY:
			{
				//FIXME: this is too harsh e.g. we can convert static to dynamic
				if( pdLegacy->arraySizeIsFixed() != pd->arraySizeIsFixed() )
					return true;

				physx::PxI32 size;
				if( pdLegacy->arraySizeIsFixed() )
				{
					//FIXME: this is too harsh e.g. we may just copy first elements
					if( pdLegacy->arraySize() != pd->arraySize() )
						return true;

					size = pd->arraySize();
				}
				else
				{
					if( ERROR_NONE != legacyHandle.getArraySize(size) )
						return false;

					handle.resizeArray(size);
				}

				if( size > 100 && pdLegacy->isSimpleType(false, false) && pdLegacy->type() == pd->type() )
				{
					// Large array of simple types, fast path

					switch( pdLegacy->type() )
					{
					default:
						// Fall to slow path
						break;

#define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
	case TYPE_ ## enum_name: { \
		c_type *data = (c_type *)mTraits->alloc(sizeof(c_type) * size); \
		if( ERROR_NONE != legacyHandle.getParam ## type_name ## Array(data, size) ) return false; \
		if( ERROR_NONE != handle.setParam ## type_name ## Array(data, size) ) return false; \
		mTraits->free(data); \
		break; \
	}
#include "NxParameterized_types.h"

					}
				}

				// Slow path for arrays of aggregates

				for(physx::PxI32 i = 0; i < size; ++i)
				{
					legacyHandle.set(i);
					handle.set(i);

					copy(legacyHandle, handle);

					handle.popIndex();
					legacyHandle.popIndex();
				}

				break;
			}

		case TYPE_REF:
			{
				Interface *legacyRefObj;
				legacyHandle.getParamRef(legacyRefObj);

				// Skip references which have unsupported classes (keep default version)
				if( legacyRefObj && -1 == handle.parameterDefinition()->refVariantValIndex(legacyRefObj->className()) )
					return true;

				Interface *refObj;
				handle.getParamRef(refObj);

				switch( mRefMode )
				{
				default:
					DEBUG_ALWAYS_ASSERT();
					break;

				case RefConversionMode::REF_CONVERT_SKIP:
					break;

				case RefConversionMode::REF_CONVERT_COPY:
					{
						// TODO: check that this class is allowed in new version?

						// Update to preferred version if necessary
						if( legacyRefObj && pdLegacy->isIncludedRef() )
						{
							const char *className = legacyRefObj->className();

							const MapName2Ver::Entry *e = mapName2Ver.find(pd->longName());
							if( e )
							{
								physx::PxU32 prefVer = e->second,
									curVer = legacyRefObj->version();

								if( curVer > prefVer )
								{
									NX_PARAM_TRAITS_WARNING(
										mTraits,
										"Unable to upgrade included reference of class %s, "
											"actual version (%u) is higher than preferred version (%u), "
											"your conversion may not work properly",
										className,
										(unsigned)curVer,
										(unsigned)prefVer
									);
									DEBUG_ALWAYS_ASSERT();
									return false;
								}
								else if( curVer < prefVer )
								{
									Interface *newLegacyRefObj = mTraits->createNxParameterized(className, prefVer);

									if( !mTraits->updateLegacyNxParameterized(*legacyRefObj, *newLegacyRefObj) )
									{
										NX_PARAM_TRAITS_WARNING(
											mTraits,
											"Failed to upgrade reference of class %s to version %u",
											className,
											(unsigned)prefVer
										);
										DEBUG_ALWAYS_ASSERT();
										newLegacyRefObj->destroy();
										return false;
									}

									legacyRefObj->destroy();
									legacyRefObj = newLegacyRefObj;

									legacyHandle.setParamRef(legacyRefObj);
								}
								else
								{
									// Object has preferred version => do not do anything
								}
							}
						}

						if( ERROR_NONE != handle.setParamRef(legacyRefObj) )
							return true;

						if( ERROR_NONE != legacyHandle.setParamRef(0) ) { //Prevent destruction in legacyObj->destroy()
							handle.setParamRef(0);
							return false;
						}

						if( refObj )
							refObj->destroy();

						return true;
					}

				case RefConversionMode::REF_CONVERT_UPDATE:
					{
						DEBUG_ASSERT( 0 && "This was not used/tested for a long time" );
						return false;

/*						if( !refObj )
						{
							//TODO: do we need to create refObj here?
							return false;
						}

						if( 0 != strcmp(legacyRefObj->className(), refObj->className()) )
						{
							if( NxParameterized::ERROR_NONE != handle.initParamRef(legacyRefObj->className(), true) )
								return false;

							if( NxParameterized::ERROR_NONE != handle.getParamRef(refObj) )
								return false;
						}

						if( legacyRefObj->version() != refObj->version() )
							return mTraits->updateLegacyNxParameterized(*legacyRefObj, *refObj);

						//Same version => recursive copy

						Handle refHandle(*refObj, "");
						DEBUG_ASSERT( refHandle.isValid() );

						Handle legacyRefHandle(*legacyRefObj, "");
						DEBUG_ASSERT( legacyRefHandle.isValid() );

						return copy(legacyRefHandle, refHandle);*/
					}
				} //switch( mRefMode )

				break;
			}

		case TYPE_POINTER:
			// do nothing?
			break;

		case TYPE_ENUM:
			{
				const char *val;
				legacyHandle.getParamEnum(val);

				physx::PxI32 valIdx = handle.parameterDefinition()->enumValIndex(val);
				if( -1 != valIdx )
				{
					const char *canonicVal = handle.parameterDefinition()->enumVal(valIdx);
					if( ERROR_NONE != handle.setParamEnum(canonicVal) )
					{
						NX_PARAM_TRAITS_WARNING(
							mTraits, "Internal error while setting enum value %s", canonicVal ? canonicVal : "" );
						DEBUG_ALWAYS_ASSERT();
						return false;
					}
				}

				break;
			}

#		define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#		define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#		define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
		case TYPE_##enum_name: { c_type val; legacyHandle.getParam##type_name(val); handle.setParam##type_name(val); break; }

		NX_PARAMETERIZED_TYPE(String, STRING, const char *)

#		include "NxParameterized_types.h"

		default:
			DEBUG_ALWAYS_ASSERT();
		}

		return true;
	}

public:
	DefaultParamConversion(Traits *traits, const PrefVer *prefVers, RefConversionMode::Enum refMode)
		: mTraits(traits), mRefMode(refMode), mapName2Ver(0, 0.75, Traits::Allocator(traits))
	{
		for(; prefVers && prefVers->longName; ++prefVers)
			mapName2Ver[prefVers->longName] = prefVers->ver;
	}

	bool operator()(NxParameterized::Interface &legacyObj, NxParameterized::Interface &obj)
	{
		obj.initDefaults();

		Handle legacyHandle(legacyObj, "");
		DEBUG_ASSERT( legacyHandle.isValid() );

		Handle handle(obj, "");
		DEBUG_ASSERT( handle.isValid() );

		return copy(legacyHandle, handle);
	}

	void release()
	{
		this->~DefaultParamConversion();
		mTraits->free(this);
	}
};

Conversion *internalCreateDefaultConversion(Traits *traits, const PrefVer *prefVers, RefConversionMode::Enum refMode)
{
	void *buf = traits->alloc(sizeof(DefaultParamConversion));
	return PX_PLACEMENT_NEW(buf, DefaultParamConversion)(traits, prefVers, refMode);
}

WrappedTraits::WrappedTraits(Traits *wrappedTraits_): wrappedTraits(wrappedTraits_) {}

void WrappedTraits::registerFactory( Factory & factory )
{
	wrappedTraits->registerFactory(factory);
}

Factory *WrappedTraits::removeFactory( const char * className )
{
	return wrappedTraits->removeFactory(className);
}

Factory *WrappedTraits::removeFactory( const char * className, physx::PxU32 version )
{
	return wrappedTraits->removeFactory(className, version);
}

NxParameterized::Interface * WrappedTraits::createNxParameterized( const char * name )
{
	return wrappedTraits->createNxParameterized(name);
}

NxParameterized::Interface * WrappedTraits::createNxParameterized( const char * name, physx::PxU32 ver )
{
	return wrappedTraits->createNxParameterized(name, ver);
}

NxParameterized::Interface * WrappedTraits::finishNxParameterized( const char * name, void *obj, void *buf, physx::PxI32 *refCount )
{
	return wrappedTraits->finishNxParameterized(name, obj, buf, refCount);
}

NxParameterized::Interface * WrappedTraits::finishNxParameterized( const char * name, physx::PxU32 ver, void *obj, void *buf, physx::PxI32 *refCount )
{
	return wrappedTraits->finishNxParameterized(name, ver, obj, buf, refCount);
}

physx::PxU32 WrappedTraits::getCurrentVersion(const char *className) const
{
	return wrappedTraits->getCurrentVersion(className);
}

physx::PxU32 WrappedTraits::getAlignment(const char *className, physx::PxU32 classVersion) const
{
	return wrappedTraits->getAlignment(className, classVersion);
}

void WrappedTraits::registerConversion(const char *className, physx::PxU32 from, physx::PxU32 to, Conversion &conv)
{
	return wrappedTraits->registerConversion(className, from, to, conv);
}

Conversion *WrappedTraits::removeConversion(const char *className, physx::PxU32 from, physx::PxU32 to)
{
	return wrappedTraits->removeConversion(className, from, to);
}

bool WrappedTraits::updateLegacyNxParameterized(NxParameterized::Interface &legacyObj, NxParameterized::Interface &obj)
{
	return wrappedTraits->updateLegacyNxParameterized(legacyObj, obj);
}

bool WrappedTraits::getNxParameterizedNames( const char ** names, physx::PxU32 &outCount, physx::PxU32 inCount) const
{
	return wrappedTraits->getNxParameterizedNames(names, outCount, inCount);
}

physx::PxI32 WrappedTraits::incRefCount(physx::PxI32 *refCount)
{
	return wrappedTraits->incRefCount(refCount);
}

physx::PxI32 WrappedTraits::decRefCount(physx::PxI32 *refCount)
{
	return wrappedTraits->decRefCount(refCount);
}

void WrappedTraits::onInplaceObjectDestroyed(void *buf, NxParameterized::Interface *obj)
{
	wrappedTraits->onInplaceObjectDestroyed(buf, obj);
}

void WrappedTraits::onAllInplaceObjectsDestroyed(void *buf)
{
	wrappedTraits->onAllInplaceObjectsDestroyed(buf);
}

void *WrappedTraits::alloc(physx::PxU32 nbytes)
{
	return wrappedTraits->alloc(nbytes);
}

void *WrappedTraits::alloc(physx::PxU32 nbytes, physx::PxU32 align)
{
	return wrappedTraits->alloc(nbytes, align);
}

void WrappedTraits::free(void *buf)
{
	wrappedTraits->free(buf);
}

char *WrappedTraits::strdup(const char *str)
{
	return wrappedTraits->strdup(str);
}

void WrappedTraits::strfree(char *str)
{
	wrappedTraits->strfree(str);
}

void WrappedTraits::traitsWarn(const char *msg) const
{
	wrappedTraits->traitsWarn(msg);
}

}; // end of namespace
