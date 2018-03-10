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
#include <DestructibleActor.h>
#include <NxRenderMeshAsset.h>
#include <DestructibleStructure.h>

#include "Actor.h"

#include "FracturePattern.h"
#include "SimScene.h"
#include "Compound.h"
#include "Mesh.h"

namespace physx
{
namespace fracture
{

Actor::Actor(base::SimScene* scene, DestructibleActor* actor):
	base::Actor(scene),
	mActor(actor),
	mRenderResourcesDirty(false),
	mMinRadius(0.f),
	mRadiusMultiplier(1.f),
	mImpulseScale(1.f),
	mSheetFracture(true)
{
	if(actor == NULL)
	{
		const PxU32 numSectors = 10;
		const PxF32 sectorRand = 0.3f;
		const PxF32 firstSegmentSize = 0.06f;
		const PxF32 segmentScale = 1.4f;
		const PxF32 segmentRand = 0.3f;
		const float size = 10.f;
		const PxF32 radius = size;
		const PxF32 thickness = size;
		mDefaultFracturePattern = (FracturePattern*)mScene->createFracturePattern();
		mDefaultFracturePattern->createGlass(radius,thickness,numSectors,sectorRand,firstSegmentSize,segmentScale,segmentRand);
	}
	else
	{
		// create fracture pattern
		const physx::apex::destructible::DestructibleActorParamNS::RuntimeFracture_Type& params = actor->getParams()->destructibleParameters.runtimeFracture;
		const PxU32 numSectors = params.glass.numSectors;
		const PxF32 sectorRand = params.glass.sectorRand;
		const PxF32 firstSegmentSize = params.glass.firstSegmentSize;
		const PxF32 segmentScale = params.glass.segmentScale;
		const PxF32 segmentRand = params.glass.segmentRand;
		const float size = actor->getBounds().getDimensions().maxElement();
		const PxF32 radius = size;
		const PxF32 thickness = size;
		mDefaultFracturePattern = (FracturePattern*)mScene->createFracturePattern();
		mDefaultFracturePattern->createGlass(radius,thickness,(physx::PxI32)numSectors,sectorRand,firstSegmentSize,segmentScale,segmentRand);
		// attachment
		mAttachmentFlags.posX = (physx::PxU32)params.attachment.posX;
		mAttachmentFlags.negX = (physx::PxU32)params.attachment.negX;
		mAttachmentFlags.posY = (physx::PxU32)params.attachment.posY;
		mAttachmentFlags.negY = (physx::PxU32)params.attachment.negY;
		mAttachmentFlags.posZ = (physx::PxU32)params.attachment.posZ;
		mAttachmentFlags.negZ = (physx::PxU32)params.attachment.negZ;
		// other
		mDepthLimit = params.depthLimit;
		mDestroyIfAtDepthLimit = params.destroyIfAtDepthLimit;
		mImpulseScale = params.impulseScale;
		mMinConvexSize = params.minConvexSize;
		mSheetFracture = params.sheetFracture;
	}
}

Actor::~Actor()
{
	PX_DELETE(mDefaultFracturePattern);
	mDefaultFracturePattern = NULL;
}

Compound* Actor::createCompound()
{
	base::Compound* c = mScene->createCompound((base::FracturePattern*)mDefaultFracturePattern);
	addCompound(c);
	return (Compound*)c;
}

Compound* Actor::createCompoundFromChunk(const apex::destructible::DestructibleActor& actor, PxU32 partIndex)
{
	Mesh RTmesh;
	RTmesh.loadFromRenderMesh(*actor.getAsset()->getRenderMeshAsset(),partIndex);

	// Fix texture v's (different between right hand and left hand)
	RTmesh.flipV();
	
	PxTransform pose(actor.getChunkPose(partIndex));
	pose.p = actor.getStructure()->getChunkWorldCentroid(actor.getChunk(partIndex));

	Compound* c = createCompound();

	c->createFromMesh(&RTmesh,pose,actor.getChunkLinearVelocity(partIndex),actor.getChunkAngularVelocity(partIndex),-1,actor.getScale());

	const DestructibleActorParamNS::BehaviorGroup_Type& behaviorGroup = actor.getBehaviorGroup(partIndex);
	mMinRadius = behaviorGroup.damageSpread.minimumRadius;
	mRadiusMultiplier = behaviorGroup.damageSpread.radiusMultiplier;

	// attachment
	//const DestructibleActorParamNS::RuntimeFracture_Type& params = mActor->getParams()->destructibleParameters.runtimeFracture;
	NxDestructibleParameters& params = mActor->getDestructibleParameters();
	mAttachmentFlags.posX |= params.rtFractureParameters.attachment.posX;
	mAttachmentFlags.negX |= params.rtFractureParameters.attachment.negX;
	mAttachmentFlags.posY |= params.rtFractureParameters.attachment.posY;
	mAttachmentFlags.negY |= params.rtFractureParameters.attachment.negY;
	mAttachmentFlags.posZ |= params.rtFractureParameters.attachment.posZ;
	mAttachmentFlags.negZ |= params.rtFractureParameters.attachment.negZ;

	attachBasedOnFlags(c);

	mRenderResourcesDirty = true;

	return c;
}

bool Actor::patternFracture(const PxVec3& hitLocation, const PxVec3& dirIn, float scale, float vel, float radiusIn)
{
	int compoundNr = -1;
	int convexNr = -1;
	PxVec3 normal;
	float dist;
	bool ret = false;
	PxVec3 dir = dirIn;

	mScene->getScene()->lockWrite();

	bool hit = false;
	if (dir.magnitudeSquared() < 0.5f)
	{
		dir = PxVec3(1.f,0.f,0.f);
		hit = base::Actor::rayCast(hitLocation-dir,dir,dist,compoundNr,convexNr,normal);
		if(!hit)
		{
			dir = PxVec3(0.f,1.f,0.f);
			hit = base::Actor::rayCast(hitLocation-dir,dir,dist,compoundNr,convexNr,normal);
			if(!hit)
			{
				dir = PxVec3(0.f,0.f,1.f);
				hit = base::Actor::rayCast(hitLocation-dir,dir,dist,compoundNr,convexNr,normal);
			}
		}
	}
	else
	{
		hit = base::Actor::rayCast(hitLocation-dir,dir,dist,compoundNr,convexNr,normal);
	}
	if (hit)
	{
		float radius = mMinRadius + mRadiusMultiplier*radiusIn;
		float impulseMagn = scale*vel*mImpulseScale;

		if (mSheetFracture)
		{
			normal = ((Compound*)mCompounds[(physx::PxU32)compoundNr])->mNormal;
		}
		PxVec3 a(0.f,0.f,1.f);
		normal.normalize();
		a -= a.dot(normal)*normal;
		if( a.magnitudeSquared() < 0.1f )
		{
			a = PxVec3(0.f,1.f,0.f);
			a -= a.dot(normal)*normal;
		}
		a.normalize();
		PxVec3 b(normal.cross(a));
		PxMat33 trans(a,b,normal);
		ret = base::Actor::patternFracture(hitLocation,dir,compoundNr,trans,radius,impulseMagn,impulseMagn);
	}

	mScene->getScene()->unlockWrite();

	mRenderResourcesDirty = true;

	return ret;
}

bool Actor::patternFracture(const DamageEvent& damageEvent)
{
	mDamageEvent.position = damageEvent.position;
	mDamageEvent.direction = damageEvent.direction;
	mDamageEvent.damage = damageEvent.damage;
	mDamageEvent.radius = damageEvent.radius;
	return patternFracture(damageEvent.position, damageEvent.direction, 1.f, damageEvent.damage, damageEvent.radius);
}

bool Actor::patternFracture(const FractureEvent& fractureEvent, bool fractureOnLoad)
{
	createCompoundFromChunk(*mActor,fractureEvent.chunkIndexInAsset);
	if(fractureOnLoad)
	{
		patternFracture(mDamageEvent.position, mDamageEvent.direction, 1.f, mDamageEvent.damage, mDamageEvent.radius);
		//patternFracture(fractureEvent.position,fractureEvent.hitDirection,1.f,fractureEvent.impulse.magnitude()/mActor->getChunkMass(fractureEvent.chunkIndexInAsset),1);
	}
	return true;
}

bool Actor::rayCast(const PxVec3& orig, const PxVec3& dir, float &dist) const
{
	int compoundNr = 0;
	int convexNr = 0;
	PxVec3 n;
	bool ret = false;

	mScene->getScene()->lockRead();
	ret = base::Actor::rayCast(orig,dir,dist,compoundNr,convexNr,n);
	mScene->getScene()->unlockRead();

	return ret;
}

void Actor::attachBasedOnFlags(base::Compound* c)
{
	PxBounds3 b; 
	c->getLocalBounds(b);
	// Determine sheet face
	if (mSheetFracture)
	{
		PxVec3 dim = b.getDimensions();
		if ( dim.x < dim.y && dim.x < dim.z )
		{
			((Compound*)c)->mNormal = PxVec3(1.f,0.f,0.f);
		}
		else if ( dim.y < dim.x && dim.y < dim.z )
		{
			((Compound*)c)->mNormal = PxVec3(0.f,1.f,0.f);
		}
		else
		{
			((Compound*)c)->mNormal = PxVec3(0.f,0.f,1.f);
		}
	}
	// Attach
	const float w = 0.01f*b.getDimensions().maxElement();
	shdfnd::Array<PxBounds3> bounds;
	bounds.reserve(6);
	if (mAttachmentFlags.posX)
	{
		PxBounds3 a(b);
		a.minimum.x = a.maximum.x-w;
		bounds.pushBack(a);
	}
	if (mAttachmentFlags.negX)
	{
		PxBounds3 a(b);
		a.maximum.x = a.minimum.x+w;
		bounds.pushBack(a);
	}
	if (mAttachmentFlags.posY)
	{
		PxBounds3 a(b);
		a.minimum.y = a.maximum.y-w;
		bounds.pushBack(a);
	}
	if (mAttachmentFlags.negY)
	{
		PxBounds3 a(b);
		a.maximum.y = a.minimum.y+w;
		bounds.pushBack(a);
	}
	if (mAttachmentFlags.posZ)
	{
		PxBounds3 a(b);
		a.minimum.z = a.maximum.z-w;
		bounds.pushBack(a);
	}
	if (mAttachmentFlags.negZ)
	{
		PxBounds3 a(b);
		a.maximum.z = a.minimum.z+w;
		bounds.pushBack(a);
	}
	c->attachLocal(bounds);
}

}
}
#endif
