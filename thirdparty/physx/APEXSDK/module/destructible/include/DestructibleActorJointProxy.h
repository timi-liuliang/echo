/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEACTORJOINT_PROXY_H__
#define __DESTRUCTIBLEACTORJOINT_PROXY_H__

#include "NxApex.h"
#include "NxDestructibleActorJoint.h"
#include "DestructibleActorJoint.h"
#include "PsUserAllocated.h"
#include "ApexRWLockable.h"
#include "ReadCheck.h"
#include "WriteCheck.h"

namespace physx
{
namespace apex
{

class DestructibleScene;	// Forward declaration

namespace destructible
{

class DestructibleActorJointProxy : public NxDestructibleActorJoint, public NxApexResource, public physx::UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	DestructibleActorJoint impl;

#pragma warning(disable : 4355) // disable warning about this pointer in argument list
	DestructibleActorJointProxy(const NxDestructibleActorJointDesc& destructibleActorJointDesc, DestructibleScene& dscene, NxResourceList& list) :
		impl(destructibleActorJointDesc, dscene)
	{
		list.add(*this);
	};

	~DestructibleActorJointProxy()
	{
	};

	// NxDestructibleActorJoint methods
	virtual NxJoint* joint()
	{
		NX_WRITE_ZONE();
		return impl.getJoint();
	}

	virtual void release()
	{
		// impl.release();
		delete this;
	};

	// NxApexResource methods
	virtual void	setListIndex(NxResourceList& list, physx::PxU32 index)
	{
		impl.m_listIndex = index;
		impl.m_list = &list;
	}

	virtual physx::PxU32	getListIndex() const
	{
		return impl.m_listIndex;
	}
};

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLEACTORJOINT_PROXY_H__
