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

#ifndef CCT_CHARACTER_CONTROLLER_MANAGER
#define CCT_CHARACTER_CONTROLLER_MANAGER

//Exclude file from docs
/** \cond */

#include "PxControllerManager.h"
#include "PxControllerObstacles.h"
#include "PxMeshQuery.h"
#include "CmRenderOutput.h"
#include "CctUtils.h"
#include "PsHashSet.h"

namespace physx
{
namespace Cct
{
	class Controller;
	class ObstacleContext;

	//Implements the PxControllerManager interface, this class used to be called ControllerManager
	class CharacterControllerManager : public PxControllerManager   , public Ps::UserAllocated
	{
	public:
														CharacterControllerManager(PxScene& scene, bool lockingEnabled = false);
		virtual											~CharacterControllerManager();

		// PxControllerManager
		virtual			void							release();
		virtual			PxScene&						getScene() const;
		virtual			PxU32							getNbControllers()	const;
		virtual			PxController*					getController(PxU32 index);
        virtual			PxController*					createController(const PxControllerDesc& desc);
       
        PX_DEPRECATED virtual PxController*		        createController(PxPhysics& physics, PxScene* scene, const PxControllerDesc& desc)
	    {
		    return PxControllerManager::createController(physics, scene, desc);
	    }
		virtual			void							purgeControllers();
		virtual			PxRenderBuffer&					getRenderBuffer();
		virtual			void							setDebugRenderingFlags(PxControllerDebugRenderFlags flags);
		virtual			PxU32							getNbObstacleContexts() const;
		virtual			PxObstacleContext*				getObstacleContext(PxU32 index);
		virtual			PxObstacleContext*				createObstacleContext();
		virtual			void							computeInteractions(PxF32 elapsedTime, PxControllerFilterCallback* cctFilterCb);
		virtual			void							setTessellation(bool flag, float maxEdgeLength);
		virtual			void							setOverlapRecoveryModule(bool flag);
		virtual			void							setPreciseSweeps(bool flag);
		virtual			void							setPreventVerticalSlidingAgainstCeiling(bool flag);
		virtual			void							shiftOrigin(const PxVec3& shift);		
		//~PxControllerManager

		// ObstacleContextNotifications
						void							onObstacleRemoved(ObstacleHandle index) const;
						void							onObstacleUpdated(ObstacleHandle index, const PxObstacleContext* ) const;
						void							onObstacleAdded(ObstacleHandle index, const PxObstacleContext*) const;

						void							releaseController(PxController& controller);
						Controller**					getControllers();
						void							releaseObstacleContext(ObstacleContext& oc);
						void							resetObstaclesBuffers();

						PxScene&						mScene;

						Cm::RenderBuffer*				mRenderBuffer;
						PxControllerDebugRenderFlags	mDebugRenderingFlags;
		// Shared buffers for obstacles
						Ps::Array<const void*>			mBoxUserData;
						Ps::Array<PxExtendedBox>		mBoxes;

						Ps::Array<const void*>			mCapsuleUserData;
						Ps::Array<PxExtendedCapsule>	mCapsules;

						Ps::Array<Controller*>			mControllers;
						Ps::HashSet<PxShape*>			mCCTShapes;

						Ps::Array<ObstacleContext*>		mObstacleContexts;

						float							mMaxEdgeLength;
						bool							mTessellation;

						bool							mOverlapRecovery;
						bool							mPreciseSweeps;
						bool							mPreventVerticalSlidingAgainstCeiling;

						bool							mLockingEnabled;
	protected:
		CharacterControllerManager &operator=(const CharacterControllerManager &);
	};

} // namespace Cct

}

/** \endcond */
#endif //CCT_CHARACTER_CONTROLLER_MANAGER
