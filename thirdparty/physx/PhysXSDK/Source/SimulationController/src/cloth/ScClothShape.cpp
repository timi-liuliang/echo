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


#include "ScClothShape.h"
#if PX_USE_CLOTH_API

#include "ScNPhaseCore.h"
#include "ScScene.h"

#include "ScClothSim.h"
#include "PxsContext.h"
#include "PxsAABBManager.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ClothShape::ClothShape(ClothSim& cloth) :
	ElementSim(cloth, PX_ELEMENT_TYPE_CLOTH),
    mClothCore(cloth.getCore()),
	mIsInBroadPhase(mClothCore.getClothFlags() & PxClothFlag::eSCENE_COLLISION)
{
	if(mIsInBroadPhase)
		getActorSim().getScene().addBroadPhaseVolume(*this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ClothShape::~ClothShape()
{
	if(mIsInBroadPhase)
		getActorSim().getScene().removeBroadPhaseVolume(*this);

	PX_ASSERT(!hasAABBMgrHandle());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ClothShape::getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const
{
	filterAttr = 0;
	ElementSim::setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eCLOTH);
	filterData = mClothCore.getSimulationFilterData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sc::ClothShape::updateBoundsInAABBMgr()
{
	if(~mClothCore.getClothFlags() & PxClothFlag::eSCENE_COLLISION)
	{
		if(mIsInBroadPhase)
		{
			getActorSim().getScene().removeBroadPhaseVolume(*this);
			mIsInBroadPhase = false;
		}
		return;
	}

	if(!mIsInBroadPhase)
	{
		getActorSim().getScene().addBroadPhaseVolume(*this);
		mIsInBroadPhase = true;
	}

    PxsAABBManager* aabbMgr = getInteractionScene().getLowLevelContext()->getAABBManager();
	PxBounds3 worldBounds = mClothCore.getWorldBounds();
	worldBounds.fattenSafe(mClothCore.getContactOffset()); // fatten for fast moving colliders
    aabbMgr->setVolumeBounds(getAABBMgrHandle(), worldBounds);
}


#endif	// PX_USE_CLOTH_API
