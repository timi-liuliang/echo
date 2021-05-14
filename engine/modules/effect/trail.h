#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/proxy/render_proxy.h"
#include <queue>

namespace Echo
{
	class Trail : public Render
	{
		ECHO_CLASS(Trail, Render)

	public:
		struct VertexFormat
		{
			Vector3		m_position;
			Vector2		m_uv;

			VertexFormat() 
			{}

			VertexFormat(const Vector3& pos, const Vector2& uv)
				: m_position(pos), m_uv(uv)
			{}
		};
		typedef vector<VertexFormat>::type	VertexArray;
		typedef vector<Word>::type	IndiceArray;

		struct ControlPoint
		{
			Vector3 m_position;
			Vector3 m_forward;
			Vector3	m_up;
			float	m_width;
			float	m_length = 0.f;
			Color	m_color;
			bool	m_separator = false;
			float	m_life = 0.f;
		};

	public:
		Trail();
		virtual ~Trail();

		// reset 
		void reset();

		// add
		void add(const Vector3& position, const Vector3& forward, const Vector3& up, float width, const Color& color, bool separator = false);

		// time
		float getFadeTime() const { return m_fadeTime; }
		void setFadeTime(float fadeTime);

		// step length
		float getStepLength() const { return m_stepLength; }
		void setStepLength(float length);

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

	protected:
		// build drawable
		void buildRenderable();

		// update
		virtual void update_self() override;

		// update control points
		void updateControlPoints();

		// update vertex buffer
		void updateMeshBuffer();

		// get global uniform value
		virtual void* getGlobalUniformValue(const String& name) override;

	private:
		bool						m_isRenderableDirty = true;
		vector<ControlPoint>::type	m_controlPoints;
		float						m_fadeTime = 1.f;
		float						m_stepLength = 3.f;
		Vector2						m_uvScale = Vector2(1.f, 1.f);
		MeshPtr						m_mesh;
		MaterialPtr					m_material;
		RenderProxyPtr				m_renderable;
	};
}
