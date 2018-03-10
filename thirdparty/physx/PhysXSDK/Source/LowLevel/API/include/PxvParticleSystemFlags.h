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


#ifndef PXV_PARTICLE_SYSTEM_FLAGS_H
#define PXV_PARTICLE_SYSTEM_FLAGS_H

#include "PxvConfig.h"
#include "particles/PxParticleFlag.h"
#include "particles/PxParticleBaseFlag.h"

namespace physx
{

/*
ParticleSystems related constants
*/
// Maximum number of particles per particle system
#define PXD_PARTICLE_SYSTEM_PARTICLE_LIMIT		0xfffffffe

/*!
PxParticleBaseFlag extension.
*/
struct PxvInternalParticleSystemFlag
{
	enum Enum
	{
		//flags need to go into the unused bits of PxParticleBaseFlag
		eSPH								= (1 << 16),
		eDISABLE_POSITION_UPDATE_ON_CREATE	= (1 << 17),
		eDISABLE_POSITION_UPDATE_ON_SETPOS	= (1 << 18)
	};
};

struct PxvInternalParticleFlag
{
	enum Enum
	{
		//constraint info
		eCONSTRAINT_0_VALID								= (1<<0),
		eCONSTRAINT_1_VALID								= (1<<1),
		eANY_CONSTRAINT_VALID							= (eCONSTRAINT_0_VALID | eCONSTRAINT_1_VALID),
		eCONSTRAINT_0_DYNAMIC							= (1<<2),
		eCONSTRAINT_1_DYNAMIC							= (1<<3),
		eALL_CONSTRAINT_MASK							= (eANY_CONSTRAINT_VALID | eCONSTRAINT_0_DYNAMIC | eCONSTRAINT_1_DYNAMIC),
		
		//static geometry cache: 00 (cache invalid), 11 (cache valid and refreshed), 01 (cache valid, but aged by one step).
		eGEOM_CACHE_BIT_0								= (1<<4),
		eGEOM_CACHE_BIT_1								= (1<<5),
		eGEOM_CACHE_MASK								= (eGEOM_CACHE_BIT_0 | eGEOM_CACHE_BIT_1),

		//cuda update info
		eCUDA_NOTIFY_CREATE								= (1<<6),
		eCUDA_NOTIFY_SET_POSITION						= (1<<7),
		eCUDA_NOTIFY_POSITION_CHANGE					= (eCUDA_NOTIFY_CREATE | eCUDA_NOTIFY_SET_POSITION)
	};
};

struct PxvParticleFlags
{
	PxU16     api;    //this is PxParticleFlag
	PxU16	  low;    //this is PxvInternalParticleFlag
};

}

#endif // PXV_PARTICLE_SYSTEM_FLAGS_H
