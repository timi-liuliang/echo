#pragma once

#include "Quaternion.h"

namespace Echo
{
	class Transform
	{
	public:
		Vector3		m_pos;
		Vector3		m_scale;
		Quaternion	m_quat;

	public:
		Transform() { reset(); }
		Transform(const Vector3& pos, const Vector3& scale = Vector3::ONE, const Quaternion& rot=Quaternion::IDENTITY);

		// transector3
		Vector3 transformVec3(const Vector3& orig) const;

		// build matrix
		void buildMatrix(Matrix4& mat) const;

		// build inverse matrix
		void buildInvMatrix(Matrix4& invMat) const;

		// operate "*"
		Transform operator* (const Transform& b) const;

		// reset
		void reset();
	};
}