
//=================================================================================================================================
//   Copyright (c) 2013 Qualcomm Technologies, Inc.  All rights reserved.     
//=================================================================================================================================
// Standard types

#ifndef _ARGDEFINES_H_
#define _ARGDEFINES_H_


#include "Crossplatform.h"

#if defined( _WIN32 ) || defined( _WIN64 )
typedef char               char8;
typedef char               int8;
typedef short              int16;
typedef int                int32;
typedef __int64            int64;
typedef float              float32;
typedef double             float64;
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef unsigned __int64   uint64;
#elif LINUX_OR_OSX

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef char               char8;
typedef char               int8;
typedef short              int16;
typedef int                int32;
typedef int64_t            int64;
typedef float              float32;
typedef double             float64;
typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;
typedef uint64_t   	   uint64;







#endif


#endif // _ARGDEFINES_H_

