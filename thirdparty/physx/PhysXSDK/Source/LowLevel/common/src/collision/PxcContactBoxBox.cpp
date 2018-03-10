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

#include "GuContactBuffer.h"
#include "PxcNpCache.h"
#include "PxcContactMethodImpl.h"
#include "CmMatrix34.h"

using namespace physx;
using namespace Gu;

#define MAX_NB_CTCS	8 + 12*5 + 6*4
#define ABS_GREATER(x, y)			(PxAbs(x) > (y))
#define ABS_SMALLER_EQUAL(x, y)		(PxAbs(x) <= (y))
//#define AIR(x)					((PxU32&)(x)&SIGN_BITMASK)
//#define ABS_GREATER(x, y)			(AIR(x) > IR(y))
//#define ABS_SMALLER_EQUAL(x, y)		(AIR(x) <= IR(y))

#ifdef PX_X86

	// Some float optimizations ported over from novodex.

	//returns non zero if the value is negative.
	#define PXC_IS_NEGATIVE(x) (((PxU32&)(x)) & 0x80000000)

#else

	//On most platforms using the integer rep is worse(produces LHSs) since the CPU has more registers.

	//returns non zero if the value is negative.
	#define PXC_IS_NEGATIVE(x) ((x) < 0.0f)

#endif


enum
{
	AXIS_A0, AXIS_A1, AXIS_A2,
	AXIS_B0, AXIS_B1, AXIS_B2
};

struct VertexInfo
{
	PxVec3	pos;
	bool	penetrate;
	bool	area;
};


/*static PxI32 doBoxBoxContactGeneration(PxVec3 ctcPts[MAX_NB_CTCS], PxReal depths[MAX_NB_CTCS], PxVec3* ctcNrm,
									 const PxVec3& extents0, const PxVec3& extents1,
									 PxU32& collisionData,
									 const Cm::Matrix34& transform0, const Cm::Matrix34& transform1, PxReal contactDistance);*/

static PxI32 doBoxBoxContactGeneration(ContactBuffer& contactBuffer,
									 const PxVec3& extents0, const PxVec3& extents1,
									 PxU32& collisionData,
									 const Cm::Matrix34& transform0, const Cm::Matrix34& transform1, PxReal contactDistance);

namespace physx
{
bool PxcContactBoxBox(CONTACT_METHOD_ARGS)
{
	// Get actual shape data
	const PxBoxGeometry& shapeBox0 = shape0.get<const PxBoxGeometry>();
	const PxBoxGeometry& shapeBox1 = shape1.get<const PxBoxGeometry>();

	PxU32 pd = npCache.getPairData();
	PxI32 Nb = doBoxBoxContactGeneration(contactBuffer,
					shapeBox0.halfExtents, shapeBox1.halfExtents,
					pd,
					Cm::Matrix34(transform0), Cm::Matrix34(transform1),
					contactDistance);

	npCache.setPairData(pd);

	if(!Nb)
	{
		npCache.setPairData(0);	// Mark as separated for temporal coherence
		return false;			// WARNING: the contact stream code below used to output stuff even for 0 contacts (!). Now we just return here.
	}
	return true;
}
}

// face => 4 vertices of a face of the cube (i.e. a quad)
static PX_FORCE_INLINE PxReal IsInYZ(const PxReal y, const PxReal z, const VertexInfo** PX_RESTRICT face)
{
	// Warning, indices have been remapped. We're now actually like this:
	//
	// 3+------+2
	//  |   |  |
	//  |   *--|
	//  | (y,z)|
	// 0+------+1
	PxReal PreviousY = face[3]->pos.y;
	PxReal PreviousZ = face[3]->pos.z;

	// Loop through quad vertices
	for(PxI32 i=0; i<4; i++)
	{
		const PxReal CurrentY = face[i]->pos.y;
		const PxReal CurrentZ = face[i]->pos.z;

		// |CurrentY - PreviousY      y - PreviousY|
		// |CurrentZ - PreviousZ      z - PreviousZ|
		// => similar to backface culling, check each one of the 4 triangles are consistent, in which case
		// the point is within the parallelogram.
		if((CurrentY - PreviousY)*(z - PreviousZ) - (CurrentZ - PreviousZ)*(y - PreviousY) >= 0.0f)	return -1.0f;

		PreviousY = CurrentY;
		PreviousZ = CurrentZ;
	}

	PxReal x = face[0]->pos.x;
	{
		const PxReal ay = y - face[0]->pos.y;
		const PxReal az = z - face[0]->pos.z;

		PxVec3 b = face[1]->pos - face[0]->pos;	// ### could be precomputed ?
		x += b.x * (ay*b.y + az*b.z) / b.magnitudeSquared();		// ### could be precomputed ?

		b = face[3]->pos - face[0]->pos;		// ### could be precomputed ?
		x += b.x * (ay*b.y + az*b.z) / b.magnitudeSquared();		// ### could be precomputed ?
	}

	return x;
}

// Test with respect to the quad defined by (0,-y1,-z1) and (0,y1,z1)
//  +------+ y1    y
//  |      |       |
//  |  *   |       |
//  |      |       |
//  +------+ -y1   *-----z
static PxI32 generateContacts(//PxVec3 ctcPts[], PxReal depths[], 
								ContactBuffer& contactBuffer, const PxVec3& contactNormal,
								PxReal y1, PxReal z1, const PxVec3& box2,
								const Cm::Matrix34& transform0, const Cm::Matrix34& transform1, PxReal contactDistance)
{
//	PxI32 NbContacts=0;
	contactBuffer.reset();
	y1 += contactDistance;
	z1 += contactDistance;

	const Cm::Matrix34 trans1to0 = transform0.getInverseRT() * transform1;

	VertexInfo vtx[8];	// The 8 cube vertices
//	PxI32 i;

	//     6+------+7
	//     /|     /|
	//    / |    / |
	//   / 4+---/--+5
	// 2+------+3 /    y   z
	//  | /    | /     |  /
	//  |/     |/      |/
	// 0+------+1      *---x

	{
		const PxVec3 ex = trans1to0.base0 * box2.x;
		const PxVec3 ey = trans1to0.base1 * box2.y;
		const PxVec3 ez = trans1to0.base2 * box2.z;
		
		/*		
		vtx[0].pos = mat.pos - ex - ey - ez;
		vtx[1].pos = mat.pos + ex - ey - ez;
		vtx[2].pos = mat.pos - ex + ey - ez;
		vtx[3].pos = mat.pos + ex + ey - ez;
		vtx[4].pos = mat.pos - ex - ey + ez;
		vtx[5].pos = mat.pos + ex - ey + ez;
		vtx[6].pos = mat.pos - ex + ey + ez;
		vtx[7].pos = mat.pos + ex + ey + ez;
		*/

		// 12 vector ops = 12*3 = 36 FPU ops
		vtx[0].pos = vtx[2].pos = vtx[4].pos = vtx[6].pos = trans1to0.base3 - ex;
		vtx[1].pos = vtx[3].pos = vtx[5].pos = vtx[7].pos = trans1to0.base3 + ex;

		PxVec3 e = ey+ez;
		vtx[0].pos -= e;
		vtx[1].pos -= e;
		vtx[6].pos += e;
		vtx[7].pos += e;
		
		e = ey-ez;
		vtx[2].pos += e;
		vtx[3].pos += e;
		vtx[4].pos -= e;
		vtx[5].pos -= e;
	}

	// Create vertex info for 8 vertices
	for(PxU32 i=0; i<8; i++)
	{
		// Vertex suivant
		VertexInfo& p = vtx[i];
		// test the point with respect to the x = 0 plane
		//		if(p.pos.x < 0)
		if(p.pos.x < -contactDistance)	//if(PXC_IS_NEGATIVE(p.pos.x))
		{
			p.area		= false;
			p.penetrate	= false;
			continue;
		}

		{
			// we penetrated the quad plane
			p.penetrate = true;
			// test to see if we are in the quad
			// PxAbs => thus we test Y with respect to -Y1 and +Y1 (same for Z)
			//			if(PxAbs(p->pos.y) <= y1 && PxAbs(p->pos.z) <= z1)
			if(ABS_SMALLER_EQUAL(p.pos.y, y1) && ABS_SMALLER_EQUAL(p.pos.z, z1))
			{
				// the point is inside the quad
				p.area=true;
				// Since we are testing with respect to x = 0, the penetration is directly the x coordinate.
//				depths[NbContacts] = p.pos.x;

				// We take the vertex as the impact point
//				ctcPts[NbContacts++] = p.pos;
				contactBuffer.contact(p.pos, contactNormal, -p.pos.x);
			}
			else
			{
				p.area=false;
			}
		}
	}

	// Teste 12 edges on the quad
	static const PxI32 indices[]={ 0,1, 1,3, 3,2, 2,0, 4,5, 5,7, 7,6, 6,4, 0,4, 1,5, 2,6, 3,7, };
	const PxI32* runningLine = indices;
	const PxI32* endLine = runningLine+24;
	while(runningLine!=endLine)
	{
		// The two vertices of the current edge
		const VertexInfo* p1 = &vtx[*runningLine++];
		const VertexInfo* p2 = &vtx[*runningLine++];

		// Penetrate|Area|Penetrate|Area => 16 cases

		// We only take the edges that at least penetrated the quad's plane into account.
		if(p1->penetrate || p2->penetrate)
			//		if(p1->penetrate + p2->penetrate)	// One branch only
		{
			// If at least one of the two vertices is not in the quad...
			if(!p1->area || !p2->area)
				//			if(!p1->area + !p2->area)	// One branch only
			{
				// Test y
				if(p1->pos.y > p2->pos.y)		{ const VertexInfo* tmp=p1; p1=p2; p2=tmp; }
				// Impact on the +Y1 edge of the quad
				if(p1->pos.y < +y1 && p2->pos.y >= +y1)	
					// => a point under Y1, the other above
				{
					// Case 1
					PxReal a = (+y1 - p1->pos.y)/(p2->pos.y - p1->pos.y);
					PxReal z = p1->pos.z + (p2->pos.z - p1->pos.z)*a;
					if(PxAbs(z) <= z1)
					{
						PxReal x = p1->pos.x + (p2->pos.x - p1->pos.x)*a;
						if(x+contactDistance>=0.0f)
						{
//							depths[NbContacts] = x;
//							ctcPts[NbContacts++] = PxVec3(x, y1, z);
							contactBuffer.contact(PxVec3(x, y1, z), contactNormal, -x);
						}
					}
				}
				// Impact on the edge -Y1 of the quad
				if(p1->pos.y < -y1 && p2->pos.y >= -y1)
				{
					// Case 2
					PxReal a = (-y1 - p1->pos.y)/(p2->pos.y - p1->pos.y);
					PxReal z = p1->pos.z + (p2->pos.z - p1->pos.z)*a;
					if(PxAbs(z) <= z1)
					{
						PxReal x = p1->pos.x + (p2->pos.x - p1->pos.x)*a;
						if(x+contactDistance>=0.0f)
						{
//							depths[NbContacts] = x;
//							ctcPts[NbContacts++] = PxVec3(x, -y1, z);
							contactBuffer.contact(PxVec3(x, -y1, z), contactNormal, -x);
						}
					}
				}

				// Test z
				if(p1->pos.z > p2->pos.z)		{ const VertexInfo* tmp=p1; p1=p2; p2=tmp; }
				// Impact on the edge +Z1 of the quad
				if(p1->pos.z < +z1 && p2->pos.z >= +z1)
				{
					// Case 3
					PxReal a = (+z1 - p1->pos.z)/(p2->pos.z - p1->pos.z);
					PxReal y = p1->pos.y + (p2->pos.y - p1->pos.y)*a;
					if(PxAbs(y) <= y1)
					{
						PxReal x = p1->pos.x + (p2->pos.x - p1->pos.x)*a;
						if(x+contactDistance>=0.0f)
						{
//							depths[NbContacts] = x;
//							ctcPts[NbContacts++] = PxVec3(x, y, z1);
							contactBuffer.contact(PxVec3(x, y, z1), contactNormal, -x);
						}
					}
				}
				// Impact on the edge -Z1 of the quad
				if(p1->pos.z < -z1 && p2->pos.z >= -z1)
				{
					// Case 4
					PxReal a = (-z1 - p1->pos.z)/(p2->pos.z - p1->pos.z);
					PxReal y = p1->pos.y + (p2->pos.y - p1->pos.y)*a;
					if(PxAbs(y) <= y1)
					{
						PxReal x = p1->pos.x + (p2->pos.x - p1->pos.x)*a;
						if(x+contactDistance>=0.0f)
						{
//							depths[NbContacts] = x;
//							ctcPts[NbContacts++] = PxVec3(x, y, -z1);
							contactBuffer.contact(PxVec3(x, y, -z1), contactNormal, -x);
						}
					}
				}
			}

			// The case where one point penetrates the plane, and the other is not in the quad.
			if((!p1->penetrate && !p2->area) || (!p2->penetrate && !p1->area))
			{
				// Case 5
				PxReal a = (-p1->pos.x)/(p2->pos.x - p1->pos.x);
				PxReal y = p1->pos.y + (p2->pos.y - p1->pos.y)*a;
				if(PxAbs(y) <= y1)
				{
					PxReal z = p1->pos.z + (p2->pos.z - p1->pos.z)*a;
					if(PxAbs(z) <= z1)
					{
//						depths[NbContacts] = 0;
//						ctcPts[NbContacts++] = PxVec3(0, y, z);
						contactBuffer.contact(PxVec3(0, y, z), contactNormal, 0);
					}
				}
			}

		}
	}

	{
		// 6 quads => 6 faces of the cube
		static const PxI32 face[][4]={ {0,1,3,2}, {1,5,7,3}, {5,4,6,7}, {4,0,2,6}, {2,3,7,6}, {0,4,5,1} };
		PxI32 addflg=0;
		for(PxU32 i=0; i<6 && addflg!=0x0f; i++)
		{
			const PxI32* p = face[i];
			const VertexInfo* q[4];
			if((q[0]=&vtx[p[0]])->penetrate && (q[1]=&vtx[p[1]])->penetrate && (q[2]=&vtx[p[2]])->penetrate && (q[3]=&vtx[p[3]])->penetrate)
			{
				if(!q[0]->area || !q[1]->area || !q[2]->area || !q[3]->area)
				{
					if(!(addflg&1))	{ PxReal x = IsInYZ(-y1, -z1, q); if(x>=0.0f)	{ addflg|=1; contactBuffer.contact(PxVec3(x, -y1, -z1), contactNormal, -x); /*depths[NbContacts]=x; ctcPts[NbContacts++] = PxVec3(x, -y1, -z1);*/ } }
					if(!(addflg&2))	{ PxReal x = IsInYZ(+y1, -z1, q); if(x>=0.0f)	{ addflg|=2; contactBuffer.contact(PxVec3(x, +y1, -z1), contactNormal, -x); /*depths[NbContacts]=x; ctcPts[NbContacts++] = PxVec3(x, +y1, -z1);*/ } }
					if(!(addflg&4))	{ PxReal x = IsInYZ(-y1, +z1, q); if(x>=0.0f)	{ addflg|=4; contactBuffer.contact(PxVec3(x, -y1, +z1), contactNormal, -x); /*depths[NbContacts]=x; ctcPts[NbContacts++] = PxVec3(x, -y1, +z1);*/ } }
					if(!(addflg&8))	{ PxReal x = IsInYZ(+y1, +z1, q); if(x>=0.0f)	{ addflg|=8; contactBuffer.contact(PxVec3(x, +y1, +z1), contactNormal, -x); /*depths[NbContacts]=x; ctcPts[NbContacts++] = PxVec3(x, +y1, +z1);*/ } }
				}
			}
		}
	}

//	for(i=0; i<NbContacts; i++)
	for(PxU32 i=0; i<contactBuffer.count; i++)
//		ctcPts[i] = transform0.transform(ctcPts[i]);	// local to world
		contactBuffer.contacts[i].point = transform0.transform(contactBuffer.contacts[i].point);	// local to world

	//PX_ASSERT(NbContacts);	//if this did not make contacts then something went wrong in theory, but even the old code without distances had this flaw!
//	return NbContacts;
	return (PxI32)contactBuffer.count;
}

#if !defined(PX_VMX)

//static PxI32 doBoxBoxContactGeneration(PxVec3 ctcPts[MAX_NB_CTCS], PxReal depths[MAX_NB_CTCS], PxVec3* ctcNrm,
static PxI32 doBoxBoxContactGeneration(ContactBuffer& contactBuffer,
									 const PxVec3& extents0, const PxVec3& extents1,
									 PxU32& collisionData,
									 const Cm::Matrix34& transform0, const Cm::Matrix34& transform1, PxReal contactDistance)
{
	PxReal aafC[3][3];			// matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
	PxReal aafAbsC[3][3];		// |c_{ij}|
	PxReal afAD[3];			// Dot(A_i,D)

	PxReal d1[6];
	PxReal overlap[6];

	PxVec3 kD = transform1.base3 - transform0.base3;

	const PxVec3& axis00 = transform0.base0;
	const PxVec3& axis01 = transform0.base1;
	const PxVec3& axis02 = transform0.base2;
	const PxVec3& axis10 = transform1.base0;
	const PxVec3& axis11 = transform1.base1;
	const PxVec3& axis12 = transform1.base2;

	// Perform Class I tests

	aafC[0][0]	= axis00.dot(axis10);
	aafC[0][1]	= axis00.dot(axis11);
	aafC[0][2]	= axis00.dot(axis12);
	afAD[0]		= axis00.dot(kD);
	aafAbsC[0][0] = 1e-6f + PxAbs(aafC[0][0]);
	aafAbsC[0][1] = 1e-6f + PxAbs(aafC[0][1]);
	aafAbsC[0][2] = 1e-6f + PxAbs(aafC[0][2]);
	d1[AXIS_A0] = afAD[0];
	PxReal d0 = extents0.x + extents1.x*aafAbsC[0][0] + extents1.y*aafAbsC[0][1] + extents1.z*aafAbsC[0][2];
	overlap[AXIS_A0] = d0 - PxAbs(d1[AXIS_A0]) + contactDistance;
	if(PXC_IS_NEGATIVE(overlap[AXIS_A0]))		return 0;

	aafC[1][0]	= axis01.dot(axis10);
	aafC[1][1]	= axis01.dot(axis11);
	aafC[1][2]	= axis01.dot(axis12);
	afAD[1]		= axis01.dot(kD);
	aafAbsC[1][0] = 1e-6f + PxAbs(aafC[1][0]);
	aafAbsC[1][1] = 1e-6f + PxAbs(aafC[1][1]);
	aafAbsC[1][2] = 1e-6f + PxAbs(aafC[1][2]);
	d1[AXIS_A1] = afAD[1];
	d0 = extents0.y + extents1.x*aafAbsC[1][0] + extents1.y*aafAbsC[1][1] + extents1.z*aafAbsC[1][2];
	overlap[AXIS_A1] = d0 - PxAbs(d1[AXIS_A1]) + contactDistance;
	if(PXC_IS_NEGATIVE(overlap[AXIS_A1]))		return 0;

	aafC[2][0]	= axis02.dot(axis10);
	aafC[2][1]	= axis02.dot(axis11);
	aafC[2][2]	= axis02.dot(axis12);
	afAD[2]		= axis02.dot(kD);
	aafAbsC[2][0] = 1e-6f + PxAbs(aafC[2][0]);
	aafAbsC[2][1] = 1e-6f + PxAbs(aafC[2][1]);
	aafAbsC[2][2] = 1e-6f + PxAbs(aafC[2][2]);
	d1[AXIS_A2] = afAD[2];
	d0 = extents0.z + extents1.x*aafAbsC[2][0] + extents1.y*aafAbsC[2][1] + extents1.z*aafAbsC[2][2];
	overlap[AXIS_A2] = d0 - PxAbs(d1[AXIS_A2]) + contactDistance;
	if(PXC_IS_NEGATIVE(overlap[AXIS_A2]))		return 0;

	// Perform Class II tests

	d1[AXIS_B0] = axis10.dot(kD);
	d0 = extents1.x + extents0.x*aafAbsC[0][0] + extents0.y*aafAbsC[1][0] + extents0.z*aafAbsC[2][0];
	overlap[AXIS_B0] = d0 - PxAbs(d1[AXIS_B0]) + contactDistance;
	if(PXC_IS_NEGATIVE(overlap[AXIS_B0]))		return 0;

	d1[AXIS_B1] = axis11.dot(kD);
	d0 = extents1.y + extents0.x*aafAbsC[0][1] + extents0.y*aafAbsC[1][1] + extents0.z*aafAbsC[2][1];
	overlap[AXIS_B1] = d0 - PxAbs(d1[AXIS_B1]) + contactDistance;
	if(PXC_IS_NEGATIVE(overlap[AXIS_B1]))		return 0;

	d1[AXIS_B2] = axis12.dot(kD);
	d0 = extents1.z + extents0.x*aafAbsC[0][2] + extents0.y*aafAbsC[1][2] + extents0.z*aafAbsC[2][2];
	overlap[AXIS_B2] = d0 - PxAbs(d1[AXIS_B2]) + contactDistance;
	if(PXC_IS_NEGATIVE(overlap[AXIS_B2]))		return 0;

	// Perform Class III tests - we don't need to store distances for those ones.
	// We only test those axes when objects are likely to be separated, i.e. when they where previously non-colliding. For stacks, we'll have
	// to do full contact generation anyway, and those tests are useless - so we skip them. This is similar to what I did in Opcode.
	if(!collisionData)	// separated or first run
	{
		PxReal d = afAD[2]*aafC[1][0] - afAD[1]*aafC[2][0];
		d0 = contactDistance + extents0.y*aafAbsC[2][0] + extents0.z*aafAbsC[1][0] + extents1.y*aafAbsC[0][2] + extents1.z*aafAbsC[0][1];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[2]*aafC[1][1] - afAD[1]*aafC[2][1];
		d0 = contactDistance + extents0.y*aafAbsC[2][1] + extents0.z*aafAbsC[1][1] + extents1.x*aafAbsC[0][2] + extents1.z*aafAbsC[0][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[2]*aafC[1][2] - afAD[1]*aafC[2][2];
		d0 = contactDistance + extents0.y*aafAbsC[2][2] + extents0.z*aafAbsC[1][2] + extents1.x*aafAbsC[0][1] + extents1.y*aafAbsC[0][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[0]*aafC[2][0] - afAD[2]*aafC[0][0];
		d0 = contactDistance + extents0.x*aafAbsC[2][0] + extents0.z*aafAbsC[0][0] + extents1.y*aafAbsC[1][2] + extents1.z*aafAbsC[1][1];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[0]*aafC[2][1] - afAD[2]*aafC[0][1];
		d0 = contactDistance + extents0.x*aafAbsC[2][1] + extents0.z*aafAbsC[0][1] + extents1.x*aafAbsC[1][2] + extents1.z*aafAbsC[1][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[0]*aafC[2][2] - afAD[2]*aafC[0][2];
		d0 = contactDistance + extents0.x*aafAbsC[2][2] + extents0.z*aafAbsC[0][2] + extents1.x*aafAbsC[1][1] + extents1.y*aafAbsC[1][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[1]*aafC[0][0] - afAD[0]*aafC[1][0];
		d0 = contactDistance + extents0.x*aafAbsC[1][0] + extents0.y*aafAbsC[0][0] + extents1.y*aafAbsC[2][2] + extents1.z*aafAbsC[2][1];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[1]*aafC[0][1] - afAD[0]*aafC[1][1];
		d0 = contactDistance + extents0.x*aafAbsC[1][1] + extents0.y*aafAbsC[0][1] + extents1.x*aafAbsC[2][2] + extents1.z*aafAbsC[2][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[1]*aafC[0][2] - afAD[0]*aafC[1][2];
		d0 = contactDistance + extents0.x*aafAbsC[1][2] + extents0.y*aafAbsC[0][2] + extents1.x*aafAbsC[2][1] + extents1.y*aafAbsC[2][0];
		if(ABS_GREATER(d, d0))	return 0;
	}

	/* djs - tempUserData can be zero when it gets here 
	- maybe if there was no previous axis? 
	- which causes stack corruption, and thence a crash, in .NET

	PT:	right! At first tempUserData wasn't ever supposed to be zero, but then I used that
	value to mark separation of boxes, and forgot to update the code below. Now I think
	the test is redundant with the one performed above, and the line could eventually
	be merged in the previous block. I'll do that later when removing all the #defines.
	*/
	// NB: the "16" here has nothing to do with MAX_NB_CTCS. Don't touch.
	if(collisionData) // if initialized & not previously separated
		overlap[collisionData-1] *= 0.999f;	 // Favorise previous axis  .999 is too little.

	PxReal	minimum = PX_MAX_REAL;
	PxI32	minIndex = 0;

	for(PxU32 i=AXIS_A0; i<6; i++)
	{
		PxReal d = overlap[i];

		if(d>=0.0f && d<minimum)	{ minimum=d; minIndex=(PxI32)i; }		// >=0 !!  otherwise bug at sep = 0
	}

	collisionData = PxU32(minIndex + 1);	// Leave "0" for separation


#ifdef PX_X86
	PxU32 sign = PXC_IS_NEGATIVE(d1[minIndex]);
#else
	PxU32 sign = (PxU32)PXC_IS_NEGATIVE(d1[minIndex]);
#endif
	Cm::Matrix34 trs;
	PxVec3 ctcNrm;

	switch(minIndex)
	{
	default:
		return 0;

	case AXIS_A0:
//		*ctcNrm = axis00;
		if(sign)	
		{
			ctcNrm = axis00;
			trs.base0 = transform0.base0;
			trs.base1 = transform0.base1;
			trs.base2 = transform0.base2;
			trs.base3 = transform0.base3 - extents0.x*axis00;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;
			ctcNrm = -axis00;

			trs.base0 = -axis00;
			trs.base1 = -axis01;
			trs.base2 = axis02;
			trs.base3 = transform0.base3 + extents0.x*axis00;
		}
//		return generateContacts(ctcPts, depths, extents0.y, extents0.z, extents1, trs, transform1, contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents0.y, extents0.z, extents1, trs, transform1, contactDistance);

	case AXIS_A1:
//		*ctcNrm = axis01;
		trs.base2 = axis00;	// Factored out
		if(sign)	
		{
			ctcNrm = axis01;
			trs.base0 = axis01;
			trs.base1 = axis02;
			trs.base3 = transform0.base3 - extents0.y*axis01;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis01;
			
			trs.base0 = -axis01;
			trs.base1 = -axis02;
			trs.base3 = transform0.base3 + extents0.y*axis01;
		}
//		return generateContacts(ctcPts, depths, extents0.z, extents0.x, extents1, trs, transform1, contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents0.z, extents0.x, extents1, trs, transform1, contactDistance);

	case AXIS_A2:
//		*ctcNrm = axis02;
		trs.base2 = axis01;	// Factored out

		if(sign)	
		{
			ctcNrm = axis02;
			trs.base0 = axis02;
			trs.base1 = axis00;
			trs.base3 = transform0.base3 - extents0.z*axis02;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis02;
			
			trs.base0 = -axis02;
			trs.base1 = -axis00;
			trs.base3 = transform0.base3 + extents0.z*axis02;
		}
//		return generateContacts(ctcPts, depths, extents0.x, extents0.y, extents1, trs, transform1, contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents0.x, extents0.y, extents1, trs, transform1, contactDistance);

	case AXIS_B0:
//		*ctcNrm = axis10;
		if(sign)	
		{
			ctcNrm = axis10;
			trs.base0 = -axis10;
			trs.base1 = -axis11;
			trs.base2 = axis12;
			trs.base3 = transform1.base3 + extents1.x*axis10;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis10;
			
			trs.base0 = transform1.base0;
			trs.base1 = transform1.base1;
			trs.base2 = transform1.base2;
			trs.base3 = transform1.base3 - extents1.x*axis10;
		
		}
//		return generateContacts(ctcPts, depths, extents1.y, extents1.z, extents0, trs, transform0, contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents1.y, extents1.z, extents0, trs, transform0, contactDistance);

	case AXIS_B1:
//		*ctcNrm = axis11;
		trs.base2 = axis10;	// Factored out
		if(sign)	
		{
			ctcNrm = axis11;
			trs.base0 = -axis11;
			trs.base1 = -axis12;
			trs.base3 = transform1.base3 + extents1.y*axis11;
		}
		else		
		{ 
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis11;
			
			trs.base0 = axis11;
			trs.base1 = axis12;
			trs.base2 = axis10;
			trs.base3 = transform1.base3 - extents1.y*axis11;
		}
//		return generateContacts(ctcPts, depths, extents1.z, extents1.x, extents0, trs, transform0, contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents1.z, extents1.x, extents0, trs, transform0, contactDistance);

	case AXIS_B2:
//		*ctcNrm = axis12;
		trs.base2 = axis11;	// Factored out

		if(sign)	
		{
			ctcNrm = axis12;
			trs.base0 = -axis12;
			trs.base1 = -axis10;
			trs.base3 = transform1.base3 + extents1.z*axis12;
		}
		else		
		{ 
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis12;
			
			trs.base0 = axis12;
			trs.base1 = axis10;
			trs.base3 = transform1.base3 - extents1.z*axis12;
		}

//		return generateContacts(ctcPts, depths, extents1.x, extents1.y, extents0, trs, transform0, contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents1.x, extents1.y, extents0, trs, transform0, contactDistance);
	}
}

#else

#include "PsVecMath.h"
using namespace Ps::aos;

//static PxI32 doBoxBoxContactGeneration(PxVec3 ctcPts[MAX_NB_CTCS], PxReal depths[MAX_NB_CTCS], PxVec3* ctcNrm,
static PxI32 doBoxBoxContactGeneration(ContactBuffer& contactBuffer,
									 const PxVec3& extents0, const PxVec3& extents1,
									 PxU32& collisionData,
									 const Cm::Matrix34& transform0, const Cm::Matrix34& transform1, PxReal contactDistance)
{
	const FloatV fzero = FZero();

	FloatV gContactDistance4 = FLoad(contactDistance);
	Vec3V aafC0, aafC1, aafC2;
	Vec3V aafAbsC0, aafAbsC1, aafAbsC2;

	FloatV afAD0, afAD1, afAD2;
	
	FloatV overlap_A0,overlap_A1,overlap_A2;

	Vec3V kD = V3Sub(V3LoadU(transform1.base3), V3LoadU(transform0.base3));
	const Vec3V eps = V3Load(1e-6f);

	/////////////////////////
	Vec3V axis00_4 = V3LoadU(transform0.base0);
	
	Vec3V axis10_4 = V3LoadU(transform1.base0);
	Vec3V axis11_4 = V3LoadU(transform1.base1);
	Vec3V axis12_4 = V3LoadU(transform1.base2);

	Vec3V extents0_4 = V3LoadU(extents0);
	Vec3V extents1_4 = V3LoadU(extents1);

	{
		aafC0 = V3Merge(
			V3Dot(axis00_4, axis10_4), 
			V3Dot(axis00_4, axis11_4), 
			V3Dot(axis00_4, axis12_4));

		afAD0 = V3Dot(axis00_4, kD);
		aafAbsC0 = V3Add(eps, V3Abs(aafC0));
		
		FloatV d0 = FAdd(V3GetX(extents0_4), V3Dot(extents1_4, aafAbsC0));

		overlap_A0 = FAdd(FSub(d0, FAbs(afAD0)), gContactDistance4);

		if(FAllGrtr(fzero, overlap_A0))
			return 0;
	}

	////////////////////
	
	Vec3V axis01_4 = V3LoadU(transform0.base1);
	
	{
		aafC1 = V3Merge(
			V3Dot(axis01_4, axis10_4), 
			V3Dot(axis01_4, axis11_4), 
			V3Dot(axis01_4, axis12_4));

		afAD1 = V3Dot(axis01_4, kD);
		aafAbsC1 = V3Add(eps, V3Abs(aafC1));

		FloatV d0 = FAdd(V3GetY(extents0_4), V3Dot(extents1_4, aafAbsC1));

		overlap_A1 = FAdd(FSub(d0, FAbs(afAD1)), gContactDistance4);

		if(FAllGrtr(fzero, overlap_A1))
			return 0;
	}

	/////////////////

	Vec3V axis02_4 = V3LoadU(transform0.base2);

	{
		aafC2 = V3Merge(
			V3Dot(axis02_4, axis10_4), 
			V3Dot(axis02_4, axis11_4), 
			V3Dot(axis02_4, axis12_4));

		afAD2 = V3Dot(axis02_4, kD);
		aafAbsC2 = V3Add(eps, V3Abs(aafC2));

		FloatV d0 = FAdd(V3GetZ(extents0_4), V3Dot(extents1_4, aafAbsC2));

		overlap_A2 = FAdd(FSub(d0, FAbs(afAD2)), gContactDistance4);

		if(FAllGrtr(fzero, overlap_A2))
			return 0;
	}
	///////////

	// Perform Class II tests

	FloatV d1_B0, d1_B1, d1_B2;
	FloatV overlap_B0,overlap_B1,overlap_B2;
	//////////
	{
		d1_B0 = V3Dot(axis10_4, kD);
		FloatV d0 = FAdd(V3GetX(extents1_4), 
			V3Dot(extents0_4, V3ColX(aafAbsC0, aafAbsC1, aafAbsC2)));

		overlap_B0 = FAdd(FSub(d0, FAbs(d1_B0)), gContactDistance4);
		
		if(FAllGrtr(fzero, overlap_B0))
			return 0;
	}
	////////
	{
		d1_B1 = V3Dot(axis11_4, kD);
		FloatV d0 = FAdd(V3GetY(extents1_4), 
			V3Dot(extents0_4, V3ColY(aafAbsC0, aafAbsC1, aafAbsC2)));

		overlap_B1 = FAdd(FSub(d0, FAbs(d1_B1)), gContactDistance4);
		
		if(FAllGrtr(fzero, overlap_B1))
			return 0;
	}
	////////
	{	
		d1_B2 = V3Dot(axis12_4, kD);
		FloatV d0 = FAdd(V3GetZ(extents1_4), 
			V3Dot(extents0_4, V3ColZ(aafAbsC0, aafAbsC1, aafAbsC2)));

		overlap_B2 = FAdd(FSub(d0, FAbs(d1_B2)), gContactDistance4);
		
		if(FAllGrtr(fzero, overlap_B2))
			return 0;
	}


	const PxVec3& axis00 = transform0.base0;
	const PxVec3& axis01 = transform0.base1;
	const PxVec3& axis02 = transform0.base2;
	const PxVec3& axis10 = transform1.base0;
	const PxVec3& axis11 = transform1.base1;
	const PxVec3& axis12 = transform1.base2;

	PxReal PX_ALIGN(16, overlap[6]);
	PxReal PX_ALIGN(16, d1[6]);

	FStore(overlap_A0,&overlap[AXIS_A0]);
	FStore(overlap_A1,&overlap[AXIS_A1]);
	FStore(overlap_A2,&overlap[AXIS_A2]);

	FStore(overlap_B0,&overlap[AXIS_B0]);
	FStore(overlap_B1,&overlap[AXIS_B1]);
	FStore(overlap_B2,&overlap[AXIS_B2]);

	FStore(afAD0,&d1[AXIS_A0]);
	FStore(afAD1,&d1[AXIS_A1]);
	FStore(afAD2,&d1[AXIS_A2]);

	FStore(d1_B0,&d1[AXIS_B0]);
	FStore(d1_B1,&d1[AXIS_B1]);
	FStore(d1_B2,&d1[AXIS_B2]);

//TODO: vmx128 class III tests.

#if 0
	// Perform Class III tests - we don't need to store distances for those ones.
	// We only test those axes when objects are likely to be separated, i.e. when they where previously non-colliding. For stacks, we'll have
	// to do full contact generation anyway, and those tests are useless - so we skip them. This is similar to what I did in Opcode.
	if(!collisionData || collisionData==0xff)	// separated or first run
	{
/////////


		PxReal d = afAD[2]*aafC[1][0] - afAD[1]*aafC[2][0];
		d0 = extents0.y*aafAbsC[2][0] + extents0.z*aafAbsC[1][0] + extents1.y*aafAbsC[0][2] + extents1.z*aafAbsC[0][1];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[2]*aafC[1][1] - afAD[1]*aafC[2][1];
		d0 = extents0.y*aafAbsC[2][1] + extents0.z*aafAbsC[1][1] + extents1.x*aafAbsC[0][2] + extents1.z*aafAbsC[0][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[2]*aafC[1][2] - afAD[1]*aafC[2][2];
		d0 = extents0.y*aafAbsC[2][2] + extents0.z*aafAbsC[1][2] + extents1.x*aafAbsC[0][1] + extents1.y*aafAbsC[0][0];
		if(ABS_GREATER(d, d0))	return 0;
////////////////

		d = afAD[0]*aafC[2][0] - afAD[2]*aafC[0][0];
		d0 = extents0.x*aafAbsC[2][0] + extents0.z*aafAbsC[0][0] + extents1.y*aafAbsC[1][2] + extents1.z*aafAbsC[1][1];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[0]*aafC[2][1] - afAD[2]*aafC[0][1];
		d0 = extents0.x*aafAbsC[2][1] + extents0.z*aafAbsC[0][1] + extents1.x*aafAbsC[1][2] + extents1.z*aafAbsC[1][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[0]*aafC[2][2] - afAD[2]*aafC[0][2];
		d0 = extents0.x*aafAbsC[2][2] + extents0.z*aafAbsC[0][2] + extents1.x*aafAbsC[1][1] + extents1.y*aafAbsC[1][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[1]*aafC[0][0] - afAD[0]*aafC[1][0];
		d0 = extents0.x*aafAbsC[1][0] + extents0.y*aafAbsC[0][0] + extents1.y*aafAbsC[2][2] + extents1.z*aafAbsC[2][1];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[1]*aafC[0][1] - afAD[0]*aafC[1][1];
		d0 = extents0.x*aafAbsC[1][1] + extents0.y*aafAbsC[0][1] + extents1.x*aafAbsC[2][2] + extents1.z*aafAbsC[2][0];
		if(ABS_GREATER(d, d0))	return 0;

		d = afAD[1]*aafC[0][2] - afAD[0]*aafC[1][2];
		d0 = extents0.x*aafAbsC[1][2] + extents0.y*aafAbsC[0][2] + extents1.x*aafAbsC[2][1] + extents1.y*aafAbsC[2][0];
		if(ABS_GREATER(d, d0))	return 0;
	}
#endif

	/* djs - tempUserData can be zero when it gets here 
	- maybe if there was no previous axis? 
	- which causes stack corruption, and thence a crash, in .NET

	PT:	right! At first tempUserData wasn't ever supposed to be zero, but then I used that
	value to mark separation of boxes, and forgot to update the code below. Now I think
	the test is redundant with the one performed above, and the line could eventually
	be merged in the previous block. I'll do that later when removing all the #defines.
	*/
	// NB: the "16" here has nothing to do with MAX_NB_CTCS. Don't touch.
	if(collisionData <= 16 && collisionData>0) // if initialized & not previously separated
		overlap[collisionData-1] *= 0.999f;	 // Favorise previous axis  .999 is too little.

	PxReal	minimum = PX_MAX_REAL;
	PxI32	minIndex = 0;

	for(PxU32 i=AXIS_A0; i<6; i++)
	{
		PxReal d = overlap[i];

		if(d>=0.0f && d<minimum)	{ minimum=d; minIndex=(PxI32)i; }		// >=0 !!  otherwise bug at sep = 0
	}

	collisionData = PxU32(minIndex + 1);	// Leave "0" for separation

	PxU32 sign = (PxU32)PXC_IS_NEGATIVE(d1[minIndex]);

	Cm::Matrix34 trs;
	PxVec3 ctcNrm;

	switch(minIndex)
	{
	default:
		return 0;

	case AXIS_A0:
//		*ctcNrm = axis00;
		if(sign)	
		{
			ctcNrm = axis00;
			trs.base0 = transform0.base0;
			trs.base1 = transform0.base1;
			trs.base2 = transform0.base2;
			trs.base3 = transform0.base3 - extents0.x*axis00;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;
			ctcNrm = -axis00;

			trs.base0 = -axis00;
			trs.base1 = -axis01;
			trs.base2 = axis02;
			trs.base3 = transform0.base3 + extents0.x*axis00;
		}
//		return generateContacts(ctcPts, depths, extents0.y, extents0.z, extents1, trs, transform1,contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents0.y, extents0.z, extents1, trs, transform1,contactDistance);

	case AXIS_A1:
//		*ctcNrm = axis01;
		trs.base2 = axis00;	// Factored out
		if(sign)	
		{
			ctcNrm = axis01;
			trs.base0 = axis01;
			trs.base1 = axis02;
			trs.base3 = transform0.base3 - extents0.y*axis01;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis01;
			
			trs.base0 = -axis01;
			trs.base1 = -axis02;
			trs.base3 = transform0.base3 + extents0.y*axis01;
		}
//		return generateContacts(ctcPts, depths, extents0.z, extents0.x, extents1, trs, transform1,contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents0.z, extents0.x, extents1, trs, transform1,contactDistance);

	case AXIS_A2:
//		*ctcNrm = axis02;
		trs.base2 = axis01;	// Factored out

		if(sign)	
		{
			ctcNrm = axis02;
			trs.base0 = axis02;
			trs.base1 = axis00;
			trs.base3 = transform0.base3 - extents0.z*axis02;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis02;
			
			trs.base0 = -axis02;
			trs.base1 = -axis00;
			trs.base3 = transform0.base3 + extents0.z*axis02;
		}
//		return generateContacts(ctcPts, depths, extents0.x, extents0.y, extents1, trs, transform1,contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents0.x, extents0.y, extents1, trs, transform1,contactDistance);

	case AXIS_B0:
//		*ctcNrm = axis10;
		if(sign)	
		{
			ctcNrm = axis10;
			trs.base0 = -axis10;
			trs.base1 = -axis11;
			trs.base2 = axis12;
			trs.base3 = transform1.base3 + extents1.x*axis10;
		}
		else		
		{
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis10;
			
			trs.base0 = transform1.base0;
			trs.base1 = transform1.base1;
			trs.base2 = transform1.base2;
			trs.base3 = transform1.base3 - extents1.x*axis10;
		
		}
//		return generateContacts(ctcPts, depths, extents1.y, extents1.z, extents0, trs, transform0,contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents1.y, extents1.z, extents0, trs, transform0,contactDistance);

	case AXIS_B1:
//		*ctcNrm = axis11;
		trs.base2 = axis10;	// Factored out
		if(sign)	
		{
			ctcNrm = axis11;
			trs.base0 = -axis11;
			trs.base1 = -axis12;
			trs.base3 = transform1.base3 + extents1.y*axis11;
		}
		else		
		{ 
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis11;
			
			trs.base0 = axis11;
			trs.base1 = axis12;
			trs.base2 = axis10;
			trs.base3 = transform1.base3 - extents1.y*axis11;
		}
//		return generateContacts(ctcPts, depths, extents1.z, extents1.x, extents0, trs, transform0,contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents1.z, extents1.x, extents0, trs, transform0,contactDistance);

	case AXIS_B2:
//		*ctcNrm = axis12;
		trs.base2 = axis11;	// Factored out

		if(sign)	
		{
			ctcNrm = axis12;
			trs.base0 = -axis12;
			trs.base1 = -axis10;
			trs.base3 = transform1.base3 + extents1.z*axis12;
		}
		else		
		{ 
//			*ctcNrm = -*ctcNrm;	
			ctcNrm = -axis12;
			
			trs.base0 = axis12;
			trs.base1 = axis10;
			trs.base3 = transform1.base3 - extents1.z*axis12;
		}

//		return generateContacts(ctcPts, depths, extents1.x, extents1.y, extents0, trs, transform0,contactDistance);
		return generateContacts(contactBuffer, ctcNrm, extents1.x, extents1.y, extents0, trs, transform0,contactDistance);
	}
}

#endif

