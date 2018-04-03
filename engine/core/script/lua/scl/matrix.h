////////////////////////////////////////////////////////////////////////////////
//	3D matrix
//	2010.08.01 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/math.h"

namespace scl {

class vector3;
class vector4;
class quaternion;

////////////////////////////////////
//	matrix:
//		x1 y1 z1 d1
//		x2 y2 z2 d2
//		x3 y3 z3 d3
//		x4 y4 z4 d4
////////////////////////////////////
class matrix
{
public:
	union 
	{
		//第一个括号是行，第二个括号是列，
		//例如第3行第2个 m[3 - 1][2 - 1] = y3;
		float m[4][4];
		struct
		{
			float x1; float y1; float z1; float d1;
			float x2; float y2; float z2; float d2;
			float x3; float y3; float z3; float d3;
			float x4; float y4; float z4; float d4;
		};
	};

	//matrix() { memset(this, 0, sizeof(matrix)); }
	void clear();

	inline void set(
		float _x1, float _y1, float	_z1, float _d1,
		float _x2, float _y2, float _z2, float _d2,
		float _x3, float _y3, float _z3, float _d3,
		float _x4, float _y4, float _z4, float _d4 );

	//inline void set(const float** new_m);


	//矩阵乘法
	inline void mul(const matrix& other);

	inline bool operator==(const matrix& other) const;

	////////////////////////////////////////////////////////////
	//static 函数
	////////////////////////////////////////////////////////////
	//旋转矩阵
	static matrix& rotate_x(float a) { rotate_x_radian(radian(a)); }	//角度制
	static matrix& rotate_y(float a) { rotate_y_radian(radian(a)); };	//角度制
	static matrix& rotate_z(float a) { rotate_z_radian(radian(a)); };	//角度制
	static matrix& rotate_x_radian(float r);	//弧度制
	static matrix& rotate_y_radian(float r);	//弧度制
	static matrix& rotate_z_radian(float r);	//弧度制
	
	//平移，正方向为各坐标系的正半轴
	//例如x轴，当d > 0，表示右移d个单位(x轴右侧为正方向)
	static matrix& move(float dx, float dy, float dz);
	static matrix& move_x(float d) { return move(d, 0, 0); }
	static matrix& move_y(float d) { return move(0, d, 0); }
	static matrix& move_z(float d) { return move(0, 0, d); }
	
	//缩放
	static matrix& scale(float x, float y, float z);

	//单位矩阵
	static matrix& identity();

	//绕向量v旋转angle角度的矩阵
	static matrix& rotate_axis(const vector3& v, float angle);

	//绕向量v = v2 - v1 旋转angle角度的矩阵
	//v1是起始点，v2是结束点
	static matrix& rotate_any_axis(const vector3& v1, const vector3& v2, float angle);

	//求从v1向量旋转到v2向量的矩阵
	static matrix& rotate_between(const vector3& v1, const vector3& v2);

	//以某个点为基准点，四元数做旋转参数
	static matrix& rotate_pivot_quaternion(const vector3& pivot, const quaternion& q);

	//投影矩阵
	static void ortho		(scl::matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ);
	static void perspective	(scl::matrix& m, float fovy, float aspect, float nearZ, float farZ);
	static void frustum		(scl::matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ);

	//摄像机矩阵
	static void lookat		(scl::matrix& result, float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ);
};

////////////////////////////////////
//	
//	Matrix实现	
//	
////////////////////////////////////
inline void matrix::set(
	float _x1, float _y1, float	_z1, float _d1,
	float _x2, float _y2, float _z2, float _d2,
	float _x3, float _y3, float _z3, float _d3,
	float _x4, float _y4, float _z4, float _d4 )
{
	x1 = _x1; y1 = _y1; z1 = _z1; d1 = _d1;
	x2 = _x2; y2 = _y2; z2 = _z2; d2 = _d2;
	x3 = _x3; y3 = _y3; z3 = _z3; d3 = _d3;
	x4 = _x4; y4 = _y4; z4 = _z4; d4 = _d4;
}


inline void matrix::mul(const matrix& other)
{
    matrix result;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            result.m[row][col] =
            m[row][0] * other.m[0][col]
            + m[row][1] * other.m[1][col]
            + m[row][2] * other.m[2][col]
            + m[row][3] * other.m[3][col];
        }
    }
    *this = result;
}


inline bool matrix::operator==(const matrix& other) const
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (!float_equal(m[i][j], other.m[i][j], 0.0001f))
			{
				return false;
			}
		}
	}
	return true;
}

} //namespace scl

