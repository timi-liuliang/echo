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


#ifndef PXC_NPCONTACTPREP_H
#define PXC_NPCONTACTPREP_H

#include "PxcSolverConstraintDesc.h"


namespace physx
{

struct PxcNpWorkUnit;
class PxcNpThreadContext;
struct PxsMaterialInfo;
struct PxcSolverBody;
struct PxcSolverBodyData;
class PxsDynamicsContext;
struct PxcSolverConstraintDesc;
class PxsConstraintBlockManager;
struct PxcSolverContact4Desc;
class PxcNpThreadContext;

#ifndef __SPU__
#define CREATE_FINALIZE_SOLVER_CONTACT_METHOD_ARGS			\
	PxcNpWorkUnit& n,										\
	PxcNpThreadContext& threadContext,						\
	bool forceContacts,										\
	bool perPointFriction,									\
	PxcSolverConstraintDesc& desc,							\
	const PxcSolverBody* sBody0,							\
	const PxcSolverBody* sBody1,							\
	const PxcSolverBodyData* solverBodyData0,				\
	const PxcSolverBodyData* solverBodyData1,				\
	const PxsRigidCore* body0,								\
	const PxsRigidCore* body1,								\
	const PxReal invDtF32,									\
	PxReal bounceThresholdF32,								\
	PxReal frictionOffsetThreshold,							\
	PxReal	correlationDistance,							\
	PxsConstraintBlockManager& constraintBlockManager,		\
	PxFrictionType::Enum frictionType
#else
#define CREATE_FINALIZE_SOLVER_CONTACT_METHOD_ARGS			\
	PxcNpWorkUnit& n,										\
	PxcNpThreadContext& threadContext,						\
	bool forceContacts,										\
	bool perPointFriction,									\
	PxcSolverConstraintDesc& desc,							\
	const PxcSolverBody* sBody0,							\
	const PxcSolverBody* sBody1,							\
	const PxcSolverBodyData* solverBodyData0,				\
	const PxcSolverBodyData* solverBodyData1,				\
	const PxsRigidCore* body0,								\
	const PxsRigidCore* body1,								\
	const PxReal invDtF32,									\
	PxReal bounceThresholdF32,								\
	PxReal frictionOffsetThreshold,							\
	PxReal	correlationDistance,							\
	PxsConstraintBlockManager& constraintBlockManager,		\
	PxFrictionType::Enum frictionType,							\
	bool& overflowed
#endif

#define CREATE_FINALIZE_SOVLER_CONTACT_METHOD_ARGS_4									\
								 PxcNpWorkUnit** n,										\
								 PxcNpThreadContext& threadContext,						\
								 PxcSolverConstraintDesc** desc,						\
								 const PxcSolverBody** sBody0,							\
								 const PxcSolverBody** sBody1,							\
								 const PxcSolverBodyData** solverBodyData0,				\
								 const PxcSolverBodyData** solverBodyData1,				\
								 const PxsRigidCore** body0,							\
								 const PxsRigidCore** body1,							\
								 const PxReal invDtF32,									\
								 PxReal bounceThresholdF32,								\
								 PxReal	frictionThresholdF32,							\
								 PxReal	correlationDistanceF32,							\
								 PxsConstraintBlockManager& constraintBlockManager,		\
								 PxFrictionType::Enum frictionType							

	
/*!
Method prototype for create finalize solver contact
*/

typedef	bool (*PxcCreateFinalizeSolverContactMethod)(CREATE_FINALIZE_SOLVER_CONTACT_METHOD_ARGS);

extern PxcCreateFinalizeSolverContactMethod createFinalizeMethods[3];


static const PxReal PXC_SAME_NORMAL = 0.999f;
static const PxReal PXC_SAME_NORMAL_PF = 0.999f;

bool finishContacts(PxcNpWorkUnit& n, PxcNpThreadContext& threadContext, PxsMaterialInfo* pMaterialInfo);

PxU32 writeCompressedContact(const Gu::ContactPoint* const PX_RESTRICT contactPoints, const PxU32 numContactPoints, PxcNpThreadContext& threadContext,
									PxU16& writtenContactCount, PxU8*& compressedContacts, PxU32& compressedContactSize, PxReal*& contactForces, PxU32 contactForceByteSize,
									PxU16& statusFlag, const PxsMaterialManager* matManager, bool hasModifiableContacts, bool forceNoResponse, PxsMaterialInfo* PX_RESTRICT pMaterial,
									PxU32 additionalHeaderSize = 0, bool useConstraintData = false, PxcNpThreadContext* islandContex = NULL, bool insertAveragePoint = false);

typedef	PxcSolverConstraintPrepState::Enum (*PxcCreateFinalizeSolverContactMethod4)(CREATE_FINALIZE_SOVLER_CONTACT_METHOD_ARGS_4);

extern PxcCreateFinalizeSolverContactMethod4 createFinalizeMethods4[3];


bool createFinalizeSolverContacts(PxcNpWorkUnit& n,
								 PxcNpThreadContext& threadContext,
								 bool forceContacts,
								 bool perPointFriction,
								 PxcSolverConstraintDesc& desc,
								 const PxcSolverBody* sBody0,
								 const PxcSolverBody* sBody1,
								 const PxcSolverBodyData* solverBodyData0,
								 const PxcSolverBodyData* solverBodyData1,
								 const PxsRigidCore* body0,
								 const PxsRigidCore* body1,
								 const PxReal invDtF32,
								 PxReal bounceThresholdF32,
								 PxReal frictionOffsetThreshold,
								 PxReal correlationDistance,
								 PxsConstraintBlockManager& constraintBlockManager,
								 PxFrictionType::Enum frictionType
#ifdef __SPU__
								 , bool& overflowed
#endif
								 );

PxcSolverConstraintPrepState::Enum createFinalizeSolverContacts4(PxcNpWorkUnit** n,
								 PxcNpThreadContext& threadContext,
								 PxcSolverConstraintDesc** desc,
								 const PxcSolverBody** sBody0,
								 const PxcSolverBody** sBody1,
								 const PxcSolverBodyData** solverBodyData0,
								 const PxcSolverBodyData** solverBodyData1,
								 const PxsRigidCore** body0,
								 const PxsRigidCore** body1,
								 const PxReal invDtF32,
								 PxReal bounceThresholdF32,
								 PxReal frictionOffsetThreshold,
								 PxReal correlationDistance,
								 PxsConstraintBlockManager& constraintBlockManager,
								 PxFrictionType::Enum frictionType);


bool createFinalizeSolverContactsCoulomb(PxcNpWorkUnit& n,
								 PxcNpThreadContext& threadContext,
								 bool forceContacts,
								 bool perPointFriction,
								 PxcSolverConstraintDesc& desc,
								 const PxcSolverBody* sBody0,
								 const PxcSolverBody* sBody1,
								 const PxcSolverBodyData* solverBodyData0,
								 const PxcSolverBodyData* solverBodyData1,
								 const PxsRigidCore* body0,
								 const PxsRigidCore* body1,
								 const PxReal invDtF32,
								 PxReal bounceThresholdF32,
								 PxReal frictionOffsetThreshold,
								 PxReal correlationDistance,
								 PxsConstraintBlockManager& constraintBlockManager,
								 PxFrictionType::Enum frictionType
#ifdef __SPU__
								 , bool& overflowed
#endif
								 );

PxcSolverConstraintPrepState::Enum createFinalizeSolverContacts4Coulomb(PxcNpWorkUnit** n,
								 PxcNpThreadContext& threadContext,
								 PxcSolverConstraintDesc** desc,
								 const PxcSolverBody** sBody0,
								 const PxcSolverBody** sBody1,
								 const PxcSolverBodyData** solverBodyData0,
								 const PxcSolverBodyData** solverBodyData1,
								 const PxsRigidCore** body0,
								 const PxsRigidCore** body1,
								 const PxReal invDtF32,
								 PxReal bounceThresholdF32,
								 PxReal frictionOffsetThreshold,
								 PxReal correlationDistance,
								 PxsConstraintBlockManager& constraintBlockManager,
								 PxFrictionType::Enum frictionType);


void setupFinalizeSolverConstraints(PxcNpWorkUnit& n,
							const Gu::ContactBuffer& buffer,
							const PxcCorrelationBuffer& c,
							const PxTransform& bodyFrame0,
							const PxTransform& bodyFrame1,
							bool perPointFriction,
							PxU8* workspace,
							const PxcSolverBody& b0,
							const PxcSolverBody& b1,
							const PxcSolverBodyData& data0,
							const PxcSolverBodyData& data1,
							const PxReal invDtF32,
							PxReal bounceThresholdF32,
							PxReal invMassScale0, PxReal invInertiaScale0, 
							PxReal invMassScale1, PxReal invInertiaScale1);

void setupFinalizeSolverConstraints4(PxcSolverContact4Desc* PX_RESTRICT descs, PxcNpThreadContext& threadContext, PxU8* PX_RESTRICT workspace, 
									 const PxReal invDtF32, PxReal bounceThresholdF32,
									 const Ps::aos::Vec4VArg invMassScale0, const Ps::aos::Vec4VArg invInertiaScale0, 
									 const Ps::aos::Vec4VArg invMassScale1, const Ps::aos::Vec4VArg invInertiaScale1
									 );


void setupFinalizeSolverConstraintsCoulomb(PxcNpWorkUnit& n,
							const Gu::ContactBuffer& buffer,
							const PxcCorrelationBufferCoulomb& c,
							const PxTransform& bodyFrame0,
							const PxTransform& bodyFrame1,
							bool perPointFriction,
							PxU8* workspace,
							PxcSolverBody& b0,
							PxcSolverBody& b1,
							PxcSolverBodyData& data0,
							PxcSolverBodyData& data1,
							const PxReal invDtF32,
							PxReal bounceThresholdF32,
							PxU32 numFrictionsPerPoint,
							PxReal invMassScale0, PxReal invInertiaScale0, 
							PxReal invMassScale1, PxReal invInertiaScale1,
							const PxReal maxPenetration);

bool setupFinalizeSolverConstraintsCoulomb4(PxcSolverContact4Desc* PX_RESTRICT descs, PxcNpThreadContext& threadContext, PxU8* PX_RESTRICT workspace, 
											const PxReal invDtF32, PxReal bounceThresholdF32, PxcCorrelationBufferCoulomb& c, const PxU32 numFrictionPerPoint,
											const PxU32 numContactPoints4, const PxU32 solverConstraintByteSize,
											const Ps::aos::Vec4VArg invMassScale0, const Ps::aos::Vec4VArg invInertiaScale0, 
											const Ps::aos::Vec4VArg invMassScale1, const Ps::aos::Vec4VArg invInertiaScale);


}

#endif
