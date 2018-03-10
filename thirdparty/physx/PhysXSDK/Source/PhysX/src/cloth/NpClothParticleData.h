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


#ifndef PX_PHYSICS_NP_CLOTH_READ_DATA
#define PX_PHYSICS_NP_CLOTH_READ_DATA

#include "PxPhysXConfig.h"

#if PX_USE_CLOTH_API

#include "PxClothParticleData.h"
#include "PsUserAllocated.h"

namespace physx
{

class NpCloth;

class NpClothParticleData : public PxClothParticleData, public shdfnd::UserAllocated
{
public:

	NpClothParticleData(NpCloth& cloth) 
		: mCloth(cloth), mNbLocks(0), mFlags(PxDataAccessFlag::eREADABLE)
	{}

	virtual	~NpClothParticleData()
	{}

	// implementation for PxLockedData
	virtual PxDataAccessFlags getDataAccessFlags() { return mFlags; }
	virtual void unlock();

	bool tryLock(PxDataAccessFlags);

private:
	NpClothParticleData& operator=(const NpClothParticleData&);
	NpCloth& mCloth;
	PxU32 mNbLocks;
	PxDataAccessFlags mFlags;
};

}

#endif // PX_USE_CLOTH_API

#endif
