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


#ifndef NP_SPHERICALJOINTCONSTRAINT_H
#define NP_SPHERICALJOINTCONSTRAINT_H

#include "ExtJoint.h"
#include "PxSphericalJoint.h"
#include "CmUtils.h"

namespace physx
{
struct PxSphericalJointGeneratedValues;
namespace Ext
{
	struct SphericalJointData: public JointData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
		PxJointLimitCone		limit;
		PxReal					tanQYLimit;
		PxReal					tanQZLimit;
		PxReal					tanQPad;

		PxReal					projectionLinearTolerance;

		PxSphericalJointFlags	jointFlags;
		// forestall compiler complaints about not being able to generate a constructor
	private:
		SphericalJointData(const PxJointLimitCone &cone):
			limit(cone) {}
	};
    
    typedef Joint<PxSphericalJoint, PxSphericalJointGeneratedValues> SphericalJointT;
   
	class SphericalJoint : public SphericalJointT
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
									SphericalJoint(PxBaseFlags baseFlags) : SphericalJointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context);
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		SphericalJoint*	createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
		virtual						~SphericalJoint()
		{
			if(getBaseFlags()&PxBaseFlag::eOWNS_MEMORY)
				PX_FREE(mData);
		}

		SphericalJoint(const PxTolerancesScale& /*scale*/,
					   PxRigidActor* actor0, const PxTransform& localFrame0, 
					   PxRigidActor* actor1, const PxTransform& localFrame1)
		: SphericalJointT(PxJointConcreteType::eSPHERICAL, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
		{
			SphericalJointData* data = reinterpret_cast<SphericalJointData*>(PX_ALLOC(sizeof(SphericalJointData), PX_DEBUG_EXP("SphericalJointData")));
			Cm::markSerializedMem(data, sizeof(SphericalJointData));
			mData = data;

			initCommonData(*data,actor0, localFrame0, actor1, localFrame1);
			data->projectionLinearTolerance = 1e10f;
			data->limit = PxJointLimitCone(PxPi/2, PxPi/2);
			data->jointFlags = PxSphericalJointFlags();
		}

		void					setProjectionLinearTolerance(PxReal distance);
		PxReal					getProjectionLinearTolerance() const;

		void					setLimitCone(const PxJointLimitCone &limit);
		PxJointLimitCone		getLimitCone() const;

		PxSphericalJointFlags	getSphericalJointFlags(void) const;
		void					setSphericalJointFlags(PxSphericalJointFlags flags);
		void					setSphericalJointFlag(PxSphericalJointFlag::Enum flag, bool value);

		void*					prepareData();

		bool					attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1);
	private:

		static PxConstraintShaderTable sShaders;

		PX_FORCE_INLINE SphericalJointData& data() const				
		{	
			return *static_cast<SphericalJointData*>(mData);
		}
	};

} // namespace Ext

namespace Ext
{
	extern "C"  PxU32 SphericalJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		PxConstraintInvMassScale& invMassScale,
		const void* constantBlock,							  
		const PxTransform& bA2w,
		const PxTransform& bB2w);
}

}

#endif
