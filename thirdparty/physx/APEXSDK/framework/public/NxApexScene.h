/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_SCENE_H
#define NX_APEX_SCENE_H

/*!
\file
\brief classes NxApexScene, NxApexSceneStats, NxApexSceneDesc
*/

#include "NxApexDesc.h"
#include "NxApexRenderable.h"
#include "NxApexContext.h"
#include "foundation/PxVec3.h"
#include <NxApexDefs.h>

namespace physx
{
class PxCpuDispatcher;
class PxGpuDispatcher;
class PxTaskManager;
class PxBaseTask;
}

#if NX_SDK_VERSION_MAJOR == 2
class NxScene;
class NxDebugRenderable;
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxFiltering.h"
#include "NxMirrorScene.h"
namespace physx
{
class PxActor;
class PxScene;
class PxRenderBuffer;
}
#endif

namespace NxParameterized
{
class Interface;
}

namespace physx
{
namespace apex
{
// Forward declaration for the NxApexPhysX3Interface::setContactReportFlags() callback
class NxDestructibleActor;

PX_PUSH_PACK_DEFAULT

#if NX_SDK_VERSION_MAJOR == 3
/**
\brief Interface used to call setter function for PhysX 3.0
*/
class NxApexPhysX3Interface
{
public:
	/**
	\brief Set the current contact report pair flags any time the Destructible module creates a PhysX 3.x shape

	User can set the report pair flags to specified actor
	*/
	virtual void setContactReportFlags(PxShape* shape, PxPairFlags flags, NxDestructibleActor* actor, PxU16 actorChunkIndex) = 0;

	/**
	\brief Get the current contact report pair flags

	*/
	virtual physx::PxPairFlags getContactReportFlags(const physx::PxShape* shape) const = 0;
};
#endif

/**
\brief Data used to initialize a new NxApexScene
*/
class NxApexSceneDesc : public NxApexDesc
{
public:

	PX_INLINE NxApexSceneDesc() : NxApexDesc()
	{
		init();
	}

	PX_INLINE void setToDefault()
	{
		NxApexDesc::setToDefault();
		init();
	}

#if NX_SDK_VERSION_MAJOR == 2

	/** \brief Give this ApexScene an existing NxScene */
	NxScene* scene;

	/**
	\brief Give this ApexScene a user defined CpuDispatcher

	If cpuDispatcher is NULL, the APEX SDK will create and use a default
	thread pool.
	*/
	physx::PxCpuDispatcher* cpuDispatcher;

	/**
	\brief Give this ApexScene a user defined GpuDispatcher

	If gpuDispatcher is NULL, this APEX scene will not use any GPU accelerated
	features.
	*/
	physx::PxGpuDispatcher* gpuDispatcher;

#elif NX_SDK_VERSION_MAJOR == 3

	/** \brief Give this ApexScene an existing PxScene

	The ApexScene will use the same CpuDispatcher and GpuDispatcher that were
	provided to the PxScene when it was created.  There is no need to provide
	these pointers in the ApexScene descriptor.
	*/
	PxScene* scene;


	/** \brief Give this ApexScene a user defined interface to the PhysX 3.*
	*/
	NxApexPhysX3Interface*	physX3Interface;

#endif

	/**
	\brief Toggle the use of a legacy NxDebugRenderable for PhysX 2.8.x or PxRenderBuffer for 3.x

	If true, then debug rendering will happen through the legacy DebugRenderable interface.
	If false (the default value) it will render through the NxUserRenderResources interface.
	*/
	bool	useDebugRenderable;

	/**
	\brief Transmits debug rendering to PVD2 as well
	*/
	bool	debugVisualizeRemotely;

	/**
	\brief If 'debugVisualizeLocally' is true, then debug visualization which is being transmitted remotely will also be shown locally as well.
	*/
	bool	debugVisualizeLocally;

#if defined(APEX_CUDA_SUPPORT)
	/**
	\brief Switch for particles (BasicIOS, ParticleIOS, FieldSampler) which defines whether to use CUDA or not.
	*/
	bool useCuda;
#endif

private:
	PX_INLINE void init()
	{
#if NX_SDK_VERSION_MAJOR == 2
		cpuDispatcher = 0;
		gpuDispatcher = 0;
#elif NX_SDK_VERSION_MAJOR == 3
		physX3Interface	= 0;
#endif
		scene = 0;
		useDebugRenderable = false;
		debugVisualizeRemotely = false;
		debugVisualizeLocally = true;
#if defined(APEX_CUDA_SUPPORT)
		useCuda = true;
#endif
	}
};


/**
\brief APEX stat struct that contains the type enums
*/
struct ApexStatDataType
{
	/**
	\brief Enum of the APEX stat types
	*/
	enum Enum
	{
		INVALID = 0,
		STRING  = 1,
		INT     = 2,
		FLOAT   = 3,
		ENUM    = 4,
		BOOL    = 5,
	};
};

/**
\brief data value definitions for stats (derived from openautomate)
*/
typedef struct oaValueStruct
{
	union
	{
		char*			String;
		physx::PxI32	Int;
		physx::PxF32	Float;
		char*			Enum;
		bool			Bool;
	};
} ApexStatValue;

/**
\brief data value definitions for stats
*/
typedef struct
{
	///name
	const char*				StatName;

	///type
	ApexStatDataType::Enum	StatType;

	///current value
	ApexStatValue			StatCurrentValue;
} ApexStatsInfo;

/**
\brief Per scene statistics
*/
struct NxApexSceneStats
{
	/**
	\brief The number of actors in the scene
	*/
//todo remove	physx::PxU32			nbActors;
	/**\brief The number of ApexStats structures stored.
	*/
	physx::PxU32			numApexStats;
	/**\brief Array of ApexStatsInfo structures.
	*/
	ApexStatsInfo*			ApexStatsInfoPtr;
};

/**
\brief Types of view matrices handled by APEX

USER_CUSTOMIZED : APEX simply uses the view matrix given. Need to setViewParams()
LOOK_AT_RH: APEX gets the eye direction and eye position based on this kind of matrix.
LOOK_AT_LH: APEX gets the eye direction and eye position based on this kind of matrix.

*/
struct ViewMatrixType
{
	/**
	\brief Enum of the view matrices types
	*/
	enum Enum
	{
		USER_CUSTOMIZED = 0,
		LOOK_AT_RH,
		LOOK_AT_LH,
	};
};

/**
\brief Types of projection matrices handled by APEX

USER_CUSTOMIZED : APEX simply uses the projection matrix given. Need to setProjParams()

*/
struct ProjMatrixType
{
	/**
	\brief Enum of the projection matrices types
	*/
	enum Enum
	{
		USER_CUSTOMIZED = 0,
	};
};

/**
\brief Enum of the bounding box types
*/
struct UserBoundingBoxFlags
{
	/**
	\brief Enum of the bounding box types
	*/
	enum Enum
	{
		NONE    = 1 << 0,
		ENTER   = 1 << 1,
		LEAVE   = 1 << 2
	};
};

/**
\brief An APEX wrapper for an NxScene
*/
class NxApexScene : public NxApexRenderable, public NxApexContext, public NxApexInterface
{
public:
	/**
	\brief Associate an NxScene with this NxApexScene.

	All NxApexActors in the NxApexSene will be added to the NxScene.
	The NxScene pointer can be NULL, which will cause all APEX actors to be removed
	from the previously specified NxScene.  This must be done before the NxScene
	can be released.
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual void setPhysXScene(NxScene* s) = 0;
#else
	virtual void setPhysXScene(PxScene* s) = 0;
#endif

	/**
	\brief Retrieve the NxScene associated with this NxApexScene
	*/
#if NX_SDK_VERSION_MAJOR == 2
	virtual NxScene* getPhysXScene() const = 0;
#else
	virtual PxScene* getPhysXScene() const = 0;
#endif

	/**
	\brief Retrieve scene statistics
	*/
	virtual const NxApexSceneStats* getStats(void) const = 0;

	/**
	\brief Particle rendering synchronization method

	This method switches the staging particle data over to the render buffers to support a single
	render buffer for non-interop and a double render buffer for D3D-CUDA interop.
	
	\note This function must be called outside of interval between simulate & fetchResults.
	*/
	virtual void prepareRenderResourceContexts() const = 0;

#if NX_SDK_VERSION_MAJOR == 2
	/**
	\brief Start simulation of the APEX (and PhysX) scene

	Start simulation of the NxApexActors and the NxScene associated with this NxApexScene.
	No NxApexActors should be added, deleted, or modified until fetchResults() is called.

	Calls to simulate() should pair with calls to fetchResults():
 	Each fetchResults() invocation corresponds to exactly one simulate()
 	invocation; calling simulate() twice without an intervening fetchResults()
 	or fetchResults() twice without an intervening simulate() causes an error
 	condition.
 
	\param[in] elapsedTime Amount of time to advance simulation by. <b>Range:</b> (0,inf)

	\param[in] finalStep should be left as true, unless your application is manually sub stepping APEX
	(and PhysX) and you do not intend to try to render the output of intermediate steps.

	\param[in] completionTask if non-NULL, this task will have its refcount incremented in simulate(), then
	decremented when the scene is ready to have fetchResults called. So the task will not run until the
	application also calls removeReference() after calling simulate.
	*/
	virtual void simulate(physx::PxF32 elapsedTime, 
						  bool finalStep = true, 
						  physx::PxBaseTask *completionTask = NULL) = 0;
#else

	/**
	\brief Start simulation of the APEX (and PhysX) scene

	Start simulation of the NxApexActors and the NxScene associated with this NxApexScene.
	No NxApexActors should be added, deleted, or modified until fetchResults() is called.

	Calls to simulate() should pair with calls to fetchResults():
 	Each fetchResults() invocation corresponds to exactly one simulate()
 	invocation; calling simulate() twice without an intervening fetchResults()
 	or fetchResults() twice without an intervening simulate() causes an error
 	condition.
 
	\param[in] elapsedTime Amount of time to advance simulation by. <b>Range:</b> (0,inf)

	\param[in] finalStep should be left as true, unless your application is manually sub stepping APEX
	(and PhysX) and you do not intend to try to render the output of intermediate steps.

	\param[in] completionTask if non-NULL, this task will have its refcount incremented in simulate(), then
	decremented when the scene is ready to have fetchResults called. So the task will not run until the
	application also calls removeReference() after calling simulate.

	\param[in] scratchMemBlock a memory region for physx to use for temporary data during simulation. This block may be reused by the application
	after fetchResults returns. Must be aligned on a 16-byte boundary

	\param[in] scratchMemBlockSize the size of the scratch memory block. Must be a multiple of 16K.
	*/
	virtual void simulate(physx::PxF32 elapsedTime, 
						  bool finalStep = true, 
						  physx::PxBaseTask *completionTask = NULL,
						  void* scratchMemBlock = 0, 
						  PxU32 scratchMemBlockSize = 0) = 0;
#endif

	/**
	\brief Checks, and optionally blocks, for simulation completion.  Updates scene state.

	Checks if NxApexScene has completed simulating (optionally blocking for completion).  Updates
	new state of NxApexActors and the NxScene.  Returns true if simulation is complete.

	\param block [in] - block until simulation is complete
	\param errorState [out] - error value is written to this address, if not NULL
	*/
	virtual bool fetchResults(bool block, physx::PxU32* errorState) = 0;

#if NX_SDK_VERSION_MAJOR == 2
	/**
	\brief Returns an NxDebugRenderable object that contains the data for debug rendering of this scene
	*/
	virtual const NxDebugRenderable* getDebugRenderable() const = 0;

	/**
	\brief Returns an NxDebugRenderable object that contains the data for debug rendering of this scene in screenspace
	*/
	virtual const NxDebugRenderable* getDebugRenderableScreenSpace() const = 0;

#endif

#if NX_SDK_VERSION_MAJOR == 3
	/**
	\brief Returns an NxDebugRenderable object that contains the data for debug rendering of this scene
	*/
	virtual const PxRenderBuffer* getRenderBuffer() const = 0;

	/**
	\brief Returns an NxDebugRenderable object that contains the data for debug rendering of this scene, in screenspace
	*/
	virtual const PxRenderBuffer* getRenderBufferScreenSpace() const = 0;

#endif


	/**
	\brief Checks, and optionally blocks, for simulation completion.

	Performs same function as fetchResults(), but does not update scene state.  fetchResults()
	must still be called before the next simulation step can begin.
	*/
	virtual bool checkResults(bool block) const = 0;

	/**
	\brief Set the resource budget for this scene.

	Sets a total resource budget that the LOD system will distribute among modules,
	and eventually among the objects of those modules.

	The resource is specified in an abstract 'resource unit' rather than any real world quantity.
	*/
	virtual void setLODResourceBudget(physx::PxF32 totalResource) = 0;

	/**
	\brief Returns the ammount of LOD resource consumed.

	Retrieves the amount of LOD resource consumed in the last simulation frame.  The system attempts
	to keep this below the value set in setLODResourceBudget().
	*/
	virtual physx::PxF32 getLODResourceConsumed() const = 0;

	/**
	\brief Allocate a view matrix. Returns a viewID that identifies this view matrix
		   for future calls to setViewMatrix(). The matrix is de-allocated automatically
		   when the scene is released.

	Each call of this function allocates space for one view matrix. Since many features in
	APEX require a projection matrix it is _required_ that the application call this method.
	Max calls restricted to 1 for now.
	If ViewMatrixType is USER_CUSTOMIZED, setViewParams() as well using this viewID.
	If connected to PVD, PVD camera is set up.
	@see ViewMatrixType
	@see setViewParams()
	*/
	virtual physx::PxU32		allocViewMatrix(ViewMatrixType::Enum) = 0;

	/**
	\brief Allocate a projection matrix. Returns a projID that identifies this projection matrix
	       for future calls to setProjMatrix(). The matrix is de-allocated automatically
		   when the scene is released.

	Each call of this function allocates space for one projection matrix.  Since many features in
	APEX require a projection matrix it is _required_ that the application call this method.
	Max calls restricted to 1 for now.
	If ProjMatrixType is USER_CUSTOMIZED, setProjParams() as well using this projID
	@see ProjMatrixType
	@see setProjParams()
	*/
	virtual physx::PxU32			allocProjMatrix(ProjMatrixType::Enum) = 0;

	/**
	\brief Returns the number of view matrices allocated.
	*/
	virtual physx::PxU32			getNumViewMatrices() const = 0;

	/**
	\brief Returns the number of projection matrices allocated.
	*/
	virtual physx::PxU32			getNumProjMatrices() const = 0;

	/**
	\brief Sets the view matrix for the given viewID. Should be called whenever the view matrix needs to be updated.

	If the given viewID's matrix type is identifiable as indicated in ViewMatrixType, eye position and eye direction are set as well, using values from this matrix.
	Otherwise, make a call to setViewParams().
	If connected to PVD, PVD camera is updated.
	*/
	virtual void					setViewMatrix(const physx::PxMat44& viewTransform, const physx::PxU32 viewID = 0) = 0;

	/**
	\brief Returns the view matrix set by the user for the given viewID.

	@see setViewMatrix()
	*/
	virtual physx::PxMat44			getViewMatrix(const physx::PxU32 viewID = 0) const = 0;

	/**
	\brief Sets the projection matrix for the given projID. Should be called whenever the projection matrix needs to be updated.

	Make a call to setProjParams().
	@see setProjParams()
	*/
	virtual void					setProjMatrix(const physx::PxMat44& projTransform, const physx::PxU32 projID = 0) = 0;

	/**
	\brief Returns the projection matrix set by the user for the given projID.

	@see setProjMatrix()
	*/
	virtual physx::PxMat44			getProjMatrix(const physx::PxU32 projID = 0) const = 0;

	/**
	\brief Sets the use of the view matrix and projection matrix as identified by their IDs. Should be called whenever either matrices needs to be updated.
	*/
	virtual void					setUseViewProjMatrix(const physx::PxU32 viewID = 0, const physx::PxU32 projID = 0) = 0;

	/**
	\brief Sets the necessary information for the view matrix as identified by its viewID. Should be called whenever any of the listed parameters needs to be updated.

	@see ViewMatrixType
	*/
	virtual void					setViewParams(const physx::PxVec3& eyePosition, const physx::PxVec3& eyeDirection, const physx::PxVec3& worldUpDirection = PxVec3(0, 1, 0), const physx::PxU32 viewID = 0) = 0;

	/**
	\brief Sets the necessary information for the projection matrix as identified by its projID. Should be called whenever any of the listed parameters needs to be updated.

	@see ProjMatrixType
	*/
	virtual void					setProjParams(physx::PxF32 nearPlaneDistance, physx::PxF32 farPlaneDistance, physx::PxF32 fieldOfViewDegree, physx::PxU32 viewportWidth, physx::PxU32 viewportHeight, const physx::PxU32 projID = 0) = 0;

	/**
	\brief Returns the world space eye position.

	@see ViewMatrixType
	@see setViewMatrix()
	*/
	virtual physx::PxVec3			getEyePosition(const physx::PxU32 viewID = 0) const = 0;

	/**
	\brief Returns the world space eye direction.

	@see ViewMatrixType
	@see setViewMatrix()
	*/
	virtual physx::PxVec3			getEyeDirection(const physx::PxU32 viewID = 0) const = 0;

	/**
	\brief Returns the APEX scene's task manager
	*/
	virtual physx::PxTaskManager* getTaskManager() const = 0;

	/**
	\brief Toggle the use of a debug renderable
	*/
	virtual void setUseDebugRenderable(bool state) = 0;

	/**
	\brief Gets debug rendering parameters from NxParameterized
	*/
	virtual ::NxParameterized::Interface* getDebugRenderParams() const = 0;

	/**
	\brief Gets module debug rendering parameters from NxParameterized
	*/
	virtual ::NxParameterized::Interface* getModuleDebugRenderParams(const char* name) const = 0;

#if NX_SDK_VERSION_MAJOR == 3
	/**
	\brief Acquire the PhysX scene lock for read access.
	
	The PhysX 3.2.2 SDK (and higher) provides a multiple-reader single writer mutex lock to coordinate	
	access to the PhysX SDK API from multiple concurrent threads.  This method will in turn invoke the
	lockRead call on the PhysX Scene.  The source code fileName and line number a provided for debugging
	purposes.
	*/
	virtual void lockRead(const char *fileName,PxU32 lineo) = 0;

	/**
	\brief Acquire the PhysX scene lock for write access.
	
	The PhysX 3.2.2 SDK (and higher) provides a multiple-reader single writer mutex lock to coordinate	
	access to the PhysX SDK API from multiple concurrent threads.  This method will in turn invoke the
	lockWrite call on the PhysX Scene.  The source code fileName and line number a provided for debugging
	purposes.
	*/
	virtual void lockWrite(const char *fileName,PxU32 lineno) = 0;

	/**
	\brief Release the PhysX scene read lock
	*/
	virtual void unlockRead() = 0;

	/**
	\brief Release the PhysX scene write lock
	*/
	virtual void unlockWrite() = 0;
#else
	/**
	\brief Acquire the PhysX scene lock
	
	While a simulation is running, PhysX 3 scenes support read and write access to objects in the scene.
	Concurrent access from the application and APEX scene (after simulate has returned and before fetchResults is called) 
	might corrupt the state of the objects or lead to data races or inconsistent views in the simulation code.
	When accessing the PhysX scene the APEX scene will aquire the PhysX lock, and so should the application.
	
	For more information on this PhysX 3 feature, please see the PhysX documentation regarding data access.
	Note, this method will be deprecated under PhysX 3.3 which will provide a multiple-reader-single-writer mutex
	instead to co-ordinate access to the PhysX API from multiple threads and multiple libraries.
	*/
	virtual void					acquirePhysXLock(void) = 0;

	/**
	\brief Release the PhysX scene lock
	*/
	virtual void					releasePhysXLock(void) = 0;
#endif

#if NX_SDK_VERSION_MAJOR == 3
	/**
	\brief Allows the application to specify a pair of PhysX actors for the purposes of collision filtering.

	The set of methods addActorPair/removeActorPair and findActorPair can be used to implement collision filtering.
	This is a feature typically required to implement ragdoll systems to prevent nearby bodies from generating 
	contacts which create jitter in the simulation; causing artifacts and preventing a ragdoll from coming to rest.
	
	These methods are not required but are helper methods available if needed by the application.

	\param actor0 The first actor in the actor pair to consider
	\param actor1 The second actor in the actor pair to consider for filtering
	*/
	virtual void	addActorPair(PxActor *actor0,PxActor *actor1) = 0;

	/**
	\brief Removes a previously specified pair of actors from the actor-pair filter table

	\param actor0 The first actor in the actor pair to remove
	\param actor1 The second actor in the actor pair to remove
	*/
	virtual void	removeActorPair(PxActor *actor0,PxActor *actor1) = 0;

	/**
	\brief This method is used to determine if two actor pairs match.

	If actor0 and actor1 were previously added to the actor-pair filter table, then this method will return true.  
	Order is not important, actor0+actor1 will return the same result as actor1+actor0

	\param actor0 The first actor to consider
	\param actor1 The second actor to consider

	\return Returns true if the two actors match false if they have not been previously defined.
	*/
	virtual bool	findActorPair(PxActor *actor0,PxActor *actor1) const = 0;
#endif

#if NX_SDK_VERSION_MAJOR == 3
	/**
	\brief Returns an NxMirrorScene helper class which keeps a primary APEX scene mirrored into a secondary scene.


	\param mirrorScene [in] - The APEX scene that this scene will be mirrored into.
	\param mirrorFilter [in] - The application provided callback interface to filter which actors/shapes get mirrored.
	\param mirrorStaticDistance [in] - The distance to mirror static objects from the primary scene into the mirrored scene.
	\param mirrorDynamicDistance [in] - The distance to mirror dynamic objects from the primary scene into the mirrored scene.
	\param mirrorRefreshDistance [in] - The distance the camera should have moved before revising the trigger shapes controlling which actors get mirrored.
	*/
	virtual NxMirrorScene *createMirrorScene(physx::apex::NxApexScene &mirrorScene,
		NxMirrorScene::MirrorFilter &mirrorFilter,
		physx::PxF32 mirrorStaticDistance,
		physx::PxF32 mirrorDynamicDistance,
		physx::PxF32 mirrorRefreshDistance) = 0;
#endif

	/**
	\brief Adds user-defined bounding box into the scene. Each module can use these bounding boxes in a module-specific way. See documentation on each module.
	Some modules might use it as a valid volume of simulation, deleting actors or parts of actors upon leaving a BB.


	\param bounds [in] - The bounding box in world coordinates.
	\param flags [in] - The flag for supplied bounding box.
	*/
	virtual void  addBoundingBox(const PxBounds3& bounds, UserBoundingBoxFlags::Enum flags) = 0;

	/**
	\brief Returns user-defined bounding box added previously. In case there is no bounding box for the given index, zero sized PxBounds3 is returned.


	\param index [in] - Index of the bounding box. User could acquire total number of bounding boxes using getBoundingBoxCount.
	*/
	virtual const PxBounds3 getBoundingBox(const PxU32 index) const = 0;

	/**
	\brief Returns user-defined bounding box flags. In case there is no bounding box (and its flags) for the given index, UserBoundingBoxFlags::NONE is returned.


	\param index [in] - Index of the bounding box. User could acquire total number of bounding boxes using getBoundingBoxCount.
	*/
	virtual UserBoundingBoxFlags::Enum getBoundingBoxFlags(const PxU32 index) const = 0;

	/**
	\brief Returns user-defined bounding box count.
	*/
	virtual PxU32 getBoundingBoxCount() const = 0;

	/**
	\brief Removes user-defined bounding box at index. In case index is invalid (there is no bounding box for this index) nothing is happenning.

	\param index [in] - Index of the bounding box. User could acquire total number of bounding boxes using getBoundingBoxCount.
	*/
	virtual void  removeBoundingBox(const PxU32 index) = 0;

	/**
	\brief Removed all of the user-specified bounding boxes.
	*/
	virtual void  removeAllBoundingBoxes() = 0;

#if defined(APEX_CUDA_SUPPORT)
	virtual void* getNxApexCudaTestManager() const = 0;
	virtual void* getNxApexCudaProfileManager() const = 0;

	/**
	\brief Enables/disables Cuda error check after each kernel launch. Use ONLY for DEBUG purposes, when enabled could dramatically SLOW down performance!
	*/
	virtual void setCudaKernelCheckEnabled(bool enabled) = 0;
	virtual bool getCudaKernelCheckEnabled() const = 0;
#endif

#if defined(APEX_TEST)
	virtual void setSeed(physx::PxU32 seed) = 0;
#endif
};

/**
\brief This helper class creates a scoped read access to the PhysX SDK API
	
This helper class is used to create a scoped read lock/unlock pair around a section of code
which is trying to do read access against the PhysX SDK.
*/
class ScopedPhysXLockRead
{
public:

	/**
	\brief Constructor for ScopedPhysXLockRead
	\param[in] scene the APEX scene
	\param[in] fileName used to determine what file called the lock for debugging purposes
	\param[in] lineno used to determine what line number called the lock for debugging purposes
	*/
	ScopedPhysXLockRead(physx::NxApexScene& scene,const char *fileName,int lineno) : mScene(&scene)
	{
#if NX_SDK_VERSION_MAJOR == 3
		mScene->lockRead(fileName, (physx::PxU32)lineno);
#else
		PX_UNUSED(fileName);
		PX_UNUSED(lineno);
		scene.acquirePhysXLock();
#endif
	}
	~ScopedPhysXLockRead()
	{
#if NX_SDK_VERSION_MAJOR == 3
		mScene->unlockRead();
#else
		mScene->releasePhysXLock();
#endif
	}
private:
	physx::NxApexScene* mScene;
};

/**
\brief This helper class creates a scoped write access to the PhysX SDK API
	
This helper class is used to create a scoped write lock/unlock pair around a section of code
which is trying to do read access against the PhysX SDK.
*/
class ScopedPhysXLockWrite
{
public:
	/**
	\brief Constructor for ScopedPhysXLockWrite
	\param[in] scene the APEX scene
	\param[in] fileName used to determine what file called the lock for debugging purposes
	\param[in] lineno used to determine what line number called the lock for debugging purposes
	*/
	ScopedPhysXLockWrite(physx::NxApexScene &scene,const char *fileName,int lineno) : mScene(&scene)
	{
#if NX_SDK_VERSION_MAJOR == 3
		mScene->lockWrite(fileName, (physx::PxU32)lineno);
#else
		PX_UNUSED(fileName);
		PX_UNUSED(lineno);
		mScene->acquirePhysXLock();
#endif
	}
	~ScopedPhysXLockWrite()
	{
#if NX_SDK_VERSION_MAJOR == 3
		mScene->unlockWrite();
#else
		mScene->releasePhysXLock();
#endif
	}
private:
	physx::NxApexScene* mScene;
};


PX_POP_PACK

}
} // end namespace physx::apex


#if defined(_DEBUG) || defined(PX_CHECKED)
/**
\brief This macro creates a scoped write lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_WRITE(x) physx::apex::ScopedPhysXLockWrite _wlock(x,__FILE__,__LINE__);
#else
/**
\brief This macro creates a scoped write lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_WRITE(x) physx::apex::ScopedPhysXLockWrite _wlock(x,"",0);
#endif

#if defined(_DEBUG) || defined(PX_CHECKED)
/**
\brief This macro creates a scoped read lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_READ(x) physx::apex::ScopedPhysXLockRead _rlock(x,__FILE__,__LINE__);
#else
/**
\brief This macro creates a scoped read lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_READ(x) physx::apex::ScopedPhysXLockRead _rlock(x,"",0);
#endif


#endif // NX_APEX_SCENE_H
