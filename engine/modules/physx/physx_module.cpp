#include "physx_module.h"
#include <thread>
#include "pvd/PxPvdTransport.h"
#include "cmd/pvd_command.h"
#include "physx_cb.cx"
#include "shape/physx_shape.h"
#include "shape/physx_shape_sphere.h"
#include "shape/physx_shape_box.h"
#include "shape/physx_shape_capsule.h"
#include "shape/physx_shape_plane.h"
#include "shape/physx_shape_heightfield.h"
#include "shape/physx_shape_mesh.h"
#include "physx_body.h"
#include "physx_character_controller.h"
#include "editor/physx_body_editor.h"
#include "editor/physx_shape_sphere_editor.h"
#include "editor/physx_shape_box_editor.h"
#include "editor/physx_shape_capsule_editor.h"
#include "editor/physx_shape_plane_editor.h"
#include "editor/physx_shape_heightfield_editor.h"
#include "editor/physx_shape_mesh_editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	DECLARE_MODULE(PhysxModule)

	PhysxModule::PhysxModule()
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
			m_pxControllerManager = PxCreateControllerManager(*m_pxScene);

			if (physx::PxPvdSceneClient* pvdClient = m_pxScene->getScenePvdClient())
			{
				pvdClient->setScenePvdFlags(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS | physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS | physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES);
			}
		}
	}
    
    PhysxModule::~PhysxModule()
    {
		physx::PxCloseVehicleSDK();

		if (m_pxScene) m_pxScene->release();
		if (m_pxPhysics) m_pxPhysics->release();
		if (m_pxPvd->getTransport()) m_pxPvd->getTransport()->release();
		if (m_pxPvd) m_pxPvd->release();

		if (m_pxCPUDispatcher) m_pxCPUDispatcher->release();
		if (m_pxFoundation) m_pxFoundation->release();

		EchoSafeDelete(m_pxAllocatorCb, PxAllocatorCallback);
		EchoSafeDelete(m_pxErrorCb, PxErrorCallback);
		EchoSafeDelete(m_debugDraw, PhysxDebugDraw);
    }

	PhysxModule* PhysxModule::instance()
	{
		static PhysxModule* inst = EchoNew(PhysxModule);
		return inst;
	}

	void PhysxModule::bindMethods()
	{
        CLASS_BIND_METHOD(PhysxModule, getDebugDrawOption, DEF_METHOD("getDebugDrawOption"));
        CLASS_BIND_METHOD(PhysxModule, setDebugDrawOption, DEF_METHOD("setDebugDrawOption"));
		CLASS_BIND_METHOD(PhysxModule, getGravity, DEF_METHOD("getGravity"));
		CLASS_BIND_METHOD(PhysxModule, setGravity, DEF_METHOD("setGravity"));
		CLASS_BIND_METHOD(PhysxModule, getShift, DEF_METHOD("getShift"));
		CLASS_BIND_METHOD(PhysxModule, setShift, DEF_METHOD("setShift"));

        CLASS_REGISTER_PROPERTY(PhysxModule, "DebugDraw", Variant::Type::StringOption, "getDebugDrawOption", "setDebugDrawOption");
		CLASS_REGISTER_PROPERTY(PhysxModule, "Gravity", Variant::Type::Vector3, "getGravity", "setGravity");
		CLASS_REGISTER_PROPERTY(PhysxModule, "Shift", Variant::Type::Vector3, "getShift", "setShift");
	}

	void PhysxModule::setGravity(const Vector3& gravity)
	{
		m_gravity = gravity;
		if (m_pxScene)
		{
			m_pxScene->setGravity(physx::PxVec3(m_gravity.x, m_gravity.y, m_gravity.z));
		}
	}

	void PhysxModule::setShift(const Vector3& shift)
	{
		if (m_pxScene)
		{
			Vector3 offset = shift - m_shift;
			m_pxScene->shiftOrigin(physx::PxVec3(offset.x, offset.y, offset.z));

			m_shift = shift;
		}
	}

	void PhysxModule::registerTypes()
	{
		Class::registerType<PvdCommand>();
		Class::registerType<PhysxBody>();
		Class::registerType<PhysxCharacterController>();
		Class::registerType<PhysxShape>();
		Class::registerType<PhysxShapeSphere>();
		Class::registerType<PhysxShapeBox>();
		Class::registerType<PhysxShapeCapsule>();
		Class::registerType<PhysxShapePlane>();
		Class::registerType<PhysxShapeHeightfield>();
		Class::registerType<PhysxShapeMesh>();

		CLASS_REGISTER_EDITOR(PhysxBody, PhysxBodyEditor)
		CLASS_REGISTER_EDITOR(PhysxShapeBox, PhysxShapeBoxEditor)
		CLASS_REGISTER_EDITOR(PhysxShapeSphere, PhysxShapeSphereEditor)
		CLASS_REGISTER_EDITOR(PhysxShapeCapsule, PhysxShapeCapsuleEditor)
		CLASS_REGISTER_EDITOR(PhysxShapePlane, PhysxShapePlaneEditor)
		CLASS_REGISTER_EDITOR(PhysxShapeHeightfield, PhysxShapeHeightfieldEditor)
		CLASS_REGISTER_EDITOR(PhysxShapeMesh, PhysxShapeMeshEditor)
	}

	void PhysxModule::update(float elapsedTime)
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

    void PhysxModule::setDebugDrawOption(const StringOption& option)
    {
        m_drawDebugOption.setValue(option.getValue());
    }

	bool PhysxModule::initPhysx()
	{
		m_pxAllocatorCb = EchoNew(PhysxAllocatorCb);
		m_pxErrorCb = EchoNew(PhysxErrorReportCb);
		m_pxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *m_pxAllocatorCb, *m_pxErrorCb);

		m_pxPvd = PxCreatePvd(*m_pxFoundation);

		bool isRecordMemoryAllocations = false;
		m_pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pxFoundation, physx::PxTolerancesScale(), isRecordMemoryAllocations, m_pxPvd);

		// vehicle
		PxInitVehicleSDK(*m_pxPhysics, nullptr);
		PxVehicleSetBasisVectors((const physx::PxVec3&)Vector3::UNIT_Y, (const physx::PxVec3&)Vector3::UNIT_X);
		PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::eVELOCITY_CHANGE);

		physx::PxTolerancesScale scale;
		m_pxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pxFoundation, physx::PxCookingParams(scale));

		return m_pxPhysics ? true : false;
	}
}
