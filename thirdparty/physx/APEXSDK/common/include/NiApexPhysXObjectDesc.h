/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_APEX_PHYSX_OBJECT_DESC_H__
#define __NI_APEX_PHYSX_OBJECT_DESC_H__

#include "NxApexPhysXObjectDesc.h"


namespace physx
{
namespace apex
{

/**
 * Module/Asset interface to actor info structure.  This allows the asset to
 * set the various flags without knowing their implementation.
 */
class NiApexPhysXObjectDesc : public NxApexPhysXObjectDesc
{
public:
	void setIgnoreTransform(bool b)
	{
		if (b)
		{
			mFlags |= TRANSFORM;
		}
		else
		{
			mFlags &= ~(physx::PxU32)TRANSFORM;
		}
	};
	void setIgnoreRaycasts(bool b)
	{
		if (b)
		{
			mFlags |= RAYCASTS;
		}
		else
		{
			mFlags &= ~(physx::PxU32)RAYCASTS;
		}
	};
	void setIgnoreContacts(bool b)
	{
		if (b)
		{
			mFlags |= CONTACTS;
		}
		else
		{
			mFlags &= ~(physx::PxU32)CONTACTS;
		}
	};
	void setUserDefinedFlag(physx::PxU32 index, bool b)
	{
		if (b)
		{
			mFlags |= (1 << index);
		}
		else
		{
			mFlags &= ~(1 << index);
		}
	}

	/**
	\brief Implementation of pure virtual functions in NxApexPhysXObjectDesc, used for external (read-only)
	access to the NxApexActor list
	*/
	physx::PxU32				getApexActorCount() const
	{
		return mApexActors.size();
	}
	const NxApexActor*	getApexActor(physx::PxU32 i) const
	{
		return mApexActors[i];
	}


	void swap(NiApexPhysXObjectDesc& rhs)
	{
		mApexActors.swap(rhs.mApexActors);
		shdfnd::swap(mPhysXObject, rhs.mPhysXObject);

		shdfnd::swap(userData, rhs.userData);
		shdfnd::swap(mFlags, rhs.mFlags);
	}

	/**
	\brief Array of pointers to APEX actors assiciated with this PhysX object

	Pointers may be NULL in cases where the APEX actor has been deleted
	but PhysX actor cleanup has been deferred
	*/
	physx::Array<const NxApexActor*>	mApexActors;

	/**
	\brief the PhysX object which uses this descriptor
	*/
	const void* mPhysXObject;
protected:
	virtual ~NiApexPhysXObjectDesc(void) {}
};

}
} // end namespace physx::apex

#endif // __NI_APEX_PHYSX_OBJECT_DESC_H__
