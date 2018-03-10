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

#ifndef GU_VEC_BOX_H
#define GU_VEC_BOX_H

/** \addtogroup geomutils
@{
*/
#include "PxPhysXCommonConfig.h"
#include "GuVecConvex.h"
#include "PsVecTransform.h"
#include "GuConvexSupportTable.h"
  


namespace physx
{
extern const Ps::aos::BoolV boxVertexTable[8];


namespace Gu
{
	class CapsuleV;


	PX_FORCE_INLINE void CalculateBoxMargin(const Ps::aos::Vec3VArg extent, Ps::aos::FloatV& margin, Ps::aos::FloatV& minMargin)
	{
		using namespace Ps::aos;

		const FloatV marginRatio = Ps::aos::FLoad(0.15f);
		const FloatV minMarginRatio = Ps::aos::FLoad(0.05f);
		const FloatV min = V3ExtractMin(extent);

		margin = FMul(min, marginRatio);
		minMargin = FMul(min, minMarginRatio);
		
	}

	PX_FORCE_INLINE Ps::aos::FloatV CalculateBoxTolerance(const Ps::aos::Vec3VArg extent)
	{

		using namespace Ps::aos;
		
		const FloatV r0 = FLoad(0.01f);
		const FloatV min = V3ExtractMin(extent);//FMin(V3GetX(extent), FMin(V3GetY(extent), V3GetZ(extent)));
		return FMul(min, r0);
	}

	PX_FORCE_INLINE Ps::aos::FloatV CalculatePCMBoxMargin(const Ps::aos::Vec3VArg extent)
	{

		using namespace Ps::aos;
		
		const FloatV r0 = FLoad(0.15f);
		const FloatV min = V3ExtractMin(extent);//FMin(V3GetX(extent), FMin(V3GetY(extent), V3GetZ(extent)));
		return FMul(min, r0);
	
	}


	/**
	\brief Represents an oriented bounding box. 

	As a center point, extents(radii) and a rotation. i.e. the center of the box is at the center point, 
	the box is rotated around this point with the rotation and it is 2*extents in width, height and depth.
	*/

	/**
	Box geometry

	The rot member describes the world space orientation of the box.
	The center member gives the world space position of the box.
	The extents give the local space coordinates of the box corner in the positive octant.
	Dimensions of the box are: 2*extent.
	Transformation to world space is: worldPoint = rot * localPoint + center
	Transformation to local space is: localPoint = T(rot) * (worldPoint - center)
	Where T(M) denotes the transpose of M.
	*/

	class BoxV : public ConvexV
	{
	public:

		/**
		\brief Constructor
		*/
		PX_INLINE BoxV() : ConvexV(E_BOX)
		{
		}

		/**
		\brief Constructor

		\param _center Center of the OBB
		\param _extents Extents/radii of the obb.
		\param _rot rotation to apply to the obb.
		*/


		//! Construct from center, extent and rotation
		PX_FORCE_INLINE BoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent, const Ps::aos::Mat33V& base) : 
																											ConvexV(E_BOX, origin), rot(base), extents(extent)
		{
			CalculateBoxMargin(extent, margin, minMargin);
		}

		PX_FORCE_INLINE BoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent) : 
																				ConvexV(E_BOX, origin), extents(extent)
		{
			CalculateBoxMargin(extent, margin, minMargin);
		}

		PX_FORCE_INLINE BoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent, 
			const Ps::aos::Vec3VArg col0, const Ps::aos::Vec3VArg col1, const Ps::aos::Vec3VArg col2) : 
																									ConvexV(E_BOX, origin), extents(extent)
		{
			CalculateBoxMargin(extent, margin, minMargin);
			rot.col0 = col0;
			rot.col1 = col1;
			rot.col2 = col2;
		}

		PX_FORCE_INLINE BoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent, const Ps::aos::Mat33V& base, const Ps::aos::FloatVArg _margin) : 
																											ConvexV(E_BOX, origin, _margin), rot(base), extents(extent)
		{
		}

		PX_FORCE_INLINE BoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent, const Ps::aos::Mat33V& base, const Ps::aos::FloatVArg _margin, const Ps::aos::FloatVArg _minMargin) : 
																											ConvexV(E_BOX, origin, _margin, _minMargin), rot(base), extents(extent)
		{
		}

		PX_FORCE_INLINE BoxV(const Ps::aos::Vec3VArg origin, const Ps::aos::Vec3VArg extent,const Ps::aos::FloatVArg _margin, const Ps::aos::FloatVArg _minMargin) : 
																											ConvexV(E_BOX, origin, _margin, _minMargin), extents(extent)
		{
		}
																				
		//! construct from a matrix(center and rotation) + extent
		PX_FORCE_INLINE BoxV(const Ps::aos::Mat34V& mat, const Ps::aos::Vec3VArg extent) : ConvexV(E_BOX, mat.col3), rot(Ps::aos::Mat33V(mat.col0, mat.col1, mat.col2)), extents(extent)
		{
		}

		//! Copy constructor
		PX_FORCE_INLINE BoxV(const BoxV& other) : ConvexV(E_BOX, other.center, other.margin), rot(other.rot), extents(other.extents)
		{
		}

		
		/**
		\brief Destructor
		*/
		PX_INLINE ~BoxV()
		{
		}

		//! Assignment operator
		PX_INLINE const BoxV& operator=(const BoxV& other)
		{
			rot		= other.rot;
			center	= other.center;
			extents	= other.extents;
			margin =  other.margin;
			minMargin = other.minMargin;
			return *this;
		}

		/**
		\brief Setups an empty box.
		*/
		PX_INLINE void setEmpty()
		{
			using namespace Ps::aos;
			center = V3Zero();
			extents = Vec3V_From_FloatV(FLoad(-PX_MAX_REAL));
			rot = M33Identity();
			margin = minMargin = FZero();

		}

		/**
		\brief Checks the box is valid.

		\return	true if the box is valid
		*/
		PX_INLINE bool isValid() const
		{
			// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
			using namespace Ps::aos;
			const Vec3V zero = V3Zero();
			return BAllEq(V3IsGrtrOrEq(extents, zero), BTTTT()) == 1;
		}

/////////////
		PX_FORCE_INLINE	void	setAxes(const Ps::aos::Vec3VArg axis0, const Ps::aos::Vec3VArg axis1, const Ps::aos::Vec3VArg axis2)
		{
			rot.col0 = axis0;
			rot.col1 = axis1;
			rot.col2 = axis2;
		}


		
		PX_INLINE	Ps::aos::Vec3V	rotate(const Ps::aos::Vec3VArg src)	const
		{
			return Ps::aos::M33MulV3(rot, src);
		}

		PX_INLINE	Ps::aos::Vec3V	rotateInv(const Ps::aos::Vec3VArg src)	const
		{
			return Ps::aos::M33TrnspsMulV3(rot, src);
		}

		//get the world space point from the local space
		PX_INLINE	Ps::aos::Vec3V	transformFromLocalToWorld(const Ps::aos::Vec3VArg src)	const
		{
			return Ps::aos::V3Add(Ps::aos::M33MulV3(rot, src), center);
		}

		PX_INLINE	Ps::aos::Vec3V	transformFromWorldToLocal(const Ps::aos::Vec3VArg src)	const
		{
			return Ps::aos::M33TrnspsMulV3(rot, Ps::aos::V3Sub(src, center));
		}

		PX_INLINE Ps::aos::Vec3V computeAABBExtent() const
		{
			using namespace Ps::aos;
			return M33TrnspsMulV3(rot, extents);
		}

		void computeOBBPoints(Ps::aos::Vec3V* PX_RESTRICT pts) const
		{
			using namespace Ps::aos;
			//     7+------+6			0 = ---
			//     /|     /|			1 = +--
			//    / |    / |			2 = ++-
			//   / 4+---/--+5			3 = -+-
			// 3+------+2 /    y   z	4 = --+
			//  | /    | /     |  /		5 = +-+
			//  |/     |/      |/		6 = +++
			// 0+------+1      *---x	7 = -++

			const Vec3V nExtents = V3Neg(extents);
			const FloatV x = V3GetX(extents);
			const FloatV y = V3GetY(extents);
			const FloatV z = V3GetZ(extents);
			const FloatV nx = V3GetX(nExtents);
			const FloatV ny = V3GetY(nExtents);
			const FloatV nz = V3GetZ(nExtents);
		
			pts[0] = nExtents;
			pts[1] = V3Merge(x, ny, nz);  
			pts[2] = V3Merge(x,  y, nz);
			pts[3] = V3Merge(nx, y, nz);
			pts[4] = V3Merge(nx, ny, z);
			pts[5] = V3Merge(x,  ny, z);
			pts[6] = extents;
			pts[7] = V3Merge(nx,  y, z);

		}

		void computeOBBPoints(Ps::aos::Vec3V* PX_RESTRICT pts, const Ps::aos::Vec3VArg _center, const Ps::aos::Vec3VArg _extents, const Ps::aos::Vec3VArg base0, const Ps::aos::Vec3VArg base1, const Ps::aos::Vec3VArg base2) const
		{
			using namespace Ps::aos;

			PX_ASSERT(pts);

			// "Rotated extents"
			const Vec3V axis0 = V3Scale(base0,  V3GetX(_extents));
			const Vec3V axis1 = V3Scale(base1,  V3GetY(_extents));
			const Vec3V axis2 = V3Scale(base2,  V3GetZ(_extents));

			//     7+------+6			0 = ---
			//     /|     /|			1 = +--
			//    / |    / |			2 = ++-
			//   / 4+---/--+5			3 = -+-
			// 3+------+2 /    y   z	4 = --+
			//  | /    | /     |  /		5 = +-+
			//  |/     |/      |/		6 = +++
			// 0+------+1      *---x	7 = -++


			const Vec3V a0 = V3Sub(_center, axis0);
			const Vec3V a1 = V3Add(_center, axis0);
			const Vec3V a2 = V3Add(axis1,  axis2);
			const Vec3V a3 = V3Sub(axis1,  axis2);

			pts[0] = V3Sub(a0, a2);
			pts[1] = V3Sub(a1, a2);

			pts[2] = V3Add(a1, a3);
			pts[3] = V3Add(a0, a3);

			pts[4] = V3Sub(a0, a3);
			pts[5] = V3Sub(a1, a3);
			
			pts[6] = V3Add(a1, a2);
			pts[7] = V3Add(a0, a2);
		}

		/**
		\brief recomputes the OBB after an arbitrary transform by a 4x4 matrix.
		\param	mtx		[in] the transform matrix
		\param	obb		[out] the transformed OBB
		*/
		PX_INLINE	void rotate(const Ps::aos::Mat34V& mtx, BoxV& obb)	const
		{
			using namespace Ps::aos;
			// The extents remain constant
			obb.extents = extents;
			// The center gets x-formed
			obb.center =M34MulV3(mtx, obb.center);
			// Combine rotations
			const Mat33V mtxR = Mat33V(mtx.col0, mtx.col1, mtx.col2);
			obb.rot =M33MulM33(mtxR, rot);
		}

		void create(const Gu::CapsuleV& capsule);

		/**
		\brief checks the OBB is inside another OBB.
		\param		box		[in] the other OBB
		*/
		PxU32 isInside(const BoxV& box)	const; 

	
		PX_FORCE_INLINE void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts)const
		{
			using namespace Ps::aos;

			for(PxU32 i=0; i<numInds; ++i)
			{
				verts[i] = V3LoadU(originalVerts[inds[i]]);
			}
		}


		PX_FORCE_INLINE Ps::aos::Vec3V supportPoint(const PxI32 index)const
		{
			using namespace Ps::aos;
			const BoolV con = boxVertexTable[index];
			return V3Sel(con, extents, V3Neg(extents));
		}  

		PX_FORCE_INLINE void getIndex(const Ps::aos::BoolV con, PxI32& index)const 
		{ 
			using namespace Ps::aos;
			index = (PxI32)BGetBitMask(con) & 0x7; 
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportSweepRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aToB)const  
		{
			return supportRelative(dir, aToB);	
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportSweepLocal(const Ps::aos::Vec3VArg dir)const  
		{
			return supportLocal(dir);	
		}

		PX_FORCE_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir)const  
		{
			using namespace Ps::aos;
			return V3Sel(V3IsGrtr(dir,  V3Zero()), extents, V3Neg(extents));
		}

		//this is used in the sat test for the full contact gen
		PX_SUPPORT_INLINE void supportLocal(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max)const
		{
			using namespace Ps::aos;
			const Vec3V point = V3Sel(V3IsGrtr(dir,  V3Zero()), extents, V3Neg(extents));
			max = V3Dot(dir, point);
			min = FNeg(max);
		}


		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob)const  
		{

			//a is the current object, b is the other object, dir is in the local space of b
			using namespace Ps::aos;
			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);//from b to a
			const Vec3V p =  supportLocal(_dir);
			//transfer p into the b space
			return aTob.transform(p);//relTra.transform(p);
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir, PxI32& index)const  
		{
			using namespace Ps::aos;
			const BoolV comp = V3IsGrtr(dir, V3Zero());
			getIndex(comp, index);
			return  V3Sel(comp, extents, V3Neg(extents));
		}

		

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, PxI32& index)const  
		{

			//a is the current object, b is the other object, dir is in the local space of b
			using namespace Ps::aos;
			const Vec3V _dir = aTob.rotateInv(dir);//relTra.rotateInv(dir);//from b to a
			const Vec3V p = supportLocal(_dir, index);
			//transfer p into the b space
			return aTob.transform(p);//relTra.transform(p);
		}


		//local space point
		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(const Ps::aos::Vec3VArg dir,  Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
		
			const Vec3V zero = V3Zero();
			const BoolV comp = V3IsGrtr(dir, zero);
			getIndex(comp, index);
			const Vec3V p = V3Sel(comp, extents, V3Neg(extents));
			support = p;
			return p;
		}

		PX_FORCE_INLINE Ps::aos::BoolV supportLocalIndex(const Ps::aos::Vec3VArg dir, PxI32& index)const
		{
			using namespace Ps::aos;
			const Vec3V zero = V3Zero();
			const BoolV comp = V3IsGrtr(dir, zero);
			getIndex(comp, index);
			return comp;
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportLocal(Ps::aos::Vec3V& support, const PxI32& index, const Ps::aos::BoolV comp)const
		{
			PX_UNUSED(index);

			using namespace Ps::aos;
			//const Vec3V _extents = V3Sub(extents,  V3Splat(margin));
			const Vec3V p = V3Sel(comp, extents, V3Neg(extents));
			support = p;
			return p;
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, Ps::aos::Vec3V& support, PxI32& index)const
		{
			using namespace Ps::aos;
		
			const Vec3V zero = V3Zero();
			const Vec3V _dir =aTob.rotateInv(dir);//relTra.rotateInv(dir);
			const BoolV comp = V3IsGrtr(_dir, zero);
			const Vec3V p = V3Sel(comp, extents, V3Neg(extents));
			getIndex(comp, index);
			//transfer p into the world space
			const Vec3V ret = aTob.transform(p);//relTra.transform(p);//V3Add(center, M33MulV3(rot, p));
			support = ret;
			return ret;
		}

		PX_FORCE_INLINE Ps::aos::BoolV supportRelativeIndex(const Ps::aos::Vec3VArg dir, const Ps::aos::PsMatTransformV& aTob, PxI32& index)const
		{
			using namespace Ps::aos;
			const Vec3V zero = V3Zero();
			//scale dir and put it in the vertex space
			const Vec3V _dir =aTob.rotateInv(dir);//relTra.rotateInv(dir);
			const BoolV comp = V3IsGrtr(_dir, zero);
			getIndex(comp, index);
			return comp;
		}

		PX_SUPPORT_INLINE Ps::aos::Vec3V supportRelative(Ps::aos::Vec3V& support, const Ps::aos::PsMatTransformV& aTob, const PxI32& index, const Ps::aos::BoolV comp)const
		{
			PX_UNUSED(index);

			using namespace Ps::aos;

			const Vec3V p = V3Sel(comp, extents, V3Neg(extents));
			//transfer p into the world space
			const Vec3V ret = aTob.transform(p);
			support = ret;
			return ret;
		}
	
		Ps::aos::Mat33V rot;
		Ps::aos::Vec3V extents;
	};
}	//PX_COMPILE_TIME_ASSERT(sizeof(Gu::BoxV) == 96);

}

/** @} */
#endif
