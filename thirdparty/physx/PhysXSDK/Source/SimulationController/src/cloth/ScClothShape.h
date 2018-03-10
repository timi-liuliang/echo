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


#ifndef PX_PHYSICS_CLOTHSHAPE
#define PX_PHYSICS_CLOTHSHAPE

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_CLOTH_API

#include "ScElementSim.h"
#include "ScClothCore.h"

namespace physx
{
namespace Sc
{
	/**
	A collision detection primitive for cloth.
	*/
	class ClothShape : public ElementSim
	{
		ClothShape &operator=(const ClothShape &);
		public:
												ClothShape(ClothSim& cloth);
												~ClothShape();

		// Element implementation
		virtual		bool						isActive() const { return false; }
		// ~Element

		// ElementSim implementation
		virtual		void						getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const;
		// ~ElementSim

		public:
        PX_INLINE	PxBounds3					getWorldBounds() const { return mClothCore.getWorldBounds(); }
		PX_INLINE	ClothSim&	                getClothSim() const { return *mClothCore.getSim(); }

                    void                        updateBoundsInAABBMgr();

		private:
                    ClothCore&                  mClothCore;
					bool						mIsInBroadPhase;
	};

} // namespace Sc

}

#endif	// PX_USE_CLOTH_API

#endif
