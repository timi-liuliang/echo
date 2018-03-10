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


#ifndef PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_MANAGER
#define PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_MANAGER

#include "PsPool.h"
#include "PsHashSet.h"
#include "ScConstraintGroupNode.h"

namespace physx
{
	class PxcScratchAllocator;

namespace Sc
{
	class ConstraintSim;
	class BodySim;
	template<typename T, const PxU32 elementsPerBlock = 64> class ScratchAllocatorList;

	class ConstraintProjectionManager : public Ps::UserAllocated
	{
	public:
		ConstraintProjectionManager();
		~ConstraintProjectionManager() {}

		void addToPendingGroupUpdates(ConstraintSim& s);
		void removeFromPendingGroupUpdates(ConstraintSim& s);

		void addToPendingTreeUpdates(ConstraintGroupNode& n);
		void removeFromPendingTreeUpdates(ConstraintGroupNode& n);

		void processPendingUpdates(PxcScratchAllocator&);
		void invalidateGroup(ConstraintGroupNode& node, ConstraintSim* constraintDeleted);

	private:
		PX_INLINE Sc::ConstraintGroupNode* createGroupNode(BodySim& b);

		void addToGroup(BodySim& b, BodySim* other, ConstraintSim& c);
		void groupUnion(ConstraintGroupNode& root0, ConstraintGroupNode& root1);
		void markConnectedConstraintsForUpdate(BodySim& b, ConstraintSim* c);
		PX_FORCE_INLINE void processConstraintForGroupBuilding(ConstraintSim* c, ScratchAllocatorList<ConstraintSim*>&);


	private:
		Ps::Pool<ConstraintGroupNode>				mNodePool;
		Ps::CoalescedHashSet<ConstraintSim*>		mPendingGroupUpdates; //list of constraints for which constraint projection groups need to be generated/updated
		Ps::CoalescedHashSet<ConstraintGroupNode*>	mPendingTreeUpdates;	//list of constraint groups that need their projection trees rebuilt. Note: non of the
																			//constraints in those groups are allowed to be in mPendingGroupUpdates at the same time
																			//because a group update will automatically trigger tree rebuilds.
	};

} // namespace Sc

}

#endif
