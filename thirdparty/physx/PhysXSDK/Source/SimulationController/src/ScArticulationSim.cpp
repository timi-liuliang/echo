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


#include "ScArticulationSim.h"
#include "ScArticulationCore.h"
#include "ScArticulationJointSim.h"
#include "ScArticulationJointCore.h"
#include "ScBodySim.h"
#include "ScScene.h"
#include "ScInteractionScene.h"

#include "PxsArticulation.h"
#include "PxsContext.h"
#include "CmSpatialVector.h"

using namespace physx;

Sc::ArticulationSim::ArticulationSim(ArticulationCore& core, Scene& scene, BodyCore& root) : 
	mLLArticulation(NULL),
	mScene(scene),
	mCore(core),
	mLinks				(PX_DEBUG_EXP("ScArticulationSim::links")),
	mBodies				(PX_DEBUG_EXP("ScArticulationSim::bodies")),
	mJoints				(PX_DEBUG_EXP("ScArticulationSim::joints")),
	mInternalLoads		(PX_DEBUG_EXP("ScArticulationSim::internalLoads")),
	mExternalLoads		(PX_DEBUG_EXP("ScArticulationSim::externalLoads")),
	mPose				(PX_DEBUG_EXP("ScArticulationSim::poses")),
	mMotionVelocity		(PX_DEBUG_EXP("ScArticulationSim::motion velocity")),
	mFsDataBytes		(PX_DEBUG_EXP("ScArticulationSim::fsData")),
	mScratchMemory		(PX_DEBUG_EXP("ScArticulationSim::scratchMemory")),					
	mUpdateSolverData(true)
{
	PxsContext* llContext = mScene.getInteractionScene().getLowLevelContext();

	mLinks.reserve(16);
	mJoints.reserve(16);
	mBodies.reserve(16);

	mLLArticulation = llContext->createArticulation();

	if(!mLLArticulation)
	{
		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Articulation: could not allocate low-level resources.");
		return;
	}

	PX_ASSERT(root.getSim());

	addBody(*root.getSim(), NULL, NULL);

	mCore.setSim(this);

	mSolverData.core			= &core.getCore();
	mSolverData.internalLoads	= NULL;
	mSolverData.externalLoads	= NULL;
	mSolverData.fsData			= NULL;
	mSolverData.poses			= NULL;
	mSolverData.motionVelocity	= NULL;
	mSolverData.totalDataSize	= 0;
	mSolverData.solverDataSize	= 0;
	mSolverData.linkCount		= 0;
	mSolverData.scratchMemory	= NULL;
	mSolverData.scratchMemorySize = 0;
}


Sc::ArticulationSim::~ArticulationSim()
{
	if (!mLLArticulation)
		return;

	mScene.getInteractionScene().getLowLevelContext()->destroyArticulation(*mLLArticulation);

	mCore.setSim(NULL);
}

PxU32 Sc::ArticulationSim::findBodyIndex(BodySim& body) const
{
	for(PxU32 i=0; i<mBodies.size(); i++)
	{
		if(mBodies[i]==&body)
			return i;
	}
	PX_ASSERT(0);
	return 0x80000000;
}

void Sc::ArticulationSim::updateCachedTransforms(PxsTransformCache& cache, Cm::BitMap* shapeChangedMap)
{
	for(PxU32 i=0; i<mBodies.size(); i++)
	{
		mBodies[i]->updateCachedTransforms(cache);
		PxcBpHandle handle = mBodies[i]->getLowLevelBody().getAABBMgrId().mActorHandle;
		if(shapeChangedMap && handle != PX_INVALID_BP_HANDLE)
			shapeChangedMap->growAndSet(handle);
	}
}


void Sc::ArticulationSim::setIslandHandle(BodySim &body,
										  PxU32 index)
{
	PxsArticulationLinkHandle handle = reinterpret_cast<size_t>(mLLArticulation) | index;
	if(!isArticulationRootLink(handle))
	{
		body.getScene().getInteractionScene().getLLIslandManager().setArticulationLinkHandle(handle, this,
																								body.getLLIslandManagerNodeHook());
	}
	else
	{
		body.getScene().getInteractionScene().getLLIslandManager().setArticulationRootLinkHandle(handle, this,
																				  					body.getLLIslandManagerNodeHook());
	}
}

PxsArticulationLinkHandle Sc::ArticulationSim::getLinkHandle(BodySim &body) const
{
	return reinterpret_cast<size_t>(mLLArticulation) | findBodyIndex(body);
}

void Sc::ArticulationSim::addBody(BodySim& body, 
								  BodySim* parent, 
								  ArticulationJointSim* joint)
{
	mBodies.pushBack(&body);
	mJoints.pushBack(joint);

	PxU32 index = mLinks.size();

	PX_ASSERT((((index==0) && (joint == 0)) && (parent == 0)) ||
			  (((index!=0) && joint) && (parent && (parent->getArticulation() == this))));

	PxsArticulationLink &link = mLinks.insert();
	link.body = &body.getLowLevelBody();
	link.bodyCore = &body.getBodyCore().getCore();
	link.children = 0;
	bool shouldSleep;
	bool currentlyAsleep;
	bool bodyReadyForSleep = body.checkSleepReadinessBesidesWakeCounter();
	PxReal wakeCounter = getCore().getWakeCounter();

	if(parent)
	{
		currentlyAsleep = !mBodies[0]->isActive();
		shouldSleep = currentlyAsleep && bodyReadyForSleep;

		PxU32 parentIndex = findBodyIndex(*parent);
		link.parent = parentIndex;
		link.pathToRoot = mLinks[parentIndex].pathToRoot | PxcArticulationBitField(1)<<index;
		link.inboundJoint = &joint->getCore().getCore();
		mLinks[parentIndex].children |= PxcArticulationBitField(1)<<index;
	}
	else
	{
		currentlyAsleep = (wakeCounter == 0.0f);
		shouldSleep = currentlyAsleep && bodyReadyForSleep;

		link.parent = PXS_ARTICULATION_LINK_NONE;
		link.pathToRoot = 1;
		link.inboundJoint = NULL;
	}

	if (currentlyAsleep && (!shouldSleep))
	{
		for(PxU32 i=0; i < (mBodies.size() - 1); i++)
			mBodies[i]->internalWakeUpArticulationLink(wakeCounter);
	}

	body.setArticulation(this, wakeCounter, shouldSleep);
	setIslandHandle(body, index);

	mUpdateSolverData = true;

}


void Sc::ArticulationSim::removeBody(BodySim &body)
{
	PX_ASSERT(body.getArticulation() == this);
	PxU32 index = findBodyIndex(body);
	body.setArticulation(NULL, 0.0f, true);

	PxsArticulationLink &link0 = mLinks[index];

	PX_ASSERT(link0.children == 0);
	PX_UNUSED(link0);

	// copy all the later links down by one
	for(PxU32 i=index+1;i<mLinks.size();i++)
	{
		mLinks[i-1] = mLinks[i];
		mBodies[i-1] = mBodies[i];
		mJoints[i-1] = mJoints[i];
		setIslandHandle(*mBodies[i-1], i-1);
	}

	// adjust parent/child indices
	PxcArticulationBitField fixedIndices = (PxcArticulationBitField(1)<<index)-1;
	PxcArticulationBitField shiftIndices = ~(fixedIndices|(PxcArticulationBitField(1)<<index));

	for(PxU32 i=0;i<mLinks.size();i++)
	{
		PxsArticulationLink &link = mLinks[i];

		if(link.parent != PXS_ARTICULATION_LINK_NONE && link.parent>index)
			link.pathToRoot = (link.pathToRoot&fixedIndices) | (link.pathToRoot&shiftIndices)>>1;
		link.children = (link.children&fixedIndices) | (link.children&shiftIndices)>>1;
	}

	mLinks.popBack();

	mUpdateSolverData = true;
}


void Sc::ArticulationSim::checkResize() const
{
	if(!mBodies.size() || !mBodies[0]->getLLIslandManagerNodeHook().isManaged())
		return;

	// make sure the whole articulation is in.
	for(PxU32 i=0;i<mLinks.size();i++)
		PX_ASSERT(mBodies[i]->getLLIslandManagerNodeHook().isManaged());

	for(PxU32 i=1;i<mLinks.size();i++)
		PX_ASSERT(mJoints[i]->getLLIslandManagerEdgeHook().isManaged());

	if(!mUpdateSolverData)
		return;

	if(mLinks.size()!=mSolverData.linkCount)
	{
		PxU32 linkCount = mLinks.size();

		mMotionVelocity.resize(linkCount, Cm::SpatialVector(PxVec3(0.0f), PxVec3(0.0f)));
		mPose.resize(linkCount, PxTransform(PxIdentity));
		mExternalLoads.resize(linkCount, M33Identity());
		mInternalLoads.resize(linkCount, M33Identity());

		PxU32 solverDataSize, totalSize, scratchSize;
		PxcArticulationHelper::getDataSizes(linkCount, solverDataSize, totalSize, scratchSize);

		PX_ASSERT(mFsDataBytes.size()!=totalSize);
		PX_ASSERT(!(totalSize&15) && !(solverDataSize&15));
		mFsDataBytes.resize(totalSize);

		mSolverData.motionVelocity			= mMotionVelocity.begin();
		mSolverData.externalLoads			= mExternalLoads.begin();
		mSolverData.internalLoads			= mInternalLoads.begin();
		mSolverData.poses					= mPose.begin();
		mSolverData.solverDataSize			= Ps::to16(solverDataSize);
		mSolverData.totalDataSize			= Ps::to16(totalSize);
		mSolverData.fsData					= reinterpret_cast<PxcFsData *>(mFsDataBytes.begin());

		mScratchMemory.resize(scratchSize);
		mSolverData.scratchMemory			= mScratchMemory.begin();
		mSolverData.scratchMemorySize		= Ps::to16(scratchSize);
	}


	// something changed... e.g. a link deleted and one added - we need to change the warm start

	PxMemZero(mExternalLoads.begin(), sizeof(Mat33V) * mExternalLoads.size());
	PxMemZero(mInternalLoads.begin(), sizeof(Mat33V) * mExternalLoads.size());

	mSolverData.links			= mLinks.begin();
	mSolverData.linkCount		= Ps::to16(mLinks.size());

	mLLArticulation->setSolverDesc(mSolverData);

	mUpdateSolverData = false;
}

void Sc::ArticulationSim::sleepCheck(PxReal dt, PxReal invDt, bool /*enableStabilization*/)
{
	if(!mBodies.size())
		return;

#ifdef PX_CHECKED
	{
		PxReal maxTimer = 0.0f, minTimer = PX_MAX_F32;
		bool allActive = true, noneActive = true;
		for(PxU32 i=0;i<mLinks.size();i++)
		{
			PxReal timer = mBodies[i]->getBodyCore().getWakeCounter();
			maxTimer = PxMax(maxTimer, timer);
			minTimer = PxMin(minTimer, timer);
			bool active = mBodies[i]->isActive();
			allActive &= active;
			noneActive &= !active;
		}
		// either all links are asleep, or no links are asleep
		PX_ASSERT(maxTimer==0 || minTimer!=0);
		PX_ASSERT(allActive || noneActive);
	}

#endif

	if(!mBodies[0]->isActive())
		return;

	PxReal sleepThreshold = getCore().getCore().sleepThreshold;
	PxReal freezeThreshold = getCore().getCore().freezeThreshold;

	PxReal maxTimer = 0.0f, minTimer = PX_MAX_F32;

	for(PxU32 i=0;i<mLinks.size();i++)
	{
		bool notReadyForSleeping = false;
		PxReal timer = mBodies[i]->updateWakeCounter(dt, sleepThreshold, freezeThreshold, invDt, false, notReadyForSleeping);//enableStabilization);
		maxTimer = PxMax(maxTimer, timer);
		minTimer = PxMin(minTimer, timer);
		if(notReadyForSleeping)
		{
			mBodies[i]->notifyNotReadyForSleeping();
		}
	}

	mCore.setWakeCounterInternal(maxTimer);

	if(maxTimer != 0.0f)
	{
		if(minTimer == 0.0f)
		{
			// make sure nothing goes to sleep unless everything does
			for(PxU32 i=0;i<mLinks.size();i++)
				mBodies[i]->getBodyCore().setWakeCounterFromSim(PxMax(1e-6f, mBodies[i]->getBodyCore().getWakeCounter()));
		}
		return;
	}

	for(PxU32 i=0;i<mLinks.size();i++)
	{
		mBodies[i]->notifyReadyForSleeping();
		mBodies[i]->resetSleepFilter();
	}
}

bool Sc::ArticulationSim::isSleeping() const
{
	return (mBodies.size() > 0) ? (!mBodies[0]->isActive()) : true;
}

void Sc::ArticulationSim::internalWakeUp(PxReal wakeCounter)
{
	if(mCore.getWakeCounter() < wakeCounter)
	{
		mCore.setWakeCounterInternal(wakeCounter);
		for(PxU32 i=0;i<mLinks.size();i++)
			mBodies[i]->internalWakeUpArticulationLink(wakeCounter);
	}
}

void Sc::ArticulationSim::setActive(const bool b, const PxU32 infoFlag)
{
	for(PxU32 i=0;i<mBodies.size();i++)
	{
		if (i+1 < mBodies.size())
		{
			Ps::prefetchLine(mBodies[i+1],0);
			Ps::prefetchLine(mBodies[i+1],128);
		}
		mBodies[i]->setActive(b, infoFlag);
	}
}

void Sc::ArticulationSim::updateForces(PxReal dt, PxReal oneOverDt, bool updateGravity, const PxVec3& gravity, bool hasStaticTouch, bool simUsesAdaptiveForce)
{
	for(PxU32 i=0;i<mBodies.size();i++)
	{
		if (i+1 < mBodies.size())
		{
			Ps::prefetchLine(mBodies[i+1],128);
			Ps::prefetchLine(mBodies[i+1],256);
		}
		mBodies[i]->updateForces(dt, oneOverDt, updateGravity, gravity, hasStaticTouch, simUsesAdaptiveForce);
	}
}

void Sc::ArticulationSim::saveLastCCDTransform()
{
	for(PxU32 i=0;i<mBodies.size();i++)
	{
		if (i+1 < mBodies.size())
		{
			Ps::prefetchLine(mBodies[i+1],128);
			Ps::prefetchLine(mBodies[i+1],256);
		}
		mBodies[i]->getLowLevelBody().saveLastCCDTransform();
	}
}


Sc::ArticulationDriveCache* Sc::ArticulationSim::createDriveCache(PxReal compliance,
																  PxU32 driveIterations) const
{
	checkResize();
	PxU32 solverDataSize, totalSize, scratchSize;
	PxcArticulationHelper::getDataSizes(mLinks.size(), solverDataSize, totalSize, scratchSize);

	// In principle we should only need solverDataSize here. But right now prepareFsData generates the auxiliary data 
	// for use in potential debugging, which takes up extra space. 
	PxcFsData* data = reinterpret_cast<PxcFsData*>(PX_ALLOC(totalSize,"Articulation Drive Cache"));
	PxcArticulationHelper::initializeDriveCache(*data, Ps::to16(mLinks.size()), mLinks.begin(), compliance, driveIterations, mScratchMemory.begin(), mScratchMemory.size());
	return data;
}


void Sc::ArticulationSim::updateDriveCache(ArticulationDriveCache& cache,
										   PxReal compliance,
										   PxU32 driveIterations) const
{
	checkResize();
	PxcArticulationHelper::initializeDriveCache(cache,  Ps::to16(mLinks.size()), mLinks.begin(), compliance, driveIterations, mScratchMemory.begin(), mScratchMemory.size());
}


void Sc::ArticulationSim::releaseDriveCache(Sc::ArticulationDriveCache& driveCache) const
{
	PX_FREE(&driveCache);
}


void Sc::ArticulationSim::applyImpulse(Sc::BodyCore& link,
									   const Sc::ArticulationDriveCache& driveCache,
									   const PxVec3& force,
									   const PxVec3& torque)
{
	PxcSIMDSpatial v[PXC_ARTICULATION_MAX_SIZE], z[PXC_ARTICULATION_MAX_SIZE];
	PxMemZero(z, mLinks.size()*sizeof(Cm::SpatialVector));
	PxMemZero(v, mLinks.size()*sizeof(Cm::SpatialVector));

	PxU32 bodyIndex = findBodyIndex(*link.getSim());
	z[bodyIndex].linear = V3LoadU(-force);
	z[bodyIndex].angular = V3LoadU(-torque);

	PxcArticulationHelper::applyImpulses(driveCache, z, v);
	for(PxU32 i=0;i<mLinks.size();i++)
	{
		Sc::BodyCore& body = mBodies[i]->getBodyCore();
		PxVec3 lv, av;
		V3StoreU(v[i].linear, lv);
		V3StoreU(v[i].angular, av);

		body.setLinearVelocity(body.getLinearVelocity()+lv);
		body.setAngularVelocity(body.getAngularVelocity()+av);
	}
}

void Sc::ArticulationSim::computeImpulseResponse(Sc::BodyCore& link,
												  PxVec3& linearResponse, 
												  PxVec3& angularResponse,
												  const Sc::ArticulationDriveCache& driveCache,
												  const PxVec3& force,
												  const PxVec3& torque) const
{
	PxcSIMDSpatial v;
	PxcArticulationHelper::getImpulseResponse(driveCache, findBodyIndex(*link.getSim()), 
		PxcSIMDSpatial(V3LoadU(force), V3LoadU(torque)), v);
	V3StoreU(v.linear, linearResponse);
	V3StoreU(v.angular, angularResponse);
}

void Sc::ArticulationSim::debugCheckWakeCounterOfLinks(PxReal wakeCounter) const
{
	PX_UNUSED(wakeCounter);

#ifdef _DEBUG
	// make sure the links are in sync with the articulation
	for(PxU32 i=0; i < mBodies.size(); i++)
	{
		PX_ASSERT(mBodies[i]->getBodyCore().getWakeCounter() == wakeCounter);
	}
#endif
}

void Sc::ArticulationSim::debugCheckSleepStateOfLinks(bool isSleeping) const
{
	PX_UNUSED(isSleeping);

#ifdef _DEBUG
	// make sure the links are in sync with the articulation
	for(PxU32 i=0; i < mBodies.size(); i++)
	{
		if (isSleeping)
		{
			PX_ASSERT(!mBodies[i]->isActive());
			PX_ASSERT(mBodies[i]->getBodyCore().getWakeCounter() == 0.0f);
			PX_ASSERT(mBodies[i]->checkSleepReadinessBesidesWakeCounter());
		}
		else
			PX_ASSERT(mBodies[i]->isActive());
	}
#endif
}
