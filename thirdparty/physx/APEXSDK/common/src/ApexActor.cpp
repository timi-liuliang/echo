/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "ApexActor.h"
#include "ApexContext.h"

#include "NiApexRenderDebug.h"
#include "NiApexScene.h"

#include "PsAtomic.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxScene.h"
#include "NxFromPx.h"
#endif

namespace physx
{
namespace apex
{

#if UNIQUE_ACTOR_ID
PxI32 ApexActor::mUniqueActorIdCounter = 0;
#endif

ApexActor::ApexActor() : mInRelease(false), mEnableDebugVisualization(true)
{
#if UNIQUE_ACTOR_ID
	mUniqueActorId = shdfnd::atomicIncrement(&mUniqueActorIdCounter);
#endif
}


ApexActor::~ApexActor()
{
	destroy();
}

void ApexActor::addSelfToContext(ApexContext& ctx, ApexActor* actorPtr)
{
	ContextTrack t;

	t.ctx = &ctx;
	t.index = ctx.addActor(*this, actorPtr);
	mContexts.pushBack(t);
}

void ApexActor::updateIndex(ApexContext& ctx, physx::PxU32 index)
{
	for (physx::PxU32 i = 0 ; i < mContexts.size() ; i++)
	{
		ContextTrack& t = mContexts[i];
		if (t.ctx == &ctx)
		{
			t.index = index;
			break;
		}
	}
}

bool ApexActor::findSelfInContext(ApexContext& ctx)
{
	for (physx::PxU32 i = 0 ; i < mContexts.size() ; i++)
	{
		ContextTrack& t = mContexts[i];
		if (t.ctx == &ctx)
		{
			return true;
		}
	}

	return false;
}

void ApexActor::destroy()
{
	mInRelease = true;

	renderDataLock();

	for (physx::PxU32 i = 0 ; i < mContexts.size() ; i++)
	{
		ContextTrack& t = mContexts[i];
		t.ctx->removeActorAtIndex(t.index);
	}
	mContexts.clear();
}



void ApexActor::visualizeLodBenefit(NiApexRenderDebug& renderDebug, NiApexScene& apexScene, const PxVec3& centroid, PxF32 radius, PxF32 absoluteBenefit, ActorState state)
{
#ifdef WITHOUT_DEBUG_VISUALIZE
	PX_UNUSED(renderDebug);
	PX_UNUSED(apexScene);
	PX_UNUSED(centroid);
	PX_UNUSED(radius);
	PX_UNUSED(absoluteBenefit);
	PX_UNUSED(state);
#else
	DebugColors::Enum color = DebugColors::Green;
	switch (state)
	{
	case StateEnabled:
		color = DebugColors::Green;
		break;
	case StateDisabled:
		color = DebugColors::Red;
		break;
	case StateEnabling:
		color = DebugColors::DarkGreen;
		break;
	case StateDisabling:
		color = DebugColors::DarkRed;
		break;
	}

	renderDebug.pushRenderState();
	renderDebug.setCurrentTextScale(radius / 3.0f);
	renderDebug.setCurrentColor(renderDebug.getDebugColor(color));

	PxVec3 upAxis(0.0f, 1.0f, 0.0f);
#if NX_SDK_VERSION_MAJOR == 2
	NxVec3 gravity;
	apexScene.getPhysXScene()->getGravity(gravity);
	upAxis = PxFromNxVec3Fast(-gravity);
	upAxis.normalize();
#endif

	PxMat44 cameraFacingPose((apexScene.getViewMatrix(0)).inverseRT());

	PxVec3 textLocation = centroid + upAxis * radius;
	cameraFacingPose.setPosition(textLocation);

	renderDebug.debugOrientedText(cameraFacingPose, "Benefit: %.3f", absoluteBenefit);

#if UNIQUE_ACTOR_ID
	textLocation = centroid + upAxis * radius * 1.5f;
	cameraFacingPose.setPosition(textLocation);

	renderDebug.debugOrientedText(cameraFacingPose, "Actor %d", mUniqueActorId);
#endif


	renderDebug.removeFromCurrentState(DebugRenderState::SolidShaded);
	renderDebug.debugSphere(centroid, radius);

	renderDebug.popRenderState();
#endif
}


}
} // end namespace physx::apex
