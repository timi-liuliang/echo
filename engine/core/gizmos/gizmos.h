#pragma once

#include "engine/core/math/Math.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/proxy/render_proxy.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
	class Gizmos : public Render
	{
		ECHO_CLASS(Gizmos, Render)

	public:
		// render flags
		enum RenderFlags
		{
			FixedPixel = 1 << 0,
		};

		// vertex format
		struct VertexFormat
		{
			Vector3		m_position;
			Dword		m_color;
			Vector2		m_uv;

			VertexFormat(const Vector3& pos, const Color& color, const Vector2& uv=Vector2::ZERO)
				: m_position(pos), m_color(color.getABGR()), m_uv(uv)
			{		
			}
		};
		typedef vector<VertexFormat>::type	VertexArray;
		typedef vector<Word>::type			IndiceArray;

		// batch
		struct Batch
		{
			Gizmos*			m_gizmos;
			VertexArray		m_vertexs;
			IndiceArray		m_indices;
			TexturePtr		m_albedo;
			MaterialPtr		m_material;
			MeshPtr			m_mesh;
			RenderProxy*		m_renderable;
			bool			m_meshDirty;

			Batch(Material* material, Gizmos* gizmos);
			~Batch();
			void update();
			void addVertex(const VertexFormat& vert);
			void addIndex(Word idx);
			void clear();
		};

	public:
		Gizmos();
		~Gizmos();

		// draw point
		void drawPoint(const Vector3& position, const Color& color, float pixels, int segments=4, int flags=RenderFlags::FixedPixel);

		// draw line
		void drawLine(const Vector3& from, const Vector3& to, const Color& color);

		// draw triangle
		void drawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color);

		// draw sprite
		void drawSprite(const Vector3& position, const Color& color, float pixels, TexturePtr texture, int flags = RenderFlags::FixedPixel);

		// auto clear
		bool isAutoClear() const { return m_autoClear; }
		void setAutoClear(bool autClear) { m_autoClear = autClear; }

		// clear mesh data
		void clear();

	protected:
		// update
		virtual void update_self() override;
        
        // get default shader
		ShaderProgramPtr initDefaultShader(bool enableAlbedoTexture);

		// get sprite batch
		Batch* getSpriteBatch(TexturePtr texture);

		// adjust point size
		void adjustPointSize(float& radius, const Vector3& position, int flags);

	private:
		bool			    m_autoClear = false;
        ShaderProgramPtr    m_shader;
		MaterialPtr		    m_material;
		Batch*			    m_lineBatch = nullptr;
		Batch*			    m_triangleBatch = nullptr;
		ShaderProgramPtr	m_shaderSprite;
		set<Batch*>::type	m_spriteBatchs;
	};
}
