#pragma once

#include "engine/core/geom/Box.h"
#include "VertexData.h"


namespace Echo
{
	/**
	* Mesh 2013-11-6
	*/
	class Mesh
	{
	public:
		// Vertex Define
		struct VertexDefine
		{
			bool	m_isUseNormal;
			bool	m_isUseVertexColor;
			bool	m_isUseDiffuseUV;

			VertexDefine()
				: m_isUseNormal(false), m_isUseVertexColor(false), m_isUseDiffuseUV(false)
			{}
		};

	public:
		// 创建
		static Mesh* create();

		// 释放
		void release();

		// 获取名称
		const String& getName() const { return m_name; }

		// 获取顶点信息
		VertexData& getVertexData() { return m_vertData; }

		// 获取顶点格式大小
		ui32 getVertexStride() const { return m_vertData.m_stride; }

		// 获取顶点数量
		ui32 getVertexCount() const { return m_vertData.m_count; }

		// 获取顶点数据
		Byte* getVertices() const { return m_vertData.m_vertices; }

		// 获取顶点缓冲
		GPUBuffer* getVertexBuffer() const;

		// 获取索引缓冲
		GPUBuffer* getIndexBuffer() const;

		// 获取面数量
		ui32 getFaceCount() const;

		// 获取索引数量
		ui32 getIndexCount() const;

		// 获取索引格式大小
		ui32 getIndexStride() const;

		// 获取索引数据
		Word* getIndices() const;

		// 是否为蒙皮
		bool isSkin() const { return isVertexUsage(RenderInput::VS_BLENDINDICES); }

		// 判断顶点格式中是否含有指定类型的数据
		bool isVertexUsage(RenderInput::VertexSemantic semantic) const { return m_vertData.isVertexUsage(semantic); }

		// 获取顶点格式
		const RenderInput::VertexElementList& getVertexElements() const;

		// 生成切线数据
		void generateTangentData(bool useNormalMap);

		// 获取本地包围盒
		const Box& getLocalBox() const { return m_box; }

		// 获取受影响的骨骼数量
		ui32 getBoneNum() const { return static_cast<ui32>(m_boneIdxs.size()); }

		// 获取受影响的骨骼索引
		ui32 getBoneIdx(int idx) { return m_boneIdxs[idx]; }

		// 设置数据
		void set(const VertexDefine& format,ui32 vertCount, const Byte* vertices, ui32 indicesCount, const ui16* indices, const Box& box);

		// 清空数据
		void clear();

	protected:
		Mesh();
		~Mesh();

		// 附加数据
		void insertVertexData(const RenderInput::VertexElement& element, void* templateData);

		// 移除数据
		void removeVertexData(RenderInput::VertexSemantic semantic);

		// 获取内存占用大小
		ui32 getMemeoryUsage() const;

		// 计算切线数据
		void buildTangentData();

		// 建立顶点与索引缓冲
		bool buildBuffer();

		// 新建顶点缓冲
		void buildVertexBuffer();

		// 建立索引缓冲
		void buildIndexBuffer();

	protected:
		String					m_name;					// 名称
		Box						m_box;					// 包围盒
		ui32					m_idxCount;				// 索引数量
		ui32					m_idxStride;			// 索引格式大小
		Byte*					m_indices;				// 索引数据
		VertexData				m_vertData;				// 顶点数据
		GPUBuffer*				m_vertexBuffer;			// 顶点缓冲
		GPUBuffer*				m_indexBuffer;			// 索引缓冲
		vector<ui32>::type		m_boneIdxs;				// 骨骼索引(mesh only use a part of bones of a skeleton)
	};
}

