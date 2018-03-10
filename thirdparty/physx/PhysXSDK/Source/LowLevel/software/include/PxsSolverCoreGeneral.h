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


#ifndef PXS_SOLVERCOREGENERAL_H
#define PXS_SOLVERCOREGENERAL_H

#include "PxsSolverCore.h"
#include "PxsSolverContext.h"

namespace physx
{

struct PxcFsData;

//-----------------------------------

//typedef void (*SolveMethod)(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
//typedef void (*SolveBlockMethod)(const PxcSolverConstraintDesc* desc, const PxU32 constraintCount, PxcSolverContext& cache);
//typedef void (*SolveWriteBackBlockMethod)(const PxcSolverConstraintDesc* desc, const PxU32 constraintCount, PxcSolverContext& cache,
//										  PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void solve1D				(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveContact			(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveExtContact		(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveExt1D				(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveContact_BStatic	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

void solveConclude1D				(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveConcludeContact			(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveConcludeExtContact		(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveConcludeExt1D				(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveConcludeContact_BStatic	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);



void solve1DBlock					(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactBlock				(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveExtContactBlock			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveExt1DBlock				(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContact_BStaticBlock		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactPreBlock			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactPreBlock_Static	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solve1D4_Block					(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solve1D4_StaticBlock			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);


void solve1DConcludeBlock				(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactConcludeBlock			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveExtContactConcludeBlock		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveExt1DConcludeBlock			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContact_BStaticConcludeBlock	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactPreBlock_Conclude		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactPreBlock_ConcludeStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solve1D4Block_Conclude				(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solve1D4StaticBlock_ConcludeStatic	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);

void solve1DBlockWriteBack				(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveContactBlockWriteBack			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveExtContactBlockWriteBack		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveExt1DBlockWriteBack			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveContact_BStaticBlockWriteBack	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void solveContactPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void solveContactPreBlock_WriteBackStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void solve1D4Block_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void writeBack1DBlock				(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void contactBlockWriteBack			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void extContactBlockWriteBack		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void ext1DBlockWriteBack			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void contactPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void writeBack1D4Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);




void solveContactCoulomb					(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveExtContactCoulomb				(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveContactCoulomb_BStatic			(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

void solveConcludeContactCoulomb			(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveConcludeExtContactCoulomb		(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveConcludeContactCoulomb_BStatic	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

void solveFriction	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveExtFriction	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveFriction_BStatic	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void solveFrictionBlock	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveFriction_BStaticBlock	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveExtFrictionBlock	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);


void solveContactCoulombBlock			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveExtContactCoulombBlock		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactCoulomb_BStaticBlock	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);

void solveContactCoulombConcludeBlock			(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveExtContactCoulombConcludeBlock		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactCoulomb_BStaticConcludeBlock	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);

void solveContactCoulombBlockWriteBack		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
													PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveExtContactCoulombBlockWriteBack		(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
													PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveContactCoulomb_BStaticBlockWriteBack	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void solveFrictionBlockWriteBack	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveFriction_BStaticBlockWriteBack	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveExtFrictionBlockWriteBack	(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

//Pre-block coulomb stuff...

void solveContactCoulombPreBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactCoulombPreBlock_Static(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactCoulombPreBlock_Conclude(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactCoulombPreBlock_ConcludeStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveContactCoulombPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveContactCoulombPreBlock_WriteBackStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);
void solveFrictionCoulombPreBlock(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);

void solveFrictionCoulombPreBlock_Static(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveFrictionCoulombPreBlock_Conclude(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);
void solveFrictionCoulombPreBlock_ConcludeStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache);

void solveFrictionCoulombPreBlock_WriteBack(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);

void solveFrictionCoulombPreBlock_WriteBackStatic(const PxcSolverConstraintDesc* PX_RESTRICT desc, const PxU32 constraintCount, PxcSolverContext& cache,
										PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs);


// could move this to PxPreprocessor.h but 
// no implementation available for MSVC
#if defined(PX_GNUC)
#define PX_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define PX_UNUSED_ATTRIBUTE 
#endif
 
#ifdef __SPU__ 
#define DYNAMIC_ARTICULATION_REGISTRATION(x) x
#else
#define DYNAMIC_ARTICULATION_REGISTRATION(x) 0
#endif

static SolveMethod gVTableSolve[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContact,												// PXS_SC_TYPE_RB_CONTACT
	solve1D,													// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContact),			// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1D),				// PXS_SC_TYPE_EXT_1D
	solveContact_BStatic,										// PXS_SC_TYPE_STATIC_CONTACT
	solveContact,												// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
};

static SolveMethod gVTableSolveCoulomb[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactCoulomb,										// PXS_SC_TYPE_RB_CONTACT
	solve1D,													// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactCoulomb),	// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1D),				// PXS_SC_TYPE_EXT_1D
	solveContactCoulomb_BStatic,								// PXS_SC_TYPE_STATIC_CONTACT
	solveContactCoulomb,										// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	NULL,														// PXS_SC_TYPE_BLOCK_RB_CONTACT
	NULL,														// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	NULL,														// PXS_SC_TYPE_BLOCK_1D,
	NULL,														// PXS_SC_TYPE_BLOCK_1D_STATIC,
	solveFriction,												// PXS_SC_TYPE_FRICTION_CONSTRAINT
	solveFriction_BStatic,										// PXS_SC_TYPE_STATIC_FRICTION_CONSTRAINT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtFriction),		// PXS_SC_TYPE_EXT_FRICTION_CONSTRAINT

};

static SolveMethod gVTableSolveConclude[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveConcludeContact,											// PXS_SC_TYPE_RB_CONTACT
	solveConclude1D,												// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveConcludeExtContact),		// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveConcludeExt1D),			// PXS_SC_TYPE_EXT_1D
	solveConcludeContact_BStatic,									// PXS_SC_TYPE_STATIC_CONTACT
	solveConcludeContact,											// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
};

static SolveMethod gVTableSolveConcludeCoulomb[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveConcludeContactCoulomb,											// PXS_SC_TYPE_RB_CONTACT
	solveConclude1D,														// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveConcludeExtContactCoulomb),		// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveConcludeExt1D),					// PXS_SC_TYPE_EXT_1D
	solveConcludeContactCoulomb_BStatic,									// PXS_SC_TYPE_STATIC_CONTACT
	solveConcludeContactCoulomb,											// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	NULL,																	// PXS_SC_TYPE_BLOCK_RB_CONTACT
	NULL,																	// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	NULL,																	// PXS_SC_TYPE_BLOCK_1D,
	solveFriction,															// PXS_SC_TYPE_FRICTION_CONSTRAINT
	solveFriction_BStatic,													// PXS_SC_TYPE_STATIC_FRICTION_CONSTRAINT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtFriction),					// PXS_SC_TYPE_EXT_FRICTION_CONSTRAINT

}; 

static SolveBlockMethod gVTableSolveBlock[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactBlock,														// PXS_SC_TYPE_RB_CONTACT
	solve1DBlock,															// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactBlock),				// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DBlock),						// PXS_SC_TYPE_EXT_1D
	solveContact_BStaticBlock,												// PXS_SC_TYPE_STATIC_CONTACT
	solveContactBlock,														// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactPreBlock,													// PXS_SC_TYPE_BLOCK_RB_CONTACT
	solveContactPreBlock_Static,											// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4_Block,															// PXS_SC_TYPE_BLOCK_1D,
};

static SolveBlockMethod gVTableSolveBlockCoulomb[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactCoulombBlock,												// PXS_SC_TYPE_RB_CONTACT
	solve1DBlock,															// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactCoulombBlock),			// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DBlock),						// PXS_SC_TYPE_EXT_1D
	solveContactCoulomb_BStaticBlock,										// PXS_SC_TYPE_STATIC_CONTACT
	solveContactCoulombBlock,												// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactCoulombPreBlock,											// PXS_SC_TYPE_BLOCK_RB_CONTACT
	solveContactCoulombPreBlock_Static,										// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4_Block,															// PXS_SC_TYPE_BLOCK_1D,
	solveFrictionBlock,														// PXS_SC_TYPE_FRICTION_CONSTRAINT
	solveFriction_BStaticBlock,												// PXS_SC_TYPE_STATIC_FRICTION_CONSTRAINT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtFrictionBlock),				// PXS_SC_TYPE_EXT_FRICTION_CONSTRAINT
	solveFrictionCoulombPreBlock,											// PXS_SC_TYPE_BLOCK_FRICTION					
	solveFrictionCoulombPreBlock_Static										// PXS_SC_TYPE_BLOCK_STATIC_FRICTION
};

static SolveWriteBackBlockMethod gVTableSolveWriteBackBlock[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactBlockWriteBack,												// PXS_SC_TYPE_RB_CONTACT
	solve1DBlockWriteBack,													// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactBlockWriteBack),		// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DBlockWriteBack),			// PXS_SC_TYPE_EXT_1D
	solveContact_BStaticBlockWriteBack,										// PXS_SC_TYPE_STATIC_CONTACT
	solveContactBlockWriteBack,												// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactPreBlock_WriteBack,											// PXS_SC_TYPE_BLOCK_RB_CONTACT
	solveContactPreBlock_WriteBackStatic,									// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4Block_WriteBack,												// PXS_SC_TYPE_BLOCK_1D,
};

static WriteBackBlockMethod gVTableWriteBackBlock[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	contactBlockWriteBack,												// PXS_SC_TYPE_RB_CONTACT
	writeBack1DBlock,													// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(extContactBlockWriteBack),		// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(ext1DBlockWriteBack),			// PXS_SC_TYPE_EXT_1D
	contactBlockWriteBack,										// PXS_SC_TYPE_STATIC_CONTACT
	contactBlockWriteBack,												// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	contactPreBlock_WriteBack,											// PXS_SC_TYPE_BLOCK_RB_CONTACT
	contactPreBlock_WriteBack,									// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	writeBack1D4Block,	
};

static SolveWriteBackBlockMethod gVTableSolveWriteBackBlockCoulomb[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactCoulombBlockWriteBack,												// PXS_SC_TYPE_RB_CONTACT
	solve1DBlockWriteBack,															// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactCoulombBlockWriteBack),		// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DBlockWriteBack),					// PXS_SC_TYPE_EXT_1D
	solveContactCoulomb_BStaticBlockWriteBack,										// PXS_SC_TYPE_STATIC_CONTACT
	solveContactCoulombBlockWriteBack,												// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactCoulombPreBlock_WriteBack,											// PXS_SC_TYPE_BLOCK_RB_CONTACT
	solveContactCoulombPreBlock_WriteBackStatic,									// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4Block_WriteBack,														// PXS_SC_TYPE_BLOCK_1D,
	solveFrictionBlockWriteBack,													// PXS_SC_TYPE_FRICTION_CONSTRAINT
	solveFriction_BStaticBlockWriteBack,											// PXS_SC_TYPE_STATIC_FRICTION_CONSTRAINT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtFrictionBlockWriteBack),				// PXS_SC_TYPE_EXT_FRICTION_CONSTRAINT
	solveFrictionCoulombPreBlock_WriteBack,											// PXS_SC_TYPE_BLOCK_FRICTION
	solveFrictionCoulombPreBlock_WriteBackStatic									// PXS_SC_TYPE_BLOCK_STATIC_FRICTION
};

static SolveBlockMethod gVTableSolveConcludeBlock[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactConcludeBlock,												// PXS_SC_TYPE_RB_CONTACT
	solve1DConcludeBlock,													// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactConcludeBlock),		// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DConcludeBlock),				// PXS_SC_TYPE_EXT_1D
	solveContact_BStaticConcludeBlock,										// PXS_SC_TYPE_STATIC_CONTACT
	solveContactConcludeBlock,												// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactPreBlock_Conclude,											// PXS_SC_TYPE_BLOCK_RB_CONTACT
	solveContactPreBlock_ConcludeStatic,									// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4Block_Conclude,													// PXS_SC_TYPE_BLOCK_1D,
};

static SolveBlockMethod gVTableSolveConcludeBlockCoulomb[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	solveContactCoulombConcludeBlock,												// PXS_SC_TYPE_RB_CONTACT
	solve1DConcludeBlock,															// PXS_SC_TYPE_RB_1D
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtContactCoulombConcludeBlock),			// PXS_SC_TYPE_EXT_CONTACT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExt1DConcludeBlock),						// PXS_SC_TYPE_EXT_1D
	solveContactCoulomb_BStaticConcludeBlock,										// PXS_SC_TYPE_STATIC_CONTACT
	solveContactCoulombConcludeBlock,												// PXS_SC_TYPE_NOFRICTION_RB_CONTACT
	solveContactCoulombPreBlock_Conclude,											// PXS_SC_TYPE_BLOCK_RB_CONTACT
	solveContactCoulombPreBlock_ConcludeStatic,										// PXS_SC_TYPE_BLOCK_STATIC_RB_CONTACT
	solve1D4Block_Conclude,															// PXS_SC_TYPE_BLOCK_1D,
	solveFrictionBlock,																// PXS_SC_TYPE_FRICTION_CONSTRAINT
	solveFriction_BStaticBlock,														// PXS_SC_TYPE_STATIC_FRICTION_CONSTRAINT
	DYNAMIC_ARTICULATION_REGISTRATION(solveExtFrictionBlock),						// PXS_SC_TYPE_EXT_FRICTION_CONSTRAINT
	solveFrictionCoulombPreBlock_Conclude,											// PXS_SC_TYPE_BLOCK_FRICTION
	solveFrictionCoulombPreBlock_ConcludeStatic										// PXS_SC_TYPE_BLOCK_STATIC_FRICTION
};

//----------------------------------

typedef void (*ConcludeMethod)(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

void conclude1D			(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);
void concludeContact	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

static ConcludeMethod gVTableConclude[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	concludeContact,		// PXS_SC_TYPE_RB_CONTACT
	conclude1D,				// PXS_SC_TYPE_RB_1D
	concludeContact,		// PXS_SC_TYPE_EXT_CONTACT
	conclude1D,				// PXS_SC_TYPE_EXT_1D
	concludeContact,		// PXS_SC_TYPE_INF_CONTACT
	concludeContact,		// PXS_SC_TYPE_STATIC_CONTACT
};

void concludeContactCoulomb (const PxcSolverConstraintDesc& desc, PxcSolverContext& cache);

static ConcludeMethod gVTableConcludeCoulomb[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	concludeContactCoulomb,		// PXS_SC_TYPE_RB_CONTACT
	conclude1D,					// PXS_SC_TYPE_RB_1D
	concludeContactCoulomb,		// PXS_SC_TYPE_EXT_CONTACT
	conclude1D,					// PXS_SC_TYPE_EXT_1D
	concludeContactCoulomb,		// PXS_SC_TYPE_INF_CONTACT
	concludeContactCoulomb,		// PXS_SC_TYPE_STATIC_CONTACT
};

//----------------------------------

void writeBack1D		(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBodyData& sbd0, PxcSolverBodyData& sbd1);

void writeBackContact	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBodyData& sbd0, PxcSolverBodyData& sbd1);

static WriteBackMethod gVTableWriteBack[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	writeBackContact,		// PXS_SC_TYPE_RB_CONTACT
	writeBack1D,			// PXS_SC_TYPE_RB_1D
	writeBackContact,		// PXS_SC_TYPE_EXT_CONTACT
	writeBack1D,			// PXS_SC_TYPE_EXT_1D
	writeBackContact,		// PXS_SC_TYPE_INF_CONTACT
	writeBackContact,		// PXS_SC_TYPE_STATIC_CONTACT
};

void writeBackContactCoulomb	(const PxcSolverConstraintDesc& desc, PxcSolverContext& cache, PxcSolverBodyData& sbd0, PxcSolverBodyData& sbd1);

static WriteBackMethod gVTableWriteBackCoulomb[] PX_UNUSED_ATTRIBUTE = 
{
	0,
	writeBackContactCoulomb,		// PXS_SC_TYPE_RB_CONTACT
	writeBack1D,					// PXS_SC_TYPE_RB_1D
	writeBackContactCoulomb,		// PXS_SC_TYPE_EXT_CONTACT
	writeBack1D,					// PXS_SC_TYPE_EXT_1D
	writeBackContactCoulomb,		// PXS_SC_TYPE_INF_CONTACT
	writeBackContactCoulomb,		// PXS_SC_TYPE_STATIC_CONTACT
};


void writeBack1D4(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& cache,
							 const PxcSolverBodyData** PX_RESTRICT bd0, const PxcSolverBodyData** PX_RESTRICT bd1);

void writeBackContact4_Block(const PxcSolverConstraintDesc* PX_RESTRICT desc, PxcSolverContext& cache,
							 const PxcSolverBodyData** PX_RESTRICT bd0, const PxcSolverBodyData** PX_RESTRICT bd1);

void  writeBackContactCoulomb4(const PxcSolverConstraintDesc* desc, PxcSolverContext& cache,
					  const PxcSolverBodyData** PX_RESTRICT bd0, const PxcSolverBodyData** PX_RESTRICT bd1);


class PxsSolverCoreGeneral : public PxsSolverCore
{
public:
	static PxsSolverCoreGeneral* create();

	// Implements PxsSolverCore
	virtual void destroyV();

	//! Solve
	virtual void solveV
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
 		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList, const PxU32 contactConstraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintListSize,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray) const;

	virtual void solveVCoulomb
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
 		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList, const PxU32 contactConstraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintListSize,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray) const;


	virtual void solveVParallelAndWriteBack
		(const PxReal dt, const PxU32 _positionIterations, const PxU32 _velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 _atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList, const PxU32 contactConstraintListSize,
		 PxI32* pConstraintIndex, PxI32* pConstraintIndex2, PxI32* pAtomListIndex, PxI32* pAtomListIndex2,
		 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs,
		 const Ps::Array<PxsConstraintBatchHeader>& constraintBatchHeaders, const Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatches,
		 const Ps::Array<PxU32>& headersPerPartition, Cm::SpatialVector* PX_RESTRICT motionVelocityArray, PxI32& normalIterations,
		 const PxU32 batchSize) const;

	virtual void solveVCoulombParallelAndWriteBack
		(const PxReal dt, const PxU32 _positionIterations, const PxU32 _velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 _atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList, const PxU32 contactConstraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintConstraintListSize,
		 PxI32* pConstraintIndex, PxI32* pConstraintIndex2, PxI32* pFrictionConstraintIndex, PxI32* pAtomListIndex, PxI32* pAtomListIndex2,
		 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxI32* outThresholdPairs,
		 Ps::Array<PxsConstraintBatchHeader>& constraintBatchHeaders, Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatches,
		 Ps::Array<PxU32>& headersPerPartition, Ps::Array<PxU32>& frictionHeadersPerPartition, Cm::SpatialVector* PX_RESTRICT motionVelocityArray, 
		 PxI32& normalIterations, PxI32& frictionIterations, const PxU32 batchSize) const;

	virtual void solveV_Blocks
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList,const PxU32 contactConstraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintConstraintListSize,
		 Ps::Array<PxsConstraintBatchHeader>& contactConstraintBatchHeaders,  Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatchHeaders,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray, PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs) const;

	virtual void solveVCoulomb_Blocks
		(const PxReal dt, const PxU32 positionIterations, const PxU32 velocityIterations, 
		 PxcSolverBody* PX_RESTRICT atomListStart, PxcSolverBodyData* PX_RESTRICT atomDataList, const PxU32 solverBodyOffset, const PxU32 atomListSize,
		 PxcArticulationSolverDesc* PX_RESTRICT articulationListStart, const PxU32 articulationListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList, const PxU32 contactConstraintListSize,
		 PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList, const PxU32 frictionConstraintListSize,
		 Ps::Array<PxsConstraintBatchHeader>& contactConstraintBatchHeaders, Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatchHeaders,
		 Cm::SpatialVector* PX_RESTRICT motionVelocityArray,PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs) const;

	//! Write back - threshold stream, friction byte, write back forces.
	virtual void writeBackV
		(const PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize,
		 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs,
		 PxcSolverBodyData* atomListData, WriteBackMethod writeBackTable[]) const;

	virtual void writeBackV
		(const PxcSolverConstraintDesc* PX_RESTRICT constraintList, const PxU32 constraintListSize, PxsConstraintBatchHeader* contactConstraintBatches, const PxU32 numBatches,
		 PxcThresholdStreamElement* PX_RESTRICT thresholdStream, const PxU32 thresholdStreamLength, PxU32& outThresholdPairs,
		 PxcSolverBodyData* atomListData, WriteBackBlockMethod writeBackTable[]) const;

private:

	//~Implements PxsSolverCore
};

#define SOLVEV_METHOD_ARGS													\
	PxsSolverCore*	solverCore,												\
	const PxReal dt,														\
	const PxU32 positionIterations,											\
	const PxU32 velocityIterations,											\
	PxcSolverBody* PX_RESTRICT atomListStart,								\
	PxcSolverBodyData* PX_RESTRICT atomDataList,							\
	const PxU32 solverBodyOffset,											\
	const PxU32 atomListSize,												\
	PxcArticulationSolverDesc* PX_RESTRICT articulationListStart,			\
	const PxU32 articulationListSize,										\
	PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList,				\
	const PxU32 contactConstraintListSize,									\
	PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList,			\
	const PxU32 frictionConstraintListSize,									\
	Cm::SpatialVector* PX_RESTRICT motionVelocityArray

typedef	void (*PxsSolveVMethod)(SOLVEV_METHOD_ARGS);
extern PxsSolveVMethod solveV[3];

void solveVDefaultFriction(SOLVEV_METHOD_ARGS);
void solveVCoulombFriction(SOLVEV_METHOD_ARGS);

#define SOLVEV_BLOCK_METHOD_ARGS											\
	PxsSolverCore*	solverCore,												\
	const PxReal dt,														\
	const PxU32 positionIterations,											\
	const PxU32 velocityIterations,											\
	PxcSolverBody* PX_RESTRICT atomListStart,								\
	PxcSolverBodyData* PX_RESTRICT atomDataList,							\
	const PxU32 solverBodyOffset,											\
	const PxU32 atomListSize,												\
	PxcArticulationSolverDesc* PX_RESTRICT articulationListStart,			\
	const PxU32 articulationListSize,										\
	PxcSolverConstraintDesc* PX_RESTRICT contactConstraintList,				\
	const PxU32 contactConstraintListSize,									\
	PxcSolverConstraintDesc* PX_RESTRICT frictionConstraintList,			\
	const PxU32 frictionConstraintConstraintListSize,						\
	Ps::Array<PxsConstraintBatchHeader>& contactConstraintBatchHeaders,		\
	Ps::Array<PxsConstraintBatchHeader>& frictionConstraintBatchHeaders,	\
	Cm::SpatialVector* PX_RESTRICT motionVelocityArray,						\
	PxcThresholdStreamElement* PX_RESTRICT thresholdStream,					\
	const PxU32 thresholdStreamLength,										\
	PxU32& outThresholdPairs

typedef	void (*PxsSolveVBlockMethod)(SOLVEV_BLOCK_METHOD_ARGS);
extern PxsSolveVBlockMethod solveVBlock[3];

void solveVBlockDefaultFriction(SOLVEV_BLOCK_METHOD_ARGS);
void solveVBlockCoulombFriction(SOLVEV_BLOCK_METHOD_ARGS);

void PxsSolverCoreRegisterArticulationFns();

void PxsSolverCoreRegisterArticulationFnsCoulomb();


}

#endif
