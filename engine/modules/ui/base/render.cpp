#include "render.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader/shader_program.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	UiRender::UiRender()
	{
		setRenderType("ui");
	}

	UiRender::~UiRender()
	{
	}

	void UiRender::bindMethods()
	{
		CLASS_BIND_METHOD(UiRender, getAlpha, DEF_METHOD("getAlpha"));
		CLASS_BIND_METHOD(UiRender, setAlpha, DEF_METHOD("setAlpha"));

		CLASS_REGISTER_PROPERTY(UiRender, "Alpha", Variant::Type::Real, "getAlpha", "setAlpha");
	}

	void* UiRender::getGlobalUniformValue(const String& name)
	{
		void* value = Render::getGlobalUniformValue(name);
		if (value)
			return value;

		if (name == "u_Alpha")
			return (void*)(&m_alpha);

		return nullptr;
	}
}
