/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_INTERFACE_H
#define NX_APEX_INTERFACE_H

/*!
\file
\brief class NxApexInterface
*/

#include "NxApexUsingNamespace.h"
#include "NxApexRWLockable.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Base class for all interface classes implemented by APEX SDK

User callback interfaces should not derive from this class.
*/
class NxApexInterface : public NxApexRWLockable
{
public:
	NxApexInterface() : userData(NULL) {}

	/**
	\brief Release an object instance.

	Calling this will unhook the class and delete it from memory.
	You should not keep any reference to this class instance after calling release
	*/
	virtual void release() = 0;

	/**
	\brief User data variable.

	Practical in scenarios where this object has a clear and unique owner,
	not so useful in other scenarios.
	*/
	void* userData;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_INTERFACE_H
