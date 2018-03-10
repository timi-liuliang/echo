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

#ifndef GU_SEPARATINGAXES_H
#define GU_SEPARATINGAXES_H

#include "PsArray.h"
#include "PxVec3.h"
#include "PxPhysXCommonConfig.h"

namespace physx
{
namespace Gu
{
	// PT: this is a number of axes. Multiply by sizeof(PxVec3) for size in bytes.
	#define SEP_AXIS_FIXED_MEMORY	256

	// This class holds a list of potential separating axes.
	// - the orientation is irrelevant so V and -V should be the same vector
	// - the scale is irrelevant so V and n*V should be the same vector
	// - a given separating axis should appear only once in the class
#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
	class PX_PHYSX_COMMON_API SeparatingAxes
	{
	public:
		PX_INLINE SeparatingAxes() : mNbAxes(0)	{}

		bool addAxis(const PxVec3& axis);

		PX_FORCE_INLINE const PxVec3* getAxes() const
		{
			return mAxes;
		}

		PX_FORCE_INLINE PxU32 getNumAxes() const
		{
			return mNbAxes;
		}

		PX_FORCE_INLINE void reset()
		{
			mNbAxes = 0;
		}

	private:
		PxU32	mNbAxes;
		PxVec3	mAxes[SEP_AXIS_FIXED_MEMORY];
	};
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

	enum PxcSepAxisType
	{
		SA_NORMAL0,		// Normal of object 0
		SA_NORMAL1,		// Normal of object 1
		SA_EE			// Cross product of edges
	};

}
}

#endif
