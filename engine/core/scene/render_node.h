#pragma once

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

		static void setRenderTypes(i32 types = Type_2D | Type_3D | Type_Ui) { m_renderTypes = types; }
		bool isNeedRender() const;

		// render type
		const StringOption& getRenderType() { return m_renderType; }
		void setRenderType(const StringOption& type);

		void setVisible(bool isVisible) { m_isVisible = isVisible; }
		bool isVisible() const { return m_isVisible; }

		virtual void update(float delta, bool bUpdateChildren) override;

	public:
		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

	protected:
		static i32		m_renderTypes;
		StringOption	m_renderType = StringOption("2d", { "2d", "3d", "ui"});
		bool			m_isVisible;
		Matrix4			m_matWVP;
	};
}
