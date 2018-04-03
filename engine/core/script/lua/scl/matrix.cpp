////////////////////////////////////////////////////////////////////////////////
//	3D matrix
//	2010.08.01 caolei
////////////////////////////////////////////////////////////////////////////////
#include "scl/matrix.h"

#include "scl/vector.h"
#include "scl/quaternion.h"

#ifdef SCL_ANDROID
#include <stdlib.h>
#else
#include <memory.h>
#endif

#include <math.h>

namespace scl {

//绕向量v旋转angle角度的矩阵
matrix& matrix::rotate_axis(const vector3& v, float angle)
{
	static matrix result;
	quaternion q;
	q.from_pivot_radian(v, angle);
	q.to_matrix(result);
	return result;
}

//绕向量v = v2 - v1 旋转angle角度的矩阵
//v1是起始点，v2是结束点
matrix& matrix::rotate_any_axis(const vector3& v1, const vector3& v2, float angle)
{
	static matrix result;

	//先平移到以v1.x v1,y v1.z为原点的坐标系
	result = matrix::move(-v1.x, -v1.y, -v1.z);

	//绕v2 - v1旋转angle角度
	vector3 temp = { vector3_dec(v2, v1) };
	vector3 vq = { temp.x, temp.y, temp.z };
	quaternion q;
	q.from_pivot_radian(vq, angle);
	matrix rotateAxis;
	q.to_matrix(rotateAxis);
	result.mul(rotateAxis);

	//平移回原来的坐标系
	result.mul(matrix::move(v1.x, v1.y, v1.z));

	return result;
}

//求从v1向量旋转到v2向量的矩阵
matrix& matrix::rotate_between(const vector3& from_v1, const vector3& to_v2)
{
	static matrix result;
	vector3 v1 = from_v1;
	vector3 v2 = to_v2;
	v1.normalize();
	v2.normalize();

	if (v1 == v2 || v1.empty() || v2.empty())
	{
		result = matrix::identity();
		return result;
	}

	//求出旋转轴
	vector3& axis = vector3::cross(v1, v2);

	//求出旋转角度
	float a = vector3::angle(v1, v2);

	//利用旋转轴pivot和旋转角度acos(cosa)计算旋转四元数
	quaternion q;
	q.from_pivot_radian(axis, a);

	//利用四元数生成旋转矩阵
	q.to_matrix(result);

	return result;
}

matrix& matrix::rotate_pivot_quaternion(const vector3& pivot, const quaternion& q)
{
	static matrix transform;
	transform = matrix::move(-pivot.x, -pivot.y, -pivot.z);
	matrix rotateMatrix;
	q.to_matrix(rotateMatrix);
	transform.mul(rotateMatrix);
	transform.mul(matrix::move(pivot.x, pivot.y, pivot.z));
	return transform;
}

void matrix::clear()
{
	memset(this, 0, sizeof(matrix));
}

matrix& matrix::identity()
{
	//这里三层外三层的括号是为了消除linux的警告！
	static matrix m = { { {

		{ 1,	0,	0,	0 },
		{ 0,	1,	0,	0 },
		{ 0,	0,	1,	0 },
		{ 0,	0,	0,	1 }

	} } };

	return m;
}


matrix& matrix::move(float dx, float dy, float dz)
{
	static matrix m;
	m.set
		(
		1,	0,	0,	0,
		0,	1,	0,	0,
		0,	0,	1,	0,
		dx,	dy,	dz,	1
		);
	return m;
}

matrix& matrix::scale(float x, float y, float z)
{
	static matrix m;
	m.set
		(
		x,	0,	0,	0,
		0,	y,	0,	0,
		0,	0,	z,	0,
		0,	0,	0,	1
		);
	return m;
}

matrix& matrix::rotate_x_radian(float a)
{
	static matrix m;
	float cosa = cos(a);
	float sina = sin(a);
	m.set
		(
		//xz为地平面，左手坐标系
		1,		0,		0,		0, 
		0,		cosa,	sina,	0,
		0,		-sina,	cosa,	0,
		0,		0,		0,		1
		);
	return m;
}

matrix& matrix::rotate_y_radian(float a)
{
	static matrix m;
	float cosa = cos(a);
	float sina = sin(a);
	m.set
		(
		cosa,	0,	-sina,	0,
		0,		1,	0,		0,
		sina,	0,	cosa,	0,
		0,		0,	0,		1
		);								
	return m;
}

matrix& matrix::rotate_z_radian(float a)
{
	static matrix m;
	float cosa = cos(a);
	float sina = sin(a);
	m.set
		(
		cosa,	sina,	0,	0,
		-sina,	cosa,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1
		);
	return m;
}

void matrix::ortho(matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ)
{
   float dx = right - left;
   float dy = top - bottom;
   float dz = farZ - nearZ;

   if (dx == 0.0f || dy == 0.0f || dz == 0.0f)
      return;

   m.set(
		2.0f / dx,				0,						0,						0,
		0,						2.0f / dy,				0, 						0,
		0,						0,						-2.0f / dz,				0,		
		-(right + left) / dx,	-(top + bottom) / dy,	-(nearZ + farZ) / dz,	1);
}

void matrix::perspective(scl::matrix& m, float fovy, float aspect, float nearZ, float farZ)
{
   float frustumH = tanf ( fovy / 360.0f * PI ) * nearZ;
   float frustumW = frustumH * aspect;
   scl::matrix::frustum(m, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}

void matrix::frustum(scl::matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ)
{
   float dx = right - left;
   float dy = top - bottom;
   float dz = farZ - nearZ;

   if (dx <= 0.0f || dy <= 0.0f || dz <= 0.0f)
      return;

   m.set(
		2.0f * nearZ / dx,		0,						0,							0,
		0,						2.0f * nearZ / dy,		0,							0,
		2 * (right + left) / dx, 2 * (top + bottom) / dy,	-(nearZ + farZ) / dz,		-1.0f,
		0,						0,						2.0f * nearZ * farZ / dz,	0);
}

void matrix::lookat(scl::matrix& result, float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ)
{
   float axisX[3], axisY[3], axisZ[3];
   float length;

   // axisZ = lookAt - pos
   axisZ[0] = lookAtX - posX;
   axisZ[1] = lookAtY - posY;
   axisZ[2] = lookAtZ - posZ;

   // normalize axisZ
   length = sqrtf ( axisZ[0] * axisZ[0] + axisZ[1] * axisZ[1] + axisZ[2] * axisZ[2] );

   if ( length != 0.0f )
   {
      axisZ[0] /= length;
      axisZ[1] /= length;
      axisZ[2] /= length;
   }

   // axisX = up X axisZ
   axisX[0] = upY * axisZ[2] - upZ * axisZ[1];
   axisX[1] = upZ * axisZ[0] - upX * axisZ[2];
   axisX[2] = upX * axisZ[1] - upY * axisZ[0];

   // normalize axisX
   length = sqrtf ( axisX[0] * axisX[0] + axisX[1] * axisX[1] + axisX[2] * axisX[2] );

   if ( length != 0.0f )
   {
      axisX[0] /= length;
      axisX[1] /= length;
      axisX[2] /= length;
   }

   // axisY = axisZ x axisX
   axisY[0] = axisZ[1] * axisX[2] - axisZ[2] * axisX[1];
   axisY[1] = axisZ[2] * axisX[0] - axisZ[0] * axisX[2];
   axisY[2] = axisZ[0] * axisX[1] - axisZ[1] * axisX[0];

   // normalize axisY
   length = sqrtf ( axisY[0] * axisY[0] + axisY[1] * axisY[1] + axisY[2] * axisY[2] );

   if ( length != 0.0f )
   {
      axisY[0] /= length;
      axisY[1] /= length;
      axisY[2] /= length;
   }

   result.clear();

   result.m[0][0] = -axisX[0];
   result.m[0][1] =  axisY[0];
   result.m[0][2] = -axisZ[0];

   result.m[1][0] = -axisX[1];
   result.m[1][1] =  axisY[1];
   result.m[1][2] = -axisZ[1];

   result.m[2][0] = -axisX[2];
   result.m[2][1] =  axisY[2];
   result.m[2][2] = -axisZ[2];

   // translate (-posX, -posY, -posZ)
   result.m[3][0] =  axisX[0] * posX + axisX[1] * posY + axisX[2] * posZ;
   result.m[3][1] = -axisY[0] * posX - axisY[1] * posY - axisY[2] * posZ;
   result.m[3][2] =  axisZ[0] * posX + axisZ[1] * posY + axisZ[2] * posZ;
   result.m[3][3] = 1.0f;
}

} //namespace scl
