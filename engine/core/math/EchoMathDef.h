#pragma once

#include "engine/core/base/echo_def.h"

namespace Echo{
namespace Math
{
	extern const Real PI;				//!< 3.14159265358979323846264338327950288419716939937511
	extern const Real PI_2;				//!< Math::PI * 2.0
	extern const Real PI_DIV2;			//!< Math::PI / 2.0
	extern const Real PI_DIV3;			//!< Math::PI / 3.0
	extern const Real PI_DIV4;			//!< Math::PI / 4.0
	extern const Real PI_DIV5;			//!< Math::PI / 5.0
	extern const Real PI_DIV6;			//!< Math::PI / 6.0
	extern const Real PI_DIV8;			//!< Math::PI / 8.0
	extern const Real PI_DIV180;		//!< 180 / Math::PI
	extern const Real PI_SQR;			//!< 9.86960440108935861883449099987615113531369940724079
	extern const Real PI_INV;			//!< 0.31830988618379067153776752674502872406891929148091
	extern const Real EPSILON;			//!< FLT: 1.1920929e-007; DBL: 2.2204460492503131e-016
	extern const Real LOWEPSILON;		//!< 1e-04
	extern const Real POS_INFINITY;		//!< infinity
	extern const Real NEG_INFINITY;		//!< -infinity
	extern const Real LN2;				//!< Math::log(2.0)
	extern const Real LN10;				//!< Math::log(10.0)
	extern const Real INV_LN2;			//!< 1.0f/Math::LN2
	extern const Real INV_LN10;			//!< 1.0f/Math::LN10
	extern const Real DEG2RAD;			//!< 0.01745329
	extern const Real RAD2DEG;			//!< 57.29577

	//extern const Real MIN_REAL;			//!< FLT: 1.175494351e-38F; DBL: 2.2250738585072014e-308 ,min positive value 最小正�?防hh)
	extern const Real MAX_REAL;			//!< FLT: 3.402823466e+38F; DBL: 1.7976931348623158e+308

	extern const float MIN_FLOAT;		//!< 1.175494351e-38F
	extern const float MAX_FLOAT;		//!< 3.402823466e+38F
	extern const double MIN_DOUBLE;		//!< 2.2250738585072014e-308
	extern const double MAX_DOUBLE;		//!< 1.7976931348623158e+308

	extern const Byte MAX_BYTE;			//!< 0xff
	extern const short MIN_SHORT;		//!< -32768
	extern const short MAX_SHORT;		//!< 32767
	extern const int MIN_INT;			//!< -2147483648
	extern const int MAX_INT;			//!< 2147483647
	extern const Word MAX_WORD;			//!< 0xff
	extern const Dword MAX_DWORD;		//!< 0xffff
	extern const i8 MIN_I8;				//!< -128
	extern const i8 MAX_I8;				//!< 127
	extern const ui8 MAX_UI8;			//!< 0xff
	extern const i16 MIN_I16;			//!< -32768
	extern const i16 MAX_I16;			//!< 32767
	extern const ui16 MAX_UI16;			//!< 0xffff
	extern const i32 MIN_I32;			//!< -2147483648
	extern const i32 MAX_I32;			//!< 2147483647
	extern const ui32 MAX_UI32;			//!< 0xffffffff
	extern const i64 MIN_I64;			//!< -9223372036854775808
	extern const i64 MAX_I64;			//!< 9223372036854775807
	extern const ui64 MAX_UI64;			//!< 0xffffffffffffffff
}}
