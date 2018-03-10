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

#ifndef GU_BOX_CONVERSION_H
#define GU_BOX_CONVERSION_H

#include "GuBox.h"
#include "PsMathUtils.h"
#include "CmMatrix34.h"

namespace physx
{
	// PT: TODO: get rid of this
	PX_FORCE_INLINE void buildFrom(Gu::Box& dst, const PxVec3& center, const PxVec3& extents, const PxQuat& q)
	{
		dst.center	= center;
		dst.extents	= extents;
		dst.rot		= PxMat33(q);
	}

	PX_FORCE_INLINE void buildMatrixFromBox(Cm::Matrix34& mat34, const Gu::Box& box)
	{
		mat34.base0	= box.rot.column0;
		mat34.base1	= box.rot.column1;
		mat34.base2	= box.rot.column2;
		mat34.base3	= box.center;
	}

	// SD: function is now the same as FastVertex2ShapeScaling::transformQueryBounds
	// PT: lots of LHS in that one. TODO: revisit...
	PX_INLINE Gu::Box transform(const Cm::Matrix34& transfo, const Gu::Box& box)
	{
		Gu::Box ret;
		PxMat33& obbBasis = ret.rot;

		obbBasis.column0 = transfo.rotate(box.rot.column0 * box.extents.x);
		obbBasis.column1 = transfo.rotate(box.rot.column1 * box.extents.y);
		obbBasis.column2 = transfo.rotate(box.rot.column2 * box.extents.z);

		ret.center = transfo.transform(box.center);
		ret.extents = Ps::optimizeBoundingBox(obbBasis);
		return ret;
	}

	PX_INLINE Gu::Box transformBoxOrthonormal(const Gu::Box& box, const PxTransform& t)
	{
		Gu::Box ret;
		PxMat33& obbBasis = ret.rot;
		obbBasis.column0 = t.rotate(box.rot.column0);
		obbBasis.column1 = t.rotate(box.rot.column1);
		obbBasis.column2 = t.rotate(box.rot.column2);
		ret.center = t.transform(box.center);
		ret.extents = box.extents;
		return ret;
	}

	/**
	\brief recomputes the OBB after an arbitrary transform by a 4x4 matrix.
	\param	mtx		[in] the transform matrix
	\param	obb		[out] the transformed OBB
	*/
	PX_INLINE	void rotate(const Gu::Box& src, const Cm::Matrix34& mtx, Gu::Box& obb)
	{
		// The extents remain constant
		obb.extents = src.extents;
		// The center gets x-formed
		obb.center = mtx.transform(src.center);
		// Combine rotations
		obb.rot = PxMat33(mtx.base0, mtx.base1, mtx.base2) * src.rot;
	}

// PT: TODO: move this to a better place
	PX_FORCE_INLINE void getInverse(PxMat33& dstRot, PxVec3& dstTrans, const PxMat33& srcRot, const PxVec3& srcTrans)
	{
		const PxMat33 invRot = srcRot.getInverse();
		dstTrans = invRot.transform(-srcTrans);
		dstRot = invRot;
	}

}

#endif
