////////////////////////////////////////////////////////////////////////////////
//	四元数
//	2010.12.01 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace scl {

class matrix;
class vector3;
class vector4;

//xi + yj + zk + w
class quaternion
{
public:
	float x;
	float y;
	float z;
	float w;

	void	set					(float x, float y, float z, float w) { this->x = x; this->y = y; this->z = z; this->w = w; normalize(); }
	void	clear				();
	void	normalize			();
	void	from_pivot_radian	(const vector3& v, const float angle); //angle为弧度制
	void	from_vector4		(const vector4& v);
	void	from_euler_radian	(const float x, const float y, const float z);		//单位是弧度制
	void	from_euler_angle	(const float x, const float y, const float z);		//单位是角度制
	void	to_euler_radian		(float& _x, float& y, float& z) const;
	void	to_euler_radian		(scl::vector3& v) const;
	void	to_euler_angle		(float& _x, float& y, float& z) const;
	void	to_euler_angle		(scl::vector3&) const;
	void	to_euler_radian2	(float& _rx, float& _ry, float& _rz) const;
	void	to_euler_radian2	(scl::vector3& v) const;
	void	to_matrix			(matrix& matrix) const;
	void	to_vector4			(vector4& v) const; //!!!the quaternion must have been normalized
	void	reverse				() { x = -x; y = -y; z = -z; w = -w; }
	bool	operator==			(const quaternion& other) const;
	bool	operator!=			(const quaternion& other) const;

	static const quaternion&	zero		() { static quaternion _zero = { 0, 0, 0, 1 }; return _zero; }
	static float				dot			(const quaternion& q1, const quaternion& q2);
	static void					slerp		(const quaternion& q1, const quaternion& q2, const float t, quaternion& result);
	static void					lerp		(const quaternion& q1, const quaternion& q2, const float t, quaternion& result);
	static void					mul			(const quaternion& q1, float v);
};

//void QuaternionToAxisAngle(const quaternion q, vector4 &v);

}	//namespace scl

