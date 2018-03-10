/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_ASSET_AUTHORING_H
#define APEX_ASSET_AUTHORING_H

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{


class ApexAssetAuthoring
{
public:
	virtual void setToolString(const char* toolName, const char* toolVersion, PxU32 toolChangelist);

	virtual void setToolString(const char* toolString);
};

} // namespace apex
} // namespace physx

#endif // APEX_ASSET_AUTHORING_H