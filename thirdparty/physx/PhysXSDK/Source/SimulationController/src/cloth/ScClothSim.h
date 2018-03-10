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


#ifndef PX_PHYSICS_SCP_CLOTH_SIM
#define PX_PHYSICS_SCP_CLOTH_SIM

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_CLOTH_API

#include "CmMatrix34.h"
#include "ScClothShape.h"

namespace physx
{

#if PX_SUPPORT_GPU_PHYSX
	struct PxClothCollisionSphere;
#endif

namespace Sc
{
	class ClothCore;
	class ShapeSim;

	class ClothSim : public ActorSim
	{
	public:

		ClothSim(Scene&, ClothCore&);
		~ClothSim();

		//---------------------------------------------------------------------------------
		// Actor implementation
		//---------------------------------------------------------------------------------
	public:
		ClothCore& getCore() const;

		void updateBounds();
		void startStep();
		void reinsert();

		bool addCollisionShape(const ShapeSim* shape);
		void removeCollisionShape(const ShapeSim* shape);

		bool addCollisionSphere(const ShapeSim* shape);
		void removeCollisionSphere(const ShapeSim* shape);

		bool addCollisionCapsule(const ShapeSim* shape);
		void removeCollisionCapsule(const ShapeSim* shape);

		bool addCollisionPlane(const ShapeSim* shape);
		void removeCollisionPlane(const ShapeSim* shape);

		bool addCollisionBox(const ShapeSim* shape);
		void removeCollisionBox(const ShapeSim* shape);

		bool addCollisionConvex(const ShapeSim* shape);
		void removeCollisionConvex(const ShapeSim* shape);

		bool addCollisionMesh(const ShapeSim* shape);
		void removeCollisionMesh(const ShapeSim* shape);

		bool addCollisionHeightfield(const ShapeSim* shape);
		void removeCollisionHeightfield(const ShapeSim* shape);

		void updateRigidBodyPositions();
		void clearCollisionShapes();

	private:
		void insertShapeSim(PxU32, const ShapeSim*);
		ClothSim &operator=(const ClothSim &);

	private:
        ClothShape mClothShape;

		PxU32 mNumSpheres;
		PxU32 mNumCapsules;
		PxU32 mNumPlanes;
		PxU32 mNumBoxes;
		PxU32 mNumConvexes;
		PxU32 mNumMeshes;
		PxU32 mNumHeightfields;
		PxU32 mNumConvexPlanes;

		shdfnd::Array<const ShapeSim*> mShapeSims;
		shdfnd::Array<Cm::Matrix34> mStartShapeTrafos;

	};

} // namespace Sc
}

#endif	// PX_PHYSICS_SCP_CLOTH_SIM

#endif
