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



#ifndef PXC_ARTICULATION_REFERENCE_H
#define PXC_ARTICULATION_REFERENCE_H

// a per-row struct where we put extra data for debug and setup - ultimately this will move to be just
// debug only



#include "PxcArticulation.h"
#include "PxcArticulationScalar.h"
#include "PxcArticulationFnsScalar.h"
#include "PxcSpatial.h"

#if PXC_ARTICULATION_DEBUG_VERIFY

namespace physx
{

PX_FORCE_INLINE Cm::SpatialVector propagateVelocity(const PxcFsRow& row, 
													const PxcFsJointVectors& jv,
													const PxVec3& SZ, 
													const Cm::SpatialVector& v,
													const PxcFsRowAux& aux)
{
	typedef PxcArticulationFnsScalar Fns;

	Cm::SpatialVector w = Fns::translateMotion(-getParentOffset(jv), v);
	PxVec3 DSZ = Fns::multiply(row.D, SZ);

	PxVec3 n = Fns::axisDot(getDSI(row), w) + DSZ;
	Cm::SpatialVector result = w - Cm::SpatialVector(getJointOffset(jv).cross(n),n);
#if PXC_ARTICULATION_DEBUG_VERIFY
	Cm::SpatialVector check = PxcArticulationRef::propagateVelocity(row, jv, SZ, v, aux);
	PX_ASSERT((result-check).magnitude()<1e-5*PxMax(check.magnitude(), 1.0f));
#endif
	return result;
}

PX_FORCE_INLINE Cm::SpatialVector propagateImpulse(const PxcFsRow& row, 
												   const PxcFsJointVectors& jv,
												   PxVec3& SZ, 
												   const Cm::SpatialVector& Z,
												   const PxcFsRowAux& aux)
{
	typedef PxcArticulationFnsScalar Fns;

	SZ = Z.angular + Z.linear.cross(getJointOffset(jv));
	Cm::SpatialVector result = Fns::translateForce(getParentOffset(jv), Z - Fns::axisMultiply(getDSI(row), SZ));
#if PXC_ARTICULATION_DEBUG_VERIFY
	PxVec3 SZcheck;
	Cm::SpatialVector check = PxcArticulationRef::propagateImpulse(row, jv, SZcheck, Z, aux);
	PX_ASSERT((result-check).magnitude()<1e-5*PxMax(check.magnitude(), 1.0f));
	PX_ASSERT((SZ-SZcheck).magnitude()<1e-5*PxMax(SZcheck.magnitude(), 1.0f));
#endif
	return result;
}

}
#endif

#endif //PXC_ARTICULATION
