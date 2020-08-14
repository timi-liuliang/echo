#include "engine/core/log/Log.h"
#include "MeshRes.h"
#include "engine/core/render/base/Renderer.h"
#include <algorithm>

namespace Echo
{
	MeshRes* MeshRes::create(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer)
	{
		return EchoNew(MeshRes(isDynamicVertexBuffer, isDynamicIndicesBuffer));
	}

	MeshRes::MeshRes(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer)
		: m_topologyType(TT_TRIANGLELIST)
		, m_isDynamicVertexBuffer(isDynamicVertexBuffer)
		, m_isDynamicIndicesBuffer(isDynamicIndicesBuffer)
	{
	}

	MeshRes::~MeshRes()
	{
		clear();
	}

	void MeshRes::bindMethods()
	{

	}

	GPUBuffer* MeshRes::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	GPUBuffer* MeshRes::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	void MeshRes::buildTangentData()
	{
		ui32 faceCount = getFaceCount();

		// build tangent data by position and uv
		vector<Vector3>::type tangentDatas;  tangentDatas.resize(m_vertData.getVertexCount(), Vector3::ZERO);
		vector<Vector3>::type binormalDatas;  binormalDatas.resize(m_vertData.getVertexCount(), Vector3::ZERO);
		for (ui32 i = 0; i < faceCount; i++)
		{
			Word* indices = (Word*)m_indices.data();

			// only support trang list topology
			Word baseIdx = i * 3;
			Word vertIdx0 = indices[baseIdx + 0];
			Word vertIdx1 = indices[baseIdx + 1];
			Word vertIdx2 = indices[baseIdx + 2];

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

	void MeshRes::clear()
	{
		m_indices.clear();
		m_indices.shrink_to_fit();

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		EchoSafeDelete(m_indexBuffer, GPUBuffer);

		m_vertData.reset();
	}

	ui32 MeshRes::getIndexCount() const
	{
		return m_idxCount;
	}

	ui32 MeshRes::getFaceCount() const
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

	ui32 MeshRes::getIndexStride() const
	{
		return m_idxStride;
	}

	Word* MeshRes::getIndices() const
	{
		return (Word*)m_indices.data();
	}

	ui32 MeshRes::getMemeoryUsage() const
	{
		return m_vertData.getVertexStride()*m_vertData.getVertexCount() + m_idxCount*m_idxStride;
	}

	void MeshRes::generateTangentData(bool useNormalMap)
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

	const VertexElementList& MeshRes::getVertexElements() const
	{
		return m_vertData.getFormat().m_vertexElements;
	}

	bool MeshRes::buildBuffer()
	{
		buildIndexBuffer();
		buildVertexBuffer();

		return true;
	}

	void MeshRes::buildIndexBuffer()
	{
		Buffer indexBuff(m_idxCount*m_idxStride, m_indices.data());
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

	void MeshRes::buildVertexBuffer()
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

	void MeshRes::updateIndices(ui32 indicesCount, ui32 indicesStride, const void* indices)
	{
		// load indices
		m_idxCount = indicesCount;
		m_idxStride = indicesStride;
		if (m_idxCount)
		{
			const Byte* indicesInByte = (const Byte*)indices;
			ui32 idxBuffSize = m_idxCount * m_idxStride;
			m_indices.assign(indicesInByte, indicesInByte +idxBuffSize);

			buildIndexBuffer();
		}
	}

	void MeshRes::updateVertexs(const MeshVertexFormat& format, ui32 vertCount, const Byte* vertices)
	{
		m_vertData.set(format, vertCount);
		if (vertCount)
		{
			// copy data
			memcpy(m_vertData.getVertices(), vertices, vertCount * m_vertData.getVertexStride());

			// calculate local aabb
			m_box.reset();
			for (i32 i = 0; i < m_vertData.getVertexCount(); i++)
			{
				m_box.addPoint(m_vertData.getPosition(i));
			}

			// update vertex buffer
			buildVertexBuffer();
		}
	}

	void MeshRes::updateVertexs(const MeshVertexData& vertexData)
	{
		m_vertData = vertexData;

		// calculate local aabb
		m_box.reset();
		for (i32 i = 0; i < m_vertData.getVertexCount(); i++)
		{
			m_box.addPoint(m_vertData.getPosition(i));
		}

		buildVertexBuffer();
	}

	Res* MeshRes::load(const ResourcePath& path)
	{
		if (!path.isEmpty())
		{
			MeshRes* res = EchoNew(MeshRes);
			return res;
		}

		return nullptr;
	}

	void MeshRes::save()
	{

	}
}
