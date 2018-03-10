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


#include "PxsMaterialCombiner.h"
#include "PsMathUtils.h"
#include "CmPhysXCommon.h"
#include "PsFoundation.h"

namespace physx   
{


PxsMaterialCombiner::PxsMaterialCombiner(PxReal staticFrictionScaling, PxReal dynamicFrictionScaling)
:	mStaticFrictionScaling(staticFrictionScaling), mDynamicFrictionScaling(dynamicFrictionScaling)
{}


PxReal PxsMaterialCombiner::combineRestitution(const PxsMaterialData& mat0, const PxsMaterialData& mat1)
{
	/*return combineScalars(mat0.restitution, mat1.restitution, PxMax(mat0.restitutionCombineMode, mat1.restitutionCombineMode));*/
	return combineScalars(mat0.restitution, mat1.restitution, PxMax(mat0.getRestitutionCombineMode(), mat1.getRestitutionCombineMode()));
}
   
PxsMaterialCombiner::PxsCombinedMaterial PxsMaterialCombiner::combineIsotropicFriction(const PxsMaterialData& mat0, const PxsMaterialData& mat1)
{
	PxsCombinedMaterial dest;

	dest.flags = (mat0.flags | mat1.flags); //& (PxMaterialFlag::eDISABLE_STRONG_FRICTION|PxMaterialFlag::eDISABLE_FRICTION);	//eventually set DisStrongFric flag, lower all others.

	if (!(dest.flags & PxMaterialFlag::eDISABLE_FRICTION))
	{
		const PxI32 fictionCombineMode = PxMax(mat0.getFrictionCombineMode(), mat1.getFrictionCombineMode());
		PxReal dynFriction = 0.f;
		PxReal staFriction = 0.f;


		switch (fictionCombineMode)
		{
		case PxCombineMode::eAVERAGE:
			dynFriction = 0.5f * (mat0.dynamicFriction + mat1.dynamicFriction);
			staFriction = 0.5f * (mat0.staticFriction + mat1.staticFriction);
			break;
		case PxCombineMode::eMIN:
			dynFriction = PxMin(mat0.dynamicFriction, mat1.dynamicFriction);
			staFriction = PxMin(mat0.staticFriction, mat1.staticFriction);
			break;
		case PxCombineMode::eMULTIPLY:
			dynFriction = (mat0.dynamicFriction * mat1.dynamicFriction);
			staFriction = (mat0.staticFriction * mat1.staticFriction);
			break;
		case PxCombineMode::eMAX:
			dynFriction = PxMax(mat0.dynamicFriction, mat1.dynamicFriction);
			staFriction = PxMax(mat0.staticFriction, mat1.staticFriction);
			break;
		}   

		dynFriction*=mDynamicFrictionScaling;
		staFriction*=mStaticFrictionScaling;
		//isotropic case
		const PxReal fDynFriction = PxMax(dynFriction, 0.f);

		const PxReal fStaFriction = physx::intrinsics::fsel(staFriction - fDynFriction, staFriction, fDynFriction);
		dest.dynFriction = fDynFriction;
		dest.staFriction = fStaFriction;
	}
	else
	{
		dest.flags |= PxMaterialFlag::eDISABLE_STRONG_FRICTION;
		dest.staFriction = 0.0f;
		dest.dynFriction = 0.0f;
	}

	return dest;
}
}
