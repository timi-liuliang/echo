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

#include "GuGeometryUnion.h"
#include "GuPCMTriangleContactGen.h"
#include "GuPCMContactConvexCommon.h"
#include "GuVecTriangle.h"

using namespace physx;
using namespace Gu;

#ifdef	PCM_LOW_LEVEL_DEBUG
#include "CmRenderOutput.h"
extern physx::Cm::RenderOutput* gRenderOutPut;
#endif


#ifdef PX_WIIU  
#pragma ghs nowarning 1656 //within a function using alloca or VLAs, alignment of local variables
#endif

namespace physx
{


	static void barycentricCoordinates(const Ps::aos::Vec3VArg p, const Ps::aos::Vec3VArg a, const Ps::aos::Vec3VArg b, const Ps::aos::Vec3VArg c, Ps::aos::FloatV& v, Ps::aos::FloatV& w)
	{
		using namespace Ps::aos;
		const Vec3V v0 = V3Sub(b, a);
		const Vec3V v1 = V3Sub(c, a);
		const Vec3V v2 = V3Sub(p, a);
		const FloatV d00 = V3Dot(v0, v0);
		const FloatV d01 = V3Dot(v0, v1);
		const FloatV d11 = V3Dot(v1, v1);
		const FloatV d20 = V3Dot(v2, v0);
		const FloatV d21 = V3Dot(v2, v1);
		const FloatV d00d11 = FMul(d00, d11);
		const FloatV d01d01 = FMul(d01, d01);
		const FloatV d11d20 = FMul(d11, d20);
		const FloatV d01d21 = FMul(d01, d21);
		const FloatV d00d21 = FMul(d00, d21);
		const FloatV d01d20 = FMul(d01, d20);
		const FloatV denom = FRecip(FSub(d00d11, d01d01));
		v = FMul(FSub(d11d20, d01d21), denom);
		w = FMul(FSub(d00d21, d01d20), denom);
	}

	static bool testPolyFaceNormal(const Gu::TriangleV& triangle, const Gu::PolygonalData& polyData, Gu::SupportLocalImpl<Gu::TriangleV>* triMap, SupportLocal* polyMap,  const Ps::aos::FloatVArg contactDist, 
		Ps::aos::FloatV& minOverlap, PxU32& feature, Ps::aos::Vec3V& faceNormal, const FeatureStatus faceStatus, FeatureStatus& status)
	{
		PX_UNUSED(triangle);

		using namespace Ps::aos;
		FloatV _minOverlap = FMax();
		PxU32  _feature = 0;
		Vec3V  _faceNormal = faceNormal;
		FloatV min0, max0;
		FloatV min1, max1;
		const BoolV bTrue = BTTTT();
		const FloatV eps = FEps();

		if(polyMap->isIdentityScale)
		{
			//in the local space of polyData0
			for(PxU32 i=0; i<polyData.mNbPolygons; ++i)
			{
				const Gu::HullPolygonData& polygon = polyData.mPolygons[i];

				const Vec3V minVert = V3LoadU(polyData.mVerts[polygon.mMinIndex]);
				const FloatV planeDist = FLoad(polygon.mPlane.d);
				//shapeSpace and vertexSpace are the same
				const Vec3V planeNormal = V3LoadU(polygon.mPlane.n);
	
				//ML::avoid lHS, don't use the exiting function
				min0 = V3Dot(planeNormal, minVert);
				max0 = FNeg(planeDist);

				triMap->doSupportFast(planeNormal, min1, max1);

				const BoolV con = BOr(FIsGrtr(min1, FAdd(max0, contactDist)), FIsGrtr(min0, FAdd(max1, contactDist)));

				if(BAllEq(con, bTrue))
					return false;

				const FloatV tempOverlap = FSub(max0, min1);

				if(FAllGrtr(_minOverlap, tempOverlap))
				{
					_minOverlap = tempOverlap;
					_feature = i;
					_faceNormal = planeNormal;
				}
			}   
		}
		else
		{
		
			//in the local space of polyData0
			for(PxU32 i=0; i<polyData.mNbPolygons; ++i)
			{
				const Gu::HullPolygonData& polygon = polyData.mPolygons[i];

				const Vec3V minVert = V3LoadU(polyData.mVerts[polygon.mMinIndex]);
				const FloatV planeDist = FLoad(polygon.mPlane.d);
				const Vec3V vertexSpacePlaneNormal = V3LoadU(polygon.mPlane.n);
				//transform plane n to shape space
				const Vec3V shapeSpacePlaneNormal = M33TrnspsMulV3(polyMap->shape2Vertex, vertexSpacePlaneNormal);

				const FloatV magnitude = FRsqrtFast(V3LengthSq(shapeSpacePlaneNormal)); //FRecip(V3Length(shapeSpacePlaneNormal));

				//ML::avoid lHS, don't use the exiting function
				min0 = FMul(V3Dot(vertexSpacePlaneNormal, minVert), magnitude);
				max0 = FMul(FNeg(planeDist), magnitude);

				//normalize the shapeSpacePlaneNormal
				const Vec3V planeN = V3Scale(shapeSpacePlaneNormal, magnitude);

				triMap->doSupportFast(planeN, min1, max1);

				const BoolV con = BOr(FIsGrtr(min1, FAdd(max0, contactDist)), FIsGrtr(min0, FAdd(max1, contactDist)));

				if(BAllEq(con, bTrue))
					return false;

				const FloatV tempOverlap = FSub(max0, min1);

				if(FAllGrtr(_minOverlap, tempOverlap))
				{
					_minOverlap = tempOverlap;
					_feature = i;
					_faceNormal = planeN;
				}
			} 
		}

		if(FAllGrtr(minOverlap, FAdd(_minOverlap, eps)))
		{
			faceNormal = _faceNormal;
			minOverlap = _minOverlap;
			status = faceStatus;
		}

		feature = _feature;

		return true;

	}



	//triangle is in the local space of polyData
	static bool testTriangleFaceNormal(const Gu::TriangleV& triangle, const Gu::PolygonalData& polyData, Gu::SupportLocalImpl<Gu::TriangleV>* triMap, SupportLocal* polyMap, const Ps::aos::FloatVArg contactDist, 
		Ps::aos::FloatV& minOverlap, PxU32& feature, Ps::aos::Vec3V& faceNormal, const FeatureStatus faceStatus, FeatureStatus& status)
	{
		PX_UNUSED(triMap);
		PX_UNUSED(polyData);

		using namespace Ps::aos;


		FloatV min1, max1;
		const FloatV eps = FEps();
		const BoolV bTrue = BTTTT();

		const Vec3V triangleLocNormal = triangle.normal();

		const FloatV min0 = V3Dot(triangleLocNormal, triangle.verts[0]);
		const FloatV max0 = min0;

		polyMap->doSupport(triangleLocNormal, min1, max1);

		const BoolV con = BOr(FIsGrtr(min1, FAdd(max0, contactDist)), FIsGrtr(min0, FAdd(max1, contactDist)));

		if(BAllEq(con, bTrue))
			return false;

		minOverlap = FSub(FSub(max0, min1), eps);
		status = faceStatus;
		feature = 0;
		faceNormal=triangleLocNormal;

		return true;

	}

	static bool testPolyEdgeNormal(const Gu::TriangleV& triangle, const PxU8 triFlags, const Gu::HullPolygonData& /*polygon*/, const Gu::PolygonalData& polyData, Gu::SupportLocalImpl<Gu::TriangleV>* triMap, SupportLocal* polyMap, const Ps::aos::FloatVArg contactDist,
		Ps::aos::FloatV& minOverlap, Ps::aos::Vec3V& minNormal, const FeatureStatus edgeStatus, FeatureStatus& status)
	{
		using namespace Ps::aos;
		FloatV overlap = minOverlap;
		FloatV min0, max0;
		FloatV min1, max1;
		const BoolV bTrue = BTTTT();
		const FloatV zero = FZero();
		const Vec3V eps2 = V3Splat(FLoad(1e-6));
		
		const Vec3V v0 = M33MulV3(polyMap->shape2Vertex, triangle.verts[0]);
		const Vec3V v1 = M33MulV3(polyMap->shape2Vertex, triangle.verts[1]);
		const Vec3V v2 = M33MulV3(polyMap->shape2Vertex, triangle.verts[2]);

		TriangleV vertexSpaceTriangle(v0, v1, v2);

		PxU32 nbTriangleAxes = 0;
		Vec3V triangleAxes[3];
		for(PxI8 kStart = 0, kEnd =2; kStart<3; kEnd = kStart++)
		{
			bool active = (triFlags & (1 << (kEnd+3))) != 0;
	
			if(active)
			{
				const Vec3V p00 = vertexSpaceTriangle.verts[kStart];
				const Vec3V p01 = vertexSpaceTriangle.verts[kEnd];
				//change to shape space
				triangleAxes[nbTriangleAxes++] =  V3Sub(p01, p00);
			}
		}

		if(nbTriangleAxes == 0)
			return true;

		
		//create localTriPlane in the vertex space
		const Vec3V vertexSpaceTriangleNormal = vertexSpaceTriangle.normal();
		const FloatV vertexSpaceTriangleD = FNeg(V3Dot(vertexSpaceTriangleNormal, triangle.verts[0]));
	
		for(PxU32 i =0; i<polyData.mNbPolygons; ++i)
		{
			const Gu::HullPolygonData& polygon = polyData.mPolygons[i];
			const PxU8* inds = polyData.mPolygonVertexRefs + polygon.mVRef8;
			const Vec3V vertexSpacePlaneNormal = V3LoadU(polygon.mPlane.n);

			//fast culling. 
			if(FAllGrtr(V3Dot(vertexSpacePlaneNormal, vertexSpaceTriangleNormal), zero))
				continue;

			// Loop through polygon vertices == polygon edges;
			for(PxU32 lStart = 0, lEnd =PxU32(polygon.mNbVerts-1); lStart<polygon.mNbVerts; lEnd = PxU32(lStart++))
			{
				//in the vertex space
				const Vec3V p10 = V3LoadU(polyData.mVerts[inds[lStart]]);
				const Vec3V p11 = V3LoadU(polyData.mVerts[inds[lEnd]]);
				const FloatV signDistP10 = FAdd(V3Dot(p10, vertexSpaceTriangleNormal), vertexSpaceTriangleD);
				const FloatV signDistP11 = FAdd(V3Dot(p11, vertexSpaceTriangleNormal), vertexSpaceTriangleD);

				const BoolV con0 = BOr(FIsGrtrOrEq(contactDist, signDistP10), FIsGrtrOrEq(contactDist, signDistP11));
				if(BAllEq(con0, bTrue))
				{
					const Vec3V dir = V3Sub(p11, p10);

					for(PxU32 k = 0; k < nbTriangleAxes; ++k)
					{
						const Vec3V currentPolyEdge = triangleAxes[k];
						const Vec3V v = V3Cross(dir, currentPolyEdge); 
						const Vec3V absV = V3Abs(v);
						
						if(!V3AllGrtr(eps2, absV))
						{
							//transform the v back to the space space
							const Vec3V shapeSpaceV =  M33TrnspsMulV3(polyMap->shape2Vertex, v);
							const Vec3V n0 = V3Normalize(shapeSpaceV);
							triMap->doSupportFast(n0, min0, max0);
							polyMap->doSupport(n0, min1, max1);
							const BoolV con = BOr(FIsGrtr(min1, FAdd(max0, contactDist)), FIsGrtr(min0, FAdd(max1, contactDist)));
							if(BAllEq(con, bTrue))
								return false;

							const FloatV tempOverlap = FSub(max0, min1);

							if(FAllGrtr(overlap, tempOverlap))
							{
								overlap = tempOverlap;
								minNormal = n0;
								status = edgeStatus;
							}

						}
					}
				}
			}
		}
		minOverlap = overlap;
		
		return true;

	}


	static void generatedTriangleContacts(const Gu::TriangleV& triangle, const PxU32 triangleIndex, const PxU8 _triFlags, const Gu::PolygonalData& polyData1, const Gu::HullPolygonData& incidentPolygon,  Gu::SupportLocal* map1, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts, 
		const Ps::aos::FloatVArg contactDist, const Ps::aos::Vec3VArg contactNormal)
	{
		using namespace Ps::aos;

		PxU8 triFlags = _triFlags;
		const PxU32 previousContacts = numContacts;
		
		const FloatV zero = FZero();
		const BoolV bTrue = BTTTT();
	
		const Mat33V rot = findRotationMatrixFromZAxis(contactNormal);

		const PxU8* inds1 = polyData1.mPolygonVertexRefs + incidentPolygon.mVRef8;

		Vec3V points0In0[3];
		Vec3V* points1In0 = (Vec3V*)PxAllocaAligned(sizeof(Vec3V)*incidentPolygon.mNbVerts, 16);
		FloatV* points1In0TValue = (FloatV*)PxAllocaAligned(sizeof(FloatV)*incidentPolygon.mNbVerts, 16);
		bool* points1In0Penetration = (bool*)PxAlloca(sizeof(bool)*incidentPolygon.mNbVerts);
		

		points0In0[0] = triangle.verts[0];
		points0In0[1] = triangle.verts[1];
		points0In0[2] = triangle.verts[2];



		//Transform all the verts from vertex space to shape space
		map1->populateVerts(inds1, incidentPolygon.mNbVerts, polyData1.mVerts, points1In0);

#ifdef PCM_LOW_LEVEL_DEBUG
		Gu::PersistentContactManifold::drawPolygon(*gRenderOutPut, map1->transform, points1In0, incidentPolygon.mNbVerts, (PxU32)PxDebugColor::eARGB_RED);
		//Gu::PersistentContactManifold::drawTriangle(*gRenderOutPut, map1->transform.transform(points1In0[0]), map1->transform.transform(points0In0[1]), map1->transform.transform(points0In0[2]), (PxU32)PxDebugColor::eARGB_BLUE);
#endif

		PX_ASSERT(incidentPolygon.mNbVerts <= 64);
 
		Vec3V eps = Vec3V_From_FloatV(FEps());
		Vec3V max = Vec3V_From_FloatV(FMax());
		Vec3V nmax = V3Neg(max); 

		//transform reference polygon to 2d, calculate min and max
		Vec3V rPolygonMin= max;
		Vec3V rPolygonMax = nmax;
		for(PxU32 i=0; i<3; ++i)
		{
			points0In0[i] = M33MulV3(rot, points0In0[i]);
			rPolygonMin = V3Min(rPolygonMin, points0In0[i]);
			rPolygonMax = V3Max(rPolygonMax, points0In0[i]);
		}
		
		
		rPolygonMin = V3Sub(rPolygonMin, eps);
		rPolygonMax = V3Add(rPolygonMax, eps);

		const FloatV d = V3GetZ(points0In0[0]);
		const FloatV rd = FAdd(d, contactDist);

		Vec3V iPolygonMin= max; 
		Vec3V iPolygonMax = nmax;


		PxU32 inside = 0;
		for(PxU32 i=0; i<incidentPolygon.mNbVerts; ++i)
		{
			const Vec3V vert1 =points1In0[i]; //this still in polyData1's local space
			points1In0[i] = M33MulV3(rot, vert1);
			const FloatV z = V3GetZ(points1In0[i]);
			points1In0TValue[i] = FSub(z, d);
			points1In0[i] = V3SetZ(points1In0[i], d);
			iPolygonMin = V3Min(iPolygonMin, points1In0[i]);
			iPolygonMax = V3Max(iPolygonMax, points1In0[i]);
			if(FAllGrtr(rd, z))
			{
				points1In0Penetration[i] = true;

				if(contains(points0In0, 3, points1In0[i], rPolygonMin, rPolygonMax))
				{
					inside++;


					const FloatV t = V3Dot(contactNormal, V3Sub(triangle.verts[0], vert1));
					const Vec3V projectPoint = V3ScaleAdd(contactNormal, t, vert1); 
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(contactNormal), FNeg(t));
					manifoldContacts[numContacts].mLocalPointA = vert1;
					manifoldContacts[numContacts].mLocalPointB = projectPoint;
					manifoldContacts[numContacts].mLocalNormalPen = localNormalPen;
					manifoldContacts[numContacts++].mFaceIndex = triangleIndex;

					PX_ASSERT(numContacts <= 64);
				}
			}
			
		}



		if(inside == incidentPolygon.mNbVerts)
		{
			return;
		}

		inside = 0;
		iPolygonMin = V3Sub(iPolygonMin, eps);
		iPolygonMax = V3Add(iPolygonMax, eps);

		const Vec3V incidentNormal = V3Normalize(M33TrnspsMulV3(map1->shape2Vertex, V3LoadU(incidentPolygon.mPlane.n)));
		const FloatV iPlaneD = V3Dot(incidentNormal, M33MulV3(map1->vertex2Shape, V3LoadU(polyData1.mVerts[inds1[0]])));

		for(PxU32 i=0; i<3; ++i)
		{
			if(contains(points1In0, incidentPolygon.mNbVerts, points0In0[i], iPolygonMin, iPolygonMax))
			{
				const Vec3V vert0 = M33TrnspsMulV3(rot, points0In0[i]);
				const FloatV t = FSub(V3Dot(incidentNormal, vert0), iPlaneD);

				if(FAllGrtr(t, contactDist))
					continue;

				const Vec3V projPoint = V3NegScaleSub(incidentNormal, t, vert0);

				const Vec3V v = V3Sub(projPoint, vert0);
				const FloatV t3 = V3Dot(v, contactNormal);

				const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(contactNormal), t3);
				manifoldContacts[numContacts].mLocalPointA = projPoint;
				manifoldContacts[numContacts].mLocalPointB = vert0;
				manifoldContacts[numContacts].mLocalNormalPen = localNormalPen;
				manifoldContacts[numContacts++].mFaceIndex = triangleIndex;
				
					
			}
				
		}


		if(inside == 3)
			return;  
	

		for(PxU32 a = 0; a < 2; ++a)
		{

	#ifdef	__SPU__
			bool allEdgesNotActives = false;
	#else
			bool allEdgesNotActives = ((triFlags >>3) == 0);
	#endif
			if(!allEdgesNotActives)
			{
				//(2) segment intesection
				for (PxU32 rStart = 0, rEnd = 2; rStart < 3; rEnd = rStart++) 
				{
	#ifdef	__SPU__
					bool active = true;
	#else
					bool active = (triFlags & (1 << (rEnd+3))) != 0;
	#endif
					if(!active)
						continue;


					const Vec3V rpA = points0In0[rStart];  
					const Vec3V rpB = points0In0[rEnd];

					const Vec3V rMin = V3Min(rpA, rpB);
					const Vec3V rMax = V3Max(rpA, rpB);

					for (PxU32 iStart = 0, iEnd = PxU32(incidentPolygon.mNbVerts - 1); iStart < incidentPolygon.mNbVerts; iEnd = iStart++)
					{
						if((!points1In0Penetration[iStart] && !points1In0Penetration[iEnd] ) )//|| (points1In0[i].status == POINT_OUTSIDE && points1In0[incidentIndex].status == POINT_OUTSIDE))
							continue;

						const Vec3V ipA = points1In0[iStart];
						const Vec3V ipB = points1In0[iEnd];

						const Vec3V iMin = V3Min(ipA, ipB);
						const Vec3V iMax = V3Max(ipA, ipB);

						const BoolV tempCon =BOr(V3IsGrtr(iMin, rMax), V3IsGrtr(rMin, iMax));
						const BoolV con = BOr(BGetX(tempCon), BGetY(tempCon));
					
						if(BAllEq(con, bTrue))
							continue;

						FloatV a1 = signed2DTriArea(rpA, rpB, ipA);
						FloatV a2 = signed2DTriArea(rpA, rpB, ipB);

						if(FAllGrtr(zero, FMul(a1, a2)))
						{
							FloatV a3 = signed2DTriArea(ipA, ipB, rpA);
							FloatV a4 = signed2DTriArea(ipA, ipB, rpB);

							if(FAllGrtr(zero, FMul(a3, a4)))
							{

								//these two segment intersect in 2d
								const FloatV t = FMul(a1, FRecip(FSub(a2, a1)));

								const Vec3V ipAOri = V3SetZ(points1In0[iStart], FAdd(points1In0TValue[iStart], d));
								const Vec3V ipBOri = V3SetZ(points1In0[iEnd], FAdd(points1In0TValue[iEnd], d));

								const Vec3V pBB = V3NegScaleSub(V3Sub(ipBOri, ipAOri), t, ipAOri); 
								const Vec3V pAA = V3SetZ(pBB, d);
								const Vec3V pA = M33TrnspsMulV3(rot, pAA);
								const Vec3V pB = M33TrnspsMulV3(rot, pBB);
								const FloatV pen = FSub(V3GetZ(pBB), V3GetZ(pAA));
			
								if(FAllGrtr(pen, contactDist))
									continue;
							
								const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(contactNormal), pen);
								manifoldContacts[numContacts].mLocalPointA = pB;
								manifoldContacts[numContacts].mLocalPointB = pA;
								manifoldContacts[numContacts].mLocalNormalPen = localNormalPen;
								manifoldContacts[numContacts++].mFaceIndex = triangleIndex;
								
								PX_ASSERT(numContacts <= 64);
							}
						}
					}
					
				}
			}

			if(previousContacts != numContacts)
			{
				break;
			}
			//invert bits in the active edge flags - inactive edges become active and actives become inactive. Re-run, generating edge contacts with inactive edges
			triFlags = PxU8(triFlags ^ 0xff);
		}
	}


	static void generatedPolyContacts(const Gu::PolygonalData& polyData0, const Gu::HullPolygonData& referencePolygon, const Gu::TriangleV& triangle, const PxU32 triangleIndex, const PxU8 triFlags, Gu::SupportLocal* map0, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts, 
		const Ps::aos::FloatVArg contactDist, const Ps::aos::Vec3VArg contactNormal)
	{
		PX_UNUSED(triFlags);

		using namespace Ps::aos;

		const FloatV zero = FZero();
		const BoolV bTrue = BTTTT();


		const PxU8* inds0 = polyData0.mPolygonVertexRefs + referencePolygon.mVRef8;

		const Vec3V nContactNormal = V3Neg(contactNormal);

		//this is the matrix transform all points to the 2d plane
		const Mat33V rot = findRotationMatrixFromZAxis(contactNormal);

		Vec3V* points0In0=(Vec3V*)PxAllocaAligned(sizeof(Vec3V)*referencePolygon.mNbVerts, 16);
		Vec3V points1In0[3];
		FloatV points1In0TValue[3];

		bool points1In0Penetration[3];
		
		//Transform all the verts from vertex space to shape space
		map0->populateVerts(inds0, referencePolygon.mNbVerts, polyData0.mVerts, points0In0);

		points1In0[0] = triangle.verts[0];
		points1In0[1] = triangle.verts[1];
		points1In0[2] = triangle.verts[2];

#ifdef PCM_LOW_LEVEL_DEBUG
		Gu::PersistentContactManifold::drawPolygon(*gRenderOutPut, map0->transform, points0In0, referencePolygon.mNbVerts, (PxU32)PxDebugColor::eARGB_GREEN);
		//Gu::PersistentContactManifold::drawTriangle(*gRenderOutPut, map0->transform.transform(points1In0[0]), map0->transform.transform(points1In0[1]), map0->transform.transform(points1In0[2]), (PxU32)PxDebugColor::eARGB_BLUE);
#endif

		//the first point in the reference plane
		const Vec3V referencePoint = points0In0[0];
 
		Vec3V eps = Vec3V_From_FloatV(FEps());
		Vec3V max = Vec3V_From_FloatV(FMax());
		Vec3V nmax = V3Neg(max); 

		//transform reference polygon to 2d, calculate min and max
		Vec3V rPolygonMin= max;
		Vec3V rPolygonMax = nmax;
		for(PxU32 i=0; i<referencePolygon.mNbVerts; ++i)
		{
			//points0In0[i].vertext = M33TrnspsMulV3(rot, Vec3V_From_PxVec3(polyData0.mVerts[inds0[i]]));
			points0In0[i] = M33MulV3(rot, points0In0[i]);
			rPolygonMin = V3Min(rPolygonMin, points0In0[i]);
			rPolygonMax = V3Max(rPolygonMax, points0In0[i]);
		}
		
		rPolygonMin = V3Sub(rPolygonMin, eps);
		rPolygonMax = V3Add(rPolygonMax, eps);

		
		
		const FloatV d = V3GetZ(points0In0[0]);

		const FloatV rd = FAdd(d, contactDist);

		Vec3V iPolygonMin= max; 
		Vec3V iPolygonMax = nmax;

		PxU32 inside = 0;
		for(PxU32 i=0; i<3; ++i)
		{
			const Vec3V vert1 =points1In0[i]; //this still in polyData1's local space
			points1In0[i] = M33MulV3(rot, vert1);
			const FloatV z = V3GetZ(points1In0[i]);
			points1In0TValue[i] = FSub(z, d);
			points1In0[i] = V3SetZ(points1In0[i], d);
			iPolygonMin = V3Min(iPolygonMin, points1In0[i]);
			iPolygonMax = V3Max(iPolygonMax, points1In0[i]);
			if(FAllGrtr(rd, z))
			{
				points1In0Penetration[i] = true;

				//ML : check to see whether all the points of triangles in 2D space are within reference polygon's range
				if(contains(points0In0, referencePolygon.mNbVerts, points1In0[i], rPolygonMin, rPolygonMax))
				{
					inside++;

					//calculate projection point
					const FloatV t = V3Dot(contactNormal, V3Sub(vert1, referencePoint));
					const Vec3V projectPoint = V3NegScaleSub(contactNormal, t, vert1);
					
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(nContactNormal), t);
					manifoldContacts[numContacts].mLocalPointA = projectPoint;//M33TrnspsMulV3(rot, points1In0[i]);
					manifoldContacts[numContacts].mLocalPointB =	vert1;
					manifoldContacts[numContacts].mLocalNormalPen = localNormalPen;
					manifoldContacts[numContacts++].mFaceIndex = triangleIndex;

					PX_ASSERT(numContacts <= 64);
				}
			}
			
		}


		if(inside == 3)
		{
			return;
		}

		inside = 0;
		iPolygonMin = V3Sub(iPolygonMin, eps);
		iPolygonMax = V3Add(iPolygonMax, eps);

		const Vec3V incidentNormal = triangle.normal();
		const FloatV iPlaneD = V3Dot(incidentNormal, triangle.verts[0]);
		const FloatV one = FOne();
		for(PxU32 i=0; i<referencePolygon.mNbVerts; ++i)
		{
			if(contains(points1In0, 3, points0In0[i], iPolygonMin, iPolygonMax))
			{
				const Vec3V vert0 = M33TrnspsMulV3(rot, points0In0[i]);

				const FloatV t =FSub(V3Dot(incidentNormal, vert0), iPlaneD);
				
				if(FAllGrtr(t, contactDist))
					continue;

				const Vec3V projPoint = V3NegScaleSub(incidentNormal, t, vert0);

				FloatV u, w;
				barycentricCoordinates(projPoint, triangle.verts[0], triangle.verts[1], triangle.verts[2], u, w);
				const BoolV con = BAnd(FIsGrtrOrEq(u, zero), BAnd(FIsGrtrOrEq(w, zero),  FIsGrtrOrEq(one, FAdd(u, w))));
			
				if(BAllEq(con, bTrue))
				{
					const Vec3V v = V3Sub(projPoint, vert0);
					const FloatV t3 = V3Dot(v, contactNormal);
					
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(nContactNormal), t3);
					manifoldContacts[numContacts].mLocalPointA = vert0;
					manifoldContacts[numContacts].mLocalPointB = projPoint;
					manifoldContacts[numContacts].mLocalNormalPen = localNormalPen;
					manifoldContacts[numContacts++].mFaceIndex = triangleIndex;
				
					PX_ASSERT(numContacts <= 64);
				}
					
			}
				
		}

		if(inside == referencePolygon.mNbVerts)
			return;


	
		//Always generate segment contacts
		//(2) segment intesection
		for (PxU32 iStart = 0, iEnd = 2; iStart < 3; iEnd = iStart++)
		{
			if((!points1In0Penetration[iStart] && !points1In0Penetration[iEnd] ) )
				continue;
			
			const Vec3V ipA = points1In0[iStart];
			const Vec3V ipB = points1In0[iEnd];

			const Vec3V iMin = V3Min(ipA, ipB);
			const Vec3V iMax = V3Max(ipA, ipB);
		
			for (PxU32 rStart = 0, rEnd = PxU32(referencePolygon.mNbVerts - 1); rStart < referencePolygon.mNbVerts; rEnd = rStart++) 
			{
	
				const Vec3V rpA = points0In0[rStart];
				const Vec3V rpB = points0In0[rEnd];

				const Vec3V rMin = V3Min(rpA, rpB);
				const Vec3V rMax = V3Max(rpA, rpB);
				
				const BoolV tempCon =BOr(V3IsGrtr(iMin, rMax), V3IsGrtr(rMin, iMax));
				const BoolV con = BOr(BGetX(tempCon), BGetY(tempCon));
		
				if(BAllEq(con, bTrue))
					continue;
			
					
				FloatV a1 = signed2DTriArea(rpA, rpB, ipA);
				FloatV a2 = signed2DTriArea(rpA, rpB, ipB);


				if(FAllGrtr(zero, FMul(a1, a2)))
				{
					FloatV a3 = signed2DTriArea(ipA, ipB, rpA);
					FloatV a4 = signed2DTriArea(ipA, ipB, rpB);

					if(FAllGrtr(zero, FMul(a3, a4)))
					{
						
						//these two segment intersect
						const FloatV t = FMul(a1, FRecip(FSub(a2, a1)));

						const Vec3V ipAOri = V3SetZ(points1In0[iStart], FAdd(points1In0TValue[iStart], d));
						const Vec3V ipBOri = V3SetZ(points1In0[iEnd], FAdd(points1In0TValue[iEnd], d));

						const Vec3V pBB = V3NegScaleSub(V3Sub(ipBOri, ipAOri), t, ipAOri); 
						const Vec3V pAA = V3SetZ(pBB, d);
						const Vec3V pA = M33TrnspsMulV3(rot, pAA);
						const Vec3V pB = M33TrnspsMulV3(rot, pBB);
						const FloatV pen = FSub(V3GetZ(pBB), V3GetZ(pAA));
				
						if(FAllGrtr(pen, contactDist))
							continue;
		
					
						const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(nContactNormal), pen);
						manifoldContacts[numContacts].mLocalPointA = pA;
						manifoldContacts[numContacts].mLocalPointB = pB;
						manifoldContacts[numContacts].mLocalNormalPen = localNormalPen;
						manifoldContacts[numContacts++].mFaceIndex = triangleIndex;
					}
				}
			}
		}
			
	}


	bool Gu::PCMConvexVsMeshContactGeneration::generateTriangleFullContactManifold(Gu::TriangleV& localTriangle, const PxU32 triangleIndex, const PxU32* triIndices, const PxU8 triFlags, const Gu::PolygonalData& polyData,  Gu::SupportLocalImpl<Gu::TriangleV>* localTriMap, Gu::SupportLocal* polyMap, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& patchNormal)
	{
	
		using namespace Ps::aos;

		const FloatV threshold = FLoad(0.7071f);//about 45 degree
		PX_UNUSED(threshold);
		{
				
			FeatureStatus status = POLYDATA0;
			FloatV minOverlap = FMax();
			//minNormal will be in the local space of polyData
			Vec3V minNormal = V3Zero();


			PxU32 feature0;
			if(!testTriangleFaceNormal(localTriangle, polyData, localTriMap, polyMap, contactDist, minOverlap, feature0, minNormal, POLYDATA0, status))
				return false;
			
			PxU32 feature1;
			if(!testPolyFaceNormal(localTriangle, polyData, localTriMap, polyMap, contactDist, minOverlap, feature1, minNormal, POLYDATA1, status))
				return false;

	
			const Gu::HullPolygonData& polygon1 = polyData.mPolygons[feature1];
			if(!testPolyEdgeNormal(localTriangle, triFlags, polygon1, polyData, localTriMap, polyMap, contactDist, minOverlap, minNormal, EDGE0, status))
				return false;

			const Vec3V triNormal = localTriangle.normal();

			if(status == POLYDATA0)
			{
				//minNormal is the triangle normal and it is in the local space of polydata0
				const Gu::HullPolygonData& referencePolygon = polyData.mPolygons[getPolygonIndex(polyData, polyMap, minNormal)];

				patchNormal = triNormal;
				generatedTriangleContacts(localTriangle, triangleIndex, triFlags, polyData, referencePolygon, polyMap, manifoldContacts, numContacts, contactDist, triNormal);
			
			}
			else
			{

				if(status == POLYDATA1)
				{
					const Gu::HullPolygonData* referencePolygon = &polyData.mPolygons[feature1];
			
					const Vec3V contactNormal = V3Normalize(M33TrnspsMulV3(polyMap->shape2Vertex, V3LoadU(referencePolygon->mPlane.n)));
					const Vec3V nContactNormal = V3Neg(contactNormal);
					const FloatV cosTheta = V3Dot(nContactNormal, triNormal);

					if(FAllGrtr(cosTheta, threshold))
					{
						patchNormal = triNormal;
						generatedTriangleContacts(localTriangle, triangleIndex, triFlags, polyData, *referencePolygon, polyMap, manifoldContacts, numContacts, contactDist, triNormal);
					}
					else
					{
						//ML : defer the contacts generation
						const PxU32 nb = sizeof(PCMDeferredPolyData)/sizeof(PxU32);
						PCMDeferredPolyData* PX_RESTRICT data = (PCMDeferredPolyData*)mDeferredContacts.Reserve(nb);
						data->mTriangleIndex = triangleIndex;
						data->mFeatureIndex = feature1;
						data->triFlags = triFlags;  
						data->mInds[0] = triIndices[0];
						data->mInds[1] = triIndices[1];
						data->mInds[2] = triIndices[2];
						V3StoreU(localTriangle.verts[0], data->mVerts[0]);
						V3StoreU(localTriangle.verts[1], data->mVerts[1]);
						V3StoreU(localTriangle.verts[2], data->mVerts[2]);
						return true;

					}
				}
				else
				{
					feature1 = (PxU32)getPolygonIndex(polyData, polyMap, minNormal);
					const Gu::HullPolygonData* referencePolygon = &polyData.mPolygons[feature1];
			
					const Vec3V contactNormal = V3Normalize(M33TrnspsMulV3(polyMap->shape2Vertex, V3LoadU(referencePolygon->mPlane.n)));
					const Vec3V nContactNormal = V3Neg(contactNormal);

					//if the minimum sperating axis is edge case, we don't defer it because it is an activeEdge
					patchNormal = nContactNormal;
					generatedPolyContacts(polyData, *referencePolygon, localTriangle, triangleIndex, triFlags, polyMap, manifoldContacts, numContacts, contactDist, contactNormal);

				}


			}

		}

		return true;
	}

	bool Gu::PCMConvexVsMeshContactGeneration::generateTriangleFullContactManifold(Gu::TriangleV& localTriangle,  const PxU32 triangleIndex, const PxU8 triFlags, const Gu::PolygonalData& polyData,  Gu::SupportLocalImpl<Gu::TriangleV>* localTriMap, Gu::SupportLocal* polyMap, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts,
		const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& patchNormal)
	{
	
		using namespace Ps::aos;

		const FloatV threshold = FLoad(0.7071f);//about 45 degree
		PX_UNUSED(threshold);
		{
				
			FeatureStatus status = POLYDATA0;
			FloatV minOverlap = FMax();
			//minNormal will be in the local space of polyData
			Vec3V minNormal = V3Zero();

			PxU32 feature0;
			if(!testTriangleFaceNormal(localTriangle, polyData, localTriMap, polyMap, contactDist, minOverlap, feature0, minNormal, POLYDATA0, status))
				return false;
			
			PxU32 feature1;
			if(!testPolyFaceNormal(localTriangle, polyData, localTriMap, polyMap, contactDist, minOverlap, feature1, minNormal, POLYDATA1, status))
				return false;

	
			const Gu::HullPolygonData& polygon1 = polyData.mPolygons[feature1];
			if(!testPolyEdgeNormal(localTriangle, triFlags, polygon1, polyData, localTriMap, polyMap, contactDist, minOverlap, minNormal, EDGE0, status))
				return false;

			const Vec3V triNormal = localTriangle.normal();
			patchNormal = triNormal;

			const Gu::HullPolygonData* referencePolygon = &polyData.mPolygons[getPolygonIndex(polyData, polyMap, triNormal)];
			generatedTriangleContacts(localTriangle, triangleIndex, triFlags, polyData, *referencePolygon, polyMap, manifoldContacts, numContacts, contactDist, triNormal);
			
		}

		return true;
	}

	bool Gu::PCMConvexVsMeshContactGeneration::generatePolyDataContactManifold(Gu::TriangleV& localTriangle, const PxU32 featureIndex,  const PxU32 triangleIndex, const PxU8 triFlags, Gu::MeshPersistentContact* manifoldContacts, PxU32& numContacts, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& patchNormal)
	{
	
		using namespace Ps::aos;

		const Gu::HullPolygonData* referencePolygon = &mPolyData.mPolygons[featureIndex];
		
		const Vec3V contactNormal = V3Normalize(M33TrnspsMulV3(mPolyMap->shape2Vertex, V3LoadU(referencePolygon->mPlane.n)));
		const Vec3V nContactNormal = V3Neg(contactNormal);
		
		patchNormal = nContactNormal;
		generatedPolyContacts(mPolyData, *referencePolygon, localTriangle, triangleIndex, triFlags, mPolyMap, manifoldContacts, numContacts, contactDist, contactNormal);

		return true;
	}


}//physx
