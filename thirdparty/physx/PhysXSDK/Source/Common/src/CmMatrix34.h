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


#ifndef PX_PHYSICS_COMMON_MATRIX34
#define PX_PHYSICS_COMMON_MATRIX34

#include "CmPhysXCommon.h"
#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"
#include "foundation/PxMat33.h"

namespace physx
{
namespace Cm
{

/*!
Basic mathematical 3x4 matrix

Some clarifications, as there have been much confusion about matrix formats etc in the past.

Short:
- Matrix have base vectors in columns (vectors are column matrices, 3x1 matrices).
- Matrix is physically stored in column major format
- Matrices are concaternated from left

Long:
Given four base vectors a, b, c and d the matrix is stored as
         
/a.x b.x c.x d.x\
|a.y b.y c.y d.y|
\a.z b.z c.z d.z/

Vectors are treated as columns, so the vector v is 

/x\
|y|
\z/

And matrices are applied _before_ the vector (pre-multiplication)
v' = M*v

Depending on if it is treated as a vector or point the result will be a
bit different.

Vector:

/x'\   /a.x b.x c.x d.x\   /x\   /a.x*x + b.x*y + c.x*z + 0\
|y'| = |a.y b.y c.y d.y| * |y| = |a.y*x + b.y*y + c.y*z + 0|
\z'/   \a.z b.z c.z d.z/   |z|   \a.z*x + b.z*y + c.z*z + 0/
                           \0/

Point:

/x'\   /a.x b.x c.x d.x\   /x\   /a.x*x + b.x*y + c.x*z + d.x\
|y'| = |a.y b.y c.y d.y| * |y| = |a.y*x + b.y*y + c.y*z + d.y|
\z'/   \a.z b.z c.z d.z/   |z|   \a.z*x + b.z*y + c.z*z + d.z/
                           \1/


Physical storage and indexing:
To be compatible with popular 3d rendering APIs (read D3d and OpenGL)
the physical indexing is 

/0 3 6  9\
|1 4 7 10|
\2 5 8 11/

index = column*3 + row

which in C++ translates to M[column][row]

The mathematical indexing is M_row,column and this is what is used for _-notation 
so _12 is 1st row, second column and operator(row, column)!

*/
class Matrix34
{
public:
	//! Default constructor
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34()
	{}

	//! Construct from four base vectors
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34(const PxVec3& b0, const PxVec3& b1, const PxVec3& b2, const PxVec3& b3)
		: base0(b0), base1(b1), base2(b2), base3(b3)
	{}

	//! Construct from float[12]
	explicit PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34(PxReal values[]):
		base0(values[0], values[1], values[2]),
		base1(values[3], values[4], values[5]),
		base2(values[6], values[7], values[8]),
		base3(values[9], values[10], values[11])
	{		
	}

	//! Construct from a 3x3 matrix
	explicit PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34(const PxMat33& other)
		: base0(other.column0), base1(other.column1), base2(other.column2), base3(PxVec3(0))
	{
	}

	//! Construct from a 3x3 matrix and a translation vector
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34(const PxMat33& other, const PxVec3& t)
		: base0(other.column0), base1(other.column1), base2(other.column2), base3(t)
	{}

	//! Construct from a PxTransform
	explicit PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34(const PxTransform& other)
	{
		set(other.q);
		base3 = other.p;
	}

	//! Construct from a quaternion
	explicit PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34(const PxQuat& q)
	{
		set(q);
	}

	//! Copy constructor
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34(const Matrix34& other)
	{
		base0 = other.base0;
		base1 = other.base1;
		base2 = other.base2;
		base3 = other.base3;
	}

	//! Assignment operator
	PX_CUDA_CALLABLE PX_FORCE_INLINE const Matrix34& operator=(const Matrix34& other)
	{
		base0 = other.base0;
		base1 = other.base1;
		base2 = other.base2;
		base3 = other.base3;

		return *this;
	}

	//! Set to identity matrix
	PX_CUDA_CALLABLE PX_FORCE_INLINE void setIdentity()
	{
		base0[0] = base1[1] = base2[2] = 1.0f;

		base0[1] = base0[2] = 0.0f;
		base1[0] = base1[2] = 0.0f;
		base2[0] = base2[1] = 0.0f;
		base3[0] = base3[1] = base3[2] = 0.0f;
	}

	//! Check for identity matrix
	PX_CUDA_CALLABLE PX_FORCE_INLINE bool isIdentity() const
	{
		if(base0[0] != 1.0f || base1[0] != 0.0f || base2[0] != 0.0f || base3[0] != 0.0f) return false;
		if(base0[1] != 0.0f || base1[1] != 1.0f || base2[1] != 0.0f || base3[1] != 0.0f) return false;
		if(base0[2] != 0.0f || base1[2] != 0.0f || base2[2] != 1.0f || base3[2] != 0.0f) return false;

		return true;
	}

	//! Set to zero matrix
	PX_CUDA_CALLABLE PX_FORCE_INLINE void setZero()
	{
		base0[0] = base0[1] = base0[2] = 0.0f;
		base1[0] = base1[1] = base1[2] = 0.0f;
		base2[0] = base2[1] = base2[2] = 0.0f;
		base3[0] = base3[1] = base3[2] = 0.0f;
	}

	//! Check for zero matrix
	PX_CUDA_CALLABLE PX_FORCE_INLINE bool isZero() const
	{
		if(base0[0] != 0.0f || base0[1] != 0.0f || base0[2] != 0.0f || base3[0] != 0.0f) return false;
		if(base1[0] != 0.0f || base1[1] != 0.0f || base1[2] != 0.0f || base3[1] != 0.0f) return false;
		if(base2[0] != 0.0f || base2[1] != 0.0f || base2[2] != 0.0f || base3[2] != 0.0f) return false;

		return true;
	}

	
	// Simpler operators
	//! Equality operator
	PX_CUDA_CALLABLE PX_FORCE_INLINE bool operator==(const Matrix34& other) const
	{
		return (base0 == other.base0 &&
				base1 == other.base1 &&
				base2 == other.base2 &&
				base3 == other.base3);
	}

	//! Inequality operator
	PX_CUDA_CALLABLE PX_FORCE_INLINE bool operator!=(const Matrix34& other) const
	{
		return (base0 != other.base0 ||
				base1 != other.base1 ||
				base2 != other.base2 ||
				base3 != other.base3);
	}

	//! Unary minus
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34 operator-() const
	{
		return Matrix34(-base0, -base1, -base2, -base3);
	}

	//! Add
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34 operator+(const Matrix34& other) const
	{
		return Matrix34( base0+other.base0,
							base1+other.base1,
							base2+other.base2,
							base3+other.base3);
	}

	//! Subtract
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34 operator-(const Matrix34& other) const
	{
		return Matrix34( base0-other.base0,
							base1-other.base1,
							base2-other.base2,
							base3-other.base3);
	}

	//! Scalar multiplication
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34 operator*(PxReal scalar) const
	{
		return Matrix34(base0*scalar, base1*scalar, base2*scalar, base3*scalar);
	}

	friend Matrix34 operator*(PxReal, const Matrix34&);

	//! Matrix multiplication
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34 operator*(const Matrix34& other) const
	{
		//Rows from this <dot> columns from other
		//base0 = rotate(other.base0) etc
		return Matrix34(rotate(other.base0), rotate(other.base1), rotate(other.base2),
			rotate(other.base3)+base3);
	}

	//! Matrix multiplication, extend the second matrix
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34 operator*(const PxMat33& other) const
	{
		//Rows from this <dot> columns from other
		//base0 = transform(other.base0) etc
		return Matrix34(rotate(other.column0), rotate(other.column1), rotate(other.column2), base3);
	}

	friend Matrix34 operator*(const PxMat33& a, const Matrix34& b);
	
	// a <op>= b operators

	//! Equals-add
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34& operator+=(const Matrix34& other)
	{
		base0 += other.base0;
		base1 += other.base1;
		base2 += other.base2;
		base3 += other.base3;
		return *this;
	}

	//! Equals-sub
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34& operator-=(const Matrix34& other)
	{
		base0 -= other.base0;
		base1 -= other.base1;
		base2 -= other.base2;
		base3 -= other.base3;
		return *this;
	}

	//! Equals scalar multiplication
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34& operator*=(PxReal scalar)
	{
		base0 *= scalar;
		base1 *= scalar;
		base2 *= scalar;
		base3 *= scalar;
		return *this;
	}

	//! Element access, mathematical way!
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxReal operator()(PxU32 row, PxU32 col) const
	{
		return (*this)[col][row];
	}

	//! Element access, mathematical way!
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxReal& operator()(PxU32 row, PxU32 col)
	{
		return (*this)[col][row];
	}

	// Transform etc
	
	//! Transform vector by matrix, equal to v' = M*v
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 rotate(const PxVec3& other) const
	{
		return base0*other.x + base1*other.y + base2*other.z;
	}

	//! Transform vector by transpose of matrix, equal to v' = M^t*v
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 rotateTranspose(const PxVec3& other) const
	{
		return PxVec3(	base0.dot(other),
						base1.dot(other),
						base2.dot(other));
	}

	//! Transform point by matrix
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 transform(const PxVec3& other) const
	{
		return base0*other.x + base1*other.y + base2*other.z + base3;
		//return PxVec3(
		//	base0.x*other.x + base1.x*other.y + base2.x*other.z + base3.x,
		//	base0.y*other.x + base1.y*other.y + base2.y*other.z + base3.y,
		//	base0.z*other.x + base1.z*other.y + base2.z*other.z + base3.z);
	}

	//! Transform point by transposed matrix
	PX_CUDA_CALLABLE PX_FORCE_INLINE PxVec3 transformTranspose(const PxVec3& other) const
	{
		return rotateTranspose(other-base3);
	}

	//! Invert matrix treating it as a rotation+translation matrix only
	PX_CUDA_CALLABLE PX_FORCE_INLINE Matrix34 getInverseRT() const
	{
		//M  = T  * R
		//M' = R' * T'
		PxMat33 rot = PxMat33(base0, base1, base2);
		return Matrix34(rot.getTranspose(), rot.transformTranspose(-base3));
	}


	// Conversion
	//! Set matrix from quaternion
	PX_CUDA_CALLABLE PX_FORCE_INLINE void set(const PxQuat& q) 
	{
		// Convert from quaternion

		//		base0 = q.getBasisVector0();
		//		base1 = q.getBasisVector1();
		//		base2 = q.getBasisVector2();
		// PT: the above simple implementation is very slow. Don't use.

		const PxReal xs = q.x + q.x;	const PxReal ys = q.y + q.y;	const PxReal zs = q.z + q.z;

		const PxReal wx = q.w * xs;		const PxReal wy = q.w * ys;		const PxReal wz = q.w * zs;
		const PxReal xx = q.x * xs;		const PxReal xy = q.x * ys;		const PxReal xz = q.x * zs;
		const PxReal yy = q.y * ys;		const PxReal yz = q.y * zs;		const PxReal zz = q.z * zs;

		base0.x	=	1.0f -	yy - zz;
		base0.y	= 			xy + wz;
		base0.z	= 			xz - wy;

		base1.x	= 			xy - wz;
		base1.y	= 	1.0f -	xx - zz;
		base1.z	= 			yz + wx;

		base2.x	= 			xz + wy;
		base2.y	= 			yz - wx;
		base2.z	= 	1.0f -	xx - yy;

		// Col3
		base3.x = 0.0f;
		base3.y = 0.0f;
		base3.z = 0.0f;
	}

	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor44(const PxF32 * d) 
	{
		base0.x = d[0];		base0.y = d[1];		base0.z = d[2];	
		base1.x = d[4];		base1.y = d[5];		base1.z = d[6];	
		base2.x = d[8];		base2.y = d[9];		base2.z = d[10];
		base3.x = d[12];	base3.y = d[13];	base3.z = d[14];	
	}

	PX_CUDA_CALLABLE PX_INLINE void setColumnMajor44(const PxF32 d[4][4]) 
	{
		base0.x = d[0][0];		base0.y = d[0][1];		base0.z = d[0][2];
		base1.x = d[1][0];		base1.y = d[1][1];		base1.z = d[1][2];
		base2.x = d[2][0];		base2.y = d[2][1];		base2.z = d[2][2];
		base3.x = d[3][0];		base3.y = d[3][1];		base3.z = d[3][2];
	}

	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor44(PxF32 * d) const
	{
		d[0] = base0.x;		d[1]  = base0.y;		d[2]  = base0.z;	
		d[4] = base1.x;		d[5]  = base1.y;		d[6]  = base1.z;	
		d[8] = base2.x;		d[9]  = base2.y;		d[10] = base2.z;
		d[12]= base3.x;		d[13] = base3.y;		d[14] = base3.z;

		d[3] = d[7] = d[11] = 0.0f;
		d[15] = 1.0f;
	}

	PX_CUDA_CALLABLE PX_INLINE void getColumnMajor44(PxF32 d[4][4]) const
	{

		d[0][0] = base0.x;		d[0][1] = base0.y;		d[0][2] = base0.z;
		d[1][0] = base1.x;		d[1][1] = base1.y;		d[1][2] = base1.z;
		d[2][0] = base2.x;		d[2][1] = base2.y;		d[2][2] = base2.z;
		d[3][0] = base3.x;		d[3][1] = base3.y;		d[3][2] = base3.z;

		d[0][3] = d[1][3] = d[2][3] = 0.0f;
		d[3][3] = 1.0f;
	}

	PX_CUDA_CALLABLE PX_INLINE void setRowMajor44(const PxF32 * d) 
	{
		base0.x = d[0];		base0.y = d[4];		base0.z = d[8];	
		base1.x = d[1];		base1.y = d[5];		base1.z = d[9];	
		base2.x = d[2];		base2.y = d[6];		base2.z = d[10];
		base3.x = d[3];		base3.y = d[7];		base3.z = d[11];
	}

	PX_INLINE void setRowMajor44(const PxF32 d[4][4])
	{
		base0.x = d[0][0];		base0.y = d[1][0];		base0.z = d[2][0];
		base1.x = d[0][1];		base1.y = d[1][1];		base1.z = d[2][1];
		base2.x = d[0][2];		base2.y = d[1][2];		base2.z = d[2][2];
		base3.x = d[0][3];		base3.y = d[1][3];		base3.z = d[2][3];
	}

	PX_CUDA_CALLABLE PX_INLINE void getRowMajor44(PxF32 * d) const
	{
		d[0] = base0.x;		d[4] = base0.y;		d[8]  = base0.z;	
		d[1] = base1.x;		d[5] = base1.y;		d[9]  = base1.z;	
		d[2] = base2.x;		d[6] = base2.y;		d[10] = base2.z;
		d[3] = base3.x;		d[7] = base3.y;		d[11] = base3.z;

		d[12] = d[13] = d[14] = 0.0f;
		d[15] = 1.0f;
	}

	PX_CUDA_CALLABLE PX_INLINE void getRowMajor44(PxF32 d[4][4]) const
	{
		d[0][0] = base0.x;		d[1][0] = base0.y;		d[2][0] = base0.z;
		d[0][1] = base1.x;		d[1][1] = base1.y;		d[2][1] = base1.z;
		d[0][2] = base2.x;		d[1][2] = base2.y;		d[2][2] = base2.z;
		d[0][3] = base3.x;		d[1][3] = base3.y;		d[2][3] = base3.z;

		d[3][0] = d[3][1] = d[3][2] = 0.0f;
		d[3][3] = 1.0f;
	}

	PX_CUDA_CALLABLE PX_FORCE_INLINE		PxVec3& operator[](unsigned int num)			{return (&base0)[num];}
	PX_CUDA_CALLABLE PX_FORCE_INLINE		PxVec3& operator[](int num)			{return (&base0)[num];}
	PX_CUDA_CALLABLE PX_FORCE_INLINE const	PxVec3& operator[](unsigned int num) const	{return (&base0)[num];}
	PX_CUDA_CALLABLE PX_FORCE_INLINE const	PxVec3& operator[](int num) const	{return (&base0)[num];}

	//Data, see above for format!

	PxVec3 base0, base1, base2, base3; //the four base vectors

};


//! Multiply a*b, a is extended
PX_INLINE Matrix34 operator*(const PxMat33& a, const Matrix34& b)
{
	return Matrix34(a.transform(b.base0), a.transform(b.base1), a.transform(b.base2), a.transform(b.base3));
}


} // namespace Cm

}

#endif
