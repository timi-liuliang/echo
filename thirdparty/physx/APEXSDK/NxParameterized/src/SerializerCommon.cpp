/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "SerializerCommon.h"
#include "NxTraitsInternal.h"

namespace NxParameterized
{

#define CHECK(x) NX_BOOL_ERR_CHECK_RETURN(x, 0)

bool UpgradeLegacyObjects(Serializer::DeserializedData &data, bool &isUpdated, Traits *t)
{
	isUpdated = false;

	for(physx::PxU32 i = 0; i < data.size(); ++i)
	{
		Interface *obj = data[i];
		if( !obj )
			continue;

		Interface *newObj = UpgradeObject(*obj, isUpdated, t);
		if( !newObj )
		{
			NX_PARAM_TRAITS_WARNING(
				t,
				"Failed to upgrade object of class %s and version %u",
				obj->className(),
				(unsigned)obj->version());

			DEBUG_ALWAYS_ASSERT();

			for(physx::PxU32 j = 0; j < data.size(); ++j)
				data[i]->destroy();

			data.init(0, 0);

			return false;
		}

		if( newObj != obj )
		{
			// Need to retain the name of the old object into the new object
			const char *name = obj->name();
			newObj->setName(name);
			obj->destroy();
			data[i] = newObj;
		}
	}

	return true;
}

bool UpgradeIncludedRefs(Handle &h, bool &isUpdated, Traits *t)
{
	const Definition *pd = h.parameterDefinition();

	switch( pd->type() )
	{
	default:
		break;

	case TYPE_ARRAY:
		{
			if( pd->child(0)->isSimpleType() )
				break;

			physx::PxI32 size;
			CHECK( NxParameterized::ERROR_NONE == h.getArraySize(size) );

			for(physx::PxI32 i = 0; i < size; ++i)
			{
				h.set(i);
				CHECK( UpgradeIncludedRefs(h, isUpdated, t) );
				h.popIndex();
			}

			break;
		}

	case TYPE_STRUCT:
		{
			if( pd->isSimpleType() )
				break;

			for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
			{
				h.set(i);
				CHECK( UpgradeIncludedRefs(h, isUpdated, t) );
				h.popIndex();
			}

			break;
		}

	case TYPE_REF:
		{
			if( !pd->isIncludedRef() )
				break;

			Interface *refObj = 0;
			h.getParamRef(refObj);

			if( !refObj ) // No reference there?
				break;

			Interface *newRefObj = UpgradeObject(*refObj, isUpdated, t);
			CHECK( newRefObj );

			if( newRefObj == refObj ) // No update?
				break;

			refObj->destroy();

			if( NxParameterized::ERROR_NONE != h.setParamRef(newRefObj) )
			{
				DEBUG_ALWAYS_ASSERT();
				newRefObj->destroy();
				return false;
			}

			break;
		}
	}

	return true;
}

bool UpgradeIncludedRefs(Interface &obj, bool &isUpdated, Traits *t)
{
	Handle h(obj, "");
	CHECK( h.isValid() );

	return UpgradeIncludedRefs(h, isUpdated, t);
}

Interface *UpgradeObject(Interface &obj, bool &isUpdated, Traits *t)
{
	const char *className = obj.className();

	Interface *newObj = &obj;

	if( obj.version() != t->getCurrentVersion(className) )
	{
		isUpdated = true;

		newObj = t->createNxParameterized(className);

		if( !newObj )
		{
			NX_PARAM_TRAITS_WARNING(t, "Failed to create object of class %s", className);
			DEBUG_ALWAYS_ASSERT();
			return 0;
		}

		if( !t->updateLegacyNxParameterized(obj, *newObj) )
		{
			NX_PARAM_TRAITS_WARNING(t, "Failed to upgrade object of class %s and version %u",
				className,
				(unsigned)obj.version() );
			newObj->destroy();
			return 0;
		}
	}

	if( !UpgradeIncludedRefs(*newObj, isUpdated, t) )
	{
		newObj->destroy();
		return 0;
	}

	return newObj;
}

void *serializerMemAlloc(physx::PxU32 size, Traits *t)
{
	if( t )
		return t->alloc(size);
	else
	{
		DEBUG_ALWAYS_ASSERT(); // indicates a memory leak
		return ::malloc(size);
	}
}

void serializerMemFree(void *data, Traits *t)
{
	if( t )
		t->free(data);
	else
	{
		DEBUG_ALWAYS_ASSERT();
		::free(data);
	}
}

bool DoIgnoreChecksum(const NxParameterized::Interface &obj)
{
	// Most of our classes initially do not have classVersion field.
	// When it is finally added (e.g. after adding new version)
	// schema checksum changes and we get invalid "checksum not equal" warnings;
	// because of that we ignore checksum differences for all 0.0 classes.
	return 0 == obj.version();
}

} // namespace NxParameterized
