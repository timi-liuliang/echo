/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "DestructibleStructureStressSolver.h"
#include "DestructibleStructure.h"
#include "DestructibleActor.h"

namespace
{
	const physx::PxU32 InvalidLinkedChunkIndex = 0xFFFFFFFF;
#if NX_SDK_VERSION_MAJOR == 3
	const physx::PxU32 MaxWarmingFrame = 200;
	const physx::PxU32 MaxLeaveInAdvaceWarmingWindow = 100;
	const physx::PxU32 MaxActiveCycle = 1000;
	const physx::PxReal ShadowSceneDeltaTime = 0.001;
#endif
}; // namespace nameless

namespace physx
{
namespace apex
{
namespace destructible
{

// a helper data structure used to hold a [actor pointer] [chunk pointer] pair
struct ActorChunkPair
{
public:
	ActorChunkPair(const DestructibleActor * actorAlias, const DestructibleStructure::Chunk * chunkAlias);
	~ActorChunkPair();

	const DestructibleActor *				actorAlias;
	const DestructibleStructure::Chunk *	chunkAlias;
private:
	ActorChunkPair();
}; // struct ActorChunkPair

ActorChunkPair::ActorChunkPair(const DestructibleActor * actorAlias_, const DestructibleStructure::Chunk * chunkAlias_)
	:actorAlias(actorAlias_)
	,chunkAlias(chunkAlias_)
{
	PX_ASSERT(NULL != actorAlias);
	PX_ASSERT(NULL != chunkAlias);
}

ActorChunkPair::~ActorChunkPair()
{
	chunkAlias = NULL;
	actorAlias = NULL;
}

StructureScratchBuffer::StructureScratchBuffer(physx::PxU8 * const bufferStart_, physx::PxU32 bufferCount_)
	:bufferStart(bufferStart_)
	,bufferCount(bufferCount_)
{
	PX_ASSERT(NULL != bufferStart && 0 != bufferCount);
	::memset(static_cast<void*>(bufferStart), StructureScratchBuffer::ByteEmpty, bufferCount * sizeof(bufferStart[0]));
}

StructureScratchBuffer::~StructureScratchBuffer()
{
}

bool StructureScratchBuffer::isOccupied(physx::PxU32 index) const
{
	PX_ASSERT(index < bufferCount);
	PX_ASSERT(StructureScratchBuffer::ByteEmpty == bufferStart[index] || StructureScratchBuffer::ByteExist == bufferStart[index]);
	return StructureScratchBuffer::ByteExist == bufferStart[index];
}

void StructureScratchBuffer::setOccupied(physx::PxU32 index)
{
	PX_ASSERT(index < bufferCount);
	PX_ASSERT(StructureScratchBuffer::ByteEmpty == bufferStart[index]);
	bufferStart[index] = StructureScratchBuffer::ByteExist;
}

PeninsulaData::PeninsulaData()
	:rootLinkedChunkIndex(InvalidLinkedChunkIndex)
	,peninsulaState(0x00)
	,chunkCount(0)
	,aggregateMass(0.0f)
	,geometricCenter(0.0f)
{
}

PeninsulaData::PeninsulaData(physx::PxU8 peninsulaState_, physx::PxU32 chunkCount_)
	:rootLinkedChunkIndex(InvalidLinkedChunkIndex)
	,peninsulaState(peninsulaState_)
	,chunkCount(chunkCount_)
	,aggregateMass(0.0f)
	,geometricCenter(0.0f)
{
}

PeninsulaData::PeninsulaData(physx::PxU8 peninsulaState_, physx::PxU32 chunkCount_, physx::PxF32 aggregateMass_, physx::PxVec3 geometricCenter_)
	:rootLinkedChunkIndex(InvalidLinkedChunkIndex)
	,peninsulaState(peninsulaState_)
	,chunkCount(chunkCount_)
	,aggregateMass(aggregateMass_)
	,geometricCenter(geometricCenter_)
{
}

PeninsulaData::~PeninsulaData()
{
}

bool PeninsulaData::isValid() const
{
	PX_ASSERT(peninsulaState < PeninsulaData::PeninsulaInvalidMaxFlag);
	return (!hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport));
}

void PeninsulaData::resetData()
{
	chunkCount		= 0;
	aggregateMass	= 0.0f;
	geometricCenter = physx::PxVec3(0.0f);
}

void PeninsulaData::setFlag(PeninsulaData::PeninsulaFlag peninsulaFlag)
{
	PX_ASSERT(peninsulaFlag		< PeninsulaData::PeninsulaInvalidMaxFlag);
	PX_ASSERT(!hasFlag(peninsulaFlag));
	peninsulaState |= peninsulaFlag;
	PX_ASSERT(peninsulaState	< PeninsulaData::PeninsulaInvalidMaxFlag);
}

void PeninsulaData::setLinkedChunkData(const ActorChunkPair & actorChunkPair, Int2Type<StressEvaluationType::EvaluateByCount>)
{
	PX_ASSERT(NULL != &actorChunkPair);
	PX_ASSERT(peninsulaState < PeninsulaData::PeninsulaInvalidMaxFlag);
	PX_ASSERT(0 == chunkCount);
	PX_ASSERT(0.0f == aggregateMass);
	PX_ASSERT(geometricCenter.isZero());
	chunkCount		= 1;
	PX_UNUSED(actorChunkPair);
}

void PeninsulaData::setLinkedChunkData(const ActorChunkPair & actorChunkPair, Int2Type<StressEvaluationType::EvaluateByMoment>)
{
	PX_ASSERT(NULL != &actorChunkPair);
	PX_ASSERT(peninsulaState < PeninsulaData::PeninsulaInvalidMaxFlag);
	PX_ASSERT(0 == chunkCount);
	PX_ASSERT(0.0f == aggregateMass);
	PX_ASSERT(geometricCenter.isZero());
	chunkCount		= 1;
	aggregateMass	= actorChunkPair.actorAlias->getChunkMass(actorChunkPair.chunkAlias->indexInAsset);
	geometricCenter = actorChunkPair.actorAlias->getStructure()->getChunkWorldCentroid(*(actorChunkPair.chunkAlias));
}

void PeninsulaData::setRootLinkedChunkIndex(physx::PxU32 linkedChunkIndex)
{
	PX_ASSERT(InvalidLinkedChunkIndex == rootLinkedChunkIndex);
	rootLinkedChunkIndex = linkedChunkIndex;
}

void PeninsulaData::assimilate(const PeninsulaData & that, Int2Type<StressEvaluationType::EvaluateByCount>)
{
	this->peninsulaState	|= that.peninsulaState;
	this->chunkCount		+= that.chunkCount;
}

void PeninsulaData::assimilate(const PeninsulaData & that, Int2Type<StressEvaluationType::EvaluateByMoment>)
{
	this->peninsulaState	|= that.peninsulaState;
	this->chunkCount		+= that.chunkCount;
	this->geometricCenter	= ((this->geometricCenter * this->aggregateMass) + (that.geometricCenter * that.aggregateMass)) / (this->aggregateMass + that.aggregateMass);
	this->aggregateMass		+= that.aggregateMass;
}

bool PeninsulaData::hasFlag(PeninsulaData::PeninsulaFlag peninsulaFlag) const
{
	PX_ASSERT(peninsulaFlag		< PeninsulaData::PeninsulaInvalidMaxFlag);
	PX_ASSERT(peninsulaState	< PeninsulaData::PeninsulaInvalidMaxFlag);
	return (0 != (peninsulaState & peninsulaFlag));
}

physx::PxU32 PeninsulaData::getDataChunkCount() const
{
	return chunkCount;
}

physx::PxF32 PeninsulaData::getDataAggregateMass() const
{
	return aggregateMass;
}

const physx::PxVec3 & PeninsulaData::getDataGeometricCenter() const
{
	return geometricCenter;
}

physx::PxU32 PeninsulaData::getRootLinkedChunkIndex() const
{
	PX_ASSERT(InvalidLinkedChunkIndex != rootLinkedChunkIndex);
	return rootLinkedChunkIndex;
}

SnapEvent * SnapEvent::instantiate(FractureEventProxy * fractureEventProxyBufferStart, physx::PxU32 fractureEventProxyBufferCount, physx::PxF32 tickSecondsToSnap)
{
	SnapEvent * snapEvent = NULL;
	PX_ASSERT(NULL != fractureEventProxyBufferStart);
	PX_ASSERT(0 != fractureEventProxyBufferCount);
	PX_ASSERT(tickSecondsToSnap >= 0.0f);
	snapEvent = PX_NEW(SnapEvent)(fractureEventProxyBufferStart, fractureEventProxyBufferCount, tickSecondsToSnap);
	return snapEvent;
}

void SnapEvent::onDestroy()
{
	PX_ASSERT(NULL != fractureEventProxyBufferStart);
	PX_DELETE([]fractureEventProxyBufferStart); // derived from UserAllocated, don't have to use PX_FREE(x)
	fractureEventProxyBufferStart = NULL;
	PX_DELETE(this);
}

bool SnapEvent::isRipeAfterUpdatingTime(physx::PxF32 deltaTime)
{
	bool isRipeAfterUpdatingTime = false;
	PX_ASSERT(deltaTime >= 0.0f);
	if(tickSecondsRemainingToSnap >= 0.0f)
	{
		PX_ASSERT(deltaTime > FLT_EPSILON);
		tickSecondsRemainingToSnap -= deltaTime;
		if(tickSecondsRemainingToSnap < 0.0f)
		{
			isRipeAfterUpdatingTime = true;
		}
	}
	return isRipeAfterUpdatingTime;
}

bool SnapEvent::isExpired() const
{
	return (tickSecondsRemainingToSnap < 0.0f);
}

physx::PxU32 SnapEvent::getFractureEventProxyCount() const
{
	return fractureEventProxyBufferCount;
}

const FractureEventProxy & SnapEvent::getFractureEventProxy(physx::PxU32 index) const
{
	PX_ASSERT(index < fractureEventProxyBufferCount);
	PX_ASSERT(NULL != fractureEventProxyBufferStart);
	return fractureEventProxyBufferStart[index];
}

SnapEvent::SnapEvent(FractureEventProxy * fractureEventProxyBufferStart_, physx::PxU32 fractureEventProxyBufferCount_, physx::PxF32 tickSecondsToSnap_)
	:fractureEventProxyBufferStart(fractureEventProxyBufferStart_)
	,fractureEventProxyBufferCount(fractureEventProxyBufferCount_)
	,tickSecondsRemainingToSnap(tickSecondsToSnap_)
{
	PX_ASSERT(NULL != fractureEventProxyBufferStart);
	PX_ASSERT(0 != fractureEventProxyBufferCount);
	PX_ASSERT(tickSecondsRemainingToSnap >= 0.0f);
}

SnapEvent::~SnapEvent()
{
	PX_ASSERT(NULL == fractureEventProxyBufferStart);
}
#if NX_SDK_VERSION_MAJOR == 3
DestructibleStructureStressSolver::DestructibleStructureStressSolver(const DestructibleStructure & bindedStructureAlias_ , PxF32 strength_)
#elif NX_SDK_VERSION_MAJOR == 2
DestructibleStructureStressSolver::DestructibleStructureStressSolver(const DestructibleStructure & bindedStructureAlias_ )
#endif
	:bindedStructureAlias(bindedStructureAlias_)
	,structureLinkCount(0)
	,structureLinkCondition(NULL)
	,scratchBuffer(NULL)
	,scratchBufferLocked(false)
	,userTimeDelay(PX_MAX_F32)
	,userMassThreshold(PX_MAX_F32)
{
	PX_ASSERT(NULL != &bindedStructureAlias);

	// initialize internal members
	structureLinkCount = bindedStructureAlias.chunks.size();
	structureLinkCondition = PX_NEW(physx::PxU8)[structureLinkCount];
	::memset(static_cast<void*>(structureLinkCondition), 0x00, structureLinkCount * sizeof(physx::PxU8));
	for(physx::PxU32 index = 0; index < bindedStructureAlias.supportDepthChunks.size(); ++index)
	{
		structureLinkCondition[bindedStructureAlias.supportDepthChunks[index]] = DestructibleStructureStressSolver::LinkedChunkExist;
	}
	scratchBuffer = PX_NEW(physx::PxU8)[structureLinkCount];
	::memset(static_cast<void*>(scratchBuffer), 0x00, structureLinkCount * sizeof(physx::PxU8));
	PX_ASSERT(structureLinkCount > 0 && NULL != structureLinkCondition && NULL != scratchBuffer);

	// get user's parameters
	for(physx::Array<DestructibleActor*>::ConstIterator kIter = bindedStructureAlias.destructibles.begin(); kIter != bindedStructureAlias.destructibles.end(); ++kIter)
	{
		const DestructibleActor & currentDestructibleActor = *(*kIter);
		PX_ASSERT(NULL != &currentDestructibleActor);
		const physx::PxF32 currentUserTimeDelay	= currentDestructibleActor.getStressSolverTimeDelay();
		PX_ASSERT(currentUserTimeDelay >= 0.0f);
		const physx::PxF32 currentUserMassThreshold	= currentDestructibleActor.getStressSolverMassThreshold();
		PX_ASSERT(currentUserMassThreshold >= 0.0f);
		if(currentUserTimeDelay < userTimeDelay)
		{
			userTimeDelay = currentUserTimeDelay;
		}
		if(currentUserMassThreshold < userMassThreshold)
		{
			userMassThreshold = currentUserMassThreshold;
		}
	}
	PX_ASSERT(userTimeDelay < PX_MAX_F32);
	PX_ASSERT(userMassThreshold < PX_MAX_F32);
#if NX_SDK_VERSION_MAJOR == 3
	//Define the real joint for the chunks within the Actors 
	isPhysxBasedSim = false;
	isSimulating = false;
	if(strength_ > 0)
	{
		createShadowScene(MaxWarmingFrame,MaxActiveCycle,strength_);
	}
#endif
}

DestructibleStructureStressSolver::~DestructibleStructureStressSolver()
{
#if NX_SDK_VERSION_MAJOR == 3
	if(isPhysxBasedSim)
	{
		removePhysStressSolver();
	}
#endif
	while(!snapEventContainer.empty())
	{
		snapEventContainer.popBack()->onDestroy();
	}
	if(NULL != scratchBuffer)
	{
		PX_ASSERT(!scratchBufferLocked);
		PX_FREE(scratchBuffer); // use PX_FREE(x) instead of PX_DELETE([]x)
		scratchBuffer = NULL;
	}
	if(NULL != structureLinkCondition)
	{
		PX_FREE(structureLinkCondition); // use PX_FREE(x) instead of PX_DELETE([]x)
		structureLinkCondition = NULL;
	}
}

void DestructibleStructureStressSolver::onTick(physx::PxF32 deltaTime)
{
	PX_ASSERT(deltaTime >= 0.0f);
	if(!snapEventContainer.empty())
	{
		physx::PxU32 expiredSnapEventCount = 0;

		// update and fracture ripe linked chunk(s)
		for(physx::Array<SnapEvent*>::ConstIterator kIter = snapEventContainer.begin(); kIter != snapEventContainer.end(); ++kIter)
		{
			SnapEvent & currentSnapEvent = *(*kIter);
			PX_ASSERT(NULL != &currentSnapEvent);
			const bool snapEventJustRipe = currentSnapEvent.isRipeAfterUpdatingTime(deltaTime);
			if(snapEventJustRipe)
			{
				for(physx::PxU32 index = 0; index < currentSnapEvent.getFractureEventProxyCount(); ++index)
				{
					// skip fracturing linked chunks that were fractured in the meantime
					const FractureEventProxy & currentFractureEventProxy = currentSnapEvent.getFractureEventProxy(index);
					PX_ASSERT(NULL != &currentFractureEventProxy);
					const physx::PxU32 currentLinkedChunkIndex = currentFractureEventProxy.rootLinkedChunkIndex;
					PX_ASSERT(assertLinkedChunkIndexOk(currentLinkedChunkIndex));
					if(!isLinkedChunkBroken(currentLinkedChunkIndex))
					{
						setLinkedChunkBroken(currentLinkedChunkIndex);
						const FractureEvent & currentFractureEvent = interpret(currentFractureEventProxy);
						PX_ASSERT(NULL != &currentFractureEvent);
						getBindedStructureMutable().fractureChunk(currentFractureEvent);
					}
				}
				continue;
			}
			else
			{
				if(currentSnapEvent.isExpired())
				{
					++expiredSnapEventCount;
					continue;
				}
			}
		}

		// clear the container if it no longer contain any pending snap events
		PX_ASSERT(expiredSnapEventCount <= snapEventContainer.size());
		if(snapEventContainer.size() == expiredSnapEventCount)
		{
			while(!snapEventContainer.empty())
			{
				snapEventContainer.popBack()->onDestroy();
			}
		}
	}
}
#if NX_SDK_VERSION_MAJOR == 3
void	DestructibleStructureStressSolver::physcsBasedStressSolverTick()
{
		PX_PROFILER_PERF_SCOPE("DestructiblePhysBasedStressSolver");

		if(currentWarmingFrame < initialWarmFrames)
		{
			currentWarmingFrame++;
		}
		else
		{
			initializingMaxForce = false;
		}
		if(continuousNoUpdationCount > MaxLeaveInAdvaceWarmingWindow)
		{
			initializingMaxForce = false;
		}
		PxU32 breakChunk = 0xFFFFFFFF;

		if(initializingMaxForce || !isSleep)
		{
			breakChunk = predictBreakChunk(ShadowSceneDeltaTime,supportStrength);
		}
		if(currentActiveSimulationFrame < activeCycle)
		{
			if(isActiveWakedUp)
			{
				isSleep = false;
			}
			++currentActiveSimulationFrame;
		}
		else
		{
			isSleep = true;
			currentActiveSimulationFrame = 0;
		}
		isActiveWakedUp = false;
		if(!initializingMaxForce)
		{
			for(PxU32 i = 0;i < this->initialWarmingKillList.size();i++)
			{
				removeChunkFromShadowScene(true,initialWarmingKillList[i]);
			}
			initialWarmingKillList.clear();
			for(PxU32 i = 0;i < islandKillList.size();i++)
			{
				isActiveWakedUp = true;
				if(!initializingMaxForce)
				{
					removeChunkFromShadowScene(false,islandKillList[i]);
				}
			}
			islandKillList.clear();
		}
		
	
		if(breakChunk == 0xFFFFFFFF)
		{
				return;
		}
		else
		{
			breakChunks(breakChunk);
		}
}

void	DestructibleStructureStressSolver::removeChunkFromIsland(PxU32 index)
{
		islandKillList.pushBack(index);
}

void	DestructibleStructureStressSolver::breakChunks(PxU32 breakChunk)
{
		removeChunkFromShadowScene(false, breakChunk);
		DestructibleStructure::Chunk &chunk = getBindedStructureMutable().chunks[breakChunk];
		if(!chunk.isDestroyed())
		{
			FractureEvent e2;
			e2.position= getBindedStructureMutable().getChunkWorldCentroid(chunk);
			e2.chunkIndexInAsset = chunk.indexInAsset;
			e2.impulse= PxVec3(0.0f);
			e2.destructibleID = chunk.destructibleID;
			e2.flags= chunk.flags;
			e2.hitDirection		= physx::PxVec3(0.0f);
			getBindedStructureMutable().fractureChunk(e2);
		}
}

void	DestructibleStructureStressSolver::calculateMaxForce(PxU32& index,  PxReal supportStrength)
{
	PX_PROFILER_PERF_SCOPE("DestructiblePhysBasedStressSolverCalculateForce");
	PxReal maximumForce = 0;
	PxU32  maximumIndex = 0xFFFFFFFF;
	PxVec3 linearForce(0),angularForce(0);
	PxU32  updateCount = 0;
	for(PxU32 i = 0;i < linkedJoint.size();i++)
	{
		if(!linkedJoint[i].isDestroyed)
		{
			linkedJoint[i].joint->getConstraint()->getForce(linearForce,angularForce);
			forces[linkedJoint[i].actor1Index] += angularForce.magnitude();
		}
	}
	for(PxU32 i = 0;i<forces.size();i++)
	{
		if(!shadowActors[i].isDestroyed)
		{
			shadowActors[i].currentForce = forces[i];
			if(initializingMaxForce)
			{
				if(forces[i] > shadowActors[i].maxInitialForce)
				{
					updateCount++;
				}
				shadowActors[i].maxInitialForce = PxMax( shadowActors[i].maxInitialForce,forces[i]);
			}
			if(forces[i] > maximumForce)
			{
				maximumForce = forces[i];
				maximumIndex = i;
			}
		}
	}
	for(PxU32 i = 0; i<forces.size();i++)
	{
		forces[i] = 0;
	}
	 
	if(updateCount <= (supportChunkSize/((MaxLeaveInAdvaceWarmingWindow*2))))
	{
		if(isNoChunkWarmingUpdate)
		{
			continuousNoUpdationCount++;
		}
		isNoChunkWarmingUpdate = true;
	}
	else
	{
		isNoChunkWarmingUpdate = false;
		continuousNoUpdationCount = 0;
	}
	if(initializingMaxForce)
	{
		index = 0xFFFFFFFF;
		return;
	}
	if(maximumIndex < shadowActors.size() && shadowActors[maximumIndex].currentForce >=  shadowActors[maximumIndex].maxInitialForce* (1+supportStrength))
	{
		index = bindedStructureAlias.supportDepthChunks[maximumIndex];
		return;
	}
	else
	{
		index = 0xFFFFFFFF;
		return;
	}
	return;
}

void	DestructibleStructureStressSolver::removeChunkFromShadowScene(bool localIndex, PxU32 index)
{
	PX_PROFILER_PERF_SCOPE("DestructiblePhysBasedStressSolverRemoveChunk");
	if(!localIndex)
	{
		index = globalHashedtoSupportChunks[index];	
	}
	if(index == 0xFFFFFFFF)
	{
		return;
	}
	if(initializingMaxForce)
	{
		initialWarmingKillList.pushBack(index);
		return;
	}
	if(!shadowActors[index].isDestroyed)
	{
		shadowScene->removeActor(*shadowActors[index].actor);
		isActiveWakedUp = true;
		shadowActors[index].isDestroyed = true;
	}
}

PxU32	DestructibleStructureStressSolver::predictBreakChunk(PxReal deltaTime, PxReal supportStrength)
{
	// Simulate shadowScene for prediction 
	PxU32  maximumIndex = 0xFFFFFFFF;
	//if(shadowScene->checkResults())
	{
		if(isSimulating)
		{
			shadowScene->fetchResults(true);	
		}
	}
	
	shadowScene->simulate(deltaTime);
	isSimulating = true;
	
	calculateMaxForce(maximumIndex,supportStrength);
	return maximumIndex;
}

void DestructibleStructureStressSolver::resetActiveCycle(PxU32 actCycle)
{
	activeCycle = actCycle;
}

void DestructibleStructureStressSolver::resetInitialWarmFrames(PxU32 iniWarmFrames)
{
	initialWarmFrames = iniWarmFrames;
}

void DestructibleStructureStressSolver::resetSupportStrength(PxReal supStrength)
{
	supportStrength = supStrength;
}

void DestructibleStructureStressSolver::createShadowScene(PxU32 iniWarmFrames,PxU32 actCycle, PxReal supStrength)
{

	PX_PROFILER_PERF_SCOPE("DestructiblePhysBasedStressSolverInitial");
	//Initialize Parameters 
	initializingMaxForce = true;
	isSimulating = false;
	initialWarmFrames = iniWarmFrames;
	currentWarmingFrame = 0;
	supportChunkSize = bindedStructureAlias.supportDepthChunks.size();
	isSleep = true;
	isActiveWakedUp = false;
	currentActiveSimulationFrame = 0;
	continuousNoUpdationCount = 0;
	activeCycle = actCycle;
	isPhysxBasedSim = true;
	activeCycle = actCycle;
	supportStrength = supStrength;
	isCustomEnablesSimulating = true;
	isNoChunkWarmingUpdate = false;

	//Initialize the shadow scene

	bindedStructureAlias.dscene->getApexScene()->getPhysXScene()->lockRead();

	newSceneSDK = &bindedStructureAlias.dscene->getApexScene()->getPhysXScene()->getPhysics();
	PxSceneDesc sceneDesc(newSceneSDK->getTolerancesScale());
	sceneDesc.gravity = bindedStructureAlias.dscene->getApexScene()->getPhysXScene()->getGravity();
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(4);
	sceneDesc.filterCallback = NULL;
	sceneDesc.filterShader = bindedStructureAlias.dscene->getApexScene()->getPhysXScene()->getFilterShader();
	shadowScene = newSceneSDK->createScene(sceneDesc);
	shadowScene->setSimulationEventCallback(NULL);

	bindedStructureAlias.dscene->getApexScene()->getPhysXScene()->unlockRead();


	//Initialize arrays
	globalHashedtoSupportChunks.resize(bindedStructureAlias.chunks.size());
	shadowActors.resize(supportChunkSize);
	for(PxU32 i = 0;i < supportChunkSize; ++i)
	{
		globalHashedtoSupportChunks[i] = 0xFFFFFFFF;
	}
	for(PxU32 i = 0 ;i < supportChunkSize; ++i)
	{
		globalHashedtoSupportChunks[bindedStructureAlias.supportDepthChunks[i]] = i;
	}
	PxBounds3 bound;
	bound.setEmpty();
	for(PxU32 i = 0; i < supportChunkSize; ++i)
	{
		if(!bindedStructureAlias.chunks[bindedStructureAlias.supportDepthChunks[i]].isDestroyed())
		{
			bound.include(bindedStructureAlias.getChunkWorldCentroid(bindedStructureAlias.chunks[bindedStructureAlias.supportDepthChunks[i]]));
		}
	}
	PxVec3 translate = bound.getCenter();
	for(PxU32 i = 0; i< supportChunkSize; ++i)
	{
		forces.pushBack(0);
	}
	//Initialize shadow actors
	for(PxU32 i = 0;i < supportChunkSize;++i)
	{
		const physx::apex::destructible::DestructibleStructure::Chunk*  chunk = &bindedStructureAlias.chunks[bindedStructureAlias.supportDepthChunks[i]];
		if(!chunk->isDestroyed()/*&&!(chunk->flags&ChunkRemoved)*/)	// BRG - reclaimed ChunkRemoved bit.  This should not have been used here anyhow - it had function-limited scope
		{
			shadowActors[i].actor =  newSceneSDK->createRigidDynamic(PxTransform(bindedStructureAlias.getChunkWorldCentroid(bindedStructureAlias.chunks[bindedStructureAlias.supportDepthChunks[i]])-translate));
			DestructibleActor* dactor = bindedStructureAlias.dscene->mDestructibles.direct(chunk->destructibleID);
			shadowActors[i].actor->setMass(dactor->getChunkMass(chunk->indexInAsset));
			shadowScene->addActor(*shadowActors[i].actor);
			shadowActors[i].actor->wakeUp();
			shadowActors[i].isDestroyed = false;
			shadowActors[i].currentForce = 0.0;
			shadowActors[i].maxInitialForce = 0.0;
			//Add the static joint objects to the externally supported chunks
			if(chunk->flags& ChunkExternallySupported)
			{
				PxFixedJointCreate(*newSceneSDK,shadowActors[i].actor,PxTransform(PxVec3(0.0f, 0.0, 0.0f), PxQuat::createIdentity()),NULL,shadowActors[i].actor->getGlobalPose());
			}
		}
		else
		{
			shadowActors[i].isDestroyed=true;
		}
	}
	//Create the joints using a local hash array.. .after that we clear it. 
	Array<Array<bool> > linkedHash;
	linkedHash.resize(supportChunkSize);
	for(PxU32 i = 0;i < supportChunkSize;++i)
	{
		linkedHash[i].resize(supportChunkSize);
		for(PxU32 j = 0;j < supportChunkSize; ++j)
		{
			linkedHash[i][j]=false;
		}
	}
	//Link all the chunks which are overlapping. 
	for(PxU32 i = 0; i < supportChunkSize;++i)
	{
		const physx::PxU32 firstOverlapIndex	= bindedStructureAlias.firstOverlapIndices[bindedStructureAlias.supportDepthChunks[i]];
    	const physx::PxU32 stopOverlapIndex		= bindedStructureAlias.firstOverlapIndices[bindedStructureAlias.supportDepthChunks[i]+1];
		for(physx::PxU32 index = firstOverlapIndex; index < stopOverlapIndex; ++index)
		{
			const physx::PxU32 currentLinkedChunkIndex = bindedStructureAlias.overlaps[index];	
			linkedHash[globalHashedtoSupportChunks[currentLinkedChunkIndex]][i]=true;
			linkedHash[i][globalHashedtoSupportChunks[currentLinkedChunkIndex]]=true;
		}
	}
	//We use hash since we don't know if there is redundant overlapping info, for example A-B and B-A.So we should gurantee that only one joint is linked between two rigid bodies.
	//The overhead to initial hash is not so big since it is in the initializing stage, not affect the frame rate when simulating. 
	for(PxU32 i = 0; i < supportChunkSize; ++i )
	{
		for(PxU32 j = 0; j < i; ++j)
		{
			if(linkedHash[i][j])
			{
				//Create the joints for actor1 and actor2
				if(!shadowActors[i].isDestroyed&&!shadowActors[j].isDestroyed)
				{
					PxFixedJoint* createdJoint = PxFixedJointCreate(*newSceneSDK,shadowActors[i].actor,shadowActors[j].actor->getGlobalPose(),shadowActors[j].actor,shadowActors[i].actor->getGlobalPose());
					if(createdJoint)
					{
						LinkedJoint joint;
						joint.actor1Index = i; 
						joint.actor2Index = j;
						joint.isDestroyed = false;
						joint.joint = createdJoint; 
						linkedJoint.pushBack(joint);
					}
				}
			}
		}
	}
	for(PxU32 i = 0;i < linkedHash.size(); ++i)
	{
		linkedHash[i].clear();
	}
	linkedHash.clear();
}

void DestructibleStructureStressSolver::enablePhyStressSolver(PxF32 strength)
{
	PX_ASSERT(strength > 0);
	isCustomEnablesSimulating = true;
	if(!isPhysxBasedSim)
	{
		createShadowScene(MaxWarmingFrame,MaxActiveCycle,strength);
	}
	bindedStructureAlias.dscene->setStructureUpdate(&getBindedStructureMutable(), true);	// Get a tick to update this
}

void DestructibleStructureStressSolver::removePhysStressSolver()
{
	if(isPhysxBasedSim)
	{
		for(PxU32 i=0;i<shadowActors.size();i++)
		{
			if(!shadowActors[i].isDestroyed)
			{
				shadowScene->removeActor(*shadowActors[i].actor);
			}
		}
		shadowActors.clear();
		for(PxU32 i=0;i<linkedJoint.size();i++)
		{
			if(!linkedJoint[i].isDestroyed)
			{
				if(linkedJoint[i].joint)
				{
					linkedJoint[i].joint->release();
					linkedJoint[i].joint=NULL;
				}
			}
		}
		linkedJoint.clear();

		if (isSimulating) 
		{
			shadowScene->fetchResults(true);
			isSimulating = false;
		}

		if(shadowScene)
		{
			shadowScene->release();
		}
		
		globalHashedtoSupportChunks.clear();	
		initialWarmingKillList.clear();
		islandKillList.clear();
		forces.clear(); 
	}
}

void DestructibleStructureStressSolver::disablePhyStressSolver()
{
	isCustomEnablesSimulating = false;
}
#endif
void DestructibleStructureStressSolver::onUpdate(physx::PxU32 linkedChunkIndex)
{
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
	if(!isLinkedChunkBroken(linkedChunkIndex))
	{
		setLinkedChunkBroken(linkedChunkIndex);
		recentlyBrokenLinkedChunkIndices.pushBack(linkedChunkIndex);
	}
}

void DestructibleStructureStressSolver::onResolve()
{
	physx::Array<physx::PxU32> linkedChunkIndicesForEvaluation;
	processLinkedChunkIndicesForEvaluation(linkedChunkIndicesForEvaluation);
	if(!linkedChunkIndicesForEvaluation.empty())
	{
		evaluateForPotentialIslands(linkedChunkIndicesForEvaluation);
	}
}

void DestructibleStructureStressSolver::processLinkedChunkIndicesForEvaluation(physx::Array<physx::PxU32> & linkedChunkIndicesForEvaluation)
{
	PX_ASSERT(NULL != &linkedChunkIndicesForEvaluation);
	PX_ASSERT(linkedChunkIndicesForEvaluation.empty());

	StructureScratchBuffer evaluationRecord = acquireScratchBuffer();
	
	// avoid getting snap events' links because they either pending fracture or are already broken (the snap event container only clear out when all snap events have been executed)
	for(physx::Array<SnapEvent*>::ConstIterator kIter = snapEventContainer.begin(); kIter != snapEventContainer.end(); ++kIter)
	{
		SnapEvent & currentSnapEvent = *(*kIter);
		PX_ASSERT(NULL != &currentSnapEvent);
		for(physx::PxU32 index = 0; index < currentSnapEvent.getFractureEventProxyCount(); ++index)
		{
			const physx::PxU32 currentLinkedChunkIndex = currentSnapEvent.getFractureEventProxy(index).rootLinkedChunkIndex;
			PX_ASSERT(assertLinkedChunkIndexOk(currentLinkedChunkIndex));
			if(!evaluationRecord.isOccupied(currentLinkedChunkIndex))
			{
				evaluationRecord.setOccupied(currentLinkedChunkIndex);
			}
		}
	}

	// push unique adjacent unbroken links of recently broken links
	while(!recentlyBrokenLinkedChunkIndices.empty())
	{
		const physx::PxU32 currentLinkedChunkIndex = recentlyBrokenLinkedChunkIndices.popBack();
		PX_ASSERT(assertLinkedChunkIndexOk(currentLinkedChunkIndex));
		physx::Array<physx::PxU32> unbrokenAdjacentLinkedChunkIndices;
		getUnbrokenAdjacentLinkedChunkIndices(currentLinkedChunkIndex, unbrokenAdjacentLinkedChunkIndices);
		for(physx::Array<physx::PxU32>::ConstIterator kIter = unbrokenAdjacentLinkedChunkIndices.begin(); kIter != unbrokenAdjacentLinkedChunkIndices.end(); ++kIter)
		{
			const physx::PxU32 currentUnbrokenAdjacentLinkedChunkIndex = *kIter;
			PX_ASSERT(assertLinkedChunkIndexOk(currentUnbrokenAdjacentLinkedChunkIndex));
			if(!evaluationRecord.isOccupied(currentUnbrokenAdjacentLinkedChunkIndex))
			{
				evaluationRecord.setOccupied(currentUnbrokenAdjacentLinkedChunkIndex);
				linkedChunkIndicesForEvaluation.pushBack(currentUnbrokenAdjacentLinkedChunkIndex);
			}
		}
	}

	// push unique previously-identified strained links
	if(!strainedLinkedChunkIndices.empty())
	{
		bool containerRequireRefresh = false;

		// we also take this chance to mark links that may already be broken off
		for(physx::Array<physx::PxU32>::Iterator iter = strainedLinkedChunkIndices.begin(); iter != strainedLinkedChunkIndices.end(); ++iter)
		{
			const physx::PxU32 currentLinkedChunkIndex = *iter;
			PX_ASSERT(assertLinkedChunkIndexOk(currentLinkedChunkIndex));
			PX_ASSERT(InvalidLinkedChunkIndex != *iter);
			if(isLinkedChunkBroken(currentLinkedChunkIndex))
			{
				*iter = InvalidLinkedChunkIndex;
				containerRequireRefresh = true;
			}
			else
			{
				PX_ASSERT(isLinkedChunkStrained(currentLinkedChunkIndex));
				if(!evaluationRecord.isOccupied(currentLinkedChunkIndex))
				{
					evaluationRecord.setOccupied(currentLinkedChunkIndex);
					linkedChunkIndicesForEvaluation.pushBack(currentLinkedChunkIndex);
				}
			}
		}

		// update the container to hold only unbroken links
		if(containerRequireRefresh)
		{
			physx::Array<physx::PxU32> updatedContainer;
			for(physx::Array<physx::PxU32>::ConstIterator kIter = strainedLinkedChunkIndices.begin(); kIter != strainedLinkedChunkIndices.end(); ++kIter)
			{
				if(InvalidLinkedChunkIndex != *kIter)
				{
					updatedContainer.pushBack(*kIter);
				}
			}
			strainedLinkedChunkIndices = updatedContainer;
		}
	}

	relinquishScratchBuffer();
	PX_ASSERT(assertLinkedChunkIndicesForEvaluationOk(linkedChunkIndicesForEvaluation));
	PX_ASSERT(recentlyBrokenLinkedChunkIndices.empty());
}

void DestructibleStructureStressSolver::evaluateForPotentialIslands(const physx::Array<physx::PxU32> & linkedChunkIndicesForEvaluation)
{
	PX_COMPILE_TIME_ASSERT(2 == DestructibleStructureStressSolver::PathTraversalCount);
	PX_ASSERT(NULL != &linkedChunkIndicesForEvaluation);
	PX_ASSERT(!linkedChunkIndicesForEvaluation.empty());
	for(physx::Array<physx::PxU32>::ConstIterator kIter = linkedChunkIndicesForEvaluation.begin(); kIter != linkedChunkIndicesForEvaluation.end(); ++kIter)
	{
		const physx::PxU32 currentLinkedChunkIndex = *kIter;
		PX_ASSERT(assertLinkedChunkIndexOk(currentLinkedChunkIndex));
		physx::Array<physx::PxU32> unbrokenAdjacentLinkedChunkIndices;
		if(passLinkCountTest(currentLinkedChunkIndex, unbrokenAdjacentLinkedChunkIndices))
		{
			PX_ASSERT(!unbrokenAdjacentLinkedChunkIndices.empty());
			physx::PxU32 linkedChunkIndicesForTraversal[2] = {InvalidLinkedChunkIndex, InvalidLinkedChunkIndex};
			if(passLinkAdjacencyTest(unbrokenAdjacentLinkedChunkIndices, linkedChunkIndicesForTraversal))
			{
				PX_ASSERT(InvalidLinkedChunkIndex != linkedChunkIndicesForTraversal[0] && InvalidLinkedChunkIndex != linkedChunkIndicesForTraversal[1]);
				evaluateForPotentialPeninsulas(currentLinkedChunkIndex, linkedChunkIndicesForTraversal);
			}
		}
	}
}

bool DestructibleStructureStressSolver::passLinkCountTest(physx::PxU32 linkedChunkIndex, physx::Array<physx::PxU32> & unbrokenAdjacentLinkedChunkIndices) const
{
	PX_COMPILE_TIME_ASSERT(2 == DestructibleStructureStressSolver::LowerLimitActivationCount && 3 == DestructibleStructureStressSolver::UpperLimitActivationCount);
	bool passLinkCountTest = false;
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
	PX_ASSERT(NULL != &unbrokenAdjacentLinkedChunkIndices);
	PX_ASSERT(unbrokenAdjacentLinkedChunkIndices.empty());

	// check that the candidate linked chunk has 2 or 3 remaining unbroken links, and that initially it had more
	const physx::PxU32 firstOverlapIndex	= bindedStructureAlias.firstOverlapIndices[linkedChunkIndex];
	const physx::PxU32 stopOverlapIndex		= bindedStructureAlias.firstOverlapIndices[linkedChunkIndex + 1];
	const physx::PxU32 initialLinkCount = stopOverlapIndex - firstOverlapIndex;
	if(initialLinkCount > 2)
	{
		physx::PxU32 unbrokenLinkCount = 0;
		for(physx::PxU32 index = firstOverlapIndex; index < stopOverlapIndex; ++index)
		{
			const physx::PxU32 currentLinkedChunkIndex = bindedStructureAlias.overlaps[index];
			if(!isLinkedChunkBroken(currentLinkedChunkIndex))
			{
				++unbrokenLinkCount;
				if(unbrokenLinkCount > 3)
				{
					break;
				}
			}	
		}
		switch(unbrokenLinkCount)
		{
		case 0:
			// ignore
			break;
		case 1:
			// ignore
			break;
		case 2:
			{
				PX_ASSERT(initialLinkCount > 2);
				getUnbrokenAdjacentLinkedChunkIndices(linkedChunkIndex, unbrokenAdjacentLinkedChunkIndices);
				passLinkCountTest = true;
			}
			break;
		case 3:
			{
				if(initialLinkCount > 3)
				{
					getUnbrokenAdjacentLinkedChunkIndices(linkedChunkIndex, unbrokenAdjacentLinkedChunkIndices);
					passLinkCountTest = true;
				}
			}
			break;
		case 4:
			// ignore
			break;
		default:
			PX_ASSERT(!"!");
		}
	}
	PX_ASSERT(passLinkCountTest ? (2 == unbrokenAdjacentLinkedChunkIndices.size() || 3 == unbrokenAdjacentLinkedChunkIndices.size()) : unbrokenAdjacentLinkedChunkIndices.empty());
	return passLinkCountTest;
}

bool DestructibleStructureStressSolver::passLinkAdjacencyTest(const physx::Array<physx::PxU32> & unbrokenAdjacentLinkedChunkIndices, physx::PxU32 (&linkedChunkIndicesForTraversal)[2]) const
{
	PX_COMPILE_TIME_ASSERT(2 == DestructibleStructureStressSolver::LowerLimitActivationCount && 3 == DestructibleStructureStressSolver::UpperLimitActivationCount);
	PX_COMPILE_TIME_ASSERT(2 == DestructibleStructureStressSolver::PathTraversalCount);
	bool passLinkAdjacencyTest = false;
	PX_ASSERT(NULL != &unbrokenAdjacentLinkedChunkIndices);
	PX_ASSERT(2 == unbrokenAdjacentLinkedChunkIndices.size() || 3 == unbrokenAdjacentLinkedChunkIndices.size());
	PX_ASSERT(NULL != linkedChunkIndicesForTraversal);
	linkedChunkIndicesForTraversal[0] = InvalidLinkedChunkIndex;
	linkedChunkIndicesForTraversal[1] = InvalidLinkedChunkIndex;

	// check that the candidate linked chunk has 2 traversal paths which are not immediately adjacent to each other, and that only 1 traversal path can optionally include 1 different starting point
	const physx::PxU32 unbrokenLinkCount = unbrokenAdjacentLinkedChunkIndices.size();
	switch(unbrokenLinkCount)
	{
	case 2:
		{
			// the 2 linked chunks must not be adjacent to each other
			if(!areUnbrokenLinkedChunksAdjacent(unbrokenAdjacentLinkedChunkIndices[0], unbrokenAdjacentLinkedChunkIndices[1]))
			{
				linkedChunkIndicesForTraversal[0] = unbrokenAdjacentLinkedChunkIndices[0];
				linkedChunkIndicesForTraversal[1] = unbrokenAdjacentLinkedChunkIndices[1];
				passLinkAdjacencyTest = true;
			}
		}
		break;
	case 3:
		{
			// if 2 linked chunks are adjacent, the 3rd linked chunk cannot be adjacent to either one
			if(areUnbrokenLinkedChunksAdjacent(unbrokenAdjacentLinkedChunkIndices[0], unbrokenAdjacentLinkedChunkIndices[1]))
			{
				if(!areUnbrokenLinkedChunksAdjacent(unbrokenAdjacentLinkedChunkIndices[0], unbrokenAdjacentLinkedChunkIndices[2]) &&
				   !areUnbrokenLinkedChunksAdjacent(unbrokenAdjacentLinkedChunkIndices[1], unbrokenAdjacentLinkedChunkIndices[2]) )
				{
					linkedChunkIndicesForTraversal[0] = unbrokenAdjacentLinkedChunkIndices[0];
					linkedChunkIndicesForTraversal[1] = unbrokenAdjacentLinkedChunkIndices[2];
					passLinkAdjacencyTest = true;
				}
			}
			// if 2 linked chunks are not adjacent, the 3rd linked chunk must be adjacent to only one other linked chunk
			else
			{
				if( areUnbrokenLinkedChunksAdjacent(unbrokenAdjacentLinkedChunkIndices[0], unbrokenAdjacentLinkedChunkIndices[2]) ?
				   !areUnbrokenLinkedChunksAdjacent(unbrokenAdjacentLinkedChunkIndices[1], unbrokenAdjacentLinkedChunkIndices[2]) :
					areUnbrokenLinkedChunksAdjacent(unbrokenAdjacentLinkedChunkIndices[1], unbrokenAdjacentLinkedChunkIndices[2]) )
				{
					linkedChunkIndicesForTraversal[0] = unbrokenAdjacentLinkedChunkIndices[0];
					linkedChunkIndicesForTraversal[1] = unbrokenAdjacentLinkedChunkIndices[1];
					passLinkAdjacencyTest = true;
				}
			}
		}
		break;
	default:
		PX_ASSERT(!"!");
	}
	PX_ASSERT(passLinkAdjacencyTest ? (InvalidLinkedChunkIndex != linkedChunkIndicesForTraversal[0] && InvalidLinkedChunkIndex != linkedChunkIndicesForTraversal[1]) : (InvalidLinkedChunkIndex == linkedChunkIndicesForTraversal[0] && InvalidLinkedChunkIndex == linkedChunkIndicesForTraversal[1]));
	return passLinkAdjacencyTest;
}

void DestructibleStructureStressSolver::evaluateForPotentialPeninsulas(physx::PxU32 rootLinkedChunkIndex, const physx::PxU32 (&linkedChunkIndicesForTraversal)[2])
{
	PX_COMPILE_TIME_ASSERT(2 == DestructibleStructureStressSolver::PathTraversalCount);
	PX_ASSERT(assertLinkedChunkIndexOk(rootLinkedChunkIndex));
	PX_ASSERT(NULL != linkedChunkIndicesForTraversal);

	// collect both peninsulas' data
	PeninsulaData peninsulaData[2];
	for(physx::PxU32 index = 0; index < 2; ++index)
	{
		StructureScratchBuffer traverseRecord = acquireScratchBuffer();
		traverseRecord.setOccupied(rootLinkedChunkIndex);
		peninsulaData[index].resetData();
		peninsulaData[index] = traverseLink(linkedChunkIndicesForTraversal[index], traverseRecord);
		peninsulaData[index].setRootLinkedChunkIndex(rootLinkedChunkIndex);
		relinquishScratchBuffer();
	}

	evaluatePeninsulas(peninsulaData);
}

PeninsulaData DestructibleStructureStressSolver::traverseLink(physx::PxU32 linkedChunkIndex, StructureScratchBuffer & traverseRecord) const
{
	PeninsulaData peninsulaData;
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));

	// this will not count as a valid peninsula if it contains any chunk that is being externally supported
	traverseRecord.setOccupied(linkedChunkIndex);
	if(0 != (ChunkExternallySupported & bindedStructureAlias.chunks[linkedChunkIndex].flags))
	{
		peninsulaData.setFlag(PeninsulaData::PeninsulaEncounteredExternalSupport);
	}
	if(isLinkedChunkStrained(linkedChunkIndex))
	{
		peninsulaData.setFlag(PeninsulaData::PeninsulaEncounteredAnotherWeakLink);
	}

	// gather data for remaining unbroken links
	if(peninsulaData.isValid())
	{
		peninsulaData.setLinkedChunkData(getActorChunkPair(linkedChunkIndex), Int2Type<StressEvaluationEnum>());
		const physx::PxU32 firstOverlapIndex	= bindedStructureAlias.firstOverlapIndices[linkedChunkIndex];
		const physx::PxU32 stopOverlapIndex		= bindedStructureAlias.firstOverlapIndices[linkedChunkIndex + 1];
		for(physx::PxU32 index = firstOverlapIndex; index < stopOverlapIndex; ++index)
		{
			const physx::PxU32 currentLinkedChunkIndex = bindedStructureAlias.overlaps[index];
			if(!isLinkedChunkBroken(currentLinkedChunkIndex))
			{
				if(!traverseRecord.isOccupied(currentLinkedChunkIndex))
				{
					peninsulaData.assimilate(traverseLink(currentLinkedChunkIndex, traverseRecord), Int2Type<StressEvaluationEnum>());
					if(!peninsulaData.isValid())
					{
						peninsulaData.resetData();
						break;
					}
				}
			}
		}
	}

	return peninsulaData;
}

void DestructibleStructureStressSolver::evaluatePeninsulas(const PeninsulaData (&peninsulasForEvaluation)[2])
{
	PX_COMPILE_TIME_ASSERT(2 == DestructibleStructureStressSolver::PathTraversalCount);
	PX_ASSERT(NULL != peninsulasForEvaluation);

	// determine type of operation
	const bool trySnapOffWithinFreeIsland		= !peninsulasForEvaluation[0].hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport) && !peninsulasForEvaluation[1].hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport);
	const bool trySnapOffFromSupportedPeninsula	= peninsulasForEvaluation[0].hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport) ^ peninsulasForEvaluation[1].hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport);
	const bool trackLinkForFutureEvaluation		= peninsulasForEvaluation[0].hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport) && peninsulasForEvaluation[1].hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport);
	PX_ASSERT(trySnapOffWithinFreeIsland ? !trySnapOffFromSupportedPeninsula && !trackLinkForFutureEvaluation : trySnapOffFromSupportedPeninsula ? !trackLinkForFutureEvaluation : trackLinkForFutureEvaluation);

	// try snapping off based on data of the peninsula with more mass
	if(trySnapOffWithinFreeIsland)
	{
		const PeninsulaData & subjectPeninsulaData = peninsulasForEvaluation[0].getDataAggregateMass() > peninsulasForEvaluation[1].getDataAggregateMass() ? peninsulasForEvaluation[0] : peninsulasForEvaluation[1];
		SnapEvent * snapEvent = NULL;
		snapEvent = interpret(subjectPeninsulaData, Int2Type<StressEvaluationEnum>());
		if(NULL != snapEvent)
		{
			snapEventContainer.pushBack(snapEvent);
			snapEvent = NULL;
		}
	}
	// try snapping off based on data of the peninsula that is not being externally supported
	else if(trySnapOffFromSupportedPeninsula)
	{
		const PeninsulaData & subjectPeninsulaData = peninsulasForEvaluation[0].hasFlag(PeninsulaData::PeninsulaEncounteredExternalSupport) ? peninsulasForEvaluation[1] : peninsulasForEvaluation[0];
		SnapEvent * snapEvent = NULL;
		snapEvent = interpret(subjectPeninsulaData, Int2Type<StressEvaluationEnum>());
		if(NULL != snapEvent)
		{
			snapEventContainer.pushBack(snapEvent);
			snapEvent = NULL;
		}
	}
	// track this link for re-evaluation because it is susceptible to snapping off
	else if(trackLinkForFutureEvaluation)
	{
		PX_ASSERT(peninsulasForEvaluation[0].getRootLinkedChunkIndex() == peninsulasForEvaluation[1].getRootLinkedChunkIndex());
		const physx::PxU32 linkedChunkIndex = peninsulasForEvaluation[0].getRootLinkedChunkIndex();
		PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
		if(!isLinkedChunkStrained(linkedChunkIndex))
		{
			setLinkedChunkStrained(linkedChunkIndex);
			strainedLinkedChunkIndices.pushBack(linkedChunkIndex);
		}
	}
}

SnapEvent * DestructibleStructureStressSolver::interpret(const PeninsulaData & peninsulaData, Int2Type<StressEvaluationType::EvaluateByCount>) const
{
	SnapEvent * snapEvent= NULL;
	PX_ASSERT(NULL != &peninsulaData);
	const bool stressConditionPassed = (peninsulaData.getDataChunkCount() > 20);
	if(stressConditionPassed)
	{
		const bool breakAdjacentLinkedChunks = true;

		// set up data for proxy fracture events
		physx::Array<physx::PxU32> unbrokenAdjacentLinkedChunkIndices;
		const physx::PxU32 linkedChunkIndex = peninsulaData.getRootLinkedChunkIndex();
		PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
		if(breakAdjacentLinkedChunks)
		{
			getUnbrokenAdjacentLinkedChunkIndices(linkedChunkIndex, unbrokenAdjacentLinkedChunkIndices);
		}
		FractureEventProxy * fractureEventProxyBufferStart = NULL;
		const physx::PxU32 fractureEventProxyBufferCount = 1 + unbrokenAdjacentLinkedChunkIndices.size();
		fractureEventProxyBufferStart = PX_NEW(FractureEventProxy)[fractureEventProxyBufferCount];
		::memset(static_cast<void*>(fractureEventProxyBufferStart), 0x00, fractureEventProxyBufferCount * sizeof(FractureEventProxy));

		// set up data for primary weak link
		ActorChunkPair actorChunkPair				= getActorChunkPair(linkedChunkIndex);
		FractureEventProxy & fractureEventProxy		= fractureEventProxyBufferStart[0];
		fractureEventProxy.rootLinkedChunkIndex		= linkedChunkIndex;
		fractureEventProxy.chunkIndexInAsset		= actorChunkPair.chunkAlias->indexInAsset;
		fractureEventProxy.destructibleId			= actorChunkPair.actorAlias->getID();
		fractureEventProxy.fractureEventProxyFlags	= 0;
		fractureEventProxy.fractureEventProxyFlags	|= FractureEvent::Snap;
		fractureEventProxy.rootLinkedChunkPosition	= physx::PxVec3(0.0f);
		fractureEventProxy.impulse					= physx::PxVec3(0.0f);

		// set up data for secondary weak links
		if(breakAdjacentLinkedChunks)
		{
			for(physx::PxU32 index = 0; index < unbrokenAdjacentLinkedChunkIndices.size(); ++index)
			{
				ActorChunkPair currentActorChunkPair				= getActorChunkPair(unbrokenAdjacentLinkedChunkIndices[index]);
				FractureEventProxy & currentFractureEventProxy		= fractureEventProxyBufferStart[1 + index];
				currentFractureEventProxy.rootLinkedChunkIndex		= unbrokenAdjacentLinkedChunkIndices[index];
				currentFractureEventProxy.chunkIndexInAsset			= currentActorChunkPair.chunkAlias->indexInAsset;
				currentFractureEventProxy.destructibleId			= currentActorChunkPair.actorAlias->getID();
				currentFractureEventProxy.fractureEventProxyFlags	= 0;
				currentFractureEventProxy.fractureEventProxyFlags	|= FractureEvent::Snap;
				currentFractureEventProxy.rootLinkedChunkPosition	= physx::PxVec3(0.0f);
				currentFractureEventProxy.impulse					= physx::PxVec3(0.0f);
			}
		}

		// store them in a snap event
		const physx::PxF32 tickSecondsToSnap = peninsulaData.getDataChunkCount() > 40 ? 2.5f : 5.0f;
		PX_ASSERT(NULL != fractureEventProxyBufferStart && 0 != fractureEventProxyBufferCount);
		snapEvent = SnapEvent::instantiate(fractureEventProxyBufferStart, fractureEventProxyBufferCount, tickSecondsToSnap);
		PX_ASSERT(NULL != snapEvent);
		fractureEventProxyBufferStart = NULL;
	}
	return snapEvent;
}

SnapEvent * DestructibleStructureStressSolver::interpret(const PeninsulaData & peninsulaData, Int2Type<StressEvaluationType::EvaluateByMoment>) const
{
	SnapEvent * snapEvent= NULL;
	PX_ASSERT(NULL != &peninsulaData);
	const bool stressConditionPassed = peninsulaData.getDataAggregateMass() > userMassThreshold;
	if(stressConditionPassed)
	{
		// set up data for proxy fracture events
		const physx::PxU32 linkedChunkIndex = peninsulaData.getRootLinkedChunkIndex();
		PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
		FractureEventProxy * fractureEventProxyBufferStart = NULL;
		const physx::PxU32 fractureEventProxyBufferCount = 1;
		fractureEventProxyBufferStart = PX_NEW(FractureEventProxy)[fractureEventProxyBufferCount];
		::memset(static_cast<void*>(fractureEventProxyBufferStart), 0x00, fractureEventProxyBufferCount * sizeof(FractureEventProxy));

		// set up data for primary weak link
		ActorChunkPair actorChunkPair				= getActorChunkPair(linkedChunkIndex);
		FractureEventProxy & fractureEventProxy		= fractureEventProxyBufferStart[0];
		fractureEventProxy.rootLinkedChunkIndex		= linkedChunkIndex;
		fractureEventProxy.chunkIndexInAsset		= actorChunkPair.chunkAlias->indexInAsset;
		fractureEventProxy.destructibleId			= actorChunkPair.actorAlias->getID();
		fractureEventProxy.fractureEventProxyFlags	= 0;
		fractureEventProxy.fractureEventProxyFlags	|= FractureEvent::Snap;
		fractureEventProxy.rootLinkedChunkPosition	= actorChunkPair.actorAlias->getStructure()->getChunkWorldCentroid(*(actorChunkPair.chunkAlias));
		fractureEventProxy.impulse					= -1 * (peninsulaData.getDataGeometricCenter() - fractureEventProxy.rootLinkedChunkPosition).getNormalized();

		// store them in a snap event
		const physx::PxF32 tickSecondsToSnap = userTimeDelay;
		PX_ASSERT(NULL != fractureEventProxyBufferStart && 0 != fractureEventProxyBufferCount);
		snapEvent = SnapEvent::instantiate(fractureEventProxyBufferStart, fractureEventProxyBufferCount, tickSecondsToSnap);
		PX_ASSERT(NULL != snapEvent);
		fractureEventProxyBufferStart = NULL;
	}
	return snapEvent;
}

const FractureEvent & DestructibleStructureStressSolver::interpret(const FractureEventProxy & fractureEventProxy) const
{
	static FractureEvent staticFractureEvent;
	PX_ASSERT(NULL != &fractureEventProxy);
	::memset(static_cast<void*>(&staticFractureEvent), 0x00, 1 * sizeof(FractureEvent));
	staticFractureEvent.position			= fractureEventProxy.rootLinkedChunkPosition;
	staticFractureEvent.chunkIndexInAsset	= fractureEventProxy.chunkIndexInAsset;
	staticFractureEvent.impulse				= fractureEventProxy.impulse;
	staticFractureEvent.destructibleID		= fractureEventProxy.destructibleId;
	staticFractureEvent.flags				= fractureEventProxy.fractureEventProxyFlags;
	staticFractureEvent.hitDirection		= physx::PxVec3(0.0f);
	return staticFractureEvent;
}

bool DestructibleStructureStressSolver::isLinkedChunkBroken(physx::PxU32 linkedChunkIndex) const
{
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
	PX_ASSERT(NULL != structureLinkCondition);
	PX_ASSERT(structureLinkCondition[linkedChunkIndex] < DestructibleStructureStressSolver::LinkedChunkInvalidMaxFlag);
	PX_ASSERT(0 != (DestructibleStructureStressSolver::LinkedChunkExist & structureLinkCondition[linkedChunkIndex]));
	return (0 != (DestructibleStructureStressSolver::LinkedChunkBroken & structureLinkCondition[linkedChunkIndex]));
}

void DestructibleStructureStressSolver::setLinkedChunkBroken(physx::PxU32 linkedChunkIndex)
{
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
	PX_ASSERT(NULL != structureLinkCondition);
	PX_ASSERT(0 != (DestructibleStructureStressSolver::LinkedChunkExist & structureLinkCondition[linkedChunkIndex]));
	PX_ASSERT(0 == (DestructibleStructureStressSolver::LinkedChunkBroken & structureLinkCondition[linkedChunkIndex]) && "usage error!");
	structureLinkCondition[linkedChunkIndex] |= DestructibleStructureStressSolver::LinkedChunkBroken;
	PX_ASSERT(structureLinkCondition[linkedChunkIndex] < DestructibleStructureStressSolver::LinkedChunkInvalidMaxFlag);
}

bool DestructibleStructureStressSolver::isLinkedChunkStrained(physx::PxU32 linkedChunkIndex) const
{
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
	PX_ASSERT(NULL != structureLinkCondition);
	PX_ASSERT(structureLinkCondition[linkedChunkIndex] < DestructibleStructureStressSolver::LinkedChunkInvalidMaxFlag);
	PX_ASSERT(0 != (DestructibleStructureStressSolver::LinkedChunkExist & structureLinkCondition[linkedChunkIndex]));
	PX_ASSERT(!isLinkedChunkBroken(linkedChunkIndex) && "usage error!");
	return (0 != (DestructibleStructureStressSolver::LinkedChunkStrained & structureLinkCondition[linkedChunkIndex]));
}

void DestructibleStructureStressSolver::setLinkedChunkStrained(physx::PxU32 linkedChunkIndex)
{
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
	PX_ASSERT(NULL != structureLinkCondition);
	PX_ASSERT(0 != (DestructibleStructureStressSolver::LinkedChunkExist & structureLinkCondition[linkedChunkIndex]));
	PX_ASSERT(0 == (DestructibleStructureStressSolver::LinkedChunkStrained & structureLinkCondition[linkedChunkIndex]) && "usage error!");
	PX_ASSERT(!isLinkedChunkBroken(linkedChunkIndex) && "usage error!");
	structureLinkCondition[linkedChunkIndex] |= DestructibleStructureStressSolver::LinkedChunkStrained;
	PX_ASSERT(structureLinkCondition[linkedChunkIndex] < DestructibleStructureStressSolver::LinkedChunkInvalidMaxFlag);
}

void DestructibleStructureStressSolver::getUnbrokenAdjacentLinkedChunkIndices(physx::PxU32 linkedChunkIndex, physx::Array<physx::PxU32> & unbrokenAdjacentLinkedChunkIndices) const
{
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndex));
	PX_ASSERT(NULL != &unbrokenAdjacentLinkedChunkIndices);
	PX_ASSERT(unbrokenAdjacentLinkedChunkIndices.empty());
	const physx::PxU32 firstOverlapIndex	= bindedStructureAlias.firstOverlapIndices[linkedChunkIndex];
	const physx::PxU32 stopOverlapIndex		= bindedStructureAlias.firstOverlapIndices[linkedChunkIndex + 1];
	for(physx::PxU32 index = firstOverlapIndex; index < stopOverlapIndex; ++index)
	{
		const physx::PxU32 currentLinkedChunkIndex = bindedStructureAlias.overlaps[index];
		if(!isLinkedChunkBroken(currentLinkedChunkIndex))
		{
			unbrokenAdjacentLinkedChunkIndices.pushBack(currentLinkedChunkIndex);
		}
	}
}

bool DestructibleStructureStressSolver::areUnbrokenLinkedChunksAdjacent(physx::PxU32 linkedChunkIndexReference, physx::PxU32 linkedChunkIndexSubject) const
{
	bool areUnbrokenLinkedChunksAdjacent = false;
	PX_ASSERT(assertLinkedChunkIndexOk(linkedChunkIndexReference) && assertLinkedChunkIndexOk(linkedChunkIndexSubject));
	PX_ASSERT(!isLinkedChunkBroken(linkedChunkIndexReference) && !isLinkedChunkBroken(linkedChunkIndexSubject));
	const physx::PxU32 firstOverlapIndex	= bindedStructureAlias.firstOverlapIndices[linkedChunkIndexReference];
	const physx::PxU32 stopOverlapIndex		= bindedStructureAlias.firstOverlapIndices[linkedChunkIndexReference + 1];
	for(physx::PxU32 index = firstOverlapIndex; index < stopOverlapIndex; ++index)
	{
		const physx::PxU32 currentLinkedChunkIndex = bindedStructureAlias.overlaps[index];
		if(currentLinkedChunkIndex == linkedChunkIndexSubject)
		{
			PX_ASSERT(!isLinkedChunkBroken(currentLinkedChunkIndex));
			areUnbrokenLinkedChunksAdjacent = true;
			break;
		}
	}
	return areUnbrokenLinkedChunksAdjacent;
}

ActorChunkPair DestructibleStructureStressSolver::getActorChunkPair(physx::PxU32 chunkIndexInStructure) const
{
	const DestructibleActor * actor				= NULL;
	const DestructibleStructure::Chunk * chunk	= NULL;
	PX_ASSERT(chunkIndexInStructure < bindedStructureAlias.chunks.size());
	for(physx::Array<DestructibleActor*>::ConstIterator kIter = bindedStructureAlias.destructibles.begin(); kIter != bindedStructureAlias.destructibles.end(); ++kIter)
	{
		const DestructibleActor * currentDestructibleActor = *kIter;
		PX_ASSERT(NULL != currentDestructibleActor);
		if(chunkIndexInStructure >= currentDestructibleActor->getFirstChunkIndex() && chunkIndexInStructure < (currentDestructibleActor->getFirstChunkIndex() + currentDestructibleActor->getAsset()->getChunkCount()))
		{
			actor = currentDestructibleActor;
			chunk = &(bindedStructureAlias.chunks[chunkIndexInStructure]);
			PX_ASSERT((actor->getFirstChunkIndex() + chunk->indexInAsset) == chunkIndexInStructure);
			break;
		}
	}
	PX_ASSERT(NULL != actor && NULL != chunk);
	return ActorChunkPair(actor, chunk);
}

StructureScratchBuffer DestructibleStructureStressSolver::acquireScratchBuffer() const
{
	// bitwise const qualifier only - so as to allow const methods to use this method too
	PX_ASSERT(!scratchBufferLocked);
	scratchBufferLocked = true;
	return StructureScratchBuffer(scratchBuffer, structureLinkCount);
}

void DestructibleStructureStressSolver::relinquishScratchBuffer() const
{
	// bitwise const qualifier only - so as to allow const methods to use this method too
	PX_ASSERT(scratchBufferLocked);
	scratchBufferLocked = false;
}

DestructibleStructure & DestructibleStructureStressSolver::getBindedStructureMutable()
{
	return const_cast<DestructibleStructure&>(bindedStructureAlias);
}

bool DestructibleStructureStressSolver::assertLinkedChunkIndexOk(physx::PxU32 linkedChunkIndex) const
{
	PX_ASSERT(bindedStructureAlias.chunks.size() == structureLinkCount ? linkedChunkIndex < structureLinkCount : false);
	PX_UNUSED(linkedChunkIndex);
	return true;
}

bool DestructibleStructureStressSolver::assertLinkedChunkIndicesForEvaluationOk(const physx::Array<physx::PxU32> & linkedChunkIndicesForEvaluation) const
{
	for(physx::Array<physx::PxU32>::ConstIterator kIter = linkedChunkIndicesForEvaluation.begin(); kIter != linkedChunkIndicesForEvaluation.end(); ++kIter)
	{
		const physx::PxU32 currentLinkedChunkIndex = *kIter;
		PX_ASSERT(assertLinkedChunkIndexOk(currentLinkedChunkIndex));
		PX_ASSERT(!isLinkedChunkBroken(currentLinkedChunkIndex));
		PX_UNUSED(currentLinkedChunkIndex);
	}
	for(physx::Array<physx::PxU32>::ConstIterator kIter = linkedChunkIndicesForEvaluation.begin(); kIter != linkedChunkIndicesForEvaluation.end(); ++kIter)
	{
		const physx::PxU32 referenceLinkedChunkIndex = *kIter;
		for(physx::Array<physx::PxU32>::ConstIterator kJter = kIter + 1; kJter != linkedChunkIndicesForEvaluation.end(); ++kJter)
		{
			const physx::PxU32 subjectLinkedChunkIndex = *kJter;
			PX_ASSERT(referenceLinkedChunkIndex != subjectLinkedChunkIndex);
			PX_UNUSED(referenceLinkedChunkIndex);
			PX_UNUSED(subjectLinkedChunkIndex);
		}
	}
	return true;
}

} // namespace destructible
} // namespace apex
} // namespace physx