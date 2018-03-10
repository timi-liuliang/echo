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


#ifndef PX_COLLISION_RB_ELEMENT_INTERACTION
#define PX_COLLISION_RB_ELEMENT_INTERACTION

#include "ScElementSimInteraction.h"
#include "ScShapeSim.h"

namespace physx
{
namespace Sc
{

	class RbElementInteraction : public ElementSimInteraction
	{
	public:
		PX_INLINE RbElementInteraction(ShapeSim& shape0, ShapeSim& shape1, InteractionType type, PxU8 flags);
		virtual ~RbElementInteraction() {}

		PX_INLINE ShapeSim& getShape0() const;
		PX_INLINE ShapeSim& getShape1() const;
	};
} // namespace Sc

//////////////////////////////////////////////////////////////////////////
Sc::RbElementInteraction::RbElementInteraction(ShapeSim& shape0, ShapeSim& shape1, InteractionType type, PxU8 flags) :
	ElementSimInteraction	(shape0, shape1, type, flags)
{
}

PX_INLINE Sc::ShapeSim& Sc::RbElementInteraction::getShape0() const
{
	return static_cast<ShapeSim&>(getElement0());
}

PX_INLINE Sc::ShapeSim& Sc::RbElementInteraction::getShape1() const
{
	return static_cast<ShapeSim&>(getElement1());
}

}

#endif
