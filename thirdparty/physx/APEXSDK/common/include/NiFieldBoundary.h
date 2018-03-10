/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_FIELD_BOUNDARY_H__
#define __NI_FIELD_BOUNDARY_H__

#include "InplaceTypes.h"

namespace physx
{
namespace apex
{


struct NiFieldShapeType
{
	enum Enum
	{
		NONE = 0,
		SPHERE,
		BOX,
		CAPSULE,

		FORCE_DWORD = 0xFFFFFFFFu
	};
};

//struct NiFieldShapeDesc
//dimensions for
//SPHERE: x = radius
//BOX:    (x,y,z) = 1/2 size
//CAPUSE: x = radius, y = height
#define INPLACE_TYPE_STRUCT_NAME NiFieldShapeDesc
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(InplaceEnum<NiFieldShapeType::Enum>,	type) \
	INPLACE_TYPE_FIELD(physx::PxMat34Legacy,				worldToShape) \
	INPLACE_TYPE_FIELD(physx::PxVec3,						dimensions) \
	INPLACE_TYPE_FIELD(physx::PxF32,						weight)
#include INPLACE_TYPE_BUILD()


#ifndef __CUDACC__

struct NiFieldBoundaryDesc
{
#if NX_SDK_VERSION_MAJOR == 2
	NxGroupsMask64	boundaryFilterData;
#else
	PxFilterData	boundaryFilterData;
#endif
};

class NiFieldBoundary
{
public:
	virtual bool updateFieldBoundary(physx::Array<NiFieldShapeDesc>& shapes) = 0;

protected:
	virtual ~NiFieldBoundary() {}
};
#endif

}
} // end namespace physx::apex

#endif // #ifndef __NI_FIELD_BOUNDARY_H__
