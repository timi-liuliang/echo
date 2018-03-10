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


#include "ScActorPair.h"
#include "ScScene.h"

using namespace physx;


void Sc::ActorPair::releaseContactReportData() const
{
	PX_ASSERT(mReportData);
	mScene.getNPhaseCore()->releaseActorPairContactReportData(mReportData);
	mReportData = NULL;
}


void Sc::ActorPair::createContactReportData() const
{
	PX_ASSERT(!mReportData);
	Sc::ActorPairContactReportData* reportData = mScene.getNPhaseCore()->createActorPairContactReportData(); 
	mReportData = reportData;

	if(reportData)
	{
		reportData->mActorAID = mActorA.getID();
		reportData->mActorBID = mActorB.getID();

		reportData->mPxActorA = mActorA.getPxActor();
		reportData->mPxActorB = mActorB.getPxActor();

		const ActorCore& actorCoreA = mActorA.getActorCore();
		const ActorCore& actorCoreB = mActorB.getActorCore();

		reportData->mActorAClientID = actorCoreA.getOwnerClient();
		reportData->mActorBClientID = actorCoreB.getOwnerClient();

		reportData->mActorAClientBehavior = actorCoreA.getClientBehaviorFlags();
		reportData->mActorBClientBehavior = actorCoreB.getClientBehaviorFlags();
	}
}
