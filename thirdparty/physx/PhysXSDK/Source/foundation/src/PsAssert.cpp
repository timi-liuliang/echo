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

#include "PxAssert.h"

#include <stdio.h>
#include "PsPrintString.h"

#if defined(PX_WINDOWS)
#include <crtdbg.h>
#elif defined(PX_PS3)
#include "ps3/PsPS3Abort.h"
#elif defined(PX_WIIU)
#include "wiiu/PsWiiUAbort.h"
#endif

namespace
{
	class DefaultAssertHandler : public physx::PxAssertHandler
	{
		virtual void operator()(const char* expr, const char* file, int line, bool& ignore)
		{
			PX_UNUSED(ignore); // is used only in debug windows config
			char buffer[1024];
			sprintf(buffer, "%s(%d) : Assertion failed: %s\n", file, line, expr);
			physx::shdfnd::printString(buffer);
	#if defined(PX_WINDOWS) && defined(PX_DEBUG)
			// _CrtDbgReport returns -1 on error, 1 on 'retry', 0 otherwise including 'ignore'. 
			// Hitting 'abort' will terminate the process immediately.
			int result = _CrtDbgReport(_CRT_ASSERT, file, line, NULL, "%s", buffer);
			int mode = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_REPORT_MODE);
			ignore = _CRTDBG_MODE_WNDW == mode && result == 0;
			if(ignore)
				return;
			__debugbreak();
	#elif defined(PX_WINDOWS) && defined(PX_CHECKED)
			__debugbreak();
	#elif defined(PX_PS3)
			pause();
	#elif defined(PX_WIIU)
			abort(buffer);
	#else
			abort();
	#endif
		}
	};

	DefaultAssertHandler sAssertHandler;
	physx::PxAssertHandler* sAssertHandlerPtr = &sAssertHandler;
}

namespace physx
{
	PxAssertHandler& PxGetAssertHandler()
	{
		return *sAssertHandlerPtr;
	}

	void PxSetAssertHandler(PxAssertHandler& handler)
	{
		sAssertHandlerPtr = &handler;
	}
}
