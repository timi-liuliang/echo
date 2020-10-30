#pragma once

#include "engine/core/camera/Camera.h"
#include "node.h"

namespace Echo
{
	class Render : public Node
	{
		ECHO_VIRTUAL_CLASS(Render, Node)

	public:
		enum Type
		{
			Type_2D = 1 << 0,
			Type_3D = 1 << 1,
			Type_Ui = 1 << 2,
		};

	public:
		Render();
		virtual ~Render();

		// render types
		static void setRenderTypes(i32 types = Type_2D | Type_3D | Type_Ui) { m_renderTypes = types; }
		static i32 getRenderTypes() { return m_renderTypes; }

		// is need render by types
		bool isNeedRender() const;

		// render type
		const StringOption& getRenderType() { return m_renderType; }
		void setRenderType(const StringOption& type);

		// visible
		void setVisible(bool isVisible) { m_isVisible = isVisible; }
		bool isVisible() const { return m_isVisible; }

		// get camera
		Camera* getCamera();

		// update
		virtual void update(float delta, bool bUpdateChildren) override;

	public:
		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

	protected:
		i32				m_bvhNodeId = -1;
		static i32		m_renderTypes;
		StringOption	m_renderType = StringOption("2d", { "2d", "3d", "ui"});
		bool			m_isVisible;
	};
}
