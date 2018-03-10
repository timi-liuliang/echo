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


#ifndef PX_FOUNDATION_PS_UNIX_STRING_H
#define PX_FOUNDATION_PS_UNIX_STRING_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "Ps.h"
#include "foundation/PxMath.h"

#pragma warning(push)
#pragma warning(disable: 4996)

namespace physx
{
	namespace string
	{

		//PX_INLINE int strcasecmp(const char *str, const char *str1) {return(::strcasecmp(str, str1));}

		PX_INLINE PxI32 stricmp(const char *str, const char *str1) {return(::strcasecmp(str, str1));}
		PX_INLINE PxI32 strnicmp(const char *str, const char *str1, size_t len) {return(::strncasecmp(str, str1, len));}

		PX_INLINE PxI32 strncat_s(char *strDest, size_t size, const char *strSource, size_t count)
		{
			/*
			strncat_s tries to append the first D characters of strSource to the end of strDest,
			where D is the lesser of count and the length of strSource. If appending those D characters
			will fit within strDest (whose size is given as numberOfElements) and still leave room
			for a null terminator, then those characters are appended, starting at the original terminating
			null of strDest, and a new terminating null is appended; otherwise, strDest[0] is set to
			the null character
			*/

			size_t d, destStringLen, srcStringLen;

			if (	strDest		== NULL ||
					strSource	== NULL ||
					size		== 0	)
			{
				return -1;
			}

			srcStringLen	= PxMin<size_t>(count, strlen(strSource));
			destStringLen	= strlen(strDest);
			d				= srcStringLen + destStringLen;

			if ( size <= d )
			{
				strDest[0] = '\0';
				return -1;
			}

			::memcpy( &strDest[destStringLen], strSource, srcStringLen );
			strDest[d] = '\0';

			return 0;
		}

		PX_INLINE PxI32 _vsnprintf(char* dest, size_t size, const char* src, va_list arg)
		{
			PX_UNUSED(size);
			PxI32 r = ::vsnprintf(dest, size, src, arg);
			return r;
		}

		PX_INLINE PxI32 vsprintf(char* dest, size_t size, const char* src, va_list arg)
		{
			PX_UNUSED(size);
			PxI32 r = ::vsprintf(dest, src, arg);
			return r;
		}

		PX_INLINE int vsprintf_s(char* dest, size_t size, const char* src, va_list arg)
		{
			PX_UNUSED(size);
			int r = ::vsprintf( dest, src, arg );
			return r;
		}

		PX_INLINE int sprintf_s( char * _DstBuf, size_t _DstSize, const char * _Format, ...)
		{
			PX_UNUSED(_DstSize);
			if ( _DstBuf == NULL || _Format == NULL )
			{
				return -1;
			}

			va_list arg;
			va_start( arg, _Format );
			int r = ::vsprintf( _DstBuf, _Format, arg );
			va_end(arg);

			return r;
		}

		PX_INLINE PxI32 sprintf( char * _DstBuf, size_t _DstSize, const char * _Format, ...)
		{
			PX_UNUSED(_DstSize);
			va_list arg;
			va_start( arg, _Format );
			PxI32 r = ::vsprintf(_DstBuf, _Format, arg);
			va_end(arg);

			return r;
		}

		PX_INLINE int strncpy_s(char *strDest, size_t sizeInBytes, const char *strSource, size_t count)
		{
			/*
			From Microsoft help: strncpy_s will try to copy the first d characters of strSource to strDest,
			where d is the lesser of count and the length of strSource. If those d characters will fit within
			strDest (whose size is given as sizeInBytes) and still leave room for a null terminator,
			then those characters are copied and a terminating null is appended; otherwise, strDest[0] is
			set to the null character
			*/

			if (	strDest		== NULL ||
					strSource	== NULL ||
					sizeInBytes == 0	)
			{
				return -1;
			}
			size_t srcLen =  strlen(strSource);
			size_t d = PxMin<size_t>(count, srcLen);

			if ( sizeInBytes <= d )
			{
				strDest[0] = 0;
				return -1;
			}

			::memcpy( strDest, strSource, d );
			strDest[d] = '\0';

			return 0;
		}

		PX_INLINE int strcpy_s(char* dest, size_t size, const char* src)
		{
			/*
			The strcpy_s function copies the contents in the address of src, including the 
			terminating null character, to the location specified by dest. 
			The destination string must be large enough to hold the source string, including the 
			terminating null character.
			*/
			size_t d;

			if (	dest		== NULL ||
					src			== NULL ||
					size		== 0	)
			{
				return -1;
			}

			d = strlen(src);

			if ( size <= d )
			{
				dest[0] = 0;
				return -1;
			}

			::memcpy( dest, src, d );
			dest[d] = '\0';

			return 0;
		}

		PX_INLINE int strcat_s(char* dest, size_t size, const char* src)
		{
			size_t d, destStringLen, srcStringLen;

			if (	dest		== NULL ||
					src			== NULL ||
					size		== 0	)
			{
				return -1;
			}

			destStringLen	= strlen(dest);
			srcStringLen	= strlen(src);
			d				= srcStringLen + destStringLen;

			if ( size <= d )
			{
				return -1;
			}

			::memcpy( &dest[destStringLen], src, srcStringLen );
			dest[d] = '\0';

			return 0;
		}

		PX_INLINE PxI32 sscanf( const char *buffer, const char *format,  ...)
		{
			va_list arg;
			va_start( arg, format );
			PxI32 r = ::sscanf(buffer, format, arg);
			va_end(arg);

			return r;
		}

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

		PX_INLINE void strcpy(char* dest, size_t size, const char* src)
		{
			strncpy_s(dest,size,src,size);
		}

		PX_INLINE void strcat(char* dest, size_t size, const char* src)
		{
			strncat_s(dest,size,src,size);
		}


	}
} // namespace physx

#pragma warning(pop)

#endif

