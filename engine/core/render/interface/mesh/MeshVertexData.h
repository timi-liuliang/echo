#pragma once

#include "engine/core/geom/AABB.h"
#include "engine/core/render/interface/image/PixelFormat.h"

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
		// Blending weights(Vector4)
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

		bool		m_isUseNormal;
		bool		m_isUseVertexColor;
		bool		m_isUseUV;
		bool		m_isUseLightmapUV;
		bool		m_isUseBoneData;
		bool		m_isUseTangentBinormal;
		ui32		m_stride;
		Byte		m_posOffset;
		Byte		m_normalOffset;
		Byte		m_colorOffset;
		Byte		m_uv0Offset;
		Byte		m_uv1Offset;
		Byte		m_boneIndicesOffset;
		Byte		m_boneWeightsOffset;
		Byte		m_tangentOffset;
		RIVEL		m_vertexElements;

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
		void setColor(i32 idx, Dword color);

		// 获取顶点UV数据0
		const Vector2& getUV0(Word index);
		void setUV0(int idx, const Vector2& uv0);

		// 获取顶点UV数据1
		const Vector2& getUV1(Word index);

		 // set skin weight
		void setJoint(int idx, Dword weight);

		// set skin joint
		void setWeight(int idx, const Vector4& joint);

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
		AABB				m_aabb;			// 轴向包围盒
	};
}
