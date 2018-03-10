/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_MODULE_LEGACY_H
#define NX_MODULE_LEGACY_H

#include <NxApexUsingNamespace.h>

namespace physx
{
namespace apex
{

#if !defined(_USRDLL)
void instantiateModuleLegacy();
#endif

}
} // end namespace physx::apex

#endif // NX_MODULE_LEGACY_H
