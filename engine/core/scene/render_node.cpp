#include "render_node.h"
#include "node_tree.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	i32	Render::m_renderTypes = Render::Type_2D;

	Render::Render()
		: m_is2d(true)
		, m_isVisible(true)
	{
	}

	void Render::bindMethods()
	{
		CLASS_BIND_METHOD(Render, set2d,		DEF_METHOD("set2d"));
		CLASS_BIND_METHOD(Render, is2d,			DEF_METHOD("is2d"));
		CLASS_BIND_METHOD(Render, setVisible,	DEF_METHOD("setVisible"));
		CLASS_BIND_METHOD(Render, isVisible,	DEF_METHOD("isVisible"));

		CLASS_REGISTER_PROPERTY(Render, "Is2D", Variant::Type::Bool, "is2d", "set2d");
		CLASS_REGISTER_PROPERTY(Render, "Visible", Variant::Type::Bool, "isVisible", "setVisible");
	}

	bool Render::isNeedRender() const
	{
#ifdef ECHO_EDITOR_MODE
		if (!Engine::instance()->getConfig().m_isGame)
		{
			if (m_is2d) return (m_renderTypes & Type_2D) != 0 && m_isVisible;
			else		return (m_renderTypes & Type_3D) != 0 && m_isVisible;
		}
		else
		{
			return m_isVisible;
		}
#else
		return m_isVisible;
#endif
	}

	void Render::update(float delta, bool bUpdateChildren)
	{
		if (!m_isEnable)
			return;

		Node::update(delta, bUpdateChildren);

		// update world view project matrix
		Camera* camera = is2d() ? NodeTree::instance()->get2dCamera() : NodeTree::instance()->get3dCamera();
		if (camera)
		{
			m_matWVP = getWorldMatrix() * camera->getViewProjMatrix();;
		}
	}

	// get global uniforms
	void* Render::getGlobalUniformValue(const String& name)
	{
		if (name == "u_WorldMatrix")
			return (void*)(&m_matWorld);

		Camera* camera = is2d() ? NodeTree::instance()->get2dCamera() : NodeTree::instance()->get3dCamera();
		if (camera)
		{
			if (name == "u_WorldViewProjMatrix")
				return (void*)(&m_matWVP);
			else if (name == "u_ViewProjMatrix")
				return (void*)(&camera->getViewProjMatrix());
			else if (name == "u_CameraPosition")
				return (void*)(&camera->getPosition());
			else if (name == "u_CameraDirection")
				return (void*)(&camera->getDirection());
			else if (name == "u_CameraNear")
				return (void*)(&camera->getNearClip());
			else if (name == "u_CameraFar")
				return (void*)(&camera->getFarClip());
		}

		return nullptr;
	}
}
