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


#include "ExtSphericalJoint.h"
#include "ExtConstraintHelper.h"
#include "CmConeLimitHelper.h"
#include "CmRenderOutput.h"

namespace physx
{
namespace Ext
{
	PxU32 SphericalJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 /*maxConstraints*/,
		PxConstraintInvMassScale &invMassScale,
		const void* constantBlock,							  
		const PxTransform& bA2w,
		const PxTransform& bB2w)
	{

		using namespace joint;
		const SphericalJointData& data = *reinterpret_cast<const SphericalJointData*>(constantBlock);
		invMassScale = data.invMassScale;


		PxTransform cA2w = bA2w * data.c2b[0];
		PxTransform cB2w = bB2w * data.c2b[1];

		if(cB2w.q.dot(cA2w.q)<0) 
			cB2w.q = -cB2w.q;

		body0WorldOffset = cB2w.p-bA2w.p;
		joint::ConstraintHelper ch(constraints, cB2w.p - bA2w.p, cB2w.p - bB2w.p);

		if(data.jointFlags & PxSphericalJointFlag::eLIMIT_ENABLED)
		{
			PxQuat swing, twist;
			Ps::separateSwingTwist(cA2w.q.getConjugate() * cB2w.q, swing, twist);
			PX_ASSERT(PxAbs(swing.x)<1e-6f);

			Cm::ConeLimitHelper coneHelper(data.tanQZLimit, data.tanQYLimit, data.tanQPad);

			PxVec3 axis;
			PxReal error;
			if(coneHelper.getLimit(swing, axis, error))
				ch.angularLimit(cA2w.rotate(axis),error,data.limit);

		}

		ch.prepareLockedAxes(cA2w.q, cB2w.q, cA2w.transformInv(cB2w.p), 7, 0);

		return ch.getCount();
	}
}//namespace

}

//~PX_SERIALIZATION

