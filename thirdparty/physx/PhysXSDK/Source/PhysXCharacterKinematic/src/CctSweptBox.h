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

#ifndef CCT_SWEPT_BOX
#define CCT_SWEPT_BOX

#include "CctSweptVolume.h"

namespace physx
{
namespace Cct
{

	class SweptBox : public SweptVolume
	{
	public:
						SweptBox();
		virtual			~SweptBox();

		virtual	void	computeTemporalBox(const SweepTest&, PxExtendedBounds3& box, const PxExtendedVec3& center, const PxVec3& direction) const;

				PxVec3	mExtents;
	};

} // namespace Cct

}

#endif
