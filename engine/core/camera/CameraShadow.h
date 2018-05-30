#pragma once

#include <engine/core/math/Math.h>
#include "engine/core/geom/Box.h"

namespace Echo
{
	/**
	 * 阴影摄像机(Used by exponential shadow map)
	 */
	class CameraShadow
	{
	public:
		CameraShadow();
		~CameraShadow();

		// 更新
		void update(const Box* visibleActorsAABB);

		// 手动设置投影灯光朝向
		void setLightDir(const Vector3& dir);

		// 获取观察投影矩阵
		const Matrix4& getViewProjMatrix() const { return m_viewProj; }

		const Matrix4& getViewMatrix() const { return m_view; }

		// 当前帧是否可用
		bool isEnable() { return m_enable; }

		// 获取ActorsAABB
		Box& getBox() { return m_Box; }

		const Box& getCalcBox() { return m_CalcBox; }

	private:
		// 根据包围盒与观察矩阵计算正交矩阵
		void calcOrthoRH(Matrix4& oOrth, const Box& box, const Matrix4& viewMat);

	private:
		bool		m_enable;			// 当前是否可用
		Matrix4		m_viewProj;			// 观察投影矩阵
		Matrix4		m_view;
		Vector3     m_dir;				// 投影方向
		Box 		m_Box;              // ActorsAABB
		Box			m_CalcBox;			// 用于计算的AABB(在真实的AABB基础上进行扩大)
	};
}