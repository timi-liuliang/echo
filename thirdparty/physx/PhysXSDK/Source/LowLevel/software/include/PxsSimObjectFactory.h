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


#ifndef PXS_SIM_OBJECTFACTORY_H
#define PXS_SIM_OBJECTFACTORY_H

#include "PxvConfig.h"

namespace physx
{

class PxsRigidBody;
struct PxsBodyCore;

struct PxsConstraint;

class PxsArticulation;
struct PxsArticulationCore;

class PxsContext;
class PxsDynamicsContext;

class PxsSimObjectFactory
{
public:
							PxsSimObjectFactory();
							~PxsSimObjectFactory();

	PxsArticulation*		createArticulation();
	void					destroyArticulation(PxsArticulation&);

private:
	struct PxsSimFactoryPools*	mPools;
};

}

#endif
