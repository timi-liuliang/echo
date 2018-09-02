#include "engine/core/log/Log.h"
#include "Mesh.h"
#include "Render/Renderer.h"
#include <algorithm>

namespace Echo
{
	Mesh* Mesh::create(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer)
	{
		return EchoNew(Mesh(isDynamicVertexBuffer, isDynamicIndicesBuffer));
	}

	void Mesh::release()
	{
		ECHO_DELETE_T(this, Mesh);
	}

	Mesh::Mesh(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer)
		: m_topologyType(TT_TRIANGLELIST)
		, m_vertexBuffer(NULL)
		, m_indexBuffer(NULL)
		, m_idxCount(0)
		, m_idxStride(0)
		, m_indices(NULL)
		, m_startVert(0)
		, m_startIdx(0)
		, m_isDynamicVertexBuffer(isDynamicVertexBuffer)
		, m_isDynamicIndicesBuffer(isDynamicIndicesBuffer)
	{
	}

	Mesh::~Mesh()
	{
		clear();
	}

	GPUBuffer* Mesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	GPUBuffer* Mesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	void Mesh::buildTangentData()
	{
		ui32 faceCount = getFaceCount();

		// build tangent data by position and uv
		vector<Vector3>::type tangentDatas;  tangentDatas.resize(m_vertData.getVertexCount(), Vector3::ZERO);
		vector<Vector3>::type binormalDatas;  binormalDatas.resize(m_vertData.getVertexCount(), Vector3::ZERO);
		for (ui32 i = 0; i < faceCount; i++)
		{
			// only support trang list topology
			Word baseIdx = i * 3;
			Word vertIdx0 = ((Word*)m_indices)[baseIdx + 0];
			Word vertIdx1 = ((Word*)m_indices)[baseIdx + 1];
			Word vertIdx2 = ((Word*)m_indices)[baseIdx + 2];

			const Vector3& pos0 = m_vertData.getPosition(vertIdx0);
			const Vector3& pos1 = m_vertData.getPosition(vertIdx1);
			const Vector3& pos2 = m_vertData.getPosition(vertIdx2);

			const Vector2& uv0 = m_vertData.getUV0(vertIdx0);
			const Vector2& uv1 = m_vertData.getUV0(vertIdx1);
			const Vector2& uv2 = m_vertData.getUV0(vertIdx2);

			Vector3 deltaPos1 = pos1 - pos0;
			Vector3 deltaPos2 = pos2 - pos0;

			Vector2 deltauv1 = uv1 - uv0;
			Vector2 deltauv2 = uv2 - uv0;

			float r = 1.f / (deltauv1.x*deltauv2.y - deltauv1.y*deltauv2.x);
			Vector3 tangent = (deltaPos1*deltauv2.y - deltaPos2*deltauv1.y) * r;
			Vector3 binormal = (deltaPos2*deltauv1.x - deltaPos1*deltauv2.x) * r;

			tangentDatas[vertIdx0] += tangent;
			tangentDatas[vertIdx1] += tangent;
			tangentDatas[vertIdx2] += tangent;

			binormalDatas[vertIdx0] += binormal;
			binormalDatas[vertIdx1] += binormal;
			binormalDatas[vertIdx2] += binormal;
		}

		// normalize
		for (size_t i = 0; i < tangentDatas.size(); i++)
		{
			tangentDatas[i].normalize();
			binormalDatas[i].normalize();
		}
	}

	void Mesh::clear()
	{
		EchoSafeFree(m_indices);

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		EchoSafeDelete(m_indexBuffer, GPUBuffer);

		m_vertData.reset();
	}

	ui32 Mesh::getIndexCount() const
	{
		return m_idxCount;
	}

	ui32 Mesh::getFaceCount() const
	{
		ui32 count = m_indexBuffer ? m_idxCount : getVertexCount();
		switch (m_topologyType)
		{
		case TT_POINTLIST:		return count;
		case TT_LINELIST:		return count / 2;
		case TT_LINESTRIP:		return count - 1;
		case TT_TRIANGLELIST:	return count / 3;
		case TT_TRIANGLESTRIP:	return count - 2;
		default:				return 0;
		}
	}

	ui32 Mesh::getIndexStride() const
	{
		return m_idxStride;
	}

	Word* Mesh::getIndices() const
	{
		return (Word*)m_indices;
	}

	ui32 Mesh::getMemeoryUsage() const
	{
		return m_vertData.getVertexStride()*m_vertData.getVertexCount() + m_idxCount*m_idxStride;
	}

	void Mesh::generateTangentData(bool useNormalMap)
	{
		if (useNormalMap)
		{
			buildTangentData();
		}
		else
		{
			//m_vertData.m_isUseTangentBinormal = false;
			//removeVertexData(RenderInput::VS_TANGENT);
			//removeVertexData(RenderInput::VS_BINORMAL);
		}
	}

	const VertexElementList& Mesh::getVertexElements() const
	{
		return m_vertData.getFormat().m_vertexElements;
	}

	bool Mesh::buildBuffer()
	{
		buildIndexBuffer();
		buildVertexBuffer();

		return true;
	}

	void Mesh::buildIndexBuffer()
	{
		Buffer indexBuff(m_idxCount*m_idxStride, m_indices);
		if (m_isDynamicIndicesBuffer)
		{
			if (!m_indexBuffer)
				m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_DYNAMIC, indexBuff);
			else
				m_indexBuffer->updateData(indexBuff);
		}
		else
		{
			if(!m_indexBuffer)
				m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, indexBuff);
			else
				EchoLogError("Cannot modify static mesh index buffer");
		}	
	}

	void Mesh::buildVertexBuffer()
	{
		Buffer vertBuff(m_vertData.getByteSize(), m_vertData.getVertices());
		if (m_isDynamicVertexBuffer)
		{
			if (!m_vertexBuffer)
				m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_DYNAMIC, vertBuff);
			else
				m_vertexBuffer->updateData(vertBuff);
		}
		else
		{
			if (!m_vertexBuffer)
				m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
			else
				EchoLogError("Cannot modify static mesh vertex buffer");
		}	
	}

	// update indices data
	void Mesh::updateIndices(ui32 indicesCount, ui32 indicesStride, const void* indices)
	{
		// load indices
		m_idxCount = indicesCount;
		m_idxStride = indicesStride;
		if (m_idxCount)
		{
			// process data
			EchoSafeFree(m_indices);

			ui32 idxBuffSize = m_idxCount * m_idxStride;
			m_indices = EchoAlloc(Byte, idxBuffSize);
			memcpy(m_indices, indices, idxBuffSize);

			buildIndexBuffer();
		}
	}

	// update vertex data
	void Mesh::updateVertexs(const MeshVertexFormat& format, ui32 vertCount, const Byte* vertices, const AABB& box)
	{
		m_vertData.set(format, vertCount);
		if (vertCount)
		{
			// copy data
			memcpy(m_vertData.getVertices(), vertices, vertCount * m_vertData.getVertexStride());
			m_box = box;

			buildVertexBuffer();
		}
	}

	// update vertex data
	void Mesh::updateVertexs(const MeshVertexData& vertexData, const AABB& box)
	{
		m_vertData = vertexData;
		m_box = box;

		buildVertexBuffer();
	}
}
