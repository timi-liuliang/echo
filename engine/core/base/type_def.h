#pragma once

#include <stddef.h>

// Precision
// #define ECHO_PREC_DOUBLE

namespace Echo
{
	typedef signed char			i8;			//!< ：C128 to 127
	typedef short				i16;		//!< ：C32,768 to 32,767
	typedef int					i32;		//!< ：C2,147,483,648 to 2,147,483,647
	typedef long long			i64;		//!< ：C9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
	typedef unsigned char		ui8;
	typedef unsigned short		ui16;
	typedef unsigned int		ui32;
	typedef unsigned long long	ui64;

	typedef unsigned long		ulong;
	typedef float				real32;		//!< .4E +/- 38 (7 digits)
	typedef double				real64;		//!< 1.7E +/- 308 (15 digits)

	typedef unsigned int		Dword;
	typedef int					Bool;
	typedef unsigned char		Byte;
	typedef unsigned short		Word;

#ifdef ECHO_PREC_DOUBLE
	typedef double				Real;
#else
	typedef float				Real;
#endif
}