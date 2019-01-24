#pragma once

#include "physx_base.h"

namespace Echo
{
	class PhysxDebugDraw
	{
	public:
		PhysxDebugDraw(physx::PxScene* scene);

		// void step
		void update(float elapsedTime, const physx::PxRenderBuffer& rb);

		// set enable
		void setEnable(bool isEnable);

	private:
		// draw all lines
		void drawLines(const physx::PxRenderBuffer& rb);

		// draw triangles
		void drawTriangles(const physx::PxRenderBuffer& rb);

	private:
		physx::PxScene*			m_pxScene = nullptr;
		bool					m_isEnable = false;
		class Gizmos*			m_gizmosNode;		// used for render, update by this b2Draw
	};
}
