#include <thread>
#include "physx_world.h"
#include "physx_cb.cx"
#include "physx_module.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	PhysxWorld::PhysxWorld()
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
		physx::PxCloseVehicleSDK();

		m_pxScene->release();
		m_pxPhysics->release();
		m_pxCPUDispatcher->release();
		m_pxFoundation->release();

		EchoSafeDelete(m_pxAllocatorCb, PxAllocatorCallback);
		EchoSafeDelete(m_pxErrorCb, PxErrorCallback);
		EchoSafeDelete(m_debugDraw, PhysxDebugDraw);
	}

	bool PhysxWorld::initPhysx()
	{
		m_pxAllocatorCb = EchoNew(PhysxAllocatorCb);
		m_pxErrorCb = EchoNew(PhysxErrorReportCb);
		m_pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *m_pxAllocatorCb, *m_pxErrorCb);

		bool isRecordMemoryAllocations = false;
		m_pxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *m_pxFoundation, physx::PxTolerancesScale(), isRecordMemoryAllocations, nullptr);

		// vehicle
		PxInitVehicleSDK(*m_pxPhysics, nullptr);
		PxVehicleSetBasisVectors((const physx::PxVec3&)Vector3::UNIT_Y, (const physx::PxVec3&)Vector3::UNIT_X);
		PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::eVELOCITY_CHANGE);

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
			bool isGame = Engine::instance()->getConfig().m_isGame;

			// step
			m_accumulator += elapsedTime;
			while (m_accumulator > m_stepLength)
			{
				m_pxScene->simulate(isGame ? m_stepLength : 0.f);
				m_pxScene->fetchResults(true);

				m_accumulator -= m_stepLength;
			}

			// draw debug data
            const StringOption& debugDrawOption = PhysxModule::instance()->getDebugDrawOption();
			if (debugDrawOption.getIdx() == 3 || (debugDrawOption.getIdx() == 1 && !isGame) || (debugDrawOption.getIdx() == 2 && isGame))
			{
                m_debugDraw->setEnable(true);
                
				const physx::PxRenderBuffer& rb = m_pxScene->getRenderBuffer();
				m_debugDraw->update(elapsedTime, rb);
			}
            else
            {
                m_debugDraw->setEnable(false);
            }
		}
	}
}
