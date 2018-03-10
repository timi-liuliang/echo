/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_DESTRUCTIBLE_ACTOR_JOINT_H
#define NX_DESTRUCTIBLE_ACTOR_JOINT_H

#include "NxModuleDestructible.h"

#if NX_SDK_VERSION_MAJOR == 2
class NxJointDesc;
#elif NX_SDK_VERSION_MAJOR == 3
#include "extensions/PxJoint.h"
#endif

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
	Descriptor used to create the Destructible actor joint.
*/
class NxDestructibleActorJointDesc : public NxApexDesc
{
public:

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleActorJointDesc();

	/**
	\brief Resets descriptor to default settings.
	*/
	PX_INLINE void	setToDefault();

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const;

#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/**
		PhysX SDK 2.8.X only.
		The PhysX SDK joint descriptor.  Note - localAnchor[i], localAxis[i], and localNormal[i]
		are not used if actor[i] is part of an NxDestructibleActor.
	*/
	NxJointDesc*			jointDesc;
#endif
#if (NX_SDK_VERSION_MAJOR == 3) || defined(DOXYGEN)
	/**
		PhysX SDK 3.X only.
	*/
	PxJointConcreteType::Enum		type;
	/**
		PhysX SDK 3.X only.
	*/
	PxRigidActor*			actor[2];
	/**
		PhysX SDK 3.X only.
	*/
	PxVec3					localAxis[2];
	/**
		PhysX SDK 3.X only.
	*/
	PxVec3					localAnchor[2];
	/**
		PhysX SDK 3.X only.
	*/
	PxVec3					localNormal[2];
#endif

	/**
		If destructible[i] is not NULL, it will effectively replace the actor[i] in jointDesc.
		At least one must be non-NULL.
	*/
	NxDestructibleActor*	destructible[2];

	/**
		If destructible[i] is not NULL and attachmentChunkIndex[i] is a valid chunk index within that
		destructible, then that chunk will be used for attachment.
		If destructible[i] is not NULL and attachmentChunkIndex[i] is NOT a valid chunk index (such as the
		default NxModuleDestructibleConst::INVALID_CHUNK_INDEX), then the nearest chunk to globalAnchor[i] is used instead.
		Once a chunk is determined, the chunk's associated NxActor will be the NxJoint's attachment actor.
	*/
	physx::PxI32					attachmentChunkIndex[2];


	/**
		Global attachment data.  Since destructibles come apart, there is no single referece frame
		associated with them.  Therefore it makes more sense to use a global reference frame
		when describing the attachment positions and axes.
	*/

	/**
	\brief global attachment positions
	*/
	physx::PxVec3			globalAnchor[2];

	/**
	\brief global axes
	*/
	physx::PxVec3			globalAxis[2];

	/**
	\brief global normals
	*/
	physx::PxVec3			globalNormal[2];
};

// NxDestructibleActorJointDesc inline functions

PX_INLINE NxDestructibleActorJointDesc::NxDestructibleActorJointDesc() : NxApexDesc()
{
	setToDefault();
}

PX_INLINE void NxDestructibleActorJointDesc::setToDefault()
{
	NxApexDesc::setToDefault();
#if NX_SDK_VERSION_MAJOR == 2
	jointDesc = NULL;
#elif NX_SDK_VERSION_MAJOR == 3
	for (int i=0; i<2; i++)
	{
		actor[i] = 0;
		localAxis[i]	= PxVec3(0,0,1);
		localNormal[i]	= PxVec3(1,0,0);
		localAnchor[i]	= PxVec3(0);
	}
#endif
	for (int i = 0; i < 2; ++i)
	{
		destructible[i] = NULL;
		attachmentChunkIndex[i] = NxModuleDestructibleConst::INVALID_CHUNK_INDEX;
		globalAnchor[i] = physx::PxVec3(0.0f);
		globalAxis[i] = physx::PxVec3(0.0f, 0.0f, 1.0f);
		globalNormal[i] = physx::PxVec3(1.0f, 0.0f, 0.0f);
	}
}

PX_INLINE bool NxDestructibleActorJointDesc::isValid() const
{
#if NX_SDK_VERSION_MAJOR == 2
	if (jointDesc == NULL)
	{
		return false;
	}
#endif

	if (destructible[0] == NULL && destructible[1] == NULL)
	{
		return false;
	}

	for (int i = 0; i < 2; ++i)
	{
		if (physx::PxAbs(globalAxis[i].magnitudeSquared() - 1.0f) > 0.1f)
		{
			return false;
		}
		if (physx::PxAbs(globalNormal[i].magnitudeSquared() - 1.0f) > 0.1f)
		{
			return false;
		}
		//check orthogonal pairs
		if (physx::PxAbs(globalAxis[i].dot(globalNormal[i])) > 0.1f)
		{
			return false;
		}
	}

	return NxApexDesc::isValid();
}


/**
	Destructible actor joint - a wrapper for an NxJoint.  This needs to be used because APEX may need to replace an NxJoint
	when fracturing occurs.  Always use the joint() method to get the currently valid joint.
*/
class NxDestructibleActorJoint : public NxApexInterface
{
public:

#if (NX_SDK_VERSION_MAJOR == 2) || defined(DOXYGEN)
	/**
		PhysX SDK 2.8.X
		Access to the NxJoint represented by the NxDestructibleActorJoint.  This is a temporary interface,
		and there are some shortcomings and restrictions:
		1) The user will have to upcast the result of joint() to the correct joint type in order
		to access the interface for derived joints.
		2) The user must never delete the joint using the PhysX SDK.  To release this joint, simply
		use the release() method of this object (defined in the NxApexInterface base class).
	*/
	virtual	NxJoint*	joint()	= 0;
#endif
#if (NX_SDK_VERSION_MAJOR == 3) || defined(DOXYGEN)
	/**
		PhysX SDK 3.X
		Access to the {xJoint represented by the NxDestructibleActorJoint.  This is a temporary interface,
		and there are some shortcomings and restrictions:
		1) The user will have to upcast the result of joint() to the correct joint type in order
		to access the interface for derived joints.
		2) The user must never delete the joint using the PhysX SDK.  To release this joint, simply
		use the release() method of this object (defined in the NxApexInterface base class).
	*/
	virtual	PxJoint*	joint()	= 0;
#endif

protected:
	virtual				~NxDestructibleActorJoint() {}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_DESTRUCTIBLE_ACTOR_JOINT_H
