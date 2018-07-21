#pragma once

#include "node.h"

namespace Echo
{
	class Render : public Node
	{
		ECHO_CLASS(Render, Node)

	public:
		enum Type
		{
			Type_2D = 1 << 0,
			Type_3D = 1 << 1,
		};

	public:
		static void setRenderTypes(i32 types = Type_2D | Type_3D) { m_renderTypes = types; }
		bool isNeedRender() const;

		void set2d(bool is2d) { m_is2d = is2d; }
		bool is2d() const { return m_is2d; }

		virtual void update(float delta, bool bUpdateChildren);

	public:
		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

	protected:
		static i32		m_renderTypes;
		bool			m_is2d;
		Matrix4			m_matWVP;
	};
}