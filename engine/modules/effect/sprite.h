#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/Renderable.h"

namespace Echo
{
	class Sprite : public Render
	{
		ECHO_CLASS(Sprite, Render)

		struct VertexFormat
		{
			Vector3		m_position;
			Vector2		m_uv;

			VertexFormat(const Vector3& pos, const Vector2& uv)
				: m_position(pos), m_uv(uv)
			{}
		};
		typedef vector<VertexFormat>::type	VertexArray;
		typedef vector<Word>::type	IndiceArray;

	public:
		Sprite();
		virtual ~Sprite();

		// texture res path
		void setTextureRes(const ResourcePath& path);
		const ResourcePath& getTextureRes() { return m_textureRes; }

		// width
		i32 getWidth() const { return m_width; }
		void setWidth(i32 width);

		// width
		i32 getHeight() const { return m_height; }
		void setHeight(i32 height);

	protected:
		// build drawable
		void buildMaterial();

		// update
		virtual void update_self() override;

		// update vertex buffer
		void updateMeshBuffer();

		// clear
		void clear();

	private:
		ResourcePath			m_textureRes = ResourcePath("", ".png");
		Mesh*					m_mesh = nullptr;			// Geometry Data for render
        ShaderProgramPtr        m_shader;
		MaterialPtr				m_material;		            // Material Instance
		Renderable*				m_renderable = nullptr;
		Matrix4					m_matWVP;
		i32						m_width = 0;
		i32						m_height = 0;
	};
}
