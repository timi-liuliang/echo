#pragma once

#include "base/Renderable.h"
#include "base/mesh/mesh.h"

namespace Echo
{
	class GLES2Renderable : public Renderable
	{
	public:
		// vertex declaration
		struct VertexDeclaration
		{
			i32		m_attribute;
			ui32	count;
			ui32	type;
			bool	bNormalize;
			ui32	elementOffset;
		};
		typedef vector<VertexDeclaration>::type	VertexDeclarationList;

		// stream unit
		struct StreamUnit
		{
			VertexElementList		m_vertElements;
			VertexDeclarationList	m_vertDeclaration;
			ui32					m_vertStride;
			GPUBuffer*				m_buffer;

			StreamUnit()
				: m_buffer(nullptr)
			{}
		};

	public:
		GLES2Renderable(int identifier);
		~GLES2Renderable();

		// bind geometry data
		void bind( Renderable* pre);

		// unbind geometry data
		void unbind();

		// bind shader params
		void bindShaderParams();

		// bind render state
		void bindRenderState();

	private:
		// set mesh
		virtual void setMesh(MeshPtr mesh) override;

		// set material
		virtual void setMaterial(Material* material) override;

		// bind vertex stream
		void bindVertexStream();

		// build vertex declaration
		virtual bool buildVertStreamDeclaration(StreamUnit* stream);

	private:
		vector<StreamUnit>::type		m_vertexStreams;
		//GLuint						m_vao = -1;
	};
}
