/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEACTORJOINT_H__
#define __DESTRUCTIBLEACTORJOINT_H__

#include "NxApex.h"
#include "ApexInterface.h"
#include "PhysX3ClassWrapper.h"

namespace physx
{
namespace apex
{
class NxDestructibleActorJointDesc;

namespace destructible
{
class DestructibleStructure;
class DestructibleScene;

class DestructibleActorJoint : public ApexResource
{
public:
	DestructibleActorJoint(const NxDestructibleActorJointDesc& destructibleActorJointDesc, DestructibleScene& dscene);

	virtual		~DestructibleActorJoint();

	NxJoint*	getJoint()
	{
		return joint;
	}

	bool		updateJoint();

protected:

	NxJoint*				joint;
	DestructibleStructure*	structure[2];
	physx::PxI32			attachmentChunkIndex[2];
};

}
}
} // end namespace physx::apex

#endif // __DESTRUCTIBLEACTORJOINT_H__
