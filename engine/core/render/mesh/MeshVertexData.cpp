#include "MeshVertexData.h"

namespace Echo
{
	MeshVertexFormat::MeshVertexFormat()
		: m_isUseNormal(false)
		, m_isUseVertexColor(false)
		, m_isUseUV(false)
		, m_isUseLightmapUV(false)
		, m_isUseBoneData(false)
		, m_isUseTangentBinormal(false)
		, m_stride(0)
		, m_posOffset(0)
		, m_normalOffset(0)
		, m_colorOffset(0)
		, m_uv0Offset(0)
		, m_uv1Offset(0)
		, m_boneIndicesOffset(0)
		, m_boneWeightsOffset(0)
	{

	}

	// 计算偏移量,顶点格式
	void MeshVertexFormat::build()
	{
		m_posOffset = 0;
		m_normalOffset = m_posOffset + sizeof(Vector3);
		m_colorOffset = m_normalOffset + (m_isUseNormal ? sizeof(Vector3) : 0);
		m_uv0Offset = m_colorOffset + (m_isUseVertexColor ? sizeof(Dword) : 0);
		m_uv1Offset = m_uv0Offset + (m_isUseUV ? sizeof(Vector2) : 0);
		m_boneIndicesOffset = m_uv1Offset + (m_isUseLightmapUV ? sizeof(Vector2) : 0);
		m_boneWeightsOffset = m_boneIndicesOffset + (m_isUseBoneData ? sizeof(Dword) : 0);
		m_tangentOffset = m_boneWeightsOffset + (m_isUseBoneData ? sizeof(Vector3) : 0);
		m_stride = m_tangentOffset + (m_isUseTangentBinormal ? sizeof(Vector3) * 2 : 0);

		m_vertexElements.clear();

		// 增加位置的空间
		m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_POSITION, PF_RGB32_FLOAT));

		// 使用法线
		if (m_isUseNormal)
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_NORMAL, PF_RGB32_FLOAT));

		// 增加顶点色
		if (m_isUseVertexColor)
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_COLOR, PF_RGBA8_UNORM));

		// 纹理坐标
		if (m_isUseUV)
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_TEXCOORD0, PF_RG32_FLOAT));

		// 灯光图
		if (m_isUseLightmapUV)
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_TEXCOORD1, PF_RG32_FLOAT));

		// 动画数据(骨骼权重与索引)
		if (m_isUseBoneData)
		{
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_BLENDINDICES, PF_RGBA8_UINT));
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_BLENDWEIGHTS, PF_RGB32_FLOAT));
		}

		// 切空间数据
		if (m_isUseTangentBinormal)
		{
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_TANGENT, PF_RGB32_FLOAT));
			m_vertexElements.push_back(RenderInput::VertexElement(RenderInput::VS_BINORMAL, PF_RGB32_FLOAT));
		}
	}

	// 判断顶点格式中是否含有指定类型的数据
	bool MeshVertexFormat::isVertexUsage(RenderInput::VertexSemantic semantic) const
	{
		size_t num = m_vertexElements.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (m_vertexElements[i].m_semantic == semantic)
				return true;
		}

		return false;
	}

	// reset
	void MeshVertexFormat::reset()
	{
		m_isUseNormal = false;
		m_isUseVertexColor = false;
		m_isUseUV = false;
		m_isUseLightmapUV = false;
		m_isUseBoneData = false;
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

	// 构造函数
	MeshVertexData::MeshVertexData()
		: m_count(0)
	{}

	// set
	void MeshVertexData::set(const MeshVertexFormat& format, ui32 count)
	{
		m_format = format;
		m_format.build();

		m_count = count;
		m_vertices.resize(m_count * m_format.m_stride);
	}

	// 获取顶点格式大小
	ui32 MeshVertexData::getVertexStride() const
	{
		return m_format.m_stride;
	}

	// 获取顶点数量
	ui32 MeshVertexData::getVertexCount() const
	{
		return m_count;
	}

	// 获取顶点数据
	Byte* MeshVertexData::getVertices()
	{
		if (m_vertices.size())
			return m_vertices.data();
		else
			return nullptr;
	}

	// 获取顶点地址
	Byte* MeshVertexData::getVertice(int idx)
	{
		Byte* verticesPtr = m_vertices.data();
		return verticesPtr + idx * m_format.m_stride;
	}

	// 判断顶点格式中是否含有指定类型的数据
	bool MeshVertexData::isVertexUsage(RenderInput::VertexSemantic semantic) const
	{
		return m_format.isVertexUsage(semantic);
	}

	// 获取顶点位置数据
	Vector3& MeshVertexData::getPosition(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_POSITION));

		return *(Vector3*)(getVertice(index) + m_format.m_posOffset);
	}

	// set position
	void MeshVertexData::setPosition(int idx, const Vector3& pos)
	{
		*(Vector3*)(getVertice(idx) + m_format.m_posOffset) = pos;

		m_aabb.addPoint(pos);
	}

	// 获取顶点法线数据
	const Vector3& MeshVertexData::getNormal(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_NORMAL));

		return *(Vector3*)(getVertice(index) + m_format.m_normalOffset);
	}

	void MeshVertexData::setNormal(int idx, const Vector3& normal)
	{
		*(Vector3*)(getVertice(idx) + m_format.m_normalOffset) = normal;
	}

	// 获取顶点颜色数据
	Dword& MeshVertexData::getColor(Word index)
	{
		EchoAssert(index < m_count && RenderInput::VS_COLOR);

		return *(Dword*)(getVertice(index) + m_format.m_colorOffset);
	}

	// 获取顶点UV数据0
	const Vector2& MeshVertexData::getUV0(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_TEXCOORD0));

		return *(Vector2*)(getVertice(index) + m_format.m_uv0Offset);
	}

	void MeshVertexData::setUV0(int idx, const Vector2& uv0)
	{
		*(Vector2*)(getVertice(idx) + m_format.m_uv0Offset) = uv0;
	}

	// 获取顶点UV数据1
	const Vector2& MeshVertexData::getUV1(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_TEXCOORD1));

		return *(Vector2*)(getVertice(index) + m_format.m_uv1Offset);
	}

	// 获取切线
	Vector3& MeshVertexData::getTangent(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_TANGENT));

		return *(Vector3*)(getVertice(index) + m_format.m_tangentOffset);
	}

	void MeshVertexData::reset()
	{
		m_count = 0;
		m_vertices.clear();
		m_aabb.reset();
	}
}