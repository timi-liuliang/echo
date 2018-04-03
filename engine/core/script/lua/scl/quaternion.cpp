////////////////////////////////////////////////////////////////////////////////
//	ËÄÔªÊý
//	
//	2010.12.01 caolei
////////////////////////////////////////////////////////////////////////////////
#include "scl/math.h"

#include "scl/quaternion.h"
#include "scl/matrix.h"
#include "scl/vector.h"

#include <math.h>

namespace scl {

void quaternion::from_pivot_radian(const vector3& v, const float angle)
{
	vector3 nv = v;
	nv.normalize();
	float sina = ::sin(angle / 2);
	x = sina * nv.x;
	y = sina * nv.y;
	z = sina * nv.z;
	w = ::cos(angle / 2);
}

void quaternion::from_vector4(const vector4& v)
{
	vector3 nv = {v.x, v.y, v.z};
	nv.normalize();
	float sina = ::sin(v.d / 2);
	x = sina * nv.x;
	y = sina * nv.y;
	z = sina * nv.z;
	w = ::cos(v.d / 2);
}

void quaternion::to_matrix(matrix& matrix) const
{
	matrix.x1 = 1 - 2*(y*y + z*z);
	matrix.y1 = 2*(x*y + w*z);
	matrix.z1 = 2*(x*z - w*y);
	matrix.d1 = 0;

	matrix.x2 = 2*(x*y - w*z);
	matrix.y2 = 1 - 2*(x*x + z*z);
	matrix.z2 = 2*(y*z + w*x);
	matrix.d2 = 0;

	matrix.x3 = 2*(w*y + x*z);
	matrix.y3 = 2*(y*z - w*x);
	matrix.z3 = 1 - 2*(x*x + y*y);
	matrix.d3 = 0;

	matrix.x4 = 0;
	matrix.y4 = 0;
	matrix.z4 = 0;
	matrix.d4 = 1;
}

//!!!the quaternion must have been normalized!!!
void quaternion::to_vector4(vector4& v) const
{
	//!!!the quaternion must have been normalized!!!

	float sqrLength = x*x + y*y + z*z;
	if (sqrLength > 0) 
	{
		float invLength = 1 / ::sqrtf(sqrLength);
		v.x = x * invLength;
		v.y = y * invLength;
		v.z = z * invLength;
		if (w <= 1.0f)
		{
			v.d = 2.0f * ::acosf(w);
		}
		else
		{
			v.d = 0;
		}
	}
	else 
	{
		v.d = 0;
		v.x = 1;
		v.y = 0;
		v.z = 0;
	}
}

void quaternion::normalize()
{
	float n = ::sqrt(x*x + y*y + z*z + w*w);
	x /= n;
	y /= n;
	z /= n;
	w /= n;
}

bool quaternion::operator==( const quaternion& other ) const
{
	return scl::float_equal(x, other.x) &&
		scl::float_equal(y, other.y) &&
		scl::float_equal(z, other.z) &&
		scl::float_equal(w, other.w);
}

bool quaternion::operator!=( const quaternion& other ) const
{
	return	!scl::float_equal(x, other.x) || 
			!scl::float_equal(y, other.y) || 
			!scl::float_equal(z, other.z) || 
			!scl::float_equal(w, other.w);
}

float quaternion::dot(const quaternion& q1, const quaternion& q2)
{
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

void quaternion::lerp(const quaternion& q1, const quaternion& q2, const float t, quaternion& result)
{
	result.x = q1.x + t * (q2.x - q1.x);	
	result.y = q1.y + t * (q2.y - q1.y);	
	result.z = q1.z + t * (q2.z - q1.z);	
	result.w = q1.w + t * (q2.w - q1.w);	
}

void quaternion::slerp(const quaternion& _q0, const quaternion& _q1, const float t, quaternion& result)
{
	quaternion q0 = _q0;
	quaternion q1 = _q1;
	float cosa = quaternion::dot(q0, q1);
	if (cosa < 0)
	{
		q1.reverse();
		cosa = -cosa;
	}

	if (cosa > 0.9999f)
	{
		// angle a is nearly 0, use linear interpolation instead.
		quaternion::lerp(q0, q1, t, result);
	}
	else 
	{
		// Assume q_result = k0 * q0 + k1 * q1, so we need to caculate k0 and k1.
		float sina			= sqrtf(1 - cosa * cosa);
		float a				= atan2(sina, cosa);
		float one_div_sina	= 1 / sina;
		float k0			= sin((1 - t) * a) * one_div_sina;
		float k1			= sin(t * a) * one_div_sina;

		result.x			= k0 * q0.x + k1 * q1.x;
		result.y 			= k0 * q0.y + k1 * q1.y;
		result.z 			= k0 * q0.z + k1 * q1.z;
		result.w 			= k0 * q0.w + k1 * q1.w;
	}
}

void quaternion::clear()
{
	x = y = z = 0;
	w = 1;
}

void quaternion::from_euler_radian(const float _x, const float _y, const float _z)
{
	const float cosx = cos(_x/2);
	const float sinx = sin(_x/2);
	const float cosy = cos(_y/2);
	const float siny = sin(_y/2);
	const float cosz = cos(_z/2);
	const float sinz = sin(_z/2);

	w = cosz * cosx * cosy + sinz * sinx * siny;
	x = cosz * sinx * cosy + sinz * cosx * siny;
	y = cosz * cosx * siny - sinz * sinx * cosy;
	z = sinz * cosx * cosy - cosz * sinx * siny;
}


void quaternion::to_euler_radian(float& _x, float& _y, float& _z) const
{
	float ay	= 2 * (w*z + x*y);
	float ax	= 1 - 2 * (z*z + x*x);
	float b		= 2 * (w*x - y*z);
	float cy	= 2 * (w*y + z*x);
	float cx	= 1 - 2 * (x*x+y*y);
	
	_z = atan2(ay, ax);
	_x = asin(b);
	_y = atan2(cy, cx);
}

void quaternion::to_euler_radian(scl::vector3& v) const
{
	to_euler_radian(v.x, v.y, v.z);
}

void quaternion::from_euler_angle(const float x, const float y, const float z)
{
	from_euler_radian(radian(x), radian(y), radian(z));
}

void quaternion::to_euler_angle(float& _x, float& _y, float& _z) const
{
	to_euler_radian(_x, _y, _z);
	_x = angle(_x);
	_y = angle(_y);
	_z = angle(_z);
}

void quaternion::to_euler_angle(scl::vector3& v) const
{
	to_euler_angle(v.x, v.y, v.z);
}

}	//namespace scl

