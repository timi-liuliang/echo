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


#ifndef PX_PHYSICS_SCP_ELEMENT_INTERACTION
#define PX_PHYSICS_SCP_ELEMENT_INTERACTION

#include "ScCoreInteraction.h"
#include "ScElementInteraction.h"
#include "ScElementSim.h"

namespace physx
{
namespace Sc
{

	class NPhaseCore;
	class ElementSim;

	class ElementSimInteraction : public CoreInteraction, public ElementInteraction
	{
	public:
		PX_INLINE ElementSimInteraction(ElementSim& element0, ElementSim& element1, InteractionType type, PxU8 flags);
		virtual ~ElementSimInteraction() {}

		PX_INLINE ElementSim& getElementSim0() const;
		PX_INLINE ElementSim& getElementSim1() const;

		//--------- CoreInteraction ---------
		virtual NPhaseCore* getNPhaseCore() const;
		//-----------------------------------

		// Method to check if this interaction is the last filter relevant interaction between the two elements,
		// i.e., if this interaction gets deleted, the pair is considered lost
		virtual bool isLastFilterInteraction() const { return true; }

	private:
	};

} // namespace Sc

//////////////////////////////////////////////////////////////////////////
PX_INLINE Sc::ElementSimInteraction::ElementSimInteraction(ElementSim& element0, ElementSim& element1, InteractionType type, PxU8 flags) :
	CoreInteraction		(true),
	ElementInteraction	(element0, element1, type, flags)
{
}

PX_INLINE Sc::ElementSim& Sc::ElementSimInteraction::getElementSim0() const
{
	return static_cast<ElementSim&>(getElement0());
}

PX_INLINE Sc::ElementSim& Sc::ElementSimInteraction::getElementSim1() const
{
	return static_cast<ElementSim&>(getElement1());
}

}

#endif
