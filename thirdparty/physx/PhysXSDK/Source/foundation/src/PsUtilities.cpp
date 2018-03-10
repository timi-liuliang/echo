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


#include "foundation/PxMat33.h"
#include "foundation/PxQuat.h"
#include "foundation/PxTransform.h"
#include "PsUtilities.h"
#include "PsUserAllocated.h"
#include "PsFPU.h"

namespace physx
{
namespace shdfnd
{

bool checkValid(const float& f)
{
	return PxIsFinite(f);
}
bool checkValid(const PxVec3& v)
{
	return PxIsFinite(v.x) && PxIsFinite(v.y) && PxIsFinite(v.z);
}

bool checkValid(const PxTransform& t)
{
	return checkValid(t.p) && checkValid(t.q);
}

bool checkValid(const PxQuat& q)
{
	return PxIsFinite(q.x) && PxIsFinite(q.y) && PxIsFinite(q.z) && PxIsFinite(q.w);
}
bool checkValid(const PxMat33& m)
{
	return PxIsFinite(m(0,0)) && PxIsFinite(m(1,0)) && PxIsFinite(m(2,0)) && 
		PxIsFinite(m(0,1)) && PxIsFinite(m(1,1)) && PxIsFinite(m(2,1)) &&
		PxIsFinite(m(0,3)) && PxIsFinite(m(1,3)) && PxIsFinite(m(2,3));
}
bool checkValid(const char* string)
{
	static const PxU32 maxLength = 4096;
#if defined(PX_GNUC) && !defined(PX_GHS) // no strnlen on PS3
	for(const char* end=string+maxLength; string<end; ++string)
		if(*string)
			return true;
	return false;
#else
	return strnlen(string, maxLength) != maxLength;
#endif
}

} // namespace shdfnd
} // namespace physx
