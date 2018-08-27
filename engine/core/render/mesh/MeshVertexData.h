#pragma once

#include "core/geom/AABB.h"
#include "core/render/render/PixelFormat.h"

namespace Echo
{
	enum VertexSemantic
	{
		VS_UNKNOWN = -1,
		// Position (Vector3)
		VS_POSITION,
		// Normal (Vector3)
		VS_NORMAL,
		// Diffuse & specular color (Dword)
		VS_COLOR,
		// Texture coordinates 0(Vector2)
		VS_TEXCOORD0,
		// Texture coordinates 1(Vector2)
		VS_TEXCOORD1,
		// Blending indices	(Dword)
		VS_BLENDINDICES,
		// Blending weights(Vector3)
		VS_BLENDWEIGHTS,
		// Tangent (X axis if normal is Z)
		VS_TANGENT,
		VS_BINORMAL,

		// VR Mode
		VS_SCREEN_POS_NDC,
		VS_TAN_EYE_ANGLE_R,
		VS_TAN_EYE_ANGLE_G,
		VS_TAN_EYE_ANGLE_B,

		VS_MAX
	};

	struct VertexElement
	{
		VertexSemantic		m_semantic;		// Vertex Semantic
		PixelFormat			m_pixFmt;		// Vertex pixel format

		VertexElement(VertexSemantic semantic = VS_UNKNOWN, PixelFormat pixFmt = PF_UNKNOWN)
			: m_semantic(semantic)
			, m_pixFmt(pixFmt)
		{}
	};
	typedef vector<VertexElement>::type	VertexElementList;

	struct MeshVertexFormat
	{
		typedef VertexElementList RIVEL;

		bool		m_isUseNormal;			// 是否使用法线
		bool		m_isUseVertexColor;		// 是否使用顶点色
		bool		m_isUseUV;				// 是否使用漫反射贴图UV 
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
		bool isVertexUsage(VertexSemantic semantic) const;

		// reset
		void reset();
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
		bool isVertexUsage(VertexSemantic semantic) const;

		// 获取顶点位置数据
		Vector3& getPosition(Word index);
		void setPosition(int idx, const Vector3& pos);

		// 获取顶点法线数据
		const Vector3& getNormal(Word index);
		void setNormal(int idx, const Vector3& normal);

		// 获取顶点颜色数据
		Dword& getColor(Word index);

		// 获取顶点UV数据0
		const Vector2& getUV0(Word index);
		void setUV0(int idx, const Vector2& uv0);

		// 获取顶点UV数据1
		const Vector2& getUV1(Word index);

		// 获取切线
		Vector3& getTangent(Word index);

		// 获取包围盒
		const AABB& getAABB() const { return m_aabb; }

		//状态置空
		void reset();

	private:
		ui32				m_count;		// 顶点数量
		MeshVertexFormat	m_format;		// 顶点格式
		ByteArray			m_vertices;		// 顶点数据
		AABB					m_aabb;			// 轴向包围盒
	};
}
