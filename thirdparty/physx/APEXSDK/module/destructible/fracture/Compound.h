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
#ifndef COMPOUND_H
#define COMPOUND_H

#include "CompoundBase.h"

namespace physx
{
namespace fracture
{

class Compound : public base::Compound
{
	friend class SimScene;
	friend class Actor;
protected:
	Compound(SimScene* scene, const base::FracturePattern *pattern, const base::FracturePattern *secondaryPattern = NULL, PxReal contactOffset = 0.005f, PxReal restOffset = -0.001f):
		physx::fracture::base::Compound((base::SimScene*)scene,pattern,secondaryPattern,contactOffset,restOffset) {}
public:
	virtual void applyShapeTemplate(PxShape* shape);
	virtual ::physx::apex::destructible::DestructibleActor* getDestructibleActor() const;
};

}
}


#endif
#endif