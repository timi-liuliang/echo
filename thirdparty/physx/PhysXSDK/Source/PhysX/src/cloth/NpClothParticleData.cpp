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

#include "PxPhysXConfig.h"
#if PX_USE_CLOTH_API

#include "NpClothParticleData.h"
#include "NpCloth.h"

using namespace physx;

void NpClothParticleData::unlock() 
{
	PX_ASSERT(mNbLocks);

	if(!mFlags.isSet(PxDataAccessFlag::eDEVICE))
	{
		if (mFlags.isSet(PxDataAccessFlag::eWRITABLE))
			mCloth.setParticles(particles, previousParticles);
		mCloth.unlockParticleData();
	}
	mFlags.clear(PxDataAccessFlag::eWRITABLE);
	mFlags.clear(PxDataAccessFlag::eDEVICE);

	--mNbLocks;
}

bool NpClothParticleData::tryLock( PxDataAccessFlags flags )
{
	flags |= mFlags;
	if((flags.isSet(PxDataAccessFlag::eWRITABLE) || flags.isSet(PxDataAccessFlag::eDEVICE)) && mNbLocks)
		return false;

	mFlags = flags;
	++mNbLocks;
	return true;
}

#endif // PX_USE_CLOTH_API
