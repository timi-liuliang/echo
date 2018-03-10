/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2002-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2006 NovodeX. All rights reserved.   

#ifndef PX_FOUNDATION_PXMAT34LEGACY_H
#define PX_FOUNDATION_PXMAT34LEGACY_H

/** \addtogroup foundation
@{
*/

#include "foundation/PxBounds3.h"
#include "PxMat33Legacy.h"
#include "foundation/PxMat44.h"

namespace physx
{
namespace general_shared3
{

PX_PUSH_PACK_DEFAULT

/**
\brief Combination of a 3x3 rotation matrix and a translation vector.

homogenous transform class composed of a matrix and a vector.
*/

PX_DEPRECATED class PxMat34Legacy
{
public:
	/**
	\brief [ M t ]
	*/
	PxMat33Legacy M;
	PxVec3 t;

	/**
	\brief by default M is inited and t isn't.  Use this ctor to either init or not init in full.
	*/
	PX_CUDA_CALLABLE PX_INLINE explicit PxMat34Legacy(bool init = true);

	PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy(const PxMat33Legacy& rot, const PxVec3& trans) 
		: M(rot), t(trans)
	{}

	PX_CUDA_CALLABLE PX_INLINE explicit PxMat34Legacy(const PxTransform& pose)
		: M(pose.q), t(pose.p)
	{}

	PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy(const PxMat44& mat44)
	{
		M(0, 0) = mat44(0, 0);
		M(1, 0) = mat44(1, 0);
		M(2, 0) = mat44(2, 0);

		M(0, 1) = mat44(0, 1);
		M(1, 1) = mat44(1, 1);
		M(2, 1) = mat44(2, 1);

		M(0, 2) = mat44(0, 2);
		M(1, 2) = mat44(1, 2);
		M(2, 2) = mat44(2, 2);

		t[0] = mat44(0, 3);
		t[1] = mat44(1, 3);
		t[2] = mat44(2, 3);
	}

	PX_CUDA_CALLABLE PX_INLINE operator PxMat44() const
	{
		PxMat44 result;

		result(0, 0) = M(0, 0);
		result(1, 0) = M(1, 0);
		result(2, 0) = M(2, 0);
		result(3, 0) = 0;
				   
		result(0, 1) = M(0, 1);
		result(1, 1) = M(1, 1);
		result(2, 1) = M(2, 1);
		result(3, 1) = 0;
				   
		result(0, 2) = M(0, 2);
		result(1, 2) = M(1, 2);
		result(2, 2) = M(2, 2);
		result(3, 2) = 0;

		result(0, 3) = t[0];
		result(1, 3) = t[1];
		result(2, 3) = t[2];
		result(3, 3) = 1;

		return result;
	}

	// explicit conversion to PxTransform 
	// don't want automatic conversion through operator PxTransform
	// also don't want to expose PxMat34Legacy in explicit PxTransform::PxTransform()
	PX_CUDA_CALLABLE PX_INLINE PxTransform toPxTransform() const;

	PX_CUDA_CALLABLE PX_INLINE void setZero();

	PX_CUDA_CALLABLE PX_INLINE void setIdentity();

	PX_CUDA_CALLABLE PX_INLINE void id(void) // preserve legacy syntax
	{
		setIdentity();
	}

	/**
	\brief returns true for identity matrix
	*/
	PX_CUDA_CALLABLE PX_INLINE bool isIdentity() const;

	/**
	\brief returns true if all elems are finite (not NAN or INF, etc.)
	*/
	PX_CUDA_CALLABLE PX_INLINE bool isFinite() const;

	/**
	\brief assigns inverse to dest. 

	Returns false if singular (i.e. if no inverse exists), setting dest to identity.  dest may equal this.
	*/
	PX_CUDA_CALLABLE PX_INLINE bool getInverse(PxMat34Legacy& dest) const;

	/**
	\brief same as #getInverse(), but assumes that M is orthonormal
	*/
	PX_CUDA_CALLABLE PX_INLINE bool getInverseRT(PxMat34Legacy& dest) const;
	PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy getInverseRT() const;

	/**
	\brief dst = this * src
	*/
	PX_CUDA_CALLABLE PX_CUDA_CALLABLE PX_INLINE void multiply(const PxVec3& src, PxVec3& dst) const;

	/**
	\brief operator wrapper for multiply
	*/
	PX_CUDA_CALLABLE PX_INLINE PxVec3 operator*  (const PxVec3& src) const { PxVec3 dest; multiply(src, dest); return dest; }
	/**
	\brief dst = inverse(this) * src	-- assumes M is rotation matrix!!!
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiplyByInverseRT(const PxVec3& src, PxVec3 &dst) const;
	PX_CUDA_CALLABLE PX_INLINE PxVec3 multiplyByInverseRT(const PxVec3& src) const;

	/**
	\brief operator wrapper for multiplyByInverseRT
	*/
	PX_CUDA_CALLABLE PX_INLINE PxVec3 operator%  (const PxVec3& src) const { PxVec3 dest; multiplyByInverseRT(src, dest); return dest; }

	/**
	\brief this = left * right	
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiply(const PxMat34Legacy& left, const PxMat34Legacy& right);

	/**
	\brief this = inverse(left) * right	-- assumes M is rotation matrix!!!
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiplyInverseRTLeft(const PxMat34Legacy& left, const PxMat34Legacy& right);

	/**
	\brief this = left * inverse(right)	-- assumes M is rotation matrix!!!
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiplyInverseRTRight(const PxMat34Legacy& left, const PxMat34Legacy& right);

	/**
	\brief operator wrapper for multiply
	*/
	PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy operator*  (const PxMat34Legacy& right) const { PxMat34Legacy dest(false); dest.multiply(*this, right); return dest; }

	//! Add
	PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy operator+(const PxMat34Legacy& other) const
	{
		return PxMat34Legacy(M + other.M, t + other.t);
	}

	//! Subtract
	PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy operator-(const PxMat34Legacy& other) const
	{
		return PxMat34Legacy(M - other.M, t - other.t);
	}

	//! Scalar multiplication
	PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy operator*(PxReal scalar) const
	{
		return PxMat34Legacy(M * scalar, t * scalar);
	}

	/**
	\brief convert from a matrix format appropriate for rendering
	*/
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor44(const PxF32 *);
	/**
	\brief convert from a matrix format appropriate for rendering
	*/
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor44(const PxF32 d[4][4]);
	/**
	\brief convert to a matrix format appropriate for rendering
	*/
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor44(PxF32 *) const;
	/**
	\brief convert to a matrix format appropriate for rendering
	*/
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor44(PxF32 d[4][4]) const;
	/**
	\brief set the matrix given a row major matrix.
	*/
	PX_CUDA_CALLABLE PX_INLINE void setRowMajor44(const PxF32 *);
	/**
	\brief set the matrix given a row major matrix.
	*/
	PX_CUDA_CALLABLE PX_INLINE void setRowMajor44(const PxF32 d[4][4]);
	/**
	\brief retrieve the matrix in a row major format.
	*/
	PX_CUDA_CALLABLE PX_INLINE void getRowMajor44(PxF32 *) const;
	/**
	\brief retrieve the matrix in a row major format.
	*/
	PX_CUDA_CALLABLE PX_INLINE void getRowMajor44(PxF32 d[4][4]) const;
};


PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy::PxMat34Legacy(bool init)
{
	if (init)
	{
		t = PxVec3(0.0f);
		M.setIdentity();
	}
}

PX_CUDA_CALLABLE PX_INLINE PxTransform PxMat34Legacy::toPxTransform() const
{
	return PxTransform(t, PxQuat(M));
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::setZero()
{
	M.setZero();
	t = PxVec3(0.0f);
}


PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::setIdentity()
{
	M.setIdentity();
	t = PxVec3(0.0f);
}


PX_CUDA_CALLABLE PX_INLINE bool PxMat34Legacy::isIdentity() const
{
	if(!M.isIdentity())	return false;
	if(!t.isZero())		return false;
	return true;
}


PX_CUDA_CALLABLE PX_INLINE bool PxMat34Legacy::isFinite() const
{
	if(!M.isFinite())	return false;
	if(!t.isFinite())	return false;
	return true;
}


PX_CUDA_CALLABLE PX_INLINE bool PxMat34Legacy::getInverse(PxMat34Legacy& dest) const
{
	// inv(this) = [ inv(M) , inv(M) * -t ]
	bool status = M.getInverse(dest.M);
	dest.M.multiply(-t, dest.t); 
	return status;
}


PX_CUDA_CALLABLE PX_INLINE bool PxMat34Legacy::getInverseRT(PxMat34Legacy& dest) const
{
	// inv(this) = [ M' , M' * -t ]
	dest.M.setTransposed(M);
	dest.M.multiply(-t, dest.t); 
	return true;
}

PX_CUDA_CALLABLE PX_INLINE PxMat34Legacy PxMat34Legacy::getInverseRT() const
{
	PxMat34Legacy result;
	getInverseRT(result);
	return result;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::multiply(const PxVec3& src, PxVec3& dst) const
{
	dst = M * src + t;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::multiplyByInverseRT(const PxVec3& src, PxVec3& dst) const
{
	//dst = M' * src - M' * t = M' * (src - t)
	M.multiplyByTranspose(src - t, dst);
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 PxMat34Legacy::multiplyByInverseRT(const PxVec3& src) const
{
	return M.multiplyByTranspose(src - t);
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::multiply(const PxMat34Legacy& left, const PxMat34Legacy& right)
{
	//[aR at] * [bR bt] = [aR * bR		aR * bt + at]  NOTE: order of operations important so it works when this ?= left ?= right.
	t = left.M * right.t + left.t;
	M.setMultiply(left.M, right.M);
}


PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::multiplyInverseRTLeft(const PxMat34Legacy& left, const PxMat34Legacy& right)
{
	//[aR' -aR'*at] * [bR bt] = [aR' * bR		aR' * bt  - aR'*at]	//aR' ( bt  - at )	NOTE: order of operations important so it works when this ?= left ?= right.
	t = left.M % (right.t - left.t);
	M.setMultiplyTransposeLeft(left.M, right.M);
}


PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::multiplyInverseRTRight(const PxMat34Legacy& left, const PxMat34Legacy& right)
{
	//[aR at] * [bR' -bR'*bt] = [aR * bR'		-aR * bR' * bt + at]	NOTE: order of operations important so it works when this ?= left ?= right.
	M.setMultiplyTransposeRight(left.M, right.M);
	t = left.t - M * right.t;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::setColumnMajor44(const PxF32 * d) 
{
	M.setColumnMajorStride4(d);
	t.x = d[12];
	t.y = d[13];
	t.z = d[14];
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::setColumnMajor44(const PxF32 d[4][4]) 
{
	M.setColumnMajorStride4(d);
	t.x = d[3][0];
	t.y = d[3][1];
	t.z = d[3][2];
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::getColumnMajor44(PxF32 * d) const
{
	M.getColumnMajorStride4(d);
	d[12] = t.x;
	d[13] = t.y;
	d[14] = t.z;
	d[3] = d[7] = d[11] = 0.0f;
	d[15] = 1.0f;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::getColumnMajor44(PxF32 d[4][4]) const
{
	M.getColumnMajorStride4(d);
	d[3][0] = t.x;
	d[3][1] = t.y;
	d[3][2] = t.z;
	d[0][3] = d[1][3] = d[2][3] = 0.0f;
	d[3][3] = 1.0f;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::setRowMajor44(const PxF32 * d) 
{
	M.setRowMajorStride4(d);
	t.x = d[3];
	t.y = d[7];
	t.z = d[11];
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::setRowMajor44(const PxF32 d[4][4])
{
	M.setRowMajorStride4(d);
	t.x = d[0][3];
	t.y = d[1][3];
	t.z = d[2][3];
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::getRowMajor44(PxF32 * d) const
{
	M.getRowMajorStride4(d);
	d[3] = t.x;
	d[7] = t.y;
	d[11] = t.z;
	d[12] = d[13] = d[14] = 0.0f;
	d[15] = 1.0f;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat34Legacy::getRowMajor44(PxF32 d[4][4]) const
{
	M.getRowMajorStride4(d);
	d[0][3] = t.x;
	d[1][3] = t.y;
	d[2][3] = t.z;
	d[3][0] = d[3][1] = d[3][2] = 0.0f;
	d[3][3] = 1.0f;
}

/**
\brief transforms this AABB (resulting in bigger extent).
\param matrix Transform to apply, can contain scaling as well
*/
PX_CUDA_CALLABLE PX_INLINE PxBounds3 transform(const PxMat34Legacy& matrix, const PxBounds3& bounds)
{
	return bounds.isEmpty() ? bounds :
		PxBounds3::basisExtent(matrix * bounds.getCenter(), matrix.M, bounds.getExtents());
}

PX_POP_PACK

} // end namespace general_shared3
using namespace general_shared3;
} // end namespace physx

/** @} */
#endif
