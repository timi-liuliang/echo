#include "Transform.h"

namespace Echo
{
	Transform::Transform(const Vector3& pos, const Vector3& scale, const Quaternion& rot)
		: m_pos(pos)
		, m_scale(scale)
		, m_quat(rot)
	{

	}

	Vector3 Transform::transformVec3(const Vector3& orig) const
	{
		Vector3 result;
		result = m_quat * orig;
		result = result * m_scale;
		result = result + m_pos;

		return result;
	}

	void Transform::buildMatrix(Matrix4& mat) const
	{
		mat.makeScaling(m_scale);

		Matrix4 matRot;
		matRot.fromQuan(m_quat);
		mat = mat * matRot;

		mat.translate(m_pos);
	}

	void Transform::buildInvMatrix(Matrix4& invMat) const
	{
		invMat.makeTranslation(-m_pos);
		Matrix4 matRot;
		Quaternion rot = m_quat;
		rot.conjugate();
		matRot.fromQuan(rot);
		invMat = invMat * matRot;
		matRot.makeScaling(1.0f / m_scale.x, 1.0f / m_scale.y, 1.0f / m_scale.z);
		invMat = invMat * matRot;
	}

	Transform Transform::operator * (const Transform& b) const
	{
		Transform result;

		result.m_quat = m_quat * b.m_quat;
		result.m_scale = m_scale * b.m_scale;
		result.m_pos = m_quat * (m_scale * b.m_pos);
		result.m_pos += m_pos;

		return result;
	}

	void Transform::reset()
	{
		m_pos = Vector3::ZERO;
		m_quat = Quaternion::IDENTITY;
		m_scale = Vector3::ONE;
	}
}