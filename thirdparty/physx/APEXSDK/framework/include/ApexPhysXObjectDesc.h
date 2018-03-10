/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_PHYSX_OBJECT_DESC_H__
#define __APEX_PHYSX_OBJECT_DESC_H__

#include "NxApex.h"
#include "NiApexPhysXObjectDesc.h"

namespace physx
{
namespace apex
{

class ApexPhysXObjectDesc : public NiApexPhysXObjectDesc
{
public:
	typedef NiApexPhysXObjectDesc Parent;
	ApexPhysXObjectDesc() : mNext(0), mPrev(0)
	{
		mFlags = 0;
		userData = NULL;
		mPhysXObject = NULL;
	}

	// Need a copy constructor because we contain an array, and we are in arrays
	ApexPhysXObjectDesc(const ApexPhysXObjectDesc& desc) : NiApexPhysXObjectDesc(desc)
	{
		*this = desc;
	}

	ApexPhysXObjectDesc& operator = (const ApexPhysXObjectDesc& desc)
	{
		mFlags = desc.mFlags;
		userData = desc.userData;
		mApexActors = desc.mApexActors;
		mPhysXObject = desc.mPhysXObject;
		mNext = desc.mNext;
		mPrev = desc.mPrev;
		return *this;
	}

	void swap(ApexPhysXObjectDesc& rhs)
	{
		Parent::swap(rhs);
		shdfnd::swap(mNext, rhs.mNext);
		shdfnd::swap(mPrev, rhs.mPrev);
	}

	static physx::PxU16 makeHash(size_t hashable);

	physx::PxU32		mNext, mPrev;

	friend class ApexSDK;
	virtual ~ApexPhysXObjectDesc(void)
	{

	}
};

}
} // end namespace physx::apex

#endif // __APEX_PHYSX_OBJECT_DESC_H__
