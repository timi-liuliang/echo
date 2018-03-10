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


#ifndef PX_COLLISION_ELEMENT_INTERACTION_MARKER
#define PX_COLLISION_ELEMENT_INTERACTION_MARKER

#include "ScElementSimInteraction.h"

namespace physx
{
namespace Sc
{

	class ElementInteractionMarker : public ElementSimInteraction
	{
	public:
		PX_INLINE ElementInteractionMarker(ElementSim& element0, ElementSim& element1);
		virtual ~ElementInteractionMarker() {}

		virtual bool onActivate(PxU32 infoFlag);
		virtual bool onDeactivate(PxU32 infoFlag);
	};

} // namespace Sc


PX_INLINE Sc::ElementInteractionMarker::ElementInteractionMarker(ElementSim& element0, ElementSim& element1) :
	ElementSimInteraction(element0, element1, PX_INTERACTION_TYPE_MARKER, PX_INTERACTION_FLAG_RB_ELEMENT|PX_INTERACTION_FLAG_FILTERABLE)
{
}

}

#endif //PX_COLLISION_SHAPEINTERACTIONMARKER
