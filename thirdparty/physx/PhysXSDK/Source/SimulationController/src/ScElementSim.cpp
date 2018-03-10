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


#include "ScElementSim.h"
#include "ScElementSimInteraction.h"

using namespace physx;

void Sc::ElementSim::setElementInteractionsDirty(CoreInteraction::DirtyFlag flag, PxU8 interactionFlag)
{
	Element::ElementInteractionIterator iter = getElemInteractions();
	ElementInteraction* interaction = iter.getNext();
	while(interaction)
	{
		if (interaction->getInteractionFlags() & interactionFlag)
		{
			PX_ASSERT(CoreInteraction::isCoreInteraction(interaction) != NULL);  // all element vs. element interactions are core interactions at the moment
			static_cast<ElementSimInteraction*>(interaction)->setDirty(flag);
		}

		interaction = iter.getNext();
	}
}

Sc::Scene& Sc::ElementSim::getScene()	
{
	return getInteractionScene().getOwnerScene();
}
