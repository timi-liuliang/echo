#pragma once

#include <engine/core/base/object.h>
#include <engine/core/math/Math.h>
#include "physx_debug_draw.h"
#include "physx_base.h"

namespace Echo
{
	class PhysxWorld : public Object
	{
		ECHO_SINGLETON_CLASS(PhysxWorld, Object)

	public:
        virtual ~PhysxWorld();
        
		// instance
		static PhysxWorld* instance();

		// step
		void step(float elapsedTime);

		// get pxPhysics
		physx::PxPhysics* getPxPhysics() { return m_pxPhysics; }

		// get scene
		physx::PxScene* getPxScene() { return m_pxScene; }

	public:
		// gravity
		const Vector3& getGravity() const { return m_gravity; }
		void setGravity(const Vector3& gravity);

		// shift origin
		const Vector3& getShift() const { return m_shift; }
		void setShift(const Vector3& shift);

	private:
		PhysxWorld();

		// initialize
		bool initPhysx();

	private:
		physx::PxAllocatorCallback*		m_pxAllocatorCb = nullptr;
		physx::PxErrorCallback*			m_pxErrorCb = nullptr;
		physx::PxFoundation*			m_pxFoundation = nullptr;
		physx::PxPhysics*				m_pxPhysics = nullptr;
		physx::PxDefaultCpuDispatcher*	m_pxCPUDispatcher = nullptr;

		Vector3							m_gravity = Vector3(0.f, -9.8f, 0.f);
		Vector3							m_shift = Vector3::ZERO;
		physx::PxScene*					m_pxScene = nullptr;
		float							m_stepLength = 0.025f;
		float							m_accumulator = 0.f;

		PhysxDebugDraw*					m_debugDraw = nullptr;
	};
}
