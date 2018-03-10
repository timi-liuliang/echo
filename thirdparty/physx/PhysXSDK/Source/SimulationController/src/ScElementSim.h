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


#ifndef PX_PHYSICS_SCP_ELEMENT_SIM
#define PX_PHYSICS_SCP_ELEMENT_SIM

#include "ScElement.h"
#include "ScActorSim.h"
#include "ScCoreInteraction.h"
#include "PxFiltering.h"

namespace physx
{
namespace Sc
{
	class Scene;

	class ElementSim : public Element
	{
		PX_NOCOPY(ElementSim)
	public:
		PX_FORCE_INLINE ElementSim(ActorSim& actor, ElementType type) : Element(actor, type)	{}
		PX_FORCE_INLINE ~ElementSim()															{}

		PX_FORCE_INLINE ActorSim& getActorSim() const	{ return static_cast<ActorSim&>(getScActor());	}

		//---------- Filtering ----------
		virtual void getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const = 0;

		PX_INLINE void setFilterObjectAttributeType(PxFilterObjectAttributes& attr, PxFilterObjectType::Enum type) const;
		//-------------------------------

		void setElementInteractionsDirty(CoreInteraction::DirtyFlag flag, PxU8 interactionFlag);

		Scene& getScene();
	};

} // namespace Sc

// SFD: duplicated attribute generation in SqFiltering.h
PX_INLINE void Sc::ElementSim::setFilterObjectAttributeType(PxFilterObjectAttributes& attr, PxFilterObjectType::Enum type) const
{
	PX_ASSERT((attr & (PxFilterObjectType::eMAX_TYPE_COUNT-1)) == 0);
	attr |= type;
}

}

#endif
