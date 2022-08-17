#pragma once

#include "deferred_lighting.h"

namespace Echo
{
	class DirectLighting : public DeferredLighting
	{
		ECHO_CLASS(DirectLighting, DeferredLighting)

		// Vertex Format
		struct VertexFormat
		{
			Vector3       m_position;
			Vector3       m_lightDir;
			Dword		  m_lightColor;

			VertexFormat(const Vector3& pos, const Vector3& dir, const Dword& color)
				: m_position(pos), m_lightDir(dir), m_lightColor(color)
			{}
		};
		typedef vector<VertexFormat>::type  VertexArray;
		typedef vector<ui16>::type          IndiceArray;

	public:
		DirectLighting();
		virtual ~DirectLighting();

		// Process
		virtual void render(FrameBufferPtr& frameBuffer) override;

	protected:
		// build render able
		bool buildRenderable();

		// update mesh buffer
		void updateMeshBuffer();

		// build mesh data by Draw ables data
		void buildMeshData(VertexArray& oVertices, IndiceArray& oIndices);

		// clear render able
		void clearRenderable();

	protected:
		MeshPtr			m_mesh;
		RenderProxyPtr	m_renderable;
	};
}