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


#ifndef PX_FRAMEWORK_PXELEMENTINTERACTION
#define PX_FRAMEWORK_PXELEMENTINTERACTION

#include "ScInteraction.h"
#include "ScElement.h"
#include "PxvConfig.h"

namespace physx
{


namespace Sc
{

	class ElementInteraction : public Interaction
	{
		friend class Element;
	public:
		virtual ~ElementInteraction() {}
		PX_FORCE_INLINE	Element&	getElement0()	const	{ return mElement0; }
		PX_FORCE_INLINE	Element&	getElement1()	const	{ return mElement1; }

	protected:
		PX_INLINE ElementInteraction(Element& element0, Element& element1, InteractionType type, PxU8 flags);
		ElementInteraction& operator=(const ElementInteraction&);

	private:
		Element& mElement0;
		Element& mElement1;
	};

} // namespace Sc

//////////////////////////////////////////////////////////////////////////
PX_INLINE Sc::ElementInteraction::ElementInteraction(Element& element0, Element& element1, InteractionType type, PxU8 flags) :
	Interaction	(element0.getScActor(), element1.getScActor(), type, flags),
	mElement0	(element0),
	mElement1	(element1)
{
}


}

#endif
