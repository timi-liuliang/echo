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
\brief NxParameterized type X-macro template
\note See http://en.wikipedia.org/wiki/C_preprocessor#X-Macros for more details
*/


// NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type)

#if !defined(PX_PS4)
	#pragma warning(push)
	#pragma warning(disable:4555)
#endif	//!PX_PS4

PX_PUSH_PACK_DEFAULT

#ifndef NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#ifndef NX_PARAMETERIZED_TYPES_ONLY_SCALAR_TYPES
NX_PARAMETERIZED_TYPE(Array, ARRAY, void *)
NX_PARAMETERIZED_TYPE(Struct, STRUCT, void *)
NX_PARAMETERIZED_TYPE(Ref, REF, NxParameterized::Interface *)
#endif
#endif

#ifndef NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#ifndef NX_PARAMETERIZED_TYPES_ONLY_SCALAR_TYPES
NX_PARAMETERIZED_TYPE(String, STRING, const char *)
NX_PARAMETERIZED_TYPE(Enum, ENUM, const char *)
#endif
#endif

NX_PARAMETERIZED_TYPE(Bool, BOOL, bool)

NX_PARAMETERIZED_TYPE(I8,  I8,  physx::PxI8)
NX_PARAMETERIZED_TYPE(I16, I16, physx::PxI16)
NX_PARAMETERIZED_TYPE(I32, I32, physx::PxI32)
NX_PARAMETERIZED_TYPE(I64, I64, physx::PxI64)

NX_PARAMETERIZED_TYPE(U8,  U8,  physx::PxU8)
NX_PARAMETERIZED_TYPE(U16, U16, physx::PxU16)
NX_PARAMETERIZED_TYPE(U32, U32, physx::PxU32)
NX_PARAMETERIZED_TYPE(U64, U64, physx::PxU64)

NX_PARAMETERIZED_TYPE(F32, F32, physx::PxF32)
NX_PARAMETERIZED_TYPE(F64, F64, physx::PxF64)

#ifndef NX_PARAMETERIZED_TYPES_ONLY_SCALAR_TYPES
NX_PARAMETERIZED_TYPE(Vec2,      VEC2,      physx::PxVec2)
NX_PARAMETERIZED_TYPE(Vec3,      VEC3,      physx::PxVec3)
NX_PARAMETERIZED_TYPE(Vec4,      VEC4,      physx::PxVec4)
NX_PARAMETERIZED_TYPE(Quat,      QUAT,      physx::PxQuat)
NX_PARAMETERIZED_TYPE(Bounds3,   BOUNDS3,   physx::PxBounds3)
NX_PARAMETERIZED_TYPE(Mat33,     MAT33,     physx::PxMat33)
NX_PARAMETERIZED_TYPE(Mat44,     MAT44,     physx::PxMat44)
NX_PARAMETERIZED_TYPE(Transform, TRANSFORM, physx::PxTransform)
#endif

#ifndef NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#ifndef NX_PARAMETERIZED_TYPES_ONLY_SCALAR_TYPES
NX_PARAMETERIZED_TYPE(Mat34, MAT34, physx::PxMat44)
#endif
#endif

#ifdef NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#   undef NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#endif

#ifdef NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#   undef NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#endif

#ifdef NX_PARAMETERIZED_TYPES_ONLY_SCALAR_TYPES
#   undef NX_PARAMETERIZED_TYPES_ONLY_SCALAR_TYPES
#endif

#ifdef NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#	undef NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#endif

#if !defined(PX_PS4)
	#pragma warning(pop)
#endif	//!PX_PS4

PX_POP_PACK

#undef NX_PARAMETERIZED_TYPE
