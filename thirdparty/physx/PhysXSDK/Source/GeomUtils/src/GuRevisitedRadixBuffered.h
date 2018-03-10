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

#ifndef GU_RADIX_SORT_BUFFERED_H
#define GU_RADIX_SORT_BUFFERED_H

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "GuRevisitedRadix.h"

namespace physx
{
namespace Gu
{
	class PX_PHYSX_COMMON_API RadixSortBuffered : public RadixSort
	{
	public:
							RadixSortBuffered();
							~RadixSortBuffered();

		RadixSortBuffered&	Sort(const PxU32* input, PxU32 nb, RadixHint hint=RADIX_SIGNED);
		RadixSortBuffered&	Sort(const float* input, PxU32 nb);

	private:
							RadixSortBuffered(const RadixSortBuffered& object);
							RadixSortBuffered& operator=(const RadixSortBuffered& object);

		// Internal methods
		void				CheckResize(PxU32 nb);
		bool				Resize(PxU32 nb);
	};
}

}

#endif // GU_RADIX_SORT_BUFFERED_H
