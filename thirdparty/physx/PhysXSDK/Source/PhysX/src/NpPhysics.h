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


#ifndef PX_PHYSICS_NP_PHYSICS
#define PX_PHYSICS_NP_PHYSICS

#include "PxPhysics.h"
#include "PsUserAllocated.h"
#include "CmEventProfiler.h"
#include "GuMeshFactory.h"
#include "NpMaterial.h"
#include "NpPhysicsInsertionCallback.h"
#include "PxProfileZone.h"
#include "PxProfileZoneManager.h"
#include "NpMaterialManager.h"
#include "ScPhysics.h"
#include "PsHashSet.h"

#ifdef PX_PS3
#include "ps3/CmPhysicsPS3.h"
#endif

#if PX_SUPPORT_GPU_PHYSX
#include "gpu/NpPhysicsGpu.h"
#endif

#ifdef LINUX
#include <string.h>
#endif

#if PX_SUPPORT_GPU_PHYSX
#include "device/PhysXIndicator.h"
#endif

#include "pvd/PxVisualDebugger.h"
#if PX_SUPPORT_VISUAL_DEBUGGER
#include "PvdVisualDebugger.h"
#endif

#include "PxMetaData.h"


namespace physx
{
	struct NpMaterialIndexTranslator
	{
		NpMaterialIndexTranslator() : indicesNeedTranslation(false) {}

		Ps::HashMap<PxU16, PxU16>	map;
		bool						indicesNeedTranslation;
	};

	class NpScene;	
	struct PxvOffsetTable;

#pragma warning(push)
#pragma warning(disable:4996)	// We have to implement deprecated member functions, do not warn.


class NpPhysics : public PxPhysics, public Ps::UserAllocated
{
	NpPhysics& operator=(const NpPhysics&);
	NpPhysics(const NpPhysics &);

	struct NpDelListenerEntry : public UserAllocated
	{
		NpDelListenerEntry(const PxDeletionEventFlags& de, bool restrictedObjSet)
			: flags(de)
			, restrictedObjectSet(restrictedObjSet)
		{
		}

		Ps::HashSet<const PxBase*> registeredObjects;  // specifically registered objects for deletion events
		PxDeletionEventFlags flags;
		bool restrictedObjectSet;
	};


									NpPhysics(const PxTolerancesScale& scale, const PxvOffsetTable& pxvOffsetTable, bool trackOutstandingAllocations, PxProfileZoneManager* profileZoneManager);
	virtual							~NpPhysics();

public:
	
	static      NpPhysics*			createInstance(	PxU32 version, 
													PxFoundation& foundation, 
													const PxTolerancesScale& scale,
													bool trackOutstandingAllocations,
													PxProfileZoneManager* profileZoneManager);

	static		PxU32			releaseInstance();

	static      NpPhysics&		getInstance() { return *mInstance; }

	virtual     void			release();

	virtual		PxScene*		createScene(const PxSceneDesc&);
				void			releaseSceneInternal(PxScene&);
	virtual		PxU32			getNbScenes()	const;
	virtual		PxU32			getScenes(PxScene** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	virtual		PxRigidStatic*		createRigidStatic(const PxTransform&);
	virtual		PxRigidDynamic*		createRigidDynamic(const PxTransform&);
	virtual		PxArticulation*		createArticulation();
	virtual		PxConstraint*		createConstraint(PxRigidActor* actor0, PxRigidActor* actor1, PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize);
	virtual		PxAggregate*		createAggregate(PxU32 maxSize, bool selfCollision);

	virtual		PxShape*			createShape(const PxGeometry&, PxMaterial*const *, PxU16, bool, PxShapeFlags shapeFlags);
	virtual		PxU32				getNbShapes()	const;
	virtual		PxU32				getShapes(PxShape** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

#if PX_USE_PARTICLE_SYSTEM_API
	virtual		PxParticleSystem*	createParticleSystem(PxU32 maxParticles, bool perParticleRestOffset);
	virtual		PxParticleFluid*	createParticleFluid(PxU32 maxParticles, bool perParticleRestOffset);
#endif

#if PX_USE_CLOTH_API
	virtual		PxCloth*			createCloth(const PxTransform& globalPose, PxClothFabric& fabric, const PxClothParticle* particles, PxClothFlags flags);
#endif

	virtual		PxMaterial*			createMaterial(PxReal staticFriction, PxReal dynamicFriction, PxReal restitution);
	virtual		PxU32				getNbMaterials() const;
	virtual		PxU32				getMaterials(PxMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	virtual		PxTriangleMesh*		createTriangleMesh(PxInputStream&);
	virtual		PxU32				getNbTriangleMeshes()	const;
	virtual		PxU32				getTriangleMeshes(PxTriangleMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex=0)	const;


	virtual		PxHeightField*		createHeightField(const PxHeightFieldDesc& desc);
	virtual		PxHeightField*		createHeightField(PxInputStream& stream);
	virtual		PxU32				getNbHeightFields()	const;
	virtual		PxU32				getHeightFields(PxHeightField** userBuffer, PxU32 bufferSize, PxU32 startIndex=0)	const;

	virtual		PxConvexMesh*		createConvexMesh(PxInputStream&);
	virtual		PxU32				getNbConvexMeshes() const;
	virtual		PxU32				getConvexMeshes(PxConvexMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

#if PX_USE_CLOTH_API
	virtual		PxClothFabric*		createClothFabric(PxInputStream&);
	virtual		PxClothFabric*		createClothFabric(const PxClothFabricDesc& desc);
	virtual		PxU32				getNbClothFabrics() const;
	virtual		PxU32				getClothFabrics(PxClothFabric** userBuffer, PxU32 bufferSize) const;
				void				registerCloth();
#endif

#if PX_SUPPORT_GPU_PHYSX
	void							registerPhysXIndicatorGpuClient();
	void							unregisterPhysXIndicatorGpuClient();
#else
	PX_FORCE_INLINE void			registerPhysXIndicatorGpuClient() {}
	PX_FORCE_INLINE void			unregisterPhysXIndicatorGpuClient() {}
#endif

	virtual		const PxTolerancesScale&		getTolerancesScale() const;

	virtual		PxFoundation&		getFoundation();

	virtual		PxVisualDebugger*	getVisualDebugger();
	
	virtual		PxVisualDebuggerConnectionManager* getPvdConnectionManager();
	
	virtual		PxProfileZone& getProfileZone();

	virtual		PxProfileZoneManager* getProfileZoneManager();

	bool							lockScene();
	bool							unlockScene();
	void							destroySceneLock();
	bool							lockScenePS3();
	bool							unlockScenePS3();
	void							destroySceneLockPS3();

	PX_INLINE	NpScene*			getScene(PxU32 i) const { return mSceneArray[i]; }
	PX_INLINE	PxU32				getNumScenes() const { return mSceneArray.size(); }
	PX_INLINE   PxProfileEventSender* getProfileEventSender() { return mProfileZone; }
#ifdef PX_CHECKED
	static PX_INLINE	void		heightfieldsAreRegistered() { mHeightFieldsRegistered = true;  }
#endif

#if PX_SUPPORT_GPU_PHYSX
	virtual		NpPhysicsGpu&		getNpPhysicsGpu() { return mPhysicsGpu; }
#endif

	virtual		void				registerDeletionListener(PxDeletionListener& observer, const PxDeletionEventFlags& deletionEvents, bool restrictedObjectSet);
	virtual		void				unregisterDeletionListener(PxDeletionListener& observer);
	virtual		void				registerDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount);
	virtual		void				unregisterDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount);

				void				notifyDeletionListeners(const PxBase*, void* userData, PxDeletionEventFlag::Enum deletionEvent);
	PX_FORCE_INLINE void			notifyDeletionListenersUserRelease(const PxBase* b, void* userData) { notifyDeletionListeners(b, userData, PxDeletionEventFlag::eUSER_RELEASE); }
	PX_FORCE_INLINE void			notifyDeletionListenersMemRelease(const PxBase* b, void* userData) { notifyDeletionListeners(b, userData, PxDeletionEventFlag::eMEMORY_RELEASE); }

	virtual		PxPhysicsInsertionCallback&	getPhysicsInsertionCallback() { return mObjectInsertion; }

				void				removeMaterialFromTable(NpMaterial&);
				void				updateMaterial(NpMaterial&);
				bool				sendMaterialTable(NpScene&);

				NpMaterialManager&	getMaterialManager()	{	return mMasterMaterialManager;	}

				NpMaterial*			addMaterial(NpMaterial* np);

	static		void				initOffsetTables(PxvOffsetTable& pxvOffsetTable);

	static bool apiReentryLock;

private:
				typedef Ps::CoalescedHashMap<PxDeletionListener*, NpDelListenerEntry*> DeletionListenerMap;

				Ps::Array<NpScene*>	mSceneArray;

				void*				mSceneRunning;

				Sc::Physics					mPhysics;
				NpMaterialManager			mMasterMaterialManager;

				NpPhysicsInsertionCallback	mObjectInsertion;

				struct MeshDeletionListener: public GuMeshFactoryListener
				{
					void onGuMeshFactoryBufferRelease(const PxBase* object, PxType type, bool memRelease)
					{
						PX_UNUSED(type);
						NpPhysics::getInstance().notifyDeletionListeners(object, NULL, memRelease ? PxDeletionEventFlag::eMEMORY_RELEASE : PxDeletionEventFlag::eUSER_RELEASE);
					}
				};

				Ps::Mutex								mDeletionListenerMutex;
				DeletionListenerMap						mDeletionListenerMap;
				MeshDeletionListener					mDeletionMeshListener;
				bool									mDeletionListenersExist;

				Ps::Mutex								mSceneAndMaterialMutex;  // guarantees thread safety for API calls related to scene and material containers

#if PX_SUPPORT_GPU_PHYSX
				PhysXIndicator		mPhysXIndicator;
				PxU32				mNbRegisteredGpuClients;
				Ps::Mutex			mPhysXIndicatorMutex;
				NpPhysicsGpu		mPhysicsGpu;
#endif
				physx::PxProfileZone* mProfileZone;
				physx::PxProfileZoneManager* mProfileZoneManager;
				
#if PX_SUPPORT_VISUAL_DEBUGGER
				Pvd::VisualDebugger*							mVisualDebugger;
				physx::debugger::comm::PvdConnectionManager*	mPVDFactoryManager;
#endif

	static		PxU32				mRefCount;
	static		NpPhysics*			mInstance;

#ifdef PX_CHECKED
	static		bool				mHeightFieldsRegistered;	//just for error checking
#endif

	friend class NpCollection;
};

#pragma warning(pop)
}

#endif
