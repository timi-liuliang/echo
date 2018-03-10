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
#pragma warning (push)
#pragma warning (disable : 4668) //'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#include <windows.h>
#pragma warning (pop)

namespace physx
{
namespace shdfnd
{

void printString(const char* str)
{
	puts(str); // do not use printf here, since str can contain multiple % signs that will not be printed
	OutputDebugStringA(str);
	OutputDebugStringA("\n");
}

void printFormatted(const char* format, ...)
{
	char buf[PS_MAX_PRINTFORMATTED_LENGTH];

	va_list arg;
	va_start( arg, format );
	vsprintf_s( buf, PS_MAX_PRINTFORMATTED_LENGTH, format, arg );
	va_end(arg);

	printString(buf);
}

} // namespace shdfnd
} // namespace physx

