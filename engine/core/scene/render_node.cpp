#include "render_node.h"
#include "node_tree.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	i32	Render::m_renderTypes = Render::Type_2D | Render::Type_Ui;

	Render::Render()
		: m_isVisible(true)
	{
	}

	Render::~Render()
	{
		if (m_bvhNodeId != -1)
		{
			Bvh& bvh = m_renderType.getIdx() == 1 ? NodeTree::instance()->get3dBvh() : NodeTree::instance()->get2dBvh();
			//bvh.destroyProxy(m_bvhNodeId);
		}
	}

	void Render::bindMethods()
	{
		CLASS_BIND_METHOD(Render, setRenderType);
		CLASS_BIND_METHOD(Render, getRenderType);
		CLASS_BIND_METHOD(Render, setVisible);
		CLASS_BIND_METHOD(Render, isVisible);

		CLASS_REGISTER_PROPERTY(Render, "RenderType", Variant::Type::StringOption, getRenderType, setRenderType);
		CLASS_REGISTER_PROPERTY(Render, "Visible", Variant::Type::Bool, isVisible, setVisible);
	}

	bool Render::isNeedRender() const
	{
#ifdef ECHO_EDITOR_MODE
		if (!Engine::instance()->getConfig().m_isGame)
		{
			if (m_renderType.getIdx()==0)			return (m_renderTypes & Type_2D) != 0 && m_isVisible;
			else if (m_renderType.getIdx() == 1)	return (m_renderTypes & Type_3D) != 0 && m_isVisible;
			else									return (m_renderTypes & Type_Ui) != 0 && m_isVisible;
		}
		else
		{
			return m_isVisible;
		}
#else
		return m_isVisible;
#endif
	}

	void Render::setRenderType(const StringOption& type)
	{
		if (m_bvhNodeId != -1)
		{
			Bvh& bvh = m_renderType.getIdx() == 1 ? NodeTree::instance()->get3dBvh() : NodeTree::instance()->get2dBvh();
			bvh.destroyProxy(m_bvhNodeId);

			m_bvhNodeId = -1;
		}

		m_renderType.setValue(type.getValue());
	}

	Camera* Render::getCamera()
	{
		Camera* camera = m_renderType.getIdx() == 0 ? NodeTree::instance()->get2dCamera() : (m_renderType.getIdx() == 1 ? NodeTree::instance()->get3dCamera() : NodeTree::instance()->getUiCamera());
		return camera;
	}

	void Render::update(float delta, bool bUpdateChildren)
	{
		if (!m_isEnable)
			return;

		Node::update(delta, bUpdateChildren);

		if (m_bvhNodeId == -1)
		{
			AABB worldAABB = getLocalAABB();
			if (worldAABB.isValid())
			{
				worldAABB = worldAABB.transform(getWorldMatrix());
				Bvh& bvh = m_renderType.getIdx() == 1 ? NodeTree::instance()->get3dBvh() : NodeTree::instance()->get2dBvh();
				
				m_bvhNodeId = bvh.createProxy(worldAABB, getId());
			}
		}
	}

	void* Render::getGlobalUniformValue(const String& name)
	{
		if (name == "u_WorldMatrix")
			return (void*)(&m_matWorld);

		if (name == "u_Time")
			return (void*)FrameState::instance()->getCurrentTimeSecondsPtr();

		Camera* camera = getCamera();
		if (camera)
		{
			if (name == "u_ViewProjMatrix")
				return (void*)(&camera->getViewProjMatrix());
			else if (name == "u_CameraPosition")
				return (void*)(&camera->getPosition());
			else if (name == "u_CameraDirection")
				return (void*)(&camera->getDirection());
			else if (name == "u_CameraNear")
				return (void*)(&camera->getNear());
			else if (name == "u_CameraFar")
				return (void*)(&camera->getFar());
		}

		return nullptr;
	}
}
