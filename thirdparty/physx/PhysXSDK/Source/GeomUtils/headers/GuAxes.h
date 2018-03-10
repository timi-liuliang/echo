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

#ifndef GU_AXES_H
#define GU_AXES_H

#include "PxSimpleTypes.h"

namespace physx
{
namespace Gu
{
	enum PointComponent
	{
		X_					= 0,
		Y_					= 1,
		Z_					= 2,
		W_					= 3,

		PC_FORCE_DWORD		= 0x7fffffff
	};

	enum AxisOrder
	{
		AXES_XYZ			= (X_)|(Y_<<2)|(Z_<<4),
		AXES_XZY			= (X_)|(Z_<<2)|(Y_<<4),
		AXES_YXZ			= (Y_)|(X_<<2)|(Z_<<4),
		AXES_YZX			= (Y_)|(Z_<<2)|(X_<<4),
		AXES_ZXY			= (Z_)|(X_<<2)|(Y_<<4),
		AXES_ZYX			= (Z_)|(Y_<<2)|(X_<<4),

		AXES_FORCE_DWORD	= 0x7fffffff
	};

	class Axes
	{
		public:

		PX_FORCE_INLINE			Axes(AxisOrder order)
								{
									mAxis0 = PxU32((order   ) & 3);
									mAxis1 = PxU32((order>>2) & 3);
									mAxis2 = PxU32((order>>4) & 3);
								}
		PX_FORCE_INLINE			~Axes()		{}

						PxU32	mAxis0;
						PxU32	mAxis1;
						PxU32	mAxis2;
	};
}

}

#endif
