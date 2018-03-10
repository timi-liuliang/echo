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


#ifndef PXD_SHADER_H
#define PXD_SHADER_H

#include "PxvConfig.h"
#include "PxvDynamics.h"

#include "PxVec3.h"
#include "PxTransform.h"
#include "PxConstraint.h"

namespace physx
{

class PxsRigidBody;


PX_ALIGN_PREFIX(16)
struct PxsConstraintWriteback
{
	PxVec3	linearImpulse;
	PxU32	broken;
	PxVec3  angularImpulse;
	PxU32	pad;
}
PX_ALIGN_SUFFIX(16);


#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif
PX_ALIGN_PREFIX(16)
struct PxsConstraint
{
public:
	static const PxU32 MAX_CONSTRAINTS = 12;

	PxReal								linBreakForce;														//0
	PxReal								angBreakForce;														//4
	PxU32								solverPrepSpuByteSize;												//8
	PxU32								constantBlockSize;													//12

	PxConstraintSolverPrep				solverPrep;															//16
	PxConstraintProject					project;															//20
	void*								solverPrepSpu;														//24
	void*								constantBlock;														//28

	PxsConstraintWriteback*				writeback;															//32
	PxsRigidBody*						body0;																//36
	PxsRigidBody*						body1;																//40

	// lifted here from the bodies to eliminate an extra level of pointer chasing on SPU

	PxsBodyCore*						bodyCore0;															//44
	PxsBodyCore*						bodyCore1;															//48
	PxU32								flags;																//52
	PxU32								index;																//56
}
PX_ALIGN_SUFFIX(16);
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

#if !defined(PX_P64)
PX_COMPILE_TIME_ASSERT(64==sizeof(PxsConstraint));
#endif

}

#endif
