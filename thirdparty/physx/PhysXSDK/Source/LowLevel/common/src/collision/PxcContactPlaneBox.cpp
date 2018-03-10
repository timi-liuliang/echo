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

#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"
#include "GuContactBuffer.h"
#include "CmMatrix34.h"

#include "PsArray.h"
#include "PsUtilities.h"


using namespace physx;

namespace physx
{
bool PxcContactPlaneBox(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);
	PX_UNUSED(shape0);

	// Get actual shape data      
	//const PxPlaneGeometry& shapePlane = shape.get<const PxPlaneGeometry>();
	const PxBoxGeometry& shapeBox = shape1.get<const PxBoxGeometry>();
	
	const PxVec3 negPlaneNormal = -transform0.q.getBasisVector0();
	
	//Make sure we have a normalized plane
	//PX_ASSERT(PxAbs(shape0.mNormal.magnitudeSquared() - 1.0f) < 0.000001f);

	Cm::Matrix34 boxMatrix(transform1);
	Cm::Matrix34 boxToPlane(transform0.transformInv(transform1));

	PxVec3 point;

	PX_ASSERT(contactBuffer.count==0);

/*	for(int vx=-1; vx<=1; vx+=2)
		for(int vy=-1; vy<=1; vy+=2)
			for(int vz=-1; vz<=1; vz+=2)
			{				
				//point = boxToPlane.transform(PxVec3(shapeBox.halfExtents.x*vx, shapeBox.halfExtents.y*vy, shapeBox.halfExtents.z*vz));	
				//PxReal planeEq = point.x;
				//Optimized a bit
				point.set(shapeBox.halfExtents.x*vx, shapeBox.halfExtents.y*vy, shapeBox.halfExtents.z*vz);
				const PxReal planeEq = boxToPlane.base0.x*point.x + boxToPlane.base1.x*point.y + boxToPlane.base2.x*point.z + boxToPlane.base3.x;

				if(planeEq <= contactDistance)
				{
					contactBuffer.contact(boxMatrix.transform(point), negPlaneNormal, planeEq);
					
					//no point in making more than 4 contacts.
					if (contactBuffer.count >= 6) //was: 4)	actually, with strong interpenetration more than just the bottom surface goes through,
						//and we want to find the *deepest* 4 vertices, really.
						return true;
				}
			}*/

	// PT: the above code is shock full of LHS/FCMPs. And there's no point in limiting the number of contacts to 6 when the max possible is 8.

	const PxReal limit = contactDistance - boxToPlane.base3.x;
	const PxReal dx = shapeBox.halfExtents.x;
	const PxReal dy = shapeBox.halfExtents.y;
	const PxReal dz = shapeBox.halfExtents.z;
	const PxReal bxdx = boxToPlane.base0.x * dx;
	const PxReal bxdy = boxToPlane.base1.x * dy;
	const PxReal bxdz = boxToPlane.base2.x * dz;

	PxReal depths[8];
	depths[0] =   bxdx + bxdy + bxdz - limit;
	depths[1] =   bxdx + bxdy - bxdz - limit;
	depths[2] =   bxdx - bxdy + bxdz - limit;
	depths[3] =   bxdx - bxdy - bxdz - limit;
	depths[4] = - bxdx + bxdy + bxdz - limit;
	depths[5] = - bxdx + bxdy - bxdz - limit;
	depths[6] = - bxdx - bxdy + bxdz - limit;
	depths[7] = - bxdx - bxdy - bxdz - limit;

	//const PxU32* binary = reinterpret_cast<const PxU32*>(depths);
	const PxU32* binary = PxUnionCast<PxU32*, PxF32*>(depths);

	if(binary[0] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(dx, dy, dz)), negPlaneNormal, depths[0] + contactDistance);
	if(binary[1] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(dx, dy, -dz)), negPlaneNormal, depths[1] + contactDistance);
	if(binary[2] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(dx, -dy, dz)), negPlaneNormal, depths[2] + contactDistance);
	if(binary[3] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(dx, -dy, -dz)), negPlaneNormal, depths[3] + contactDistance);
	if(binary[4] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(-dx, dy, dz)), negPlaneNormal, depths[4] + contactDistance);
	if(binary[5] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(-dx, dy, -dz)), negPlaneNormal, depths[5] + contactDistance);
	if(binary[6] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(-dx, -dy, dz)), negPlaneNormal, depths[6] + contactDistance);
	if(binary[7] & 0x80000000)
		contactBuffer.contact(boxMatrix.transform(PxVec3(-dx, -dy, -dz)), negPlaneNormal, depths[7] + contactDistance);

	return contactBuffer.count > 0;
}
}
