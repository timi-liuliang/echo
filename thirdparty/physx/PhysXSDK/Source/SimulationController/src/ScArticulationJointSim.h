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


#ifndef PX_PHYSICS_SCP_ARTICULATION_JOINT_SIM
#define PX_PHYSICS_SCP_ARTICULATION_JOINT_SIM

#include "ScActorInteraction.h"
#include "PxvArticulation.h"
#include "PxsIslandManager.h"

namespace physx
{
namespace Sc
{
	class ArticulationJointCore;
	class BodySim;

	class ArticulationJointSim : public ActorInteraction
	{
		ArticulationJointSim &operator=(const ArticulationJointSim &);

	public:

		ArticulationJointSim(ArticulationJointCore& joint, ActorSim& parent, ActorSim& child);

		~ArticulationJointSim();

		//---------- Interaction ----------
		virtual void destroy();

		// MS: Not needed as long as there are no dirty states that need to be updated
		//virtual void updateState();

		virtual bool onActivate(PxU32 infoFlag);
		virtual bool onDeactivate(PxU32 infoFlag);
		//-----------------------------------

		PX_INLINE ArticulationJointCore& getCore() const;
		PX_INLINE static bool isArticulationInteraction(const Interaction& interaction);

		PX_INLINE PxsIslandManagerEdgeHook& getLLIslandManagerEdgeHook() { return mIslandHook; }

		BodySim&			getParent() const;
		BodySim&			getChild() const;

		//---------------------------------------------------------------------------------
		// Low Level data access
		//---------------------------------------------------------------------------------
	private:

		PxsIslandManagerEdgeHook	mIslandHook;

		ArticulationJointCore&		mCore;
	};

} // namespace Sc


PX_INLINE Sc::ArticulationJointCore& Sc::ArticulationJointSim::getCore() const
{
	return mCore;
}


PX_INLINE bool Sc::ArticulationJointSim::isArticulationInteraction(const Interaction& interaction)
{
	return (interaction.getType() == PX_INTERACTION_TYPE_ARTICULATION);
}

}

#endif
