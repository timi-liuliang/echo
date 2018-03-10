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


#ifndef PXS_ARTICULATION_HELPER_H
#define PXS_ARTICULATION_HELPER_H

#include "PxcArticulation.h"
#include "PxcSpatial.h"

namespace physx
{

struct PxcArticulationSolverDesc;
struct PxcSolverConstraintDesc;
struct PxsBodyCore;
struct PxsArticulationJointCore;


class PxcConstraintBlockStream;
class PxcRigidBody;
class PxsConstraintBlockManager;

struct PxcSolverConstraint1DExt;

namespace Cm
{
	class EventProfiler;
}

struct PxcArticulationJointTransforms
{
	PxTransform		cA2w;				// joint parent frame in world space 
	PxTransform		cB2w;				// joint child frame in world space
	PxTransform		cB2cA;				// joint relative pose in world space
};



class PxcArticulationHelper
{
public:
	static PxU32	computeUnconstrainedVelocities(const PxcArticulationSolverDesc& desc,
												   PxReal dt,
												   PxcConstraintBlockStream& stream,
												   PxcSolverConstraintDesc* constraintDesc,
												   PxU32& acCount,
												   Cm::EventProfiler& profiler,
												   PxsConstraintBlockManager& constraintBlockManager);

	static void		updateBodies(const PxcArticulationSolverDesc& desc,
							 	 PxReal dt);


	static void		getImpulseResponse(const PxcFsData& matrix, 
									   PxU32 linkID, 
									   const PxcSIMDSpatial& impulse,
									   PxcSIMDSpatial& deltaV);


	static PX_FORCE_INLINE 
			void	getImpulseResponse(const PxcFsData& matrix, 
									   PxU32 linkID, 
									   const Cm::SpatialVector& impulse,
									   Cm::SpatialVector& deltaV)
	{
		getImpulseResponse(matrix, linkID, reinterpret_cast<const PxcSIMDSpatial&>(impulse), reinterpret_cast<PxcSIMDSpatial&>(deltaV));
	}

	static void		getImpulseSelfResponse(const PxcFsData& matrix, 
										   PxU32 linkID0, 
										   const PxcSIMDSpatial& impulse0,
										   PxcSIMDSpatial& deltaV0,
										   PxU32 linkID1,
										   const PxcSIMDSpatial& impulse1,
										   PxcSIMDSpatial& deltaV1);

	static void		flushVelocity(PxcFsData& matrix);

	static void		saveVelocity(const PxcArticulationSolverDesc& m);

	static PX_FORCE_INLINE 
			PxcSIMDSpatial getVelocityFast(const PxcFsData& matrix,PxU32 linkID)
	{
		return getVelocity(matrix)[linkID];
	}

	static void		getDataSizes(PxU32 linkCount, PxU32 &solverDataSize, PxU32& totalSize, PxU32& scratchSize);

	static void		initializeDriveCache(PxcFsData &data,
										 PxU16 linkCount,
										 const PxsArticulationLink* links,
										 PxReal compliance,
										 PxU32 iterations,
										 char* scratchMemory,
										 PxU32 scratchMemorySize);

	static void		applyImpulses(const PxcFsData& matrix,
								  PxcSIMDSpatial* Z,
								  PxcSIMDSpatial* V);

private:
	static PxU32	getLtbDataSize(PxU32 linkCount);
	static PxU32	getFsDataSize(PxU32 linkCount);

	static void		prepareDataBlock(PxcFsData& fsData,
									 const PxsArticulationLink* links,
									 PxU16 linkCount,	
									 PxTransform* poses,
								 	 PxcFsInertia *baseInertia,
									 PxcArticulationJointTransforms* jointTransforms,
									 PxU32 expectedSize);

	static void		setInertia(PxcFsInertia& inertia,
							   const PxsBodyCore& body,
							   const PxTransform& pose);

	static void		setJointTransforms(PxcArticulationJointTransforms& transforms,
									   const PxTransform& parentPose,
									   const PxTransform& childPose,
									   const PxsArticulationJointCore& joint);

	static void		prepareLtbMatrix(PxcFsData& fsData,
									 const PxcFsInertia* baseInertia,
									 const PxTransform* poses,
									 const PxcArticulationJointTransforms* jointTransforms,
									 PxReal recipDt);

	static void		prepareFsData(PxcFsData& fsData,
								  const PxsArticulationLink* links);

	static PX_FORCE_INLINE PxReal getResistance(PxReal compliance);


	static void		createHardLimit(const PxcFsData& fsData,
									const PxsArticulationLink* links,
									PxU32 linkIndex,
									PxcSolverConstraint1DExt& s, 
									const PxVec3& axis, 
									PxReal err,
									PxReal recipDt);

	static void		createTangentialSpring(const PxcFsData& fsData,
										   const PxsArticulationLink* links,
										   PxU32 linkIndex,
										   PxcSolverConstraint1DExt& s, 
										   const PxVec3& axis, 
										   PxReal stiffness,
										   PxReal damping,
										   PxReal dt);

	static PxU32 setupSolverConstraints(PxcFsData& fsData, PxU32 solverDataSize,
													PxcConstraintBlockStream& stream,
													PxcSolverConstraintDesc* constraintDesc,
													const PxsArticulationLink* links,
													const PxcArticulationJointTransforms* jointTransforms,
													PxReal dt,
													PxU32& acCount,
													PxsConstraintBlockManager& constraintBlockManager);

	static void		computeJointDrives(PxcFsData& fsData,
									   Vec3V* drives, 
									   const PxsArticulationLink* links,
									   const PxTransform* poses, 
									   const PxcArticulationJointTransforms* transforms, 
									   const Mat33V* loads, 
									   PxReal dt);

};

}

#endif
