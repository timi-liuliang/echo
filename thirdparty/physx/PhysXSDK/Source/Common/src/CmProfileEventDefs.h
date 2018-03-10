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

#define THERE_IS_NO_INCLUDE_GUARD_HERE_FOR_A_REASON

// This file should only be included by CmEventProfiler.h. It is included there multiple times

#define PX_PROFILE_TASK_PRIORITY Detail
// simulation task graph - mostly coarse function-level profiling seems preferable, since
// the aggregation of functionality into tasks isn't very insightful unless you're trying
// to debug scheduling issues

#ifndef PX_PROFILE_EVENT_NO_COMMA 
#define PX_PROFILE_EVENT_NO_COMMA(subsys, name, priority) PX_PROFILE_EVENT(subsys, name, priority)
#endif

#define PX_PROFILE_BASIC_PRIORITY Coarse
PX_PROFILE_BEGIN_SUBSYSTEM(Basic)
	PX_PROFILE_EVENT(Basic, collision, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, broadPhase, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, narrowPhase, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, rigidBodySolver, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, islandGen, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, dynamics, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, pvdFrameStart, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, simulate, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, checkResults, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, checkCollision, PX_PROFILE_BASIC_PRIORITY)
	PX_PROFILE_EVENT(Basic, fetchResults, PX_PROFILE_BASIC_PRIORITY)
PX_PROFILE_END_SUBSYSTEM(Basic)
	
PX_PROFILE_BEGIN_SUBSYSTEM(API)

PX_PROFILE_EVENT_DETAIL(API, flushSimulation, Medium)
PX_PROFILE_EVENT_DETAIL(API, flushQueryUpdates, Medium)

PX_PROFILE_EVENT_DETAIL(API, addActor, Medium)
PX_PROFILE_EVENT_DETAIL(API, addActors, Medium)
PX_PROFILE_EVENT_DETAIL(API, removeActor, Medium)
PX_PROFILE_EVENT_DETAIL(API, removeActors, Medium)
PX_PROFILE_EVENT_DETAIL(API, addAggregate, Medium)
PX_PROFILE_EVENT_DETAIL(API, removeAggregate, Medium)
PX_PROFILE_EVENT_DETAIL(API, addArticulation, Medium)
PX_PROFILE_EVENT_DETAIL(API, removeArticulation, Medium)
PX_PROFILE_EVENT_DETAIL(API, addCollection, Medium)

PX_PROFILE_EVENT_DETAIL(API, addParticleSystem, Medium)
PX_PROFILE_EVENT_DETAIL(API, removeParticleSystem, Medium)
PX_PROFILE_EVENT_DETAIL(API, addParticleFluid, Medium)
PX_PROFILE_EVENT_DETAIL(API, removeParticleFluid, Medium)

PX_PROFILE_EVENT_DETAIL(API, addCloth, Medium)
PX_PROFILE_EVENT_DETAIL(API, removeCloth, Medium)

PX_PROFILE_EVENT_DETAIL(API, createBatchQuery, Medium)
PX_PROFILE_EVENT_DETAIL(API, releaseBatchQuery, Medium)

PX_PROFILE_EVENT_DETAIL(API, forceDynamicTreeRebuild, Medium)
PX_PROFILE_EVENT_DETAIL(API, shiftOrigin, Medium)

#define PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY Never
PX_PROFILE_EVENT_DETAIL(API, addActorToSim, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_EVENT_DETAIL(API, removeActorFromSim, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_EVENT_DETAIL(API, addShapesToSim, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_EVENT_DETAIL(API, removeShapesFromSim, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_EVENT_DETAIL(API, addShapesToSQ, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_EVENT_DETAIL(API, removeShapesFromSQ, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_EVENT_DETAIL(API, simAddShapeToBroadPhase, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_EVENT_DETAIL(API, findAndReplaceWithLast, PX_PROFILE_ADDREMOVE_DETAIL_PRIORITY)
PX_PROFILE_END_SUBSYSTEM(API)

PX_PROFILE_BEGIN_SUBSYSTEM(Sim)

// scene initialization
PX_PROFILE_EVENT(Sim, updateDirtyShaders, Coarse)
PX_PROFILE_EVENT(Sim, taskFrameworkSetup, Coarse)
PX_PROFILE_EVENT(Sim, resetDependencies, Coarse)
PX_PROFILE_EVENT(Sim, solveQueueTasks, Coarse)
PX_PROFILE_EVENT(Sim, collideQueueTasks, Coarse)
PX_PROFILE_EVENT(Sim, stepSetupSimulate, Coarse)
PX_PROFILE_EVENT(Sim, stepSetupSolve, Coarse)
PX_PROFILE_EVENT(Sim, stepSetupCollide, Coarse)
PX_PROFILE_EVENT_DETAIL(Sim, projectionTreeUpdates, Medium)

// handle broad phase outputs
PX_PROFILE_EVENT(Sim, processNewOverlaps, Coarse)
PX_PROFILE_EVENT(Sim, processLostOverlaps, Coarse)

// handle narrow phase
PX_PROFILE_EVENT(Sim, processTriggers, Coarse)
PX_PROFILE_EVENT(Sim, queueNarrowPhase, Coarse)
PX_PROFILE_EVENT(Sim, narrowPhase, Coarse)
PX_PROFILE_EVENT(Sim, finishModifiablePairs, Coarse)
PX_PROFILE_EVENT(Sim, narrowPhaseMerge, Coarse)

// island manager
PX_PROFILE_EVENT(Sim, preIslandGen, Coarse)
PX_PROFILE_EVENT(Sim, updateIslands, Coarse)
PX_PROFILE_EVENT(Sim, preIslandGenSecondPass, Coarse)
PX_PROFILE_EVENT(Sim, updateIslandsSecondPass, Coarse)
PX_PROFILE_EVENT(Sim, postIslandGenSecondPass, Coarse)

// RB Dynamics solver
PX_PROFILE_EVENT(Sim, updateForces, Coarse)

// CCD
PX_PROFILE_EVENT(Sim, updateCCDMultiPass, Coarse)
PX_PROFILE_EVENT(Sim, ccdBroadPhase, Coarse)
PX_PROFILE_EVENT(Sim, updateCCDSinglePass, Coarse)
PX_PROFILE_EVENT(Sim, ccdBroadPhaseComplete, Coarse)
PX_PROFILE_EVENT_DETAIL(Sim, projectContacts, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, ccdSweep, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, ccdSweepMerge, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, ccdPair, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, ccdIsland, Medium)

// reports & finalization
PX_PROFILE_EVENT_DETAIL(Sim, getSimEvents, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, syncBodiesAfterSolver, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, syncBodiesAfterIntegration, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, projectConstraints, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, checkConstraintBreakage, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, visualizeStartStep, Medium)
PX_PROFILE_EVENT_DETAIL(Sim, visualizeEndStep, Medium)

PX_PROFILE_EVENT(Sim, fireCustomFilteringCallbacks, Coarse)
PX_PROFILE_EVENT(Sim, fireCallBacksPreSync, Coarse)
PX_PROFILE_EVENT(Sim, updatePruningTrees, Coarse)
PX_PROFILE_EVENT(Sim, fireCallBacksPostSync, Coarse)
PX_PROFILE_EVENT(Sim, syncState, Coarse)
PX_PROFILE_EVENT(Sim, buildActiveTransforms, Coarse)
PX_PROFILE_EVENT(Sim, fetchResults, Coarse)

PX_PROFILE_EVENT(Sim, sceneFinalization, Coarse)
PX_PROFILE_END_SUBSYSTEM(Sim)

PX_PROFILE_BEGIN_SUBSYSTEM(ParticleSim)
PX_PROFILE_EVENT(ParticleSim, startStep, Coarse)
PX_PROFILE_EVENT(ParticleSim, endStep, Coarse)
PX_PROFILE_EVENT(ParticleSim, shapesUpdateProcessing, Coarse)
PX_PROFILE_EVENT(ParticleSim, updateCollision, Coarse)
PX_PROFILE_END_SUBSYSTEM(ParticleSim)

PX_PROFILE_BEGIN_SUBSYSTEM(Dynamics)
PX_PROFILE_EVENT(Dynamics, solverQueueTasks, Coarse)
PX_PROFILE_EVENT(Dynamics, solveGroup, Coarse)
PX_PROFILE_EVENT(Dynamics, solverMergeResults, Coarse)
PX_PROFILE_EVENT(Dynamics, solver, Coarse)
PX_PROFILE_EVENT_DETAIL(Dynamics, createFinalize, Detail)
PX_PROFILE_EVENT_DETAIL(Dynamics, setupFinalize, Detail)
PX_PROFILE_EVENT_DETAIL(Dynamics, getFrictionPatch, Detail)
PX_PROFILE_EVENT_DETAIL(Dynamics, createContactPatches, Detail)
PX_PROFILE_EVENT_DETAIL(Dynamics, correlatePatches, Detail)
PX_PROFILE_EVENT_DETAIL(Dynamics, growPatches, Detail)
PX_PROFILE_EVENT_DETAIL(Dynamics, parallelSolve, Detail)
PX_PROFILE_EVENT_DETAIL(Dynamics, updateKinematics, Medium)
PX_PROFILE_EVENT_DETAIL(Dynamics, updateVelocities, Medium)
PX_PROFILE_EVENT_DETAIL(Dynamics, updatePositions, Medium)
PX_PROFILE_END_SUBSYSTEM(Dynamics)

#define PX_PROFILE_SCENEQUERY_PRIORITY Detail
PX_PROFILE_BEGIN_SUBSYSTEM(SceneQuery)
PX_PROFILE_EVENT_DETAIL(SceneQuery, raycastAny, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, raycastSingle, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, raycastMultiple, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, overlapMultiple, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, sweepAny, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, sweepAnyList, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, sweepSingle, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, sweepSingleList, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, sweepMultiple, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, sweepMultipleList, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(SceneQuery, flushUpdates, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_END_SUBSYSTEM(SceneQuery)

#define PX_PROFILE_SCENEQUERY_PRIORITY Detail
PX_PROFILE_BEGIN_SUBSYSTEM(BatchedSceneQuery)
PX_PROFILE_EVENT_DETAIL(BatchedSceneQuery, execute, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BatchedSceneQuery, raycastAny, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BatchedSceneQuery, raycastSingle, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BatchedSceneQuery, raycastMultiple, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BatchedSceneQuery, overlapMultiple, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BatchedSceneQuery, sweepSingle, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BatchedSceneQuery, sweepMultiple, PX_PROFILE_SCENEQUERY_PRIORITY)
PX_PROFILE_END_SUBSYSTEM(BatchedSceneQuery)


#define PX_PROFILE_ARTICULATION_PRIORITY Medium
PX_PROFILE_BEGIN_SUBSYSTEM(Articulations)
PX_PROFILE_EVENT_DETAIL(Articulations, prepareDataBlock, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, setupProject, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, prepareFsData, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, setupDrives, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, jointLoads, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, propagateDrivenInertia, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, computeJointDrives, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, applyJointDrives, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, applyExternalImpulses, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, setupConstraints, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_EVENT_DETAIL(Articulations, integrate, PX_PROFILE_ARTICULATION_PRIORITY)
PX_PROFILE_END_SUBSYSTEM(Articulations)

// broad phase 
#define PX_PROFILE_BROADPHASE_PRIORITY Medium
PX_PROFILE_BEGIN_SUBSYSTEM(BroadPhase)
PX_PROFILE_EVENT_DETAIL(BroadPhase, computeAABBUpdateLists, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, updateAggLocalSpaceBounds, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, updateShapeAABBs, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, updateAggAABBs, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, updateBroadphase, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, SapUpdate, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, SapPostUpdate, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, SapUpdateSPU, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, SapPostUpdateSPU, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, MBPUpdate, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, MBPPostUpdate, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, MBPUpdateSPU, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, MBPPostUpdateSPU, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, processBPResults, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, updateLazyAggShapeAABBs, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, computeAggAggShapePairs, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_EVENT_DETAIL(BroadPhase, finish, PX_PROFILE_BROADPHASE_PRIORITY)
PX_PROFILE_END_SUBSYSTEM(BroadPhase)

#define PX_PROFILE_ISLANDGEN_PRIORITY Medium
PX_PROFILE_BEGIN_SUBSYSTEM(IslandGen)
PX_PROFILE_EVENT_DETAIL(IslandGen, resizeArrays, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, resizeForKinematics, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, cleanupEdgeEvents, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, emptyIslands, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, joinedEdges, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, createdNodes, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, duplicateKinematicNodes, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, brokenEdgeIslands, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, processSleepingIslands, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, freeBuffers, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_EVENT_DETAIL(IslandGen, deletedNodesEdges, PX_PROFILE_ISLANDGEN_PRIORITY)
PX_PROFILE_END_SUBSYSTEM(IslandGen)

PX_PROFILE_BEGIN_SUBSYSTEM(PVD)
PX_PROFILE_EVENT_DETAIL(PVD, updateContacts, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, updateDynamicBodies, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, updateJoints, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, updateCloths, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, updateSleeping, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, updatePariclesAndFluids, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, sceneUpdate, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, createPVDInstance, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, releasePVDInstance, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, updatePVDProperties, Medium)
PX_PROFILE_EVENT_DETAIL(PVD, sendPVDArrays, Medium)
PX_PROFILE_EVENT_NO_COMMA(PVD, originShift, Medium)
PX_PROFILE_END_SUBSYSTEM(PVD)


#undef PX_PROFILE_EVENT_NO_COMMA 

#undef THERE_IS_NO_INCLUDE_GUARD_HERE_FOR_A_REASON
