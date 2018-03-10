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

#ifndef GU_VEC_CONVEXHULL_H
#define GU_VEC_CONVEXHULL_H

#include "PxPhysXCommonConfig.h"
#include "GuVecConvex.h"
#include "GuConvexMeshData.h"
#include "GuBigConvexData.h"
#include "GuConvexSupportTable.h"
#include "GuCubeIndex.h"
#include "PsFPU.h"


namespace physx
{
namespace Gu
{

	//This margin is used in Persistent contact manifold
	PX_SUPPORT_FORCE_INLINE Ps::aos::FloatV CalculatePCMConvexMargin(const Gu::ConvexHullData* hullData, const Ps::aos::Vec3VArg scale)
	{
		using namespace Ps::aos;
		const Vec3V extents= V3Mul(V3LoadU(hullData->mInternal.mExtents), scale);
		const FloatV min = V3ExtractMin(extents);
		//ML: 25% of the minimum extents of the internal AABB as this convex hull's margin
		return FMul(min, FLoad(0.25f));

		/*using namespace Ps::aos;
		const FloatV _extents = FloatV_From_F32(hullData->mInternal.mRadius);
		const FloatV extents = FMul(_extents, scale);
		const FloatV perc = FloatV_From_F32(0.15f);
		return FMul(extents, perc);*/

		
	}
	
	PX_SUPPORT_FORCE_INLINE Ps::aos::FloatV CalculateConvexTolerance(const Gu::ConvexHullData* hullData, const Ps::aos::Vec3VArg scale)
	{

		using namespace Ps::aos;
		const Vec3V _extents =  V3LoadU(hullData->mInternal.mExtents);
		const Vec3V extents = V3Mul(_extents, scale);
		const FloatV ratio1 = Ps::aos::FLoad(0.01f);
		const FloatV min = V3ExtractMin(extents);
		return FMul(min, ratio1);
	}


	//This margin is used in PCM contact gen
	PX_SUPPORT_FORCE_INLINE void CalculateConvexMargin(const Gu::ConvexHullData* hullData, Ps::aos::FloatV& margin, Ps::aos::FloatV& minMargin, const Ps::aos::Vec3VArg scale)
	{

		using namespace Ps::aos;
		const Vec3V _extents = V3LoadU(hullData->mInternal.mExtents);
		const FloatV marginRatio = Ps::aos::FLoad(0.2f);
		const FloatV minMarginRatio = Ps::aos::FLoad(0.1f);

		const Vec3V extents = V3Mul(_extents, scale);
		const FloatV min = V3ExtractMin(extents);

		//Margin is used in the plane shifting for the shrunk convex hull
		margin = FMul(min, marginRatio);
		//minMargin is used in the GJK termination condition
		minMargin = FMul(min, minMarginRatio);

	}



	PX_SUPPORT_FORCE_INLINE Ps::aos::Mat33V ConstructSkewMatrix(const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg rotation) 
	{
		using namespace Ps::aos;
		const Mat33V rot = QuatGetMat33V(rotation);
		Mat33V trans = M33Trnsps(rot);
		trans.col0 = V3Scale(trans.col0, V3GetX(scale));
		trans.col1 = V3Scale(trans.col1, V3GetY(scale));
		trans.col2 = V3Scale(trans.col2, V3GetZ(scale));
		return M33MulM33(trans, rot);
	}

	PX_SUPPORT_FORCE_INLINE void ConstructSkewMatrix(const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg rotation, Ps::aos::Mat33V& vertex2Shape, Ps::aos::Mat33V& shape2Vertex) 
	{
		// V3Recip is unguarded, can throw
		PX_SIMD_GUARD;

		using namespace Ps::aos;
		const Mat33V rot = QuatGetMat33V(rotation);
		const Mat33V trans = M33Trnsps(rot);
		/*
			vertex2shape
			skewMat = Inv(R)*Diagonal(scale)*R;
		*/

		const Mat33V temp(V3Scale(trans.col0, V3GetX(scale)), V3Scale(trans.col1, V3GetY(scale)), V3Scale(trans.col2, V3GetZ(scale)));
		vertex2Shape = M33MulM33(temp, rot);

		//don't need it in the support function
		/*
			shape2Vertex
			invSkewMat =(invSkewMat)= Inv(R)*Diagonal(1/scale)*R;
		*/
		const Vec3V invScale = V3Recip(scale);
		shape2Vertex.col0 = V3Scale(trans.col0, V3GetX(invScale));
		shape2Vertex.col1 = V3Scale(trans.col1, V3GetY(invScale));
		shape2Vertex.col2 = V3Scale(trans.col2, V3GetZ(invScale));
		shape2Vertex = M33MulM33(shape2Vertex, rot);

		//shape2Vertex = M33Inverse(vertex2Shape);

	}

	PX_SUPPORT_FORCE_INLINE Ps::aos::Mat33V ConstructVertex2ShapeMatrix(const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg rotation) 
	{
		using namespace Ps::aos;
		const Mat33V rot = QuatGetMat33V(rotation);
		const Mat33V trans = M33Trnsps(rot);
		/*
			vertex2shape
			skewMat = Inv(R)*Diagonal(scale)*R;
		*/

		const Mat33V temp(V3Scale(trans.col0, V3GetX(scale)), V3Scale(trans.col1, V3GetY(scale)), V3Scale(trans.col2, V3GetZ(scale)));
		return M33MulM33(temp, rot);

	}


	class ConvexHullV : public ConvexV
	{

		class TinyBitMap
		{
		public:
			PxU32 m[8];
			PX_FORCE_INLINE TinyBitMap()			{ m[0] = m[1] = m[2] = m[3] = m[4] = m[5] = m[6] = m[7] = 0;	}
			PX_FORCE_INLINE void set(PxU8 v)		{ m[v>>5] |= 1<<(v&31);											}
			PX_FORCE_INLINE bool get(PxU8 v) const	{ return (m[v>>5] & 1<<(v&31)) != 0;							}
		};


		public:
		/**
		\brief Constructor
		*/
		PX_SUPPORT_INLINE ConvexHullV(): ConvexV(E_CONVEXHULL)
		{
		}

		PX_SUPPORT_INLINE ConvexHullV(const Gu::ConvexHullData* _hullData, const Ps::aos::Vec3VArg _center, const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg scaleRot):
													ConvexV(E_CONVEXHULL, _center)
		{
			using namespace Ps::aos;

			hullData = _hullData;	
			const PxVec3* __restrict tempVerts = _hullData->getHullVertices();
			//const PxU8* __restrict polyInds = _hullData->getFacesByVertices8();
			//const HullPolygonData* __restrict polygons = _hullData->mPolygons;
			verts = tempVerts;
			numVerts = _hullData->mNbHullVertices;
			CalculateConvexMargin( _hullData, margin, minMargin, scale);
			ConstructSkewMatrix(scale, scaleRot, vertex2Shape, shape2Vertex);
			/*skewScale = Mat33V temp(V3Scale(trans.col0, V3GetX(scale)), V3Scale(trans.col1, V3GetY(scale)), V3Scale(trans.col2, V3GetZ(scale)));
			skewRot = QuatGetMat33V(scaleRot);*/

		//	searchIndex = 0;
			data = _hullData->mBigConvexRawData;

			PxU8* startAddress = (PxU8*)_hullData->mPolygons;
			PxI32 totalPrefetchBytes = PxI32((_hullData->getFacesByVertices8() + _hullData->mNbHullVertices * 3) - startAddress);

			//Prefetch core data
			
			while(totalPrefetchBytes > 0)
			{
				totalPrefetchBytes -= 128;
				Ps::prefetchLine(startAddress);
				startAddress += 128;
			}

			if(data)
			{
				PxI32 totalSize = PxI32(data->mNbSamples + data->mNbVerts * sizeof(Gu::Valency) + data->mNbAdjVerts);
				startAddress = data->mSamples;
				while(totalSize > 0)
				{
					totalSize -= 128;
					Ps::prefetchLine(startAddress);
					startAddress += 128;
				}
			}
			
		}


		PX_SUPPORT_INLINE ConvexHullV(const Gu::ConvexHullData* _hullData, const Ps::aos::Vec3VArg _center, const Ps::aos::FloatVArg _margin, const Ps::aos::FloatVArg _minMargin, const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg scaleRot) : 
													ConvexV(E_CONVEXHULL, _center)
		{
			using namespace Ps::aos;

			hullData = _hullData;
			margin = _margin;
			minMargin = _minMargin;
	
			const PxVec3* tempVerts = _hullData->getHullVertices();
			const PxU8* __restrict polyInds = _hullData->getFacesByVertices8();
			const HullPolygonData* __restrict polygons = _hullData->mPolygons;
			verts = tempVerts;
			numVerts = _hullData->mNbHullVertices;

			ConstructSkewMatrix(scale, scaleRot, vertex2Shape, shape2Vertex);

			Ps::prefetchLine(tempVerts);
			Ps::prefetchLine(tempVerts,128);
			Ps::prefetchLine(tempVerts,256);

			Ps::prefetchLine(polyInds);
			Ps::prefetchLine(polyInds,128);

			Ps::prefetchLine(polygons);
			Ps::prefetchLine(polygons, 128);
			Ps::prefetchLine(polygons, 256);
		}


		PX_SUPPORT_INLINE void initialize(const Gu::ConvexHullData* _hullData, const Ps::aos::Vec3VArg _center, /*const Ps::aos::Mat33V& _rot,*/ const Ps::aos::Vec3VArg scale, const Ps::aos::QuatVArg scaleRot)
		{	
			using namespace Ps::aos;
			
			const PxVec3* tempVerts = _hullData->getHullVertices();
			CalculateConvexMargin(_hullData, margin, minMargin, scale);
			ConstructSkewMatrix(scale, scaleRot, vertex2Shape, shape2Vertex);

			verts = tempVerts;
			numVerts = _hullData->mNbHullVertices;
			//rot = _rot;	
			
			center = _center;

		//	searchIndex = 0;
			data = _hullData->mBigConvexRawData;

			hullData = _hullData;
			if(_hullData->mBigConvexRawData)
			{
				Ps::prefetchLine(hullData->mBigConvexRawData->mValencies);
				Ps::prefetchLine(hullData->mBigConvexRawData->mValencies,128);
				Ps::prefetchLine(hullData->mBigConvexRawData->mAdjacentVerts);
			}
		}


		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index)const
		{
			using namespace Ps::aos;
			//return planeShift(index, margin);
			return M33MulV3(vertex2Shape, V3LoadU(verts[index]));
		}


		PX_SUPPORT_INLINE PxU32 HillClimbing(const Ps::aos::Vec3VArg _dir)const
		{
			using namespace Ps::aos;

			const Gu::Valency* valency = data->mValencies;
			const PxU8* adjacentVerts = data->mAdjacentVerts;
			
			//NotSoTinyBitMap visited;
			PxU32 smallBitMap[8] = {0,0,0,0,0,0,0,0};

		//	PxU32 index = searchIndex;
			PxU32 index = 0;

			{
				PxVec3 vertexSpaceDirection;
				V3StoreU(_dir, vertexSpaceDirection);
				const PxU32 offset = ComputeCubemapNearestOffset(vertexSpaceDirection, data->mSubdiv);
				//const PxU32 offset = ComputeCubemapOffset(vertexSpaceDirection, data->mSubdiv);
				index = data->mSamples[offset];
			}

			Vec3V maxPoint = V3LoadU(verts[index]);
			FloatV max = V3Dot(maxPoint, _dir);
	
			PxU32 initialIndex = index;

			
			do
			{
				initialIndex = index;
				const PxU32 numNeighbours = valency[index].mCount;
				const PxU32 offset = valency[index].mOffset;

				for(PxU32 a = 0; a < numNeighbours; ++a)
				{
					const PxU32 neighbourIndex = adjacentVerts[offset + a];

					const Vec3V vertex = V3LoadU(verts[neighbourIndex]);
					const FloatV dist = V3Dot(vertex, _dir);
					if(FAllGrtr(dist, max))
					{
						const PxU32 ind = neighbourIndex>>5;
						const PxU32 mask = PxU32(1 << (neighbourIndex & 31));
						if((smallBitMap[ind] & mask) == 0)
						{
							smallBitMap[ind] |= mask;
							max = dist;
							index = neighbourIndex;
						}
					}
				}

			}while(index != initialIndex);

			return index;
		}

		

#ifndef PX_USE_BRANCHLESS_SUPPORT

		PX_SUPPORT_INLINE PxU32 BruteForceSearch(const Ps::aos::Vec3VArg _dir)const 
		{
			using namespace Ps::aos;
			//brute force
			//get the support point from the orignal margin
			FloatV max = V3Dot(V3LoadU(verts[0]), _dir);
			PxU32 maxIndex=0;

			for(PxU32 i = 1; i < numVerts; ++i)
			{
				Ps::prefetchLine(&verts[i], 128);
				const Vec3V vertex = V3LoadU(verts[i]);
				const FloatV dist = V3Dot(vertex, _dir);
				if(FAllGrtr(dist, max))
				{
					max = dist;
					maxIndex = i;
				}
			}

			return maxIndex;
		}

	
		//points are in vertex space, _dir in vertex space
		PX_SUPPORT_INLINE PxU32 supportVertexIndex(const Ps::aos::Vec3VArg _dir)const
		{
			using namespace Ps::aos;
			if(data)
			{
				return HillClimbing(_dir);
			}
			else
			{
				return BruteForceSearch(_dir);
			}
		}


#else

		PX_SUPPORT_INLINE PxU32 BruteForceSearch(const Ps::aos::Vec3VArg _dir)const 
		{
			using namespace Ps::aos;
			FloatV max = V3Dot(V3LoadU(verts[0]), _dir);

			const VecI32V vIOne = VecI32V_One();
			const VecI32V vZero = VecI32V_Zero();
			VecI32V index = vZero;
			VecI32V bestInd = vZero;

			const PxU32 numIterations = PxU32((numVerts-1)/4);
			
			PxU32 i = 1;
			for(PxU32 a = 0; a < numIterations; ++a)
			{
				Ps::prefetchLine(&verts[i+4]);
				const Vec3V p0 = V3LoadU(verts[i++]);
				const Vec3V p1 = V3LoadU(verts[i++]);
				const Vec3V p2 = V3LoadU(verts[i++]);
				const Vec3V p3 = V3LoadU(verts[i++]);

				const FloatV d0 = V3Dot(p0, _dir);
				const FloatV d1 = V3Dot(p1, _dir);
				const FloatV d2 = V3Dot(p2, _dir);
				const FloatV d3 = V3Dot(p3, _dir);

				const BoolV con0 = FIsGrtr(d0, max);
				index = VecI32V_Add(index, vIOne);
				max = FSel(con0, d0, max);
				bestInd = VecI32V_Sel(con0, index, bestInd);

				const BoolV con1 = FIsGrtr(d1, max);
				index = VecI32V_Add(index, vIOne);
				max = FSel(con1, d1, max);
				bestInd = VecI32V_Sel(con1, index, bestInd);

				const BoolV con2 = FIsGrtr(d2, max);
				index = VecI32V_Add(index, vIOne);
				max = FSel(con2, d2, max);
				bestInd = VecI32V_Sel(con2, index, bestInd);

				const BoolV con3 = FIsGrtr(d3, max);
				index = VecI32V_Add(index, vIOne);
				max = FSel(con3, d3, max);
				bestInd = VecI32V_Sel(con3, index, bestInd);
			}
			for(; i < numVerts; ++i)
			{
				index = VecI32V_Add(index, vIOne);
				const Vec3V vertex = V3LoadU(verts[i]);
				const FloatV dist = V3Dot(vertex, _dir);

				const BoolV isGrtr = FIsGrtr(dist, max);
				max = FSel(isGrtr, dist, max);
				bestInd = VecI32V_Sel(isGrtr, index, bestInd);
			}
	
			PxI32 maxIndex;
			PxI32_From_VecI32V(bestInd, &maxIndex);
			return (PxU32)maxIndex;
		}

		//points are in vertex space
		PX_SUPPORT_INLINE PxU32 supportVertexIndex(const Ps::aos::Vec3VArg _dir)const
		{
			using namespace Ps::aos;

			if(data)
			{
				return HillClimbing(_dir);
			}
			else
			{
				return BruteForceSearch(_dir);
			}
		}

#endif

		//_dir is in the shape space
		PX_SUPPORT_INLINE void BruteForceSearchMinMax(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const 
		{
			using namespace Ps::aos;
			//brute force
			//get the support point from the orignal margin
			FloatV _max = V3Dot( M33MulV3(vertex2Shape, V3LoadU(verts[0])), dir);
			FloatV _min = _max;

			for(PxU32 i = 1; i < numVerts; ++i)
			{ 
				Ps::prefetchLine(&verts[i], 128);
				const Vec3V vertex = M33MulV3(vertex2Shape, V3LoadU(verts[i]));
				const FloatV dist = V3Dot(vertex, dir);
				_max = FMax(dist, _max);
				_min = FMin(dist, _min);
			}

			min = _min;
			max = _max;
		}

	
		//This function is used in the full contact manifold generation code, points are in vertex space.
		//This function support scaling, _dir is in the vertex space	
		PX_SUPPORT_INLINE void supportVertexMinMax(const Ps::aos::Vec3VArg _dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const
		{
			using namespace Ps::aos;


			if(data)
			{
				//scale dir and put it in the vertex space
				const Vec3V dir = M33TrnspsMulV3(vertex2Shape, _dir);
				const PxU32 maxIndex= HillClimbing(dir);
				const PxU32 minIndex= HillClimbing(V3Neg(dir));
				const Vec3V maxPoint= M33MulV3(vertex2Shape, V3LoadU(verts[maxIndex]));
				const Vec3V minPoint= M33MulV3(vertex2Shape, V3LoadU(verts[minIndex]));
				min = V3Dot(_dir, minPoint);
				max = V3Dot(_dir, maxPoint);
			}
			else
			{
				//_dir is in the shape space
				BruteForceSearchMinMax(_dir, min, max);
			}
		}  
 

		//This function is used in the full contact manifold generation code
		PX_SUPPORT_INLINE void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* _verts)const
		{
			using namespace Ps::aos;

			for(PxU32 i=0; i<numInds; ++i)
			{
				_verts[i] = M33MulV3(vertex2Shape, V3LoadU(originalVerts[inds[i]]));
			}
		}


		//This function is used in epa
		//dir is in the shape space
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space
			const Vec3V _dir = M33TrnspsMulV3(vertex2Shape, dir);
			const PxU32 maxIndex = supportVertexIndex(_dir);
			return M33MulV3(vertex2Shape, V3LoadU(verts[maxIndex]));
		}

		//this is used in the sat test for the full contact gen
		PX_SUPPORT_INLINE void supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const
		{
			using namespace Ps::aos;
			//dir is in the shape space
			supportVertexMinMax(dir, min, max);
		}

		//This function is used in epa
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob)const
		{
			using namespace Ps::aos;
		
			//transform dir into the shape space
			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);
			const Vec3V maxPoint =supportLocal(_dir);
			//translate maxPoint from shape space of a back to the b space
			return aTob.transform(maxPoint);//relTra.transform(maxPoint);
		}


		PX_SUPPORT_INLINE Ps::aos::Vec3V supportSweepLocal(const Ps::aos::Vec3VArg dir)const
		{
			return supportLocal(dir);
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportSweepRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB)const
		{
			return supportRelative(dir, aToB);
		}

		//dir in the shape space, this function is used in gjk	
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
			//scale dir and put it in the vertex space, for non-uniform scale, we don't want the scale in the dir, therefore, we are using
			//the transpose of the inverse of shape2Vertex(which is vertex2shape). This will allow us igore the scale and keep the rotation
			const Vec3V _dir = M33TrnspsMulV3(vertex2Shape, dir);
			//get the extreme point index
			const PxU32 maxIndex = supportVertexIndex(_dir);
			index = (PxI32)maxIndex;
			//p is in the shape space
			const Vec3V p = M33MulV3(vertex2Shape, V3LoadU(verts[index]));
			support = p;
			return p;
		}

	
		//this function is used in gjk	
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;

			//transform dir from b space to the shape space of a space
			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);//M33MulV3(skewInvRot, dir);
			const Vec3V p = supportLocal(_dir, support, index);
			//transfrom from a to b space
			const Vec3V ret = aTob.transform(p);
			support = ret;
			return ret;
		}



		PX_SUPPORT_INLINE	bool contain(const Ps::aos::Vec3VArg p)const
		{
			using namespace Ps::aos;
			PxVec3 point;
			V3StoreU(p, point);
			PxU32 Nb = hullData->mNbPolygons;
			const Gu::HullPolygonData* Polygons = hullData->mPolygons;
			while(Nb--)
			{
				const PxPlane& pl = Polygons->mPlane;
				if(pl.distance(point) > 0.0f)	return false;
				Polygons++;
			}
			return true;

		}

		Ps::aos::Mat33V vertex2Shape;//inv(R)*S*R
		Ps::aos::Mat33V shape2Vertex;//inv(vertex2Shape)

		const Gu::ConvexHullData* hullData;
		const BigConvexRawData* data;  
		const PxVec3* verts;
		PxU8 numVerts;

	};


}

}

#endif	// 
