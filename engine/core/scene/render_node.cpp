#include "render_node.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	i32	Render::m_renderTypes = Render::Type_2D;

	void Render::bindMethods()
	{
		CLASS_BIND_METHOD(Render, set2d, DEF_METHOD("set2d"));
		CLASS_BIND_METHOD(Render, is2d, DEF_METHOD("is2d"));

		CLASS_REGISTER_PROPERTY(Render, "2D", Variant::Type::Bool, "is2d", "set2d");
	}

	bool Render::isNeedRender() const
	{
#ifdef ECHO_EDITOR_MODE
		if (!Engine::instance()->getConfig().m_isGame)
		{
			if (m_is2d) return static_cast<bool>(m_renderTypes & Type_2D);
			else		return static_cast<bool>(m_renderTypes & Type_3D);
		}
		else
		{
			return true;
		}
#else
		return true;
#endif
	}
}