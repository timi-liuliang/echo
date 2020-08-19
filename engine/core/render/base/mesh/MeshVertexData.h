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
		VS_BLENDINDICES,            // Blending indices    (Dword) Bone + Height field
		VS_BLENDWEIGHTS,            // Blending weights(Vector4) Bone + Height field
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
		bool		        m_isUseBlendingData;
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
		MeshVertexData();

		// set
		void set(const MeshVertexFormat& format, ui32 count);

		// get format
		const MeshVertexFormat& getFormat() const { return m_format; }

		// vertex info
		ui32 getVertexStride() const;
		ui32 getVertexCount() const;
		ui32 getByteSize() const { return m_count * m_format.m_stride; }

		// data point
		Byte* getVertices();
		Byte* getVertice(int idx);

		// is Vertex use
		bool isVertexUsage(VertexSemantic semantic) const;

		// Position
		Vector3& getPosition(Word index);
		void setPosition(int idx, const Vector3& pos);

		// Normal
		const Vector3& getNormal(Word index);
		void setNormal(int idx, const Vector3& normal);

		// Color
		Dword& getColor(Word index);
		void setColor(i32 idx, Dword color);

		// UV0
		const Vector2& getUV0(Word index);
		void setUV0(int idx, const Vector2& uv0);

		// UV1
		const Vector2& getUV1(Word index);

		 // set skin weight
		void setJoint(int idx, Dword weight);

		// set skin joint
		void setWeight(int idx, const Vector4& joint);

		// tangent
		Vector3& getTangent(Word index);

		// reset
		void reset();

	public:
		// get data
		ByteArray getPositions();
		ByteArray getNormals();
		ByteArray getUV0s();

	private:
		ui32				m_count;
		MeshVertexFormat	m_format;
		ByteArray			m_vertices;
	};
}
