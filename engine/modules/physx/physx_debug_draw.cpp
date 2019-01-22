#include "physx_debug_draw.h"
#include "engine/core/util/AssertX.h"
#include "engine/core/log/Log.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/math/Math.h"
#include "physx_world.h"

namespace Echo
{
	PhysxDebugDraw::PhysxDebugDraw()
	{
		m_gizmosNode = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmosNode->set2d(false);
		m_gizmosNode->setAutoClear(true);
		m_gizmosNode->setLocalPosition(Vector3(0.f, 0.f, 0.f));
	}

	void PhysxDebugDraw::update(float elapsedTime, const physx::PxRenderBuffer& rb)
	{
		m_gizmosNode->update(elapsedTime, false);

		drawLines(rb);
		drawTriangles(rb);
	}

	void PhysxDebugDraw::drawLines(const physx::PxRenderBuffer& rb)
	{
		for (physx::PxU32 i = 0; i < rb.getNbLines(); i++)
		{
			const physx::PxDebugLine& line = rb.getLines()[i];
			m_gizmosNode->drawLine(Vector3(line.pos0.x, line.pos0.y, line.pos0.z), Vector3(line.pos1.x, line.pos1.y, line.pos1.z), Color::RED);
		}
	}

	void PhysxDebugDraw::drawTriangles(const physx::PxRenderBuffer& rb)
	{
		for (physx::PxU32 i = 0; i < rb.getNbTriangles(); i++)
		{
			const physx::PxDebugTriangle& tri = rb.getTriangles()[i];
			m_gizmosNode->drawTriangle(Vector3(tri.pos0.x, tri.pos0.y, tri.pos0.z), Vector3(tri.pos1.x, tri.pos1.y, tri.pos1.z), Vector3(tri.pos2.x, tri.pos2.y, tri.pos2.z), Color::RED);
		}
	}
}