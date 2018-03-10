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


#ifndef PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_TREE
#define PX_PHYSICS_SCP_CONSTRAINT_PROJECTION_TREE

#include "PsArray.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"

namespace physx
{
namespace Sc
{
	struct ConstraintGroupNode;
	class ConstraintSim;
	class BodySim;
	class BodyRank;

	class ConstraintProjectionTree
	{
		/**
		 This class serves both the static administration of an articulation and the actual articulation itself.
		 An Articulation object holds several articulation root nodes which make up a simulation island that
		 is further connected with lagrange joints.
		*/
		public:
			ConstraintProjectionTree() {}
			~ConstraintProjectionTree() {}

			static	void		buildProjectionTrees(ConstraintGroupNode& root);	
			static	void		purgeProjectionTrees(ConstraintGroupNode& root);

			static void			projectPose(ConstraintGroupNode& root);

		private:
			static	PxU32					projectionTreeBuildStep(ConstraintGroupNode& node, ConstraintSim* cToParent, ConstraintGroupNode** nodeStack);

			static	void					getConstraintStatus(const ConstraintSim& c, const BodySim* b, BodySim*& otherBody, PxU32& projectToBody, PxU32& projectToOtherBody);
			static	void					rankConstraint(ConstraintSim&, BodyRank&, PxU32& dominanceTracking);
			static	void					projectPoseForTree(ConstraintGroupNode& node);
	};

} // namespace Sc

}

#endif
