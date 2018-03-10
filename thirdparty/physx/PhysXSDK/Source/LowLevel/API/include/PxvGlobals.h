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


#ifndef PXD_INIT_H
#define PXD_INIT_H

#include "PxvConfig.h"
#include "PsBasicTemplates.h"

namespace physx
{

/*!
\file
PhysX Low-level, Memory management
*/

/************************************************************************/
/* Error Handling                                                       */
/************************************************************************/


enum PxvErrorCode
{
	PXD_ERROR_NO_ERROR = 0,
	PXD_ERROR_INVALID_PARAMETER,
	PXD_ERROR_INVALID_PARAMETER_SIZE,
	PXD_ERROR_INTERNAL_ERROR,
	PXD_ERROR_NOT_IMPLEMENTED,
	PXD_ERROR_NO_CONTEXT,
	PXD_ERROR_NO_TASK_MANAGER,
	PXD_ERROR_WARNING
};

class PxShape;
class PxRigidActor;
struct PxsShapeCore;
struct PxsRigidCore;

struct PxvOffsetTable
{
	PX_FORCE_INLINE PxvOffsetTable() {}

	PX_FORCE_INLINE const PxShape* convertPxsShape2Px(const PxsShapeCore* pxs) const
	{
		return shdfnd::pointerOffset<const PxShape*>(pxs, pxsShapeCore2PxShape); 
	}

	PX_FORCE_INLINE const PxRigidActor* convertPxsRigidCore2PxRigidBody(const PxsRigidCore* pxs) const
	{
		return shdfnd::pointerOffset<const PxRigidActor*>(pxs, pxsRigidCore2PxRigidBody); 
	}

	PX_FORCE_INLINE const PxRigidActor* convertPxsRigidCore2PxRigidStatic(const PxsRigidCore* pxs) const
	{
		return shdfnd::pointerOffset<const PxRigidActor*>(pxs, pxsRigidCore2PxRigidStatic); 
	}

	ptrdiff_t	pxsShapeCore2PxShape;
	ptrdiff_t	pxsRigidCore2PxRigidBody;
	ptrdiff_t	pxsRigidCore2PxRigidStatic;
};
extern PxvOffsetTable gPxvOffsetTable;

/*!
Initialize low-level implementation.
*/

void PxvInit(const PxvOffsetTable& offsetTable);


/*!
Shut down low-level implementation.
*/
void PxvTerm();

/*!
Initialize low-level implementation.
*/

void PxvRegisterArticulations();

void PxvRegisterHeightFields();

void PxvRegisterLegacyHeightFields();

#if PX_USE_PARTICLE_SYSTEM_API
void PxvRegisterParticles();
#endif

#if PX_SUPPORT_GPU_PHYSX
class PxPhysXGpu* PxvGetPhysXGpu(bool createIfNeeded);
#endif

}

#endif
