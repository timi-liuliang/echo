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


#ifndef PX_FOUNDATION_PS_WINDOWS_STRING_H
#define PX_FOUNDATION_PS_WINDOWS_STRING_H

#include "Ps.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#pragma warning(push)
#pragma warning(disable: 4996)

namespace physx
{
	namespace string
	{
		PX_INLINE PxI32 stricmp(const char *str, const char *str1) {return(::_stricmp(str, str1));}
		PX_INLINE PxI32 strnicmp(const char *str, const char *str1, size_t len) {return(::_strnicmp(str, str1, len));}
		PX_INLINE PxI32 strncat_s(char* a, PxI32 b, const char* c, size_t d) {return ::strncat_s(a, (rsize_t)b, c, d);}
		PX_INLINE PxI32 strncpy_s( char *strDest, size_t sizeInBytes, const char *strSource, size_t count) {return(::strncpy_s( strDest,sizeInBytes,strSource, count));}
		PX_INLINE void strcpy_s(char* dest, size_t size, const char* src) {::strcpy_s(dest, size, src);}
		PX_INLINE void strcat_s(char* dest, size_t size, const char* src) {::strcat_s(dest, size, src);}
		PX_INLINE PxI32 _vsnprintf(char* dest, size_t size, const char* src, va_list arg)
		{
			PxI32 r = ::_vsnprintf(dest, size, src, arg);

			return r;
		}
		PX_INLINE PxI32 vsprintf_s(char* dest, size_t size, const char* src, va_list arg)
		{
			PxI32 r = ::vsprintf_s(dest, size, src, arg);

			return r;
		}

		PX_INLINE PxI32 sprintf_s( char * _DstBuf, size_t _DstSize, const char * _Format, ...)
		{
			va_list arg;
			va_start( arg, _Format );
			PxI32 r = ::vsprintf_s(_DstBuf, _DstSize, _Format, arg);
			va_end(arg);

			return r;
		}
		PX_INLINE PxI32 sscanf_s( const char *buffer, const char *format,  ...)
		{
			va_list arg;
			va_start( arg, format );
			PxI32 r = ::sscanf_s(buffer, format, arg);
			va_end(arg);

			return r;
		};

		PX_INLINE void strlwr(char* str)
		{
			while ( *str )
			{
				if ( *str>='A' &&  *str<='Z' ) *str+=32;
				str++;
			}
		}

		PX_INLINE void strupr(char* str)
		{
			while ( *str )
			{
				if ( *str>='a' &&  *str<='z' ) *str-=32;
				str++;
			}
		}

	}
} // namespace physx

#pragma warning(pop)

#endif

