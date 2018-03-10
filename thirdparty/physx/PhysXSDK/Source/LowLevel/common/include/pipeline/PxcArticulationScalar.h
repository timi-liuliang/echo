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



#ifndef PXC_ARTICULATION_SCALAR_H
#define PXC_ARTICULATION_SCALAR_H

// Scalar helpers for articulations

#include "PxcArticulation.h"
#include "PxcSpatial.h"
#include "PsFPU.h"

namespace physx
{

PX_FORCE_INLINE Cm::SpatialVector&	velocityRef(PxcFsData &m, PxU32 i)
{	
	return reinterpret_cast<Cm::SpatialVector&>(getVelocity(m)[i]); 
}

PX_FORCE_INLINE Cm::SpatialVector&	deferredVelRef(PxcFsData &m, PxU32 i)
{	
	return reinterpret_cast<Cm::SpatialVector&>(getDeferredVel(m)[i]); 
}

PX_FORCE_INLINE PxVec3& deferredSZRef(PxcFsData &m, PxU32 i)
{	
	return reinterpret_cast<PxVec3 &>(getDeferredSZ(m)[i]); 
}

PX_FORCE_INLINE const PxVec3& deferredSZ(const PxcFsData &s, PxU32 i) 
{	
	return reinterpret_cast<const PxVec3 &>(getDeferredSZ(s)[i]); 
}

PX_FORCE_INLINE Cm::SpatialVector& deferredZRef(PxcFsData &s)
{
	return unsimdRef(s.deferredZ);
}


PX_FORCE_INLINE const Cm::SpatialVector& deferredZ(const PxcFsData &s)
{
	return unsimdRef(s.deferredZ);
}

PX_FORCE_INLINE const PxVec3& getJointOffset(const PxcFsJointVectors& j) 
{	
	return reinterpret_cast<const PxVec3& >(j.jointOffset);		
}

PX_FORCE_INLINE const PxVec3& getParentOffset(const PxcFsJointVectors& j) 
{	
	return reinterpret_cast<const PxVec3&>(j.parentOffset);		
}




PX_FORCE_INLINE const Cm::SpatialVector* getDSI(const PxcFsRow& row)
{	
	return PxUnionCast<const Cm::SpatialVector*,const PxcSIMDSpatial*>(row.DSI); //reinterpret_cast<const Cm::SpatialVector*>(row.DSI); 
}

}

#endif //PXC_ARTICULATION_SCALAR
