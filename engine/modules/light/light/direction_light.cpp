#include "direction_light.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
	DirectionLight::DirectionLight()
		: Light(Light::Direction)
	{
	}

	DirectionLight::~DirectionLight()
	{

	}

	void DirectionLight::bindMethods()
	{
		CLASS_BIND_METHOD(DirectionLight, isCastShadow);
		CLASS_BIND_METHOD(DirectionLight, setCastShadow);

		CLASS_REGISTER_PROPERTY(DirectionLight, "CastShadow", Variant::Type::Bool, isCastShadow, setCastShadow);
	}

	const Vector3 DirectionLight::getDirection() const
	{
		return getWorldOrientation().rotateVec3(-Vector3::UNIT_Z);
	}

	Frustum* DirectionLight::getFrustum() 
	{
		if (m_castShadow)
		{
			if (!m_shadowCamera)
				m_shadowCamera = EchoNew(ShadowCamera);

			return m_shadowCamera->getFrustum();
		}

		return nullptr; 
	}

	void DirectionLight::setCastShadow(bool castShadow)
	{
		m_castShadow = castShadow;

		if (!m_castShadow && m_shadowCamera)
		{
			EchoSafeDelete(m_shadowCamera, ShadowCamera);
		}
	}

	void DirectionLight::updateInternal(float elapsedTime)
	{
		Node::updateInternal(elapsedTime);

		if (m_shadowCamera)
		{
			//Vector3 position = NodeTree::instance()->get3dCamera()->getPosition();
			//AABB aabb = NodeTree::instance()->get3dCamera()->getFrustum().getAABB();
			Vector3 dir = getDirection();

			AABB aabb(- Vector3(5, 20, 5), Vector3(5, 20, 5));

			m_shadowCamera->setDirection(dir);
			m_shadowCamera->update(&aabb);
		}
	}
}
