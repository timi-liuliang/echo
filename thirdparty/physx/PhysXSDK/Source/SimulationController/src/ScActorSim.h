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


#ifndef PX_PHYSICS_SCP_ACTOR_SIM
#define PX_PHYSICS_SCP_ACTOR_SIM

#include "framework/ScActor.h"
#include "CmPhysXCommon.h"
#include "ScCoreInteraction.h"


namespace physx
{

class PxActor;

// This class will be defunct once things refer to rather than inherit from the A/F. All
// Sim actors should inherit from Actor instead at that point.

namespace Sc
{

	class Scene;
	class ActorCore;

	class ActorSim : public Actor
	{
	public:
										ActorSim(Scene&, ActorCore&, IslandNodeInfo::Type);
										~ActorSim();

						Scene&			getScene()				const;

	virtual				void			postActorFlagChange(PxU32, PxU32)	{}

						void			postDominanceGroupChange();

						void			setActorsInteractionsDirty(CoreInteraction::DirtyFlag flag, const Actor* other, PxU8 interactionFlag);

		PX_FORCE_INLINE	ActorCore&		getActorCore() const { return mCore; }

	protected:
						ActorSim &operator=(const ActorSim &);

						ActorCore&		mCore;
	};

} // namespace Sc

}

#endif
