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


         
#ifndef PXC_NPWORKUNIT_H
#define PXC_NPWORKUNIT_H

#include "PxcFrictionPatch.h"
#include "PxcNpThreadContext.h"
#include "PxcContactMethodImpl.h"
#include "PxcMaterialMethodImpl.h"
#include "PxcNpCache.h"
#include "PxcNpCCDState.h"

namespace physx
{

class PxvContact;

struct PxsRigidCore;
struct PxsShapeCore;

class PxsMaterialManager;

struct PxcNpWorkUnitFlag
{
	enum Enum
	{
		eOUTPUT_CONTACTS			= 1,
		eOUTPUT_CONSTRAINTS			= 2,
		eDISABLE_STRONG_FRICTION	= 4,
		eARTICULATION_BODY0			= 8,
		eARTICULATION_BODY1			= 16,
		eDYNAMIC_BODY0				= 32,
		eDYNAMIC_BODY1				= 64,
		eMODIFIABLE_CONTACT			= 128,
		eFORCE_THRESHOLD			= 256,
		eDETECT_DISCRETE_CONTACT	= 512
	};
};

struct PxcNpWorkUnitStatusFlag
{
	enum Enum
	{
		eHAS_NO_TOUCH				= (1 << 0),
		eHAS_TOUCH					= (1 << 1),
		eHAS_SOLVER_CONSTRAINTS		= (1 << 2),
		eHAS_CCD_RETOUCH			= (1 << 3),	// Marks pairs that are touching at a CCD pass and were touching at discrete collision or at a previous CCD pass already
												// but we can not tell whether they lost contact in a pass before. We send them as pure eNOTIFY_TOUCH_CCD events to the 
												// contact report callback if requested.
		eHAS_CONTACT_FORCES			= (1 << 4),	// The pair has a valid contact force buffer
		eTOUCH_KNOWN				= eHAS_NO_TOUCH | eHAS_TOUCH	// The touch status is known (if narrowphase never ran for a pair then no flag will be set)
	};
};

/*
 * A struct to record the number of work units a particular constraint pointer references.
 * This is created at the beginning of the constriant data and is used to bypass constraint preparation when the 
 * bodies are not moving a lot. In this case, we can recycle the constraints and save ourselves some cycles.
*/
struct PxcNpWorkUnit;
struct PxcNpWorkUnitBatch
{
	PxcNpWorkUnit* mUnits[4];
	PxU32 mSize;
};

struct PxcNpWorkUnit
{
	// OUTPUT - if outputSolverConstraints is true, info about the constraints, else NULL/zero

	// PT: changed alignment back to 16 here, else we waste almost 100 bytes in each contact manager. Talk to me if you want to change this again.
//	PX_ALIGN(128,PxU8*		solverConstraintPointer);	
	PX_ALIGN(16,PxU8*		solverConstraintPointer);	// OUTPUT								//4		//8
	PxU32					solverConstraintSize;		// OUTPUT								//8		//12
	PxU32					prevSolverConstraintSize;		// OUTPUT							//12	//16
	PxU8*					prevSolverConstraintPointer;	// OUTPUT							//16	//24
	PxU16					axisConstraintCount;		// OUTPUT								//18	//26
	bool					disableResponse;			// INPUT								//19	//27
	bool					disableCCDContact;			// INPUT								//20	//28

	PxU16					contactCount;				// OUTPUT - can be 16 bits				//22	//30
	PxU16					flags;						// INPUT								//24	//32
	PxU8*					ccdContacts;				// OUTPUT								//28	//40

	PxU8*					compressedContacts;			// OUTPUT								//32	//48
	PxU32					compressedContactSize;		// OUTPUT								//36	//52


	PxU32					frictionPatchCount;			// INOUT - can be 16 bits (or even 8)	//40	//56
	PxU8*					frictionDataPtr;			// INOUT								//44	//64
	PxcNpCache				pairCache;					// INOUT								//60	//76

	PxU32					index;						// INPUT								//64	//88

	//CCD

	const PxsRigidCore*		rigidCore0;					// INPUT								//68	//96
	const PxsRigidCore*		rigidCore1;					// INPUT								//72	//104
		
	const PxsShapeCore*		shapeCore0;					// INPUT								//76	//112
	const PxsShapeCore*		shapeCore1;					// INPUT								//80	//120

	const PxsMaterialManager*	materialManager;		// INPUT								//88	//128

	PxReal					dominance0;					// INPUT								//92	//132
	PxReal					dominance1;					// INPUT								//96	//136
	PxReal					restDistance;				// INPUT								//100	//140
	
	PxU8					geomType0;					// INPUT								//101	//141
	PxU8					geomType1;					// INPUT								//102	//142

	PxU16					statusFlags;				// OUTPUT (see PxcNpWorkUnitStatusFlag) //104	//144

	PxU32					mTransformCache0;			//										//108	//148
	PxU32					mTransformCache1;			//										//112	//152

#ifdef PX_PS3
	// this is here for the moment, until BitMap is on SPU as well
	PxU32					touchLost;
	PxU32					touchFound;
	PxU32					mPad[2];
#endif
};

#if !defined(PX_P64)
PX_COMPILE_TIME_ASSERT(0 == (sizeof(PxcNpWorkUnit) & 0x0f));
#endif

PX_FORCE_INLINE void PxcNpWorkUnitClearContactState(PxcNpWorkUnit& n)
{
	n.statusFlags = 0;
	n.contactCount = 0;
	n.ccdContacts = NULL;
	n.solverConstraintPointer = NULL;
	n.axisConstraintCount = 0;
	n.solverConstraintSize = 0;
	n.compressedContacts = NULL;
	n.compressedContactSize = 0;
}


PX_FORCE_INLINE void PxcNpWorkUnitClearCachedState(PxcNpWorkUnit& n)
{
	n.frictionDataPtr = 0;
	n.frictionPatchCount = 0;
	n.contactCount = 0;
	n.ccdContacts = NULL;
	n.compressedContacts = NULL;
	n.compressedContactSize = 0;
	n.pairCache.invalidate();
	n.statusFlags &= ~PxcNpWorkUnitStatusFlag::eHAS_SOLVER_CONSTRAINTS;
}

PX_FORCE_INLINE bool PxcNpWorkUnitIsBatchable(const PxcNpWorkUnit&)
{ 
#ifdef PX_PS3
	//return (!(n.flags & PxcNpWorkUnitFlag::eDO_CCD_FINALIZATION));
	return true;
#else
	return false;
#endif
}

// for discrete collision contacts
PX_FORCE_INLINE PxReal* PxcNpWorkUnitGetContactForcesDC(const PxcNpWorkUnit& n)
{
	PX_ASSERT(n.statusFlags & PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES);  // only call this if there is a force buffer
	return reinterpret_cast<PxReal*>(n.compressedContacts + ((n.compressedContactSize + 15) & ~15));
}


PX_FORCE_INLINE PxReal* PxcNpWorkUnitGetContactForcesDC_Safe(const PxcNpWorkUnit& n)
{
	if (n.statusFlags & PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES)
		return reinterpret_cast<PxReal*>(n.compressedContacts + ((n.compressedContactSize + 15) & ~15));
	else
		return NULL;
}


PX_FORCE_INLINE PxReal* PxcNpWorkUnitGetContactForcesDC_Safe(const PxcNpWorkUnit& n, PxU32& size)
{
	if (!(n.statusFlags & PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES))
	{
		size = 0;
		return NULL;
	}
	else
	{
		size = n.contactCount * sizeof(PxReal);
		return PxcNpWorkUnitGetContactForcesDC(n);
	}
}


#if !defined(PX_P64)
//PX_COMPILE_TIME_ASSERT(sizeof(PxcNpWorkUnit)==128);
#endif

}

#endif
