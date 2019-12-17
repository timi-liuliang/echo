#pragma once

#include "engine/core/geom/AABB.h"
#include "engine/core/render/base/image/PixelFormat.h"

namespace Echo
{
	enum VertexSemantic
	{
		VS_UNKNOWN = -1,
		VS_POSITION,                // Position (Vector3)
		VS_NORMAL,                  // Normal (Vector3)
		VS_COLOR,                   // Diffuse & specular color (Dword)
		VS_TEXCOORD0,               // Texture coordinates 0(Vector2)
		VS_TEXCOORD1,               // Texture coordinates 1(Vector2)
		VS_BLENDINDICES,            // Blending indices    (Dword)
		VS_BLENDWEIGHTS,            // Blending weights(Vector4)
		VS_TANGENT,                 // Tangent (X axis if normal is Z)
		VS_BINORMAL,
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
		bool		        m_isUseNormal;
		bool		        m_isUseVertexColor;
		bool		        m_isUseUV;
		bool		        m_isUseLightmapUV;
		bool		        m_isUseBoneData;
		bool		        m_isUseTangentBinormal;
		ui32		        m_stride;
		Byte		        m_posOffset;
		Byte		        m_normalOffset;
		Byte		        m_colorOffset;
		Byte		        m_uv0Offset;
		Byte		        m_uv1Offset;
		Byte		        m_boneIndicesOffset;
		Byte		        m_boneWeightsOffset;
		Byte		        m_tangentOffset;
		VertexElementList	m_vertexElements;

		MeshVertexFormat();

		// Build
		void build();

		// Is used
		bool isVertexUsage(VertexSemantic semantic) const;

		// reset
		void reset();
	};

	// vertex data for mesh
	class MeshVertexData
	{
    public:
        typedef vector<Byte>::type ByteArray;
        
	public:
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
		ui32				m_count;
		MeshVertexFormat	m_format;
		ByteArray			m_vertices;
		AABB				m_aabb;
	};
}
