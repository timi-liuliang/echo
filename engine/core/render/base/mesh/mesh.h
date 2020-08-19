#pragma once

#include "engine/core/geom/AABB.h"
#include "MeshVertexData.h"
#include "engine/core/resource/Res.h"

namespace Echo
{
	/**
	* Mesh 2013-11-6
	*/
	class GPUBuffer;
	class Mesh : public Res
	{
		ECHO_RES(Mesh, Res, ".mesh", nullptr, Mesh::load);

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
		Mesh() {}
		~Mesh();

		// create
		static Mesh* create(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer);

		// name
		const String& getName() const { return m_name; }

		// topology type
		TopologyType getTopologyType() { return m_topologyType; }

		// vertex data
		MeshVertexData& getVertexData() { return m_vertData; }

		// vertex stride
		ui32 getVertexStride() const { return m_vertData.getVertexStride(); }

		// vertex count
		ui32 getVertexCount() const { return m_vertData.getVertexCount(); }

		// vertex data ptr
		const MeshVertexData& getVertices() const { return m_vertData; }

		// get buffer object
		GPUBuffer* getVertexBuffer() const;
		GPUBuffer* getIndexBuffer() const;

		// get face count
		ui32 getFaceCount() const;

		// get index count
		ui32 getIndexCount() const;

		// get index stride
		ui32 getIndexStride() const;

		// start vertex and index
		void setStartVertex(ui32 startVert) { m_startVert = startVert; }
		void setStartIndex(ui32 startIdx) { m_startIdx = startIdx; }
		ui32 getStartVertex() const { return m_startVert; }
		ui32 getStartIndex() const { return m_startIdx; }

		// get indices
		Word* getIndices() const;

		// is valid
		bool isValid() const { return getFaceCount() > 0; }

		// is have bone data
		bool isSkin() const { return isVertexUsage(VS_BLENDINDICES); }

		// is vertex usage
		bool isVertexUsage(VertexSemantic semantic) const { return m_vertData.isVertexUsage(semantic); }

		// get vertex elements
		const VertexElementList& getVertexElements() const;

		// generate tangent
		void generateTangentData(bool useNormalMap);

		// get local aabb
		const AABB& getLocalBox() const { return m_box; }

		// get bone count
		ui32 getBoneNum() const { return static_cast<ui32>(m_boneIdxs.size()); }

		// get bone index
		ui32 getBoneIdx(int idx) { return m_boneIdxs[idx]; }

		// set primitive type
		void setTopologyType(TopologyType type) { m_topologyType = type; }

		// update indices data
		void updateIndices(ui32 indicesCount, ui32 indicesStride, const void* indices);

		// update vertex data
		void updateVertexs(const MeshVertexFormat& format, ui32 vertCount, const Byte* vertices);
		void updateVertexs(const MeshVertexData& vertexData);

		// clear
		void clear();

		// load|save
		static Res* load(const ResourcePath& path);
		virtual void save() override;

	protected:
		Mesh(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer);

		// get memory usage
		ui32 getMemeoryUsage() const;

		// calculate tangent data
		void buildTangentData();

		// build buffer
		bool buildBuffer();

		// build Vertex|Index buffer
		void buildVertexBuffer();
		void buildIndexBuffer();

	protected:
		String						m_name;
		TopologyType				m_topologyType;
		AABB						m_box;
		ui32						m_startVert = 0;
		ui32						m_startIdx = 0;
		ui32						m_idxCount = 0;
		ui32						m_idxStride = 0;
		vector<Byte>::type			m_indices;
		MeshVertexData				m_vertData;
		bool						m_isDynamicVertexBuffer = false;
		GPUBuffer*					m_vertexBuffer = nullptr;
		bool						m_isDynamicIndicesBuffer = false;
		GPUBuffer*					m_indexBuffer = nullptr;
		vector<ui32>::type			m_boneIdxs;
	};
	typedef Echo::ResRef<Echo::Mesh> MeshResPtr;
}

