/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "NiResourceProvider.h"
#include "ApexResourceProvider.h"
#include "PsUserAllocated.h"
#include "ApexSDK.h"
#include <ctype.h> // for toupper()

namespace physx
{
namespace apex
{

#pragma warning(disable: 4355)

ApexResourceProvider::ApexResourceProvider() 
: mNSNames(this, 0, false, 0)
, mCaseSensitive(false)
{
}

ApexResourceProvider::~ApexResourceProvider()
{
}

/* == Public NxResourceProvider interface == */
void ApexResourceProvider::registerCallback(NxResourceCallback* func)
{
	mUserCallback = func;
}

void ApexResourceProvider::setResource(const char* nameSpace, const char* name, void* resource, bool incRefCount)
{
	setResource(nameSpace, name, resource, true, incRefCount);
}

void ApexResourceProvider::setResource(const char* nameSpace, const char* name, void* resource, bool valueIsSet, bool incRefCount)
{
	PX_ASSERT(nameSpace);
	PX_ASSERT(name);
	NxResID nsID = createNameSpaceInternal(nameSpace, true);
	PX_ASSERT(nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size());
	if (nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size())
	{
		NxResID id = mNameSpaces[nsID]->getOrCreateID(name, nameSpace);
		PX_ASSERT(id != INVALID_RESOURCE_ID && id < mResources.size());
		if (id != INVALID_RESOURCE_ID && id < mResources.size())
		{
			ApexResourceProvider::resource& res = mResources[id];
			res.ptr = resource;
			res.valueIsSet = (physx::PxU8)valueIsSet;
			if (incRefCount)
			{
				res.refCount++;
			}
		}
	}
}

void* ApexResourceProvider::getResource(const char* nameSpace, const char* name)
{
	PX_ASSERT(nameSpace);
	PX_ASSERT(name);
	NxResID nsID = createNameSpaceInternal(nameSpace, true);
	PX_ASSERT(nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size());
	if (nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size())
	{
		NxResID id = mNameSpaces[nsID]->getOrCreateID(name, nameSpace);
		PX_ASSERT(id != INVALID_RESOURCE_ID && id < mResources.size());
		if (id != INVALID_RESOURCE_ID && id < mResources.size())
		{
			return getResource(id);
		}
	}
	return NULL;
}

/* == Internal NiResourceProvider interface == */
NxResID ApexResourceProvider::createNameSpaceInternal(const char* &nameSpace, bool releaseAtExit)
{
	/* create or get a name space */
	size_t  nextID = mResources.size();
	NxResID nsID = mNSNames.getOrCreateID(nameSpace, "NameSpace");
	if (nsID == (NxResID) nextID)
	{
		NameSpace* ns = PX_NEW(NameSpace)(this, nsID, releaseAtExit, nameSpace);
		if (ns)
		{
			mResources[nsID].ptr = (void*)(size_t) mNameSpaces.size();
			mNameSpaces.pushBack(ns);
		}
		else
		{
			return INVALID_RESOURCE_ID;
		}
	}
	NxResID ret = (NxResID)(size_t) mResources[nsID].ptr;
	return ret;
}

NxResID ApexResourceProvider::createResource(NxResID nameSpace, const char* name, bool refCount)
{
	PX_ASSERT(nameSpace < mNameSpaces.size());
	if (nameSpace < mNameSpaces.size())
	{
		NameSpace* ns = mNameSpaces[nameSpace];
		NxResID id = ns->getOrCreateID(name, mResources[ns->getID()].name);
		PX_ASSERT(id < mResources.size());
		if (id < mResources.size() && refCount)
		{
			mResources[id].refCount++;
		}
		return id;
	}
	else
	{
		return INVALID_RESOURCE_ID;
	}
}

bool ApexResourceProvider::checkResource(NxResID nameSpace, const char* name)
{
	/* Return true is named resource has known non-null pointer */
	PX_ASSERT(nameSpace < mNameSpaces.size());
	if (nameSpace < mNameSpaces.size())
	{
		NameSpace* ns = mNameSpaces[nameSpace];
		NxResID id = ns->getOrCreateID(name, mResources[ns->getID()].name);
		PX_ASSERT(id < mResources.size());
		return checkResource(id);
	}
	return false;
}

bool ApexResourceProvider::checkResource(NxResID id)
{
	if (mResources.size() <= id)
	{
		return false;
	}

	ApexResourceProvider::resource& res = mResources[id];

	if (!res.valueIsSet)
	{
		return false;
	}

	return true;
}

void ApexResourceProvider::generateUniqueName(NxResID nameSpace, ApexSimpleString& name)
{
	PX_ASSERT(nameSpace < mNameSpaces.size());
	if (nameSpace < mNameSpaces.size())
	{
		ApexSimpleString test;
		physx::PxU32   count = 1;
		char	buf[64];
		*buf = '.';

		do
		{
			physx::string::sprintf_s(buf + 1,60, "%d", count);
			test = name + ApexSimpleString(buf);
			if (!checkResource(nameSpace, test.c_str()))
			{
				break;
			}
		}
		while (++count < 0xFFFFFFF0);

		name = test;
	}
}

void ApexResourceProvider::releaseResource(NxResID id)
{
	if (mResources.size() <= id)
	{
		return;
	}

	ApexResourceProvider::resource& res = mResources[id];

	PX_ASSERT(res.refCount);
	if (res.refCount > 0)
	{
		res.refCount--;

		if (res.refCount == 0 && mUserCallback &&
		        res.valueIsSet && res.ptr != NULL)
		{
			if (mResources[id].usedGetResource)   // Defect DE641 only callback to the user, if this resource was created by a GetResource request
			{
				mUserCallback->releaseResource(res.nameSpace, res.name, res.ptr);
			}
			res.ptr = (void*) UnknownValue;
			res.valueIsSet = false;
			res.usedGetResource = false;
		}
		// if the ptr is NULL and we're releasing it, we do want to call requestResource next time it is requested, so valueIsSet = false
		else if (res.refCount == 0 && res.valueIsSet && res.ptr == NULL)
		{
			res.ptr = (void*) UnknownValue;
			res.valueIsSet = false;
			res.usedGetResource = false;
		}
	}
}

void* ApexResourceProvider::getResource(NxResID id)
{
	if (mResources.size() <= id)
	{
		return NULL;
	}
	else if (!mResources[id].valueIsSet)
	{
		// PH: WARNING: This MUST not be a reference, mResource can be altered during the requestResource() operation!!!!
		ApexResourceProvider::resource res = mResources[id];
		if (mUserCallback)
		{
			// tmp ensures that the [] operator is called AFTER mResources is possibly
			// resized by something in requestResources
			void* tmp = mUserCallback->requestResource(res.nameSpace, res.name);
			res.ptr = tmp;
			res.valueIsSet = true;
			res.usedGetResource = true;
		}
		else
		{
			res.ptr = NULL;
		}
		mResources[id] = res;
	}
	return mResources[id].ptr;
}

const char* ApexResourceProvider::getResourceName(NxResID id)
{
	if (mResources.size() <= id)
	{
		return NULL;
	}
	return mResources[id].name;
}

bool ApexResourceProvider::getResourceIDs(const char* nameSpace, NxResID* outResIDs, physx::PxU32& outCount, physx::PxU32 inCount)
{
	outCount = 0;

	if (!outResIDs)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < mResources.size(); i++)
	{
		if (stringsMatch(mResources[i].nameSpace, nameSpace))
		{
			if (outCount > inCount)
			{
				outCount = 0;
				return false;
			}
			outResIDs[outCount++] = i;
		}
	}

	return true;
}

void ApexResourceProvider::destroy()
{
	if (mUserCallback)
	{
		for (physx::PxU32 i = 0 ; i < mResources.size() ; i++)
		{
			ApexResourceProvider::resource& res = mResources[i];
			if (res.refCount != 0 && res.valueIsSet && res.ptr != NULL)
			{
				NxResID nsID = mNSNames.getOrCreateID(res.nameSpace, "NameSpace");
				if (nsID < mNameSpaces.size() && 
					mNameSpaces[nsID]->releaseAtExit())
				{
					if (res.usedGetResource) // this check added for PhysXLab DE4349
				{
					mUserCallback->releaseResource(res.nameSpace, res.name, res.ptr);
				}
					else
					{
						APEX_DEBUG_WARNING("Unreleased resource found during teardown: Namespace <%s>, Name <%s>", res.nameSpace, res.name);
					}
				}
			}
		}
	}
	mResources.clear();
	for (physx::PxU32 i = 0 ; i < mNameSpaces.size() ; i++)
	{
		PX_DELETE(mNameSpaces[i]);
	}
	mNameSpaces.clear();
	delete this;
}

ApexResourceProvider::NameSpace::NameSpace(ApexResourceProvider* arp, NxResID nsid, bool releaseAtExit, const char* nameSpace) :
	mReleaseAtExit(releaseAtExit),
	mArp(arp),
	mId(nsid)
{
	memset(hash, 0, sizeof(hash));
	mNameSpace = 0;
	if (nameSpace)
	{
		physx::PxU32 len = (physx::PxU32) strlen(nameSpace);
		mNameSpace = (char*)PX_ALLOC(len + 1, PX_DEBUG_EXP("ApexResourceProvider::NameSpace"));
		memcpy(mNameSpace, nameSpace, len + 1);
	}
}

ApexResourceProvider::NameSpace::~NameSpace()
{
	// Free up all collision chains in the hash table
	for (physx::PxU32 i = 0 ; i < HashSize ; i++)
	{
		while (hash[i])
		{
			const char* entry = hash[i];
			const entryHeader* hdr = (const entryHeader*) entry;
			const char* next = hdr->nextEntry;
			PX_FREE((void*) entry);
			hash[i] = next;
		}
	}
	PX_FREE(mNameSpace);
}

NxResID	ApexResourceProvider::NameSpace::getOrCreateID(const char* &name, const char* NSName)
{
	/* Hash Table Entry:   | nextEntry* | NxResID | name     | */
	physx::PxU16 h = genHash(name);
	const char* entry = hash[h];

	while (entry)
	{
		entryHeader* hdr = (entryHeader*) entry;
		const char* entryName = entry + sizeof(entryHeader);

		if (mArp->stringsMatch(name, entryName))
		{
			name = entryName;
			return hdr->id;
		}

		entry = hdr->nextEntry;
	}

	size_t len = strlen(name);
	size_t bufsize = len + 1 + sizeof(entryHeader);
	char* newEntry = (char*) PX_ALLOC(bufsize, PX_DEBUG_EXP("ApexResourceProvider::NameSpace::getOrCreateID"));
	if (newEntry)
	{
#if defined(WIN32)
		strncpy_s(newEntry + sizeof(entryHeader), bufsize - sizeof(entryHeader), name, len);
#else
		strcpy(newEntry + sizeof(entryHeader), name);
#endif
		entryHeader* hdr = (entryHeader*) newEntry;
		hdr->nextEntry = hash[h];
		hdr->id = mArp->mResources.size();

		resource res;
		res.ptr = (void*) UnknownValue;
		res.valueIsSet = false;
		res.name = newEntry + sizeof(entryHeader);
		res.nameSpace = NSName;
		res.refCount = 0;
		res.usedGetResource = 0;
		mArp->mResources.pushBack(res);

		hash[h] = (const char*) newEntry;

		name = res.name;
		return hdr->id;
	}

	return INVALID_RESOURCE_ID;
}

physx::PxU16 ApexResourceProvider::NameSpace::genHash(const char* name)
{
	PX_ASSERT(name != NULL);
	/* XOR each 32bit word together */
	physx::PxU32 h = 0;
	physx::PxU32* read32 = (physx::PxU32*)name;
	size_t len = strlen(name);

	/* Add remaining bytes */
	physx::PxU8* read8 = (physx::PxU8*) read32;
	while (len)
	{
		if (mArp->isCaseSensitive())
		{
			h ^= *read8;
		}
		else
		{
			h ^= toupper(*read8);
		}
		read8++;
		len -= sizeof(physx::PxU8);
	}

	/* XOR fold top 16 bits over bottom 16 bits */
	h ^= (h >> 16);

	return (physx::PxU16)(h & (HashSize - 1));
}

void ApexResourceProvider::dumpResourceTable()
{
	APEX_DEBUG_INFO("ApexResourceProvider::dumpResourceTable");
	APEX_DEBUG_INFO("namespace name refcount pointer valueIsSet");
#if 0
	for (physx::PxU32 i = 0; i < mNameSpaces.size(); i++)
	{
		NxResID nsID = mNameSpaces[i]->getID();

		mArp->mResources.pushBack(res);
	}
#endif

	for (physx::PxU32 i = 0; i < mResources.size(); i++)
	{
		APEX_DEBUG_INFO("%s %s %d 0x%08x %d", mResources[i].nameSpace, mResources[i].name, mResources[i].refCount, mResources[i].ptr, mResources[i].valueIsSet);
	}
}


void   			ApexResourceProvider::setResourceU32(const char* nameSpace, const char* name, physx::PxU32 id, bool incRefCount)
{
	setResource(nameSpace, name, (void*)(size_t)id, true, incRefCount);
}

physx::PxU32  			ApexResourceProvider::releaseAllResourcesInNamespace(const char* nameSpace)
{
	physx::PxU32 ret = 0;

	for (physx::PxU32 i = 0; i < mResources.size(); i++)
	{
		ApexResourceProvider::resource& res = mResources[i];
		if (stringsMatch(res.nameSpace, nameSpace) &&  res.valueIsSet)
		{
			ret++;
			PX_ASSERT(res.refCount);
			if (res.refCount > 0)
			{
				res.refCount--;
				if (res.refCount == 0 && mUserCallback &&
				        res.valueIsSet && res.ptr != NULL)
				{
					mUserCallback->releaseResource(res.nameSpace, res.name, res.ptr);
					res.ptr = (void*) UnknownValue;
					res.valueIsSet = false;
				}
			}
		}
	}

	return ret;
}

physx::PxU32  			ApexResourceProvider::releaseResource(const char* nameSpace, const char* name)
{
	physx::PxU32 ret = 0;

	PX_ASSERT(nameSpace);
	PX_ASSERT(name);
	NxResID nsID = createNameSpaceInternal(nameSpace, true);
	PX_ASSERT(nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size());
	if (nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size())
	{
		NxResID id = mNameSpaces[nsID]->getOrCreateID(name, nameSpace);
		PX_ASSERT(id != INVALID_RESOURCE_ID && id < mResources.size());
		if (id != INVALID_RESOURCE_ID && id < mResources.size())
		{
			ApexResourceProvider::resource& res = mResources[id];
			if (res.valueIsSet)
			{
				ret = (physx::PxU32)res.refCount - 1;
				releaseResource(id);
			}
		}
	}


	return ret;
}

bool    		ApexResourceProvider::findRefCount(const char* nameSpace, const char* name, physx::PxU32& refCount)
{
	bool ret = false;
	refCount = 0;
	PX_ASSERT(nameSpace);
	PX_ASSERT(name);
	NxResID nsID = createNameSpaceInternal(nameSpace, true);
	PX_ASSERT(nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size());
	if (nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size())
	{
		NxResID id = mNameSpaces[nsID]->getOrCreateID(name, nameSpace);
		PX_ASSERT(id != INVALID_RESOURCE_ID && id < mResources.size());
		if (id != INVALID_RESOURCE_ID && id < mResources.size())
		{
			if (mResources[id].valueIsSet)
			{
				ret = true;
				refCount = mResources[id].refCount;
			}
		}
	}

	return ret;
}

void* 			ApexResourceProvider::findResource(const char* nameSpace, const char* name)
{
	void* ret = NULL;
	PX_ASSERT(nameSpace);
	PX_ASSERT(name);
	NxResID nsID = createNameSpaceInternal(nameSpace, true);
	PX_ASSERT(nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size());
	if (nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size())
	{
		NxResID id = mNameSpaces[nsID]->getOrCreateID(name, nameSpace);
		PX_ASSERT(id != INVALID_RESOURCE_ID && id < mResources.size());
		if (id != INVALID_RESOURCE_ID && id < mResources.size())
		{
			if (mResources[id].valueIsSet)
			{
				ret = mResources[id].ptr;
			}
		}
	}
	return ret;
}

physx::PxU32 			ApexResourceProvider::findResourceU32(const char* nameSpace, const char* name) // find an existing resource.
{
	physx::PxU32 ret = 0;
	PX_ASSERT(nameSpace);
	PX_ASSERT(name);
	NxResID nsID = createNameSpaceInternal(nameSpace, true);
	PX_ASSERT(nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size());
	if (nsID != INVALID_RESOURCE_ID && nsID < mNameSpaces.size())
	{
		NxResID id = mNameSpaces[nsID]->getOrCreateID(name, nameSpace);
		PX_ASSERT(id != INVALID_RESOURCE_ID && id < mResources.size());
		if (id != INVALID_RESOURCE_ID && id < mResources.size())
		{
			if (mResources[id].valueIsSet)
			{
#ifdef PX_X64
				physx::PxU64 ret64 = (physx::PxU64)mResources[id].ptr;
				ret = (physx::PxU32)ret64;
#else
				ret = (physx::PxU32)(size_t)(mResources[id].ptr);
#endif
			}
		}
	}
	return ret;

}

void** 		ApexResourceProvider::findAllResources(const char* nameSpace, physx::PxU32& count) // find all resources in this namespace
{
	void** ret = 0;
	count = 0;

	mCharResults.clear();
	for (physx::PxU32 i = 0; i < mResources.size(); i++)
	{
		if (stringsMatch(nameSpace, mResources[i].nameSpace))
		{
			if (mResources[i].valueIsSet)
			{
				mCharResults.pushBack((const char*)mResources[i].ptr);
			}
		}
	}
	if (!mCharResults.empty())
	{
		ret = (void**)&mCharResults[0];
		count = mCharResults.size();
	}

	return ret;
}

const char** 		ApexResourceProvider::findAllResourceNames(const char* nameSpace, physx::PxU32& count) // find all resources in this namespace
{
	const char** ret = 0;
	count = 0;

	mCharResults.clear();
	for (physx::PxU32 i = 0; i < mResources.size(); i++)
	{
		if (stringsMatch(nameSpace, mResources[i].nameSpace) && mResources[i].valueIsSet)
		{
			mCharResults.pushBack(mResources[i].name);
		}
	}
	if (!mCharResults.empty())
	{
		ret = &mCharResults[0];
		count = mCharResults.size();
	}

	return ret;
}

const char** 	ApexResourceProvider::findNameSpaces(physx::PxU32& count)
{
	const char** ret = 0;
	count = 0;

	mCharResults.clear();
	for (physx::Array<NameSpace*>::Iterator i = mNameSpaces.begin(); i != mNameSpaces.end(); ++i)
	{
		const char* nameSpace = (*i)->getNameSpace();
		if (nameSpace)
		{
			mCharResults.pushBack(nameSpace);
		}
	}

	if (!mCharResults.empty())
	{
		count = mCharResults.size();
		ret = &mCharResults[0];
	}

	return ret;
}

bool ApexResourceProvider::stringsMatch(const char* str0, const char* str1)
{
	if (mCaseSensitive)
	{
		return !strcmp(str0, str1);
	}
	else
	{
		return !physx::string::stricmp(str0, str1);
	}

}


}
} // end namespace physx::apex
