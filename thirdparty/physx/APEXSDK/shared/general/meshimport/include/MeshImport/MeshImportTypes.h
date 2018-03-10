/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MESH_IMPORT_TYPES_H

#define MESH_IMPORT_TYPES_H

// If the mesh import plugin source code is embedded directly into the APEX samples then we create a remapping between
// PhysX data types and MeshImport data types for compatbility between all platforms.
#ifdef PLUGINS_EMBEDDED

#include "foundation/PxSimpleTypes.h"

namespace mimp
{

typedef physx::PxI64	MiI64;
typedef physx::PxI32	MiI32;
typedef physx::PxI16	MiI16;
typedef physx::PxI8	MiI8;

typedef physx::PxU64	MiU64;
typedef physx::PxU32	MiU32;
typedef physx::PxU16	MiU16;
typedef physx::PxU8	MiU8;

typedef physx::PxF32	MiF32;
typedef physx::PxF64	MiF64;

#ifdef PX_X86
#define MI_X86
#endif

#ifdef PX_WINDOWS
#define MI_WINDOWS
#endif

#ifdef PX_X64
#define MI_X64
#endif

#define MI_PUSH_PACK_DEFAULT PX_PUSH_PACK_DEFAULT
#define MI_POP_PACK	 PX_POP_PACK

#define MI_INLINE PX_INLINE
#define MI_NOINLINE PX_NOINLINE
#define MI_FORCE_INLINE PX_FORCE_INLINE

};


#else

namespace mimp
{

	typedef signed __int64		MiI64;
	typedef signed int			MiI32;
	typedef signed short		MiI16;
	typedef signed char			MiI8;

	typedef unsigned __int64	MiU64;
	typedef unsigned int		MiU32;
	typedef unsigned short		MiU16;
	typedef unsigned char		MiU8;

	typedef float				MiF32;
	typedef double				MiF64;

/**
Platform define
*/
#ifdef _M_IX86
#define MI_X86
#define MI_WINDOWS
#elif defined(_M_X64)
#define MI_X64
#define MI_WINDOWS
#endif

#define MI_PUSH_PACK_DEFAULT	__pragma( pack(push, 8) )
#define MI_POP_PACK			__pragma( pack(pop) )

#define MI_INLINE inline
#define MI_NOINLINE __declspec(noinline)
#define MI_FORCE_INLINE __forceinline


};

#endif

#endif
