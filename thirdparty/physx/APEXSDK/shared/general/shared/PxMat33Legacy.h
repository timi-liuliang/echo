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

#ifndef PX_FOUNDATION_PXMAT33LEGACY_H
#define PX_FOUNDATION_PXMAT33LEGACY_H

/** \addtogroup foundation
@{
*/

#include "foundation/PxQuat.h"
#include "foundation/PxMat33.h"
#include "foundation/PxMat44.h"

namespace physx
{
namespace general_shared3
{


PX_PUSH_PACK_DEFAULT

/**
\brief 3x3 Matrix Class.

The idea of the matrix/vector classes is to partition them into two parts:
One is the data structure which may have different formatting (3x3, 3x4, 4x4),
row or column major.  The other is a template class which has all the operators
but is storage format independent.

This way it should be easier to change formats depending on what is faster/slower
on a particular platform.

Design issue: We use nameless struct/unions here.
Design issue: this used to be implemented with a template.  This had no benefit
but it added syntactic complexity.  Currently we just use a typedef and a preprocessor switch 
to change between different memory layouts.

The matrix math in this class is storage format (row/col major) independent as far
as the user is concerned.
When the user wants to get/set raw data, he needs to specify what order the data is
coming in.  

*/

	// temporary, while foundation merge happens from PhysX to trunk
#ifndef PX_CUDA_CALLABLE
#define PX_CUDA_CALLABLE
#endif

PX_DEPRECATED class PxMat33Legacy
{
public:
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy();

	/**
	\param type Special matrix type to initialize with.

	@see PxMatrixType
	*/
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy(const PxVec3& row0, const PxVec3& row1, const PxVec3& row2);
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy(const PxMat33Legacy&m);
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy(const PxMat33&m);
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy(const PxMat44&m);
	PX_CUDA_CALLABLE PX_INLINE explicit PxMat33Legacy(const PxQuat& m);
	PX_CUDA_CALLABLE PX_INLINE ~PxMat33Legacy();
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy& operator=(const PxMat33Legacy& src);

	PX_CUDA_CALLABLE PX_INLINE operator PxMat33() const;
	PX_CUDA_CALLABLE PX_INLINE operator PxMat44() const;

	// Access elements

	//low level data access, single or double precision, with eventual translation:
	//for dense 9 element data
	PX_CUDA_CALLABLE PX_INLINE void setRowMajor(const PxF32 *);
	PX_CUDA_CALLABLE PX_INLINE void setRowMajor(const PxF32 d[][3]);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor(const PxF32 *);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor(const PxF32 d[][3]);
	PX_CUDA_CALLABLE PX_INLINE void getRowMajor(PxF32 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getRowMajor(PxF32 d[][3]) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor(PxF32 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor(PxF32 d[][3]) const;

	PX_CUDA_CALLABLE PX_INLINE void setRowMajor(const PxF64 *);
	PX_CUDA_CALLABLE PX_INLINE void setRowMajor(const PxF64 d[][3]);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor(const PxF64 *);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor(const PxF64 d[][3]);
	PX_CUDA_CALLABLE PX_INLINE void getRowMajor(PxF64 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getRowMajor(PxF64 d[][3]) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor(PxF64 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor(PxF64 d[][3]) const;


	//for loose 4-padded 
	PX_CUDA_CALLABLE PX_INLINE void setRowMajorStride4(const PxF32 *);
	PX_CUDA_CALLABLE PX_INLINE void setRowMajorStride4(const PxF32 d[][4]);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajorStride4(const PxF32 *);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajorStride4(const PxF32 d[][4]);
	PX_CUDA_CALLABLE PX_INLINE void getRowMajorStride4(PxF32 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getRowMajorStride4(PxF32 d[][4]) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajorStride4(PxF32 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajorStride4(PxF32 d[][4]) const;

	PX_CUDA_CALLABLE PX_INLINE void setRowMajorStride4(const PxF64 *);
	PX_CUDA_CALLABLE PX_INLINE void setRowMajorStride4(const PxF64 d[][4]);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajorStride4(const PxF64 *);
	PX_CUDA_CALLABLE PX_INLINE void setColumnMajorStride4(const PxF64 d[][4]);
	PX_CUDA_CALLABLE PX_INLINE void getRowMajorStride4(PxF64 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getRowMajorStride4(PxF64 d[][4]) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajorStride4(PxF64 *) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumnMajorStride4(PxF64 d[][4]) const;


	PX_CUDA_CALLABLE PX_INLINE void setRow(int row, const PxVec3&);
	PX_CUDA_CALLABLE PX_INLINE void setColumn(int col, const PxVec3&);
	PX_CUDA_CALLABLE PX_INLINE void getRow(int row, PxVec3&) const;
	PX_CUDA_CALLABLE PX_INLINE void getColumn(int col, PxVec3&) const;

	PX_CUDA_CALLABLE PX_INLINE PxVec3 getRow(int row) const;
	PX_CUDA_CALLABLE PX_INLINE PxVec3 getColumn(int col) const;


	//element access:
	PX_CUDA_CALLABLE PX_INLINE PxReal & operator()(int row, int col);
	PX_CUDA_CALLABLE PX_INLINE const PxReal & operator() (int row, int col) const;

	/**
	\brief returns true for identity matrix
	*/
	PX_CUDA_CALLABLE PX_INLINE bool isIdentity() const;

	/**
	\brief returns true for zero matrix
	*/
	PX_CUDA_CALLABLE PX_INLINE bool isZero() const;

	/**
	\brief returns true if all elems are finite (not NAN or INF, etc.)
	*/
	PX_CUDA_CALLABLE PX_INLINE bool isFinite() const;

	//create special matrices:

	/**
	\brief sets this matrix to the zero matrix.
	*/
	PX_CUDA_CALLABLE PX_INLINE void setZero();

	/**
	\brief sets this matrix to the identity matrix.
	*/
	PX_CUDA_CALLABLE PX_INLINE void setIdentity();

	/**
	\brief this = -this
	*/
	PX_CUDA_CALLABLE PX_INLINE void setNegative();

	/**
	\brief sets this matrix to the diagonal matrix.
	*/
	PX_CUDA_CALLABLE PX_INLINE void setDiagonal(const PxVec3& vec);
	PX_DEPRECATED PX_CUDA_CALLABLE PX_INLINE void diagonal(const PxVec3& vec) { setDiagonal(vec); }


	/**
	\brief Sets this matrix to the Star(Skew Symetric) matrix.

	So that star(v) * x = v.cross(x) .
	*/
	PX_CUDA_CALLABLE PX_INLINE void setStar(const PxVec3& vec);
	PX_DEPRECATED PX_CUDA_CALLABLE PX_INLINE void star(const PxVec3& vec) { setStar(vec); }


	PX_CUDA_CALLABLE PX_INLINE void setFromQuat(const PxQuat&);
	PX_DEPRECATED PX_CUDA_CALLABLE PX_INLINE void fromQuat(const PxQuat& q) { setFromQuat(q); }

	PX_CUDA_CALLABLE PX_INLINE PxQuat toQuat() const;

	//modifications:

	PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &operator +=(const PxMat33Legacy& s);
	PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &operator -=(const PxMat33Legacy& s);
	PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &operator *=(PxReal s);
	PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &operator /=(PxReal s);

	/*
	Gram-Schmidt orthogonalization to correct numerical drift, plus column normalization
	Caution: I believe the current implementation does not work right!
	*/
	//	PX_CUDA_CALLABLE PX_INLINE void orthonormalize();


	/**
	\brief returns determinant
	*/
	PX_CUDA_CALLABLE PX_INLINE PxReal determinant() const;

	/**
	\brief assigns inverse to dest.

	Returns false if singular (i.e. if no inverse exists), setting dest to identity.
	*/
	PX_CUDA_CALLABLE PX_INLINE bool getInverse(PxMat33Legacy& dest) const;

	/**
	\brief this = transpose(other)

	this == other is OK.
	*/
	PX_CUDA_CALLABLE PX_INLINE void setTransposed(const PxMat33Legacy& other);

	/**
	\brief dst = this * [ d.x 0 0; 0 d.y 0; 0 0 d.z];
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiplyDiagonal(const PxVec3& d, PxMat33Legacy& dst) const;
	PX_CUDA_CALLABLE PX_INLINE void multiplyDiagonal(const PxVec3& d) { multiplyDiagonal(d, *this); }

	/**
	\brief dst = transpose(this) * [ d.x 0 0; 0 d.y 0; 0 0 d.z];
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiplyDiagonalTranspose(const PxVec3& d, PxMat33Legacy& dst) const;

	/**
	\brief dst = this * src
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiply(const PxVec3& src, PxVec3& dst) const;
	PX_CUDA_CALLABLE PX_INLINE PxVec3 multiply(const PxVec3& src) const;

	/**
	\brief dst = transpose(this) * src
	*/
	PX_CUDA_CALLABLE PX_INLINE void multiplyByTranspose(const PxVec3& src, PxVec3& dst) const;
	PX_CUDA_CALLABLE PX_INLINE PxVec3 multiplyByTranspose(const PxVec3& src) const;

	/**
	\brief this = a + b
	*/
	PX_CUDA_CALLABLE PX_INLINE void  setAdd(const PxMat33Legacy& a, const PxMat33Legacy& b);

	/***
	\brief this = a - b
	*/
	PX_CUDA_CALLABLE PX_INLINE void  setSubtract(const PxMat33Legacy& a, const PxMat33Legacy& b);

	/**
	\brief this = s * a;
	*/
	PX_CUDA_CALLABLE PX_INLINE void  setMultiply(PxReal s,  const PxMat33Legacy& a);

	/**
	\brief this = left * right
	*/
	PX_CUDA_CALLABLE PX_INLINE void setMultiply(const PxMat33Legacy left, const PxMat33Legacy right);
	PX_DEPRECATED PX_CUDA_CALLABLE PX_INLINE void multiply(const PxMat33Legacy left, const PxMat33Legacy right) { setMultiply(left, right); }

	/**
	\brief this = transpose(left) * right

	\note #multiplyByTranspose() is faster.
	*/
	PX_CUDA_CALLABLE PX_INLINE void setMultiplyTransposeLeft(const PxMat33Legacy left, const PxMat33Legacy right);
	PX_DEPRECATED PX_CUDA_CALLABLE PX_INLINE void multiplyTransposeLeft(const PxMat33Legacy left, const PxMat33Legacy right) { setMultiplyTransposeLeft(left, right); }

	/**
	\brief this = left * transpose(right)

	\note faster than #multiplyByTranspose().
	*/
	PX_CUDA_CALLABLE PX_INLINE void setMultiplyTransposeRight(const PxMat33Legacy left, const PxMat33Legacy right);
	PX_DEPRECATED void multiplyTransposeRight(const PxMat33Legacy left, const PxMat33Legacy right) { setMultiplyTransposeRight(left, right); }

	/**
	\brief this = left * transpose(right)
	*/
	PX_CUDA_CALLABLE PX_INLINE void setMultiplyTransposeRight(const PxVec3& left, const PxVec3& right);

	/**
	\brief this = rotation matrix around X axis

	<b>Unit:</b> Radians
	*/
	PX_CUDA_CALLABLE PX_INLINE void setRotX(PxReal angle);
	PX_DEPRECATED PX_CUDA_CALLABLE PX_INLINE void rotX(PxReal angle) { setRotX(angle); }

	/**
	\brief this = rotation matrix around Y axis

	<b>Unit:</b> Radians
	*/
	PX_CUDA_CALLABLE PX_INLINE void setRotY(PxReal angle);

	/**
	\brief this = rotation matrix around Z axis

	<b>Unit:</b> Radians
	*/
	PX_CUDA_CALLABLE PX_INLINE void setRotZ(PxReal angle);


	//overloaded multiply, and transposed-multiply ops:

	/**
	\brief returns transpose(this)*src
	*/
	PX_CUDA_CALLABLE PX_INLINE PxVec3 operator%  (const PxVec3& src) const;
	/**
	\brief matrix vector product
	*/
	PX_CUDA_CALLABLE PX_INLINE PxVec3 operator*  (const PxVec3& src) const;
	/**
	\brief matrix product
	*/
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy&	operator*= (const PxMat33Legacy& mat);
	/**
	\brief matrix difference
	*/
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	operator-  (const PxMat33Legacy& mat)	const;
	/**
	\brief matrix addition
	*/
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	operator+  (const PxMat33Legacy& mat)	const;
	/**
	\brief matrix product
	*/
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	operator*  (const PxMat33Legacy& mat)	const;
	/**
	\brief matrix scalar product
	*/
	PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	operator*  (float s)				const;

//private:


	union 
	{
		struct 
		{
			PxReal        _11, _12, _13;
			PxReal        _21, _22, _23;
			PxReal        _31, _32, _33;
		} s;
		PxReal m[3][3];
	};
};


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::PxMat33Legacy()
{
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::PxMat33Legacy(const PxMat33Legacy& a)
{
	s = a.s;
}

PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::PxMat33Legacy(const PxMat33&m)
{
	s._11 = m(0,0);
	s._12 = m(0,1);
	s._13 = m(0,2);

	s._21 = m(1,0);
	s._22 = m(1,1);
	s._23 = m(1,2);

	s._31 = m(2,0);
	s._32 = m(2,1);
	s._33 = m(2,2);
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::PxMat33Legacy(const PxQuat& q)
{
	setFromQuat(q);
}

PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::PxMat33Legacy(const PxVec3& row0, const PxVec3& row1, const PxVec3& row2)
{
	s._11 = row0.x;  s._12 = row0.y;  s._13 = row0.z;
	s._21 = row1.x;  s._22 = row1.y;  s._23 = row1.z;
	s._31 = row2.x;  s._32 = row2.y;  s._33 = row2.z;
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::~PxMat33Legacy()
{
	//nothing
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy& PxMat33Legacy::operator=(const PxMat33Legacy& a)
{
	s = a.s;
	return *this;
}

PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::operator PxMat33() const
{
	PxMat33 result;
	getColumnMajor(&result(0,0));
	return result;
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::PxMat33Legacy(const PxMat44& mat44)
{
	(*this)(0, 0) = mat44(0, 0);
	(*this)(1, 0) = mat44(1, 0);
	(*this)(2, 0) = mat44(2, 0);

	(*this)(0, 1) = mat44(0, 1);
	(*this)(1, 1) = mat44(1, 1);
	(*this)(2, 1) = mat44(2, 1);

	(*this)(0, 2) = mat44(0, 2);
	(*this)(1, 2) = mat44(1, 2);
	(*this)(2, 2) = mat44(2, 2);
}

PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy::operator PxMat44() const
{
	PxMat44 result;

	result(0, 0) = (*this)(0, 0);
	result(1, 0) = (*this)(1, 0);
	result(2, 0) = (*this)(2, 0);
	result(3, 0) = 0;

	result(0, 1) = (*this)(0, 1);
	result(1, 1) = (*this)(1, 1);
	result(2, 1) = (*this)(2, 1);
	result(3, 1) = 0;

	result(0, 2) = (*this)(0, 2);
	result(1, 2) = (*this)(1, 2);
	result(2, 2) = (*this)(2, 2);
	result(3, 2) = 0;

	result(0, 3) = 0;
	result(1, 3) = 0;
	result(2, 3) = 0;
	result(3, 3) = 1;

	return result;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajor(const PxF32* d)
{
	//we are also row major, so this is a direct copy
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[1];
	s._13 = (PxReal)d[2];

	s._21 = (PxReal)d[3];
	s._22 = (PxReal)d[4];
	s._23 = (PxReal)d[5];

	s._31 = (PxReal)d[6];
	s._32 = (PxReal)d[7];
	s._33 = (PxReal)d[8];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajor(const PxF32 d[][3])
{
	//we are also row major, so this is a direct copy
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[0][1];
	s._13 = (PxReal)d[0][2];

	s._21 = (PxReal)d[1][0];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[1][2];

	s._31 = (PxReal)d[2][0];
	s._32 = (PxReal)d[2][1];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajor(const PxF32* d)
{
	//we are column major, so copy transposed.
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[3];
	s._13 = (PxReal)d[6];

	s._21 = (PxReal)d[1];
	s._22 = (PxReal)d[4];
	s._23 = (PxReal)d[7];

	s._31 = (PxReal)d[2];
	s._32 = (PxReal)d[5];
	s._33 = (PxReal)d[8];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajor(const PxF32 d[][3])
{
	//we are column major, so copy transposed.
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[1][0];
	s._13 = (PxReal)d[2][0];

	s._21 = (PxReal)d[0][1];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[2][1];

	s._31 = (PxReal)d[0][2];
	s._32 = (PxReal)d[1][2];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajor(PxF32* d) const
{
	//we are also row major, so this is a direct copy
	d[0] = (PxF32)s._11;
	d[1] = (PxF32)s._12;
	d[2] = (PxF32)s._13;

	d[3] = (PxF32)s._21;
	d[4] = (PxF32)s._22;
	d[5] = (PxF32)s._23;

	d[6] = (PxF32)s._31;
	d[7] = (PxF32)s._32;
	d[8] = (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajor(PxF32 d[][3]) const
{
	//we are also row major, so this is a direct copy
	d[0][0] = (PxF32)s._11;
	d[0][1] = (PxF32)s._12;
	d[0][2] = (PxF32)s._13;

	d[1][0] = (PxF32)s._21;
	d[1][1] = (PxF32)s._22;
	d[1][2] = (PxF32)s._23;

	d[2][0] = (PxF32)s._31;
	d[2][1] = (PxF32)s._32;
	d[2][2] = (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajor(PxF32* d) const
{
	//we are column major, so copy transposed.
	d[0] = (PxF32)s._11;
	d[3] = (PxF32)s._12;
	d[6] = (PxF32)s._13;

	d[1] = (PxF32)s._21;
	d[4] = (PxF32)s._22;
	d[7] = (PxF32)s._23;

	d[2] = (PxF32)s._31;
	d[5] = (PxF32)s._32;
	d[8] = (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajor(PxF32 d[][3]) const
{
	//we are column major, so copy transposed.
	d[0][0] = (PxF32)s._11;
	d[1][0] = (PxF32)s._12;
	d[2][0] = (PxF32)s._13;

	d[0][1] = (PxF32)s._21;
	d[1][1] = (PxF32)s._22;
	d[2][1] = (PxF32)s._23;

	d[0][2] = (PxF32)s._31;
	d[1][2] = (PxF32)s._32;
	d[2][2] = (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajorStride4(const PxF32* d)
{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[1];
	s._13 = (PxReal)d[2];

	s._21 = (PxReal)d[4];
	s._22 = (PxReal)d[5];
	s._23 = (PxReal)d[6];

	s._31 = (PxReal)d[8];
	s._32 = (PxReal)d[9];
	s._33 = (PxReal)d[10];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajorStride4(const PxF32 d[][4])
{
	//we are also row major, so this is a direct copy
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[0][1];
	s._13 = (PxReal)d[0][2];

	s._21 = (PxReal)d[1][0];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[1][2];

	s._31 = (PxReal)d[2][0];
	s._32 = (PxReal)d[2][1];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajorStride4(const PxF32* d)
{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[4];
	s._13 = (PxReal)d[8];

	s._21 = (PxReal)d[1];
	s._22 = (PxReal)d[5];
	s._23 = (PxReal)d[9];

	s._31 = (PxReal)d[2];
	s._32 = (PxReal)d[6];
	s._33 = (PxReal)d[10];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajorStride4(const PxF32 d[][4])
{
	//we are column major, so copy transposed.
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[1][0];
	s._13 = (PxReal)d[2][0];

	s._21 = (PxReal)d[0][1];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[2][1];

	s._31 = (PxReal)d[0][2];
	s._32 = (PxReal)d[1][2];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajorStride4(PxF32* d) const
{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	d[0] = (PxF32)s._11;
	d[1] = (PxF32)s._12;
	d[2] = (PxF32)s._13;

	d[4] = (PxF32)s._21;
	d[5] = (PxF32)s._22;
	d[6] = (PxF32)s._23;

	d[8] = (PxF32)s._31;
	d[9] = (PxF32)s._32;
	d[10]= (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajorStride4(PxF32 d[][4]) const
{
	//we are also row major, so this is a direct copy
	d[0][0] = (PxF32)s._11;
	d[0][1] = (PxF32)s._12;
	d[0][2] = (PxF32)s._13;

	d[1][0] = (PxF32)s._21;
	d[1][1] = (PxF32)s._22;
	d[1][2] = (PxF32)s._23;

	d[2][0] = (PxF32)s._31;
	d[2][1] = (PxF32)s._32;
	d[2][2] = (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajorStride4(PxF32* d) const
{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	d[0] = (PxF32)s._11;
	d[4] = (PxF32)s._12;
	d[8] = (PxF32)s._13;

	d[1] = (PxF32)s._21;
	d[5] = (PxF32)s._22;
	d[9] = (PxF32)s._23;

	d[2] = (PxF32)s._31;
	d[6] = (PxF32)s._32;
	d[10]= (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajorStride4(PxF32 d[][4]) const
{
	//we are column major, so copy transposed.
	d[0][0] = (PxF32)s._11;
	d[1][0] = (PxF32)s._12;
	d[2][0] = (PxF32)s._13;

	d[0][1] = (PxF32)s._21;
	d[1][1] = (PxF32)s._22;
	d[2][1] = (PxF32)s._23;

	d[0][2] = (PxF32)s._31;
	d[1][2] = (PxF32)s._32;
	d[2][2] = (PxF32)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajor(const PxF64*d)
{
	//we are also row major, so this is a direct copy
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[1];
	s._13 = (PxReal)d[2];

	s._21 = (PxReal)d[3];
	s._22 = (PxReal)d[4];
	s._23 = (PxReal)d[5];

	s._31 = (PxReal)d[6];
	s._32 = (PxReal)d[7];
	s._33 = (PxReal)d[8];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajor(const PxF64 d[][3])
{
	//we are also row major, so this is a direct copy
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[0][1];
	s._13 = (PxReal)d[0][2];

	s._21 = (PxReal)d[1][0];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[1][2];

	s._31 = (PxReal)d[2][0];
	s._32 = (PxReal)d[2][1];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajor(const PxF64*d)
{
	//we are column major, so copy transposed.
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[3];
	s._13 = (PxReal)d[6];

	s._21 = (PxReal)d[1];
	s._22 = (PxReal)d[4];
	s._23 = (PxReal)d[7];

	s._31 = (PxReal)d[2];
	s._32 = (PxReal)d[5];
	s._33 = (PxReal)d[8];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajor(const PxF64 d[][3])
{
	//we are column major, so copy transposed.
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[1][0];
	s._13 = (PxReal)d[2][0];

	s._21 = (PxReal)d[0][1];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[2][1];

	s._31 = (PxReal)d[0][2];
	s._32 = (PxReal)d[1][2];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajor(PxF64*d) const
{
	//we are also row major, so this is a direct copy
	d[0] = (PxF64)s._11;
	d[1] = (PxF64)s._12;
	d[2] = (PxF64)s._13;

	d[3] = (PxF64)s._21;
	d[4] = (PxF64)s._22;
	d[5] = (PxF64)s._23;

	d[6] = (PxF64)s._31;
	d[7] = (PxF64)s._32;
	d[8] = (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajor(PxF64 d[][3]) const
{
	//we are also row major, so this is a direct copy
	d[0][0] = (PxF64)s._11;
	d[0][1] = (PxF64)s._12;
	d[0][2] = (PxF64)s._13;

	d[1][0] = (PxF64)s._21;
	d[1][1] = (PxF64)s._22;
	d[1][2] = (PxF64)s._23;

	d[2][0] = (PxF64)s._31;
	d[2][1] = (PxF64)s._32;
	d[2][2] = (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajor(PxF64*d) const
{
	//we are column major, so copy transposed.
	d[0] = (PxF64)s._11;
	d[3] = (PxF64)s._12;
	d[6] = (PxF64)s._13;

	d[1] = (PxF64)s._21;
	d[4] = (PxF64)s._22;
	d[7] = (PxF64)s._23;

	d[2] = (PxF64)s._31;
	d[5] = (PxF64)s._32;
	d[8] = (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajor(PxF64 d[][3]) const
{
	//we are column major, so copy transposed.
	d[0][0] = (PxF64)s._11;
	d[1][0] = (PxF64)s._12;
	d[2][0] = (PxF64)s._13;

	d[0][1] = (PxF64)s._21;
	d[1][1] = (PxF64)s._22;
	d[2][1] = (PxF64)s._23;

	d[0][2] = (PxF64)s._31;
	d[1][2] = (PxF64)s._32;
	d[2][2] = (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajorStride4(const PxF64*d)
{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[1];
	s._13 = (PxReal)d[2];

	s._21 = (PxReal)d[4];
	s._22 = (PxReal)d[5];
	s._23 = (PxReal)d[6];

	s._31 = (PxReal)d[8];
	s._32 = (PxReal)d[9];
	s._33 = (PxReal)d[10];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRowMajorStride4(const PxF64 d[][4])
{
	//we are also row major, so this is a direct copy
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[0][1];
	s._13 = (PxReal)d[0][2];

	s._21 = (PxReal)d[1][0];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[1][2];

	s._31 = (PxReal)d[2][0];
	s._32 = (PxReal)d[2][1];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajorStride4(const PxF64*d)
{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	s._11 = (PxReal)d[0];
	s._12 = (PxReal)d[4];
	s._13 = (PxReal)d[8];

	s._21 = (PxReal)d[1];
	s._22 = (PxReal)d[5];
	s._23 = (PxReal)d[9];

	s._31 = (PxReal)d[2];
	s._32 = (PxReal)d[6];
	s._33 = (PxReal)d[10];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumnMajorStride4(const PxF64 d[][4])
{
	//we are column major, so copy transposed.
	s._11 = (PxReal)d[0][0];
	s._12 = (PxReal)d[1][0];
	s._13 = (PxReal)d[2][0];

	s._21 = (PxReal)d[0][1];
	s._22 = (PxReal)d[1][1];
	s._23 = (PxReal)d[2][1];

	s._31 = (PxReal)d[0][2];
	s._32 = (PxReal)d[1][2];
	s._33 = (PxReal)d[2][2];
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajorStride4(PxF64*d) const
{
	//we are also row major, so this is a direct copy
	//however we've got to skip every 4th element.
	d[0] = (PxF64)s._11;
	d[1] = (PxF64)s._12;
	d[2] = (PxF64)s._13;

	d[4] = (PxF64)s._21;
	d[5] = (PxF64)s._22;
	d[6] = (PxF64)s._23;

	d[8] = (PxF64)s._31;
	d[9] = (PxF64)s._32;
	d[10]= (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRowMajorStride4(PxF64 d[][4]) const
{
	//we are also row major, so this is a direct copy
	d[0][0] = (PxF64)s._11;
	d[0][1] = (PxF64)s._12;
	d[0][2] = (PxF64)s._13;

	d[1][0] = (PxF64)s._21;
	d[1][1] = (PxF64)s._22;
	d[1][2] = (PxF64)s._23;

	d[2][0] = (PxF64)s._31;
	d[2][1] = (PxF64)s._32;
	d[2][2] = (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajorStride4(PxF64*d) const

{
	//we are column major, so copy transposed.
	//however we've got to skip every 4th element.
	d[0] = (PxF64)s._11;
	d[4] = (PxF64)s._12;
	d[8] = (PxF64)s._13;

	d[1] = (PxF64)s._21;
	d[5] = (PxF64)s._22;
	d[9] = (PxF64)s._23;

	d[2] = (PxF64)s._31;
	d[6] = (PxF64)s._32;
	d[10]= (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumnMajorStride4(PxF64 d[][4]) const
{
	//we are column major, so copy transposed.
	d[0][0] = (PxF64)s._11;
	d[1][0] = (PxF64)s._12;
	d[2][0] = (PxF64)s._13;

	d[0][1] = (PxF64)s._21;
	d[1][1] = (PxF64)s._22;
	d[2][1] = (PxF64)s._23;

	d[0][2] = (PxF64)s._31;
	d[1][2] = (PxF64)s._32;
	d[2][2] = (PxF64)s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRow(int row, const PxVec3&  v)
{
#ifndef TRANSPOSED_MAT33
	m[row][0] = v.x;
	m[row][1] = v.y;
	m[row][2] = v.z;
#else
	m[0][row] = v.x;
	m[1][row] = v.y;
	m[2][row] = v.z;
#endif
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setColumn(int col, const PxVec3& v)
{
#ifndef TRANSPOSED_MAT33
	m[0][col] = v.x;
	m[1][col] = v.y;
	m[2][col] = v.z;
#else
	m[col][0] = v.x;
	m[col][1] = v.y;
	m[col][2] = v.z;
#endif
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getRow(int row, PxVec3& v) const
{
#ifndef TRANSPOSED_MAT33
	v.x = m[row][0];
	v.y = m[row][1];
	v.z = m[row][2];
#else
	v.x = m[0][row];
	v.y = m[1][row];
	v.z = m[2][row];
#endif
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::getColumn(int col, PxVec3& v) const
{
#ifndef TRANSPOSED_MAT33
	v.x = m[0][col];
	v.y = m[1][col];
	v.z = m[2][col];
#else
	v.x = m[col][0];
	v.y = m[col][1];
	v.z = m[col][2];
#endif
}


PX_CUDA_CALLABLE PX_INLINE PxVec3 PxMat33Legacy::getRow(int row) const
{
#ifndef TRANSPOSED_MAT33
	return PxVec3(m[row][0],m[row][1],m[row][2]);
#else
	return PxVec3(m[0][row],m[1][row],m[2][row]);
#endif
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 PxMat33Legacy::getColumn(int col) const
{
#ifndef TRANSPOSED_MAT33
	return PxVec3(m[0][col],m[1][col],m[2][col]);
#else
	return PxVec3(m[col][0],m[col][1],m[col][2]);
#endif
}

PX_CUDA_CALLABLE PX_INLINE PxReal& PxMat33Legacy::operator()(int row, int col)
{
	PX_ASSERT(row>=0 && row<3 && col>=0 && col<3);
#ifndef TRANSPOSED_MAT33
	return m[row][col];
#else
	return m[col][row];
#endif
}


PX_CUDA_CALLABLE PX_INLINE const PxReal& PxMat33Legacy::operator() (int row, int col) const
{
	PX_ASSERT(row>=0 && row<3 && col>=0 && col<3);
#ifndef TRANSPOSED_MAT33
	return m[row][col];
#else
	return m[col][row];
#endif
}

//const methods


PX_CUDA_CALLABLE PX_INLINE bool PxMat33Legacy::isIdentity() const
{
	if(s._11 != 1.0f)		return false;
	if(s._12 != 0.0f)		return false;
	if(s._13 != 0.0f)		return false;

	if(s._21 != 0.0f)		return false;
	if(s._22 != 1.0f)		return false;
	if(s._23 != 0.0f)		return false;

	if(s._31 != 0.0f)		return false;
	if(s._32 != 0.0f)		return false;
	if(s._33 != 1.0f)		return false;

	return true;
}


PX_CUDA_CALLABLE PX_INLINE bool PxMat33Legacy::isZero() const
{
	if(s._11 != 0.0f)		return false;
	if(s._12 != 0.0f)		return false;
	if(s._13 != 0.0f)		return false;

	if(s._21 != 0.0f)		return false;
	if(s._22 != 0.0f)		return false;
	if(s._23 != 0.0f)		return false;

	if(s._31 != 0.0f)		return false;
	if(s._32 != 0.0f)		return false;
	if(s._33 != 0.0f)		return false;

	return true;
}


PX_CUDA_CALLABLE PX_INLINE bool PxMat33Legacy::isFinite() const
{
	return PxIsFinite(s._11)
		&& PxIsFinite(s._12)
		&& PxIsFinite(s._13)

		&& PxIsFinite(s._21)
		&& PxIsFinite(s._22)
		&& PxIsFinite(s._23)

		&& PxIsFinite(s._31)
		&& PxIsFinite(s._32)
		&& PxIsFinite(s._33);
}



PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setZero()
{
	s._11 = PxReal(0);
	s._12 = PxReal(0);
	s._13 = PxReal(0);

	s._21 = PxReal(0);
	s._22 = PxReal(0);
	s._23 = PxReal(0);

	s._31 = PxReal(0);
	s._32 = PxReal(0);
	s._33 = PxReal(0);
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setIdentity()
{
	s._11 = PxReal(1);
	s._12 = PxReal(0);
	s._13 = PxReal(0);

	s._21 = PxReal(0);
	s._22 = PxReal(1);
	s._23 = PxReal(0);

	s._31 = PxReal(0);
	s._32 = PxReal(0);
	s._33 = PxReal(1);
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setNegative()
{
	s._11 = -s._11;
	s._12 = -s._12;
	s._13 = -s._13;

	s._21 = -s._21;
	s._22 = -s._22;
	s._23 = -s._23;

	s._31 = -s._31;
	s._32 = -s._32;
	s._33 = -s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setDiagonal(const PxVec3& v)
{
	s._11 = v.x;
	s._12 = PxReal(0);
	s._13 = PxReal(0);

	s._21 = PxReal(0);
	s._22 = v.y;
	s._23 = PxReal(0);

	s._31 = PxReal(0);
	s._32 = PxReal(0);
	s._33 = v.z;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setStar(const PxVec3& v)
{
	s._11 = PxReal(0);	s._12 =-v.z;		s._13 = v.y;
	s._21 = v.z;		s._22 = PxReal(0);	s._23 =-v.x;
	s._31 =-v.y;		s._32 = v.x;		s._33 = PxReal(0);
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setFromQuat(const PxQuat& q)
{
	const PxReal w = q.w;
	const PxReal x = q.x;
	const PxReal y = q.y;
	const PxReal z = q.z;

	s._11 = PxReal(1) - y*y*PxReal(2) - z*z*PxReal(2);
	s._12 = x*y*PxReal(2) - w*z*PxReal(2);	
	s._13 = x*z*PxReal(2) + w*y*PxReal(2);	

	s._21 = x*y*PxReal(2) + w*z*PxReal(2);	
	s._22 = PxReal(1) - x*x*PxReal(2) - z*z*PxReal(2);	
	s._23 = y*z*PxReal(2) - w*x*PxReal(2);	

	s._31 = x*z*PxReal(2) - w*y*PxReal(2);	
	s._32 = y*z*PxReal(2) + w*x*PxReal(2);	
	s._33 = PxReal(1) - x*x*PxReal(2) - y*y*PxReal(2);	
}


PX_CUDA_CALLABLE PX_INLINE PxQuat PxMat33Legacy::toQuat() const
{
	PxQuat q;
	PxReal tr = s._11 + s._22 + s._33, h;
	if(tr >= 0)
	{
		h = PxSqrt(tr +1);
		q.w = PxReal(0.5) * h;
		h = PxReal(0.5) / h;

		q.x = ((*this)(2,1) - (*this)(1,2)) * h;
		q.y = ((*this)(0,2) - (*this)(2,0)) * h;
		q.z = ((*this)(1,0) - (*this)(0,1)) * h;
	}
	else
	{
		int i = 0; 
		if (s._22 > s._11)
			i = 1; 
		if(s._33 > (*this)(i,i))
			i=2; 
		switch (i)
		{
		case 0:
			h = PxSqrt((s._11 - (s._22 + s._33)) + 1);
			q.x = PxReal(0.5) * h;
			h = PxReal(0.5) / h;

			q.y = ((*this)(0,1) + (*this)(1,0)) * h; 
			q.z = ((*this)(2,0) + (*this)(0,2)) * h;
			q.w = ((*this)(2,1) - (*this)(1,2)) * h;
			break;
		case 1:
			h = PxSqrt((s._22 - (s._33 + s._11)) + 1);
			q.y = PxReal(0.5) * h;
			h = PxReal(0.5) / h;

			q.z = ((*this)(1,2) + (*this)(2,1)) * h;
			q.x = ((*this)(0,1) + (*this)(1,0)) * h;
			q.w = ((*this)(0,2) - (*this)(2,0)) * h;
			break;
		case 2:
			h = PxSqrt((s._33 - (s._11 + s._22)) + 1);
			q.z = PxReal(0.5) * h;
			h = PxReal(0.5) / h;

			q.x = ((*this)(2,0) + (*this)(0,2)) * h;
			q.y = ((*this)(1,2) + (*this)(2,1)) * h;
			q.w = ((*this)(1,0) - (*this)(0,1)) * h;
		}
	}
	return q;
}
/*

PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::orthonormalize()	//Gram-Schmidt orthogonalization to correct numerical drift, plus column normalization
{
//TODO: This is buggy!
PxVec3 w,t1,t2,t3;
PxReal norm_sq;

const PxReal m=3;			//m := linalg[rowdim](A);
const PxReal n=3;			//n := linalg[coldim](A);
int i, j, k = 0;				//k := 0;


Mat33d v = *this;				//v := linalg[col](A, 1 .. n); -- 3 column vectors indexable
PxVec3 norm_u_sq;
//# orthogonalize v[i]
for (i=0; i<n; i++)//for i to n do
{
v.getColumn(i,w);		//i-th column
for (j=0; j<k; j++)									//# pull w along projection of v[i] with u[j]
{
this->getColumn(j,t1);
this->getColumn(j,t2);
v.getColumn(i,t3);
PxVec3 temp = (t2 * (PxReal(1)/norm_u_sq[j]));
PxVec3 temp2 = temp  * t3.dot( t1 );
w -= temp;	
}
//        # compute norm of orthogonalized v[i]
norm_sq = w.Dot(w);

if (norm_sq != PxReal(0)) 
{													//           # linearly independent new orthogonal vector 
//       # add to list of u and norm_u_sq
this->SetColumn(i,w);									//u = [op(u), evalm(w)];
norm_u_sq[i] = norm_sq;						//norm_u_sq = [op(norm_u_sq), norm_sq];
k ++;
}
}


PxVec3 temp;													//may want to do this in-place -- dunno if optimizer does this for me
for (i=0; i<3; i++)
{
getColumn(i,temp);
temp.normalize();
setColumn(i,temp);
}
}
*/


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setTransposed(const PxMat33Legacy& other)
{
	//gotta special case in-place case
	if (this != &other)
	{
		s._11 = other.s._11;
		s._12 = other.s._21;
		s._13 = other.s._31;

		s._21 = other.s._12;
		s._22 = other.s._22;
		s._23 = other.s._32;

		s._31 = other.s._13;
		s._32 = other.s._23;
		s._33 = other.s._33;
	}
	else
	{
		PxReal tx, ty, tz;
		tx = s._21;	s._21 = other.s._12;	s._12 = tx;
		ty = s._31;	s._31 = other.s._13;	s._13 = ty;
		tz = s._32;	s._32 = other.s._23;	s._23 = tz;
	}
}

PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::multiplyDiagonal(const PxVec3& d, PxMat33Legacy& dst) const
{
	dst.s._11 = s._11 * d.x;
	dst.s._12 = s._12 * d.y;
	dst.s._13 = s._13 * d.z;

	dst.s._21 = s._21 * d.x;
	dst.s._22 = s._22 * d.y;
	dst.s._23 = s._23 * d.z;

	dst.s._31 = s._31 * d.x;
	dst.s._32 = s._32 * d.y;
	dst.s._33 = s._33 * d.z;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::multiplyDiagonalTranspose(const PxVec3& d, PxMat33Legacy& dst) const
{
	dst.s._11 = s._11 * d.x;
	dst.s._12 = s._21 * d.y;
	dst.s._13 = s._31 * d.z;

	dst.s._21 = s._12 * d.x;
	dst.s._22 = s._22 * d.y;
	dst.s._23 = s._32 * d.z;

	dst.s._31 = s._13 * d.x;
	dst.s._32 = s._23 * d.y;
	dst.s._33 = s._33 * d.z;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::multiply(const PxVec3& src, PxVec3& dst) const
{
	dst = multiply(src);
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 PxMat33Legacy::multiply(const PxVec3& src) const
{
	PxVec3 result;

	result.x = s._11 * src.x + s._12 * src.y + s._13 * src.z;
	result.y = s._21 * src.x + s._22 * src.y + s._23 * src.z;
	result.z = s._31 * src.x + s._32 * src.y + s._33 * src.z;

	return result;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::multiplyByTranspose(const PxVec3& src, PxVec3& dst) const
{
	dst = multiplyByTranspose(src);
}

PX_CUDA_CALLABLE PX_INLINE PxVec3 PxMat33Legacy::multiplyByTranspose(const PxVec3& src) const
{
	PxVec3 result;

	result.x = s._11 * src.x + s._21 * src.y + s._31 * src.z;
	result.y = s._12 * src.x + s._22 * src.y + s._32 * src.z;
	result.z = s._13 * src.x + s._23 * src.y + s._33 * src.z;

	return result;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setAdd(const PxMat33Legacy& a, const PxMat33Legacy& b)
{
	s._11 = a.s._11 + b.s._11;
	s._12 = a.s._12 + b.s._12;
	s._13 = a.s._13 + b.s._13;

	s._21 = a.s._21 + b.s._21;
	s._22 = a.s._22 + b.s._22;
	s._23 = a.s._23 + b.s._23;

	s._31 = a.s._31 + b.s._31;
	s._32 = a.s._32 + b.s._32;
	s._33 = a.s._33 + b.s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setSubtract(const PxMat33Legacy& a, const PxMat33Legacy& b)
{
	s._11 = a.s._11 - b.s._11;
	s._12 = a.s._12 - b.s._12;
	s._13 = a.s._13 - b.s._13;

	s._21 = a.s._21 - b.s._21;
	s._22 = a.s._22 - b.s._22;
	s._23 = a.s._23 - b.s._23;

	s._31 = a.s._31 - b.s._31;
	s._32 = a.s._32 - b.s._32;
	s._33 = a.s._33 - b.s._33;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setMultiply(PxReal d,  const PxMat33Legacy& a)
{
	s._11 = a.s._11 * d;
	s._12 = a.s._12 * d;
	s._13 = a.s._13 * d;

	s._21 = a.s._21 * d;
	s._22 = a.s._22 * d;
	s._23 = a.s._23 * d;

	s._31 = a.s._31 * d;
	s._32 = a.s._32 * d;
	s._33 = a.s._33 * d;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setMultiply(const PxMat33Legacy left, const PxMat33Legacy right)
{
	PxReal a,b,c, d,e,f, g,h,i;
	//note: temps needed so that x.multiply(x,y) works OK.
	a =left.s._11 * right.s._11 +left.s._12 * right.s._21 +left.s._13 * right.s._31;
	b =left.s._11 * right.s._12 +left.s._12 * right.s._22 +left.s._13 * right.s._32;
	c =left.s._11 * right.s._13 +left.s._12 * right.s._23 +left.s._13 * right.s._33;

	d =left.s._21 * right.s._11 +left.s._22 * right.s._21 +left.s._23 * right.s._31;
	e =left.s._21 * right.s._12 +left.s._22 * right.s._22 +left.s._23 * right.s._32;
	f =left.s._21 * right.s._13 +left.s._22 * right.s._23 +left.s._23 * right.s._33;

	g =left.s._31 * right.s._11 +left.s._32 * right.s._21 +left.s._33 * right.s._31;
	h =left.s._31 * right.s._12 +left.s._32 * right.s._22 +left.s._33 * right.s._32;
	i =left.s._31 * right.s._13 +left.s._32 * right.s._23 +left.s._33 * right.s._33;


	s._11 = a;
	s._12 = b;
	s._13 = c;

	s._21 = d;
	s._22 = e;
	s._23 = f;

	s._31 = g;
	s._32 = h;
	s._33 = i;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setMultiplyTransposeLeft(const PxMat33Legacy left, const PxMat33Legacy right)
{
	PxReal a,b,c, d,e,f, g,h,i;
	//note: temps needed so that x.multiply(x,y) works OK.
	a =left.s._11 * right.s._11 +left.s._21 * right.s._21 +left.s._31 * right.s._31;
	b =left.s._11 * right.s._12 +left.s._21 * right.s._22 +left.s._31 * right.s._32;
	c =left.s._11 * right.s._13 +left.s._21 * right.s._23 +left.s._31 * right.s._33;

	d =left.s._12 * right.s._11 +left.s._22 * right.s._21 +left.s._32 * right.s._31;
	e =left.s._12 * right.s._12 +left.s._22 * right.s._22 +left.s._32 * right.s._32;
	f =left.s._12 * right.s._13 +left.s._22 * right.s._23 +left.s._32 * right.s._33;

	g =left.s._13 * right.s._11 +left.s._23 * right.s._21 +left.s._33 * right.s._31;
	h =left.s._13 * right.s._12 +left.s._23 * right.s._22 +left.s._33 * right.s._32;
	i =left.s._13 * right.s._13 +left.s._23 * right.s._23 +left.s._33 * right.s._33;

	s._11 = a;
	s._12 = b;
	s._13 = c;

	s._21 = d;
	s._22 = e;
	s._23 = f;

	s._31 = g;
	s._32 = h;
	s._33 = i;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setMultiplyTransposeRight(const PxMat33Legacy left, const PxMat33Legacy right)
{
	PxReal a,b,c, d,e,f, g,h,i;
	//note: temps needed so that x.multiply(x,y) works OK.
	a =left.s._11 * right.s._11 +left.s._12 * right.s._12 +left.s._13 * right.s._13;
	b =left.s._11 * right.s._21 +left.s._12 * right.s._22 +left.s._13 * right.s._23;
	c =left.s._11 * right.s._31 +left.s._12 * right.s._32 +left.s._13 * right.s._33;

	d =left.s._21 * right.s._11 +left.s._22 * right.s._12 +left.s._23 * right.s._13;
	e =left.s._21 * right.s._21 +left.s._22 * right.s._22 +left.s._23 * right.s._23;
	f =left.s._21 * right.s._31 +left.s._22 * right.s._32 +left.s._23 * right.s._33;

	g =left.s._31 * right.s._11 +left.s._32 * right.s._12 +left.s._33 * right.s._13;
	h =left.s._31 * right.s._21 +left.s._32 * right.s._22 +left.s._33 * right.s._23;
	i =left.s._31 * right.s._31 +left.s._32 * right.s._32 +left.s._33 * right.s._33;

	s._11 = a;
	s._12 = b;
	s._13 = c;

	s._21 = d;
	s._22 = e;
	s._23 = f;

	s._31 = g;
	s._32 = h;
	s._33 = i;
}


PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setMultiplyTransposeRight(const PxVec3& left, const PxVec3& right)
{
	s._11 = left.x * right.x;
	s._12 = left.x * right.y;
	s._13 = left.x * right.z;

	s._21 = left.y * right.x;
	s._22 = left.y * right.y;
	s._23 = left.y * right.z;

	s._31 = left.z * right.x;
	s._32 = left.z * right.y;
	s._33 = left.z * right.z;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRotX(PxReal angle)
{
	PxReal Cos = cosf(angle);
	PxReal Sin = sinf(angle);
	setIdentity();
	m[1][1] = m[2][2] = Cos;
	m[1][2] = -Sin;
	m[2][1] = Sin;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRotY(PxReal angle)
{
	PxReal Cos = cosf(angle);
	PxReal Sin = sinf(angle);
	setIdentity();
	m[0][0] = m[2][2] = Cos;
	m[0][2] = Sin;
	m[2][0] = -Sin;
}

PX_CUDA_CALLABLE PX_INLINE void PxMat33Legacy::setRotZ(PxReal angle)
{
	PxReal Cos = cosf(angle);
	PxReal Sin = sinf(angle);
	setIdentity();
	m[0][0] = m[1][1] = Cos;
	m[0][1] = -Sin;
	m[1][0] = Sin;
}

PX_CUDA_CALLABLE PX_INLINE PxVec3  PxMat33Legacy::operator%(const PxVec3& src) const
{
	PxVec3 dest;
	this->multiplyByTranspose(src, dest);
	return dest;
}


PX_CUDA_CALLABLE PX_INLINE PxVec3  PxMat33Legacy::operator*(const PxVec3& src) const
{
	PxVec3 dest;
	this->multiply(src, dest);
	return dest;
}


PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &PxMat33Legacy::operator +=(const PxMat33Legacy& d)
{
	s._11 += d.s._11;
	s._12 += d.s._12;
	s._13 += d.s._13;

	s._21 += d.s._21;
	s._22 += d.s._22;
	s._23 += d.s._23;

	s._31 += d.s._31;
	s._32 += d.s._32;
	s._33 += d.s._33;
	return *this;
}


PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &PxMat33Legacy::operator -=(const PxMat33Legacy& d)
{
	s._11 -= d.s._11;
	s._12 -= d.s._12;
	s._13 -= d.s._13;

	s._21 -= d.s._21;
	s._22 -= d.s._22;
	s._23 -= d.s._23;

	s._31 -= d.s._31;
	s._32 -= d.s._32;
	s._33 -= d.s._33;
	return *this;
}


PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &PxMat33Legacy::operator *=(PxReal f)
{
	s._11 *= f;
	s._12 *= f;
	s._13 *= f;

	s._21 *= f;
	s._22 *= f;
	s._23 *= f;

	s._31 *= f;
	s._32 *= f;
	s._33 *= f;
	return *this;
}


PX_CUDA_CALLABLE PX_INLINE const PxMat33Legacy &PxMat33Legacy::operator /=(PxReal x)
{
	PxReal f = PxReal(1) / x;
	s._11 *= f;
	s._12 *= f;
	s._13 *= f;

	s._21 *= f;
	s._22 *= f;
	s._23 *= f;

	s._31 *= f;
	s._32 *= f;
	s._33 *= f;
	return *this;
}


PX_CUDA_CALLABLE PX_INLINE PxReal PxMat33Legacy::determinant() const
{
	return  s._11*s._22*s._33 + s._12*s._23*s._31 + s._13*s._21*s._32 
		- s._13*s._22*s._31 - s._12*s._21*s._33 - s._11*s._23*s._32;
}


bool PxMat33Legacy::getInverse(PxMat33Legacy& dest) const
{
	PxReal b00,b01,b02,b10,b11,b12,b20,b21,b22;

	b00 = s._22*s._33-s._23*s._32;	b01 = s._13*s._32-s._12*s._33;	b02 = s._12*s._23-s._13*s._22;
	b10 = s._23*s._31-s._21*s._33;	b11 = s._11*s._33-s._13*s._31;	b12 = s._13*s._21-s._11*s._23;
	b20 = s._21*s._32-s._22*s._31;	b21 = s._12*s._31-s._11*s._32;	b22 = s._11*s._22-s._12*s._21;



	/*
	compute determinant: 
	PxReal d =   a00*a11*a22 + a01*a12*a20 + a02*a10*a21	- a02*a11*a20 - a01*a10*a22 - a00*a12*a21;
	0				1			2			3				4			5

	this is a subset of the multiplies done above:

	PxReal d = b00*a00				+		b01*a10						 + b02 * a20;
	PxReal d = (a11*a22-a12*a21)*a00 +		(a02*a21-a01*a22)*a10		 + (a01*a12-a02*a11) * a20;

	PxReal d = a11*a22*a00-a12*a21*a00 +		a02*a21*a10-a01*a22*a10		 + a01*a12*a20-a02*a11*a20;
	0			5					2			4					1			3
	*/

	PxReal d = b00*s._11		+		b01*s._21				 + b02 * s._31;

	if (d == PxReal(0))		//singular?
	{
		dest.setIdentity();
		return false;
	}

	d = PxReal(1)/d;

	//only do assignment at the end, in case dest == this:


	dest.s._11 = b00*d; dest.s._12 = b01*d; dest.s._13 = b02*d;
	dest.s._21 = b10*d; dest.s._22 = b11*d; dest.s._23 = b12*d;
	dest.s._31 = b20*d; dest.s._32 = b21*d; dest.s._33 = b22*d;

	return true;
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy&	PxMat33Legacy::operator*= (const PxMat33Legacy& mat)
{
	this->setMultiply(*this, mat);
	return *this;
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	PxMat33Legacy::operator-  (const PxMat33Legacy& mat)	const
{
	PxMat33Legacy temp;
	temp.setSubtract(*this, mat);
	return temp;
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	PxMat33Legacy::operator+  (const PxMat33Legacy& mat)	const
{
	PxMat33Legacy temp;
	temp.setAdd(*this, mat);
	return temp;
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	PxMat33Legacy::operator*  (const PxMat33Legacy& mat)	const
{
	PxMat33Legacy temp;
	temp.setMultiply(*this, mat);
	return temp;
}


PX_CUDA_CALLABLE PX_INLINE PxMat33Legacy	PxMat33Legacy::operator*  (float s)			const
{
	PxMat33Legacy temp;
	temp.setMultiply(s, *this);
	return temp;
}

PX_POP_PACK

} // end namespace general_shared3
using namespace general_shared3;
} // end namespace physx

/** @} */
#endif
