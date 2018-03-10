/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PX_ASCII_CONVERSION_H
#define PX_ASCII_CONVERSION_H

/*!
\file
\brief PxAsciiConversion namespace contains string/value helper functions
*/


#include "PsShare.h"
#include "foundation/PxAssert.h"
#include "PsString.h"
#include "foundation/PxMath.h"
#include "foundation/PxIntrinsics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>

namespace physx
{
namespace general_string_parsing2
{
namespace PxAsc
{

const physx::PxU32 PxF32StrLen = 24;
const physx::PxU32 PxF64StrLen = 32;
const physx::PxU32 IntStrLen = 32;

PX_INLINE bool isWhiteSpace(char c);
PX_INLINE const char * skipNonWhiteSpace(const char *scan);
PX_INLINE const char * skipWhiteSpace(const char *scan);

//////////////////////////
// str to value functions
//////////////////////////
PX_INLINE bool strToBool(const char *str, const char **endptr);
PX_INLINE physx::PxI8  strToI8(const char *str, const char **endptr);
PX_INLINE physx::PxI16 strToI16(const char *str, const char **endptr);
PX_INLINE physx::PxI32 strToI32(const char *str, const char **endptr);
PX_INLINE physx::PxI64 strToI64(const char *str, const char **endptr);
PX_INLINE physx::PxU8  strToU8(const char *str, const char **endptr);
PX_INLINE physx::PxU16 strToU16(const char *str, const char **endptr);
PX_INLINE physx::PxU32 strToU32(const char *str, const char **endptr);
PX_INLINE physx::PxU64 strToU64(const char *str, const char **endptr);
PX_INLINE physx::PxF32 strToF32(const char *str, const char **endptr);
PX_INLINE physx::PxF64 strToF64(const char *str, const char **endptr);
PX_INLINE void strToF32s(physx::PxF32 *v,physx::PxU32 count,const char *str, const char**endptr);


//////////////////////////
// value to str functions
//////////////////////////
PX_INLINE const char * valueToStr( bool val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxI8 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxI16 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxI32 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxI64 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxU8 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxU16 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxU32 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxU64 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxF32 val, char *buf, physx::PxU32 n );
PX_INLINE const char * valueToStr( physx::PxF64 val, char *buf, physx::PxU32 n );

#include "PxAsciiConversion.inl"

}; // end of namespace
}; // end of namespace
using namespace general_string_parsing2;
}; // end of namespace


#endif // PX_ASCII_CONVERSION_H
