#pragma once

#include "irender_queue.h"

namespace Echo
{
	class ImageFilter : public IRenderQueue
	{
		ECHO_CLASS(ImageFilter, IRenderQueue)

	public:
		// Vertex Format
		struct VertexFormat
		{
			Vector3        m_position;
			Vector2        m_uv;

			VertexFormat(const Vector3& pos, const Vector2& uv)
				: m_position(pos), m_uv(uv)
			{}
		};
		typedef vector<VertexFormat>::type  VertexArray;
		typedef vector<ui16>::type          IndiceArray;

	public:
		ImageFilter() {}
		virtual ~ImageFilter();

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

		// render
		virtual void render(FrameBufferPtr& frameBuffer) override ;

	protected:
		// build render able
		bool buildRenderable();

		// update mesh buffer
		void updateMeshBuffer();

		// build mesh data by Draw ables data
		void buildMeshData(VertexArray& oVertices, IndiceArray& oIndices);

		// clear render able
		void clearRenderable();

	private:
		// set material global uniforms
		void setGlobalUniforms();

	protected:
		bool			m_isRenderableDirty = true;
		MeshPtr			m_mesh;
		MaterialPtr		m_material;
		RenderProxyPtr	m_renderable;
	};
}
