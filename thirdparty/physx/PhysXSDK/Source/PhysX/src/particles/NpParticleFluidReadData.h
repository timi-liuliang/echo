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


#ifndef PX_PHYSICS_NP_PARTICLE_FLUID_READ_DATA
#define PX_PHYSICS_NP_PARTICLE_FLUID_READ_DATA

#include "PxParticleFluidReadData.h"
#include "PsFoundation.h"

namespace physx
{

class NpParticleFluidReadData : public PxParticleFluidReadData, public Ps::UserAllocated
{
public:

	NpParticleFluidReadData() :
		mIsLocked(false),
		mFlags(PxDataAccessFlag::eREADABLE)
	{
		strncpy(mLastLockedName, "UNDEFINED", sBufferLength);
	}

	virtual	~NpParticleFluidReadData()
	{}

	// implementation for PxLockedData
	virtual void setDataAccessFlags(PxDataAccessFlags flags) { mFlags = flags; }
	virtual PxDataAccessFlags getDataAccessFlags() { return mFlags; }

	virtual void unlock() { unlockFast(); }

	// internal methods
	void unlockFast() { mIsLocked = false; }

	void lock(const char* callerName)
	{
		if (mIsLocked)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxParticleReadData access through %s while its still locked by last call of %s.", callerName, mLastLockedName);
			PX_ALWAYS_ASSERT_MESSAGE("PxParticleReadData access violation");
		}
		strncpy(mLastLockedName, callerName, sBufferLength); 
		mLastLockedName[sBufferLength-1]=0;
		mIsLocked = true;
	}

private:

	static const PxU32 sBufferLength = 128;
	bool mIsLocked;
	char mLastLockedName[sBufferLength];
	PxDataAccessFlags mFlags;
};

}

#endif
