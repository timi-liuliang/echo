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

#ifndef CCT_SWEPT_VOLUME
#define CCT_SWEPT_VOLUME

#include "CctUtils.h"

namespace physx
{
namespace Cct
{

	struct SweptVolumeType
	{
		enum Enum
		{
			eBOX,
			eCAPSULE,

			eLAST
		};
	};

	class SweepTest;

	class SweptVolume
	{
		public:
												SweptVolume();
		virtual									~SweptVolume();

		virtual			void					computeTemporalBox(const SweepTest& test, PxExtendedBounds3& box, const PxExtendedVec3& center, const PxVec3& direction)	const	= 0;

		PX_FORCE_INLINE	SweptVolumeType::Enum	getType()	const	{ return mType;	}

						PxExtendedVec3			mCenter;
						PxF32					mHalfHeight;	// UBI
		protected:
						SweptVolumeType::Enum	mType;
	};

	void computeTemporalBox(PxExtendedBounds3& _box, float radius, float height, float contactOffset, float maxJumpHeight, const PxVec3& upDirection, const PxExtendedVec3& center, const PxVec3& direction);

} // namespace Cct

}

#endif

