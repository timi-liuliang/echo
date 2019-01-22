#pragma once

#include <engine/core/base/object.h>
#include <engine/core/math/Math.h>
#include "physx_debug_draw.h"
#include <PxPhysicsAPI.h>

namespace Echo
{
	class PhysxWorld : public Object
	{
		ECHO_SINGLETON_CLASS(PhysxWorld, Object)

	public:
		// instance
		static PhysxWorld* instance();

		// step
		void step(float elapsedTime);

	private:
		PhysxWorld();
		virtual ~PhysxWorld();

		// initialize
		bool initPhysx();

	private:
		physx::PxAllocatorCallback*		m_pxAllocatorCb = nullptr;
		physx::PxErrorCallback*			m_pxErrorCb = nullptr;
		physx::PxFoundation*			m_pxFoundation = nullptr;
		physx::PxPhysics*				m_pxPhysics = nullptr;
		physx::PxCpuDispatcher*			m_pxCPUDispatcher = nullptr;

		Vector3							m_gravity = Vector3(0.f, -9.8f, 0.f);
		physx::PxScene*					m_pxScene = nullptr;
		float							m_stepLength = 0.025f;
		float							m_accumulator = 0.f;
	};
}