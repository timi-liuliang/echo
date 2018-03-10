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


#ifndef PXC_GRID_CELL_VECTOR_H
#define PXC_GRID_CELL_VECTOR_H

#include <string.h>
#include "PxvConfig.h"
#include "PxVec3.h"
#include "PsMathUtils.h"

namespace physx
{

/*!
Simple integer vector in R3 with basic operations.

Used to define coordinates of a uniform grid cell.
*/
class PxcGridCellVector
{
public:
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxcGridCellVector()
	{
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxcGridCellVector(const PxcGridCellVector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxcGridCellVector(PxI16 _x, PxI16 _y, PxI16 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE PxcGridCellVector(const PxVec3& realVec, PxReal scale)
	{
		set(realVec, scale);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE bool operator==(const PxcGridCellVector& v) const
	{
		return ((x == v.x) && (y == v.y) && (z == v.z));
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE bool operator!=(const PxcGridCellVector& v) const
	{
		return ((x != v.x) || (y != v.y) || (z != v.z));
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector operator+(const PxcGridCellVector& v)
	{
		return PxcGridCellVector(
		x + v.x,
		y + v.y,
		z + v.z
		);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector& operator+=(const PxcGridCellVector& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector operator-(const PxcGridCellVector& v)
	{
		return PxcGridCellVector(
		x - v.x,
		y - v.y,
		z - v.z
		);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector& operator-=(const PxcGridCellVector& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector& operator=(const PxcGridCellVector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}
	
	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector operator << (const PxU32 shift) const
	{
		return PxcGridCellVector(
		x << shift,
		y << shift,
		z << shift
		);
	}

	//! Shift grid cell coordinates (can be used to retrieve coordinates of a coarser grid cell that contains the defined cell)
	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector operator >> (const PxU32 shift) const
	{
		return PxcGridCellVector(
		x >> shift,
		y >> shift,
		z >> shift
		);
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector& operator <<= (const PxU32 shift)
	{
		x <<= shift;
		y <<= shift;
		z <<= shift;
		return *this;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE const PxcGridCellVector& operator >>= (const PxU32 shift)
	{
		x >>= shift;
		y >>= shift;
		z >>= shift;
		return *this;
	}

	//! Set grid cell coordinates based on a point in space and a scaling factor
	PX_CUDA_CALLABLE PX_FORCE_INLINE void set(const PxVec3& realVec, PxReal scale)
	{
		set(realVec * scale);	
	}

#ifdef __CUDACC__
	//! Set grid cell coordinates based on a point in space
	PX_CUDA_CALLABLE PX_FORCE_INLINE void set(const PxVec3& realVec)
	{
		x = static_cast<PxI16>(floorf(realVec.x));
		y = static_cast<PxI16>(floorf(realVec.y));
		z = static_cast<PxI16>(floorf(realVec.z));
	}
#else
#ifndef __SPU__
	//! Set grid cell coordinates based on a point in space
	PX_FORCE_INLINE void set(const PxVec3& realVec)
	{
		x = static_cast<PxI16>(Ps::floor(realVec.x));
		y = static_cast<PxI16>(Ps::floor(realVec.y));
		z = static_cast<PxI16>(Ps::floor(realVec.z));
	}
#else
	PX_FORCE_INLINE void set(const PxVec3& realVec);
#endif
#endif

	PX_CUDA_CALLABLE PX_FORCE_INLINE void set(PxI16 _x, PxI16 _y, PxI16 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE void setZero()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE bool isZero()	const
	{
		return x == 0 && y == 0 && z == 0;
	}

public:

	PxI16 x;
	PxI16 y;
	PxI16 z;
};

#ifdef __SPU__
#include "PS3\PxcGridCellVectorPS3.h"
#endif

}

#endif
