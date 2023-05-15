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
		CLASS_BIND_METHOD(UiRender, getColor);
		CLASS_BIND_METHOD(UiRender, setColor);

		CLASS_REGISTER_PROPERTY(UiRender, "Color", Variant::Type::Color, getColor, setColor);
	}

	void UiRender::setColor(const Color& color)
	{
		if (m_color != color)
		{
			m_color = color;
		}
	}

	void* UiRender::getGlobalUniformValue(const String& name)
	{
		void* value = Render::getGlobalUniformValue(name);
		if (value)
			return value;

		if (name == "BaseColor")
			return (void*)(&m_color);

		return nullptr;
	}
}
