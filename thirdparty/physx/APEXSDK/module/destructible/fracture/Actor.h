/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef RT_ACTOR_H
#define RT_ACTOR_H

#include <PxFoundation.h>

#include "ActorBase.h"

namespace physx
{
namespace apex
{
namespace destructible
{
	class DestructibleActor;
	struct DamageEvent;
	struct FractureEvent;
}
}
using namespace apex::destructible;
namespace fracture
{

class Compound;
class FracturePattern;

class Actor : public base::Actor
{
	friend class SimScene;
	friend class Renderable;
protected:
	Actor(base::SimScene* scene, DestructibleActor* actor);
public:
	virtual ~Actor();

	Compound* createCompound();

	Compound* createCompoundFromChunk(const apex::destructible::DestructibleActor& destructibleActor, PxU32 partIndex);

	bool patternFracture(const PxVec3& hitLocation, const PxVec3& dir, float scale = 1.f, float vel = 0.f, float radius = 0.f);
	bool patternFracture(const DamageEvent& damageEvent);
	bool patternFracture(const FractureEvent& fractureEvent, bool fractureOnLoad = true);

	bool rayCast(const PxVec3& orig, const PxVec3& dir, float &dist) const;

	DestructibleActor* getDestructibleActor() {return mActor;}

protected:
	void attachBasedOnFlags(base::Compound* c);

	FracturePattern* mDefaultFracturePattern;
	DestructibleActor* mActor;
	bool mRenderResourcesDirty;

	PxF32 mMinRadius;
	PxF32 mRadiusMultiplier;
	PxF32 mImpulseScale;
	bool mSheetFracture;

	struct AttachmentFlags
	{
		AttachmentFlags() :
			posX(0), negX(0), posY(0), negY(0), posZ(0), negZ(0) {}

		PxU32 posX : 1;
		PxU32 negX : 1;
		PxU32 posY : 1;
		PxU32 negY : 1;
		PxU32 posZ : 1;
		PxU32 negZ : 1;
	}mAttachmentFlags;

	struct MyDamageEvent
	{
		PxVec3	position;
		PxVec3	direction;
		PxF32	damage;
		PxF32	radius;
	};
	MyDamageEvent mDamageEvent;

};

}
}

#endif
#endif