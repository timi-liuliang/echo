/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#include "ApexActor.h"
#include "DestructibleScene.h"
#include "DestructibleActor.h"
#include "SimScene.h"
#include "Actor.h"
#include "PxShape.h"

#include "Compound.h"

namespace physx
{

namespace fracture
{

void Compound::applyShapeTemplate(PxShape* shape)
{
	if( shape && mActor)
	{
		DestructibleActor* dactor = ((Actor*)mActor)->getDestructibleActor();
		{
			PhysX3DescTemplate physX3Template;
			dactor->getPhysX3Template(physX3Template);
			physX3Template.apply(shape);
		}
	}
}

::physx::apex::destructible::DestructibleActor* Compound::getDestructibleActor() const
{
	::physx::apex::destructible::DestructibleActor* dactor = NULL;
	if(mActor)
		dactor = ((Actor*)mActor)->getDestructibleActor();
	return dactor;
}
}
}
#endif