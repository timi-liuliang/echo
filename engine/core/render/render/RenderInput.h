#pragma once

#include "PixelFormat.h"
#include "GPUBuffer.h"

namespace Echo
{
	class Mesh;
	class ShaderProgram;
	class RenderInput
	{
	public:
		enum VertexSemantic
		{
			VS_UNKNOWN = -1,
			// Position (Vector3)
			VS_POSITION,
			// Normal (Vector3)
			VS_NORMAL,
			// Diffuse & specular color (Dword)
			VS_COLOR,
			// Texture coordinates 0(Vector2)
			VS_TEXCOORD0,
			// Texture coordinates 1(Vector2)
			VS_TEXCOORD1,
			// Blending indices	(Dword)
			VS_BLENDINDICES,
			// Blending weights(Vector3)
			VS_BLENDWEIGHTS,
			// Tangent (X axis if normal is Z)
			VS_TANGENT,
			VS_BINORMAL,

			// VR Mode
			VS_SCREEN_POS_NDC,
			VS_TAN_EYE_ANGLE_R,
			VS_TAN_EYE_ANGLE_G,
			VS_TAN_EYE_ANGLE_B,

			VS_MAX
		};

		enum TopologyType
		{
			// A list of points, 1 vertex per point
			TT_POINTLIST, 
			// A list of lines, 2 vertices per line
			TT_LINELIST, 
			// A strip of connected lines, 1 vertex per line plus 1 start vertex
			TT_LINESTRIP, 
			// A list of triangles, 3 vertices per triangle
			TT_TRIANGLELIST, 
			// A strip of triangles, 3 vertices for the first triangle, and 1 per triangle after that 
			TT_TRIANGLESTRIP, 
		};

		// vertex stream bind state(for multi stream)
		enum BindState
		{
			BS_NORMAL= 0,
			BS_BEGIN = 1<<0,
			BS_END   = 1<<1,
		};

		struct VertexElement
		{
			VertexSemantic		m_semantic;		// Vertex Semantic
			PixelFormat			m_pixFmt;		// Vertex pixel format

			VertexElement(VertexSemantic semantic = VS_UNKNOWN, PixelFormat pixFmt = PF_UNKNOWN)
				: m_semantic(semantic)
				, m_pixFmt(pixFmt)
			{}
		};
		typedef vector<VertexElement>::type	VertexElementList;

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
				: m_buffer( NULL)
			{}

			bool isUsedFor(VertexSemantic);
		};

		RenderInput( ShaderProgram *pProgram);
		virtual ~RenderInput();

	public:
		// mesh
		void setMesh(Mesh* mesh) { m_mesh = mesh; }
		Mesh* getMesh() { return m_mesh; }

		// 绑定顶点流
		bool bindVertexStream(const VertexElementList& vertElements, GPUBuffer* vertexStream, int flag = BS_BEGIN|BS_END);
		
		// 绑定索引流
		bool bindIndexStream(GPUBuffer* indexStream, ui32 idxStride = 2);

		virtual void unbindGPUBuffers();
		virtual void bind( RenderInput* pre) = 0;
		virtual void unbind() = 0;

		void setStartVertex(ui32 startVert) { m_startVert = startVert; }
		void setStartIndex(ui32 startIdx) { m_startIdx = startIdx; }
		void setIndexCount(ui32 idxCount) { m_idxCount = idxCount;}
		
		ui32 getStartVertex() const { return m_startVert;}
		ui32 getStartIndex() const { return m_startIdx;}

		const vector<StreamUnit>::type* getVertexBuffer() const { return &m_vertexStreams; }

		GPUBuffer* getIndexBuffer() const { return m_pIdxBuff; }

		ui32 getIndexStride() const { return m_idxStride; }

		ui32 getIndexCount() const { return m_idxCount; }
		
		void enableWireFrame(bool enable) { m_supportWireframe = enable; }

		bool isSupportWireFrame() const { return m_supportWireframe; }

		// 获取顶点流HASH值
		ui32 getVertexStreamHash() const { return m_vertexStreamsHash; }

		void set_is_muti_stream(bool is_muti_stream){ m_is_muti_stream = is_muti_stream; }

		// 设置拥有者信息
		void setOwnerInfo(const String& info) { m_ownerInfo = info; }

		// 获取拥有者信息
		const String& getOwnerInfo() const { return m_ownerInfo; }

	protected:
		// 计算顶点流声明
		virtual bool buildVertStreamDeclaration( StreamUnit* stream)=0;

		// 生成顶点流Hash值(BKDR Hash)
		virtual void generateVertexStreamHash()=0;

	protected:
		ShaderProgram*				m_program;
		Mesh*						m_mesh;
		ui32						m_startVert;
		ui32						m_startIdx;
		vector<StreamUnit>::type	m_vertexStreams;
		unsigned int				m_vertexStreamsHash;
		GPUBuffer*					m_pIdxBuff;
		ui32						m_idxStride;
		ui32						m_idxCount;
		bool						m_supportWireframe;
		bool						m_is_muti_stream;
		String						m_ownerInfo;
	};
}
