#include "physx_debug_draw.h"
#include "engine/core/util/AssertX.h"
#include "engine/core/log/Log.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/math/Math.h"
#include "physx_world.h"

namespace Echo
{
	PhysxDebugDraw::PhysxDebugDraw(physx::PxScene* scene)
	{
		if (scene)
		{
			m_pxScene = scene;

			m_gizmosNode = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
			m_gizmosNode->set2d(false);
			m_gizmosNode->setAutoClear(true);
			m_gizmosNode->setLocalPosition(Vector3(0.f, 0.f, 0.f));
		}
	}

	// set enable
	void PhysxDebugDraw::setEnable(bool isEnable)
	{
		if (m_isEnable != isEnable)
		{
			m_pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, isEnable ? 1.f : 0.f);
			//m_pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES, 1.f);
			m_pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, isEnable ? 1.f : 0.f);
			m_pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_STATIC, isEnable ? 1.f : 0.f);
			m_pxScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_DYNAMIC, isEnable ? 1.f : 0.f);

			m_isEnable = isEnable;
		}
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
			Vector3 pos0(line.pos0.x, line.pos0.y, line.pos0.z);
			Vector3 pos1(line.pos1.x, line.pos1.y, line.pos1.z);
			Color   color(line.color0);
			m_gizmosNode->drawLine(pos0, pos1, color);
		}
	}

	void PhysxDebugDraw::drawTriangles(const physx::PxRenderBuffer& rb)
	{
		for (physx::PxU32 i = 0; i < rb.getNbTriangles(); i++)
		{
			const physx::PxDebugTriangle& tri = rb.getTriangles()[i];
			Vector3 pos0(tri.pos0.x, tri.pos0.y, tri.pos0.z);
			Vector3 pos1(tri.pos1.x, tri.pos1.y, tri.pos1.z);
			Vector3 pos2(tri.pos2.x, tri.pos2.y, tri.pos2.z);
			Color   color(tri.color0);
			m_gizmosNode->drawTriangle( pos0, pos1, pos2, color);
		}
	}
}