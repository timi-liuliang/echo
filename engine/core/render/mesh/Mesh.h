#pragma once

#include "engine/core/geom/AABB.h"
#include "MeshVertexData.h"

namespace Echo
{
	/**
	* Mesh 2013-11-6
	*/
	class GPUBuffer;
	class Mesh
	{
	public:
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

	public:
		// 创建
		static Mesh* create(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer);

		// 释放
		void release();

		// 获取名称
		const String& getName() const { return m_name; }

		// 图元类型
		TopologyType getTopologyType() { return m_topologyType; }

		// 获取顶点信息
		MeshVertexData& getVertexData() { return m_vertData; }

		// 获取顶点格式大小
		ui32 getVertexStride() const { return m_vertData.getVertexStride(); }

		// 获取顶点数量
		ui32 getVertexCount() const { return m_vertData.getVertexCount(); }

		// 获取顶点数据
		const MeshVertexData& getVertices() const { return m_vertData; }

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

		// start vertex and index
		void setStartVertex(ui32 startVert) { m_startVert = startVert; }
		void setStartIndex(ui32 startIdx) { m_startIdx = startIdx; }
		ui32 getStartVertex() const { return m_startVert; }
		ui32 getStartIndex() const { return m_startIdx; }

		// 获取索引数据
		Word* getIndices() const;

		// is valid
		bool isValid() const { return getFaceCount() > 0; }

		// 是否为蒙皮
		bool isSkin() const { return isVertexUsage(VS_BLENDINDICES); }

		// 判断顶点格式中是否含有指定类型的数据
		bool isVertexUsage(VertexSemantic semantic) const { return m_vertData.isVertexUsage(semantic); }

		// 获取顶点格式
		const VertexElementList& getVertexElements() const;

		// 生成切线数据
		void generateTangentData(bool useNormalMap);

		// 获取本地包围盒
		const AABB& getLocalBox() const { return m_box; }

		// 获取受影响的骨骼数量
		ui32 getBoneNum() const { return static_cast<ui32>(m_boneIdxs.size()); }

		// 获取受影响的骨骼索引
		ui32 getBoneIdx(int idx) { return m_boneIdxs[idx]; }

		// set primitive type
		void setTopologyType(TopologyType type) { m_topologyType = type; }

		// update indices data
		void updateIndices(ui32 indicesCount, ui32 indicesStride, const void* indices);

		// update vertex data
		void updateVertexs(const MeshVertexFormat& format, ui32 vertCount, const Byte* vertices, const AABB& box);
		void updateVertexs(const MeshVertexData& vertexData, const AABB& box);

		// 清空数据
		void clear();

	protected:
		Mesh(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer);
		~Mesh();

		// get memory usage
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
		String						m_name;						// 名称
		TopologyType				m_topologyType;				// 图元类型
		AABB						m_box;						// 包围盒
		ui32						m_startVert;
		ui32						m_startIdx;
		ui32						m_idxCount;					// 索引数量
		ui32						m_idxStride;				// 索引格式大小
		Byte*						m_indices;					// 索引数据
		MeshVertexData				m_vertData;					// 顶点数据
		bool						m_isDynamicVertexBuffer;	// 是否支持动态更新
		GPUBuffer*					m_vertexBuffer;				// 顶点缓冲
		bool						m_isDynamicIndicesBuffer;	// 索引缓冲
		GPUBuffer*					m_indexBuffer;				// 索引缓冲
		vector<ui32>::type			m_boneIdxs;					// 骨骼索引(mesh only use a part of bones of a skeleton)
	};
}

