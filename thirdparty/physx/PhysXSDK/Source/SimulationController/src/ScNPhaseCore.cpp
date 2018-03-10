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


#include "ScNPhaseCore.h"
#include "ScShapeInstancePairLL.h"
#include "ScTriggerInteraction.h"
#include "ScElementInteractionMarker.h"
#include "ScElementActorInteraction.h"
#include "ScSimStats.h"
#include "ScObjectIDTracker.h"

#if PX_USE_PARTICLE_SYSTEM_API
#include "ScParticleSystemCore.h"
#include "ScParticleBodyInteraction.h"
#include "ScParticlePacketShape.h"
#include "GuOverlapTests.h"
#include "GuBox.h"
#endif

#if PX_USE_CLOTH_API
#include "ScClothCore.h"
#include "ScClothSim.h"
#endif // PX_USE_CLOTH_API

#include "PsThread.h"

using namespace physx;
using namespace Sc;
using namespace Gu;


struct Sc::FilterPair : public Cm::IndexedPoolEntry
{
public:
	enum Enum
	{
		ELEMENT_ELEMENT,
		ELEMENT_ACTOR,
		INVALID  // Make sure this is the last one
	};

	FilterPair() : IndexedPoolEntry(), elementElementRef(NULL), type(INVALID) {}

	PX_INLINE void setElementElementRef(Sc::ElementSimInteraction* ei)
	{
		type = ELEMENT_ELEMENT;
		elementElementRef = ei;
	}

	PX_INLINE void setElementActorRef(Sc::ActorElementPair* aep)
	{
		type = ELEMENT_ACTOR;
		elementActorRef = aep;
	}

	union  // Allow me this one
	{
		Sc::ElementSimInteraction*	elementElementRef;
		Sc::ActorElementPair* elementActorRef;
	};
	PxU8	type;
};


struct Sc::FilterInfo
{
	FilterInfo() : filterFlags(0), pairFlags(0), filterPair(NULL) {}

	PxFilterFlags	filterFlags;
	PxPairFlags		pairFlags;
	Sc::FilterPair* filterPair;
};

/* Sc::NPhaseCore methods */
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Sc::NPhaseCore::NPhaseCore(Scene& scene, const PxSceneDesc& sceneDesc) :
	mOwnerScene						(scene),
	mContactReportActorPairSet		(PX_DEBUG_EXP("contactReportPairSet")),
	mPersistentContactEventPairList	(PX_DEBUG_EXP("persistentContactEventPairs")),
	mNextFramePersistentContactEventPairIndex(0),
	mForceThresholdContactEventPairList	(PX_DEBUG_EXP("forceThresholdContactEventPairs")),
	mContactReportBuffer			(sceneDesc.contactReportStreamBufferSize, (sceneDesc.flags & PxSceneFlag::eDISABLE_CONTACT_REPORT_BUFFER_RESIZE)),  	
	mActorPairPool					(PX_DEBUG_EXP("actorPairPool")),
	mActorElementPairPool			(PX_DEBUG_EXP("actorElementPool")),
	mLLSipPool						(PX_DEBUG_EXP("llSIPPool")),
	mTriggerPool					(PX_DEBUG_EXP("triggerPool")),
	mActorPairContactReportDataPool	(PX_DEBUG_EXP("actorPairContactReportPool")),
	mInteractionMarkerPool			(PX_DEBUG_EXP("interactionMarkerPool"))
#if PX_USE_PARTICLE_SYSTEM_API
	,mParticleBodyPool				(PX_DEBUG_EXP("particleBodyPool"))
#endif
#if PX_USE_CLOTH_API
	,mClothPool						(PX_DEBUG_EXP("clothPool"))
#endif
{
}


Sc::NPhaseCore::~NPhaseCore()
{
	// Clear pending actor pairs (waiting on contact report callback)
	clearContactReportActorPairs(false);
}

PxU32 Sc::NPhaseCore::getDefaultContactReportStreamBufferSize() const
{
	return mContactReportBuffer.getDefaultBufferSize();
}

// PT: function used only once, so safe to force inline
static PX_FORCE_INLINE Sc::ElementInteraction* findInteraction(Element* _element0, Element* _element1)
{
	Element* elementLess;
	Element* elementMore;
	if (_element0->getElemInteractionCount() < _element1->getElemInteractionCount())
	{
		elementLess = _element0;
		elementMore = _element1;
	}
	else
	{
		elementLess = _element1;
		elementMore = _element0;
	}

	ElementInteraction* result = NULL;

	Element::ElementInteractionIterator iter = elementLess->getElemInteractions();
	ElementInteraction* interaction = iter.getNext();
	while(interaction)
	{
		if ((&interaction->getElement0() == elementMore) ||
			(&interaction->getElement1() == elementMore))
		{
			result = interaction;
			break;
		}

		interaction = iter.getNext();
	}

	return result;
}

void Sc::NPhaseCore::onOverlapCreated(Element* volume0, Element* volume1, const PxU32 ccdPass)
{
#if PX_USE_PARTICLE_SYSTEM_API
	PX_COMPILE_TIME_ASSERT(PX_ELEMENT_TYPE_SHAPE < PX_ELEMENT_TYPE_PARTICLE_PACKET);
#else
	PX_UNUSED(ccdPass);
#endif

	PX_ASSERT(!findInteraction(volume0, volume1));

	Element* volumeLo = volume0;
	Element* volumeHi = volume1;

	// PT: we already order things here? So why do we also have a swap in contact managers?
	// PT: TODO: use a map here again
	if (volumeLo->getElementType() > volumeHi->getElementType())
	{
		volumeLo = volume1;
		volumeHi = volume0;
	}

	switch (volumeHi->getElementType())
	{

#if PX_USE_PARTICLE_SYSTEM_API
		case PX_ELEMENT_TYPE_PARTICLE_PACKET:
			{
				ParticlePacketShape* shapeHi = static_cast<ParticlePacketShape*>(volumeHi);

				if (volumeLo->getElementType() != PX_ELEMENT_TYPE_SHAPE)
					break;	// Only interactions with rigid body shapes are supported

				ShapeSim* shapeLo = static_cast<ShapeSim*>(volumeLo);

				if (shapeLo->getScActor().isDynamicRigid()
					&& !(shapeHi->getParticleSystem().getCore().getFlags() & PxParticleBaseFlag::eCOLLISION_WITH_DYNAMIC_ACTORS))
					break;	// Skip dynamic rigids if corresponding flag is set on the particle system

				{
#ifndef PX_PS3	
					const PxGeometryType::Enum geoType = shapeLo->getGeometryType();
					if (geoType == PxGeometryType::eTRIANGLEMESH || geoType == PxGeometryType::eHEIGHTFIELD)
					{
						PxBounds3 particleShapeBounds;
						shapeHi->computeWorldBounds(particleShapeBounds);
						bool isIntersecting = false;
						switch (geoType)
						{
							case PxGeometryType::eTRIANGLEMESH:
								{
								const PxTriangleMeshGeometry& meshGeom = static_cast<const PxTriangleMeshGeometry&>(shapeLo->getCore().getGeometry());
								const Gu::Box worldOBB(particleShapeBounds.getCenter(), particleShapeBounds.getExtents(), PxMat33(PxIdentity));
								isIntersecting = Gu::checkOverlapOBB_triangleGeom(meshGeom, shapeLo->getAbsPose(), worldOBB);
								}
								break;
							case PxGeometryType::eHEIGHTFIELD:
								{
								const PxHeightFieldGeometry& hfGeom = static_cast<const PxHeightFieldGeometry&>(shapeLo->getCore().getGeometry());
								//Gu::Sphere sphere(particleShapeBounds.getCenter(), particleShapeBounds.getExtents().magnitude());	
								//isIntersecting = Gu::checkOverlapSphere_heightFieldGeom(hfGeom, shapeLo->getAbsPose(), sphere); //sschirm: doesn't work properly.
								const Gu::Box worldOBB(particleShapeBounds.getCenter(), particleShapeBounds.getExtents(), PxMat33(PxIdentity));
								isIntersecting = Gu::checkOverlapOBB_heightFieldGeom(hfGeom, shapeLo->getAbsPose(), worldOBB); 
								}
								break;
							case PxGeometryType::eSPHERE:
							case PxGeometryType::ePLANE:
							case PxGeometryType::eCAPSULE:
							case PxGeometryType::eBOX:
							case PxGeometryType::eCONVEXMESH:
							case PxGeometryType::eGEOMETRY_COUNT:
							case PxGeometryType::eINVALID:
							default:
								break;
						}

						if (!isIntersecting)
							break;
					}
#endif
					createParticlePacketBodyInteraction(*shapeHi, *shapeLo, ccdPass);
				}
			}
			break;
#endif	// PX_USE_PARTICLE_SYSTEM_API

#if PX_USE_CLOTH_API
        case PX_ELEMENT_TYPE_CLOTH:
            {
                if (volumeLo->getElementType() != PX_ELEMENT_TYPE_SHAPE)
                    break;	// Only interactions with rigid body shapes are supported

				ClothShape* shapeHi = static_cast<ClothShape*>(volumeHi);
				ClothSim& clothSim = shapeHi->getClothSim();
                ClothCore& clothCore = clothSim.getCore();

				if(~clothCore.getClothFlags() & PxClothFlag::eSCENE_COLLISION)
					break;

				ShapeSim* shapeLo = static_cast<ShapeSim*>(volumeLo);

				FilterInfo finfo = runFilter(*shapeHi, *shapeLo, NULL);
				if (finfo.filterFlags & (PxFilterFlag::eKILL | PxFilterFlag::eSUPPRESS))  // those are treated the same for cloth
				{
					PX_ASSERT(finfo.filterPair == NULL);  // No filter callback pair info for killed pairs
					break;
				}

				if(clothSim.addCollisionShape(shapeLo))
				{
					// only add an element when the collision shape wasn't rejected
					// due to hitting the sphere/plane limit of the cloth.
					ClothListElement element(&clothSim, mClothOverlaps[shapeLo].mNext);
					mClothOverlaps[shapeLo].mNext = mClothPool.construct(element);
				}
            }
            break;
#endif // PX_USE_CLOTH_API

		case PX_ELEMENT_TYPE_SHAPE:
			{
				ShapeSim* shapeHi = static_cast<ShapeSim*>(volumeHi);
				ShapeSim* shapeLo = static_cast<ShapeSim*>(volumeLo);

				// No actor internal interactions
				PX_ASSERT(&shapeHi->getScActor() != &shapeLo->getScActor());

				createRbElementInteraction(*shapeHi, *shapeLo);
			}
			break;
		case PX_ELEMENT_TYPE_COUNT:
		default:
			PX_ASSERT(0);
			break;
	}
}


static PX_FORCE_INLINE void prefetchPairElements(const PxvBroadPhaseOverlap& pair, const Element** elementBuffer)
{
	const Element* e0 = (const Element*)pair.userdata0;
	const Element* e1 = (const Element*)pair.userdata1;

	Ps::prefetchLine(e0);
	Ps::prefetchLine(e1);

	*elementBuffer = e0;
	*(elementBuffer+1) = e1;
}


static PX_FORCE_INLINE void prefetchPairActors(const Element& e0, const Element& e1, const ActorSim** actorBuffer)
{
	const ActorSim* a0 = static_cast<const ActorSim*>(&e0.getScActor());
	const ActorSim* a1 = static_cast<const ActorSim*>(&e1.getScActor());

	Ps::prefetchLine(a0);
	Ps::prefetchLine(((PxU8*)a0) + 128);
	Ps::prefetchLine(a1);
	Ps::prefetchLine(((PxU8*)a1) + 128);

	*actorBuffer = a0;
	*(actorBuffer+1) = a1;
}


static PX_FORCE_INLINE void prefetchPairShapesCore(const Element& e0, const Element& e1)
{
	if (e0.getElementType() == PX_ELEMENT_TYPE_SHAPE)
	{
		const ShapeCore* sc = &static_cast<const ShapeSim&>(e0).getCore();
		Ps::prefetchLine(sc);
		Ps::prefetchLine(((PxU8*)sc) + 128);
	}
	if (e1.getElementType() == PX_ELEMENT_TYPE_SHAPE)
	{
		const ShapeCore* sc = &static_cast<const ShapeSim&>(e1).getCore();
		Ps::prefetchLine(sc);
		Ps::prefetchLine(((PxU8*)sc) + 128);
	}
}


static PX_FORCE_INLINE void prefetchPairActorsCore(const ActorSim& a0, const ActorSim& a1)
{
	ActorCore* ac0 = &a0.getActorCore();
	Ps::prefetchLine(ac0);
	Ps::prefetchLine(((PxU8*)ac0) + 128);
	ActorCore* ac1 = &a1.getActorCore();
	Ps::prefetchLine(ac1);
	Ps::prefetchLine(((PxU8*)ac1) + 128);
}


static PX_FORCE_INLINE void processElementPair(Sc::NPhaseCore& nPhaseCore, const PxvBroadPhaseOverlap& pair, const PxU32 ccdPass)
{
	Element* e0 = (Element*)pair.userdata0;
	Element* e1 = (Element*)pair.userdata1;

	nPhaseCore.onOverlapCreated(e0, e1, ccdPass);
}


void Sc::NPhaseCore::onOverlapCreated(const PxvBroadPhaseOverlap* PX_RESTRICT pairs, PxU32 pairCount, const PxU32 ccdPass)
{
#if PX_USE_PARTICLE_SYSTEM_API
	PX_COMPILE_TIME_ASSERT(PX_ELEMENT_TYPE_SHAPE < PX_ELEMENT_TYPE_PARTICLE_PACKET);
#endif

	const PxU32 prefetchLookAhead = 4;
	const Element* batchedElements[prefetchLookAhead * 2];
	const ActorSim* batchedActors[prefetchLookAhead * 2];
	
	PxU32 batchIterCount = pairCount / prefetchLookAhead;
	PxU32 pairIdx = 0;

	for(PxU32 i=0; i < batchIterCount; i++)
	{
		// prefetch elements for next batch
		if (i < (batchIterCount-1))
		{
			PX_ASSERT(prefetchLookAhead >= 4);
			prefetchPairElements(pairs[pairIdx + prefetchLookAhead], batchedElements);
			prefetchPairElements(pairs[pairIdx + prefetchLookAhead + 1], batchedElements + 2);
			prefetchPairElements(pairs[pairIdx + prefetchLookAhead + 2], batchedElements + 4);
			prefetchPairElements(pairs[pairIdx + prefetchLookAhead + 3], batchedElements + 6);
			// unrolling by hand leads to better perf on XBox
		}
		else
		{
			PxU32 batchedElementIdx = 0;
			for(PxU32 k=(pairIdx + prefetchLookAhead); k < pairCount; k++)
			{
				prefetchPairElements(pairs[k], batchedElements + batchedElementIdx);
				batchedElementIdx += 2;
			}
		}

		processElementPair(*this, pairs[pairIdx + 0], ccdPass);

		// prefetch actor sim for next batch
		if (i < (batchIterCount-1))
		{
			PX_ASSERT(prefetchLookAhead >= 4);
			prefetchPairActors(*batchedElements[0], *batchedElements[1], batchedActors);
			prefetchPairActors(*batchedElements[2], *batchedElements[3], batchedActors + 2);
			prefetchPairActors(*batchedElements[4], *batchedElements[5], batchedActors + 4);
			prefetchPairActors(*batchedElements[6], *batchedElements[7], batchedActors + 6);
			// unrolling by hand leads to better perf on XBox
		}
		else
		{
			PxU32 batchedElementIdx = 0;
			for(PxU32 k=(pairIdx + prefetchLookAhead); k < pairCount; k++)
			{
				prefetchPairActors(*batchedElements[batchedElementIdx], *batchedElements[batchedElementIdx+1], batchedActors + batchedElementIdx);
				batchedElementIdx += 2;
			}
		}

		processElementPair(*this, pairs[pairIdx + 1], ccdPass);

		// prefetch shape core for next batch
		if (i < (batchIterCount-1))
		{
			PX_ASSERT(prefetchLookAhead >= 4);
			prefetchPairShapesCore(*batchedElements[0], *batchedElements[1]);
			prefetchPairShapesCore(*batchedElements[2], *batchedElements[3]);
			prefetchPairShapesCore(*batchedElements[4], *batchedElements[5]);
			prefetchPairShapesCore(*batchedElements[6], *batchedElements[7]);
			// unrolling by hand leads to better perf on XBox
		}
		else
		{
			PxU32 batchedElementIdx = 0;
			for(PxU32 k=(pairIdx + prefetchLookAhead); k < pairCount; k++)
			{
				prefetchPairShapesCore(*batchedElements[batchedElementIdx], *batchedElements[batchedElementIdx+1]);
				batchedElementIdx += 2;
			}
		}

		processElementPair(*this, pairs[pairIdx + 2], ccdPass);

		// prefetch actor core for next batch
		if (i < (batchIterCount-1))
		{
			PX_ASSERT(prefetchLookAhead >= 4);
			prefetchPairActorsCore(*batchedActors[0], *batchedActors[1]);
			prefetchPairActorsCore(*batchedActors[2], *batchedActors[3]);
			prefetchPairActorsCore(*batchedActors[4], *batchedActors[5]);
			prefetchPairActorsCore(*batchedActors[6], *batchedActors[7]);
			// unrolling by hand leads to better perf on XBox
		}
		else
		{
			PxU32 batchedActorIdx = 0;
			for(PxU32 k=(pairIdx + prefetchLookAhead); k < pairCount; k++)
			{
				prefetchPairActorsCore(*batchedActors[batchedActorIdx], *batchedActors[batchedActorIdx+1]);
				batchedActorIdx += 2;
			}
		}

		processElementPair(*this, pairs[pairIdx + 3], ccdPass);

		pairIdx += prefetchLookAhead;
	}

	// process remaining pairs
	for(PxU32 i=pairIdx; i < pairCount; i++)
	{
		processElementPair(*this, pairs[i], ccdPass);
	}
}


void Sc::NPhaseCore::onOverlapRemoved(Element* volume0, Element* volume1, const PxU32 ccdPass)
{
	// PT: ordering them here is again useless, as "findInteraction" will reorder according to counts...

	Element* elementHi = volume1;
	Element* elementLo = volume0;
	// No actor internal interactions
	PX_ASSERT(&elementHi->getScActor() != &elementLo->getScActor());

	ElementInteraction* interaction = findInteraction(elementHi, elementLo);
	// MS: The check below is necessary since at the moment LowLevel broadphase still tracks 
	//     killed pairs and hence reports lost overlaps
	if (interaction)
	{
		PxU32 flags = (PxU32)PairReleaseFlag::eWAKE_ON_LOST_TOUCH;
		PX_ASSERT(CoreInteraction::isCoreInteraction(interaction) && CoreInteraction::isCoreInteraction(interaction)->isElementInteraction());
		releaseElementPair(static_cast<ElementSimInteraction*>(interaction), flags, ccdPass, true);
	}

#if PX_USE_CLOTH_API
    // Cloth doesn't use interactions
    if (elementLo->getElementType() == PX_ELEMENT_TYPE_CLOTH)
        swap(elementLo, elementHi);
    if (elementHi->getElementType() == PX_ELEMENT_TYPE_CLOTH &&
        elementLo->getElementType() == PX_ELEMENT_TYPE_SHAPE)
    {
        ShapeSim* shapeLo = static_cast<ShapeSim*>(elementLo);
        ClothShape* shapeHi = static_cast<ClothShape*>(elementHi);
		ClothSim& clothSim = shapeHi->getClothSim();

		clothSim.removeCollisionShape(shapeLo);
		removeClothOverlap(&clothSim, shapeLo);
    }
#endif // PX_USE_CLOTH_API
}


// MS: TODO: optimize this for the actor release case?
void Sc::NPhaseCore::onVolumeRemoved(Element* volume, PxU32 flags)
{
	const PxU32 ccdPass = 0;

	switch (volume->getElementType())
	{
		case PX_ELEMENT_TYPE_SHAPE:
			{
				flags |= PairReleaseFlag::eSHAPE_BP_VOLUME_REMOVED;

				// Release interactions
				// IMPORTANT: Iterate from the back of the list to the front as we release interactions which
				//            triggers a replace with last
				Element::ElementInteractionReverseIterator iter = volume->getElemInteractionsReverse();
				ElementInteraction* interaction = iter.getNext();
				while(interaction)
				{
#if PX_USE_PARTICLE_SYSTEM_API
                    PX_ASSERT(	(interaction->getType() == PX_INTERACTION_TYPE_MARKER) ||
						        (interaction->getType() == PX_INTERACTION_TYPE_OVERLAP) ||
						        (interaction->getType() == PX_INTERACTION_TYPE_TRIGGER) ||
						        (interaction->getType() == PX_INTERACTION_TYPE_PARTICLE_BODY) );
#else
			        PX_ASSERT(	(interaction->getType() == PX_INTERACTION_TYPE_MARKER) ||
						        (interaction->getType() == PX_INTERACTION_TYPE_OVERLAP) ||
						        (interaction->getType() == PX_INTERACTION_TYPE_TRIGGER) );
#endif
					
					releaseElementPair((ElementSimInteraction*)interaction, flags, ccdPass, true);

					interaction = iter.getNext();
				}

#if PX_USE_CLOTH_API
				ShapeSim* shape = static_cast<ShapeSim*>(volume);
				if(const Ps::HashMap<const ShapeSim*, ClothListElement>::Entry* entry = mClothOverlaps.find(shape))
				{
					for(ClothListElement* it = entry->second.mNext; it;)
					{
						it->mClothSim->removeCollisionShape(shape);
						ClothListElement* next = it->mNext;
						mClothPool.deallocate(it);
						it = next;
					}
					mClothOverlaps.erase(shape);
				}
#endif

				break;
			}
#if PX_USE_PARTICLE_SYSTEM_API
		case PX_ELEMENT_TYPE_PARTICLE_PACKET:
			{
				flags |= PairReleaseFlag::eBP_VOLUME_REMOVED;

				// Release interactions
				ParticlePacketShape* ps = static_cast<ParticlePacketShape*>(volume);
				Cm::Range<ParticleElementRbElementInteraction*const> interactions = ps->getPacketShapeInteractions();
				for (; !interactions.empty(); interactions.popBack())
				{
					ParticleElementRbElementInteraction*const interaction = interactions.back();
					PX_ASSERT((*interaction).getType() == PX_INTERACTION_TYPE_PARTICLE_BODY);
					// The ccdPass parameter is needed to avoid concurrent interaction updates while the gpu particle pipeline is running.
					releaseElementPair((ElementSimInteraction*)interaction, flags, ccdPass, true);
				}
				break;
			}
#endif	// PX_USE_PARTICLE_SYSTEM_API
#if PX_USE_CLOTH_API
        case PX_ELEMENT_TYPE_CLOTH:
            {
                // nothing to do
            }
            break;
#endif // PX_USE_CLOTH_API
		case PX_ELEMENT_TYPE_COUNT:
		default:
			{
				PX_ASSERT(0);
				break;
			}
	}
}

#if PX_USE_CLOTH_API
void Sc::NPhaseCore::removeClothOverlap(ClothSim* clothSim, const ShapeSim* shapeSim)
{
	// When a new overlap was rejected due to the sphere/plane limit, 
	// the entry to delete does not exist. 
	for(ClothListElement* it = &mClothOverlaps[shapeSim]; 
		ClothListElement* next = it->mNext; it = next)
	{
		if(clothSim == next->mClothSim)
		{
			it->mNext = next->mNext;
			mClothPool.deallocate(next);
			break;
		}
	}
}
#endif

Sc::ElementSimInteraction* Sc::NPhaseCore::createRbElementInteraction(ShapeSim& s0, ShapeSim& s1)
{
	FilterInfo finfo = filterRbCollisionPair(s0, s1, NULL);

	if (finfo.filterFlags & PxFilterFlag::eKILL)
	{
		PX_ASSERT(finfo.filterPair == NULL);  // No filter callback pair info for killed pairs
		return NULL;
	}

	if(!testInteractionCounts(s0.getScActor(), s1.getScActor()))
		return NULL;

	ElementSimInteraction* pair = NULL;

	if ((finfo.filterFlags & PxFilterFlag::eSUPPRESS) == false)
	{
		if (!(s0.getFlags() & PxShapeFlag::eTRIGGER_SHAPE || s1.getFlags() & PxShapeFlag::eTRIGGER_SHAPE))
		{
			pair = createShapeInstancePairLL(s0, s1, finfo.pairFlags);
		}
		else
		{
			TriggerInteraction* ti = createTriggerInteraction(s0, s1, finfo.pairFlags);
			pair = ti;
		}
	}
	else
	{
		pair = createElementInteractionMarker(s0, s1);
	}

	if (finfo.filterPair)
	{
		// Mark the pair as a filter callback pair
		pair->raiseCoreFlag(CoreInteraction::IS_FILTER_PAIR);

		// Filter callback pair: Set the link to the interaction
		finfo.filterPair->setElementElementRef(pair);
	}

	return pair;
}


#if PX_USE_PARTICLE_SYSTEM_API

// PT: function used only once, so safe to force inline
static PX_FORCE_INLINE Sc::ElementActorInteraction* findParticlePacketBodyInteraction(ParticlePacketShape* ps, Actor* actor)
{
	ParticleElementRbElementInteraction** interactions = ps->getInteractions();
	PxU32 count = ps->getInteractionsCount();

	while(count--)
	{
		ParticleElementRbElementInteraction*const interaction = *interactions++;

		PX_ASSERT(	(interaction->getActor0().getActorType() == PxActorType::ePARTICLE_SYSTEM) ||
					(interaction->getActor0().getActorType() == PxActorType::ePARTICLE_FLUID) );

		if ((&interaction->getActor1() == actor) && (&interaction->getParticleShape() == ps))
		{
			PX_ASSERT(interaction->getInteractionFlags() & PX_INTERACTION_FLAG_ELEMENT_ACTOR);
			return static_cast<ElementActorInteraction*>(interaction);
		}
	}
	return NULL;
}

Sc::ElementSimInteraction* Sc::NPhaseCore::createParticlePacketBodyInteraction(ParticlePacketShape& ps, ShapeSim& s, const PxU32 ccdPass)
{
	ActorElementPair* actorElementPair = NULL;

	Sc::Actor& scActor = s.getScActor();
	Sc::ActorSim& scActorSim = ps.getActorSim();

	ElementActorInteraction* eai = findParticlePacketBodyInteraction(&ps, &scActor);
	if (eai)
	{
		// PT: this test is for the 'insertParticleElementRbElementPair' below
		if(!testInteractionCounts(scActorSim, scActor))
			return NULL;

		// There already is an interaction between the shape and the particleSystem/...
		// In that case, fetch the filter information from the existing interaction.

		actorElementPair = eai->getActorElementPair();
		PX_ASSERT(actorElementPair);
	}
	else
	{
		FilterInfo finfo = runFilter(ps, s, NULL);

		// Note: It is valid to check here for killed pairs and not in the if-section above since in the
		//       case of killed pairs you won't ever get in the if-section.
		if (finfo.filterFlags & PxFilterFlag::eKILL)
		{
			PX_ASSERT(finfo.filterPair == NULL);  // No filter callback pair info for killed pairs
			return NULL;
		}

		// PT: this test is for the 'insertParticleElementRbElementPair' below
		// We do it after the filtering in this branch
		if(!testInteractionCounts(scActorSim, scActor))
			return NULL;

		const bool isSuppressed = finfo.filterFlags & PxFilterFlag::eSUPPRESS;
		actorElementPair = mActorElementPairPool.construct(scActorSim, s, finfo.pairFlags);

		actorElementPair->markAsSuppressed(isSuppressed);
		actorElementPair->markAsFilterPair(finfo.filterPair != NULL);

		if (finfo.filterPair)
			finfo.filterPair->setElementActorRef(actorElementPair);  // New filter callback pair: Set the link to the interaction
	}
	
	ElementSimInteraction* pair = insertParticleElementRbElementPair(ps, s, actorElementPair, ccdPass);
	if(actorElementPair->isFilterPair())
		pair->raiseCoreFlag(CoreInteraction::IS_FILTER_PAIR);  // Mark the pair as a filter callback pair

	return pair;
}
#endif


Sc::ShapeInstancePairLL* Sc::NPhaseCore::createShapeInstancePairLL(ShapeSim& s0, ShapeSim& s1, PxPairFlags pairFlags)
{
	ShapeSim* _s0 = &s0;
	ShapeSim* _s1 = &s1;

	{
		// PT: 'getRbSim()' is not inlined so call it only once here.
		Sc::RigidSim& rs0 = s0.getRbSim();
		Sc::RigidSim& rs1 = s1.getRbSim();

		/*
		This guarantees that if one of the bodies is static or kinematic, it will be body B
		There is a further optimization to force all pairs that share the same bodies to have 
		the same body ordering.  This reduces the number of required partitions in the parallel solver.
		*/
		if(rs0.getBroadphaseGroupId() < rs1.getBroadphaseGroupId() 
			|| rs0.getActorType() == PxActorType::eRIGID_STATIC)
			Ps::swap(_s0, _s1);
	}

	ActorPair* aPair = findActorPair(_s0, _s1);
	PX_ASSERT(aPair != NULL);
	ShapeInstancePairLL* sipLL = mLLSipPool.construct(*_s0, *_s1, *aPair, pairFlags);

	//KS - TODO - figure out if we need this - it can't be done right now
	//if (secondaryBroadphase && (pairFlags & (PxPairFlag::eCCD_LINEAR)))
	//{
	//	// Both actors should be active if one is fast moving and CCD is enabled for the pair.
	//	// This ensures that the LL pair gets created.
	//	BodySim* b0 = s0.getBodySim();
	//	BodySim* b1 = s1.getBodySim();
	//	bool fastDynamic0 = (b0 && s0.getLowLevelBodyShape().isMovingFast());
	//	bool fastDynamic1 = (b1 && s1.getLowLevelBodyShape().isMovingFast());
	//	if (fastDynamic0 || fastDynamic1)
	//	{
	//		if (b0 && !b0->isActive())
	//		{
	//			b0->setActive(true);
	//		}
	//		else if (b1 && !b1->isActive())
	//		{
	//			b1->setActive(true);
	//		}
	//	}
	//}

	sipLL->ShapeInstancePairLL::initialize();

	sipLL->mReportPairIndex = INVALID_REPORT_PAIR_ID;

	if(sipLL->ShapeInstancePairLL::readIntFlag(ShapeInstancePairLL::ACTIVE_MANAGER_NOT_ALLOWED) == 0)
	{
		sipLL->ShapeInstancePairLL::updateState(true);
	}
	return sipLL;
}


Sc::TriggerInteraction* Sc::NPhaseCore::createTriggerInteraction(ShapeSim& s0, ShapeSim& s1, PxPairFlags triggerFlags)
{
	ShapeSim* triggerShape;
	ShapeSim* otherShape;

	if (s1.getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
	{
		triggerShape = &s1;
		otherShape = &s0;
	}
	else
	{
		triggerShape = &s0;
		otherShape = &s1;
	}
	TriggerInteraction* pair = mTriggerPool.construct(*triggerShape, *otherShape);
	pair->TriggerInteraction::initialize();
	pair->setTriggerFlags(triggerFlags);
	return pair;
}


Sc::ElementInteractionMarker* Sc::NPhaseCore::createElementInteractionMarker(ElementSim& e0, ElementSim& e1)
{
	ElementInteractionMarker* pair = mInteractionMarkerPool.construct(e0, e1);
	pair->ElementInteractionMarker::initialize();
	return pair;
}


Sc::FilterPair* Sc::NPhaseCore::createFilterPair()
{
	FilterPair* fp = mFilterPairPool.construct();
	return fp;
}


void Sc::NPhaseCore::deleteFilterPair(FilterPair* pair)
{
	mFilterPairPool.destroy(pair);
}


Sc::FilterPair* Sc::NPhaseCore::fetchFilterPair(PxU32 pairID)
{
	return &mFilterPairPool[pairID];
}


Sc::FilterPair* Sc::NPhaseCore::fetchFilterPair(void* reference)
{
	// MS: Let's see whether this will become a performance issue. If yes, then we need to add
	//     the filter pair index to the interaction classes.
	
	Cm::IndexedPool<FilterPair, 32>::Iterator iter(mFilterPairPool);
	while(!iter.done())
	{
		FilterPair& p = *iter;

		if ((void*)p.elementElementRef == reference)
		{
			PX_ASSERT(p.type != FilterPair::INVALID);
			return &p;
		}

		++iter;
	}

	return NULL;
}

static PX_INLINE PxFilterFlags checkFilterFlags(PxFilterFlags filterFlags)
{
	if ((filterFlags & (PxFilterFlag::eKILL | PxFilterFlag::eSUPPRESS)) == (PxFilterFlag::eKILL | PxFilterFlag::eSUPPRESS))
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "Filtering: eKILL and eSUPPRESS must not be set simultaneously. eSUPPRESS will be used.");
		filterFlags.clear(PxFilterFlag::eKILL);
	}

	return filterFlags;
}

static PX_INLINE PxPairFlags checkRbPairFlags(const ShapeSim& s0, const ShapeSim& s1, PxPairFlags pairFlags, PxFilterFlags filterFlags)
{
	if(filterFlags & (PxFilterFlag::eSUPPRESS | PxFilterFlag::eKILL))
		return pairFlags;

	// PT: 'getBodySim()' not inlined, so call it only once here
	Sc::BodySim* bs0 = s0.getBodySim();
	Sc::BodySim* bs1 = s1.getBodySim();

	if (bs0 && bs0->isKinematic() && 
		bs1 && bs1->isKinematic() && 
		(pairFlags & PxPairFlag::eSOLVE_CONTACT))
	{
#ifdef PX_CHECKED
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "Filtering: Resolving contacts between two kinematic objects is invalid. Contacts will not get resolved.");
#endif
		pairFlags.clear(PxPairFlag::eSOLVE_CONTACT);
	}

#ifdef PX_CHECKED
	// we want to avoid to run contact generation for pairs that should not get resolved or have no contact/trigger reports
	if (!((PxU32)pairFlags & (PxPairFlag::eSOLVE_CONTACT | ShapeInstancePairLL::CONTACT_REPORT_EVENTS)))
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "Filtering: Pair with no contact/trigger reports detected, nor is PxPairFlag::eSOLVE_CONTACT set. It is recommended to suppress/kill such pairs for performance reasons.");
	}
	else if(!(pairFlags & (PxPairFlag::eDETECT_DISCRETE_CONTACT | PxPairFlag::eDETECT_CCD_CONTACT)))
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING,  __FILE__, __LINE__, "Filtering: Pair did not request either eDETECT_DISCRETE_CONTACT or eDETECT_CCD_CONTACT. It is recommended to suppress/kill such pairs for performance reasons.");
	}
#else
	PX_UNUSED(filterFlags);
#endif

#ifdef	PX_CHECKED
	if(((s0.getFlags() & PxShapeFlag::eTRIGGER_SHAPE)!=0 || (s1.getFlags() & PxShapeFlag::eTRIGGER_SHAPE)!=0) &&
		(pairFlags & PxPairFlag::eTRIGGER_DEFAULT) && (pairFlags & PxPairFlag::eDETECT_CCD_CONTACT))
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING,  __FILE__, __LINE__, "Filtering: CCD isn't supported on Triggers yet");
	}
#endif

	return pairFlags;
}

static Sc::InteractionType getRbElementInteractionType(const ShapeSim* primitive0, const ShapeSim* primitive1, PxFilterFlags filterFlag)
{
	if(filterFlag & PxFilterFlag::eKILL)
		return PX_INTERACTION_TYPE_COUNT;

	if(filterFlag & PxFilterFlag::eSUPPRESS)
		return PX_INTERACTION_TYPE_MARKER;

	if(primitive0->getFlags() & PxShapeFlag::eTRIGGER_SHAPE 
    || primitive1->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
		return PX_INTERACTION_TYPE_TRIGGER;

	PX_ASSERT(	(primitive0->getGeometryType() != PxGeometryType::eTRIANGLEMESH) || 
				(primitive1->getGeometryType() != PxGeometryType::eTRIANGLEMESH));

	return PX_INTERACTION_TYPE_OVERLAP;
}

Sc::ElementSimInteraction* Sc::NPhaseCore::refilterInteraction(ElementSimInteraction* pair, const FilterInfo* filterInfo, bool removeFromDirtyList)
{
	InteractionType oldType = pair->getType();

	switch (oldType)
	{
		case PX_INTERACTION_TYPE_TRIGGER:
		case PX_INTERACTION_TYPE_MARKER:
		case PX_INTERACTION_TYPE_OVERLAP:
			{
				FilterInfo finfo;
				if (filterInfo)
				{
					// The filter changes are provided by an outside source (the user filter callback)

					finfo = *filterInfo;
					PX_ASSERT(finfo.filterPair);

					if ((finfo.filterFlags & PxFilterFlag::eKILL) && 
						((finfo.filterFlags & PxFilterFlag::eNOTIFY) == PxFilterFlag::eNOTIFY) )
					{
						callPairLost(pair->getElementSim0(), pair->getElementSim1(), finfo.filterPair->getPoolIndex(), false);
						deleteFilterPair(finfo.filterPair);
						finfo.filterPair = NULL;
					}

					PX_ASSERT(pair->getElement0().getElementType() == PX_ELEMENT_TYPE_SHAPE);
					PX_ASSERT(pair->getElement1().getElementType() == PX_ELEMENT_TYPE_SHAPE);

					ShapeSim& s0 = static_cast<ShapeSim&>(pair->getElement0());
					ShapeSim& s1 = static_cast<ShapeSim&>(pair->getElement1());

					finfo.pairFlags = checkRbPairFlags(s0, s1, finfo.pairFlags, finfo.filterFlags);

					//!!!Filter  Issue: Need to check whether the requested changes don't violate hard constraints.
					//                  - Assert that the shapes are not connected through a collision disabling joint
				}
				else
				{
					FilterPair* filterPair = NULL;
					if (pair->readCoreFlag(CoreInteraction::IS_FILTER_PAIR))
					{
						filterPair = fetchFilterPair(reinterpret_cast<void*>(pair));
						PX_ASSERT(filterPair);

						callPairLost(pair->getElementSim0(), pair->getElementSim1(), filterPair->getPoolIndex(), false);
					}

					PX_ASSERT(pair->getElement0().getElementType() == PX_ELEMENT_TYPE_SHAPE);
					PX_ASSERT(pair->getElement1().getElementType() == PX_ELEMENT_TYPE_SHAPE);

					ShapeSim& s0 = static_cast<ShapeSim&>(pair->getElement0());
					ShapeSim& s1 = static_cast<ShapeSim&>(pair->getElement1());

					finfo = filterRbCollisionPair(s0, s1, filterPair);
				}

				if (pair->readCoreFlag(CoreInteraction::IS_FILTER_PAIR) &&
					((finfo.filterFlags & PxFilterFlag::eNOTIFY) != PxFilterFlag::eNOTIFY) )
				{
					// The pair was a filter callback pair but not any longer
					pair->clearCoreFlag(CoreInteraction::IS_FILTER_PAIR);

					if (finfo.filterPair)
					{
						deleteFilterPair(finfo.filterPair);
						finfo.filterPair = NULL;
					}
				}

				PX_ASSERT(pair->getElement0().getElementType() == PX_ELEMENT_TYPE_SHAPE);
				PX_ASSERT(pair->getElement1().getElementType() == PX_ELEMENT_TYPE_SHAPE);

				ShapeSim& s0 = static_cast<ShapeSim&>(pair->getElement0());
				ShapeSim& s1 = static_cast<ShapeSim&>(pair->getElement1());

				InteractionType newType = getRbElementInteractionType(&s0, &s1, finfo.filterFlags);
				if (pair->getType() != newType)  //Only convert interaction type if the type has changed
				{
					return convert(pair, newType, finfo, removeFromDirtyList);
				}
				else
				{
					//The pair flags might have changed, we need to forward the new ones
					if (oldType == PX_INTERACTION_TYPE_OVERLAP)
					{
						ShapeInstancePairLL* sip = static_cast<ShapeInstancePairLL*>(pair);

						if (sip->reportPairFlagsChanged(finfo.pairFlags))
						{
							PxU32 newPairFlags = finfo.pairFlags;
							PxU32 oldPairFlags = sip->getPairFlags();

							if (sip->readIntFlag(ShapeInstancePairLL::IN_PERSISTENT_EVENT_LIST) && (!(newPairFlags & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)))
							{
								// the new report pair flags don't require persistent checks anymore -> remove from persistent list
								// Note: The pair might get added to the force threshold list later in updateState()
								if (sip->readIntFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST))
								{
									removeFromPersistentContactEventPairs(sip);
								}
								else
								{
									sip->clearFlag(ShapeInstancePairLL::WAS_IN_PERSISTENT_EVENT_LIST);
								}
							}

							if ((oldPairFlags & ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS) && (!(newPairFlags & ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS)))
							{
								// no force threshold events needed any longer -> clear flags
								sip->clearFlag(ShapeInstancePairLL::FORCE_THRESHOLD_EXCEEDED_FLAGS);

								if (sip->readIntFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST))
									removeFromForceThresholdContactEventPairs(sip);
							}
						}
						sip->setPairFlags(finfo.pairFlags);
					}
					else if (oldType == PX_INTERACTION_TYPE_TRIGGER)
						static_cast<TriggerInteraction*>(pair)->setTriggerFlags(finfo.pairFlags);

					return pair;
				}
			}
#if (PX_USE_PARTICLE_SYSTEM_API)
		case PX_INTERACTION_TYPE_PARTICLE_BODY:
			{
				PX_ASSERT(pair->getInteractionFlags() & PX_INTERACTION_FLAG_ELEMENT_ACTOR);
				ElementActorInteraction* eai = static_cast<ElementActorInteraction*>(pair);

				ActorElementPair* aep = eai->getActorElementPair();

				FilterInfo finfo;
				if (filterInfo)
				{
					// The filter changes are provided by an outside source (the user filter callback)

					finfo = *filterInfo;

					PX_ASSERT((finfo.filterPair && aep->isFilterPair()) ||
							(!finfo.filterPair && !aep->isFilterPair()) );
					PX_ASSERT(aep->getPairFlags() == finfo.pairFlags);
					PX_ASSERT(!(finfo.filterFlags & PxFilterFlag::eKILL) ||
							((finfo.filterFlags & PxFilterFlag::eKILL) && aep->isKilled()));
					PX_ASSERT(!(finfo.filterFlags & PxFilterFlag::eSUPPRESS) ||
							((finfo.filterFlags & PxFilterFlag::eSUPPRESS) && aep->isSuppressed()) );
					// This should have been done at statusChange() level (see fireCustomFilteringCallbacks())

					if (finfo.filterPair && aep->isKilled() && pair->isLastFilterInteraction())
					{
						// This is the last of (possibly) multiple element-element interactions of the same element-actor interaction
						// --> Kill the filter callback pair

						callPairLost(eai->getElementSim0(), eai->getElementSim1(), finfo.filterPair->getPoolIndex(), false);
						deleteFilterPair(finfo.filterPair);
						finfo.filterPair = NULL;
					}
				}
				else
				{
					if (!aep->hasBeenRefiltered(mOwnerScene.getTimeStamp()))
					{
						// The the first of (possibly) multiple element-element interactions of the same element-actor interaction
						// - For filter callback pairs, call pairLost()
						// - Run the filter
						// - Pass new pair flags on

						FilterPair* filterPair = NULL;
						if (eai->readCoreFlag(CoreInteraction::IS_FILTER_PAIR))
						{
							filterPair = fetchFilterPair(reinterpret_cast<void*>(aep));
							PX_ASSERT(filterPair);

							callPairLost(eai->getElementSim0(), eai->getElementSim1(), filterPair->getPoolIndex(), false);
						}

						finfo = runFilter(pair->getElementSim0(), pair->getElementSim1(), filterPair);
						// Note: The filter run will remove the callback pair if the element-actor interaction should get killed
						//       or if the pair is no longer a filter callback pair

						aep->markAsFilterPair(finfo.filterPair != NULL);

						aep->setPairFlags(finfo.pairFlags);  // Pass on the pair flags (this needs only be done for the first pair)

						if (finfo.filterFlags & PxFilterFlag::eKILL)
						{
							aep->markAsKilled(true);  // Set such that other pairs of the same element-actor interaction know that they have to give their lives for a higher goal...
						}
						else if (finfo.filterFlags & PxFilterFlag::eSUPPRESS)
							aep->markAsSuppressed(true);
						else
						{
							aep->markAsSuppressed(false);
						}
					}
				}

				if (aep->isFilterPair())
				{
					pair->raiseCoreFlag(CoreInteraction::IS_FILTER_PAIR);
				}
				else if (pair->readCoreFlag(CoreInteraction::IS_FILTER_PAIR))
				{
					// The pair was a filter callback pair but not any longer
					pair->clearCoreFlag(CoreInteraction::IS_FILTER_PAIR);
				}

				if (aep->isKilled())
				{
					PX_ASSERT(oldType == PX_INTERACTION_TYPE_PARTICLE_BODY);
					// The ccdPass parameter is needed to avoid concurrent interaction updates while the gpu particle pipeline is running.
					pool_deleteParticleElementRbElementPair(static_cast<ParticleElementRbElementInteraction*>(pair), 0, 0);
					return NULL;
				}

				return pair;
			}
#endif  // PX_USE_PARTICLE_SYSTEM_API
		case PX_INTERACTION_TYPE_CONSTRAINTSHADER:
		case PX_INTERACTION_TYPE_ARTICULATION:
		case PX_INTERACTION_TYPE_COUNT:
		default:
			PX_ASSERT(0);
			return NULL;
	}
}


PX_INLINE void Sc::NPhaseCore::callPairLost(const ElementSim& e0, const ElementSim& e1, PxU32 pairID, bool objVolumeRemoved) const
{
	PxFilterData fd0, fd1;
	PxFilterObjectAttributes fa0, fa1;

	e0.getFilterInfo(fa0, fd0);
	e1.getFilterInfo(fa1, fd1);

	mOwnerScene.getFilterCallbackFast()->pairLost(pairID, fa0, fd0, fa1, fd1, objVolumeRemoved);
}


PX_INLINE void Sc::NPhaseCore::runFilterShader(const ElementSim& e0, const ElementSim& e1,
										   PxFilterObjectAttributes& attr0, PxFilterData& filterData0,
										   PxFilterObjectAttributes& attr1, PxFilterData& filterData1,
										   FilterInfo& filterInfo)
{
	e0.getFilterInfo(attr0, filterData0);
	e1.getFilterInfo(attr1, filterData1);

	filterInfo.filterFlags = mOwnerScene.getFilterShaderFast()(	attr0, filterData0,
																attr1, filterData1,
																filterInfo.pairFlags, 
																mOwnerScene.getFilterShaderDataFast(),
																mOwnerScene.getFilterShaderDataSizeFast() );
}


PX_FORCE_INLINE void fetchActorAndShape(const ElementSim& e, PxActor*& a, PxShape*& s)
{
	if (e.getElementType() == PX_ELEMENT_TYPE_SHAPE)
	{
		ShapeSim& sim = (ShapeSim&)e;
		a = sim.getRbSim().getPxActor();
		s = sim.getPxShape();
	}
#if PX_USE_PARTICLE_SYSTEM_API
	else
	{
		if (e.getElementType() == PX_ELEMENT_TYPE_PARTICLE_PACKET)
			a = ((Sc::ParticlePacketShape&)e).getParticleSystem().getCore().getPxParticleBase();
		s = NULL;
	}
#endif
}


PX_INLINE Sc::FilterInfo Sc::NPhaseCore::runFilter(const ElementSim& e0, const ElementSim& e1, FilterPair* filterPair)
{
	FilterInfo filterInfo;

	PxFilterData fd0, fd1;
	PxFilterObjectAttributes fa0, fa1;

	runFilterShader(e0, e1, fa0, fd0, fa1, fd1, filterInfo);

	if (filterInfo.filterFlags & PxFilterFlag::eCALLBACK)
	{
		if (mOwnerScene.getFilterCallbackFast())
		{
			if (!filterPair)
				filterPair = createFilterPair();
			// If a FilterPair is provided, then we use it, else we create a new one
			// (A FilterPair is provided in the case for a pairLost()-pairFound() sequence after refiltering)

			PxActor* a0, *a1;
			PxShape* s0, *s1;
			fetchActorAndShape(e0, a0, s0);
			fetchActorAndShape(e1, a1, s1);
			
			filterInfo.filterFlags = mOwnerScene.getFilterCallbackFast()->pairFound(filterPair->getPoolIndex(), fa0, fd0, a0, s0, fa1, fd1, a1, s1, filterInfo.pairFlags);
			filterInfo.filterPair = filterPair;
		}
		else
		{
			filterInfo.filterFlags.clear(PxFilterFlag::eNOTIFY);
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "Filtering: eCALLBACK set but no filter callback defined.");
		}
	}

	filterInfo.filterFlags = checkFilterFlags(filterInfo.filterFlags);

	if (filterPair && ((filterInfo.filterFlags & PxFilterFlag::eKILL) || ((filterInfo.filterFlags & PxFilterFlag::eNOTIFY) != PxFilterFlag::eNOTIFY)))
	{
		if ((filterInfo.filterFlags & PxFilterFlag::eKILL) && ((filterInfo.filterFlags & PxFilterFlag::eNOTIFY) == PxFilterFlag::eNOTIFY))
			mOwnerScene.getFilterCallbackFast()->pairLost(filterPair->getPoolIndex(), fa0, fd0, fa1, fd1, false);

		if ((filterInfo.filterFlags & PxFilterFlag::eNOTIFY) != PxFilterFlag::eNOTIFY)
		{
			// No notification, hence we don't need to treat it as a filter callback pair anymore.
			// Make sure that eCALLBACK gets removed as well
			filterInfo.filterFlags.clear(PxFilterFlag::eNOTIFY);
		}

		deleteFilterPair(filterPair);
		filterInfo.filterPair = NULL;
	}


	// Sanity checks
	PX_ASSERT(	(filterInfo.filterFlags != PxFilterFlag::eKILL) ||
				((filterInfo.filterFlags == PxFilterFlag::eKILL) && (!filterInfo.filterPair)) );
	PX_ASSERT(	((filterInfo.filterFlags & PxFilterFlag::eNOTIFY) != PxFilterFlag::eNOTIFY) ||
				(((filterInfo.filterFlags & PxFilterFlag::eNOTIFY) == PxFilterFlag::eNOTIFY) && filterInfo.filterPair) );


	return filterInfo;
}


Sc::FilterInfo Sc::NPhaseCore::filterRbCollisionPair(const ShapeSim& s0, const ShapeSim& s1, FilterPair* filterPair)
{
	FilterInfo filterInfo;

	RigidSim& rbActor0 = s0.getRbSim();
	RigidSim& rbActor1 = s1.getRbSim();

	bool disableCollision = false;
	PxFilterFlags filterFlag;

	bool isS0Static = !rbActor0.isDynamicRigid();
	bool isS1Static = !rbActor1.isDynamicRigid();

	Sc::BodySim* b0 = !isS0Static ? static_cast<BodySim*>(&rbActor0) : 0;
	Sc::BodySim* b1 = !isS1Static ? static_cast<BodySim*>(&rbActor1) : 0;

	bool isS0Trigger = (s0.getFlags() & PxShapeFlag::eTRIGGER_SHAPE) != 0;
	bool isS1Trigger = (s1.getFlags() & PxShapeFlag::eTRIGGER_SHAPE) != 0;

	bool isS0Kinematic = isS0Static ? false : (b0->isKinematic() != 0);
	bool isS1Kinematic = isS1Static ? false : (b1->isKinematic() != 0);

	PxSceneFlags sceneFlags = mOwnerScene.getPublicFlags();

	// Ignore kinematic vs. static pairs if not triggers
	if (!(sceneFlags & PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS))
	{
		if ( !(isS0Trigger || isS1Trigger) && (isS0Static || isS1Static) && (isS0Kinematic || isS1Kinematic) )
		{
			filterFlag = PxFilterFlag::eSUPPRESS;
			disableCollision = true;
		}
	}

	// Ignore kinematic vs. kinematic pairs if not triggers
	if (!(sceneFlags & PxSceneFlag::eENABLE_KINEMATIC_PAIRS))
	{
		if ( !(isS0Trigger || isS1Trigger) && isS0Kinematic && isS1Kinematic )
		{
			filterFlag = PxFilterFlag::eSUPPRESS;
			disableCollision = true;
		}
	}

	if (!disableCollision)
	{
		// If the bodies of the shape pair are connected by a joint, we need to check whether this connection disables the collision.
		// Note: As an optimization, the dynamic bodies have a flag which specifies whether they have any constraints at all. That works
		//       because a constraint has at least one dynamic body and an interaction is tracked by both objects.
		bool collisionDisabled = false;
		bool hasConstraintConnection = false;
		if (b0 && b0->readInternalFlag(Sc::BodySim::BF_HAS_CONSTRAINTS))
			hasConstraintConnection = b0->isConnectedTo(rbActor1, collisionDisabled);
		else if (b1 && b1->readInternalFlag(Sc::BodySim::BF_HAS_CONSTRAINTS))
			hasConstraintConnection = b1->isConnectedTo(rbActor0, collisionDisabled);
		disableCollision = hasConstraintConnection && collisionDisabled;
		filterFlag = PxFilterFlag::eSUPPRESS;

		// If the bodies are articulation links of the same articulation and one link is the parent of the other link, then disable collision
		if (rbActor0.getActorType() == PxActorType::eARTICULATION_LINK && rbActor1.getActorType() == PxActorType::eARTICULATION_LINK)
		{
			Cm::Range<Interaction*const> interactions = rbActor0.getActorInteractions();
			for(; !interactions.empty(); interactions.popFront())
			{
				Interaction *const interaction = interactions.front();
				if (interaction->getType() == PX_INTERACTION_TYPE_ARTICULATION)
				{
					if ((&interaction->getActor0() == &rbActor1) || (&interaction->getActor1() == &rbActor1))
					{
						disableCollision = true;
						filterFlag = PxFilterFlag::eKILL;
						break;
					}
				}
			}
		}
	}

	if (!disableCollision)
	{
		filterInfo = runFilter(s0, s1, filterPair);

		filterInfo.pairFlags = checkRbPairFlags(s0, s1, filterInfo.pairFlags, filterInfo.filterFlags);
	}
	else
	{
		if (filterPair)
			deleteFilterPair(filterPair);

		filterInfo.filterFlags = filterFlag;
	}

	return filterInfo;
}




Sc::ActorPair* Sc::NPhaseCore::findActorPair(ShapeSim* s0, ShapeSim* s1)
{
	PX_ASSERT(!(s0->getFlags() & PxShapeFlag::eTRIGGER_SHAPE) 
		   && !(s1->getFlags() & PxShapeFlag::eTRIGGER_SHAPE));
	// This method is only for the case where a ShapeInstancePair is going to be created.
	// Else we might create an ActorPair that does not get referenced and causes a mem leak.

	RigidSim* aLess;
	RigidSim* aMore;

	// PT: 'getRbSim()' is not inlined so call it only once here.
	Sc::RigidSim& rs0 = s0->getRbSim();
	Sc::RigidSim& rs1 = s1->getRbSim();

	if(s0->getActorSim().getActorInteractionCount() < s1->getActorSim().getActorInteractionCount())
	{
		aLess = &rs0;
		aMore = &rs1;
	}
	else
	{
		aLess = &rs1;
		aMore = &rs0;
	}

	ActorPair* ap = NULL;
	Cm::Range<Interaction*const> interactions = aLess->getActorInteractions();
	for(; !interactions.empty(); interactions.popFront())
	{
		Interaction*const interaction = interactions.front();
		if(((&interaction->getActor0() == aLess && &interaction->getActor1() == aMore) ||
			 (&interaction->getActor0() == aMore && &interaction->getActor1() == aLess)))
		{
			if(interaction->getInteractionFlags() & PX_INTERACTION_FLAG_SIP)
			{
				ShapeInstancePairLL* sip = (ShapeInstancePairLL*)interaction;
				ap = sip->getActorPair();
				break;
			}
		}
	}

	if(!ap)
	{
		return mActorPairPool.construct(rs0, rs1);
	}
	else
	{
		return ap;
	}
}


Sc::ElementSimInteraction* Sc::NPhaseCore::convert(ElementSimInteraction* pair, InteractionType newType, FilterInfo& filterInfo, bool removeFromDirtyList)
{
	PX_ASSERT(newType != pair->getType());

	ElementSim& elementA = pair->getElementSim0();
	ElementSim& elementB = pair->getElementSim1();

	ElementSimInteraction* result = NULL;

	// Wake up the actors of the pair
	if (!pair->getActor0().isActive() && pair->getActor0().getActorType() == PxActorType::eRIGID_DYNAMIC)
		static_cast<BodySim&>(pair->getActor0()).internalWakeUp();
	if (!pair->getActor1().isActive() && pair->getActor1().getActorType() == PxActorType::eRIGID_DYNAMIC)
		static_cast<BodySim&>(pair->getActor1()).internalWakeUp();

	switch (newType)
	{
		case PX_INTERACTION_TYPE_COUNT:
			// This means the pair should get killed
			break;
		case PX_INTERACTION_TYPE_MARKER:
			{
			result = createElementInteractionMarker(elementA, elementB);
			break;
			}
		case PX_INTERACTION_TYPE_OVERLAP:
			{
			PX_ASSERT(elementA.getElementType() == PX_ELEMENT_TYPE_SHAPE && elementB.getElementType() == PX_ELEMENT_TYPE_SHAPE);
			result = createShapeInstancePairLL(static_cast<ShapeSim&>(elementA), static_cast<ShapeSim&>(elementB), filterInfo.pairFlags);
			break;
			}
		case PX_INTERACTION_TYPE_TRIGGER:
			{
			PX_ASSERT(elementA.getElementType() == PX_ELEMENT_TYPE_SHAPE && elementB.getElementType() == PX_ELEMENT_TYPE_SHAPE);
			result = createTriggerInteraction(static_cast<ShapeSim&>(elementA), static_cast<ShapeSim&>(elementB), filterInfo.pairFlags);
			break;
			}
		case PX_INTERACTION_TYPE_CONSTRAINTSHADER:
#if PX_USE_PARTICLE_SYSTEM_API
		case PX_INTERACTION_TYPE_PARTICLE_BODY:
#endif
		case PX_INTERACTION_TYPE_ARTICULATION:
		default:
			PX_ASSERT(0);
			break;
	};

	if (filterInfo.filterPair)
	{
		PX_ASSERT(result);
		// If a filter callback pair is going to get killed, then the FilterPair struct should already have
		// been deleted.

		// Mark the new interaction as a filter callback pair
		result->raiseCoreFlag(CoreInteraction::IS_FILTER_PAIR);

		filterInfo.filterPair->setElementElementRef(result);
	}

	if (pair->readCoreFlag(CoreInteraction::IS_FILTER_PAIR))
		pair->clearCoreFlag(CoreInteraction::IS_FILTER_PAIR);
	// Since the FilterPair struct might have been re-used in the newly created interaction, we need to clear
	// the filter pair marker of the old interaction to avoid that the FilterPair gets deleted by the releaseElementPair()
	// call that follows.

	releaseElementPair(pair, PairReleaseFlag::eWAKE_ON_LOST_TOUCH, 0, removeFromDirtyList);

	return result;
}


void Sc::NPhaseCore::narrowPhase()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mOwnerScene,Sim,processTriggers);
	Scene& scene = mOwnerScene;

	// Triggers
	Cm::Range<Interaction*const> interactions = 
		mOwnerScene.getInteractionScene().getActiveInteractions(PX_INTERACTION_TYPE_TRIGGER);

	PxU32 count = interactions.size();
#ifdef PX_DEBUG
	Interaction*const* test = &interactions.back();
#endif

	for(PxU32 i=0; i < count; i++)
	{
		Interaction*const interaction = interactions.front();
		TriggerInteraction* tri = (TriggerInteraction*)interaction;

		PX_ASSERT(scene.getInteractionScene().isActiveInteraction(tri));
		findTriggerContacts(tri, false, false);

		if (!(tri->readIntFlag(TriggerInteraction::PROCESS_THIS_FRAME)))
		{
			// active trigger pairs for which overlap tests were not forced should remain in the active list
			// to catch transitions between overlap and no overlap
			interactions.popFront();
			continue;
		}
		else
		{
			tri->clearFlag(TriggerInteraction::PROCESS_THIS_FRAME);

			// explicitly scheduled overlap test is done (after object creation, teleport, ...). Check if trigger pair should remain active or not.

			if (!tri->onActivate(0))
			{
#ifdef PX_DEBUG
				Interaction*const testInt = *test;
#endif
				PX_ASSERT(scene.getInteractionScene().isActiveInteraction(tri));
				// Why is the assert enough?
				// Once an explicit overlap test is scheduled, the interaction can not get deactivated anymore until it got processed.

				scene.getInteractionScene().notifyInteractionDeactivated(tri); // Attention: This does a replace with last, so it is important to not pop the front of the range

#ifdef PX_DEBUG
				// make sure a replace with last is done above (not so nice but with the upcoming island gen refactor the whole thing will change anyway)
				PX_ASSERT(interactions.front() == testInt);
				test--;
#endif
			}
			else
				interactions.popFront();
		}
	}
}


PX_FORCE_INLINE bool findTriggerContacts(const ShapeSim* primitive0, const ShapeSim* primitive1, TriggerInteraction* callback)
{
	PX_ASSERT(primitive0->getFlags() & PxShapeFlag::eTRIGGER_SHAPE 
		   || primitive1->getFlags() & PxShapeFlag::eTRIGGER_SHAPE);

	// Reorder them if needed
	if(primitive0->getGeometryType() > primitive1->getGeometryType())
		Ps::swap(primitive0, primitive1);

	const Gu::GeomOverlapFunc overlapFunc =
		Gu::GetGeomOverlapMethodTable()[primitive0->getGeometryType()][primitive1->getGeometryType()];
	PX_ASSERT(overlapFunc);
	return overlapFunc(	primitive0->getCore().getGeometry(), primitive0->getAbsPose(),
						primitive1->getCore().getGeometry(), primitive1->getAbsPose(),
						&callback->getTriggerCache());
}


void Sc::NPhaseCore::findTriggerContacts(TriggerInteraction* tri, bool toBeDeleted, bool volumeRemoved)
{
	ShapeSim* s0 = tri->getTriggerShape();
	ShapeSim* s1 = tri->getOtherShape();

	PxPairFlags pairFlags = tri->getTriggerFlags();
	PxPairFlags pairEvent;

	bool overlap;
	PxU8 testForRemovedShapes = 0;
	if (toBeDeleted)
	{
		// The trigger interaction is to lie down in its tomb, hence we know that the overlap is gone.
		// What remains is to check whether the interaction was deleted because of a shape removal in
		// which case we need to later check for removed shapes.

		overlap = false;

		if (volumeRemoved)
		{
			// Note: only the first removed volume can be detected when the trigger interaction is deleted but at a later point the second volume might get removed too.
			testForRemovedShapes = TriggerPairFlag::eTEST_FOR_REMOVED_SHAPES;
		}
	}
	else
	{
#if PX_ENABLE_SIM_STATS
		mOwnerScene.getStatsInternal().incTriggerPairs(s0->getGeometryType(), s1->getGeometryType());
#endif

		overlap = ::findTriggerContacts(s0, s1, tri);
	}

	bool hadOverlap = tri->lastFrameHadContacts();
	if (hadOverlap)
	{
		if (!overlap)
			pairEvent = PxPairFlag::eNOTIFY_TOUCH_LOST;
	}
	else
	{
		if (overlap)
			pairEvent = PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}

	PxPairFlags triggeredFlags = pairEvent & pairFlags;
	if (triggeredFlags)
	{
		PxTriggerPair triggerPair;
		triggerPair.triggerShape = s0->getPxShape();
		triggerPair.otherShape = s1->getPxShape();
		triggerPair.status = (PxPairFlag::Enum)(PxU32)pairEvent;
		triggerPair.flags = PxTriggerPairFlags(testForRemovedShapes);

		const RigidCore& rigidCore0 = s0->getRbSim().getRigidCore();
		const RigidCore& rigidCore1 = s1->getRbSim().getRigidCore();

		triggerPair.triggerActor = static_cast<PxRigidActor*>(rigidCore0.getPxActor());
		triggerPair.otherActor = static_cast<PxRigidActor*>(rigidCore1.getPxActor());

		TriggerPairExtraData triggerPairExtra(s0->getID(), s1->getID(),
									rigidCore0.getOwnerClient(), rigidCore1.getOwnerClient(),
									rigidCore0.getClientBehaviorFlags(), rigidCore1.getClientBehaviorFlags());

		TriggerBufferAPI& triggerBufferAPI = mOwnerScene.getTriggerBufferAPI();
		triggerBufferAPI.pushBack(triggerPair);

		TriggerBufferExtraData& triggerBufferExtra = mOwnerScene.getTriggerBufferExtraData();
		triggerBufferExtra.pushBack(triggerPairExtra);
	}

	tri->updateLastFrameHadContacts(overlap);
}


void Sc::NPhaseCore::visualize(Cm::RenderOutput& out)
{
	if (mOwnerScene.getVisualizationScale() == 0.0f)
		return;

	Cm::Range<Interaction*const> range = mOwnerScene.getInteractionScene().getActiveInteractions(Sc::PX_INTERACTION_TYPE_OVERLAP);
	Interaction*const* prefetch = &range.front()+4;
	for ( ; !range.empty(); range.popFront())
	{
		if(prefetch <= &range.back())
			Ps::prefetchLine(*prefetch++);
		static_cast<ShapeInstancePairLL*>(range.front())->visualize(out);
	}
}


void Sc::NPhaseCore::processPersistentContactEvents()
{
	//TODO: put back this optimization -- now we have to do this stuff if at least one client has a callback registered.
	// if (ownerScene.getSimulatonEventCallbackFast() != NULL)
	{
		// Go through ShapeInstancePairs which requested persistent contact event reports. This is necessary since there are no low level events for persistent contact.
		ShapeInstancePairLL*const* persistentEventPairs = getCurrentPersistentContactEventPairs();
		PxU32 size = getCurrentPersistentContactEventPairCount();
		while(size--)
		{
			ShapeInstancePairLL* pair = *persistentEventPairs++;
			if(size)
			{
				ShapeInstancePairLL* nextPair = *persistentEventPairs;
				Ps::prefetchLine(nextPair);
			}

			ActorPair* aPair = pair->getActorPair();
			Ps::prefetchLine(aPair);

			PX_ASSERT(pair->hasTouch());
			PX_ASSERT(pair->isReportPair());

			PxU32 pairFlags = pair->getPairFlags();
			if ((pairFlags & (PxU32)(PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eDETECT_DISCRETE_CONTACT)) == (PxU32)(PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eDETECT_DISCRETE_CONTACT))
			{
				// do not process the pair if only eDETECT_CCD_CONTACT is enabled because at this point CCD did not run yet. Plus the current CCD implementation can not reliably provide eNOTIFY_TOUCH_PERSISTS events
				// for performance reasons.
				pair->processUserNotification(PxPairFlag::eNOTIFY_TOUCH_PERSISTS, 0, false, 0);
			}
		}
	}
}


void Sc::NPhaseCore::fireCustomFilteringCallbacks()
{
	CM_PROFILE_ZONE_WITH_SUBSYSTEM(mOwnerScene,Sim,fireCustomFilteringCallbacks);

	PxSimulationFilterCallback* callback = mOwnerScene.getFilterCallbackFast();

	if (callback)
	{
		// Ask user for pair filter status changes
		PxU32 pairID;
		PxFilterFlags filterFlags;
		PxPairFlags pairFlags;
		while(callback->statusChange(pairID, pairFlags, filterFlags))
		{
			FilterPair& fp = mFilterPairPool[pairID];

			PX_ASSERT(fp.type != FilterPair::INVALID);
			// Check if the user tries to update a pair even though he deleted it earlier in the same frame

			filterFlags = checkFilterFlags(filterFlags);

			if (fp.type == FilterPair::ELEMENT_ELEMENT)
			{
				ElementSimInteraction* ei = fp.elementElementRef;
				PX_ASSERT(ei->readCoreFlag(CoreInteraction::IS_FILTER_PAIR));

				FilterInfo finfo;
				finfo.filterFlags = filterFlags;
				finfo.pairFlags = pairFlags;
				finfo.filterPair = &fp;

				ElementSimInteraction* refInt = refilterInteraction(ei, &finfo, true);
				if (refInt == ei)  // No interaction conversion happened, the pairFlags were just updated
				{
					refInt->setDirty(CoreInteraction::CIF_DIRTY_FILTER_STATE);  // To make sure the call below updates internal data according to new pairFlags
					refInt->updateState(true);
				}
			}
			else
			{
				PX_ASSERT(fp.type == FilterPair::ELEMENT_ACTOR);
				ActorElementPair* aep = fp.elementActorRef;

				FilterInfo finfo;

				if ((filterFlags & PxFilterFlag::eNOTIFY) != PxFilterFlag::eNOTIFY)
				{
					deleteFilterPair(&fp);
					aep->markAsFilterPair(false);
				}
				else
					finfo.filterPair = &fp;

				finfo.filterFlags = filterFlags;
				finfo.pairFlags = pairFlags;

				aep->setPairFlags(pairFlags);
				if (filterFlags & PxFilterFlag::eKILL)
					aep->markAsKilled(true);
				else if (filterFlags & PxFilterFlag::eSUPPRESS)
					aep->markAsSuppressed(true);

#if PX_USE_PARTICLE_SYSTEM_API
				Actor* actor = &aep->getActor();
				Element* element = &aep->getElement();

				Element::ElementInteractionReverseIterator iter = element->getElemInteractionsReverse();
				ElementInteraction* interaction = iter.getNext();
				while(interaction)
				{
					if(interaction->getInteractionFlags() & PX_INTERACTION_FLAG_ELEMENT_ACTOR)
					{
						PX_ASSERT(interaction->getType() == PX_INTERACTION_TYPE_PARTICLE_BODY);  // this is the only remaining element-actor interaction now
						
						ParticleElementRbElementInteraction* pri = static_cast<ParticleElementRbElementInteraction*>(interaction);

						PX_ASSERT(pri->getElement0().getElementType() == PX_ELEMENT_TYPE_PARTICLE_PACKET);
						if ((&pri->getElement1() == element) && (&pri->getActor0() == actor))
						{
							PX_ASSERT(pri->readCoreFlag(CoreInteraction::IS_FILTER_PAIR));

							ElementSimInteraction* refInt = refilterInteraction(pri, &finfo, true);
							if (refInt == pri)
								pri->updateState(true);  // To switch between active/suppressed etc.
						}
					}

					interaction = iter.getNext();
				}
#endif // PX_USE_PARTICLE_SYSTEM_API
			}
		}
	}
}


void Sc::NPhaseCore::addToDirtyInteractionList(CoreInteraction* pair)
{
	mDirtyInteractions.insert(pair);
}


void Sc::NPhaseCore::removeFromDirtyInteractionList(CoreInteraction* pair)
{
	PX_ASSERT(mDirtyInteractions.contains(pair));

	mDirtyInteractions.erase(pair);
}


void Sc::NPhaseCore::updatePair(CoreInteraction* coreInteraction)
{
	CoreInteraction* refInt = coreInteraction;

	if (coreInteraction->isElementInteraction() && coreInteraction->needsRefiltering())
	{
		ElementSimInteraction* pair = static_cast<ElementSimInteraction*>(coreInteraction);

		refInt = refilterInteraction(pair, NULL, false);
	}
	
	if (coreInteraction == refInt)  // Refiltering might convert the pair to another type and kill the old one. In that case we don't want to update the new pair since it has been updated on creation.
		coreInteraction->updateState(false);
}


void Sc::NPhaseCore::updateDirtyInteractions()
{
	// The sleeping SIPs will be updated on activation
	// clow: Sleeping SIPs are not awaken for visualization updates
	if (mOwnerScene.readFlag(SCENE_SIP_STATES_DIRTY_DOMINANCE) || mOwnerScene.readFlag(SCENE_SIP_STATES_DIRTY_VISUALIZATION))
	{
		// Update all interactions.

		const InteractionType types[3] = { 
			Sc::PX_INTERACTION_TYPE_OVERLAP,
			Sc::PX_INTERACTION_TYPE_TRIGGER, 
			Sc::PX_INTERACTION_TYPE_MARKER 
		};

		InteractionRange range(mOwnerScene.getInteractionScene(), &InteractionScene::getInteractions, types);

		// Avoid to re-filter interactions if they are not marked dirty in that regard already
		PxU32 mask = CoreInteraction::CIF_DIRTY_ALL & ~CoreInteraction::CIF_DIRTY_FILTER_STATE;
		for ( ; !range.empty(); range.popBack())
		{
			RbElementInteraction* pair = range.back<RbElementInteraction>();
			pair->setDirty(mask);
			updatePair(pair);
		}
	}

	// Update all interactions in the dirty list
	const PxU32 dirtyItcCount = mDirtyInteractions.size();
	Sc::CoreInteraction* const* dirtyInteractions = mDirtyInteractions.getEntries();
	for (PxU32 i = 0; i < dirtyItcCount; i++)
		updatePair(dirtyInteractions[i]);

	mDirtyInteractions.clear();
}


void Sc::NPhaseCore::releaseElementPair(ElementSimInteraction* pair, PxU32 flags, const PxU32 ccdPass, bool removeFromDirtyList)
{
	pair->setClean(removeFromDirtyList);  // Removes the pair from the dirty interaction list etc.
	
	if (pair->readCoreFlag(CoreInteraction::IS_FILTER_PAIR) && pair->isLastFilterInteraction())
	{
		// Check if this is a filter callback pair
		FilterPair* fp = fetchFilterPair((void*)pair);
		PX_ASSERT(fp);

		// Is this interaction removed because one of the pair object broadphase volumes got deleted by the user?
		bool userRemovedVolume = ((flags & PairReleaseFlag::eBP_VOLUME_REMOVED) != 0);

		callPairLost(pair->getElementSim0(), pair->getElementSim1(), fp->getPoolIndex(), userRemovedVolume);

		deleteFilterPair(fp);
	}

	switch (pair->getType())
	{
		case PX_INTERACTION_TYPE_TRIGGER:
			{
				TriggerInteraction* tri = static_cast<TriggerInteraction*>(pair);
				findTriggerContacts(tri, true, ((flags & PairReleaseFlag::eBP_VOLUME_REMOVED) != 0));
				tri->TriggerInteraction::destroy();
				mTriggerPool.destroy(tri);
			}
			break;
		case PX_INTERACTION_TYPE_MARKER:
			{
				ElementInteractionMarker* interactionMarker = static_cast<ElementInteractionMarker*>(pair);
				interactionMarker->ElementInteractionMarker::destroy();
				mInteractionMarkerPool.destroy(interactionMarker);
			}
			break;
		case PX_INTERACTION_TYPE_OVERLAP:
			{
				PX_ASSERT(pair->getInteractionFlags() & PX_INTERACTION_FLAG_SIP);
				ShapeInstancePairLL* sip = static_cast<ShapeInstancePairLL*>(pair);
				releaseShapeInstancePair(sip, flags, ccdPass);
			}
			break;
#if PX_USE_PARTICLE_SYSTEM_API
		case PX_INTERACTION_TYPE_PARTICLE_BODY:
			{
				ParticleElementRbElementInteraction* pbi = static_cast<ParticleElementRbElementInteraction*>(pair);
				pool_deleteParticleElementRbElementPair(pbi, flags, ccdPass);
			}
			break;
#endif
		case PX_INTERACTION_TYPE_CONSTRAINTSHADER:
		case PX_INTERACTION_TYPE_ARTICULATION:
		case PX_INTERACTION_TYPE_COUNT:
		default:
			PX_ASSERT(0);
			return;
	}
}


void Sc::NPhaseCore::releaseShapeInstancePair(ShapeInstancePairLL* npp, PxU32 flags, const PxU32 ccdPass)
{
	ActorPair* aPair = npp->getActorPair();
	if (npp->isReportPair() && npp->hasTouch())
	{
		npp->sendLostTouchReport((flags & PairReleaseFlag::eBP_VOLUME_REMOVED) != 0, ccdPass);
	}

	if (npp->hasTouch() || (!npp->hasKnownTouchState()))
	{
		BodySim* b0 = npp->getShape0().getBodySim();
		BodySim* b1 = npp->getShape1().getBodySim();
		if (flags & PairReleaseFlag::eWAKE_ON_LOST_TOUCH)
		{
			if (!b0 || !b1)
			{
				if (b0)
					b0->internalWakeUp();
				if (b1)
					b1->internalWakeUp();
			}
			else if(!npp->readIntFlag(ShapeInstancePairLL::CONTACTS_RESPONSE_DISABLED))			
			{
				mOwnerScene.addToLostTouchList(b0, b1);
			}
		}
	}

	npp->destroy();
	mLLSipPool.destroy(npp);
	
	if (aPair->getRefCount() == 0)
		mActorPairPool.destroy(aPair);
}


void Sc::NPhaseCore::clearContactReportActorPairs(bool shrinkToZero)
{
	for(PxU32 i=0; i < mContactReportActorPairSet.size(); i++)
	{
		//TODO: prefetch?
		ActorPair* aPair = mContactReportActorPairSet[i];
		const PxU32 refCount = aPair->getRefCount();
		PX_ASSERT(aPair->isInContactReportActorPairSet());
		PX_ASSERT(refCount > 0);
		aPair->decRefCount();  // Reference held by contact callback
		if (refCount > 1)
		{
			aPair->clearInContactReportActorPairSet();
		}
		else
			mActorPairPool.destroy(aPair);
	}

	if (!shrinkToZero)
		mContactReportActorPairSet.clear();
	else
		mContactReportActorPairSet.reset();
}


#if PX_USE_PARTICLE_SYSTEM_API
Sc::ParticleElementRbElementInteraction* Sc::NPhaseCore::insertParticleElementRbElementPair(ParticlePacketShape& particleShape, ShapeSim& rbShape, ActorElementPair* actorElementPair, const PxU32 ccdPass)
{
	ParticleElementRbElementInteraction* pbi = mParticleBodyPool.construct(particleShape, rbShape, *actorElementPair);
	if (pbi)
	{
		actorElementPair->incRefCount();
		// The ccdPass parameter is needed to avoid concurrent interaction updates while the gpu particle pipeline is running.
		pbi->ParticleElementRbElementInteraction::initialize(ccdPass);
	}
	return pbi;
}
#endif


void Sc::NPhaseCore::addToPersistentContactEventPairs(ShapeInstancePairLL* sip)
{
	// Pairs which request events which do not get triggered by the sdk and thus need to be tested actively every frame.
	PX_ASSERT(sip->getPairFlags() & (PxPairFlag::eNOTIFY_TOUCH_PERSISTS | ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS));
	PX_ASSERT(sip->mReportPairIndex == INVALID_REPORT_PAIR_ID);
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST));
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST));
	PX_ASSERT(sip->hasTouch()); // only pairs which can from now on lose or keep contact should be in this list

	sip->raiseFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST);
	if (mPersistentContactEventPairList.size() == mNextFramePersistentContactEventPairIndex)
	{
		sip->mReportPairIndex = mPersistentContactEventPairList.size();
		mPersistentContactEventPairList.pushBack(sip);
	}
	else
	{
		//swap with first entry that will be active next frame
		ShapeInstancePairLL* firstDelayedSip = mPersistentContactEventPairList[mNextFramePersistentContactEventPairIndex];
		firstDelayedSip->mReportPairIndex = mPersistentContactEventPairList.size();
		mPersistentContactEventPairList.pushBack(firstDelayedSip);
		sip->mReportPairIndex = mNextFramePersistentContactEventPairIndex;
		mPersistentContactEventPairList[mNextFramePersistentContactEventPairIndex] = sip;
	}

	mNextFramePersistentContactEventPairIndex++;
}


void Sc::NPhaseCore::addToPersistentContactEventPairsDelayed(ShapeInstancePairLL* sip)
{
	// Pairs which request events which do not get triggered by the sdk and thus need to be tested actively every frame.
	PX_ASSERT(sip->getPairFlags() & (PxPairFlag::eNOTIFY_TOUCH_PERSISTS | ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS));
	PX_ASSERT(sip->mReportPairIndex == INVALID_REPORT_PAIR_ID);
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST));
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST));
	PX_ASSERT(sip->hasTouch()); // only pairs which can from now on lose or keep contact should be in this list

	sip->raiseFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST);
	sip->mReportPairIndex = mPersistentContactEventPairList.size();
	mPersistentContactEventPairList.pushBack(sip);
}


void Sc::NPhaseCore::removeFromPersistentContactEventPairs(ShapeInstancePairLL* sip)
{
	PX_ASSERT(sip->getPairFlags() & (PxPairFlag::eNOTIFY_TOUCH_PERSISTS | ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS));
	PX_ASSERT(sip->readIntFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST));
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST));
	PX_ASSERT(sip->hasTouch()); // only pairs which could lose or keep contact should be in this list

	PxU32 index = sip->mReportPairIndex;
	PX_ASSERT(index != INVALID_REPORT_PAIR_ID);

	if (index < mNextFramePersistentContactEventPairIndex)
	{
		PxU32 replaceIdx = mNextFramePersistentContactEventPairIndex - 1;

		if ((mNextFramePersistentContactEventPairIndex < mPersistentContactEventPairList.size()) && (index != replaceIdx))
		{
			// keep next frame persistent pairs at the back of the list
			ShapeInstancePairLL* tmp = mPersistentContactEventPairList[replaceIdx];
			mPersistentContactEventPairList[index] = tmp;
			tmp->mReportPairIndex = index;
			index = replaceIdx;
		}

		mNextFramePersistentContactEventPairIndex--;
	}

	sip->clearFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST);
	sip->mReportPairIndex = INVALID_REPORT_PAIR_ID;
	mPersistentContactEventPairList.replaceWithLast(index);
	if (index < mPersistentContactEventPairList.size()) // Only adjust the index if the removed SIP was not at the end of the list
		mPersistentContactEventPairList[index]->mReportPairIndex = index;
}


void Sc::NPhaseCore::addToForceThresholdContactEventPairs(ShapeInstancePairLL* sip)
{
	PX_ASSERT(sip->getPairFlags() & ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS);
	PX_ASSERT(sip->mReportPairIndex == INVALID_REPORT_PAIR_ID);
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST));
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST));
	PX_ASSERT(sip->hasTouch());

	sip->raiseFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST);
	sip->mReportPairIndex = mForceThresholdContactEventPairList.size();
	mForceThresholdContactEventPairList.pushBack(sip);
}


void Sc::NPhaseCore::removeFromForceThresholdContactEventPairs(ShapeInstancePairLL* sip)
{
	PX_ASSERT(sip->getPairFlags() & ShapeInstancePairLL::CONTACT_FORCE_THRESHOLD_PAIRS);
	PX_ASSERT(sip->readIntFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST));
	PX_ASSERT(!sip->readIntFlag(ShapeInstancePairLL::IS_IN_PERSISTENT_EVENT_LIST));
	PX_ASSERT(sip->hasTouch());

	PxU32 index = sip->mReportPairIndex;
	PX_ASSERT(index != INVALID_REPORT_PAIR_ID);

	sip->clearFlag(ShapeInstancePairLL::IS_IN_FORCE_THRESHOLD_EVENT_LIST);
	sip->mReportPairIndex = INVALID_REPORT_PAIR_ID;
	mForceThresholdContactEventPairList.replaceWithLast(index);
	if (index < mForceThresholdContactEventPairList.size()) // Only adjust the index if the removed SIP was not at the end of the list
		mForceThresholdContactEventPairList[index]->mReportPairIndex = index;
}


PxU8* Sc::NPhaseCore::reserveContactReportPairData(PxU32 pairCount, PxU32 extraDataSize, PxU32& bufferIndex)
{
	extraDataSize = Sc::ContactStreamManager::computeExtraDataBlockSize(extraDataSize);
	return mContactReportBuffer.allocateNotThreadSafe(extraDataSize + (pairCount * sizeof(Sc::ContactShapePair)), bufferIndex);
}


PxU8* Sc::NPhaseCore::resizeContactReportPairData(PxU32 pairCount, PxU32 extraDataSize, Sc::ContactStreamManager& csm)
{
	PX_ASSERT((pairCount > csm.maxPairCount) || (extraDataSize > csm.getMaxExtraDataSize()));
	PX_ASSERT((csm.currentPairCount == csm.maxPairCount) || (extraDataSize > csm.getMaxExtraDataSize()));
	PX_ASSERT(extraDataSize >= csm.getMaxExtraDataSize()); // we do not support stealing memory from the extra data part when the memory for pair info runs out

	PxU32 bufferIndex;
	Ps::prefetch(mContactReportBuffer.getData(csm.bufferIndex));

	extraDataSize = Sc::ContactStreamManager::computeExtraDataBlockSize(extraDataSize);
	PxU8* stream = mContactReportBuffer.reallocateNotThreadSafe(extraDataSize + (pairCount * sizeof(Sc::ContactShapePair)), bufferIndex, 16, csm.bufferIndex);
	PxU8* oldStream = mContactReportBuffer.getData(csm.bufferIndex);
	if(stream)
	{
		PxU32 maxExtraDataSize = csm.getMaxExtraDataSize();
		if(csm.bufferIndex != bufferIndex)
		{
			if (extraDataSize <= maxExtraDataSize)
				PxMemCopy(stream, oldStream, maxExtraDataSize + (csm.currentPairCount * sizeof(Sc::ContactShapePair)));
			else
			{
				PxMemCopy(stream, oldStream, csm.extraDataSize);
				PxMemCopy(stream + extraDataSize, oldStream + maxExtraDataSize, csm.currentPairCount * sizeof(Sc::ContactShapePair));
			}
			csm.bufferIndex = bufferIndex;
		}
		else if (extraDataSize > maxExtraDataSize)
			PxMemMove(stream + extraDataSize, oldStream + maxExtraDataSize, csm.currentPairCount * sizeof(Sc::ContactShapePair));

		if (pairCount > csm.maxPairCount)
			csm.maxPairCount = Ps::to16(pairCount);
		if (extraDataSize > maxExtraDataSize)
			csm.setMaxExtraDataSize(extraDataSize);
	}

	return stream;
}


Sc::ActorPairContactReportData* Sc::NPhaseCore::createActorPairContactReportData()
{
	return mActorPairContactReportDataPool.construct();
}


void Sc::NPhaseCore::releaseActorPairContactReportData(ActorPairContactReportData* data)
{
	mActorPairContactReportDataPool.destroy(data);
}

#if PX_USE_PARTICLE_SYSTEM_API
void Sc::NPhaseCore::pool_deleteParticleElementRbElementPair(ParticleElementRbElementInteraction* pair, PxU32 flags, const PxU32 ccdPass)
{
	ActorElementPair* aep = pair->getActorElementPair();

	// The ccdPass parameter is needed to avoid concurrent interaction updates while the gpu particle pipeline is running.
	pair->ParticleElementRbElementInteraction::destroy(((flags & PairReleaseFlag::eSHAPE_BP_VOLUME_REMOVED) == PairReleaseFlag::eSHAPE_BP_VOLUME_REMOVED), ccdPass);
	mParticleBodyPool.destroy(pair);

	if (aep->decRefCount() == 0)
		mActorElementPairPool.destroy(aep);
}
#endif	// PX_USE_PARTICLE_SYSTEM_API


