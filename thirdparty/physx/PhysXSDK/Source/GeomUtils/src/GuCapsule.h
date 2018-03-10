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

#ifndef GU_CAPSULE_H
#define GU_CAPSULE_H

/** \addtogroup geomutils
@{
*/

#include "GuSegment.h"

namespace physx
{
namespace Gu
{

/**
\brief Represents a capsule.
*/
	class Capsule : public Segment
	{
	public:
		/**
		\brief Constructor
		*/
		PX_INLINE Capsule()
		{
		}

		/**
		\brief Constructor

		\param seg Line segment to create capsule from.
		\param _radius Radius of the capsule.
		*/
		PX_INLINE Capsule(const Segment& seg, PxF32 _radius) : Segment(seg), radius(_radius)
		{
		}

		/**
		\brief Constructor

		\param p0 First segment point
		\param p1 Second segment point
		\param _radius Radius of the capsule.
		*/
		PX_INLINE Capsule(const PxVec3& _p0, const PxVec3& _p1, PxF32 _radius) : Segment(_p0, _p1), radius(_radius)
		{           
		}

		/**
		\brief Destructor
		*/
		PX_INLINE ~Capsule()
		{
		}

		PxF32	radius;
	};
}

}

/** @} */
#endif
