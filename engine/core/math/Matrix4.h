#ifndef __ECHO_MAT4_H__
#define __ECHO_MAT4_H__

#include "Vector4.h"

namespace Echo
{
	class Quaternion;
	class ECHO_EXPORT_FOUNDATION Matrix4
	{
	public:
		union
		{
			struct
			{
				Real m00, m01, m02, m03;
				Real m10, m11, m12, m13;
				Real m20, m21, m22, m23;
				Real m30, m31, m32, m33;
			};

			Real m[16];
		};

		static const Matrix4 ZERO;
		static const Matrix4 IDENTITY;
		static const Matrix4 INVALID;
		static const Matrix4 D3D2GL_PROJ;
		static ui32	 OP_COUNT;

	public:
		inline Matrix4()
		{
		}

		inline Matrix4(Real f00, Real f01, Real f02, Real f03,
			Real f10, Real f11, Real f12, Real f13,
			Real f20, Real f21, Real f22, Real f23,
			Real f30, Real f31, Real f32, Real f33)
		{
			ADD_MATH_OP_COUNT
			m00 = f00; m01 = f01; m02 = f02; m03 = f03;
			m10 = f10; m11 = f11; m12 = f12; m13 = f13;
			m20 = f20; m21 = f21; m22 = f22; m23 = f23;
			m30 = f30; m31 = f31; m32 = f32; m33 = f33;
		}

		inline Matrix4(Real* arr)
		{
			ADD_MATH_OP_COUNT
			memcpy(m, arr, sizeof(Real)*16);
		}

		inline Matrix4(const Matrix4& mat)
		{
			ADD_MATH_OP_COUNT
			memcpy(m, mat.m, sizeof(Real)*16);
		}

	public:
		inline Real operator() (int i, int j) const
		{ 
			ADD_MATH_OP_COUNT
			EchoAssertX(i>=0 && i<4, "Access out of bounds");
			EchoAssertX(j>=0 && j<4, "Access out of bounds");
			return getRow(i)[j];
		}

		inline Vector4 operator[] (int row) const
		{
			ADD_MATH_OP_COUNT
			return getRow(row);
		}

		inline Matrix4& operator= (const Matrix4& rhs)
		{
			ADD_MATH_OP_COUNT
			memcpy(m, rhs.m, sizeof(Real)*16);
			return *this;
		}

		inline Matrix4& operator+= (const Real f)
		{
			ADD_MATH_OP_COUNT
			m00 += f; m01 += f; m02 += f; m03 += f;
			m10 += f; m11 += f; m12 += f; m13 += f;
			m20 += f; m21 += f; m22 += f; m23 += f;
			m30 += f; m31 += f; m32 += f; m33 += f;

			return *this;
		}

		inline Matrix4& operator+= (const Matrix4& rhs)
		{
			ADD_MATH_OP_COUNT
			m00 += rhs.m00; m01 += rhs.m01; m02 += rhs.m02; m03 += rhs.m03;
			m10 += rhs.m10; m11 += rhs.m11; m12 += rhs.m12; m13 += rhs.m13;
			m20 += rhs.m20; m21 += rhs.m21; m22 += rhs.m22; m23 += rhs.m23;
			m30 += rhs.m30; m31 += rhs.m31; m32 += rhs.m32; m33 += rhs.m33;

			return *this;
		}

		inline Matrix4& operator-= (const Real f)
		{
			ADD_MATH_OP_COUNT
			m00 -= f; m01 -= f; m02 -= f; m03 -= f;
			m10 -= f; m11 -= f; m12 -= f; m13 -= f;
			m20 -= f; m21 -= f; m22 -= f; m23 -= f;
			m30 -= f; m31 -= f; m32 -= f; m33 -= f;

			return *this;
		}

		inline Matrix4& operator-= (const Matrix4& rhs)
		{
			ADD_MATH_OP_COUNT
			m00 -= rhs.m00; m01 -= rhs.m01; m02 -= rhs.m02; m03 -= rhs.m03;
			m10 -= rhs.m10; m11 -= rhs.m11; m12 -= rhs.m12; m13 -= rhs.m13;
			m20 -= rhs.m20; m21 -= rhs.m21; m22 -= rhs.m22; m23 -= rhs.m23;
			m30 -= rhs.m30; m31 -= rhs.m31; m32 -= rhs.m32; m33 -= rhs.m33;

			return *this;
		}

		inline Matrix4& operator*= (const Real f)
		{
			ADD_MATH_OP_COUNT
			m00 *= f; m01 *= f; m02 *= f; m03 *= f;
			m10 *= f; m11 *= f; m12 *= f; m13 *= f;
			m20 *= f; m21 *= f; m22 *= f; m23 *= f;
			m30 *= f; m31 *= f; m32 *= f; m33 *= f;

			return *this;
		}

		inline Matrix4& operator *= (const Matrix4& rhs)
		{
			ADD_MATH_OP_COUNT
			Matrix4 result;

			result.m00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20 + m03 * rhs.m30;
			result.m01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21 + m03 * rhs.m31;
			result.m02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22 + m03 * rhs.m32;
			result.m03 = m00 * rhs.m03 + m01 * rhs.m13 + m02 * rhs.m23 + m03 * rhs.m33;

			result.m10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20 + m13 * rhs.m30;
			result.m11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31;
			result.m12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32;
			result.m13 = m10 * rhs.m03 + m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33;

			result.m20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20 + m23 * rhs.m30;
			result.m21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31;
			result.m22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32;
			result.m23 = m20 * rhs.m03 + m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33;

			result.m30 = m30 * rhs.m00 + m31 * rhs.m10 + m32 * rhs.m20 + m33 * rhs.m30;
			result.m31 = m30 * rhs.m01 + m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31;
			result.m32 = m30 * rhs.m02 + m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32;
			result.m33 = m30 * rhs.m03 + m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33;

			*this = result;

			return *this;
		}

		inline Matrix4& operator/= (const Real f)
		{
			ADD_MATH_OP_COUNT
			m00 /= f; m01 /= f; m02 /= f; m03 /= f;
			m10 /= f; m11 /= f; m12 /= f; m13 /= f;
			m20 /= f; m21 /= f; m22 /= f; m23 /= f;
			m30 /= f; m31 /= f; m32 /= f; m33 /= f;

			return *this;
		}

		inline Real* ptr()
		{
			ADD_MATH_OP_COUNT
			return m;
		}

		inline const Real* ptr() const
		{
			ADD_MATH_OP_COUNT
			return m;
		}

		inline bool operator== (const Matrix4& b) const
		{
			ADD_MATH_OP_COUNT
			// true if all vectors equal to each other
			bool result = m00 == b.m00 && m01 == b.m01 && m02 == b.m02 && m03 == b.m03 &&
				m10 == b.m10 && m11 == b.m11 && m12 == b.m12 && m13 == b.m13 &&
				m20 == b.m20 && m21 == b.m21 && m22 == b.m22 && m23 == b.m23 &&
				m30 == b.m30 && m31 == b.m31 && m32 == b.m32 && m33 == b.m33;
			return result;
		}

		inline bool operator!= (const Matrix4& b) const
		{
			ADD_MATH_OP_COUNT
			// true if any one vector not-equal
			bool result =	m00 != b.m00 || m01 != b.m01 || m02 != b.m02 || m03 != b.m03 ||
				m10 != b.m10 || m11 != b.m11 || m12 != b.m12 || m13 != b.m13 ||
				m20 != b.m20 || m21 != b.m21 || m22 != b.m22 || m23 != b.m23 ||
				m30 != b.m30 || m31 != b.m31 || m32 != b.m32 || m33 != b.m33;
			return result;
		}

		inline const Matrix4 operator+ (const Real f) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 result = *this;
			result += f;
			return result;
		}

		inline friend Matrix4 operator+ (const Real f, const Matrix4 &a)
		{
			ADD_MATH_OP_COUNT
			Matrix4 result = a;
			result += f;
			return result;
		}

		inline Matrix4 operator + (const Matrix4& b) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 result;

			result.m00 = m00 + b.m00;
			result.m01 = m01 + b.m01;
			result.m02 = m02 + b.m02;
			result.m03 = m03 + b.m03;

			result.m10 = m10 + b.m10;
			result.m11 = m11 + b.m11;
			result.m12 = m12 + b.m12;
			result.m13 = m13 + b.m13;

			result.m20 = m20 + b.m20;
			result.m21 = m21 + b.m21;
			result.m22 = m22 + b.m22;
			result.m23 = m23 + b.m23;

			result.m30 = m30 + b.m30;
			result.m31 = m31 + b.m31;
			result.m32 = m32 + b.m32;
			result.m33 = m33 + b.m33;

			return result;
		}

		inline Matrix4 operator- (const Real f) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 result = *this;
			result -= f;
			return result;
		}

		inline friend Matrix4 operator- (const Real f, const Matrix4&a)
		{
			ADD_MATH_OP_COUNT
			Matrix4 result = a;
			result -= f;
			return result;
		}

		inline Matrix4 operator - (const Matrix4& b) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 result;

			result.m00 = m00 - b.m00;
			result.m01 = m01 - b.m01;
			result.m02 = m02 - b.m02;
			result.m03 = m03 - b.m03;

			result.m10 = m10 - b.m10;
			result.m11 = m11 - b.m11;
			result.m12 = m12 - b.m12;
			result.m13 = m13 - b.m13;

			result.m20 = m20 - b.m20;
			result.m21 = m21 - b.m21;
			result.m22 = m22 - b.m22;
			result.m23 = m23 - b.m23;

			result.m30 = m30 - b.m30;
			result.m31 = m31 - b.m31;
			result.m32 = m32 - b.m32;
			result.m33 = m33 - b.m33;

			return result;
		}

		inline friend const Vector4 operator * (const Vector4& v, const Matrix4& m)
		{
			ADD_MATH_OP_COUNT
			Vector4 result;

			result.x = v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30;
			result.y = v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31;
			result.z = v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32;
			result.w = v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33;

			return result;
		}

		inline friend Vector3 operator* (const Vector3& v, const Matrix4& m)
		{
			ADD_MATH_OP_COUNT
			Vector3 result;

			result.x = v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + m.m30;
			result.y = v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + m.m31;
			result.z = v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + m.m32;

			return result;
		}

		inline Matrix4 operator* (const Matrix4& b) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 result;

			result.m00 = m00 * b.m00 + m01 * b.m10 + m02 * b.m20 + m03 * b.m30;
			result.m01 = m00 * b.m01 + m01 * b.m11 + m02 * b.m21 + m03 * b.m31;
			result.m02 = m00 * b.m02 + m01 * b.m12 + m02 * b.m22 + m03 * b.m32;
			result.m03 = m00 * b.m03 + m01 * b.m13 + m02 * b.m23 + m03 * b.m33;

			result.m10 = m10 * b.m00 + m11 * b.m10 + m12 * b.m20 + m13 * b.m30;
			result.m11 = m10 * b.m01 + m11 * b.m11 + m12 * b.m21 + m13 * b.m31;
			result.m12 = m10 * b.m02 + m11 * b.m12 + m12 * b.m22 + m13 * b.m32;
			result.m13 = m10 * b.m03 + m11 * b.m13 + m12 * b.m23 + m13 * b.m33;

			result.m20 = m20 * b.m00 + m21 * b.m10 + m22 * b.m20 + m23 * b.m30;
			result.m21 = m20 * b.m01 + m21 * b.m11 + m22 * b.m21 + m23 * b.m31;
			result.m22 = m20 * b.m02 + m21 * b.m12 + m22 * b.m22 + m23 * b.m32;
			result.m23 = m20 * b.m03 + m21 * b.m13 + m22 * b.m23 + m23 * b.m33;

			result.m30 = m30 * b.m00 + m31 * b.m10 + m32 * b.m20 + m33 * b.m30;
			result.m31 = m30 * b.m01 + m31 * b.m11 + m32 * b.m21 + m33 * b.m31;
			result.m32 = m30 * b.m02 + m31 * b.m12 + m32 * b.m22 + m33 * b.m32;
			result.m33 = m30 * b.m03 + m31 * b.m13 + m32 * b.m23 + m33 * b.m33;

			return result;
		}

		inline Matrix4 operator * (const Real f) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 result;

			result.m00 = m00 * f;
			result.m01 = m01 * f;
			result.m02 = m02 * f;
			result.m03 = m03 * f;

			result.m10 = m10 * f;
			result.m11 = m11 * f;
			result.m12 = m12 * f;
			result.m13 = m13 * f;

			result.m20 = m20 * f;
			result.m21 = m21 * f;
			result.m22 = m22 * f;
			result.m23 = m23 * f;

			result.m30 = m30 * f;
			result.m31 = m31 * f;
			result.m32 = m32 * f;
			result.m33 = m33 * f;

			return result;
		}

		inline friend const Matrix4 operator* (const Real f, const Matrix4& a)
		{
			ADD_MATH_OP_COUNT
			Matrix4 result;

			result.m00 = f * a.m00;
			result.m01 = f * a.m01;
			result.m02 = f * a.m02;
			result.m03 = f * a.m03;

			result.m10 = f * a.m10;
			result.m11 = f * a.m11;
			result.m12 = f * a.m12;
			result.m13 = f * a.m13;

			result.m20 = f * a.m20;
			result.m21 = f * a.m21;
			result.m22 = f * a.m22;
			result.m23 = f * a.m23;

			result.m30 = f * a.m30;
			result.m31 = f * a.m31;
			result.m32 = f * a.m32;
			result.m33 = f * a.m33;

			return result;
		}

		inline Matrix4 operator / (const Real f) const
		{
			ADD_MATH_OP_COUNT

			return ( 1.f/f) * (*this);
		}

		inline void zero()
		{
			ADD_MATH_OP_COUNT
			memset(m, 0, sizeof(Real)*16);
		}

		inline void invalid()
		{
			ADD_MATH_OP_COUNT
			*this = INVALID;
		}

		inline void setRow(int row, Vector4& vec)
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(row >= 0 && row < 4, "Access out of bounds");
			Real* p = m + row;
			p[0] = vec.x; 
			p[1] = vec.y;
			p[2] = vec.z;
			p[3] = vec.w;
		}

		inline void setColumn(int column, Vector4& vec)
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(column >= 0 && column < 4, "Access out of bounds");
			Real* p = m + column;
			p[0] = vec.x; 
			p[4] = vec.y; 
			p[8] = vec.z; 
			p[12] = vec.w; 
		}

		inline Vector4 getRow(int row) const
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(row >= 0 && row < 4, "Access out of bounds");
			const Real* p = m + row*4;
			//return Vector4(p[0], p[1], p[2], p[3]);
			return *(Vector4*)(p);
		}

		inline Vector4 getColumn(int column) const
		{
			ADD_MATH_OP_COUNT
			EchoAssertX(column >= 0 && column < 4, "Access out of bounds");
			const Real *p = m + column;
			return Vector4(p[0], p[4], p[8], p[12]);
		}

		inline void identity()
		{
			ADD_MATH_OP_COUNT
			*this = IDENTITY;
		}

		inline void translate(Real x, Real y, Real z)
		{
			ADD_MATH_OP_COUNT
			m30 += x;
			m31 += y;
			m32 += z;
		}

		inline void translate(const Vector3& v)
		{
			ADD_MATH_OP_COUNT
			m30 += v.x;
			m31 += v.y;
			m32 += v.z;
		}

		inline void translateReplace(Real x, Real y, Real z)
		{
			ADD_MATH_OP_COUNT
			identity();
			m30 = x;
			m31 = y;
			m32 = z;
		}

		inline void translateReplace(const Vector3& v)
		{
			ADD_MATH_OP_COUNT
			identity();
			m30 = v.x;
			m31 = v.y;
			m32 = v.z;
		}
		
		inline void translateX(Real d)
		{
			ADD_MATH_OP_COUNT
			m30 += d;
		}

		inline void translateY(Real d)
		{
			ADD_MATH_OP_COUNT
			m31 += d;
		}

		inline void translateZ(Real d)
		{
			ADD_MATH_OP_COUNT
			m32 += d;
		}

		void			rotateAxis(const Vector3 &axis, Real radian);
		void			rotateAxisReplace(const Vector3 &axis, Real radian);
		void			rotateX(const Real radian);
		void			rotateXReplace(const Real radian);
		void			rotateY(const Real radian);
		void			rotateYReplace(const Real radian);
		void			rotateZ(const Real radian);
		void			rotateZReplace(const Real radian);
		Vector3			rotateVec3(const Vector3 &vec);
		Vector4			rotateVec4(const Vector4 &vec);
		

		inline void scale(const Vector3& scaleVec)
		{
			ADD_MATH_OP_COUNT
			scale(scaleVec[0], scaleVec[1], scaleVec[2]);
		}

		inline void scale(Real x, Real y, Real z)
		{
			ADD_MATH_OP_COUNT
			m00 *= x; m01 *= x; m02 *= x; m03 *= x;
			m10 *= y; m11 *= y; m12 *= y; m13 *= y;
			m20 *= z; m21 *= z; m22 *= z; m23 *= z;
		}

		inline void scaleReplace(const Vector3& scaleVec)
		{
			ADD_MATH_OP_COUNT
			scaleReplace(scaleVec[0], scaleVec[1], scaleVec[2]);
		}

		inline void scaleReplace(Real x, Real y, Real z)
		{
			ADD_MATH_OP_COUNT
			m00 = x; m01 = 0; m02 = 0; m03 = 0;
			m10 = 0; m11 = y; m12 = 0; m13 = 0;
			m20 = 0; m21 = 0; m22 = z; m23 = 0;
			m30 = 0; m31 = 0; m32 = 0; m33 = 1;
		}

		// assume input vec.w equals 1
		// result.w is garbage
		inline Vector4 transform(const Vector4& vec) const
		{
			ADD_MATH_OP_COUNT
			return vec * (*this);
		}

		inline Vector3 transform(const Vector3& vec) const
		{
			ADD_MATH_OP_COUNT
			Vector3 result;

			Real w = vec.x * m03 + vec.y * m13 + vec.z * m23 + m33;
			result.x = (vec.x * m00 + vec.y * m10 + vec.z * m20 + m30) / w;
			result.y = (vec.x * m01 + vec.y * m11 + vec.z * m21 + m31) / w;
			result.z = (vec.x * m02 + vec.y * m12 + vec.z * m22 + m32) / w;

			return result;
		}

		inline Vector4 transformNormal(const Vector4& vec) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 mat = *this;
			mat.noTranslate();

			return vec * mat;
		}

		inline Vector3 transformNormal(const Vector3& vec) const
		{
			ADD_MATH_OP_COUNT
			Matrix4 mat = *this;
			mat.noTranslate();
			return mat.transform(vec);
		}

		inline Matrix4& transpose()
		{
			ADD_MATH_OP_COUNT
			Math::Swap(m01, m10);
			Math::Swap(m02, m20);
			Math::Swap(m03, m30);
			Math::Swap(m12, m21);
			Math::Swap(m13, m31);
			Math::Swap(m23, m32);

			return *this;
		}

		inline Matrix4& inverse()
		{	
			ADD_MATH_OP_COUNT
			Math::Swap(m01, m10);
			Math::Swap(m02, m20);
			Math::Swap(m12, m21);

			m03 = 0;
			m13 = 0;
			m23 = 0;

			m30 = -m30;
			m31 = -m31;
			m32 = -m32;
			m33 = 1;

			return *this;
		}

		//Matrix4&			detInverse();
		inline Matrix4& detInverse()
		{	
			ADD_MATH_OP_COUNT
			Real _m00 = m00, _m01 = m01, _m02 = m02, _m03 = m03;
			Real _m10 = m10, _m11 = m11, _m12 = m12, _m13 = m13;
			Real _m20 = m20, _m21 = m21, _m22 = m22, _m23 = m23;
			Real _m30 = m30, _m31 = m31, _m32 = m32, _m33 = m33;

			Real v0 = _m20 * _m31 - _m21 * _m30;
			Real v1 = _m20 * _m32 - _m22 * _m30;
			Real v2 = _m20 * _m33 - _m23 * _m30;
			Real v3 = _m21 * _m32 - _m22 * _m31;
			Real v4 = _m21 * _m33 - _m23 * _m31;
			Real v5 = _m22 * _m33 - _m23 * _m32;

			Real t00 = + (v5 * _m11 - v4 * _m12 + v3 * _m13);
			Real t10 = - (v5 * _m10 - v2 * _m12 + v1 * _m13);
			Real t20 = + (v4 * _m10 - v2 * _m11 + v0 * _m13);
			Real t30 = - (v3 * _m10 - v1 * _m11 + v0 * _m12);
			
			Real detInv = 1.0f / (t00 * _m00 + t10 * _m01 + t20 * m02 + t30 * m03);

			m00 = t00 * detInv;
			m10 = t10 * detInv;
			m20 = t20 * detInv;
			m30 = t30 * detInv;

			m01 = - (v5 * _m01 - v4 * _m02 + v3 * _m03) * detInv;
			m11 = + (v5 * _m00 - v2 * _m02 + v1 * _m03) * detInv;
			m21 = - (v4 * _m00 - v2 * _m01 + v0 * _m03) * detInv;
			m31 = + (v3 * _m00 - v1 * _m01 + v0 * _m02) * detInv;

			v0 = _m10 * _m31 - _m11 * _m30;
			v1 = _m10 * _m32 - _m12 * _m30;
			v2 = _m10 * _m33 - _m13 * _m30;
			v3 = _m11 * _m32 - _m12 * _m31;
			v4 = _m11 * _m33 - _m13 * _m31;
			v5 = _m12 * _m33 - _m13 * _m32;

			m02 = + (v5 * _m01 - v4 * _m02 + v3 * _m03) * detInv;
			m12 = - (v5 * _m00 - v2 * _m02 + v1 * _m03) * detInv;
			m22 = + (v4 * _m00 - v2 * _m01 + v0 * _m03) * detInv;
			m32 = - (v3 * _m00 - v1 * _m01 + v0 * _m02) * detInv;

			v0 = _m21 * _m10 - _m20 * _m11;
			v1 = _m22 * _m10 - _m20 * _m12;
			v2 = _m23 * _m10 - _m20 * _m13;
			v3 = _m22 * _m11 - _m21 * _m12;
			v4 = _m23 * _m11 - _m21 * _m13;
			v5 = _m23 * _m12 - _m22 * _m13;

			m03 = - (v5 * _m01 - v4 * _m02 + v3 * _m03) * detInv;
			m13 = + (v5 * _m00 - v2 * _m02 + v1 * _m03) * detInv;
			m23 = - (v4 * _m00 - v2 * _m01 + v0 * _m03) * detInv;
			m33 = + (v3 * _m00 - v1 * _m01 + v0 * _m02) * detInv;

			return *this;
		}
		
		inline void makeTranslation(Real x, Real y, Real z)
		{
			ADD_MATH_OP_COUNT
			identity();
			m30 = x;
			m31 = y;
			m32 = z;
			m33 = 1;
		}

		inline void makeTranslation(const Vector3& vec)
		{
			ADD_MATH_OP_COUNT
			makeTranslation(vec.x, vec.y, vec.z);
		}

		inline void makeScaling(Real x, Real y, Real z)
		{
			ADD_MATH_OP_COUNT
			identity();
			m00 = x;
			m11 = y;
			m22 = z;
		}

		inline void makeScaling(const Vector3& vec)
		{
			ADD_MATH_OP_COUNT
			makeScaling(vec.x, vec.y, vec.z);
		}

		inline void noTranslate()
		{
			ADD_MATH_OP_COUNT
			m30 = 0;
			m31 = 0;
			m32 = 0;
			m33 = 1;
		}

		inline void noRotate()
		{
			ADD_MATH_OP_COUNT
			m00 = 1; m01 = 0; m02 = 0; m03 = 0;
			m10 = 1; m11 = 1; m12 = 0; m13 = 0;
			m20 = 1; m21 = 0; m22 = 1; m23 = 0;
		}

		void			fromQuan(const Quaternion &quan);

	public:
		static void		Transpose(Matrix4 &outMat, const Matrix4 &matrix);
		static void		TransformVec3(Vector3 &outVec, const Vector3 &v, const Matrix4 &matrix);
		static void		TransformVec4(Vector4 &outVec, const Vector4 &v, const Matrix4 &matrix);
		static void		TransformNormal(Vector3 &outVec, const Vector3 &v, const Matrix4 &matrix);
		static void		Inverse(Matrix4 &outMat, const Matrix4 &matrix);
		static void		RotateAxis(Matrix4 &outMat, const Vector3 &axis, const Real radian);
		static void		RotateYawPitchRoll(Matrix4 &outMat, Real yaw, Real pitch, Real roll);
		static void		LookAtRH(Matrix4 &outMat, const Vector3 &eye, const Vector3 &at, const Vector3 &up);
		static void		LookAtLH(Matrix4 &outMat, const Vector3 &eye, const Vector3 &at, const Vector3 &up);
		static void		OrthoRH(Matrix4 &outMat, Real w, Real h, Real zn, Real zf);
		static void		OrthoLH(Matrix4 &outMat, Real w, Real h, Real zn, Real zf);
		static void		OrthoOffCenterRH(Matrix4 &outMat, Real l, Real r, Real b, Real t, Real zn, Real zf);
		static void		OrthoOffCenterLH(Matrix4 &outMat, Real l, Real r, Real b, Real t, Real zn, Real zf);
		static void		PerspectiveFovRH(Matrix4 &outMat, Real fovy, Real aspect, Real zn, Real zf);

		// 深度范围(0,1)
		static void		PerspectiveFovRH_D3D(Matrix4 &outMat, Real fovy, Real aspect, Real zn, Real zf);

		// 深度范围(-1,1)
		static void		PerspectiveFovRH_OpenGL(Matrix4 &mat, Real fovy, Real aspect, Real zn, Real zf);
		static void		PerspectiveFovLH(Matrix4 &outMat, Real fovy, Real aspect, Real zn, Real zf);
		static void		PerspectiveOffCenterRH(Matrix4 &outMat, Real l, Real r, Real b, Real t, Real zn, Real zf);
		static void		PerspectiveOffCenterLH(Matrix4 &outMat, Real l, Real r, Real b, Real t, Real zn, Real zf);
	};
}

#endif