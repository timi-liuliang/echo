#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/Renderable.h"
#include "../render/vertex_format.h"

namespace Echo
{
	class UiRender : public Render
	{
		ECHO_VIRTUAL_CLASS(UiRender, Render)

	public:
		UiRender();
		virtual ~UiRender();

		// alpha
		float getAlpha() const { return m_alpha; }
		void setAlpha(float alpha) { m_alpha = alpha; }

	protected:
		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

	protected:
		float					m_alpha = 1.f;
	};
}
