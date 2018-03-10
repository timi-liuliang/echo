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


#ifndef NP_DISTANCEJOINTCONSTRAINT_H
#define NP_DISTANCEJOINTCONSTRAINT_H

#include "PsUserAllocated.h"
#include "ExtJoint.h"
#include "PxDistanceJoint.h"
#include "PxTolerancesScale.h"
#include "CmUtils.h"

namespace physx
{
struct PxDistanceJointGeneratedValues;
namespace Ext
{

	struct DistanceJointData : public JointData
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

							PxReal					minDistance;
							PxReal					maxDistance;
							PxReal					tolerance;
							PxReal					stiffness;
							PxReal					damping;

							PxDistanceJointFlags	jointFlags;
	};

    typedef Joint<PxDistanceJoint, PxDistanceJointGeneratedValues> DistanceJointT;
    class DistanceJoint : public DistanceJointT
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
									DistanceJoint(PxBaseFlags baseFlags) : DistanceJointT(baseFlags) {}
		virtual		void			exportExtraData(PxSerializationContext& context);
					void			importExtraData(PxDeserializationContext& context);
					void			resolveReferences(PxDeserializationContext& context);
		static		DistanceJoint*	createObject(PxU8*& address, PxDeserializationContext& context);
		static		void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
		virtual ~DistanceJoint()
		{
			if(getBaseFlags()&PxBaseFlag::eOWNS_MEMORY)
				PX_FREE(mData);
		}

		DistanceJoint(const PxTolerancesScale& scale,
					  PxRigidActor* actor0, const PxTransform& localFrame0, 
					  PxRigidActor* actor1, const PxTransform& localFrame1)
		: DistanceJointT(PxJointConcreteType::eDISTANCE, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
		{
			DistanceJointData* data = reinterpret_cast<DistanceJointData*>(PX_ALLOC(sizeof(DistanceJointData), PX_DEBUG_EXP("DistanceJointData")));
			Cm::markSerializedMem(data, sizeof(DistanceJointData));
			mData = data;

			initCommonData(*data,actor0, localFrame0, actor1, localFrame1);

			data->stiffness = 0;
			data->damping = 0;
			data->minDistance = 0;
			data->maxDistance = 0;
			data->tolerance = 0.025f * scale.length;
			data->jointFlags = PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;
		}

		PxReal					getDistance()						const;

		void					setMinDistance(PxReal distance);
		PxReal					getMinDistance()					const;

		void					setMaxDistance(PxReal distance);
		PxReal					getMaxDistance()					const;

		void					setTolerance(PxReal tolerance);
		PxReal					getTolerance()						const;

		void					setStiffness(PxReal spring);
		PxReal					getStiffness()							const;

		void					setDamping(PxReal damping);
		PxReal					getDamping()						const;
		
		PxDistanceJointFlags	getDistanceJointFlags(void)			const;
		void					setDistanceJointFlags(PxDistanceJointFlags flags);
		void					setDistanceJointFlag(PxDistanceJointFlag::Enum flag, bool value);

		bool					attach(PxPhysics &physics, PxRigidActor* actor0, PxRigidActor* actor1);
	private:

		static PxConstraintShaderTable sShaders;

		PX_FORCE_INLINE DistanceJointData& data() const				
		{	
			return *static_cast<DistanceJointData*>(mData);
		}
	};

} // namespace Ext

namespace Ext
{
	extern "C" PxU32 DistanceJointSolverPrep(Px1DConstraint* constraints,
		PxVec3& body0WorldOffset,
		PxU32 maxConstraints,
		PxConstraintInvMassScale& invMassScale,
		const void* constantBlock,
		const PxTransform& bA2w,
		const PxTransform& bB2w);
}

}

#endif
