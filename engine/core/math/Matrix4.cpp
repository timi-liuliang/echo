#include "Matrix4.h"
#include "Quaternion.h"

namespace Echo
{
	// predefined specific matrices
	const Matrix4 Matrix4::D3D2GL_PROJ(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 2, 0,
		0, 0, -1, 1);

	const Matrix4 Matrix4::ZERO(
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0);

	const Matrix4 Matrix4::INVALID(
		Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, 
		Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, 
		Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, 
		Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL);

	const Matrix4 Matrix4::IDENTITY(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	ui32  Matrix4::OP_COUNT = 0;

	//////////////////////////////////////////////////////////////////////////
	void Matrix4::fromQuan(const Quaternion& quan)
	{
		quan.toMat4(*this);
	}

	void Matrix4::rotateAxis(const Vector3& v, Real radian)
	{
		ADD_MATH_OP_COUNT
		Real x = v.x;
		Real y = v.y;
		Real z = v.z;

		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		Matrix4  local, final;

		local.m00 = ( x * x ) * ( 1.0f - fCos ) + fCos;
		local.m01 = ( x * y ) * ( 1.0f - fCos ) + (z * fSin);
		local.m02 = ( x * z ) * ( 1.0f - fCos ) - (y * fSin);
		local.m03 = 0.0f;

		local.m10 = ( y * x ) * ( 1.0f - fCos ) - (z * fSin);
		local.m11 = ( y * y ) * ( 1.0f - fCos ) + fCos;
		local.m12 = ( y * z ) * ( 1.0f - fCos ) + (x * fSin);
		local.m13 = 0.0f;

		local.m20 = ( z * x ) * ( 1.0f - fCos ) + (y * fSin);
		local.m21 = ( z * y ) * ( 1.0f - fCos ) - (x * fSin);
		local.m22 = ( z * z ) * ( 1.0f - fCos ) + fCos;
		local.m23 = 0.0f;

		local.m30 = 0.0f;
		local.m31 = 0.0f;
		local.m32 = 0.0f;
		local.m33 = 1.0f;

		final = local * (*this);
		*this = final;
	}

	void Matrix4::rotateAxisReplace(const Vector3& axis, Real radian)
	{
		ADD_MATH_OP_COUNT
		Real x = axis.x;
		Real y = axis.y;
		Real z = axis.z;

		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		m00 = ( x * x ) * ( 1.0f - fCos ) + fCos;
		m01 = ( x * y ) * ( 1.0f - fCos ) + (z * fSin);
		m02 = ( x * z ) * ( 1.0f - fCos ) - (y * fSin);
		m03 = 0.0f;

		m10 = ( y * x ) * ( 1.0f - fCos ) - (z * fSin);
		m11 = ( y * y ) * ( 1.0f - fCos ) + fCos;
		m12 = ( y * z ) * ( 1.0f - fCos ) + (x * fSin);
		m13 = 0.0f;

		m20 = ( z * x ) * ( 1.0f - fCos ) + (y * fSin);
		m21 = ( z * y ) * ( 1.0f - fCos ) - (x * fSin);
		m22 = ( z * z ) * ( 1.0f - fCos ) + fCos;
		m23 = 0.0f;

		m30 = 0.0f;
		m31 = 0.0f;
		m32 = 0.0f;
		m33 = 1.0f;
	}

	// this = Rx * this
	void Matrix4::rotateX(const Real radian)
	{
		ADD_MATH_OP_COUNT
		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		Real temp10 = m10 * fCos + m20 * fSin;
		Real temp11 = m11 * fCos + m21 * fSin;
		Real temp12 = m12 * fCos + m22 * fSin;
		Real temp13 = m13 * fCos + m23 * fSin;

		Real temp20 = m10 *-fSin + m20 * fCos;
		Real temp21 = m11 *-fSin + m21 * fCos;
		Real temp22 = m12 *-fSin + m22 * fCos;
		Real temp23 = m13 *-fSin + m23 * fCos;

		m10 = temp10;
		m11 = temp11;
		m12 = temp12;
		m13 = temp13;
		m20 = temp20;
		m21 = temp21;
		m22 = temp22;
		m23 = temp23;
	}

	void Matrix4::rotateXReplace(const Real radian)
	{
		ADD_MATH_OP_COUNT
		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		m00 = 1; m01 = 0; m02 = 0; m03 = 0;
		m10 = 0; m11 = fCos; m12 = fSin; m13 = 0;
		m20 = 0; m21 = -fSin; m22 = fCos; m23 = 0;
		m30 = 0; m31 = 0; m32 = 0; m33 = 1;
	}

	// this = Ry * this
	void Matrix4::rotateY(const Real radian)
	{
		ADD_MATH_OP_COUNT
		Real temp00, temp01, temp02, temp03;
		Real temp20, temp21, temp22, temp23;
		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		temp00 = m00 * fCos - m20 * fSin;
		temp01 = m01 * fCos - m21 * fSin;
		temp02 = m02 * fCos - m22 * fSin;
		temp03 = m03 * fCos - m23 * fSin;

		temp20 = m00 * fSin + m20 * fCos;
		temp21 = m01 * fSin + m21 * fCos;
		temp22 = m02 * fSin + m22 * fCos;
		temp23 = m03 * fSin + m23 * fCos;

		m00 = temp00;
		m01 = temp01;
		m02 = temp02;
		m03 = temp03;
		m20 = temp20;
		m21 = temp21;
		m22 = temp22;
		m23 = temp23;
	}

	void Matrix4::rotateYReplace(const Real radian)
	{
		ADD_MATH_OP_COUNT
		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		m00 = fCos; m01 = 0; m02 = -fSin; m03 = 0;
		m10 = 0; m11 = 1; m12 = 0; m13 = 0;
		m20 = -fSin; m21 = 0; m22 = fCos; m23 = 0;
		m30 = 0; m31 = 0; m32 = 0; m33 = 1;
	}

	// this = Rx * this
	void Matrix4::rotateZ(const Real radian)
	{
		ADD_MATH_OP_COUNT
		Real temp00, temp01, temp02, temp03;
		Real temp10, temp11, temp12, temp13;
		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		temp00 = m00 * fCos + m10 * fSin;
		temp01 = m01 * fCos + m11 * fSin;
		temp02 = m02 * fCos + m12 * fSin;
		temp03 = m03 * fCos + m13 * fSin;

		temp10 = m00 *-fSin + m10 * fCos;
		temp11 = m01 *-fSin + m11 * fCos;
		temp12 = m02 *-fSin + m12 * fCos;
		temp13 = m03 *-fSin + m13 * fCos;

		m00 = temp00;
		m01 = temp01;
		m02 = temp02;
		m03 = temp03;
		m10 = temp10;
		m11 = temp11;
		m12 = temp12;
		m13 = temp13;
	}

	void Matrix4::rotateZReplace(const Real radian)
	{
		ADD_MATH_OP_COUNT
		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);

		m00 = fCos; m01 = fSin; m02 = 0; m03 = 0;
		m10 = -fSin; m11 = fCos; m12 = 0; m13 = 0;
		m20 = 0; m21 = 0; m22 = 1; m23 = 0;
		m30 = 0; m31 = 0; m32 = 0; m33 = 1;
	}

	Vector3 Matrix4::rotateVec3(const Vector3& vec)
	{
		ADD_MATH_OP_COUNT
		Vector3 result;

		result.x = vec.x*m00 + vec.y*m10 + vec.z*m20;
		result.y = vec.x*m01 + vec.y*m11 + vec.z*m21;
		result.z = vec.x*m02 + vec.y*m12 + vec.z*m22;

		return result;
	}

	Vector4 Matrix4::rotateVec4(const Vector4& vec)
	{
		ADD_MATH_OP_COUNT
		Vector4 result;

		result.x = vec.x*m00 + vec.y*m10 + vec.z*m20;
		result.y = vec.x*m01 + vec.y*m11 + vec.z*m21;
		result.z = vec.x*m02 + vec.y*m12 + vec.z*m22;
		result.w = 1.0;

		return result;
	}
/*
	Matrix4& Matrix4::detInverse()
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

		Real sdet = (t00 * _m00 + t10 * _m01 + t20 * m02 + t30 * m03);
		if(Math::Abs(sdet) < 0.000001f)
			return *this;

		Real detInv = 1.0f / sdet;

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
	}*/

	void Matrix4::Transpose(Matrix4& outMat, const Matrix4& matrix)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		outMat = matrix;
		outMat.transpose();
	}

	void Matrix4::TransformVec3(Vector3& outVec, const Vector3& v, const Matrix4& matrix)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		Real x = v.x * matrix.m00 + v.y * matrix.m10 + v.z * matrix.m20 + matrix.m30;
		Real y = v.x * matrix.m01 + v.y * matrix.m11 + v.z * matrix.m21 + matrix.m31;
		Real z = v.x * matrix.m02 + v.y * matrix.m12 + v.z * matrix.m22 + matrix.m32;
		outVec.set(x, y, z);
	}

	void Matrix4::TransformVec4(Vector4& outVec, const Vector4& v, const Matrix4& matrix)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		Real x = v.x * matrix.m00 + v.y * matrix.m10 + v.z * matrix.m20 + v.w * matrix.m30;
		Real y = v.x * matrix.m01 + v.y * matrix.m11 + v.z * matrix.m21 + v.w * matrix.m31;
		Real z = v.x * matrix.m02 + v.y * matrix.m12 + v.z * matrix.m22 + v.w * matrix.m32;
		Real w = v.x * matrix.m03 + v.y * matrix.m13 + v.z * matrix.m23 + v.w * matrix.m33;
		outVec.set(x, y, z, w);
	}

	void Matrix4::Inverse(Matrix4& outMat, const Matrix4& matrix)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		outMat = matrix;
		outMat.inverse();
	}

	void Matrix4::RotateAxis(Matrix4& outMat, const Vector3& axis, const Real radian)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		Real fSin, fCos;
		fSin = Math::Sin(radian);
		fCos = Math::Cos(radian);
		Real fOneMinusCos = 1.0f - fCos;

		Vector3 v_Mul_Sin = axis * fSin;
		Vector3 multipler = fOneMinusCos * axis;

		outMat.m00 = axis.x * multipler.x + fCos;
		outMat.m01 = axis.x * multipler.y + v_Mul_Sin.z;
		outMat.m02 = axis.x * multipler.z + -v_Mul_Sin.y;
		outMat.m03 = 0;

		outMat.m10 = axis.y * multipler.x + -v_Mul_Sin.z;
		outMat.m11 = axis.y * multipler.y + fCos;
		outMat.m12 = axis.y * multipler.z + v_Mul_Sin.x;
		outMat.m13 = 0;

		outMat.m20 = axis.y * multipler.x + v_Mul_Sin.y;
		outMat.m21 = axis.y * multipler.y + -v_Mul_Sin.x;
		outMat.m22 = axis.y * multipler.z + fCos;
		outMat.m23 = 0;

		outMat.m30 = 0;
		outMat.m31 = 0;
		outMat.m32 = 0;
		outMat.m33 = 1;
	}

	void Matrix4::RotateYawPitchRoll(Matrix4& outMat, Real yaw, Real pitch, Real roll)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		outMat.identity();
		outMat.rotateZ(roll);
		outMat.rotateX(pitch);
		outMat.rotateY(yaw);
	}

	void Matrix4::LookAtRH(Matrix4& mat, const Vector3& eye, const Vector3& at, const Vector3& up)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		Vector3 upNorm;
		Vector3::Normalize(upNorm, up);

		Vector3 zAxis = eye - at;
		zAxis.normalize();

		Vector3 xAxis;
		Vector3::Cross(xAxis, upNorm, zAxis);
		xAxis.normalize();

		Vector3 yAxis;
		Vector3::Cross(yAxis, zAxis, xAxis);

		mat.m00 = xAxis.x;	mat.m01 = yAxis.x;	mat.m02 = zAxis.x;	mat.m03 = 0.0;
		mat.m10 = xAxis.y;	mat.m11 = yAxis.y;	mat.m12 = zAxis.y;	mat.m13 = 0.0;
		mat.m20 = xAxis.z;	mat.m21 = yAxis.z;	mat.m22 = zAxis.z;	mat.m23 = 0.0;
		mat.m30 = -Vector3::Dot(xAxis, eye); mat.m31 = -Vector3::Dot(yAxis, eye); mat.m32 = -Vector3::Dot(zAxis, eye); mat.m33 = 1.0;
	}

	void Matrix4::LookAtLH(Matrix4& mat, const Vector3& eye, const Vector3& at, const Vector3& up)
	{ 
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		Vector3 upNorm;
		Vector3::Normalize(upNorm, up);

		Vector3 zAxis = at - eye;
		zAxis.normalize();

		Vector3 xAxis;
		Vector3::Cross(xAxis, upNorm, zAxis);
		xAxis.normalize();

		Vector3 yAxis;
		Vector3::Cross(yAxis, zAxis, xAxis);

		mat.m00 = xAxis.x;	mat.m01 = yAxis.x;	mat.m02 = zAxis.x;	mat.m03 = 0.0;
		mat.m10 = xAxis.y;	mat.m11 = yAxis.y;	mat.m12 = zAxis.y;	mat.m13 = 0.0;
		mat.m20 = xAxis.z;	mat.m21 = yAxis.z;	mat.m22 = zAxis.z;	mat.m23 = 0.0;
		mat.m30 = -Vector3::Dot(xAxis, eye); mat.m31 = -Vector3::Dot(yAxis, eye); mat.m32 = -Vector3::Dot(zAxis, eye); mat.m33 = 1.0;
	}

	void Matrix4::OrthoRH(Matrix4& mat, Real w, Real h, Real zn, Real zf)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		Real w2 = w / 2.0f;
		Real h2 = h / 2.0f;

		OrthoOffCenterRH(mat, -w2, w2, -h2, h2, zn, zf);
	}

	void Matrix4::OrthoLH(Matrix4& mat, Real w, Real h, Real zn, Real zf)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif
		Real w2 = w / 2.0f;
		Real h2 = h / 2.0f;

		OrthoOffCenterLH(mat, -w2, w2, -h2, h2, zn, zf);
	}

	void Matrix4::OrthoOffCenterRH(Matrix4& mat, Real l, Real r, Real b, Real t, Real zn, Real zf)
	{	
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif

		Real w = 1.0f / (r - l);
		Real h = 1.0f / (t - b);
		Real q = 1.0f / (zn - zf);

		mat.m00 = w+w;		mat.m01 = 0.0;		mat.m02 = 0.0;	mat.m03 = 0.0;
		mat.m10 = 0.0;		mat.m11 = h+h;		mat.m12 = 0.0;	mat.m13 = 0.0;
		mat.m20 = 0.0;		mat.m21 = 0.0;		mat.m22 = q;	mat.m23 = 0.0;
		mat.m30 = -(l+r)*w;	mat.m31 = -(t+b)*h;	mat.m32 = zn*q;	mat.m33 = 1.0;
	}

	void Matrix4::OrthoOffCenterLH(Matrix4& mat, Real l, Real r, Real b, Real t, Real zn, Real zf)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif

		Real w = 1.0f / (r - l);
		Real h = 1.0f / (t - b);
		Real q = 1.0f / (zn - zf);

		mat.m00 = w+w;		mat.m01 = 0.0;		mat.m02 = 0.0;	mat.m03 = 0.0;
		mat.m10 = 0.0;		mat.m11 = h+h;		mat.m12 = 0.0;	mat.m13 = 0.0;
		mat.m20 = 0.0;		mat.m21 = 0.0;		mat.m22 = -q;	mat.m23 = 0.0;
		mat.m30 = -(l+r)*w;	mat.m31 = -(t+b)*h;	mat.m32 = zn*q;	mat.m33 = 1.0;
	}

 	void Matrix4::PerspectiveFovRH(Matrix4& mat, Real fovy, Real aspect, Real zn, Real zf)
 	{
 #ifdef ECHO_DEBUG
 		++Matrix4::OP_COUNT;
 #endif
 
 		Real ys = 1.0f / Math::Tan(fovy * 0.5f);
 		Real xs = ys / aspect;
 		Real q = zf / (zn - zf);
 
 		mat.m00 = xs;	mat.m01 = 0.0;	mat.m02 = 0.0;	mat.m03 = 0.0;
 		mat.m10 = 0.0;	mat.m11 = ys;	mat.m12 = 0.0;	mat.m13 = 0.0;
 		mat.m20 = 0.0;	mat.m21 = 0.0;	mat.m22 = q;	mat.m23 = -1.0;
 		mat.m30 = 0.0;	mat.m31 = 0.0;	mat.m32 = zn*q;	mat.m33 = 0.0;
 	}

	// Éî¶È·¶Î§(0,1)
	void Matrix4::PerspectiveFovRH_D3D(Matrix4 &outMat, Real fovy, Real aspect, Real zn, Real zf)
	{
		EchoAssert(false);
	}

	void Matrix4::PerspectiveFovRH_OpenGL(Matrix4 &mat, Real fovy, Real aspect, Real zn, Real zf)
 	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif

		Real ys = 1.0f / Math::Tan(fovy * 0.5f);
		Real xs = ys / aspect;
		Real c = -(zf+zn) / (zf-zn);
		Real d = -2*zf*zn / (zf-zn);

		mat.m00 = xs;	mat.m01 = 0.0;	mat.m02 = 0.0;	mat.m03 = 0.0;
		mat.m10 = 0.0;	mat.m11 = ys;	mat.m12 = 0.0;	mat.m13 = 0.0;
		mat.m20 = 0.0;	mat.m21 = 0.0;	mat.m22 = c;	mat.m23 = -1.0;
		mat.m30 = 0.0;	mat.m31 = 0.0;	mat.m32 = d;	mat.m33 = 0.0;
	}

	void Matrix4::PerspectiveFovLH(Matrix4& mat, Real fovy, Real aspect, Real zn, Real zf)
	{
#ifdef ECHO_DEBUG
	++Matrix4::OP_COUNT;
#endif

		Real ys = 1.0f / Math::Tan(fovy * 0.5f);
		Real xs = ys / aspect;
		Real q = zf / (zn - zf);

		mat.m00 = xs;	mat.m01 = 0.0;	mat.m02 = 0.0;	mat.m03 = 0.0;
		mat.m10 = 0.0;	mat.m11 = ys;	mat.m12 = 0.0;	mat.m13 = 0.0;
		mat.m20 = 0.0;	mat.m21 = 0.0;	mat.m22 = -q;	mat.m23 = 1.0;
		mat.m30 = 0.0;	mat.m31 = 0.0;	mat.m32 = zn*q;	mat.m33 = 0.0;
	}

	void Matrix4::PerspectiveOffCenterRH(Matrix4& mat, Real l, Real r, Real b, Real t, Real zn, Real zf)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif

		Real w = 1.0f / (r - l);
		Real h = 1.0f / (t - b);
		Real q = zf / (zn - zf);

		mat.m00 = 2.0f*zn*w;	mat.m01 = 0.0;			mat.m02 = 0.0;		mat.m03 = 0.0;
		mat.m10 = 0.0;			mat.m11 = 2.0f*zn*h;	mat.m12 = 0.0;		mat.m13 = 0.0;
		mat.m20 = (l+r)*w;		mat.m21 = (t+b)*h;		mat.m22 = q;		mat.m23 = -1.0;
		mat.m30 = 0.0;			mat.m31 = 0.0;			mat.m32 = zn*q;		mat.m33 = 0.0;
	}

	void Matrix4::PerspectiveOffCenterLH(Matrix4& mat, Real l, Real r, Real b, Real t, Real zn, Real zf)
	{
#ifdef ECHO_DEBUG
		++Matrix4::OP_COUNT;
#endif

		Real w = 1.0f / (r - l);
		Real h = 1.0f / (t - b);
		Real q = zf / (zn - zf);

		mat.m00 = 2.0f*zn*w;	mat.m01 = 0.0;			mat.m02 = 0.0;		mat.m03 = 0.0;
		mat.m10 = 0.0;			mat.m11 = 2.0f*zn*h;	mat.m12 = 0.0;		mat.m13 = 0.0;
		mat.m20 = -(l+r)*w;		mat.m21 = -(t+b)*h;		mat.m22 = -q;		mat.m23 = 1.0;
		mat.m30 = 0.0;			mat.m31 = 0.0;			mat.m32 = zn*q;		mat.m33 = 0.0;
	}
}