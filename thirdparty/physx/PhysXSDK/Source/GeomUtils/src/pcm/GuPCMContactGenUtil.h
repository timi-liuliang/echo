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

#ifndef GU_PCM_CONTACT_GEN_UTIL_H
#define GU_PCM_CONTACT_GEN_UTIL_H

#include "PsVecMath.h"
#include "CmPhysXCommon.h"
#include "GuShapeConvex.h"
#include "GuVecCapsule.h"
#include "GuConvexSupportTable.h"

namespace physx
{

namespace Gu
{

	enum FeatureStatus
	{
		POLYDATA0,
		POLYDATA1,
		EDGE0,
		EDGE1
	};

	PX_FORCE_INLINE bool contains(Ps::aos::Vec3V* verts, const PxU32 numVerts, const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg min, const Ps::aos::Vec3VArg max) 
	{ 
		using namespace Ps::aos;

		
		const BoolV bTrue = BTTTT();
		
		
		const BoolV tempCon = BOr(V3IsGrtr(min, p), V3IsGrtr(p, max));
		const BoolV con = BOr(BGetX(tempCon), BGetY(tempCon));
		
		if(BAllEq(con, bTrue))
			return false;  

		const FloatV tx = V3GetX(p);
		const FloatV ty = V3GetY(p); 

		const FloatV eps = FEps();
		const FloatV zero = FZero();
		PxU32 intersectionPoints = 0;

		PxU32 i = 0, j = numVerts - 1;

		for(; i < numVerts; j = i++)
		{

			const FloatV jy = V3GetY(verts[j]);
			const FloatV iy = V3GetY(verts[i]);

			//(verts[i].y > test.y) != (points[j].y > test.y) 
			const PxU32 yflag0 = FAllGrtr(jy, ty);
			const PxU32 yflag1 = FAllGrtr(iy, ty);

			//ML: the only case the ray will intersect this segment is when the p's y is in between two segments y
			if(yflag0 != yflag1)
			{
				const FloatV jx = V3GetX(verts[j]);
				const FloatV ix = V3GetX(verts[i]);
						
				//ML: choose ray, which start at p and every points in the ray will have the same y component
				//t1 = (yp - yj)/(yi - yj)
				//qx = xj + t1*(xi-xj)
				//t = qx - xp > 0 for the ray and segment intersection happen
				const FloatV jix = FSub(ix, jx);
				const FloatV jiy = FSub(iy, jy);
				//const FloatV jtx = FSub(tx, jy);
				const FloatV jty = FSub(ty, jy);
				const FloatV part1 = FMul(jty, jix);
				//const FloatV part2 = FMul(jx,  jiy);
				//const FloatV part3 = FMul(V3Sub(tx, eps),  jiy);
				const FloatV part2 = FMul(FAdd(jx, eps),  jiy);
				const FloatV part3 = FMul(tx,  jiy);

				const BoolV comp = FIsGrtr(jiy, zero);
				const FloatV tmp = FAdd(part1, part2);
				const FloatV comp1 = FSel(comp, tmp, part3);
				const FloatV comp2 = FSel(comp, part3, tmp);


				if(FAllGrtrOrEq(comp1, comp2))
				{
					if(intersectionPoints == 1)
					{
						return false;
					}
					intersectionPoints++;
				}
			}
		}
		return intersectionPoints> 0;
    } 


	PX_FORCE_INLINE bool boxContainsInXY(const Ps::aos::FloatVArg x, const Ps::aos::FloatVArg y, const Ps::aos::Vec3VArg p, const Ps::aos::Vec3V* verts, const Ps::aos::Vec3VArg min, const Ps::aos::Vec3VArg max)
	{
		using namespace Ps::aos;

		const BoolV bTrue = BTTTT();
		const BoolV tempCon = BOr(V3IsGrtr(min, p), V3IsGrtr(p, max));
		const BoolV con = BOr(BGetX(tempCon), BGetY(tempCon));
		
		if(BAllEq(con, bTrue))
			return false;  

		
		const FloatV zero = FZero();
		FloatV PreviousX = V3GetX(verts[3]);
		FloatV PreviousY = V3GetY(verts[3]);


		// Loop through quad vertices
		for(PxI32 i=0; i<4; i++)
		{
			const FloatV CurrentX = V3GetX(verts[i]);
			const FloatV CurrentY = V3GetY(verts[i]);

			// |CurrentX - PreviousX      x - PreviousX|
			// |CurrentY - PreviousY      y - PreviousY|
			// => similar to backface culling, check each one of the 4 triangles are consistent, in which case
			// the point is within the parallelogram.
			const FloatV v00 = FSub(CurrentX, PreviousX);
			const FloatV v01 = FSub(y,  PreviousY);
			const FloatV v10 = FSub(CurrentY, PreviousY);
			const FloatV v11 = FSub(x,  PreviousX);
			const FloatV temp0 = FMul(v00, v01);
			const FloatV temp1 = FMul(v10, v11);
			if(FAllGrtrOrEq(FSub(temp0, temp1), zero))
				return false;

			PreviousX = CurrentX;
			PreviousY = CurrentY;
		}

		return true;

	}


	PX_FORCE_INLINE Ps::aos::FloatV signed2DTriArea(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, const Ps::aos::Vec3VArg c)
	{
		using namespace Ps::aos;
		const Vec3V ca = V3Sub(a, c);
		const Vec3V cb = V3Sub(b, c);

		const FloatV t0 = FMul(V3GetX(ca), V3GetY(cb));
		const FloatV t1 = FMul(V3GetY(ca), V3GetX(cb));

		return FSub(t0, t1);
	}

#if defined(PX_X360) || defined(PX_PS3) || defined(__SPU__)

	//normal is in vertex space, need to transform the vertex space
	PX_FORCE_INLINE PxI32 getPolygonIndex(const Gu::PolygonalData& polyData, SupportLocal* map, const Ps::aos::Vec3VArg normal)
	{
		using namespace Ps::aos;

		//normal is in shape space, need to transform the vertex space
		const Vec3V n = M33TrnspsMulV3(map->vertex2Shape, normal);
		const Vec3V nnormal = V3Neg(n);
		const Vec3V planeNormal =  V3LoadU(polyData.mPolygons[0].mPlane.n);
		FloatV minProj = V3Dot(n, planeNormal);
	
		const VecI32V vOne = VecI32V_One();  
		const VecI32V vZero = VecI32V_Zero();
		const FloatV zero = FZero();
		//get incident face
		VecI32V ind = vZero;
		VecI32V inc = vOne;
		for(PxU32 i=1; i< polyData.mNbPolygons; ++i, inc = VecI32V_Add(inc, vOne))
		{
			Vec3V planeN = V3LoadU(polyData.mPolygons[i].mPlane.n);
			const FloatV proj = V3Dot(n, planeN);
			const BoolV res = FIsGrtr(minProj, proj);
			minProj = FSel(res, proj, minProj);
			ind = VecI32V_Sel(res, inc, ind);
		}
		PxI32 closestFaceIndex;
		PxI32_From_VecI32V(ind, (PxI32*)&closestFaceIndex);

		const PxU32 numEdges = polyData.mNbEdges;
		const PxU8* const edgeToFace = polyData.mFacesByEdges;

		//Loop through edges
		//PxU32 closestEdge = 0xffffffff;
		FloatV maxDpSq = FMul(minProj, minProj);
		ind = VecI32V_MinusOne();
		inc = vZero;
		for(PxU32 i=0; i < numEdges; ++i, inc = VecI32V_Add(inc, vOne))
		{
			const PxU32 index = i*2;
			const PxU8 f0 = edgeToFace[index];
			const PxU8 f1 = edgeToFace[index+1];

			const Vec3V planeNormal0 = V3LoadU(polyData.mPolygons[f0].mPlane.n);
			const Vec3V planeNormal1 = V3LoadU(polyData.mPolygons[f1].mPlane.n);

			// unnormalized edge normal
			const Vec3V edgeNormal = V3Add(planeNormal0, planeNormal1);//polys[f0].mPlane.n + polys[f1].mPlane.n;
			const FloatV enMagSq = V3Dot(edgeNormal, edgeNormal);//edgeNormal.magnitudeSquared();
			//Test normal of current edge - squared test is valid if dp and maxDp both >= 0
			const FloatV dp = V3Dot(edgeNormal, nnormal);//edgeNormal.dot(normal);
			const FloatV sqDp = FMul(dp, dp);
			const BoolV con0 = FIsGrtrOrEq(dp, zero);
			const BoolV con1 = FIsGrtr(sqDp, FMul(maxDpSq, enMagSq));
			const BoolV con = BAnd(con0, con1);
			maxDpSq = FSel(con, FDiv(sqDp, enMagSq), maxDpSq);
			ind = VecI32V_Sel(con, inc, ind);
		}

		PxI32 closestEdge;
		PxI32_From_VecI32V(ind, (PxI32*)&closestEdge);
		
		if(closestEdge!=-1)
		{
			const PxU8* FBE = edgeToFace;

			const PxU32 index = PxU32(closestEdge*2);
			const PxU32 f0 = FBE[index];
			const PxU32 f1 = FBE[index+1];

			const Vec3V planeNormal0 = V3LoadU(polyData.mPolygons[f0].mPlane.n);
			const Vec3V planeNormal1 = V3LoadU(polyData.mPolygons[f1].mPlane.n);

			const FloatV dp0 = V3Dot(planeNormal0, nnormal);
			const FloatV dp1 = V3Dot(planeNormal1, nnormal);
			if(FAllGrtr(dp0, dp1))
			{
				closestFaceIndex = (PxI32)f0;
			}
			else
			{
				closestFaceIndex = (PxI32)f1;
			}
		}
		//return polyData.mPolygons[closestFaceIndex];
		return closestFaceIndex;

	}

	PX_FORCE_INLINE const Gu::HullPolygonData& getGJKPolygonIndex(const Gu::PolygonalData& polyData,  SupportLocal* map, const Ps::aos::Vec3VArg normal)
	{
		using namespace Ps::aos;

		//normal is in shape space, need to transform the vertex space
		const Vec3V n = M33TrnspsMulV3(map->vertex2Shape, normal);
		Vec3V planeNoraml =  V3LoadU(polyData.mPolygons[0].mPlane.n);
		FloatV minProj = V3Dot(n, planeNoraml);
		//incidentIndex = 0;

		const VecI32V vOne = VecI32V_One();
		//get incident face
		VecI32V ind = VecI32V_Zero();
		VecI32V inc = vOne;
		for(PxU32 i=1; i< polyData.mNbPolygons; ++i, inc = VecI32V_Add(inc, vOne))
		{
			Vec3V planeN = V3LoadU(polyData.mPolygons[i].mPlane.n);
			const FloatV proj = V3Dot(n, planeN);
			const BoolV res = FIsGrtr(minProj, proj);
			minProj = FSel(res, proj, minProj);
			ind = VecI32V_Sel(res, inc, ind);
		}
		PxI32 closestFaceIndex;
		PxI32_From_VecI32V(ind, (PxI32*)&closestFaceIndex);

		return polyData.mPolygons[closestFaceIndex];
	}

#else
	PX_FORCE_INLINE PxI32 getPolygonIndex(const Gu::PolygonalData& polyData, SupportLocal* map, const Ps::aos::Vec3VArg normal)
	{
		using namespace Ps::aos;

		//normal is in shape space, need to transform the vertex space
		const Vec3V n = M33TrnspsMulV3(map->vertex2Shape, normal);
		const Vec3V nnormal = V3Neg(n);
		const Vec3V planeN =  Vec3V_From_Vec4V(V4LoadU(&polyData.mPolygons[0].mPlane.n.x));
		FloatV minProj = V3Dot(n, planeN);

		const FloatV zero = FZero();
		const BoolV bTrue = BTTTT();
		PxI32 closestFaceIndex = 0;

		for(PxU32 i=1; i< polyData.mNbPolygons; ++i)
		{
			Vec3V planeNi = V3LoadU(polyData.mPolygons[i].mPlane.n);
			const FloatV proj = V3Dot(n, planeNi);
			if(FAllGrtr(minProj, proj))
			{
				minProj = proj;
				closestFaceIndex = (PxI32)i;
			}
		}

		const PxU32 numEdges = polyData.mNbEdges;
		const PxU8* const edgeToFace = polyData.mFacesByEdges;

		//Loop through edges
		PxU32 closestEdge = 0xffffffff;
		FloatV maxDpSq = FMul(minProj, minProj);

		for(PxU32 i=0; i < numEdges; ++i)//, inc = VecI32V_Add(inc, vOne))
		{
			const PxU32 index = i*2;
			const PxU8 f0 = edgeToFace[index];
			const PxU8 f1 = edgeToFace[index+1];

			const Vec3V planeNormal0 = V3LoadU(polyData.mPolygons[f0].mPlane.n);
			const Vec3V planeNormal1 = V3LoadU(polyData.mPolygons[f1].mPlane.n);

			// unnormalized edge normal
			const Vec3V edgeNormal = V3Add(planeNormal0, planeNormal1);//polys[f0].mPlane.n + polys[f1].mPlane.n;
			const FloatV enMagSq = V3Dot(edgeNormal, edgeNormal);//edgeNormal.magnitudeSquared();
			//Test normal of current edge - squared test is valid if dp and maxDp both >= 0
			const FloatV dp = V3Dot(edgeNormal, nnormal);//edgeNormal.dot(normal);
			const FloatV sqDp = FMul(dp, dp);
			/*if(FAllGrtrOrEq(dp, zero) && FAllGrtr(sqDp, FMul(maxDpSq, enMagSq)))
			{
				maxDpSq = FDiv(sqDp, enMagSq);
				closestEdge = i;
			}*/
			const BoolV con0 = FIsGrtrOrEq(dp, zero);
			const BoolV con1 = FIsGrtr(sqDp, FMul(maxDpSq, enMagSq));
			const BoolV con = BAnd(con0, con1);
			if(BAllEq(con, bTrue))
			{
				maxDpSq = FDiv(sqDp, enMagSq);
				closestEdge = i;
			}
		} 

		if(closestEdge!=0xffffffff)
		{
			const PxU8* FBE = edgeToFace;

			const PxU32 index = closestEdge*2;
			const PxU32 f0 = FBE[index];
			const PxU32 f1 = FBE[index+1];

			const Vec3V planeNormal0 = V3LoadU(polyData.mPolygons[f0].mPlane.n);
			const Vec3V planeNormal1 = V3LoadU(polyData.mPolygons[f1].mPlane.n);

			const FloatV dp0 = V3Dot(planeNormal0, nnormal);
			const FloatV dp1 = V3Dot(planeNormal1, nnormal);
			if(FAllGrtr(dp0, dp1))
			{
				closestFaceIndex = (PxI32)f0;
			}
			else
			{
				closestFaceIndex = (PxI32)f1;
			}
		}

		return closestFaceIndex;

	}

	PX_FORCE_INLINE const Gu::HullPolygonData& getGJKPolygonIndex(const Gu::PolygonalData& polyData,  SupportLocal* map, const Ps::aos::Vec3VArg normal)
	{
		using namespace Ps::aos;

		//normal is in shape space, need to transform the vertex space
		const Vec3V n = M33TrnspsMulV3(map->vertex2Shape, normal);
		Vec3V planeN =  V3LoadU(polyData.mPolygons[0].mPlane.n);
		FloatV minProj = V3Dot(n, planeN);

		PxU32 closestFaceIndex = 0;

		for(PxU32 i=1; i< polyData.mNbPolygons; ++i)
		{
			Vec3V planeNi = V3LoadU(polyData.mPolygons[i].mPlane.n);
			const FloatV proj = V3Dot(n, planeNi);
			if(FAllGrtr(minProj, proj))
			{
				minProj = proj;
				closestFaceIndex = i;
			}
		}

		return polyData.mPolygons[closestFaceIndex];
	}
#endif	


	//ML: this function is shared by the sphere/capsule vs convex hulls full contact gen, capsule in the local space of polyData
	PX_FORCE_INLINE bool testPolyDataAxis(const Gu::CapsuleV& capsule, const Gu::PolygonalData& polyData, SupportLocal* map,  const Ps::aos::FloatVArg contactDist, Ps::aos::FloatV& minOverlap, Ps::aos::Vec3V& seperatingAxis)
	{
		using namespace Ps::aos;
		FloatV _minOverlap = FMax();//minOverlap;
		FloatV min0, max0;
		FloatV min1, max1;
		Vec3V tempAxis = V3UnitY();
		const BoolV bTrue = BTTTT();

		//capsule in the local space of polyData
		for(PxU32 i=0; i<polyData.mNbPolygons; ++i)
		{
			const Gu::HullPolygonData& polygon = polyData.mPolygons[i];

			const Vec3V minVert = V3LoadU(polyData.mVerts[polygon.mMinIndex]);
			const FloatV planeDist = FLoad(polygon.mPlane.d);
			const Vec3V vertexSpacePlaneNormal = V3LoadU(polygon.mPlane.n);

			//transform plane n to shape space
			const Vec3V shapeSpacePlaneNormal = M33TrnspsMulV3(map->shape2Vertex, vertexSpacePlaneNormal);

			const FloatV magnitude = FRecip(V3Length(shapeSpacePlaneNormal));
			//normalize shape space normal
			const Vec3V planeN = V3Scale(shapeSpacePlaneNormal, magnitude);
			//ML::use this to avoid LHS
			min0 = FMul(V3Dot(vertexSpacePlaneNormal, minVert), magnitude);
			max0 = FMul(FNeg(planeDist), magnitude);

		
			const FloatV tempMin = V3Dot(capsule.p0, planeN);
			const FloatV tempMax = V3Dot(capsule.p1, planeN);
			min1 = FMin(tempMin, tempMax);
			max1 = FMax(tempMin, tempMax);

			min1 = FSub(min1, capsule.radius);
			max1 = FAdd(max1, capsule.radius);


			const BoolV con = BOr(FIsGrtr(min1, FAdd(max0, contactDist)), FIsGrtr(min0, FAdd(max1, contactDist)));

			if(BAllEq(con, bTrue))
				return false;

			const FloatV tempOverlap = FSub(max0, min1);

			if(FAllGrtr(_minOverlap, tempOverlap))
			{
				_minOverlap = tempOverlap;
				tempAxis = planeN;
			}
		}

		seperatingAxis = tempAxis;
		minOverlap = _minOverlap;

		return true;

	}  

	//ML: this function is shared by sphere/capsule. Point a and direction d need to be in the local space of polyData
	PX_FORCE_INLINE bool intersectSegmentPolyhedron(const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg dir, const PolygonalData& polyData, Ps::aos::FloatV& tEnter, Ps::aos::FloatV& tExit)
	{
		using namespace Ps::aos;
		const FloatV zero = FZero();
		const FloatV one = FOne();
		const FloatV eps = FLoad(1e-7f);
		FloatV tFirst = zero;
		FloatV tLast= one;

		for(PxU32 k=0; k<polyData.mNbPolygons; ++k)
		{
			const Gu::HullPolygonData& data = polyData.mPolygons[k];
			const Vec3V n =V3LoadU(data.mPlane.n);

			FloatV d = FLoad(data.mPlane.d);
	
			const FloatV denominator = V3Dot(n, dir);
			const FloatV distToPlane = FAdd(V3Dot(n, a), d);
		
			if(FAllGrtr(eps, FAbs(denominator)))
			{
				if(FAllGrtr(distToPlane, zero))
					return false;
			}
			else
			{
				FloatV tTemp = FNeg(FDiv(distToPlane, denominator));

				//ML: denominator < 0 means the ray is entering halfspace; denominator > 0 means the ray is exiting halfspace
				const BoolV con = FIsGrtr(zero, denominator);
				const BoolV con0= FIsGrtr(tTemp, tFirst); 
				const BoolV con1 = FIsGrtr(tLast, tTemp);
	
				tFirst = FSel(BAnd(con, con0), tTemp, tFirst);
				tLast = FSel(BAndNot(con1, con), tTemp, tLast);
			}
			
			if(FAllGrtr(tFirst, tLast))
				return false;
		}

		//calculate the intersect p in the local space
		tEnter = tFirst;
		tExit = tLast;
	
		return true;
	}

}//Gu
}//physx

#endif
