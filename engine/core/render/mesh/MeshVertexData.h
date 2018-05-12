#pragma once

#include "render/RenderInput.h"

namespace Echo
{
	struct MeshVertexFormat
	{
		typedef RenderInput::VertexElementList RIVEL;

		bool		m_isUseNormal;			// 是否使用法线
		bool		m_isUseVertexColor;		// 是否使用顶点色
		bool		m_isUseDiffuseUV;		// 是否使用漫反射贴图UV 
		bool		m_isUseLightmapUV;		// 是否使用光照图UV
		bool		m_isUseBoneData;		// 是否使用骨骼数据(索引权重)
		bool		m_isUseTangentBinormal;	// 是否使用切线与副线
		ui32		m_stride;				// 顶点格式大小
		Byte		m_posOffset;			// 顶点位置数据偏移量
		Byte		m_normalOffset;			// 顶点法线数据偏移量
		Byte		m_colorOffset;			// 顶点颜色数据偏移量
		Byte		m_uv0Offset;			// 顶点纹理坐标0偏移量
		Byte		m_uv1Offset;			// 顶点纹理坐标1偏移量
		Byte		m_boneIndicesOffset;	// 顶点骨骼索引偏移量
		Byte		m_boneWeightsOffset;	// 顶点骨骼权重偏移量
		Byte		m_tangentOffset;		// 切线数据偏移量
		RIVEL		m_vertexElements;		// 顶点格式

		MeshVertexFormat();

		// build
		void build();

		// 判断顶点格式中是否含有指定类型的数据
		bool MeshVertexFormat::isVertexUsage(RenderInput::VertexSemantic semantic) const;

		// reset
		void MeshVertexFormat::reset();
	};

	// 顶点数据
	class MeshVertexData
	{
	public:
		typedef vector<Byte>::type ByteArray;

		// 构造函数
		MeshVertexData();

		// set
		void set(const MeshVertexFormat& format, ui32 count);

		// get format
		const MeshVertexFormat& getFormat() const { return m_format; }

		// vertex info
		ui32 getVertexStride() const;
		ui32 getVertexCount() const;
		ui32 getByteSize() const { return m_count * m_format.m_stride; }

		// 获取顶点数据
		Byte* getVertices();

		// 获取顶点地址
		Byte* getVertice(int idx);

		// 判断顶点格式中是否含有指定类型的数据
		bool isVertexUsage(RenderInput::VertexSemantic semantic) const;

		// 获取顶点位置数据
		Vector3& getPosition(Word index);
		void setPosition(int idx, const Vector3& pos);

		// 获取顶点法线数据
		const Vector3& getNormal(Word index);

		// 获取顶点颜色数据
		Dword& getColor(Word index);

		// 获取顶点UV数据0
		const Vector2& getUV0(Word index);

		// 获取顶点UV数据1
		const Vector2& getUV1(Word index);

		// 获取切线
		Vector3& getTangent(Word index);

		//状态置空
		void reset();

	private:
		ui32				m_count;		// 顶点数量
		MeshVertexFormat	m_format;		// 顶点格式
		ByteArray			m_vertices;		// 顶点数据
	};
}