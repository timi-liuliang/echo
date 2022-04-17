#pragma once

#include <engine/core/math/Math.h>
#include "engine/core/geom/AABB.h"

namespace Echo
{
	class ShadowCamera
	{
	public:
		ShadowCamera();
		~ShadowCamera();


		void update(const AABB* visibleActorsAABB);

		void setLightDir(const Vector3& dir);


		const Matrix4& getViewProjMatrix() const { return m_viewProj; }

		const Matrix4& getViewMatrix() const { return m_view; }


		bool isEnable() { return m_enable; }


		AABB& getBox() { return m_Box; }

		const AABB& getCalcBox() { return m_CalcBox; }

	private:
		void calcOrthoRH(Matrix4& oOrth, const AABB& box, const Matrix4& viewMat);

	private:
		bool		m_enable;
		Matrix4		m_viewProj;
		Matrix4		m_view;
		Vector3     m_dir;
		AABB 		m_Box;
		AABB		m_CalcBox;
	};
}