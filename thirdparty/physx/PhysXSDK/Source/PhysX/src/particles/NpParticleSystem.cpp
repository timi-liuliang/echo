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


#include "NpParticleSystem.h"

#if PX_USE_PARTICLE_SYSTEM_API

#include "PsFoundation.h"
#include "NpPhysics.h"

#include "NpScene.h"
#include "NpWriteCheck.h"

#include "PsArray.h"

using namespace physx;

NpParticleSystem::NpParticleSystem(PxU32 maxParticles, bool perParticleRestOffset)
: ParticleSystemTemplateClass(PxConcreteType::ePARTICLE_SYSTEM, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE, PxActorType::ePARTICLE_SYSTEM, maxParticles, perParticleRestOffset)
{}

NpParticleSystem::~NpParticleSystem()
{}

// PX_SERIALIZATION
NpParticleSystem* NpParticleSystem::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpParticleSystem* obj = new (address) NpParticleSystem(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpParticleSystem);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

void NpParticleSystem::setParticleReadDataFlag(PxParticleReadDataFlag::Enum flag, bool val)
{
	NP_WRITE_CHECK(getNpScene());		
	PX_CHECK_AND_RETURN( flag != PxParticleReadDataFlag::eDENSITY_BUFFER, 
		"ParticleSystem has unsupported PxParticleReadDataFlag::eDENSITY_BUFFER set.");	
	NpParticleSystemT::setParticleReadDataFlag(flag, val);
}
#endif
