#include "Engine/modules/Model/SubMesh.h"
#include "Render/Renderer.h"
#include <algorithm>

namespace Echo
{
	// 构造函数
	SubMesh::VertexInfo::VertexInfo()
		: m_count(0)
		, m_isUseNormal(false)
		, m_isUseVertexColor(false)
		, m_isUseDiffuseUV(false)
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
		, m_vertices(NULL)
	{}

	// 计算偏移量,顶点格式
	void SubMesh::VertexInfo::build()
	{
		m_posOffset = 0;
		m_normalOffset = m_posOffset + sizeof(Vector3);
		m_colorOffset = m_normalOffset + (m_isUseNormal ? sizeof(Vector3) : 0);
		m_uv0Offset = m_colorOffset + (m_isUseVertexColor ? sizeof(Dword) : 0);
		m_uv1Offset = m_uv0Offset + (m_isUseDiffuseUV ? sizeof(Vector2) : 0);
		m_boneIndicesOffset = m_uv1Offset + (m_isUseLightmapUV ? sizeof(Vector2) : 0);
		m_boneWeightsOffset = m_boneIndicesOffset + (m_isUseBoneData ? sizeof(Dword) : 0);
		m_tangentOffset = m_boneWeightsOffset + (m_isUseBoneData ? sizeof(Vector3) : 0);
		m_stride = m_tangentOffset + (m_isUseTangentBinormal ? sizeof(Vector3)*2 : 0);

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
		if (m_isUseDiffuseUV)
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

	// 获取顶点格式大小
	ui32 SubMesh::VertexInfo::getVertexStride() const
	{
		return m_stride;
	}

	// 获取顶点数量
	ui32 SubMesh::VertexInfo::getVertexCount() const
	{
		return m_count;
	}

	// 获取顶点数据
	Byte* SubMesh::VertexInfo::getVertices() const
	{
		return m_vertices;
	}

	// 判断顶点格式中是否含有指定类型的数据
	bool SubMesh::VertexInfo::isVertexUsage(RenderInput::VertexSemantic semantic) const
	{
		size_t num = m_vertexElements.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (m_vertexElements[i].m_semantic == semantic)
				return true;
		}

		return false;
	}

	// 获取顶点位置数据
	Vector3& SubMesh::VertexInfo::getPosition(Word index)
	{
		if (m_positions.empty())
		{ 
			EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_POSITION));

			return *(Vector3*)((Byte*)m_vertices + m_stride*index + m_posOffset);
		}
		else
		{
			return m_positions[index];
		}
	}

	// 获取顶点法线数据
	const Vector3& SubMesh::VertexInfo::getNormal(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_NORMAL));

		return *(Vector3*)((Byte*)m_vertices + m_stride*index + m_normalOffset);
	}

	// 获取顶点颜色数据
	Dword& SubMesh::VertexInfo::getColor(Word index)
	{
		EchoAssert(index < m_count && RenderInput::VS_COLOR);

		return *(Dword*)((Byte*)m_vertices + m_stride*index + m_colorOffset);
	}

	// 获取顶点UV数据0
	const Vector2& SubMesh::VertexInfo::getUV0(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_TEXCOORD0));

		return *(Vector2*)((Byte*)m_vertices + m_stride*index + m_uv0Offset);
	}

	// 获取顶点UV数据1
	const Vector2& SubMesh::VertexInfo::getUV1(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_TEXCOORD1));

		return *(Vector2*)((Byte*)m_vertices + m_stride*index + m_uv1Offset);
	}

	// 获取切线
	Vector3& SubMesh::VertexInfo::getTangent(Word index)
	{
		EchoAssert(index < m_count && isVertexUsage(RenderInput::VS_TANGENT));

		return *(Vector3*)((Byte*)m_vertices + m_stride*index + m_tangentOffset);
	}

	// 记录位置数据
	void SubMesh::VertexInfo::copyPositions()
	{
		vector<Vector3>::type positions;
		positions.resize(getVertexCount());
		m_positions.clear();
		for (ui32 i = 0; i < getVertexCount(); i++)
		{
			positions[i] = getPosition(i);
		}

		std::swap(m_positions, positions);
	}

	void SubMesh::VertexInfo::reset()
	{
		m_count = 0;
		m_isUseNormal = false;
		m_isUseVertexColor = false;
		m_isUseDiffuseUV = false;
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
		m_vertices = NULL;
	}

	// 构造函数
	SubMesh::SubMesh()
		: m_pDiffTex(NULL)
		, m_vertexBuffer(NULL)
		, m_indexBuffer(NULL)
		, m_idxCount(0)
		, m_idxStride(0)
		, m_indices(NULL)
		, m_pMaskTex(NULL)
		, m_bHalfFloat(false)
		, m_visible(true)
		, m_is_muti_vertex_buff(false)
	{
	}

	// 析构函数
	SubMesh::~SubMesh()
	{
		unloadImpl();
	}

	// 获取漫反射纹理
	TextureRes* SubMesh::getDiffuseTexture() const
	{
		return m_pDiffTex;
	}

	// get mask texture
	TextureRes* SubMesh::getMasktexture() const
	{
		return m_pMaskTex;
	}

	// 获取顶点缓冲
	GPUBuffer* SubMesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	// 获取索引缓冲
	GPUBuffer* SubMesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	// 加载函数
	bool SubMesh::load()
	{
		if (m_pDiffTex)
		{
			m_pDiffTex->load();
		}
		
		if (m_pMaskTex)
		{
			m_pMaskTex->load();
		}

		Buffer vertBuff(m_vertInfo.m_stride*m_vertInfo.m_count, m_vertInfo.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);

		Buffer indexBuff(m_idxCount*m_idxStride, m_indices);
		m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, indexBuff);

		return true;
	}

	// 移除数据
	void SubMesh::dataRemove(RenderInput::VertexSemantic semantic)
	{
		if (!m_vertInfo.isVertexUsage(semantic))
			return;

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);

		RenderInput::VertexElementList mlastElements = m_vertInfo.m_vertexElements;
		m_vertInfo.m_vertexElements.clear();
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
				m_vertInfo.m_vertexElements.push_back(mlastElements[j]);
				allStrite += PixelUtil::GetPixelSize(mlastElements[j].m_pixFmt);
			}
		}
		vertOffFront = allStrite;
		j++;
		for (; j < oldElementsSize; ++j)
		{
			m_vertInfo.m_vertexElements.push_back(mlastElements[j]);
			allStrite += PixelUtil::GetPixelSize(mlastElements[j].m_pixFmt);
			vertOffBack += PixelUtil::GetPixelSize(mlastElements[j].m_pixFmt);
		}

		m_vertInfo.m_stride = allStrite;

		Byte* pNewVertexArray = EchoAlloc(Byte, allStrite*m_vertInfo.m_count);
		Byte* pcopyDest = pNewVertexArray;
		Byte* pcopySrc = m_vertInfo.m_vertices;
		for (size_t vertIdx = 0; vertIdx < m_vertInfo.m_count; ++vertIdx)
		{
			memcpy(pcopyDest, pcopySrc, vertOffFront);
			pcopyDest += vertOffFront;
			pcopySrc += vertOffFront + removeStrite;
			memcpy(pcopyDest, pcopySrc, vertOffBack);
			pcopyDest += vertOffBack;
			pcopySrc += vertOffBack;
		}

		EchoSafeFree(m_vertInfo.m_vertices);
		m_vertInfo.m_vertices = pNewVertexArray;

		Buffer vertBuff(m_vertInfo.m_stride*m_vertInfo.m_count, m_vertInfo.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
	}

	// 插入数据
	void SubMesh::dataInsert(const RenderInput::VertexElement& element, void* templateData)
	{
		if (isVertexUsage(element.m_semantic))
			return;

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);

		RenderInput::VertexElementList lastElements = m_vertInfo.m_vertexElements;
		m_vertInfo.m_vertexElements.clear();

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
			m_vertInfo.m_vertexElements.push_back(lastElements[j]);
			vertOffFront += PixelUtil::GetPixelSize(lastElements[j].m_pixFmt);
		}

		m_vertInfo.m_vertexElements.push_back(element);

		/// back
		for (; backindex < oldElementsSize; ++backindex)
		{
			m_vertInfo.m_vertexElements.push_back(lastElements[backindex]);
			vertOffBack += PixelUtil::GetPixelSize(lastElements[backindex].m_pixFmt);
		}

		size_t insertStrite = PixelUtil::GetPixelSize(element.m_pixFmt);
		allStride = vertOffFront + insertStrite + vertOffBack;
		m_vertInfo.m_stride = allStride;

		Byte* pNewVertexArray = EchoAlloc(Byte, allStride*m_vertInfo.m_count);
		Byte* pcopyDest = pNewVertexArray;
		Byte* pcopySrc = m_vertInfo.m_vertices;
		Byte* copyData = (Byte*)templateData;
		for (size_t vertIdx = 0; vertIdx < m_vertInfo.m_count; ++vertIdx)
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

		EchoSafeFree(m_vertInfo.m_vertices);
		m_vertInfo.m_vertices = pNewVertexArray;

		Buffer vertBuff(m_vertInfo.m_stride*m_vertInfo.m_count, m_vertInfo.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
	}

	// 计算切线数据
	void SubMesh::buildTangentData()
	{
		ui32 faceCount = getFaceCount();

		// 根据位置，UV数据构建切线数据
		vector<Vector3>::type tangentDatas;  tangentDatas.resize(m_vertInfo.m_count, Vector3::ZERO);
		vector<Vector3>::type binormalDatas;  binormalDatas.resize(m_vertInfo.m_count, Vector3::ZERO);
		for (ui32 i = 0; i < faceCount; i++)
		{
			// 仅支持TriangeList格式
			Word baseIdx = i * 3;
			Word vertIdx0 = ((Word*)m_indices)[baseIdx + 0];
			Word vertIdx1 = ((Word*)m_indices)[baseIdx + 1];
			Word vertIdx2 = ((Word*)m_indices)[baseIdx + 2];

			const Vector3& pos0 = m_vertInfo.getPosition(vertIdx0);
			const Vector3& pos1 = m_vertInfo.getPosition(vertIdx1);
			const Vector3& pos2 = m_vertInfo.getPosition(vertIdx2);

			const Vector2& uv0 = m_vertInfo.getUV0(vertIdx0);
			const Vector2& uv1 = m_vertInfo.getUV0(vertIdx1);
			const Vector2& uv2 = m_vertInfo.getUV0(vertIdx2);

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
		dataInsert(RenderInput::VertexElement(RenderInput::VS_TANGENT, PF_RGB32_FLOAT), tangentDatas.data());
		dataInsert(RenderInput::VertexElement(RenderInput::VS_BINORMAL, PF_RGB32_FLOAT), binormalDatas.data());

		m_vertInfo.m_isUseTangentBinormal = true;
		m_vertInfo.build();
	}

	// 卸载
	void SubMesh::unloadImpl()
	{
		EchoSafeFree(m_vertInfo.m_vertices);
		EchoSafeFree(m_indices);

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		EchoSafeDelete(m_indexBuffer, GPUBuffer);

		m_vertInfo.reset();

		if (m_pDiffTex)
		{
			TextureResManager::instance()->releaseResource(m_pDiffTex);
			m_pDiffTex = NULL;
		}

		if (m_pMaskTex)
		{
			TextureResManager::instance()->releaseResource(m_pMaskTex);
			m_pMaskTex = NULL;
		}

		if (m_is_muti_vertex_buff)
		{
			itrMutiVertexBuff itr = m_muti_vertex_buff.begin();
			for (; itr != m_muti_vertex_buff.end(); itr++)
			{
				GPUBuffer* pgpubuff = itr->m_gpubuff;
				EchoSafeDelete(pgpubuff, GPUBuffer);
			}
			m_muti_vertex_buff.clear();
		}

	}

	// 获取索引数量
	ui32 SubMesh::getIndexCount() const
	{
		return m_idxCount;
	}

	// 获取面数量
	ui32 SubMesh::getFaceCount() const
	{
		// triangle strip
		// return m_idxCount - 2;

		// triangle list
		return m_idxCount / 3;
	}

	// 获取索引数据步长
	ui32 SubMesh::getIndexStride() const
	{
		return m_idxStride;
	}

	// 获取索引数据
	Word* SubMesh::getIndices() const
	{
		return (Word*)m_indices;
	}

	// 干什么用的呀
	void SubMesh::setVertexStructInfo(bool isSkinned, bool useHalfFloat, std::vector<int>& floatPos)
	{
		Echo::PixelFormat pf_float3 = PF_RGB32_FLOAT;
		Echo::PixelFormat pf_float2 = PF_RG32_FLOAT;
		if (useHalfFloat)
		{
			pf_float3 = PF_RGB16_FLOAT;
			pf_float2 = PF_RG16_FLOAT;

			m_bHalfFloat = true;
		}

		int curFloatNum = 0;
		floatPos.clear();

		// 增加位置的空间
		curFloatNum += 3;

		// 使用法线
		if (m_vertInfo.m_isUseNormal)
		{
			curFloatNum += 3;
		}

		// 增加顶点色
		if (m_vertInfo.m_isUseVertexColor)
		{
			floatPos.push_back(curFloatNum);
			curFloatNum = 0;
			floatPos.push_back(-4);
		}

		// 纹理坐标
		curFloatNum += 2;

		// 灯光图
		if (m_vertInfo.m_isUseLightmapUV)
		{
			curFloatNum += 2;
		}

		// 动画数据(骨骼权重与索引)
		if (isSkinned)
		{
			//boneIndex
			m_vertInfo.m_stride += PixelUtil::GetPixelSize(PF_RGBA8_UINT);
			floatPos.push_back(curFloatNum);
			curFloatNum = 0;
			floatPos.push_back(-4);

			//boneWeight
			m_vertInfo.m_stride += PixelUtil::GetPixelSize(pf_float3);
			curFloatNum += 3;
		}

		floatPos.push_back(curFloatNum);
	}

	// 计算占用内存大小
	size_t SubMesh::calculateSize() const
	{
		return m_vertInfo.m_count*m_vertInfo.m_stride + m_idxCount*m_idxStride;
	}

	// ...
	void SubMesh::unprepareImpl()
	{
#ifndef ECHO_EDITOR_MODE
		if (m_vertInfo.m_isUseBoneData)
		{
			// 带动画模型直接移除所有数据
			EchoSafeFree(m_vertInfo.m_vertices);
			m_vertInfo.reset();
			EchoSafeFree(m_indices)
		}
		else
		{
			// 静态模型保留索引与位置数据
			m_vertInfo.copyPositions();
			EchoSafeFree(m_vertInfo.m_vertices);
		}
#endif
	}

	void SubMesh::generateTangentData(bool useNormalMap)
	{
		if (useNormalMap)
		{
			buildTangentData();
		}
		else
		{
			m_vertInfo.m_isUseTangentBinormal = false;
			dataRemove(RenderInput::VS_TANGENT);
			dataRemove(RenderInput::VS_BINORMAL);
		}
	}

	void SubMesh::removeRedundancyVertex()
	{
		Echo::Byte* vertives = m_vertInfo.getVertices();
		Echo::vector<int>::type toRemove;
		for (unsigned int i = 0; i < m_vertInfo.getVertexCount(); i++)
		{
			if (std::find(toRemove.begin(), toRemove.end(), i)==toRemove.end())
			{
				for (unsigned int j = i + 1; j < m_vertInfo.getVertexCount(); j++)
				{
					// 如果顶点相同
					if (m_vertInfo.getPosition(i) == m_vertInfo.getPosition(j) &&
						(m_vertInfo.isVertexUsage(RenderInput::VS_TEXCOORD0) ?  m_vertInfo.getUV0(i) == m_vertInfo.getUV0(j): true) &&
						(m_vertInfo.isVertexUsage(RenderInput::VS_TEXCOORD1) ? m_vertInfo.getUV1(i) == m_vertInfo.getUV1(j): true))
					{
						int id = j;

						// 记录哪个顶点将被移除
						toRemove.push_back(id);

						// 遍历每一个索引
						for (unsigned int k = 0; k < getIndexCount(); k++)
						{
							Word idx = ((Word*)m_indices)[k];
							if (idx == id)
							{
								// 重新定位索引
								((Word*)m_indices)[k] = i;
							}
						}
					}
				}
			}
			
		}

		// 重新组织索引数组
		for (unsigned int m = 0; m < getIndexCount(); m++)
		{
			Word idx = ((Word*)m_indices)[m];

			auto it = toRemove.begin();
			for (; it != toRemove.end(); it++)
			{
				if ((*it) < idx)
					((Word*)m_indices)[m]--;
			}
		}

		// 重新构造定点数组
		int size = m_vertInfo.m_count - toRemove.size();
		Byte* pNewVertexArray = EchoAlloc(Byte, m_vertInfo.m_stride*size);
		Byte* pcopyDest = pNewVertexArray;
		Byte* pcopySrc = m_vertInfo.m_vertices;
		for (size_t vertIdx = 0; vertIdx < m_vertInfo.m_count; ++vertIdx)
		{
			auto it = toRemove.begin();
			for (; it != toRemove.end(); it++)
			{
				if ((*it) == vertIdx)
					break;
			}
			if (it == toRemove.end())
			{
				memcpy(pcopyDest, pcopySrc, m_vertInfo.m_stride);
				pcopyDest += m_vertInfo.m_stride;
			}
			pcopySrc += m_vertInfo.m_stride;
		}

		EchoSafeFree(m_vertInfo.m_vertices);
		m_vertInfo.m_vertices = pNewVertexArray;
		m_vertInfo.m_count = size;

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		EchoSafeDelete(m_indexBuffer, GPUBuffer);

		Buffer vertBuff(m_vertInfo.m_stride*m_vertInfo.m_count, m_vertInfo.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);

		Buffer indexBuff(m_idxCount*m_idxStride, m_indices);
		m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, indexBuff);

	}

	// 获取顶点格式
	const RenderInput::VertexElementList& SubMesh::getVertexElements() const
	{
		return m_vertInfo.m_vertexElements;
	}

	// 获取本地包围盒
	const Box& SubMesh::getLocalBox() const
	{
		return m_box;
	}

	void SubMesh::reBuildVertexBuffer()
	{
		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		Buffer vertBuff(m_vertInfo.m_stride*m_vertInfo.m_count, m_vertInfo.m_vertices);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
	}

}