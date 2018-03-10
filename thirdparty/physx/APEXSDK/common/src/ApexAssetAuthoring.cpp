/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexAssetAuthoring.h"

#include "P4Info.h"
#include "PsString.h"

#include "NxPhysXSDKVersion.h"
#include "NiApexSDK.h"



namespace physx
{
namespace apex
{


void ApexAssetAuthoring::setToolString(const char* toolName, const char* toolVersion, PxU32 toolChangelist)
{
#ifdef WITHOUT_APEX_AUTHORING
	PX_UNUSED(toolName);
	PX_UNUSED(toolVersion);
	PX_UNUSED(toolChangelist);
#else
	const PxU32 buflen = 256;
	char buf[buflen];
	string::strcpy_s(buf, buflen, toolName);
	string::strcat_s(buf, buflen, " ");

	if (toolVersion != NULL)
	{
		string::strcat_s(buf, buflen, toolVersion);
		string::strcat_s(buf, buflen, ":");
	}

	if (toolChangelist == 0)
	{
		toolChangelist = P4_TOOLS_CHANGELIST;
	}

	{
		char buf2[14];
		string::sprintf_s(buf2, 14, "CL %d", toolChangelist);
		string::strcat_s(buf, buflen, buf2);
		string::strcat_s(buf, buflen, " ");
	}
	
	{
#ifdef WIN64
		string::strcat_s(buf, buflen, "Win64 ");
#elif defined(WIN32)
		string::strcat_s(buf, buflen, "Win32 ");
#endif
	}

	{
		string::strcat_s(buf, buflen, "(Apex ");
		string::strcat_s(buf, buflen, P4_APEX_VERSION_STRING);
		char buf2[20];
		string::sprintf_s(buf2, 20, ", CL %d, ", P4_CHANGELIST);
		string::strcat_s(buf, buflen, buf2);
#ifdef _DEBUG
		string::strcat_s(buf, buflen, "DEBUG ");
#elif defined(PHYSX_PROFILE_SDK)
		string::strcat_s(buf, buflen, "PROFILE ");
#endif
		string::strcat_s(buf, buflen, P4_APEX_BRANCH);
		string::strcat_s(buf, buflen, ") ");
	}

	{
		string::strcat_s(buf, buflen, "(PhysX ");

		char buf2[10] = { 0 };
#if NX_SDK_VERSION_MAJOR == 2
		union
		{
			physx::PxU32 _unsigned;
			physx::PxU8 _chars[4];
		};
		_unsigned = NX_PHYSICS_SDK_VERSION;

		string::sprintf_s(buf2, 10, "%d.%d.%d) ", _chars[3], _chars[2], _chars[1]);
#elif NX_SDK_VERSION_MAJOR == 3
		string::sprintf_s(buf2, 10, "%d.%d) ", PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR);
#endif
		string::strcat_s(buf, buflen, buf2);
	}


	string::strcat_s(buf, buflen, "Apex Build Time: ");
	string::strcat_s(buf, buflen, P4_BUILD_TIME);
	
	string::strcat_s(buf, buflen, "Distribution author: ");
	string::strcat_s(buf, buflen, AUTHOR_DISTRO);
	
	string::strcat_s(buf, buflen, "The reason for the creation of the distribution: ");
	string::strcat_s(buf, buflen, REASON_DISTRO);

	//PxU32 len = strlen(buf);
	//len = len;

	//"<toolName> <toolVersion>:<toolCL> <platform> (Apex <apexVersion>, CL <apexCL> <apexConfiguration> <apexBranch>) (PhysX <physxVersion>) <toolBuildDate>"

	setToolString(buf);
#endif
}



void ApexAssetAuthoring::setToolString(const char* /*toolString*/)
{
	PX_ALWAYS_ASSERT();
	APEX_INVALID_OPERATION("Not Implemented.");
}

} // namespace apex
} // namespace physx