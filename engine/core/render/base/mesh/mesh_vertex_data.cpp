#include "mesh_vertex_data.h"

namespace Echo
{
	MeshVertexFormat::MeshVertexFormat()
	{

	}

	void MeshVertexFormat::build()
	{
		m_posOffset = 0;
		m_normalOffset = m_posOffset + sizeof(Vector3);
		m_colorOffset = m_normalOffset + (m_isUseNormal ? sizeof(Vector3) : 0);
		m_uv0Offset = m_colorOffset + (m_isUseVertexColor ? sizeof(Dword) : 0);
		m_uv1Offset = m_uv0Offset + (m_isUseUV ? sizeof(Vector2) : 0);
		m_boneIndicesOffset = m_uv1Offset + (m_isUseLightmapUV ? sizeof(Vector2) : 0);
		m_boneWeightsOffset = m_boneIndicesOffset + (m_isUseBlendingData ? sizeof(Dword) : 0);
		m_tangentOffset = m_boneWeightsOffset + (m_isUseBlendingData ? sizeof(Vector4) : 0);
		m_stride = m_tangentOffset + (m_isUseTangentBinormal ? sizeof(Vector3) * 2 : 0);

		m_vertexElements.clear();

		m_vertexElements.emplace_back(VS_POSITION, PF_RGB32_FLOAT);

		if (m_isUseNormal)
			m_vertexElements.emplace_back(VS_NORMAL, PF_RGB32_FLOAT);

		if (m_isUseVertexColor)
			m_vertexElements.emplace_back(VS_COLOR, PF_RGBA8_UNORM);

		if (m_isUseUV)
			m_vertexElements.emplace_back(VS_TEXCOORD0, PF_RG32_FLOAT);

		if (m_isUseLightmapUV)
			m_vertexElements.emplace_back(VS_TEXCOORD1, PF_RG32_FLOAT);

		if (m_isUseBlendingData)
		{
			m_vertexElements.emplace_back(VS_BLENDINDICES, PF_RGBA8_UINT);
			m_vertexElements.emplace_back(VS_BLENDWEIGHTS, PF_RGBA32_FLOAT);
		}

		if (m_isUseTangentBinormal)
		{
			m_vertexElements.emplace_back(VS_TANGENT, PF_RGB32_FLOAT);
			m_vertexElements.emplace_back(VS_BINORMAL, PF_RGB32_FLOAT);
		}
	}

	bool MeshVertexFormat::isVertexUsage(VertexSemantic semantic) const
	{
		size_t num = m_vertexElements.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (m_vertexElements[i].m_semantic == semantic)
				return true;
		}

		return false;
	}

	void MeshVertexFormat::reset()
	{
		m_isUseNormal = false;
		m_isUseVertexColor = false;
		m_isUseUV = false;
		m_isUseLightmapUV = false;
		m_isUseBlendingData = false;
		m_isUseTangentBinormal = false;
		m_stride = 0;
		m_posOffset = 0;
		m_normalOffset = 0;
		m_colorOffset = 0;
		m_uv0Offset = 0;
		m_uv1Offset = 0;
		m_boneIndicesOffset = 0;
		m_boneWeightsOffset = 0;
	}

	MeshVertexData::MeshVertexData()
		: m_count(0)
	{}

	void MeshVertexData::set(const MeshVertexFormat& format, ui32 count)
	{
		m_format = format;
		m_format.build();

		m_count = count;
		m_vertices.resize(m_count * m_format.m_stride);
	}

	ui32 MeshVertexData::getVertexStride() const
	{
		return m_format.m_stride;
	}

	ui32 MeshVertexData::getVertexCount() const
	{
		return m_count;
	}

	Byte* MeshVertexData::getVertices()
	{
		if (m_vertices.size())
			return m_vertices.data();
		else
			return nullptr;
	}

	Byte* MeshVertexData::getVertice(int idx)
	{
		Byte* verticesPtr = m_vertices.data();
		return verticesPtr + idx * m_format.m_stride;
	}

	bool MeshVertexData::isVertexUsage(VertexSemantic semantic) const
	{
		return m_format.isVertexUsage(semantic);
	}

	Vector3& MeshVertexData::getPosition(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(VS_POSITION));

		return *(Vector3*)(getVertice(index) + m_format.m_posOffset);
	}

	void MeshVertexData::setPosition(int idx, const Vector3& pos)
	{
		*(Vector3*)(getVertice(idx) + m_format.m_posOffset) = pos;
	}

	void MeshVertexData::setColor(i32 idx, Dword color)
	{
		if(m_format.m_isUseVertexColor)
			*(Dword*)(getVertice(idx) + m_format.m_colorOffset) = color;
	}

	void MeshVertexData::setJoint(int idx, Dword weight)
	{
		if(m_format.m_isUseBlendingData)
			*(Dword*)(getVertice(idx) + m_format.m_boneIndicesOffset) = weight;
	}

	void MeshVertexData::setWeight(int idx, const Vector4& joint)
	{
		if (m_format.m_isUseBlendingData)
			*(Vector4*)(getVertice(idx) + m_format.m_boneWeightsOffset) = joint;
	}

	const Vector3& MeshVertexData::getNormal(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(VS_NORMAL));

		return *(Vector3*)(getVertice(index) + m_format.m_normalOffset);
	}

	void MeshVertexData::setNormal(int idx, const Vector3& normal)
	{
		Vector3::Normalize(*(Vector3*)(getVertice(idx) + m_format.m_normalOffset),normal);
	}

	Dword& MeshVertexData::getColor(Word index)
	{
		EchoAssert(index < m_count && VS_COLOR);

		return *(Dword*)(getVertice(index) + m_format.m_colorOffset);
	}

	const Vector2& MeshVertexData::getUV0(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(VS_TEXCOORD0));

		return *(Vector2*)(getVertice(index) + m_format.m_uv0Offset);
	}

	void MeshVertexData::setUV0(int idx, const Vector2& uv0)
	{
		*(Vector2*)(getVertice(idx) + m_format.m_uv0Offset) = uv0;
	}

	const Vector2& MeshVertexData::getUV1(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(VS_TEXCOORD1));

		return *(Vector2*)(getVertice(index) + m_format.m_uv1Offset);
	}

	Vector3& MeshVertexData::getTangent(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(VS_TANGENT));

		return *(Vector3*)(getVertice(index) + m_format.m_tangentOffset);
	}

	void MeshVertexData::reset()
	{
		m_count = 0;
		m_vertices.clear();
	}

	ByteArray MeshVertexData::getPositions()
	{
		ByteArray result;
		result.resize(sizeof(Vector3) * getVertexCount());

		Vector3* dataPtr = (Vector3*)(&result[0]);
		for (size_t i = 0; i < m_count; ++i)
		{
			dataPtr[i] = getPosition(i);
		}

		return result;
	}

	ByteArray MeshVertexData::getNormals()
	{
		ByteArray result;
		result.resize(sizeof(Byte) * 3 * getVertexCount());

		Byte* dataPtr = (Byte*)(&result[0]);
		for (size_t i = 0; i < m_count; ++i)
		{
			const Vector3& normal = getNormal(i);
			dataPtr[i * 3 + 0] = (normal.x + 1.f) * 0.5f * 255;
			dataPtr[i * 3 + 1] = (normal.y + 1.f) * 0.5f * 255;
			dataPtr[i * 3 + 2] = (normal.z + 1.f) * 0.5f * 255;
		}

		return result;
	}

	ByteArray MeshVertexData::getUV0s()
	{
		ByteArray result;
		result.resize(sizeof(Vector2) * getVertexCount());

		Vector2* dataPtr = (Vector2*)(&result[0]);
		for (size_t i = 0; i < m_count; ++i)
		{
			dataPtr[i] = getUV0(i);
		}

		return result;
	}
}
