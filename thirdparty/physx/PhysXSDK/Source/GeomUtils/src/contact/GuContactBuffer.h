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

#ifndef GU_CONTACTBUFFER_H
#define GU_CONTACTBUFFER_H

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#include "PxContact.h"
#include "GuContactPoint.h"
#include "PsVecMath.h"

namespace physx
{
namespace Gu
{
class ContactBuffer
{
public:

	static const PxU32 MAX_CONTACTS = 64;

	Gu::ContactPoint	contacts[MAX_CONTACTS];
	PxU32				count;
	PxReal				meshContactMargin;	// PT: Margin used to generate mesh contacts. Temp & unclear, should be removed once GJK is default path.

	PX_FORCE_INLINE void reset()
	{
		count = 0;
	}

	PX_FORCE_INLINE bool contact(const Ps::aos::Vec3VArg worldPoint, 
		const Ps::aos::Vec3VArg worldNormalIn, 
		const Ps::aos::FloatV separation, 
		PxU32 faceIndex0 = PXC_CONTACT_NO_FACE_INDEX,
		PxU32 faceIndex1 = PXC_CONTACT_NO_FACE_INDEX
	)
	{
		//PX_ASSERT(PxAbs(worldNormalIn.magnitude()-1)<1e-3f);

		if(count>=MAX_CONTACTS)
			return false;

		Gu::ContactPoint& p	 = contacts[count++];
		//Fast allign store
		Ps::aos::V4StoreA(Ps::aos::Vec4V_From_Vec3V(worldNormalIn), (PxF32*)&p.normal.x);
		Ps::aos::V4StoreA(Ps::aos::Vec4V_From_Vec3V(worldPoint), (PxF32*)&p.point.x);
		Ps::aos::FStore(separation, &p.separation);

		p.internalFaceIndex0 = faceIndex0;
		p.internalFaceIndex1 = faceIndex1;
		return true;
	}

	PX_FORCE_INLINE bool contact(const PxVec3& worldPoint, 
				 const PxVec3& worldNormalIn, 
				 PxReal separation, 
				 PxU32 faceIndex0 = PXC_CONTACT_NO_FACE_INDEX,
				 PxU32 faceIndex1 = PXC_CONTACT_NO_FACE_INDEX
				 )
	{
		PX_ASSERT(PxAbs(worldNormalIn.magnitude()-1)<1e-3f);

		if(count>=MAX_CONTACTS)
			return false;

		Gu::ContactPoint& p	= contacts[count++];
		p.normal			= worldNormalIn;
		p.point				= worldPoint;
		p.separation		= separation;
		p.internalFaceIndex0= faceIndex0;
		p.internalFaceIndex1= faceIndex1;
		return true;
	}

	PX_FORCE_INLINE bool contact(const Gu::ContactPoint & pt)
	{
		if(count>=MAX_CONTACTS)
			return false;
		contacts[count++] = pt;
		return true;
	}

	PX_FORCE_INLINE Gu::ContactPoint* contact()
	{
		if(count>=MAX_CONTACTS)
			return NULL;
		return &contacts[count++];
	}
};

}
}

#endif
