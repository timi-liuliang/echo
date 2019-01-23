#include <thread>
#include "physx_world.h"
#include "physx_cb.cxx"
#include "engine/core/main/Engine.h"

namespace Echo
{
	PhysxWorld::PhysxWorld()
		: m_drawDebugOption("Editor", { "None","Editor","Game","All" })
	{
		if (initPhysx())
		{
			physx::PxSceneDesc pxDesc(m_pxPhysics->getTolerancesScale());
			pxDesc.gravity = physx::PxVec3(m_gravity.x, m_gravity.y, m_gravity.z);
			if (!pxDesc.cpuDispatcher)
			{
				int threadNumber = std::min<int>(std::max<int>(1, std::thread::hardware_concurrency() - 1), 4);

				m_pxCPUDispatcher = physx::PxDefaultCpuDispatcherCreate(threadNumber);
				pxDesc.cpuDispatcher = (physx::PxDefaultCpuDispatcher*)m_pxCPUDispatcher;
			}

			if (!pxDesc.filterShader)
			{
				pxDesc.filterShader = physx::PxDefaultSimulationFilterShader;
			}

			//pxDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
			//pxDesc.simulationOrder = physx::PxSimulationOrder::eCOLLIDE_SOLVE;

			// create scene
			m_pxScene = m_pxPhysics->createScene(pxDesc);
			m_debugDraw = EchoNew(PhysxDebugDraw(m_pxScene));
		}
	}

	PhysxWorld::~PhysxWorld()
	{

	}

	bool PhysxWorld::initPhysx()
	{
		m_pxAllocatorCb = EchoNew(PhysxAllocatorCb);
		m_pxErrorCb = EchoNew(PhysxErrorReportCb);
		m_pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *m_pxAllocatorCb, *m_pxErrorCb);

		bool isRecordMemoryAllocations = false;
		m_pxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *m_pxFoundation, physx::PxTolerancesScale(), isRecordMemoryAllocations, nullptr);

		return m_pxPhysics ? true : false;
	}

	void PhysxWorld::bindMethods()
	{

	}

	PhysxWorld* PhysxWorld::instance()
	{
		static PhysxWorld* inst = EchoNew(PhysxWorld);
		return inst;
	}

	void PhysxWorld::step(float elapsedTime)
	{
		if (m_pxScene)
		{
			m_accumulator += elapsedTime;
			while (m_accumulator > m_stepLength)
			{
				m_pxScene->simulate(m_stepLength);
				m_pxScene->fetchResults(true);

				m_accumulator -= m_stepLength;
			}

			// draw debug data
			bool isGame = Engine::instance()->getConfig().m_isGame;
			if (m_drawDebugOption.getIdx() == 3 || (m_drawDebugOption.getIdx() == 1 && !isGame) || (m_drawDebugOption.getIdx() == 2 && isGame))
			{
				const physx::PxRenderBuffer& rb = m_pxScene->getRenderBuffer();
				m_debugDraw->update(elapsedTime, rb);
			}
		}
	}
}