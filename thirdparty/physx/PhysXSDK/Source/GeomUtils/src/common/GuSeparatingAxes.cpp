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

#include "GuSeparatingAxes.h"

using namespace physx;

union FloatInt
{
	float	f;
	PxU32	i;
};

bool Gu::SeparatingAxes::addAxis(const PxVec3& axis)
{
#ifdef _XBOX
	// PT: this version has no FCMps but LHS instead - 4 ticks
	const float val = 0.9999f;
	const PxU32 Limit = (PxU32&)val;

	size_t numAxes = mNbAxes;
	const PxVec3* PX_RESTRICT axes = mAxes;
	const PxVec3* PX_RESTRICT axes_end = axes + numAxes;

	FloatInt batch[16];
	while(axes<axes_end)
	{
		const PxU32 nbToGo = numAxes>=16 ? 16 : numAxes;
		numAxes -= nbToGo;

		for(PxU32 i=0;i<nbToGo;i++)
			batch[i].f = axis.dot(axes[i]);
		axes += nbToGo;

		for(PxU32 i=0;i<nbToGo;i++)
		{
			if( (batch[i].i & ~PX_SIGN_BITMASK) > Limit)
				return false;
		}
	}
#else
	// PT: this version has FCMPs but no LHS - 5 ticks
	PxU32 numAxes = getNumAxes();
	const PxVec3* PX_RESTRICT axes = getAxes();
	const PxVec3* PX_RESTRICT axes_end = axes + numAxes;
	while(axes<axes_end)
	{
		if(PxAbs(axis.dot(*axes))>0.9999f)
			return false;
		axes++;
	}
#endif

#ifdef SEP_AXIS_FIXED_MEMORY
	if(mNbAxes<SEP_AXIS_FIXED_MEMORY)
	{
		mAxes[mNbAxes++] = axis;
		return true;
	}

	return false;
#else
	mAxes.pushBack(axis);
	return true;
#endif
}
