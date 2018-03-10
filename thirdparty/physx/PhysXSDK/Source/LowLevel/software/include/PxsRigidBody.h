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


#ifndef PXS_BODYATOM_H
#define PXS_BODYATOM_H

#include "PxcRigidBody.h"
#include "PxvDynamics.h"

namespace physx
{

class PxsRigidBody : public PxcRigidBody
{
	public:
	PX_FORCE_INLINE									PxsRigidBody(PxsBodyCore* core)	: PxcRigidBody(core) {  }
	PX_FORCE_INLINE									PxsRigidBody() {}
	PX_FORCE_INLINE									~PxsRigidBody() {}

	PX_FORCE_INLINE		const PxTransform&			getPose()								const	{ PX_ASSERT(mCore->body2World.isSane()); return mCore->body2World; }
						void						updatePoseDependenciesV(PxsContext &context);

	PX_FORCE_INLINE		const Cm::SpatialVector&	getAccelerationV()						const	{ return mAcceleration;		}
	PX_FORCE_INLINE		void						setAccelerationV(const Cm::SpatialVector& v)	{ mAcceleration = v;		}

	PX_FORCE_INLINE		const PxVec3&				getLinearVelocity()						const	{ PX_ASSERT(mCore->linearVelocity.isFinite()); return mCore->linearVelocity;			}
	PX_FORCE_INLINE		const PxVec3&				getAngularVelocity()					const	{ PX_ASSERT(mCore->angularVelocity.isFinite()); return mCore->angularVelocity;			}
	
	PX_FORCE_INLINE		void	 					setVelocity(const PxVec3& linear,
																const PxVec3& angular)				{ PX_ASSERT(linear.isFinite()); PX_ASSERT(angular.isFinite());
																									  mCore->linearVelocity = linear;
																									  mCore->angularVelocity = angular; }
	PX_FORCE_INLINE		void						setLinearVelocity(const PxVec3& linear)			{ PX_ASSERT(linear.isFinite()); mCore->linearVelocity = linear; }
	PX_FORCE_INLINE		void						setAngularVelocity(const PxVec3& angular)		{ PX_ASSERT(angular.isFinite()); mCore->angularVelocity = angular; }

	PX_FORCE_INLINE		PxU32						getIterationCounts()							{ return mCore->solverIterationCounts; }

	PX_FORCE_INLINE		PxReal						getReportThreshold()					const	{ return mCore->contactReportThreshold;	}

	// AP newccd todo: merge into get both velocities, compute inverse transform once, precompute mLastTransform.getInverse()
	PX_FORCE_INLINE		PxVec3						getLinearMotionVelocity(PxReal invDt)		const	{
														// delta(t0(x))=t1(x)
														// delta(t0(t0`(x)))=t1(t0`(x))
														// delta(x)=t1(t0`(x))
														PxVec3 deltaP = mCore->body2World.p - getLastCCDTransform().p;
														return deltaP * invDt;
													}
	PX_FORCE_INLINE		PxVec3						getAngularMotionVelocity(PxReal invDt)		const	{
														PxQuat deltaQ = mCore->body2World.q * getLastCCDTransform().q.getConjugate();
														PxVec3 axis;
														PxReal angle;
														deltaQ.toRadiansAndUnitAxis(angle, axis);
														return axis * angle * invDt;
													}
	PX_FORCE_INLINE		PxVec3						getLinearMotionVelocity(PxReal dt, const PxsBodyCore* PX_RESTRICT bodyCore)		const	{
															// delta(t0(x))=t1(x)
															// delta(t0(t0`(x)))=t1(t0`(x))
															// delta(x)=t1(t0`(x))
															PxVec3 deltaP = bodyCore->body2World.p - getLastCCDTransform().p;
															return deltaP * 1.0f / dt;
													}
	PX_FORCE_INLINE		PxVec3						getAngularMotionVelocity(PxReal dt, const PxsBodyCore* PX_RESTRICT bodyCore)		const	{
															PxQuat deltaQ = bodyCore->body2World.q * getLastCCDTransform().q.getConjugate();
															PxVec3 axis;
															PxReal angle;
															deltaQ.toRadiansAndUnitAxis(angle, axis);
															return axis * angle * 1.0f/dt;
													}

	PX_FORCE_INLINE		PxTransform					getLastCCDTransform()					const	{ return mLastTransform; }
	PX_FORCE_INLINE		void						saveLastCCDTransform()							{ mLastTransform = mCore->body2World; }

	PX_FORCE_INLINE		bool						isKinematic()							const	{ return (mCore->inverseMass == 0.0f); }
						
	PX_FORCE_INLINE		void						setPose(const PxTransform& pose)				{ mCore->body2World = pose; }
	PX_FORCE_INLINE		void						setPosition(const PxVec3& position)				{ mCore->body2World.p = position; }
	PX_FORCE_INLINE		PxReal						getInvMass()							const	{ return mCore->inverseMass; }	
	PX_FORCE_INLINE		PxVec3						getInvInertia()							const	{ return mCore->inverseInertia; }
	PX_FORCE_INLINE		PxReal						getMass()								const	{ return 1/mCore->inverseMass; }	
	PX_FORCE_INLINE		PxVec3						getInertia()							const	{ return PxVec3(1.0f/mCore->inverseInertia.x,
																													1.0f/mCore->inverseInertia.y,
																													1.0f/mCore->inverseInertia.z); }
	PX_FORCE_INLINE		PxsBodyCore&				getCore()										{ return *mCore;					}
	PX_FORCE_INLINE		const PxsBodyCore&			getCore()								const	{ return *mCore;					}

	PX_FORCE_INLINE		void						setAABBMgrId(const AABBMgrId& id)				{ mAABBMgrId = id;				}								
	PX_FORCE_INLINE		const AABBMgrId&			getAABBMgrId()							const	{ return mAABBMgrId;			}
	PX_FORCE_INLINE		void						resetAABBMgrId()								{ mAABBMgrId.reset(); }

						void						advanceToToi(PxReal toi, PxReal dt, bool clip);
						void						advancePrevPoseToToi(PxReal toi);
						PxTransform					getAdvancedTransform(PxReal toi) const;
};

}

#endif
