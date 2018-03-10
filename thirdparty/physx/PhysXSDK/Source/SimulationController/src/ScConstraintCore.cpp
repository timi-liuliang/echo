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


#include "PsFoundation.h"

#include "ScPhysics.h"
#include "ScBodyCore.h"
#include "ScConstraintCore.h"
#include "ScConstraintSim.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ConstraintCore::ConstraintCore(PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize)
:	mFlags(PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES)
,	mAppliedForce(PxVec3(0))
,	mAppliedTorque(PxVec3(0))
,	mConnector(&connector)
,	mProject(shaders.project)
,	mSolverPrep(shaders.solverPrep)
,	mSolverPrepSpu(shaders.solverPrepSpu)
,	mSolverPrepSpuByteSize(shaders.solverPrepSpuByteSize)
,	mVisualize(shaders.visualize)
,	mDataSize(dataSize)
,	mLinearBreakForce(PX_MAX_F32)
,	mAngularBreakForce(PX_MAX_F32)
,	mSim(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Sc::ConstraintCore::~ConstraintCore()
{
}

void Sc::ConstraintCore::setFlags(PxConstraintFlags flags)
{
	if(flags != mFlags)
	{
		PxConstraintFlags old = mFlags;
		mFlags = flags;
		if(getSim())
			getSim()->postFlagChange(old, flags);
	}
}

void Sc::ConstraintCore::getForce(PxVec3& force, PxVec3& torque) const
{
	if(!mSim)
	{
		force = PxVec3(0,0,0);
		torque = PxVec3(0,0,0);
	}
	else
		mSim->getForce(force, torque);

}

void Sc::ConstraintCore::setBodies(RigidCore* r0v, RigidCore* r1v)
{
	if(mSim)
		mSim->postBodiesChange(r0v, r1v);
}

bool Sc::ConstraintCore::updateConstants(void* addr)
{
	if (getSim())
	{
		getSim()->setConstantsLL(addr);
		return true;
	}
	return false;
}

void Sc::ConstraintCore::setBreakForce(PxReal linear, PxReal angular)
{
	mLinearBreakForce = linear;
	mAngularBreakForce = angular;

	if (getSim())
		getSim()->setBreakForceLL(linear, angular);
}

void Sc::ConstraintCore::getBreakForce(PxReal& linear, PxReal& angular) const
{
	linear = mLinearBreakForce;
	angular = mAngularBreakForce;
}

PxConstraint* Sc::ConstraintCore::getPxConstraint()
{
	return gOffsetTable.convertScConstraint2Px(this);
}

const PxConstraint* Sc::ConstraintCore::getPxConstraint() const
{
	return gOffsetTable.convertScConstraint2Px(this);
}

void Sc::ConstraintCore::breakApart()
{
	// TODO: probably want to do something with the interaction here
	// as well as remove the constraint from LL.

	mFlags |= PxConstraintFlag::eBROKEN;
}

void Sc::ConstraintCore::prepareForSetBodies()
{
	if(mSim)
		mSim->preBodiesChange();
}
