#pragma once

#include "interface/Renderable.h"
#include "interface/mesh/Mesh.h"

namespace Echo
{
	class GLES2Renderable : public Renderable
	{
	public:
		// vertex stream bind state(for multi stream)
		enum BindState
		{
			BS_NORMAL = 0,
			BS_BEGIN = 1 << 0,
			BS_END = 1 << 1,
		};

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

		// 顶点流
		struct StreamUnit
		{
			VertexElementList		m_vertElements;		// 顶点格式声明
			VertexDeclarationList	m_vertDeclaration;	// 顶点声明
			ui32					m_vertStride;		// 每顶点大小
			GPUBuffer*				m_buffer;			// 顶点数据

			StreamUnit()
				: m_buffer(NULL)
			{}

			bool isUsedFor(VertexSemantic);
		};

	public:
		GLES2Renderable(const String& renderStage, ShaderProgramRes* shader, int identifier);
		~GLES2Renderable();

		// 绑定几何体数据
		void bind( Renderable* pre);

		// 解绑几何体数据
		void unbind();

	private:
		virtual void link() override;

		// bind vertex stream
		bool bindVertexStream(const VertexElementList& vertElements, GPUBuffer* vertexBuffer, int flag = BS_BEGIN | BS_END);

		// 计算顶点流声明
		virtual bool buildVertStreamDeclaration(StreamUnit* stream);

		// 生成顶点流Hash值(BKDR Hash)
		virtual void generateVertexStreamHash();

	private:
		vector<StreamUnit>::type	m_vertexStreams;
		unsigned int				m_vertexStreamsHash;
		bool						m_is_muti_stream;
		GLuint						m_vao;					// 顶点数组(OpenGLES 3.0)
	};
}
