/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_RESOURCE_PROVIDER_H__
#define __APEX_RESOURCE_PROVIDER_H__

#include "NxApex.h"
#include "PsUserAllocated.h"
#include "NiResourceProvider.h"
#include "ApexString.h"
#include "PsArray.h"

namespace physx
{
namespace apex
{

class ApexResourceProvider : public NiResourceProvider, public physx::UserAllocated
{
public:
	/* == Public NxResourceProvider interface == */
	virtual void 			registerCallback(NxResourceCallback* impl);
	virtual void 			setResource(const char* nameSpace, const char* name, void* resource, bool incRefCount);
	virtual void   			setResourceU32(const char* nameSpace, const char* name, physx::PxU32 id, bool incRefCount);
	virtual void* 			getResource(const char* nameSpace, const char* name);
	virtual physx::PxU32  			releaseAllResourcesInNamespace(const char* nameSpace);
	virtual physx::PxU32  			releaseResource(const char* nameSpace, const char* name);
	virtual bool    		findRefCount(const char* nameSpace, const char* name, physx::PxU32& refCount);
	virtual void* 			findResource(const char* nameSpace, const char* name);
	virtual physx::PxU32 			findResourceU32(const char* nameSpace, const char* name); // find an existing resource.
	virtual void** 		findAllResources(const char* nameSpace, physx::PxU32& count); // find all resources in this namespace
	virtual const char** 	findAllResourceNames(const char* nameSpace, physx::PxU32& count); // find all resources in this namespace
	virtual const char** 	findNameSpaces(physx::PxU32& count);
	virtual void		 	dumpResourceTable();

	/* == Internal NiResourceProvider interface == */
	void	setResource(const char* nameSpace, const char* name, void* resource, bool valueIsSet, bool incRefCount);
	NxResID createNameSpace(const char* nameSpace, bool releaseAtExit)
	{
		return createNameSpaceInternal(nameSpace, releaseAtExit);
	}
	NxResID createResource(NxResID nameSpace, const char* name, bool refCount);
	bool	checkResource(NxResID nameSpace, const char* name);
	bool	checkResource(NxResID id);
	void    releaseResource(NxResID id);
	void	generateUniqueName(NxResID nameSpace, ApexSimpleString& name);
	void*   getResource(NxResID id);
	const char* getResourceName(NxResID id);
	bool	getResourceIDs(const char* nameSpace, NxResID* outResIDs, physx::PxU32& outCount, physx::PxU32 inCount);

	// [PVD INTEGRATION CODE] ===========================================
	PX_INLINE physx::PxU32	getResourceCount() const
	{
		return mResources.size();
	}
	// ==================================================================

	// the NRP can either be case sensitive or not, this method takes care of that option
	PX_INLINE bool isCaseSensitive()
	{
		return mCaseSensitive;
	}
	
	/**
	\brief Sets the resource provider's case sensitive mode.
	
	\note This must be done immediately after initialization so no names are hashed
		  using the wrong mode.
	*/
	void	setCaseSensitivity(bool caseSensitive)
	{
		mCaseSensitive = caseSensitive;
	}

	// uses the correct string matching function based on the case sensitivity mode
	bool	stringsMatch(const char* str0, const char* str1);

protected:
	ApexResourceProvider();
	virtual ~ApexResourceProvider();
	void destroy();

private:
	NxResID createNameSpaceInternal(const char* &nameSpace, bool releaseAtExit);

	class NameSpace: public physx::UserAllocated
	{
	public:
		NameSpace(ApexResourceProvider* arp, NxResID nsid, bool releaseAtExit, const char* nameSpace);
		~NameSpace();

		NxResID	getOrCreateID(const char* &name, const char* NSName);
		NxResID getID() const
		{
			return mId;
		}
		bool    releaseAtExit() const
		{
			return mReleaseAtExit;
		}
		const char* getNameSpace(void) const
		{
			return mNameSpace;
		};

	private:
		struct entryHeader
		{
			const char* nextEntry;
			NxResID     id;
		};
		enum { HashSize = 1024 };
		bool				  mReleaseAtExit;
		physx::PxU16				  genHash(const char* name);
		const char*			  hash[HashSize];
		ApexResourceProvider* mArp;
		NxResID               mId;
		char* 				  mNameSpace;
	};

	// NOTE: someone thinks this struct needs to be padded to nearest 16 or 32 bytes
	// it's not padded at the moment, but watch out for this
	struct resource
	{
		void* ptr;
		const char* name;
		const char* nameSpace;
		physx::PxU16 refCount;
		physx::PxU8  valueIsSet;
		physx::PxU8	 usedGetResource;
	};

	NameSpace				mNSNames;
	physx::Array<resource>   mResources;
	physx::Array<NameSpace*> mNameSpaces;
	NxResourceCallback*   	mUserCallback;
	physx::Array< const char* > mCharResults;
	bool						mCaseSensitive;


	enum { UnknownValue = 0xFFFFFFFF };

	friend class ApexSDK;
};

}
} // end namespace physx::apex

#endif // __APEX_RESOURCE_PROVIDER_H__
