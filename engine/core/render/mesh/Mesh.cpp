#include "Mesh.h"
#include "Render/Renderer.h"
#include <algorithm>

namespace Echo
{
	// 创建
	Mesh* Mesh::create()
	{
		return EchoNew(Mesh);
	}

	// 释放
	void Mesh::release()
	{
		ECHO_DELETE_T(this, Mesh);
	}

	// 构造函数
	Mesh::Mesh()
		: m_vertexBuffer(NULL)
		, m_indexBuffer(NULL)
		, m_idxCount(0)
		, m_idxStride(0)
		, m_indices(NULL)
	{
	}

	// 析构函数
	Mesh::~Mesh()
	{
		clear();
	}

	// 获取顶点缓冲
	GPUBuffer* Mesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	// 获取索引缓冲
	GPUBuffer* Mesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	// 移除数据
	void Mesh::removeVertexData(RenderInput::VertexSemantic semantic)
	{
		if (!m_vertData.isVertexUsage(semantic))
			return;

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);

		RenderInput::VertexElementList mlastElements = m_vertData.m_vertexElements;
		m_vertData.m_vertexElements.clear();
		size_t removeIndex = 0;
		size_t removeStrite;
		size_t oldElementsSize = mlastElements.size();

		ui32 vertOffFront = 0;
		ui32 allStrite = 0;
		ui32 vertOffBack = 0;
		size_t j;
		for (j = 0; j < oldElementsSize; ++j)
		{
			if (mlastElements[j].m_semantic == semantic)
			{
				removeIndex = j;
				removeStrite = PixelUtil::GetPixelSize(mlastElements[j].m_pixFmt);
				break;
			}
			else
			{
				m_vertData.m_vertexElements.push_back(mlastElements[j]);
				allStrite += PixelUtil::GetPixelSize(mlastElements[j].m_pixFmt);
			}
		}
		vertOffFront = allStrite;
		j++;
		for (; j < oldElementsSize; ++j)
		{
			m_vertData.m_vertexElements.push_back(mlastElements[j]);
			allStrite += PixelUtil::GetPixelSize(mlastElements[j].m_pixFmt);
			vertOffBack += PixelUtil::GetPixelSize(mlastElements[j].m_pixFmt);
		}

		m_vertData.m_stride = allStrite;

		Byte* pNewVertexArray = EchoAlloc(Byte, allStrite*m_vertData.m_count);
		Byte* pcopyDest = pNewVertexArray;
		Byte* pcopySrc = m_vertData.m_vertices;
		for (size_t vertIdx = 0; vertIdx < m_vertData.m_count; ++vertIdx)
		{
			memcpy(pcopyDest, pcopySrc, vertOffFront);
			pcopyDest += vertOffFront;
			pcopySrc += vertOffFront + removeStrite;
			memcpy(pcopyDest, pcopySrc, vertOffBack);
			pcopyDest += vertOffBack;
			pcopySrc += vertOffBack;
		}

		EchoSafeFree(m_vertData.m_vertices);
		m_vertData.m_vertices = pNewVertexArray;

		Buffer vertBuff(m_vertData.m_stride*m_vertData.m_count, m_vertData.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
	}

	// 插入数据
	void Mesh::insertVertexData(const RenderInput::VertexElement& element, void* templateData)
	{
		if (isVertexUsage(element.m_semantic))
			return;

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);

		RenderInput::VertexElementList lastElements = m_vertData.m_vertexElements;
		m_vertData.m_vertexElements.clear();

		// first get the vertex semantic index to insert.
		ui32 insertIndex = 0;
		size_t oldElementsSize = lastElements.size();
		for (size_t j = 0; j < oldElementsSize; ++j)
		{
			if (lastElements[j].m_semantic < element.m_semantic)
				insertIndex++;
			else
				break;
		}

		// position.
		ui32 vertOffFront = 0;
		ui32 allStride = 0;
		ui32 vertOffBack = 0;
		size_t backindex = insertIndex;

		/// front
		for (size_t j = 0; j < backindex; ++j)
		{
			m_vertData.m_vertexElements.push_back(lastElements[j]);
			vertOffFront += PixelUtil::GetPixelSize(lastElements[j].m_pixFmt);
		}

		m_vertData.m_vertexElements.push_back(element);

		/// back
		for (; backindex < oldElementsSize; ++backindex)
		{
			m_vertData.m_vertexElements.push_back(lastElements[backindex]);
			vertOffBack += PixelUtil::GetPixelSize(lastElements[backindex].m_pixFmt);
		}

		size_t insertStrite = PixelUtil::GetPixelSize(element.m_pixFmt);
		allStride = vertOffFront + insertStrite + vertOffBack;
		m_vertData.m_stride = allStride;

		Byte* pNewVertexArray = EchoAlloc(Byte, allStride*m_vertData.m_count);
		Byte* pcopyDest = pNewVertexArray;
		Byte* pcopySrc = m_vertData.m_vertices;
		Byte* copyData = (Byte*)templateData;
		for (size_t vertIdx = 0; vertIdx < m_vertData.m_count; ++vertIdx)
		{
			memcpy(pcopyDest, pcopySrc, vertOffFront);
			pcopyDest += vertOffFront;
			pcopySrc += vertOffFront;
			memcpy(pcopyDest, copyData, insertStrite);
			pcopyDest += insertStrite;
			copyData += insertStrite;
			memcpy(pcopyDest, pcopySrc, vertOffBack);
			pcopyDest += vertOffBack;
			pcopySrc += vertOffBack;
		}

		EchoSafeFree(m_vertData.m_vertices);
		m_vertData.m_vertices = pNewVertexArray;

		Buffer vertBuff(m_vertData.m_stride*m_vertData.m_count, m_vertData.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
	}

	// 计算切线数据
	void Mesh::buildTangentData()
	{
		ui32 faceCount = getFaceCount();

		// 根据位置，UV数据构建切线数据
		vector<Vector3>::type tangentDatas;  tangentDatas.resize(m_vertData.m_count, Vector3::ZERO);
		vector<Vector3>::type binormalDatas;  binormalDatas.resize(m_vertData.m_count, Vector3::ZERO);
		for (ui32 i = 0; i < faceCount; i++)
		{
			// 仅支持TriangeList格式
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

		// 单位化切空间数据
		for (size_t i = 0; i < tangentDatas.size(); i++)
		{
			tangentDatas[i].normalize();
			binormalDatas[i].normalize();
		}

		// 新建切线数据
		insertVertexData(RenderInput::VertexElement(RenderInput::VS_TANGENT, PF_RGB32_FLOAT), tangentDatas.data());
		insertVertexData(RenderInput::VertexElement(RenderInput::VS_BINORMAL, PF_RGB32_FLOAT), binormalDatas.data());

		m_vertData.m_isUseTangentBinormal = true;
		m_vertData.build();
	}

	// 卸载
	void Mesh::clear()
	{
		EchoSafeFree(m_vertData.m_vertices);
		EchoSafeFree(m_indices);

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		EchoSafeDelete(m_indexBuffer, GPUBuffer);

		m_vertData.reset();
	}

	// 获取索引数量
	ui32 Mesh::getIndexCount() const
	{
		return m_idxCount;
	}

	// 获取面数量
	ui32 Mesh::getFaceCount() const
	{
		// triangle strip
		// return m_idxCount - 2;

		// triangle list
		return m_idxCount / 3;
	}

	// 获取索引数据步长
	ui32 Mesh::getIndexStride() const
	{
		return m_idxStride;
	}

	// 获取索引数据
	Word* Mesh::getIndices() const
	{
		return (Word*)m_indices;
	}

	// 计算占用内存大小
	size_t Mesh::getMemeoryUsage() const
	{
		return m_vertData.m_count*m_vertData.m_stride + m_idxCount*m_idxStride;
	}

	void Mesh::generateTangentData(bool useNormalMap)
	{
		if (useNormalMap)
		{
			buildTangentData();
		}
		else
		{
			m_vertData.m_isUseTangentBinormal = false;
			removeVertexData(RenderInput::VS_TANGENT);
			removeVertexData(RenderInput::VS_BINORMAL);
		}
	}

	// 获取顶点格式
	const RenderInput::VertexElementList& Mesh::getVertexElements() const
	{
		return m_vertData.m_vertexElements;
	}

	// 加载函数
	bool Mesh::buildBuffer()
	{
		buildIndexBuffer();
		buildVertexBuffer();

		return true;
	}

	// 建立索引缓冲
	void Mesh::buildIndexBuffer()
	{
		EchoSafeDelete(m_indexBuffer, GPUBuffer);
		Buffer indexBuff(m_idxCount*m_idxStride, m_indices);
		m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, indexBuff);
	}

	void Mesh::buildVertexBuffer()
	{
		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		Buffer vertBuff(m_vertData.m_stride*m_vertData.m_count, m_vertData.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
	}

	// 设置数据
	void Mesh::set(const Mesh::VertexDefine& format ,ui32 vertCount, const Byte* vertices, ui32 indicesCount, const ui16* indices, const Box& box)
	{
		// process vertex format define
		m_vertData.m_isUseNormal = format.m_isUseNormal;
		m_vertData.m_isUseVertexColor = format.m_isUseVertexColor;
		m_vertData.m_isUseDiffuseUV = format.m_isUseDiffuseUV;
		m_vertData.build();

		// process data
		EchoSafeFree(m_vertData.m_vertices);
		EchoSafeFree(m_indices);

		m_vertData.m_count = vertCount;

		// calc vertex buffer size
		ui32 vertBuffSize = m_vertData.m_count * m_vertData.m_stride;
		m_vertData.m_vertices = EchoAlloc(Byte, vertBuffSize);

		memcpy(m_vertData.m_vertices, vertices, vertCount * m_vertData.m_stride);

		// load indices
		m_idxCount = indicesCount;
		m_idxStride = sizeof(Word);

		ui32 idxBuffSize = m_idxCount * m_idxStride;
		m_indices = EchoAlloc(Byte, idxBuffSize);
		memcpy(m_indices, indices, idxBuffSize);
		m_box = box;

		buildBuffer();
	}
}