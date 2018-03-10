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


#include "ScBodyCore.h"
#include "ScStaticCore.h"
#include "ScRigidSim.h"
#include "ScShapeSim.h"
#include "ScScene.h"
#include "ScPhysics.h"


using namespace physx;

Sc::RigidCore::RigidCore(const PxActorType::Enum type) 
: ActorCore(type, PxActorFlag::eVISUALIZATION, PX_DEFAULT_CLIENT, 0, 0)
{
}


Sc::RigidCore::~RigidCore()
{
}

void Sc::RigidCore::addShapeToScene(ShapeCore& shapeCore)
{
	Sc::RigidSim* sim = getSim();
	PX_ASSERT(sim);
	if(!sim)
		return;
	sim->getScene().addShape(*sim, shapeCore);
}

void Sc::RigidCore::removeShapeFromScene(ShapeCore& shapeCore, bool wakeOnLostTouch)
{
	Sc::RigidSim* sim = getSim();
	if(!sim)
		return;
	Sc::ShapeSim& s = sim->getSimForShape(shapeCore);
	sim->getScene().removeShape(s, wakeOnLostTouch);
}

void Sc::RigidCore::onShapeChange(Sc::ShapeCore& shape, ShapeChangeNotifyFlags notifyFlags, PxShapeFlags oldShapeFlags)
{
	// DS: We pass flags to avoid searching multiple times or exposing RigidSim outside SC, and this form is
	// more convenient for the Scb::Shape::syncState method. If we start hitting this a lot we should do it
	// a different way, but shape modification after insertion is rare. 

	Sc::RigidSim* sim = getSim();
	if(!sim)
		return;
	Sc::ShapeSim& s = sim->getSimForShape(shape);

	if(notifyFlags & ShapeChangeNotifyFlag::eGEOMETRY)
		s.onVolumeChange();
	if(notifyFlags & ShapeChangeNotifyFlag::eMATERIAL)
		s.onMaterialChange();
	if(notifyFlags & ShapeChangeNotifyFlag::eRESET_FILTERING)
		s.onResetFiltering();
	if(notifyFlags & ShapeChangeNotifyFlag::eSHAPE2BODY)
		s.onTransformChange();
	if(notifyFlags & ShapeChangeNotifyFlag::eFILTERDATA)
		s.onFilterDataChange();
	if(notifyFlags & ShapeChangeNotifyFlag::eFLAGS)
		s.onFlagChange(oldShapeFlags);
	if(notifyFlags & ShapeChangeNotifyFlag::eRESTOFFSET)
		s.onRestOffsetChange();
	if(notifyFlags & ShapeChangeNotifyFlag::eCONTACTOFFSET)
		s.onVolumeChange();
}




Sc::RigidSim* Sc::RigidCore::getSim() const
{
	return static_cast<RigidSim*>(Sc::ActorCore::getSim());
}




// The alternative to this switch is to have a virtual interface just for this (which would nullify
// the space advantage of getting rid of back pointers) or exposing the Np implementation to Sc.
PxActor* Sc::RigidCore::getPxActor() const
{
	Sc::RigidCore* r = const_cast<Sc::RigidCore*>(this);
#if PX_IS_SPU
	Sc::SpuOffsetTable& tbl = Sc::gSpuOffsetTable;
#else
	Sc::SpuOffsetTable& tbl = Sc::gOffsetTable;
#endif
	switch(getActorCoreType())
	{
	case PxActorType::eRIGID_STATIC:
		return tbl.convertScRigidStatic2PxActor(static_cast<Sc::StaticCore*>(r));
	case PxActorType::eRIGID_DYNAMIC:
		return tbl.convertScRigidDynamic2PxActor(static_cast<Sc::BodyCore*>(r));
	case PxActorType::eARTICULATION_LINK:
		return tbl.convertScArticulationLink2PxActor(static_cast<Sc::BodyCore*>(r));
#if PX_USE_PARTICLE_SYSTEM_API
	case PxActorType::ePARTICLE_SYSTEM:
	case PxActorType::ePARTICLE_FLUID:
#endif
#if PX_USE_CLOTH_API
	case PxActorType::eCLOTH:
#endif
	case PxActorType::eACTOR_COUNT:
	case PxActorType::eACTOR_FORCE_DWORD:
	default:
		PX_ASSERT(0);
		return 0;
	}
}
