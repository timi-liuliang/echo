/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexSDKVersionString.h"
#include "P4Info.h"
#include "PsUtilities.h"

namespace physx
{
namespace apex
{

const char*	NxGetApexSDKVersionString(NxApexSDKVersionString versionString)
{
	const char* result = NULL;
	switch(versionString)
	{
		case VERSION:
			result = P4_APEX_VERSION_STRING;
		break;
		case CHANGELIST:
			result = PX_STRINGIZE(P4_CHANGELIST);
		break;
		case TOOLS_CHANGELIST:
			result = PX_STRINGIZE(P4_TOOLS_CHANGELIST);
		break;
		case BRANCH:
			result = P4_APEX_BRANCH;
		break;
		case BUILD_TIME:
			result = P4_BUILD_TIME;
		break;
		case AUTHOR:
			result = AUTHOR_DISTRO;
		break;
		case REASON:
			result = REASON_DISTRO;
		break;
		default:
		break;
	}
	return result;
}

}
}
