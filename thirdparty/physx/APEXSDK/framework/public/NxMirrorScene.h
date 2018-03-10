/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_MIRROR_SCENE_H

#define NX_MIRROR_SCENE_H

/*!
\file
\brief classes NxMirrorScene, NxMirrorScene::MirrorFilter
*/


#include "foundation/PxPreprocessor.h"
#include "NxApexDefs.h"

#if NX_SDK_VERSION_MAJOR == 3
namespace physx
{
	class PxActor;
	class PxShape;
	class PxVec3;

	namespace apex
	{

		/**
		\brief NxMirrorScene is used to create a selected mirrored copy of a primary scene.  Works only with PhysX 3.x
		*/
		class NxMirrorScene
		{
		public:
			/**
			\brief MirrorFilter is a callback interface implemented by the application to confirm which actors and shapes are, or are not, replicated into the mirrored scene
			*/
			class MirrorFilter
			{
			public:
				/**
				\brief The application returns true if this actor should be mirrored into the secondary mirrored scene.

				\param[in] actor A const reference to the actor in the primary scene to be considered mirrored into the secondary scene.
				*/
				virtual bool shouldMirror(const physx::PxActor &actor) = 0;

				/**
				\brief The application returns true if this shape should be mirrored into the secondary mirrored scene.

				\param[in] shape A const reference to the shape in the primary scene to be considered mirrored into the secondary scene.
				*/
				virtual bool shouldMirror(const physx::PxShape &shape) = 0;

				/**
				\brief Affords the application with an opportunity to modify the contents/state of the shape before is placed into the mirrored scene.

				\param[in] shape A reference to the shape that is about to be placed into the mirrored scene.
				*/
				virtual void reviseMirrorShape(physx::PxShape &shape) = 0;

				/**
				\brief Affords the application with an opportunity to modify the contents/state of the actor before is placed into the mirrored scene.

				\param[in] actor A reference to the actor that is about to be placed into the mirrored scene
				*/
				virtual void reviseMirrorActor(physx::PxActor &actor) = 0;
			};

			/**
			\brief SynchronizePrimaryScene updates the positions of the objects around the camera relative to the static and dynamic distances specified
			These objects are then put in a thread safe queue to be processed when the mirror scene synchronize is called

			\param[in] cameraPos The current position of the camera relative to where objects are being mirrored
			*/
			virtual void synchronizePrimaryScene(const physx::PxVec3 &cameraPos) = 0;

			/**
			\brief Processes the updates to get this mirrored scene to reflect the subset of the
			primary scene that is being mirrored.  Completely thread safe, assumes that
			the primary scene and mirrored scene are most likely being run be completely
			separate threads.
			*/
			virtual void synchronizeMirrorScene(void) = 0;

			/**
			\brief Releases the NxMirrorScene class and all associated mirrored objects; it is important to not that this does *not* release
			the actual APEX scnee; simply the NxMirrorScene helper class.
						*/
			virtual void release(void) = 0;

		};

	}; // end of apex namespace
}; // end of physx namespace

#endif // NX_SDK_VERSION_MAJOR

#endif
