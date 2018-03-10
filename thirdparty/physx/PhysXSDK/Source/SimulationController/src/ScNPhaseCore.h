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


#ifndef PX_PHYSICS_SCP_NPHASE_CORE
#define PX_PHYSICS_SCP_NPHASE_CORE

#include "CmPhysXCommon.h"
#include "CmRenderOutput.h"
#include "PxPhysXConfig.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"
#include "PsAtomic.h"
#include "PsPool.h"
#include "PsHashSet.h"
#include "PxSimulationEventCallback.h"
#include "ScTriggerPairs.h"
#include "ScInteractionType.h"
#include "CmIndexedPool.h"
#include "ScContactReportBuffer.h"


namespace physx
{

struct PxvBroadPhaseOverlap;

namespace Sc
{
	class Scene;

	class ActorSim;
	class ElementSim;
	class ShapeSim;
	class ClothSim;

	class Actor;
	class Element;
	class ElementInteraction;

#if PX_USE_PARTICLE_SYSTEM_API
	class ParticlePacketShape;
#endif

	class CoreInteraction;
	class ElementSimInteraction;
	class ElementActorInteraction;
	class ElementInteractionMarker;
	class RbElementInteraction;
	class TriggerInteraction;
#if PX_USE_PARTICLE_SYSTEM_API 
	class ParticleElementRbElementInteraction;
#endif

	class ShapeInstancePairLL;
	class ActorElementPair;
	class ActorPair;

	class ActorPairContactReportData;
	struct ContactShapePair;

	class NPhaseContext;
	class ContactStreamManager;

	struct FilterPair;
	struct FilterInfo;


	struct PairReleaseFlag
	{
		enum Enum
		{
			eBP_VOLUME_REMOVED			=	(1 << 0),	// the broadphase volume of one pair object has been removed.
			eSHAPE_BP_VOLUME_REMOVED	=	(1 << 1) | eBP_VOLUME_REMOVED,  // the removed broadphase volume was from a rigid body shape.
			eWAKE_ON_LOST_TOUCH			=	(1 << 2)
		};
	};

	/*
	Description: NPhaseCore encapsulates the near phase processing to allow multiple implementations(eg threading and non
	threaded).

	The broadphase inserts shape pairs into the NPhaseCore, which are then processed into contact point streams.
	Pairs can then be processed into AxisConstraints by the GroupSolveCore.

	*/
	class NPhaseCore : public Ps::UserAllocated
	{
		PX_NOCOPY(NPhaseCore)

	public:
		NPhaseCore(Scene& scene, const PxSceneDesc& desc);
		~NPhaseCore();

		void onOverlapCreated(const PxvBroadPhaseOverlap* PX_RESTRICT pairs, PxU32 pairCount, const PxU32 ccdPass);
		void onOverlapCreated(Element* volume0, Element* volume1, const PxU32 ccdPass);
		void onOverlapRemoved(Element* volume0, Element* volume1, const PxU32 ccdPass);
		void onVolumeRemoved(Element* volume, PxU32 flags);

#if PX_USE_PARTICLE_SYSTEM_API
		// The ccdPass parameter is needed to avoid concurrent interaction updates while the gpu particle pipeline is running.
		ParticleElementRbElementInteraction* insertParticleElementRbElementPair(ParticlePacketShape& particleShape, ShapeSim& rbShape, ActorElementPair* actorElementPair, const PxU32 ccdPass);
#endif

#if PX_USE_CLOTH_API
		void removeClothOverlap(ClothSim* clothSim, const ShapeSim* shapeSim);
#endif

		PxU32 getDefaultContactReportStreamBufferSize() const;

		void fireCustomFilteringCallbacks();

		void addToDirtyInteractionList(CoreInteraction* interaction);
		void removeFromDirtyInteractionList(CoreInteraction* interaction);
		void updateDirtyInteractions();


		/*
		Description: Perform/Complete processing of shape instance pairs into contacts streams. Also
		links shape instance pairs into bodies to allow the later retrieval of the contacts.
		*/
		void narrowPhase();


		/*
		Description: Check candidates for persistent touch contact events and create those events if necessary.
		*/
		void processPersistentContactEvents();

		/*
		Description: Displays visualizations associated with the near phase.
		*/
		void visualize(Cm::RenderOutput& out);

		PX_FORCE_INLINE Scene& getScene() const	{ return mOwnerScene;	}

		void findTriggerContacts(TriggerInteraction* tri, bool toBeDeleted, bool volumeRemoved);

		PX_FORCE_INLINE void addToContactReportActorPairSet(ActorPair* pair) { mContactReportActorPairSet.pushBack(pair); }
		void clearContactReportActorPairs(bool shrinkToZero);
		PX_FORCE_INLINE PxU32 getNbContactReportActorPairs() const { return mContactReportActorPairSet.size(); }
		PX_FORCE_INLINE ActorPair* const* getContactReportActorPairs() const { return mContactReportActorPairSet.begin(); }

		void addToPersistentContactEventPairs(ShapeInstancePairLL*);
		void addToPersistentContactEventPairsDelayed(ShapeInstancePairLL*);
		void removeFromPersistentContactEventPairs(ShapeInstancePairLL* sip);
		PX_FORCE_INLINE PxU32 getCurrentPersistentContactEventPairCount() const { return mNextFramePersistentContactEventPairIndex; }
		PX_FORCE_INLINE ShapeInstancePairLL* const* getCurrentPersistentContactEventPairs() const { return mPersistentContactEventPairList.begin(); }
		PX_FORCE_INLINE PxU32 getAllPersistentContactEventPairCount() const { return mPersistentContactEventPairList.size(); }
		PX_FORCE_INLINE ShapeInstancePairLL* const* getAllPersistentContactEventPairs() const { return mPersistentContactEventPairList.begin(); }
		PX_FORCE_INLINE void preparePersistentContactEventListForNextFrame();

		void addToForceThresholdContactEventPairs(ShapeInstancePairLL*);
		void removeFromForceThresholdContactEventPairs(ShapeInstancePairLL*);
		PX_FORCE_INLINE PxU32 getForceThresholdContactEventPairCount() const { return mForceThresholdContactEventPairList.size(); }
		PX_FORCE_INLINE ShapeInstancePairLL* const* getForceThresholdContactEventPairs() const { return mForceThresholdContactEventPairList.begin(); }

		PX_FORCE_INLINE PxU8* getContactReportPairData(const PxU32& bufferIndex) const { return mContactReportBuffer.getData(bufferIndex); }
		PxU8* reserveContactReportPairData(PxU32 pairCount, PxU32 extraDataSize, PxU32& bufferIndex);
		PxU8* resizeContactReportPairData(PxU32 pairCount, PxU32 extraDataSize, Sc::ContactStreamManager& csm);
		PX_FORCE_INLINE void clearContactReportStream() { mContactReportBuffer.reset(); }  // Do not free memory at all
		PX_FORCE_INLINE void freeContactReportStreamMemory() { mContactReportBuffer.flush(); }

		ActorPairContactReportData* createActorPairContactReportData();
		void releaseActorPairContactReportData(ActorPairContactReportData* data);

	private:
		ElementSimInteraction* createRbElementInteraction(ShapeSim& s0, ShapeSim& s1);
#if PX_USE_PARTICLE_SYSTEM_API
		ElementSimInteraction* createParticlePacketBodyInteraction(ParticlePacketShape& ps, ShapeSim& s, const PxU32 ccdPass);
#endif
		void releaseElementPair(ElementSimInteraction* pair, PxU32 flags, const PxU32 ccdPass, bool removeFromDirtyList);
		void releaseShapeInstancePair(ShapeInstancePairLL* pair, PxU32 flags, const PxU32 ccdPass);

		ShapeInstancePairLL* createShapeInstancePairLL(ShapeSim& s0, ShapeSim& s1, PxPairFlags pairFlags);
		TriggerInteraction* createTriggerInteraction(ShapeSim& s0, ShapeSim& s1, PxPairFlags triggerFlags);
		ElementInteractionMarker* createElementInteractionMarker(ElementSim& e0, ElementSim& e1);

		//------------- Filtering -------------

		// Filter pair tracking for filter callbacks
		FilterPair* createFilterPair();
		void deleteFilterPair(FilterPair* pair);
		FilterPair* fetchFilterPair(PxU32 pairID);
		FilterPair* fetchFilterPair(void* reference);

		ElementSimInteraction* refilterInteraction(ElementSimInteraction* pair, const FilterInfo* filterInfo, bool removeFromDirtyList);

		PX_INLINE void callPairLost(const ElementSim& e0, const ElementSim& e1, PxU32 pairID, bool objVolumeRemoved) const;
		PX_INLINE void runFilterShader(const ElementSim& e0, const ElementSim& e1,
			PxFilterObjectAttributes& attr0, PxFilterData& filterData0,
			PxFilterObjectAttributes& attr1, PxFilterData& filterData1,
			FilterInfo& filterInfo);
		PX_INLINE FilterInfo runFilter(const ElementSim& e0, const ElementSim& e1, FilterPair* filterPair);
		FilterInfo filterRbCollisionPair(const ShapeSim& s0, const ShapeSim& s1, FilterPair* filterPair);
		//-------------------------------------

		void updatePair(CoreInteraction* pair);
		ElementSimInteraction* convert(ElementSimInteraction* pair, InteractionType type, FilterInfo& filterInfo, bool removeFromDirtyList);

		ActorPair* findActorPair(ShapeSim* s0, ShapeSim* s1);

		// Pooling
#if PX_USE_PARTICLE_SYSTEM_API
		void pool_deleteParticleElementRbElementPair(ParticleElementRbElementInteraction* pair, PxU32 flags, const PxU32 ccdPass);
#endif

		Scene&											mOwnerScene;

		Ps::Array<ActorPair*>							mContactReportActorPairSet;
		Ps::Array<ShapeInstancePairLL*>					mPersistentContactEventPairList;	// Pairs which request events which do not get triggered by the sdk and thus need to be tested actively every frame.
																							// May also contain force threshold event pairs (see mForceThresholdContactEventPairList)
																							// This list is split in two, the elements in front are for the current frame, the elements at the
																							// back will get added next frame.
		PxU32											mNextFramePersistentContactEventPairIndex;  // start index of the pairs which need to get added to the persistent list for next frame

		Ps::Array<ShapeInstancePairLL*>					mForceThresholdContactEventPairList;	// Pairs which request force threshold contact events. A pair is only in this list if it does have contact.
																								// Note: If a pair additionally requests PxPairFlag::eNOTIFY_TOUCH_PERSISTS events, then it
																								// goes into mPersistentContactEventPairList instead. This allows to share the list index.

		//
		//  data layout:
		//  ContactActorPair0_ExtraData, ContactShapePair0_0, ContactShapePair0_1, ... ContactShapePair0_N, 
		//  ContactActorPair1_ExtraData, ContactShapePair1_0, ...
		//
		ContactReportBuffer								mContactReportBuffer;				// Shape pair information for contact reports

		Ps::CoalescedHashSet<CoreInteraction*>			mDirtyInteractions;
		Cm::IndexedPool<FilterPair, 32>					mFilterPairPool;

		// Pools
		Ps::Pool<ActorPair>								mActorPairPool;
		Ps::Pool<ActorElementPair>						mActorElementPairPool;
		Ps::Pool<ShapeInstancePairLL>					mLLSipPool;
		Ps::Pool<TriggerInteraction>					mTriggerPool;
		Ps::Pool<ActorPairContactReportData>			mActorPairContactReportDataPool;
		Ps::Pool<ElementInteractionMarker>				mInteractionMarkerPool;
#if PX_USE_PARTICLE_SYSTEM_API
		Ps::Pool<ParticleElementRbElementInteraction>	mParticleBodyPool;
#endif

#if PX_USE_CLOTH_API
		struct ClothListElement { 
			ClothListElement(ClothSim* clothSim = NULL, ClothListElement* next = NULL) 
				: mClothSim(clothSim), mNext(next) 
			{}
			ClothSim* mClothSim; 
			ClothListElement* mNext; 
		};
		Ps::Pool<ClothListElement>						mClothPool;
		Ps::HashMap<const ShapeSim*, ClothListElement>  mClothOverlaps;
#endif
	};

} // namespace Sc


PX_FORCE_INLINE void Sc::NPhaseCore::preparePersistentContactEventListForNextFrame()
{
	// reports have been processed -> "activate" next frame candidates for persistent contact events
	mNextFramePersistentContactEventPairIndex = mPersistentContactEventPairList.size();
}


}

#endif
