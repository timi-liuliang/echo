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


#ifndef NP_FIXEDJOINTCONSTRAINT_H
#define NP_FIXEDJOINTCONSTRAINT_H

#include "ExtJoint.h"
#include "PxFixedJoint.h"
#include "CmUtils.h"

namespace physx
{
struct PxFixedJointGeneratedValues;
namespace Ext
{

	struct FixedJointData : public JointData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		PxReal	projectionLinearTolerance;
		PxReal	projectionAngularTolerance;
	};

	typedef Joint<PxFixedJoint, PxFixedJointGeneratedValues> FixedJointT;

	class FixedJoint : public FixedJointT
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
									FixedJoint(PxBaseFlags baseFlags) : FixedJointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context) const;
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		FixedJoint*		createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
		virtual						~FixedJoint()
		{
			if(getBaseFlags()&PxBaseFlag::eOWNS_MEMORY)
				PX_FREE(mData);
		}

		PxReal	getProjectionLinearTolerance() const;
		void	setProjectionLinearTolerance(PxReal tolerance);

		PxReal	getProjectionAngularTolerance() const;
		void	setProjectionAngularTolerance(PxReal tolerance);

		FixedJoint(const PxTolerancesScale& /*scale*/, 
				   PxRigidActor* actor0, const PxTransform& localFrame0, 
				   PxRigidActor* actor1, const PxTransform& localFrame1)
		: FixedJointT(PxJointConcreteType::eFIXED, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
		{
			FixedJointData* data = reinterpret_cast<FixedJointData*>(PX_ALLOC(sizeof(FixedJointData), PX_DEBUG_EXP("FixedJointData")));
			Cm::markSerializedMem(data, sizeof(FixedJointData));
			mData = data;
			data->projectionLinearTolerance = 1e10f;
			data->projectionAngularTolerance = PxPi;

			initCommonData(*data, actor0, localFrame0, actor1, localFrame1);
		}

		bool			attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1);

	private:

		static PxConstraintShaderTable sShaders;

		PX_FORCE_INLINE FixedJointData& data() const				
		{	
			return *static_cast<FixedJointData*>(mData);
		}
	};
} // namespace Ext

namespace Ext
{
	extern "C"  PxU32 FixedJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		PxConstraintInvMassScale& invMassScale,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w);
}

}

#endif
