/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


/*!
\file
\brief PxAsciiConversion namespace contains string/value helper functions
*/

#include <ctype.h>

PX_INLINE bool isWhiteSpace(char c)
{
	bool ret = false;
	if ( c == 32 || c == 9 || c == 13 || c == 10 || c == ',' ) ret = true;
	return ret;
}

PX_INLINE const char * skipNonWhiteSpace(const char *scan)
{
	while ( !isWhiteSpace(*scan) && *scan) scan++;
	if ( *scan == 0 ) scan = NULL;
	return scan;
}
PX_INLINE const char * skipWhiteSpace(const char *scan)
{
	while ( isWhiteSpace(*scan) && *scan ) scan++;
	if ( *scan == 0 ) scan = NULL;
	return scan;
}

static physx::PxF64 strtod_fast(const char * pString)
{
    //---
    // Find the start of the string
	const char* pNumberStart = skipWhiteSpace(pString);
 
    //---
    // Find the end of the string
    const char* pNumberEnd = pNumberStart;
 
    // skip optional sign
    if( *pNumberEnd == '-' || *pNumberEnd == '+' )
        ++pNumberEnd;
 
    // skip optional digits
    while( isdigit(*pNumberEnd) )
        ++pNumberEnd;
 
    // skip optional decimal and digits
    if( *pNumberEnd == '.' )
    {
        ++pNumberEnd;
 
        while( isdigit(*pNumberEnd) )
            ++pNumberEnd;
    }
 
    // skip optional exponent
    if(    *pNumberEnd == 'd'
        || *pNumberEnd == 'D'
        || *pNumberEnd == 'e'
        || *pNumberEnd == 'E' )
    {
        ++pNumberEnd;
 
        if( *pNumberEnd == '-' || *pNumberEnd == '+' )
            ++pNumberEnd;
 
        while( isdigit(*pNumberEnd) )
            ++pNumberEnd;
    }
 
    //---
    // Process the string
	const physx::PxU32 numberLen = (const physx::PxU32)(pNumberEnd-pNumberStart);
    char buffer[32];
    if( numberLen+1 < sizeof(buffer)/sizeof(buffer[0]) )
    {
        // copy into buffer and terminate with NUL before calling the
        // standard function
        memcpy( buffer, pNumberStart, numberLen*sizeof(buffer[0]) );
        buffer[numberLen] = '\0';
		const physx::PxF64 result = strtod( buffer, NULL );
 
        return result;
    }
    else
    {
        // buffer was too small so just call the standard function on the
        // source input to get a proper result
        return strtod( pString, NULL );
    }
}

static physx::PxF32 strtof_fast(const char* pString)
{
    return (physx::PxF32)strtod_fast(pString);            
}


//////////////////////////
// str to value functions
//////////////////////////
PX_INLINE bool strToBool(const char *str, const char **endptr)
{
	bool ret = false;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	physx::PxU32 len = (physx::PxU32)(end - begin);
	if ( physx::string::strnicmp(begin,"true", len) == 0 || physx::string::strnicmp(begin,"1", len) == 0 )
		ret = true;	

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);
	
	return ret;
}

PX_INLINE physx::PxI8  strToI8(const char *str, const char **endptr)
{
	physx::PxI8 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MIN_I8", (size_t)(end-begin)) == 0)
		ret = PX_MIN_I8;
	else if( strncmp(begin, "PX_MAX_I8", (size_t)(end-begin)) == 0)
		ret = PX_MAX_I8;
	else
	 	ret = (physx::PxI8)strtol(begin, 0, 0); //FIXME

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxI16 strToI16(const char *str, const char **endptr)
{
	physx::PxI16 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MIN_I16", (size_t)(end-begin)) == 0)
		ret = PX_MIN_I16;
	else if( strncmp(begin, "PX_MAX_I16", (size_t)(end-begin)) == 0)
		ret = PX_MAX_I16;
	else
	 	ret = (physx::PxI16)strtol(begin, 0, 0); //FIXME
	
	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxI32 strToI32(const char *str, const char **endptr)
{
	physx::PxI32 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MIN_I32", (size_t)(end-begin)) == 0)
		ret = PX_MIN_I32;
	else if( strncmp(begin, "PX_MAX_I32", (size_t)(end-begin)) == 0)
		ret = PX_MAX_I32;
	else
	 	ret = (physx::PxI32)strtol(begin, 0, 0); //FIXME

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxI64 strToI64(const char *str, const char **endptr)
{
	physx::PxI64 ret;
	const char *begin = skipWhiteSpace(str);
    
	//FIXME
#ifdef _WIN32 //PX_WINDOWS, PX_XBOX
 	ret = (physx::PxI64)_strtoi64(begin,0,10);
#else
	ret = (physx::PxI64)strtoll(begin,0,10);
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxU8  strToU8(const char *str, const char **endptr)
{
	physx::PxU8 ret;
	const char *begin = skipWhiteSpace(str);
	
	ret = (physx::PxU8)strtoul(begin, 0, 0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxU16 strToU16(const char *str, const char **endptr)
{
	physx::PxU16 ret;
	const char *end;
	const char *begin = skipWhiteSpace(str);

	end = skipNonWhiteSpace(begin);
	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MAX_U16", (size_t)(end-begin)) == 0)
		ret = PX_MAX_U16;
	else
	 	ret = (physx::PxU16)strtoul(begin,0,0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxU32 strToU32(const char *str, const char **endptr)
{
	physx::PxU32 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	if( strncmp(begin, "PX_MAX_U32", (size_t)(end-begin)) == 0)
		ret = PX_MAX_U32;
	else
	 	ret = (physx::PxU32)strtoul(begin,0,0);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxU64 strToU64(const char *str, const char **endptr)
{
	physx::PxU64 ret;
	const char *begin;
	begin = skipWhiteSpace(str);

	//FIXME
#ifdef _WIN32 //PX_WINDOWS, PX_XBOX
 	ret = (physx::PxU64)_strtoui64(begin,0,10);
#else
	ret = (physx::PxU64)strtoull(begin,0,10);
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE physx::PxF32 strToF32(const char *str, const char **endptr)
{
	physx::PxF32 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);

	const physx::PxU32 len = (physx::PxU32)(end - begin);
	
	const char F32_MIN[] = "PX_MIN_F32";
	const char F32_MAX[] = "PX_MAX_F32";

	if( strncmp(begin, F32_MIN, physx::PxMin(len, (physx::PxU32)(sizeof(F32_MIN) - 1))) == 0)
		ret = -PX_MAX_F32;
	else if( strncmp(begin, F32_MAX, physx::PxMin(len, (physx::PxU32)(sizeof(F32_MAX) - 1))) == 0)
		ret = PX_MAX_F32;
	else
	{
		ret = (physx::PxF32)strtof_fast(begin);
	}
	
#if DEBUGGING_MISMATCHES
	physx::PxF32 testRet = (physx::PxF32)atof(begin);
	if( ret != testRet )
	{
		PX_ASSERT(0 && "Inaccurate float string");
	}
#endif

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}


PX_INLINE physx::PxF64 strToF64(const char *str, const char **endptr)
{
	physx::PxF64 ret;
	const char *begin = skipWhiteSpace(str);
	const char *end = skipNonWhiteSpace(begin);

	end = skipNonWhiteSpace(begin);

	if( !end )
		end = begin + strlen(str);
	
	const physx::PxU32 len = (const physx::PxU32)(end - begin);
	
	const char F64_MIN[] = "PX_MIN_F64";
	const char F64_MAX[] = "PX_MAX_F64";
	
	if( strncmp(begin, F64_MIN, physx::PxMin(len, (physx::PxU32)(sizeof(F64_MIN) - 1))) == 0)
		ret = -PX_MAX_F64;
	else if( strncmp(begin, F64_MAX, physx::PxMin(len, (physx::PxU32)(sizeof(F64_MAX) - 1))) == 0)
		ret = PX_MAX_F64;
	else 
		ret = (physx::PxF64)strtod_fast(begin);

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);

	return ret;
}

PX_INLINE void strToF32s(physx::PxF32 *v,physx::PxU32 count,const char *str, const char**endptr)
{
	const char *begin = skipWhiteSpace(str);

	if ( *begin == '(' ) begin++;
	for (physx::PxU32 i=0; i<count && *begin; i++)
	{
		v[i] = (physx::PxF32)strToF32(begin, &begin);
	}

	if( endptr )
		*endptr = skipNonWhiteSpace(begin);
}


//////////////////////////
// value to str functions
//////////////////////////
PX_INLINE const char * valueToStr( bool val, char *buf, physx::PxU32 n )
{
	physx::string::sprintf_s(buf, n,"%s",val ? "true" : "false");
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxI8 val, char *buf, physx::PxU32 n )
{
	if( val == PX_MIN_I8 )
		physx::string::sprintf_s(buf, n,"%s","PX_MIN_I8" );
	else if( val == PX_MAX_I8 )
		physx::string::sprintf_s(buf, n,"%s","PX_MAX_I8" );
	else
		physx::string::sprintf_s(buf, n, "%d", val);
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxI16 val, char *buf, physx::PxU32 n )
{
	if( val == PX_MIN_I16 )
		physx::string::sprintf_s(buf, n,"%s","PX_MIN_I16" );
	else if( val == PX_MAX_I16 )
		physx::string::sprintf_s(buf, n,"%s","PX_MAX_I16" );
	else
		physx::string::sprintf_s(buf, n,"%d",val );
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxI32 val, char *buf, physx::PxU32 n )
{
	if( val == PX_MIN_I32 )
		physx::string::sprintf_s(buf, n,"%s","PX_MIN_I32" );
	else if( val == PX_MAX_I32 )
		physx::string::sprintf_s(buf, n,"%s","PX_MAX_I32" );
	else
		physx::string::sprintf_s(buf, n,"%d",val );
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxI64 val, char *buf, physx::PxU32 n )
{
	physx::string::sprintf_s(buf, n,"%lld",val );
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxU8 val, char *buf, physx::PxU32 n )
{
	physx::string::sprintf_s(buf, n, "%u", val);
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxU16 val, char *buf, physx::PxU32 n )
{
	if( val == PX_MAX_U16 )
		physx::string::sprintf_s(buf, n,"%s","PX_MAX_U16" );
	else
		physx::string::sprintf_s(buf, n,"%u",val );
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxU32 val, char *buf, physx::PxU32 n )
{
	if( val == PX_MAX_U32 )
		physx::string::sprintf_s(buf, n,"%s","PX_MAX_U32" );
	else
		physx::string::sprintf_s(buf, n,"%u",val );
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxU64 val, char *buf, physx::PxU32 n )
{
	physx::string::sprintf_s(buf, n,"%llu",val );
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxF32 val, char *buf, physx::PxU32 n )
{
	if( !physx::intrinsics::isFinite(val) )
	{
		PX_ASSERT( 0 && "invalid floating point" );
		physx::string::sprintf_s(buf, n,"%s","0" );
	}
	else if( val == -PX_MAX_F32 )
		physx::string::sprintf_s(buf, n,"%s","PX_MIN_F32" );
	else if( val == PX_MAX_F32 )
		physx::string::sprintf_s(buf, n,"%s","PX_MAX_F32" );
    else if ( val == 1 )
    	physx::string::strcpy_s(buf, n, "1");
    else if ( val == 0 )
    	physx::string::strcpy_s(buf, n, "0");
    else if ( val == - 1 )
    	physx::string::strcpy_s(buf, n, "-1");
    else
    {
		physx::string::sprintf_s(buf,n,"%.9g", (double)val ); // %g expects double
		const char *dot = strchr(buf,'.');
		const char *e = strchr(buf,'e');
		if ( dot && !e )
		{
			physx::PxI32 len = (physx::PxI32)strlen(buf);
			char *foo = &buf[len-1];
			while ( *foo == '0' ) foo--;
			if ( *foo == '.' )
				*foo = 0;
			else
				foo[1] = 0;
		}
    }
	return buf;
}

PX_INLINE const char * valueToStr( physx::PxF64 val, char *buf, physx::PxU32 n )
{
	if( !physx::intrinsics::isFinite(val) )
	{
		PX_ASSERT( 0 && "invalid floating point" );
		physx::string::sprintf_s(buf, n,"%s","0" );
	}
	else if( val == -PX_MAX_F64 )
		physx::string::sprintf_s(buf, n,"%s","PX_MIN_F64" );
	else if( val == PX_MAX_F64 )
		physx::string::sprintf_s(buf, n,"%s","PX_MAX_F64" );
    else if ( val == 1 )
		physx::string::strcpy_s(buf, n, "1");
    else if ( val == 0 )
    	physx::string::strcpy_s(buf, n, "0");
    else if ( val == - 1 )
    	physx::string::strcpy_s(buf, n, "-1");
    else
    {
		physx::string::sprintf_s(buf,n,"%.18g", val );
		const char *dot = strchr(buf,'.');
		const char *e = strchr(buf,'e');
		if ( dot && !e )
		{
			physx::PxI32 len = (physx::PxI32)strlen(buf);
			char *foo = &buf[len-1];
			while ( *foo == '0' ) foo--;
			if ( *foo == '.' )
				*foo = 0;
			else
				foo[1] = 0;
		}
    }
	return buf;
}
