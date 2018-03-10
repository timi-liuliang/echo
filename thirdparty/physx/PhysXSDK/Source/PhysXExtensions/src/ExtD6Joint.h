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


#ifndef NP_D6JOINTCONSTRAINT_H
#define NP_D6JOINTCONSTRAINT_H

#include "ExtJoint.h"
#include "PxD6Joint.h"

namespace physx
{
struct PxD6JointGeneratedValues;
namespace Ext
{
	struct D6JointData : public JointData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		PxD6Motion::Enum		motion[6]; 
		PxJointLinearLimit		linearLimit;
		PxJointAngularLimitPair	twistLimit;
		PxJointLimitCone		swingLimit;

		PxD6JointDrive			drive[PxD6Drive::eCOUNT];

		PxTransform				drivePosition;
		PxVec3					driveLinearVelocity;
		PxVec3					driveAngularVelocity;

		// derived quantities

		PxU32					locked;		// bitmap of locked DOFs
		PxU32					limited;	// bitmap of limited DOFs
		PxU32					driving;	// bitmap of active drives (implies driven DOFs not locked)

		// tan-half and tan-quarter angles

		PxReal					thSwingY;
		PxReal					thSwingZ;
		PxReal					thSwingPad;

		PxReal					tqSwingY;
		PxReal					tqSwingZ;
		PxReal					tqSwingPad;

		PxReal					tqTwistLow;
		PxReal					tqTwistHigh;
		PxReal					tqTwistPad;

		PxReal					linearMinDist;	// linear limit minimum distance to get a good direction

		// projection quantities
		PxReal					projectionLinearTolerance;
		PxReal					projectionAngularTolerance;

		// forestall compiler complaints about not being able to generate a constructor
	private:
		D6JointData(const PxJointLinearLimit& linear, const PxJointAngularLimitPair& twist, const PxJointLimitCone& swing):
			linearLimit(linear), twistLimit(twist), swingLimit(swing) {}
	};

	typedef Joint<PxD6Joint, PxD6JointGeneratedValues> D6JointT;
    
    class D6Joint : public Joint<PxD6Joint, PxD6JointGeneratedValues>
	{
	public:
// PX_SERIALIZATION
									D6Joint(PxBaseFlags baseFlags) : D6JointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context);
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		D6Joint*		createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
		virtual ~D6Joint()
		{
			if(getBaseFlags()&PxBaseFlag::eOWNS_MEMORY)
				PX_FREE(mData);
		}

		D6Joint(const PxTolerancesScale& scale,
				PxRigidActor* actor0, const PxTransform& localFrame0, 
				PxRigidActor* actor1, const PxTransform& localFrame1);


		PxReal				getTwist()							const;
		PxReal				getSwingYAngle()					const;
		PxReal				getSwingZAngle()					const;


		PxD6Motion::Enum	getMotion(PxD6Axis::Enum index)		const;
		void				setMotion(PxD6Axis::Enum index, PxD6Motion::Enum t);

		PxD6JointDrive		getDrive(PxD6Drive::Enum index)		const;
		void				setDrive(PxD6Drive::Enum index, const PxD6JointDrive& d);

		PxJointLinearLimit	getLinearLimit()					const;
		void				setLinearLimit(const PxJointLinearLimit& l);

		PxJointAngularLimitPair	getTwistLimit()					const;
		void				setTwistLimit(const PxJointAngularLimitPair& l);

		PxJointLimitCone	getSwingLimit()						const;
		void				setSwingLimit(const PxJointLimitCone& l);

		PxTransform			getDrivePosition()					const;
		void				setDrivePosition(const PxTransform& pose);

		void				getDriveVelocity(PxVec3& linear,
											 PxVec3& angular)	const;
																	
		void				setDriveVelocity(const PxVec3& linear,
											 const PxVec3& angular);

		void				setProjectionAngularTolerance(PxReal tolerance);
		PxReal				getProjectionAngularTolerance()					const;

		void				setProjectionLinearTolerance(PxReal tolerance);
		PxReal				getProjectionLinearTolerance()					const;

		void				visualize(PxRenderBuffer& out,
									  const void* constantBlock,
									  const PxTransform& body0Transform,
									  const PxTransform& body1Transform,
									  PxReal frameScale,
									  PxReal limitScale,
									  PxU32 flags)					const;

		bool				attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1);

	private:

		static PxConstraintShaderTable sShaders;

		D6JointData& data() const				
		{	
			return *static_cast<D6JointData*>(mData);
		}

		bool active(const PxD6Drive::Enum index) const
		{
			PxD6JointDrive& d = data().drive[index];
			return d.stiffness!=0 || d.damping != 0;
		}

		void* prepareData();

		bool	mRecomputeMotion;
		bool	mRecomputeLimits;
		bool	mPadding[2];	// PT: padding from prev bools
	};

} // namespace Ext

namespace Ext
{
	extern "C" PxU32 D6JointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		PxConstraintInvMassScale& invMassScale,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w);

}

}

#endif
