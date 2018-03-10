/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PS_SHARE_H

#define PS_SHARE_H

#include "foundation/PxSimpleTypes.h"
#include "Ps.h"

namespace physx
{
	namespace general_PxIOStream2
	{
		class PxFileBuf;
	}
	using namespace general_PxIOStream2;
	namespace apex{}
	using namespace shdfnd;
	using namespace apex;
};

#ifdef APEX_SHIPPING

#ifndef PX_WINDOWS
#define WITHOUT_APEX_AUTHORING
#endif

#define WITHOUT_DEBUG_VISUALIZE
#define WITHOUT_PVD

#endif

#endif
