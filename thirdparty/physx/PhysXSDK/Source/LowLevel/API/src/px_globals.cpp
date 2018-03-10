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

#include "PxvGlobals.h"
#include "PxcPoolMalloc.h"
#include "PxcArticulationPImpl.h"
#include "PxcContactMethodImpl.h"
#include "PxcSolverConstraintTypes.h"
#include "PxsSolverCoreGeneral.h"
#include "PxsContext.h"


#if PX_SUPPORT_GPU_PHYSX
#if defined(PX_WINDOWS)
#include "windows/PxvWindowsLoadPhysxGpu.h"
#elif defined(PX_LINUX)
#include "linux/PxvLinuxLoadPhysxGpu.h"
#endif
physx::PxPhysXGpu* gPxPhysXGpu;
#endif

#ifdef PX_PS3
namespace physx
{
	void gPS3RegisterHeightFields();
	void gPS3RegisterParticles();
}
#endif

namespace physx
{

PxvOffsetTable gPxvOffsetTable;

void PxvRegisterArticulations()
{
	PxcArticulationPImpl::sComputeUnconstrainedVelocities = &PxcArticulationHelper::computeUnconstrainedVelocities;
	PxcArticulationPImpl::sUpdateBodies = &PxcArticulationHelper::updateBodies;
	PxcArticulationPImpl::sSaveVelocity = &PxcArticulationHelper::saveVelocity;

	PxsSolverCoreRegisterArticulationFns();
}


bool PxcContactSphereHeightFieldUnified	(CONTACT_METHOD_ARGS);
bool PxcContactCapsuleHeightFieldUnified(CONTACT_METHOD_ARGS);
bool PxcContactBoxHeightFieldUnified	(CONTACT_METHOD_ARGS);
bool PxcContactConvexHeightFieldUnified	(CONTACT_METHOD_ARGS);

bool PxcPCMContactSphereHeightField		(CONTACT_METHOD_ARGS);
bool PxcPCMContactCapsuleHeightField	(CONTACT_METHOD_ARGS);
bool PxcPCMContactBoxHeightField		(CONTACT_METHOD_ARGS);
bool PxcPCMContactConvexHeightField		(CONTACT_METHOD_ARGS);

bool PxcContactSphereHeightField		(CONTACT_METHOD_ARGS);
bool PxcContactCapsuleHeightField		(CONTACT_METHOD_ARGS);
bool PxcContactBoxHeightField			(CONTACT_METHOD_ARGS);
bool PxcContactConvexHeightField		(CONTACT_METHOD_ARGS);

bool gUnifiedHeightfieldCollision = false;

void PxvRegisterHeightFields()
{
	g_ContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]			= PxcContactSphereHeightFieldUnified;
	g_ContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcContactCapsuleHeightFieldUnified;
	g_ContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcContactBoxHeightFieldUnified;
	g_ContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]		= PxcContactConvexHeightFieldUnified;

	g_PCMContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]		= PxcPCMContactSphereHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcPCMContactCapsuleHeightField;
	g_PCMContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcPCMContactBoxHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]	= PxcPCMContactConvexHeightField;
	gUnifiedHeightfieldCollision = true;
}

void PxvRegisterLegacyHeightFields()
{
	g_ContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]			= PxcContactSphereHeightField;
	g_ContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcContactCapsuleHeightField;
	g_ContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcContactBoxHeightField;
	g_ContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]		= PxcContactConvexHeightField;

	g_PCMContactMethodTable[PxGeometryType::eSPHERE][PxGeometryType::eHEIGHTFIELD]		= PxcContactSphereHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCAPSULE][PxGeometryType::eHEIGHTFIELD]		= PxcContactCapsuleHeightField;
	g_PCMContactMethodTable[PxGeometryType::eBOX][PxGeometryType::eHEIGHTFIELD]			= PxcContactBoxHeightField;
	g_PCMContactMethodTable[PxGeometryType::eCONVEXMESH][PxGeometryType::eHEIGHTFIELD]	= PxcContactConvexHeightField;
	gUnifiedHeightfieldCollision = false;
}

#if PX_USE_PARTICLE_SYSTEM_API

void PxvRegisterParticles()
{
	PxsContext::registerParticles();
#ifdef PX_PS3
	gPS3RegisterParticles();
#endif
}

#endif


#ifndef PX_PS3
void PxvInit(const PxvOffsetTable& offsetTable)
{
	PxcPoolMallocInit();

#if PX_SUPPORT_GPU_PHYSX
	gPxPhysXGpu = NULL;
#endif
	PxMemCopy(&gPxvOffsetTable, &offsetTable, sizeof(PxvOffsetTable));
}

void PxvTerm()
{
#if PX_SUPPORT_GPU_PHYSX
	if (gPxPhysXGpu)
	{
		gPxPhysXGpu->release();
		gPxPhysXGpu = NULL;
	}
#endif

	PxcPoolMallocTerm();
}
#endif

}

#if PX_SUPPORT_GPU_PHYSX
namespace physx
{
	PxPhysXGpu* PxvGetPhysXGpu(bool createIfNeeded)
	{
		if (!gPxPhysXGpu && createIfNeeded)
			gPxPhysXGpu = PxvLoadPhysXGpu();
		
		return gPxPhysXGpu;
	}
}
#endif
