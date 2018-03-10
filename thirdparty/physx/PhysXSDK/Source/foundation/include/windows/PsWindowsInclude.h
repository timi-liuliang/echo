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


#ifndef WINDOWSINCLUDE_H
#define WINDOWSINCLUDE_H

#include "Ps.h"

#ifndef _WIN32
	#error "This file should only be included by Windows builds!!"
#endif

#ifdef _WINDOWS_		// windows already included
	#error "Only include windows.h through this file!!"
#endif

// We only support >= Windows XP, and we need this for critical section and 
#ifndef PX_WINMODERN
#define _WIN32_WINNT 0x0501
#else
#define _WIN32_WINNT 0x0602 
#endif

//turn off as much as we can for windows. All we really need is the thread functions(critical sections/Interlocked* etc)
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define WIN32_LEAN_AND_MEAN
#ifndef PX_WINMODERN
#	define NOUSER
#	define NONLS
#	define NOMSG
#endif

#pragma warning (push)
#pragma warning (disable : 4668) //'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#include <windows.h>
#pragma warning (pop)

#ifdef PX_SUPPORT_SSE
	#include <xmmintrin.h>
#endif

#endif
