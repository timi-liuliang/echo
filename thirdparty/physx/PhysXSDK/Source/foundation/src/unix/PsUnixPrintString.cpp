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

#include "PsPrintString.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef PX_ANDROID
#include <android/log.h>
#endif


namespace physx
{
namespace shdfnd
{

void printString(const char* str)
{
#ifdef PX_ANDROID
	__android_log_print(ANDROID_LOG_INFO, "PsPrintString", "%s", str);
#else
	puts(str);
#endif
}

void printFormatted(const char* format, ...)
{
	char buf[PS_MAX_PRINTFORMATTED_LENGTH];

	va_list arg;
	va_start( arg, format );
	vsprintf( buf, format, arg );
	va_end(arg);

	printString(buf);
}

} // namespace shdfnd
} // namespace physx

