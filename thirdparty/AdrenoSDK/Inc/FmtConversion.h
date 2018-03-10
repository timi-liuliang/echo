// Copyright (c) 2013 Qualcomm Technologies, Inc.            

// Some functions to convert between formats.



#ifndef _FMTCONVERSIONS_H_

#define _FMTCONVERSIONS_H_



#include "ArgDefines.h"



//=================================================================================================================================

///

/// Converts a fixed 16.16 int to a float32

///

/// \param The fixed 16.16 int value

///

/// \return The float32 value

//=================================================================================================================================

inline float32 FixedToFloat( uint32 v )

{

   float32 v0 = (float32)(int32)v;

   uint32 v1 = v ? (*((uint32*)&v0) - 0x08000000) : 0;

   

   return *((float32*)&v1);

}



//=================================================================================================================================

///

/// Converts a fixed 32.32 int to a float32

///

/// \param The fixed 32.32 int value

///

/// \return The float64 value

//=================================================================================================================================

inline float64 FixedToFloat( uint64 v )

{

   float64 v0 = (float64)(int64)v;

   uint64 v1 = v ? (*((uint64*)&v0) - 0x08000000) : 0;

   

   return *((float64*)&v1);

}



//=================================================================================================================================

///

/// Converts a float32 to a fixed 16.16 int

///

/// \param The float32 value

///

/// \return The fixed 16.16 int value

//=================================================================================================================================

inline uint32 FloatToFixed( float32 v )

{

    float32 b = v;

    uint32 ret;

    uint32 d = *((uint32*)&b) + 0x08000000;



    ret = (int32)(*((float32*)&d));

    return ret;



}



//=================================================================================================================================

///

/// Converts a float64 to a fixed 32.32 int

///

/// \param The float64 value

///

/// \return The fixed 32.32 int value

//=================================================================================================================================

inline uint64 FloatToFixed( float64 v )

{

    float64 b = v;

    uint64 ret;

    uint64 d = *((uint64*)&b) + 0x08000000;



    ret = (int64)(*((float64*)&d));

    return ret;

}



//=================================================================================================================================

///

/// Converts a float32 to a float16

///

/// \param The float32 value

///

/// \return The float16 value

//=================================================================================================================================

inline uint16 FloatToHalf( float32 v )

{

   uint32 src = *(uint32*)(&v);

   uint32 sign = src >> 31;   // Shifts on unsigned int types are always supposed to zero fill, is this the case for Mac, and Linux, etc. as well..

   uint16 ret;



   // Extract mantissa

   uint32 mantissa = src  &  (1 << 23) - 1;



   // extract & convert exp bits

   int32 exp = int32( src >> 23  &  0xFF ) - 127;

   if( exp > 15 )

   {

      // The largest-possible positive 2-byte-float exponent value would be 11110-01111 = 11110 = 15.

      exp = 15;

      mantissa = (1 << 23) - 1;

   }

   else if( exp < -14 )

   {

      // handle wraparound of shifts (done mod 64 on PPC)

      // All float32 denormal/0 values map to float16 0

      if( exp <= - 14 - 24 )

      {

         mantissa = 0;

      }

      else

      {

         mantissa = ( mantissa | 1 << 23 )  >>  (-exp - 14);

      }

      exp = -14;

   }

   // TODO: exp is a *signed* int, left shifting it could extend the signed bit, 

   // will have to mask off the first bits where the mantissa should go.

   ret = (sign << 15)  |  ((uint16)(exp + 15)  << 10) |  (mantissa >> 13);

   return ret;

}



//=================================================================================================================================

///

/// Converts from float64 to float32

///

/// \param The float64 value

///

/// \return The float32 value in a uint32

//=================================================================================================================================

inline uint32 FloatToHalf( float64 v )

{

   // 64bit IEEE 754 float format:  1 sign bit, 11 exp bits,52 mantissa bits

   uint64 src = *(uint64*)(&v);

   uint64 sign = src >> 63;

   uint32 ret;



   // Extract mantissa

   uint64 mantissa = src  &  ((uint64)1 << 52) - 1;



   // NOTE: 2047 = 11111111111 (11 'set' bits), below it functions as a mask so that the first 11bits (which we're treating as our exponent)

   // are retained while the remaining are set to zero.

   int64 exp = int64( src >> 52  &  2047 ) - 1023;

   if( exp > 127 )

   {

      // largest possible (non infinity) 4-byte-float-number...

      exp = 127;

      mantissa = ((uint64)1 << 52) - 1;

   }

   else if( exp < -126 )

   {

      // handle wraparound of shifts (done mod 64 on PPC)

      // All float32 denorm/0 values map to float16 0

      if( exp <= - 126 - 24 )

      {

         mantissa = 0;

      }

      else

      {

         mantissa = ( mantissa | (uint64)1 << 52 )  >>  (-exp - 126);

      }

      exp = -15;

   }

   // TODO: Can't left shift *signed* integer exp as it may fill in using the sign bit which could be 1, and not 0.

   ret = (uint32)((sign << 31)  |  ((uint32)(exp + 127)  << 23) |  (mantissa >> 29));

   return ret;

}



//=================================================================================================================================

///

/// Takes subnormal float32 (cast as a uint32 so we can operate on it for this function)

///

/// \param The subnormal float32 value

///

/// \return The float32 value

//=================================================================================================================================

inline uint32 Float32SubnormalToNormal(uint32 i)

{

   uint32 mantissa = i;

   uint32 exponent = 0;          // Zero exponent





   // Recall that a float represents a (base 2) binary number using scientific notation, which is of the format: MANTISSA x 2^(EXPONENT-EXP_BIAS).

   // So to represent the "normal" version of a "subnormal" float, where the exponent==0 AND mantissa!=0 indicates "subnormal" via IEEE float 754 specification,

   // we just need to make the exponent non-zero while still having the overall float value represent the same number..

   // We do this by subtracting 



   // While not normalized..

   while(!(mantissa&0x00800000))

   { 

      exponent -= 0x00800000;   // Decrement exponent (1<<23). This is the same as subtracting 1 from the 8bit mantissa, but in the context of a uint32 instead of a uint8.

      mantissa <<= 1;           // Left shift mantissa 1, which is multiplying by 2..

   }



   mantissa &= ~0x00800000;        // Clear first exponent bit

   exponent += 0x38800000;         // Adjust bias ((127-14)<<23)

   return (mantissa | exponent);   // Return finalized float including new mantissa and exponent

}







//=================================================================================================================================

///

/// Converts a float16 to a float32

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline float32 HalfToFloat( uint16 bits )

{

   // This accounts for normal 2byte floats, but what about the special cases..

   uint32 ret = ((bits&0x8000)<<16) | (((bits&0x7c00)+0x1C000)<<13) | ((bits&0x03FF)<<13);

   // When exponent is zero: we're dealing with either +/- zero (when mantissa is also zero), OR subnormal values (when mantissa is also non-zero)

   // However zero exponents are handled by the above logic, and it also properly handles mantissas of any values.

   // When exponent is 31: we're dealing with either +/- infinity (when mantissa is also zero), OR NaN (Not a Number) (when mantissa is also non-zero)

   // For the case when the 5-bit exponent is 31, we're converting to an 8-bit exponent, and our logic will not expand the exponent to 255 like it needs to.

   

   // If the 2byte float (masking off the sign/mantissa bits) is equal to the mask for the exponent bits...

   if( (bits & 0x7c00) == 0x7c00)

   {  // Exponent==31 special case..

      ret|= 0x7F800000;    // Set the 8bits of the exponent for the 4 byte return value.

   }

   else if( 0 == (bits&0x7c00) && 0 != (bits&0x03FF)) // If 2byte float was a subnormal number...

   {

      // Return the normalized version of 4byte float, since operating on subnormal floating point numbers

      // will introduce "floating point drift" (loss of precision), after arithmetic, sooner..

      ret = Float32SubnormalToNormal(ret);

   }



   return *((float32*)&ret);

}







// TODO: Finish making this 64bit.

//=================================================================================================================================

///

/// Takes subnormal float32 (cast as a uint32 so we can operate on it for this function)

///

/// \param The subnormal float64 value

///

/// \return The float64 value

//=================================================================================================================================

inline uint64 Float64SubnormalToNormal(uint64 input)

{

   uint64 mantissa = input;

   uint64 exponent = 0;          // Zero exponent

   const uint64 firstExpBit= ((uint64)1<<52);

   const uint64 mantissaMask= firstExpBit-1;



   // Recall that a float represents a (base 2) binary number using scientific notation, which is of the format: MANTISSA x 2^EXPONENT.

   // So to represent the "normal" version of a "subnormal" float, where the exponent==0 AND mantissa!=0 indicates "subnormal" via IEEE float 754 specification

   // we just need to make the exponent non-zero, yet still have the overall float value represent the same number..

   // We do this by subtracting 



   // While not normalized..

   while(!(mantissa&firstExpBit))

   { 

      exponent -= firstExpBit;   // Decrement exponent (1<<23). This is the same as subtracting 1 from the 8bit mantissa, but in the context of a uint32 instead of a uint8.

      mantissa <<= 1;           // Left shift mantissa 1, which is multiplying by 2..

   }



   mantissa &= ~firstExpBit;        // Clear leading 1 bit

   exponent += (uint64)896<<52;                // Adjust bias ((1023-127)<<52)

   return (mantissa | exponent);    // Return finalized float including new mantissa and exponent

}





//=================================================================================================================================

///

/// Converts a float32 to a float64

///

/// \param The float32 value

///

/// \return The float64 value

//=================================================================================================================================

inline float64 HalfToFloat( uint32 bits )

{

   const uint64 bits64= (uint64)bits;

   const uint64 exponentMask64bitFloat= (uint64)0x7FF<<52;

   const uint64 exponentBiasSubAdd = (uint64)896<<23;



   // This accounts for normal 2byte floats, but what about the special cases..

   uint64 ret = ((bits64&0x80000000)<<32) | (((bits64&0x7F800000)+exponentBiasSubAdd)<<29) | ((bits64&0x7FFFFF)<<29);

   // When exponent is zero: we're dealing with either +/- zero (when mantissa is also zero), OR subnormal values (when mantissa is also non-zero)

   // However zero exponents are handled by the above logic, and it also properly handles mantissas of any values.

   // When exponent is 31: we're dealing with either +/- infinity (when mantissa is also zero), OR NaN (Not a Number) (when mantissa is also non-zero)

   // For the case when the 5-bit exponent is 31, we're converting to an 8-bit exponent, and our logic will not expand the exponent to 255 like it needs to.

   

   // If the 2byte float (masking off the sign/mantissa bits) is equal to the mask for the exponent bits...

   if( (bits&0x7F800000) == 0x7F800000)

   {  // Exponent==31 special case..

      ret|= exponentMask64bitFloat;    // Set the 11bits of the exponent for the 8byte return value.

   }

   else if( 0 == (bits&0x7F800000) && 0 != (bits&0x7FFFFF)) // If 4byte float was a subnormal number...

   {

      // Return the normalized version of 4byte float, since operating on subnormal floating point numbers

      // will introduce "floating point drift" (loss of precision), after arithmetic, sooner..

      ret = Float64SubnormalToNormal(ret);

   }



   return *((float64*)&ret);

}



//=================================================================================================================================

///

/// Converts 4 floats into a GL_UNSIGNED_INT_2_10_10_10_REV

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline uint32 FloatsToUint2_10_10_10( float32 a, float32 b, float32 g, float32 r)

{

	// Clamp from 0 to 1

	float32 fTempR = (r > 1.0f) ? 1.0f : (r < 0.0f) ? 0.0f : r;

	float32 fTempG = (g > 1.0f) ? 1.0f : (g < 0.0f) ? 0.0f : g;

	float32 fTempB = (b > 1.0f) ? 1.0f : (b < 0.0f) ? 0.0f : b;

	float32 fTempA = (a > 1.0f) ? 1.0f : (a < 0.0f) ? 0.0f : a;



	// Convert to unsigned integer 0 to 1023 range

	uint32 iR = (uint32) (fTempR * 1023.0f);

	uint32 iG = (uint32) (fTempG * 1023.0f);

	uint32 iB = (uint32) (fTempB * 1023.0f);

	uint32 iA = (uint32) (fTempA * 3.0f);



	return ((iA & 0x3) << 30) | ((iB & 0x3FF) << 20) | ((iG & 0x3FF) << 10) | (iR & 0x3FF);

}



//=================================================================================================================================

///

/// Converts 4 floats into a GL_UNSIGNED_INT_10_10_10_2

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline uint32 FloatsToUint10_10_10_2( float32 r, float32 g, float32 b, float32 a )

{

   // Clamp from 0 to 1

   float32 fTempR = (r > 1.0f) ? 1.0f : (r < 0.0f) ? 0.0f : r;

   float32 fTempG = (g > 1.0f) ? 1.0f : (g < 0.0f) ? 0.0f : g;

   float32 fTempB = (b > 1.0f) ? 1.0f : (b < 0.0f) ? 0.0f : b;

   float32 fTempA = (a > 1.0f) ? 1.0f : (a < 0.0f) ? 0.0f : a;



   // Convert to unsigned integer 0 to 1023 range

   uint32 iR = (uint32) (fTempR * 1023.0f);

   uint32 iG = (uint32) (fTempG * 1023.0f);

   uint32 iB = (uint32) (fTempB * 1023.0f);

   uint32 iA = (uint32) (fTempA * 3.0f);



   return ((iR & 0x3FF) << 22) | ((iG & 0x3FF) << 12) | ((iB & 0x3FF) << 2) | (iA & 0x3);

}



//=================================================================================================================================

///

/// Converts 4 floats into a GL_UNSIGNED_INT_10_10_10_2

///

/// \param The float32 value

///

/// \return The float64 value

//=================================================================================================================================

inline uint64 FloatsToUint10_10_10_2( float64 r, float64 g, float64 b, float64 a )

{

   // Clamp from 0 to 1

   float64 fTempR = (r > 1.0f) ? 1.0f : (r < 0.0f) ? 0.0f : r;

   float64 fTempG = (g > 1.0f) ? 1.0f : (g < 0.0f) ? 0.0f : g;

   float64 fTempB = (b > 1.0f) ? 1.0f : (b < 0.0f) ? 0.0f : b;

   float64 fTempA = (a > 1.0f) ? 1.0f : (a < 0.0f) ? 0.0f : a;



   // Convert to unsigned integer 0 to 1023 range

   uint64 iR = (uint64) (fTempR * 1023.0f);

   uint64 iG = (uint64) (fTempG * 1023.0f);

   uint64 iB = (uint64) (fTempB * 1023.0f);

   uint64 iA = (uint64) (fTempA * 3.0f);



   return ((iR & 0x3FF) << 22) | ((iG & 0x3FF) << 12) | ((iB & 0x3FF) << 2) | (iA & 0x3);

}





//=================================================================================================================================

///

/// Converts 3 floats into a GL_INT_10_10_10

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline uint32 FloatsToInt10_10_10( float32 r, float32 g, float32 b )

{

   // Clamp from -1 to 1

   float32 fTempR = (r > 1.0f) ? 1.0f : (r < -1.0f) ? -1.0f : r;

   float32 fTempG = (g > 1.0f) ? 1.0f : (g < -1.0f) ? -1.0f : g;

   float32 fTempB = (b > 1.0f) ? 1.0f : (b < -1.0f) ? -1.0f : b;



   // Convert to signed integer -511 to +511 range

   int32 iR = (uint32) (fTempR * 511.0f);

   int32 iG = (uint32) (fTempG * 511.0f);

   int32 iB = (uint32) (fTempB * 511.0f);



   return ((iR & 0x3FF) << 22) | ((iG & 0x3FF) << 12) | ((iB & 0x3FF) << 2) | (0 & 0x3);

}



//=================================================================================================================================

///

/// Converts a GL_UNSIGNED_INT_10_10_10_2 to 3 floats

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline void Uint10_10_10_2ToFloats( int32 inVal, float32 *outVal )

{

   uint32 iR = (inVal >> 22) & 0x3FF;

   uint32 iG = (inVal >> 12) & 0x3FF;

   uint32 iB = (inVal >> 2) & 0x3FF;

   uint32 iA = (inVal & 0x3 );

   

   outVal[0] = (float32)iR;

   outVal[1] = (float32)iG;

   outVal[2] = (float32)iB;

   outVal[3] = (float32)iA;



}



//=================================================================================================================================

// Converts a GL_UNSIGNED_INT_2_10_10_10_REV to 4 floats

// ABGR format

// 2 - alpha

// 10 - blue

// 10 - green

// 10 - red

//=================================================================================================================================

inline void Uint2_10_10_10_REV_ToFloats( int32 inVal, float32 *outVal )

{

    uint32 iA = (inVal >> 30) & 0x3;

    uint32 iB = (inVal >> 20) & 0x3FF;

    uint32 iG = (inVal >> 10) & 0x3FF;

    uint32 iR = (inVal >> 0) & 0x3FF;



    outVal[0] = (float32)iR;

    outVal[1] = (float32)iG;

    outVal[2] = (float32)iB;

    outVal[3] = (float32)iA;

}





//=================================================================================================================================

///

/// Converts a GL_UNSIGNED_INT_10_10_10_2 to 3 normalized floats

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline void Uint10_10_10_2ToFloatsN( int32 inVal, float32 *outVal )

{

   uint32 iR = (inVal >> 22) & 0x3FF;

   uint32 iG = (inVal >> 12) & 0x3FF;

   uint32 iB = (inVal >> 2) & 0x3FF;

   uint32 iA = (inVal & 0x3 );

   

   outVal[0] = float32(iR) / 1023.0f;

   outVal[1] = float32(iG) / 1023.0f;

   outVal[2] = float32(iB) / 1023.0f;

   outVal[3] = float32(iA) / 3.0f;

}



//=================================================================================================================================

// Converts a GL_UNSIGNED_INT_2_10_10_10_REV to 4 floats, normalized

// ABGR format

// 2 - alpha

// 10 - blue

// 10 - green

// 10 - red

//=================================================================================================================================

inline void Uint2_10_10_10_REV_ToFloatsN( int32 inVal, float32 *outVal )

{

    uint32 iA = (inVal >> 30) & 0x3;

    uint32 iB = (inVal >> 20) & 0x3FF;

    uint32 iG = (inVal >> 10) & 0x3FF;

    uint32 iR = (inVal >> 0) & 0x3FF;



    outVal[0] = (float32)iR / 1023.0f;

    outVal[1] = (float32)iG / 1023.0f;

    outVal[2] = (float32)iB / 1023.0f;

    outVal[3] = (float32)iA / 3.0f;

}



//=================================================================================================================================

///

/// Converts a GL_INT_10_10_10_2 to 3 floats

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline void Int10_10_10_2ToFloats( int32 inVal, float32 *outVal )

{

   int32 iR = (inVal >> 22) & 0x3FF;

   if( iR & 0x200 )

      iR |= 0xfffffc00; // handle negitive numbers

   int32 iG = (inVal >> 12) & 0x3FF;

   if( iG & 0x200 )

      iG |= 0xfffffc00; // handle negitive numbers

   int32 iB = (inVal >> 2) & 0x3FF;

   if( iB & 0x200 )

      iB |= 0xfffffc00; // handle negitive numbers

   

   outVal[0] = (float32)iR;

   outVal[1] = (float32)iG;

   outVal[2] = (float32)iB;

}



//=================================================================================================================================

// Converts a GL_INT_2_10_10_10_REV to 4 floats

// ABGR format

// 2 - alpha

// 10 - blue

// 10 - green

// 10 - red

//=================================================================================================================================

inline void Int2_10_10_10_REV_ToFloats( int32 inVal, float32 *outVal )

{

    int32 iR = (inVal >> 0) & 0x3FF;

    if (iR & 0x200)

        iR |= 0xfffffc00; // handle negative numbers

    

    int32 iG = (inVal >> 10) & 0x3FF;

    if (iG & 0x200)

        iG |= 0xfffffc00; // handle negative numbers



    int32 iB = (inVal >> 20) & 0x3FF;

    if (iB & 0x200)

        iB |= 0xfffffc00; // handle negative numbers



    int32 iA = (inVal >> 30) & 0x3;

    if (iA & 0x200)

        iA |= 0xfffffc00; // handle negative numbers



    outVal[0] = (float32)iR;

    outVal[1] = (float32)iG;

    outVal[2] = (float32)iB;

    outVal[3] = (float32)iA;

}



//=================================================================================================================================

///

/// Converts a GL_INT_10_10_10_2 to 3 normalized floats

///

/// \param The float16 value

///

/// \return The float32 value

//=================================================================================================================================

inline void Int10_10_10_2ToFloatsN( int32 inVal, float32 *outVal )

{

   int32 iR = (inVal >> 22) & 0x3FF;

   if( iR & 0x200 )

      iR |= 0xfffffc00; // handle negitive numbers

   int32 iG = (inVal >> 12) & 0x3FF;

   if( iG & 0x200 )

      iG |= 0xfffffc00; // handle negitive numbers

   int32 iB = (inVal >> 2) & 0x3FF;

   if( iB & 0x200 )

      iB |= 0xfffffc00; // handle negitive numbers

   

   outVal[0] = float32(iR) / 511.0f;

   outVal[1] = float32(iG) / 511.0f;

   outVal[2] = float32(iB) / 511.0f;

}

//=================================================================================================================================

// Converts a GL_INT_2_10_10_10_REV to 4 floats, normalized

// ABGR format

// 2 - alpha

// 10 - blue

// 10 - green

// 10 - red

//=================================================================================================================================

inline void Int2_10_10_10_REV_ToFloatsN( int32 inVal, float32 *outVal )

{

    int32 iR = (inVal >> 0) & 0x3FF;

    if (iR & 0x200)

        iR |= 0xfffffc00; // handle negative numbers



    int32 iG = (inVal >> 10) & 0x3FF;

    if (iG & 0x200)

        iG |= 0xfffffc00; // handle negative numbers



    int32 iB = (inVal >> 20) & 0x3FF;

    if (iB & 0x200)

        iB |= 0xfffffc00; // handle negative numbers



    int32 iA = (inVal >> 30) & 0x3;

    if (iA & 0x200)

        iA |= 0xfffffc00; // handle negative numbers



    outVal[0] = (float32)iR / 511.0f;

    outVal[1] = (float32)iG / 511.0f;

    outVal[2] = (float32)iB / 511.0f;

    outVal[3] = (float32)iA / 511.0f;

}



//=================================================================================================================================

///

/// Converts a 3 floats in the range 0-1 to a packed short565

///

/// \param r - red value

/// \param g - green value

/// \param b - blue value

///

/// \return The packed short 565

//=================================================================================================================================

inline uint16 FloatsToShort5_6_5( float32 r, float32 g, float32 b )

{

   // Clamp from 0 to max

   float32 fTempR = (r < 0.0f) ? 0.0f : (r > 1.0f) ? 1.0f : r;

   float32 fTempG = (g < 0.0f) ? 0.0f : (g > 1.0f) ? 1.0f : g;

   float32 fTempB = (b < 0.0f) ? 0.0f : (b > 1.0f) ? 1.0f : b;



   // Convert to integer max range (32 to 64)

   uint16 iR = (uint32) (fTempR * 31.0f);

   uint16 iG = (uint32) (fTempG * 63.0f);

   uint16 iB = (uint32) (fTempB * 31.0f);



   return ((iR & 0x1F) << 11) | ((iG & 0x3F) << 5) | ((iB & 0x1F) << 0);

}



//=================================================================================================================================

///

/// Converts a an uint24 to an uint32

///

/// \param val - the input uint24 value

///

/// \return The uint32 value

//=================================================================================================================================

inline uint32 Uint24ToUint32( uint8* val )

{

   uint32 p0 = val[0];

   uint32 p1 = val[1];

   uint32 p2 = val[2];

   uint32 out;



   out = ( ( p0 << 0 ) + ( p1 << 8 ) + ( p2 << 16 ) ) << 8;

   return out;

}



//=================================================================================================================================

///

/// Converts a an uint32 to an uint24

///

/// \param inVal - The input uint32 value

/// \param outVal - The output uint24 value

///

/// \return void

//=================================================================================================================================

inline void Uint32ToUint24( uint32 inVal, uint8 *outVal )

{

   outVal[0] = ((int8*)&inVal)[1];

   outVal[1] = ((int8*)&inVal)[2];

   outVal[2] = ((int8*)&inVal)[3];

}



#endif // _FMTCONVERSIONS_H_



