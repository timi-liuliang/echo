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


#include "PsPool.h"
#include "PxsSimObjectFactory.h"
#include "PxsArticulation.h"

using namespace physx;

namespace physx
{
	struct PxsSimFactoryPools
	{
		Ps::Pool<PxsArticulation, Ps::AlignedAllocator<PXC_ARTICULATION_MAX_SIZE> >		articulationPool;
	};
}

PxsSimObjectFactory::PxsSimObjectFactory()
{
	mPools = reinterpret_cast<PxsSimFactoryPools*>(PX_ALLOC(sizeof(PxsSimFactoryPools), PX_DEBUG_EXP("PxsSimFactoryPools")));
	new(mPools)PxsSimFactoryPools();
}


PxsSimObjectFactory::~PxsSimObjectFactory()
{
	mPools->~PxsSimFactoryPools();
	PX_FREE(mPools);
}


PxsArticulation* PxsSimObjectFactory::createArticulation()
{
	return mPools->articulationPool.construct();
}


void PxsSimObjectFactory::destroyArticulation(PxsArticulation& articulation)
{
	mPools->articulationPool.destroy(&articulation);
}
