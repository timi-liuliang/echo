#pragma once

#include <PxPhysicsAPI.h>

namespace Echo
{
	class PhysxDebugDraw
	{
	public:
		PhysxDebugDraw();

		// void step
		void update(float elapsedTime, const physx::PxRenderBuffer& rb);

	private:
		// draw all lines
		void drawLines(const physx::PxRenderBuffer& rb);

		// draw triangles
		void drawTriangles(const physx::PxRenderBuffer& rb);

	private:
		class Gizmos*			m_gizmosNode;		// used for render, update by this b2Draw
	};
}