/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef COOKING_PHYS_X_H
#define COOKING_PHYS_X_H

#include "CookingAbstract.h"

namespace physx
{
namespace apex
{
template<class T>
class NxParamArray;

namespace clothing
{


class CookingPhysX : public CookingAbstract
{
public:

	virtual NxParameterized::Interface* execute();

	static PxU32 getCookingVersion()
	{
#if NX_SDK_VERSION_MAJOR == 2
		return NX_SDK_VERSION_NUMBER;
#else
		return NX_PHYSICS_SDK_VERSION;
#endif
	}

protected:
};

}
} // namespace apex
} // namespace physx


#endif // COOKING_PHYS_X_H
