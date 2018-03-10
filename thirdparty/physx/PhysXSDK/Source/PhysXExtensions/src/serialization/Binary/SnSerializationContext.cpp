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

#include "PxBase.h"
#include "SnSerializationContext.h"
#include "PxCollection.h"

using namespace physx;
using namespace Sn;

PxBase* DeserializationContext::resolveReference(PxU32 kind, size_t reference) const
{
	const InternalRefMap::Entry* entry0 = mInternalReferencesMap.find(InternalRefKey(reference, kind));
	PX_ASSERT(entry0);
	SerialObjectIndex objIndex = entry0->second;
	bool isExternal;
	PxU32 index = objIndex.getIndex(isExternal);
	PxBase* base = NULL;
	if (isExternal)
	{
		const ImportReference& entry = mImportReferences[index];
		base = mExternalRefs->find(entry.id);	
	}
	else
	{
		const ManifestEntry& entry = mManifestTable[index];
		base = reinterpret_cast<PxBase*>(mObjectDataAddress + entry.offset);
	}
	PX_ASSERT(base);
	return base;
}
	
void SerializationContext::registerReference(PxBase& serializable, PxU32 kind, size_t reference)
{
#ifdef PX_CHECKED
	if ((kind & PX_SERIAL_REF_KIND_PTR_TYPE_BIT) == 0 && reference > 0xffffffff)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxSerializationContext::registerReference: only 32 bit indices supported.");
		return;
	}
#endif

	bool isExternal = mExternalRefs && mExternalRefs->contains(serializable);
	PxU32 index;
	if (isExternal)
	{
		PxSerialObjectId id = mExternalRefs->getId(serializable);
		PX_ASSERT(id != PX_SERIAL_OBJECT_ID_INVALID);
		if (const Ps::HashMap<PxSerialObjectId, PxU32>::Entry* entry = mImportReferencesMap.find(id))
		{
			index = entry->second;
		}
		else
		{
			index = mImportReferences.size();
			mImportReferencesMap.insert(id, index);
			mImportReferences.pushBack(ImportReference(id, serializable.getConcreteType()));
		}
	}
	else
	{
		PX_ASSERT(mCollection.contains(serializable));
		index = mObjToCollectionIndexMap[&serializable];
	}

	InternalRefMap& targetMap = (kind & PX_SERIAL_REF_KIND_PTR_TYPE_BIT) ? mInternalReferencesPtrMap : mInternalReferencesIdxMap;
	targetMap[InternalRefKey(reference, kind)] = SerialObjectIndex(index, isExternal);
}
