#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/proxy/render_proxy.h"
#include <queue>

namespace Echo
{
	class OpenDriveDynamicMesh : public Render
	{
		ECHO_CLASS(OpenDriveDynamicMesh, Render)

	public:
		struct VertexFormat
		{
			Vector3		m_position;
			Vector3		m_normal;
			Vector2		m_uv;

			VertexFormat() 
			{}
		};
		typedef vector<VertexFormat>::type	VertexArray;
		typedef vector<ui32>::type	IndiceArray;

		struct ControlPoint
		{
			Vector3 m_position;
			Vector3 m_forward;
			Vector3	m_up;
			float	m_width;
			float	m_length = 0.f;
			Color	m_color;
			bool	m_separator = false;
		};

	public:
		OpenDriveDynamicMesh();
		virtual ~OpenDriveDynamicMesh();

		// reset 
		void reset();

		// add
		void add(const Vector3& position, const Vector3& forward, const Vector3& up, float width, const Color& color, bool separator = false);

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
		virtual void updateInternal(float elapsedTime) override;

		// update vertex buffer
		void updateMeshBuffer();

		// get global uniform value
		virtual void* getGlobalUniformValue(const String& name) override;

	private:
		bool						m_isRenderableDirty = true;
		vector<ControlPoint>::type	m_controlPoints;
		float						m_stepLength = 3.f;
		Vector2						m_uvScale = Vector2(1.f, 1.f);
		MeshPtr						m_mesh;
		MaterialPtr					m_material;
		RenderProxyPtr				m_renderable;
	};
}
