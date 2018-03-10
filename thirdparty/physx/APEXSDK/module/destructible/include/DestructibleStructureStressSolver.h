/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DESTRUCTIBLE_STRUCTURE_STRESS_SOLVER_H
#define DESTRUCTIBLE_STRUCTURE_STRESS_SOLVER_H

#include "PsUserAllocated.h"
#include "PsShare.h"
#include "PsArray.h"


#include "DestructibleScene.h"
#include "NxApexScene.h"
#include "NxApexDefs.h"
#include <PxPhysics.h>
#include "cooking/PxCooking.h"
#if NX_SDK_VERSION_MAJOR == 3
#include <PxScene.h>
#include "extensions/PxExtensionsAPI.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "extensions/PxDefaultSimulationFilterShader.h"
#include <PxFiltering.h>
#include <PxMaterial.h>
#include "physxprofilesdk/PxProfileZoneManager.h"
#include <PxFixedJoint.h> 
#include <PxConstraint.h>
#endif

namespace physx
{
namespace apex
{
namespace destructible
{

class DestructibleStructure;
class DestructibleActor;
struct ActorChunkPair;
struct FractureEvent;

// a helper class for managing states using an array
template<class T, typename F, unsigned int N>
class ArraySet
{
public:
	explicit ArraySet(T v)
	{
		PX_COMPILE_TIME_ASSERT(N > 0);
		typedef char PxCompileTimeAssert_Dummy2[(N * sizeof(T) == sizeof(*this)) ? 1 : -1];
		::memset(static_cast<void*>(&t[0]), v, N * sizeof(T));
	}
	~ArraySet()
	{
	}
	void	set(unsigned int i, F f)
	{
		PX_ASSERT(i < N);
		PX_ASSERT(!isSet(i, f));
		t[i] |= f;
	}
	void	unset(unsigned int i, F f)
	{
		PX_ASSERT(i < N);
		PX_ASSERT(isSet(i, f));
		t[i] &= ~f;
	}
	bool	isSet(unsigned int i, F f) const
	{
		PX_ASSERT(i < N);
		return (0 != (f & t[i]));
	}
	void	reset(T v)
	{
		::memset(static_cast<void*>(&t[0]), v, N * sizeof(T));
	}
	const T		(&get() const) [N]
	{
		return t;
	}
	const T *	getPtr() const
	{
		return &t[0];
	}
private:
	T	t[N];
}; // class ArraySet

// Int-To-Type idiom
template<unsigned int I>
struct Int2Type
{
	enum
	{
		Value = I,
	};
}; // struct Int2Type

// a type container used to identify the different ways of evaluating stresses on the structure
struct StressEvaluationType
{
	enum Enum
	{
		EvaluateByCount = 0,
		EvaluateByMoment,
	};
}; // struct StressEvaluationType

// a helper data structure used for recording bit values
struct StructureScratchBuffer
{
public:
	StructureScratchBuffer(physx::PxU8 * const bufferStart, physx::PxU32 bufferCount);
	~StructureScratchBuffer();

	bool isOccupied(physx::PxU32 index) const;
	void setOccupied(physx::PxU32 index);

private:
	StructureScratchBuffer();
	StructureScratchBuffer& operator=(const StructureScratchBuffer&);

	enum ByteState
	{
		ByteEmpty	= 0x00,
		ByteExist	= 0xFF,
	};

	physx::PxU8 * const	bufferStart;
	const physx::PxU32	bufferCount;
}; // struct StructureScratchBuffer

// a data structure used to collect and hold potential new island data
struct PeninsulaData
{
public:
	PeninsulaData();
	PeninsulaData(physx::PxU8 peninsulaState, physx::PxU32 chunkCount);
	PeninsulaData(physx::PxU8 peninsulaState, physx::PxU32 chunkCount, physx::PxF32 aggregateMass, physx::PxVec3 geometricCenter);
	~PeninsulaData();
	enum PeninsulaFlag
	{
		PeninsulaEncounteredExternalSupport	= (1 << 0),
		PeninsulaEncounteredAnotherWeakLink	= (1 << 1),
		PeninsulaInvalidMaxFlag				= (1 << 2),
	};

	bool						isValid() const;
	void						resetData();
	void						setFlag(PeninsulaData::PeninsulaFlag peninsulaFlag);
	void						setLinkedChunkData(const ActorChunkPair & actorChunkPair, Int2Type<StressEvaluationType::EvaluateByCount>);
	void						setLinkedChunkData(const ActorChunkPair & actorChunkPair, Int2Type<StressEvaluationType::EvaluateByMoment>);
	void						setRootLinkedChunkIndex(physx::PxU32 linkedChunkIndex);
	void						assimilate(const PeninsulaData & that, Int2Type<StressEvaluationType::EvaluateByCount>);
	void						assimilate(const PeninsulaData & that, Int2Type<StressEvaluationType::EvaluateByMoment>);
	bool						hasFlag(PeninsulaData::PeninsulaFlag peninsulaFlag) const;
	physx::PxU32				getDataChunkCount() const;
	physx::PxF32				getDataAggregateMass() const;
	const physx::PxVec3	&		getDataGeometricCenter() const;
	physx::PxU32				getRootLinkedChunkIndex() const;
private:
	const PeninsulaData			operator+	(const PeninsulaData &);
	PeninsulaData &				operator+=	(const PeninsulaData &);

	physx::PxU32				rootLinkedChunkIndex;
	physx::PxU8					peninsulaState;
	physx::PxU32				chunkCount;
	physx::PxF32				aggregateMass;
	physx::PxVec3				geometricCenter;
}; // struct PeninsulaData

// a data structure used to hold minimally required fracture data
struct FractureEventProxy : public physx::UserAllocated
{
public:
	physx::PxU32				rootLinkedChunkIndex;
	physx::PxU32				chunkIndexInAsset;
	physx::PxU32				destructibleId;
	physx::PxU32				fractureEventProxyFlags;
	physx::PxVec3				rootLinkedChunkPosition;
	physx::PxVec3				impulse;
}; // struct FractureEventProxy

// a data structure with a 1-N convertible relationship with fracture event(s)
struct SnapEvent : public physx::UserAllocated
{
public:
	static SnapEvent*			instantiate(FractureEventProxy * fractureEventProxyBufferStart, physx::PxU32 fractureEventProxyBufferCount, physx::PxF32 tickSecondsToSnap);

	void						onDestroy();
	bool						isRipeAfterUpdatingTime(physx::PxF32 deltaTime);
	bool						isExpired() const;
	physx::PxU32				getFractureEventProxyCount() const;
	const FractureEventProxy&	getFractureEventProxy(physx::PxU32 index) const;

private:
	SnapEvent(FractureEventProxy* fractureEventProxyBufferStart, physx::PxU32 fractureEventProxyBufferCount, physx::PxF32 tickSecondsToSnap);
	SnapEvent& operator=(const SnapEvent&);

	~SnapEvent();
	SnapEvent();

	FractureEventProxy*			fractureEventProxyBufferStart;
	const physx::PxU32			fractureEventProxyBufferCount;
	physx::PxF32				tickSecondsRemainingToSnap;
}; // struct SnapEvent

// a solver with a 1-1 or 0-1 relationship with a destructible structure
class DestructibleStructureStressSolver : public physx::UserAllocated
{
public:
#if NX_SDK_VERSION_MAJOR == 3
	explicit DestructibleStructureStressSolver(const DestructibleStructure & bindedStructureAlias, PxF32 strength);
#elif NX_SDK_VERSION_MAJOR == 2
	explicit DestructibleStructureStressSolver(const DestructibleStructure & bindedStructureAlias);
#endif
	~DestructibleStructureStressSolver();

	void							onTick(physx::PxF32 deltaTime);
	void							onUpdate(physx::PxU32 linkedChunkIndex);
	void							onResolve();
private:
	DestructibleStructureStressSolver();
	DestructibleStructureStressSolver(const DestructibleStructureStressSolver &);
	DestructibleStructureStressSolver & operator= (const DestructibleStructureStressSolver &);
	enum StructureLinkFlag
	{
		LinkedChunkExist			= (1 << 0),
		LinkedChunkBroken			= (1 << 1),
		LinkedChunkStrained			= (1 << 2),
		LinkedChunkInvalidMaxFlag	= (1 << 3),
	};
	enum LinkTestParameters
	{
		LowerLimitActivationCount	= 2,
		UpperLimitActivationCount	= 3,
	};
	enum LinkTraverseParameters
	{
		PathTraversalCount			= 2,
	};
	static const StressEvaluationType::Enum StressEvaluationEnum = StressEvaluationType::EvaluateByMoment;

	void							processLinkedChunkIndicesForEvaluation(physx::Array<physx::PxU32> & linkedChunkIndicesForEvaluation);
	void							evaluateForPotentialIslands(const physx::Array<physx::PxU32> & linkedChunkIndicesForEvaluation);
	bool							passLinkCountTest(physx::PxU32 linkedChunkIndex, physx::Array<physx::PxU32> & unbrokenAdjacentLinkedChunkIndices) const;
	bool							passLinkAdjacencyTest(const physx::Array<physx::PxU32> & unbrokenAdjacentLinkedChunkIndices, physx::PxU32 (&linkedChunkIndicesForTraversal)[2]) const;
	void							evaluateForPotentialPeninsulas(physx::PxU32 rootLinkedChunkIndex, const physx::PxU32 (&linkedChunkIndicesForTraversal)[2]);
	PeninsulaData					traverseLink(physx::PxU32 linkedChunkIndex, StructureScratchBuffer & traverseRecord) const;
	void							evaluatePeninsulas(const PeninsulaData (&peninsulasForEvaluation)[2]);
	SnapEvent *						interpret(const PeninsulaData & peninsulaData, Int2Type<StressEvaluationType::EvaluateByCount>) const;
	SnapEvent *						interpret(const PeninsulaData & peninsulaData, Int2Type<StressEvaluationType::EvaluateByMoment>) const;
	const FractureEvent &			interpret(const FractureEventProxy & fractureEventProxy) const;

	bool							isLinkedChunkBroken(physx::PxU32 linkedChunkIndex) const;
	void							setLinkedChunkBroken(physx::PxU32 linkedChunkIndex);
	bool							isLinkedChunkStrained(physx::PxU32 linkedChunkIndex) const;
	void							setLinkedChunkStrained(physx::PxU32 linkedChunkIndex);
	void							getUnbrokenAdjacentLinkedChunkIndices(physx::PxU32 linkedChunkIndex, physx::Array<physx::PxU32> & unbrokenAdjacentLinkedChunkIndices) const;
	bool							areUnbrokenLinkedChunksAdjacent(physx::PxU32 linkedChunkIndex, physx::PxU32 linkedChunkIndexSubject) const;
	ActorChunkPair					getActorChunkPair(physx::PxU32 chunkIndexInStructure) const;
	StructureScratchBuffer			acquireScratchBuffer() const;
	void							relinquishScratchBuffer() const;
	DestructibleStructure &			getBindedStructureMutable();
	bool							assertLinkedChunkIndexOk(physx::PxU32 linkedChunkIndex) const;
	bool							assertLinkedChunkIndicesForEvaluationOk(const physx::Array<physx::PxU32> & linkedChunkIndicesForEvaluation) const;

	const DestructibleStructure &	bindedStructureAlias;
	physx::PxU32					structureLinkCount;
	physx::PxU8 *					structureLinkCondition;
	physx::PxU8 *					scratchBuffer;
	mutable bool					scratchBufferLocked;
	physx::PxF32					userTimeDelay;
	physx::PxF32					userMassThreshold;
	physx::Array<physx::PxU32>		recentlyBrokenLinkedChunkIndices;
	physx::Array<physx::PxU32>		strainedLinkedChunkIndices;
	physx::Array<SnapEvent*>		snapEventContainer;
#if NX_SDK_VERSION_MAJOR == 3
	//Shadow scene to simulate a set of linked rigidbodies
	public :
	struct LinkedJoint 
	{
		PxU32						actor1Index;
		PxU32						actor2Index;
		PxFixedJoint *				joint;
		bool						isDestroyed;
	};

	struct ShadowActor
	{
		physx::PxRigidDynamic*		actor;
		PxReal						currentForce;
		PxReal						maxInitialForce;
		bool						isDestroyed;
	};
private: 
	//The basic assumption is that the initialize structure is valid, so we should know the maximum force for each link through initializing, and use it to calculate the threshold.
	//So initializingMaxForce is to judge if it is initializing or something breaks the initializing. 
	bool									initializingMaxForce;

	// The isNoChunkWarmingUpdate is the variable that fewer chunks updating during the warming, in this case, just quit warming. 
	bool									isNoChunkWarmingUpdate;

	// The isSimulating symbol indicates if the shadowScene is simulating.
	bool									isSimulating;

	physx::PxU32							continuousNoUpdationCount;
	physx::Array<ShadowActor>				shadowActors;

	//Hash the global chunk index to the support chunk's index
	physx::Array<PxU32>						globalHashedtoSupportChunks;

	//initialWarmingKillList caches the chunks breaking during the warming stage, after warming is done, all chunks in this list will be removed from shadow scene.
	physx::Array<PxU32>						initialWarmingKillList;

	//islandKillList caches the chunks removed by the reason of new island generation, all supported chunks will be removed from the shadow scene at once. 
	physx::Array<PxU32>						islandKillList;

	//forces[i] are the current force for supported chunk i in the scene.  
	physx::Array<PxReal>					forces; 
	
	//linked joints are constructed as a network for chunk breaking prediction.
	physx::Array<LinkedJoint>				linkedJoint;

	//The SDK is usde to create the shadow scene
	physx::PxPhysics *						newSceneSDK;	

	//The shadow scene is used to construct linked joints which are used for chunk breaking prediction
	physx::PxScene*							shadowScene;

	//Size of the supported chunk in the real scene.
	physx::PxU32							supportChunkSize;

	//The supportStrength is used for the stress solver.  As this value is increased it takes more force to break apart the chunks under the effect of stress solver.
	physx::PxReal							supportStrength;

	//currentWarmingFrame increases by 1 in the warming stage. 
	physx::PxU32							currentWarmingFrame;

	//initialWarmFrames is used with currentWarmingFrame. If  currentWarmingFrame is larger than initialWarmFrames, the warming will stop.
	physx::PxU32							initialWarmFrames;

	//If no fracturing during one activCycle, just turn the mode to sleep.
	physx::PxU32							activeCycle;		

	//It is used with the activeCycle, 
	physx::PxU32							currentActiveSimulationFrame;

	//If any chunks break isActiveWakedUp is set to true indicates that the stress solver will wake up.
	bool									isActiveWakedUp;

	bool									isSleep;
public :
	// The isPhysxBasedSim is the variable that uses by the customer if they want to choose the physics based simulation mode. 
	bool									isPhysxBasedSim;

	// The isCustomEnablesSimulating symbol is used by the user if the they would like to stop the shadowScene simulating after creating the shadow scene. 
	bool									isCustomEnablesSimulating;

	//Functions: 
	PxU32 							predictBreakChunk(PxReal deltaTime, PxReal supportStrength);
	//create the shadow scene 
	void							createShadowScene(PxU32 initialWarmFrames,PxU32 actCycle, PxReal supStrength);
	//calculate current maximum force
	void							calculateMaxForce(PxU32& index, PxReal supportStrength);  

	//If new island genereates, remove the chunks in the island
	void							removeChunkFromIsland(PxU32 index);
	//Break specific chunk 
	void							breakChunks(PxU32 index);
	//Entry to use physx based stress solver
	void							physcsBasedStressSolverTick();
	void							removeChunkFromShadowScene(bool localIndex, PxU32 index);
	void							resetSupportStrength(PxReal supportStrength);
	void							resetActiveCycle(PxU32 actCycle);
	void							resetInitialWarmFrames(PxU32 iniWarmFrames);
	void							enablePhyStressSolver(PxF32 strength);
	// The stress solver will stop simulating after calling disablePhyStressSolver; 
	void							disablePhyStressSolver();
	void							removePhysStressSolver();
#endif
}; // class DestructibleStructureStressSolver

} // namespace destructible
} // namespace apex
} // namespace physx

#endif // DESTRUCTIBLE_STRUCTURE_STRESS_SOLVER_H

#if 0
/*
issues and improvements:
1) performance - traverseLink() may generate a stack that is too deep - explore ways to return out earlier.
		1.1) tried to return out earlier by returning when we hit a strained link, but this will not work for some composite cases
		1.2) basic assumption that a strained link will go on and find an externally supported chunk may be true initially,
			 but this condition needs to be verified again subsequently.
		1.3) tried to evaluate past strained links for evaluation runs that contain recently broken links that used to be strained,
			 this seemed promising initially, but under an uncommon, yet possible scenario, an unsupported peninsula could be instantly
			 generated by breaking links, without the intermediate stage of it being strained. Thus, this method was abandoned.
2) performance - do less traversing by assimilating previously-traversed peninsula data
		2.1) while this will not lessen the stack generated, performance could be improved if somehow we can cache peninsula information
			 for an evaluation run. We can reliably reuse this peninsula information instead traversing past it again. Using the flag
			 PeninsulaEncounteredAnotherWeakLink could be useful for this purpose. Doing this may require significant refactoring.
3) realism - widen criteria for qualifying weak links
		3.1) we can improve passLinkCountTest() and passLinkAdjacencyTest() to qualify more links for peninsula assessment. For example,
			 we can try extending it to assess 4 links as well, and also possibly to assess 3 traversal paths.
4) realism - make better use of physical data to create better break-offs
		4.1) we can try and make a peninsula break off in a better direction, such as about the pivot (the weak link), instead of it
			 dropping down vertically.
*/
#endif // notes
