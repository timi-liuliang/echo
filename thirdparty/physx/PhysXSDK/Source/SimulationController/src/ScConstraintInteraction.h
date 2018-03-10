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


#ifndef PX_PHYSICS_SCP_CONSTRAINTSHADERINTERACTION
#define PX_PHYSICS_SCP_CONSTRAINTSHADERINTERACTION

#include "ScActorInteraction.h"
#include "PxsIslandManager.h"

namespace physx
{
namespace Sc
{

	class ConstraintSim;
	class RigidSim;

	class ConstraintInteraction : public ActorInteraction
	{
	public:
												ConstraintInteraction(ConstraintSim* shader, RigidSim& r0, RigidSim& r1);
												~ConstraintInteraction();

		//---------- Interaction ----------
		virtual			void					destroy();
		virtual			void					updateState(bool removeFromDirtyList);

		virtual			bool					onActivate(PxU32 infoFlag);
		virtual			bool					onDeactivate(PxU32 infoFlag);
		//-----------------------------------

		PX_FORCE_INLINE	ConstraintSim*			getConstraint()	{ return mConstraint;	}

	private:
						ConstraintSim*			mConstraint;

						PxsIslandManagerEdgeHook mLLIslandHook;
	};

} // namespace Sc

}

#endif
