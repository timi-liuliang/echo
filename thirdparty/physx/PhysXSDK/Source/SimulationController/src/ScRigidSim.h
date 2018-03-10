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


#ifndef PX_PHYSICS_SCP_RB_SIM
#define PX_PHYSICS_SCP_RB_SIM

#include "ScActorSim.h"
#include "ScRigidCore.h"
#include "PxvBroadPhase.h"

namespace physx
{

class Interaction;

namespace Sc
{
	class ShapeCore;
	class ShapeSim;
	class Scene;

	// TODO: eventually, ActorSim should go away, since it doesn't do anything useful, and
	// then we will just inherit from Actor

	class RigidSim : public ActorSim
	{
	public:
										RigidSim(Scene&, RigidCore&, IslandNodeInfo::Type);
		virtual							~RigidSim();

		PX_FORCE_INLINE	RigidCore&		getRigidCore()			const	{ return static_cast<RigidCore&>(mCore);	}

		PX_FORCE_INLINE	PxU32			getID()					const	{ return mRigidId;	}

		PX_FORCE_INLINE	PxU32			getBroadphaseGroupId()	const	{ return (getActorType()!=PxActorType::eRIGID_STATIC ? mRigidId + PxU32(BP_GROUP_DYNAMICS) : PxU32(BP_GROUP_STATICS));}

		void							notifyShapeTranformChange();

						Sc::ShapeSim&	getSimForShape(Sc::ShapeCore& shape) const;


						PxActor*		getPxActor();
	private:
						PxU32			mRigidId;
	};

} // namespace Sc

}

#endif
