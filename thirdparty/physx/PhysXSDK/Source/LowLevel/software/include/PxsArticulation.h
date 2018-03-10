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


#ifndef PXS_ARTICULATION_H
#define PXS_ARTICULATION_H

#include "PxvArticulation.h"
#include "PxcArticulationHelper.h"
#include "PsArray.h"
#include "PxcSpatial.h"
#include "CmBitMap.h"

namespace physx
{
 
struct PxsArticulationJointCore;
class PxsDynamicsContext;
class PxsArticulationJoint;
class PxcRigidBody;

struct PxcSolverConstraintDesc;
struct PxsBodyCore;
class PxcConstraintBlockStream;
class PxsContext;

#if defined(PX_VC) 
    #pragma warning(push)   
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif
PX_ALIGN_PREFIX(64)
class PxsArticulation
{
public:
	// public interface

							PxsArticulation();
							~PxsArticulation();

	// solver methods
	PxU32					getLinkIndex(PxsArticulationLinkHandle handle)	const	{ return PxU32(handle&PXC_ARTICULATION_IDMASK); }
	PxU32					getBodyCount()									const	{ return mSolverDesc->linkCount;				}
	PxcFsData*				getFsDataPtr()									const	{ return mSolverDesc->fsData;					}
	PxU32					getTotalDataSize()								const	{ return mSolverDesc->solverDataSize;			}
	//PxU32					getTotalDataSize()								const	{ return mSolverDesc->totalDataSize;			}
	void					getSolverDesc(PxcArticulationSolverDesc& d)		const	{ d = *mSolverDesc;	}
	void					setSolverDesc(const PxcArticulationSolverDesc& d)		{ mSolverDesc = &d;	}

	const PxcArticulationSolverDesc* getSolverDescPtr()						const	{ return mSolverDesc;	}
	const PxsArticulationCore*	getCore()									const	{ return mSolverDesc->core;}	
	PxU16					getIterationCounts()							const	{ return mSolverDesc->core->solverIterationCounts; }

private:

	const PxcArticulationSolverDesc*	mSolverDesc;
	
	// debug quantities

	Cm::SpatialVector		computeMomentum(const PxcFsInertia *inertia) const;
	void					computeResiduals(const Cm::SpatialVector *, 
											 const PxcArticulationJointTransforms* jointTransforms,
											 bool dump = false) const;
	void					checkLimits() const;

} PX_ALIGN_SUFFIX(64);

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif
// we encode articulation link handles in the lower bits of the pointer, so the
// articulation has to be aligned, which in an aligned pool means we need to size it
// appropriately

PX_COMPILE_TIME_ASSERT((sizeof(PxsArticulation)&(PXC_ARTICULATION_MAX_SIZE-1))==0);

}

#endif
