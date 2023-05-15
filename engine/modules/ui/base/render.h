#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/shader/material.h"
#include "engine/core/render/base/proxy/render_proxy.h"
#include "../render/vertex_format.h"

namespace Echo
{
	class UiRender : public Render
	{
		ECHO_VIRTUAL_CLASS(UiRender, Render)

	public:
		UiRender();
		virtual ~UiRender();

		// Tint color
		const Color& getColor() const { return m_color; }
		void setColor(const Color& color);

	protected:
		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name) override;

	protected:
		Color                   m_color = Color::WHITE;
	};
}
