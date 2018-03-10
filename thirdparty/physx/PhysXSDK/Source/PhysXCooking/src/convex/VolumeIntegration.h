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


#ifndef PX_FOUNDATION_NXVOLUMEINTEGRATION
#define PX_FOUNDATION_NXVOLUMEINTEGRATION
/** \addtogroup foundation
  @{
*/


#include "Px.h"
#include "PxVec3.h"
#include "PxMat33.h"
#include "CmPhysXCommon.h"

namespace physx
{

class PxSimpleTriangleMesh;

/**
\brief Data structure used to store mass properties.
*/
struct PxIntegrals
	{
	PxVec3 COM;					//!< Center of mass
	PxF64 mass;						//!< Total mass
	PxF64 inertiaTensor[3][3];		//!< Inertia tensor (mass matrix) relative to the origin
	PxF64 COMInertiaTensor[3][3];	//!< Inertia tensor (mass matrix) relative to the COM

	/**
	\brief Retrieve the inertia tensor relative to the center of mass.

	\param inertia Inertia tensor.
	*/
	void getInertia(PxMat33& inertia)
	{
		for(PxU32 j=0;j<3;j++)
		{
			for(PxU32 i=0;i<3;i++)
			{
				inertia(i,j) = (PxF32)COMInertiaTensor[i][j];
			}
		}
	}

	/**
	\brief Retrieve the inertia tensor relative to the origin.

	\param inertia Inertia tensor.
	*/
	void getOriginInertia(PxMat33& inertia)
	{
		for(PxU32 j=0;j<3;j++)
		{
			for(PxU32 i=0;i<3;i++)
			{
				inertia(i,j) = (PxF32)inertiaTensor[i][j];
			}
		}
	}
	};

	bool computeVolumeIntegrals(const PxSimpleTriangleMesh& mesh, PxReal density, PxIntegrals& integrals);

	bool computeVolumeIntegralsEberly(const PxSimpleTriangleMesh& mesh, PxReal density, PxIntegrals& integrals, const PxVec3& origin);   // Eberly simplified method
}

 /** @} */
#endif
