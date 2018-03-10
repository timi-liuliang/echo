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


#include "ScPhysics.h"
#include "ScScene.h"
#include "PxvGlobals.h"
#include "PxTolerancesScale.h"
#include "Factory.h"

using namespace physx;

Sc::Physics* Sc::Physics::mInstance = NULL;
const PxReal Sc::Physics::sWakeCounterOnCreation = 20.0f*0.02f;

namespace physx
{
	namespace Sc
	{
		OffsetTable	gOffsetTable;
	}
}

Sc::Physics::Physics(const PxTolerancesScale& scale, const PxvOffsetTable& pxvOffsetTable)
: mScale(scale)
, mLowLevelClothFactory(0)
{
	mInstance = this;
	
	PxvInit(pxvOffsetTable);
}


Sc::Physics::~Physics()
{
#if PX_USE_CLOTH_API
	if(mLowLevelClothFactory)
		PX_DELETE(mLowLevelClothFactory);
#endif

	PxvTerm();

	mInstance = 0;
}

#if PX_USE_CLOTH_API
void Sc::Physics::registerCloth()
{
	if(!mLowLevelClothFactory)
		mLowLevelClothFactory = cloth::Factory::createFactory(cloth::Factory::CPU);
}
#endif
