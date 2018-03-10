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


#include "ScbBase.h"

using namespace physx;
using namespace Scb;

using namespace physx;
using namespace Scb;

#include "ScbActor.h"
#include "ScbRigidStatic.h"
#include "ScbBody.h"
#ifndef __SPU__
#include "ScbParticleSystem.h"
#include "ScbCloth.h"
#else
#undef PX_USE_PARTICLE_SYSTEM_API
#undef PX_USE_CLOTH_API
#endif
namespace physx
{
namespace Scb
{
	
Actor::Offsets::Offsets()
{
	size_t staticOffset		= reinterpret_cast<size_t>(&(reinterpret_cast<Scb::RigidStatic*>(0)->getScStatic()));
	size_t bodyOffset		= reinterpret_cast<size_t>(&(reinterpret_cast<Scb::Body*>(0)->getScBody()));

	scToScb[PxActorType::eRIGID_STATIC] = staticOffset;
	scToScb[PxActorType::eRIGID_DYNAMIC] = bodyOffset;
	scToScb[PxActorType::eARTICULATION_LINK] = bodyOffset;

	scbToSc[ScbType::RIGID_STATIC] = staticOffset;
	scbToSc[ScbType::BODY] = bodyOffset;
	scbToSc[ScbType::BODY_FROM_ARTICULATION_LINK] = bodyOffset;
	
#if PX_USE_PARTICLE_SYSTEM_API
	size_t particleOffset	= reinterpret_cast<size_t>(&(reinterpret_cast<Scb::ParticleSystem*>(0)->getScParticleSystem()));
	scToScb[PxActorType::ePARTICLE_FLUID] = particleOffset;
	scToScb[PxActorType::ePARTICLE_SYSTEM] = particleOffset;
	scbToSc[ScbType::PARTICLE_SYSTEM] = particleOffset;
#endif

#if PX_USE_CLOTH_API
	size_t clothOffset		= reinterpret_cast<size_t>(&(reinterpret_cast<Scb::Cloth*>(0)->getScCloth()));
	scToScb[PxActorType::eCLOTH] = clothOffset;
	scbToSc[ScbType::CLOTH] = clothOffset;
#endif


}




const Actor::Offsets Actor::sOffsets;
}
}
