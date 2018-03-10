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


#include "ExtRevoluteJoint.h"
#include "PsUtilities.h"
#include "ExtConstraintHelper.h"
#include "CmRenderOutput.h"
#include "PsMathUtils.h"

namespace physx
{
namespace Ext
{
	PxU32 RevoluteJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 /*maxConstraints*/,
		PxConstraintInvMassScale &invMassScale,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w)
	{
		const RevoluteJointData& data = *reinterpret_cast<const RevoluteJointData*>(constantBlock);
		invMassScale = data.invMassScale;

		const PxJointAngularLimitPair& limit = data.limit;

		bool limitEnabled = data.jointFlags & PxRevoluteJointFlag::eLIMIT_ENABLED;
		bool limitIsLocked = limitEnabled && limit.lower >= limit.upper;

		PxTransform cA2w = bA2w * data.c2b[0];
		PxTransform cB2w = bB2w * data.c2b[1];

		if(cB2w.q.dot(cA2w.q)<0.f)
			cB2w.q = -cB2w.q;

		body0WorldOffset = cB2w.p-bA2w.p;
		Ext::joint::ConstraintHelper ch(constraints, cB2w.p - bA2w.p, cB2w.p - bB2w.p);

		ch.prepareLockedAxes(cA2w.q, cB2w.q, cA2w.transformInv(cB2w.p), 7, PxU32(limitIsLocked ? 7 : 6));

		if(limitIsLocked)
			return ch.getCount();

		PxVec3 axis = cA2w.rotate(PxVec3(1.f,0,0));

		if(data.jointFlags & PxRevoluteJointFlag::eDRIVE_ENABLED)
		{
			Px1DConstraint *c = ch.getConstraintRow();

			c->solveHint = PxConstraintSolveHint::eNONE;

			c->linear0			= PxVec3(0);
			c->angular0			= -axis;
			c->linear1			= PxVec3(0);
			c->angular1			= -axis * data.driveGearRatio;

			c->velocityTarget	= data.driveVelocity;

			c->minImpulse = -data.driveForceLimit;
			c->maxImpulse = data.driveForceLimit;
			if(data.jointFlags & PxRevoluteJointFlag::eDRIVE_FREESPIN)
			{
				if(data.driveVelocity > 0)
					c->minImpulse = 0;
				if(data.driveVelocity < 0)
					c->maxImpulse = 0;
			}
			c->flags |= Px1DConstraintFlag::eHAS_DRIVE_LIMIT;
		}


		if(limitEnabled)
		{
			PxQuat cB2cAq = cA2w.q.getConjugate() * cB2w.q;
			PxQuat twist(cB2cAq.x,0,0,cB2cAq.w);

			PxReal magnitude = twist.normalize();
			PxReal tqPhi = physx::intrinsics::fsel(magnitude - 1e-6f, twist.x / (1.0f + twist.w), 0.f);

			ch.quarterAnglePair(tqPhi, data.tqLow, data.tqHigh, data.tqPad, axis, limit);
		}

		return ch.getCount();
	}
}//namespace

}

//~PX_SERIALIZATION

