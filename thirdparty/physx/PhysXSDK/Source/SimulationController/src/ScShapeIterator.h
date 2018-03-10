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


#ifndef PX_PHYSICS_SCP_SHAPEITERATOR
#define PX_PHYSICS_SCP_SHAPEITERATOR

#include "PxVec3.h"
#include "CmRange.h"

namespace physx
{

namespace Sc
{
	class ShapeCore;
	class ShapeSim;
	class ActorSim;
	class Element;

	class ShapeIterator
	{
		public:
									ShapeIterator() : mCurrent(NULL)	{}

			void					init(const Sc::ActorSim& b);
			Sc::ShapeSim*			getNext();

		private:
			Element*				mCurrent;
	};


}  // namespace Sc

}

#endif
