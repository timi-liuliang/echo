/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_RENDER_MESH_ASSET_H
#define NX_RENDER_MESH_ASSET_H

/*!
\file
\brief APEX RenderMesh Asset
*/

#include "NxApexUsingNamespace.h"
#include "NxVertexFormat.h"
#include "NxApexAsset.h"
#include "NxApexRenderBufferData.h"
#include "NxRenderMesh.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxRenderMeshActor;
class NxRenderMeshActorDesc;
class NxApexCustomBufferIterator;

//! \brief Name of NxRenderMesh authoring type namespace
#define NX_RENDER_MESH_AUTHORING_TYPE_NAME "ApexRenderMesh"

/**
\brief Stats for an NxRenderMeshAsset: memory usage, counts, etc.
*/
struct NxRenderMeshAssetStats
{
	physx::PxU32	totalBytes;				//!< Total byte size of mesh
	physx::PxU32	submeshCount;			//!< Number of submeshes
	physx::PxU32	partCount;				//!< Number of mesh parts
	physx::PxU32	vertexCount;			//!< Number of vertices
	physx::PxU32	indexCount;				//!< Size (count) of index buffer
	physx::PxU32	vertexBufferBytes;		//!< Byte size of vertex buffer
	physx::PxU32	indexBufferBytes;		//!< Byte size of index buffer
};

/**
\brief Instance buffer data mode.  DEPRECATED, to be removed by APEX 1.0
*/
struct NxRenderMeshAssetInstanceMode
{
	/**
	\brief Enum of instance buffer data.
	*/
	enum Enum
	{
		POSE_SCALE = 0,
		POS_VEL_LIFE,

		NUM_MODES
	};
};


/**
\brief The full RGBA color of a vertex
*/
struct NxVertexColor
{
public:

	PX_INLINE					NxVertexColor()	{}

	/**
	\brief Constructor
	*/
	PX_INLINE					NxVertexColor(const PxColorRGBA c)
	{
		const physx::PxF32 recip255 = 1 / (PxReal)255;
		set((physx::PxF32)c.r * recip255, (physx::PxF32)c.g * recip255, (physx::PxF32)c.b * recip255, (physx::PxF32)c.a * recip255);
	}

	/**
	\brief Copy assignment operator
	*/
	PX_INLINE	NxVertexColor&	operator = (const NxVertexColor& c)
	{
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
		return *this;
	}

	/// \brief set the color as RGBA floats
	PX_INLINE	void			set(physx::PxF32 _r, physx::PxF32 _g, physx::PxF32 _b, physx::PxF32 _a)
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}

	/// \brief return the color as a 32bit integer
	PX_INLINE	PxColorRGBA		toColorRGBA() const
	{
		return PxColorRGBA((physx::PxU8)(255 * r + (physx::PxF32)0.5),
		                   (physx::PxU8)(255 * g + (physx::PxF32)0.5),
		                   (physx::PxU8)(255 * b + (physx::PxF32)0.5),
		                   (physx::PxU8)(255 * a + (physx::PxF32)0.5));
	}

	physx::PxF32 r;		//!< RED
	physx::PxF32 g;		//!< GREEN
	physx::PxF32 b;		//!< BLUE
	physx::PxF32 a;		//!< ALPHA
};


/**
\brief a simple u, v coordinate struct
*/
struct NxVertexUV
{
	NxVertexUV() {}

	/**
	\brief Constructor
	*/
	NxVertexUV(physx::PxF32 _u, physx::PxF32 _v)
	{
		set(_u, _v);
	}

	/**
	\brief Constructor
	*/
	NxVertexUV(const physx::PxF32 uv[])
	{
		set(uv);
	}

	/**
	\brief Set coordinates
	*/
	void			set(physx::PxF32 _u, physx::PxF32 _v)
	{
		u = _u;
		v = _v;
	}

	/**
	\brief Set coordinates
	*/
	void			set(const physx::PxF32 uv[])
	{
		u = uv[0];
		v = uv[1];
	}

	/**
	\brief operator []
	*/
	physx::PxF32&			operator [](int i)
	{
		PX_ASSERT(i >= 0 && i <= 1);
		return (&u)[i];
	}

	/**
	\brief const operator []
	*/
	const physx::PxF32&	operator [](int i) const
	{
		PX_ASSERT(i >= 0 && i <= 1);
		return (&u)[i];
	}

	/// coordinate
	physx::PxF32	u;
	/// coordinate
	physx::PxF32	v;
};


/**
\brief An inefficient vertex description used for authoring
*/
struct NxVertex
{
	physx::PxVec3	position;		//!< Vertex position
	physx::PxVec3	normal;			//!< Surface normal at this position
	physx::PxVec3	tangent;		//!< Surface tangent at this position
	physx::PxVec3	binormal;		//!< Surface binormal at this position
	NxVertexUV		uv[NxVertexFormat::MAX_UV_COUNT]; //!< Texture UV coordinates
	NxVertexColor	color;			//!< Color ar this position
	physx::PxU16	boneIndices[NxVertexFormat::MAX_BONE_PER_VERTEX_COUNT]; //!< Bones which are attached to this vertex
	physx::PxF32	boneWeights[NxVertexFormat::MAX_BONE_PER_VERTEX_COUNT]; //!< Per bone wieght, 0.0 if no bone
	physx::PxU16	displacementFlags; //!< Flags for vertex displacement

	/**
	\brief Constructor which clears the entire structure
	*/
	NxVertex()
	{
		memset(this, 0, sizeof(NxVertex));
	}
};


/**
\brief A less inefficient triangle description used for authoring
*/
struct NxExplicitRenderTriangle
{
	NxVertex		vertices[3];		//!< The three verts that define the triangle
	physx::PxI32	submeshIndex;		//!< The submesh to which this triangle belongs
	physx::PxU32	smoothingMask;		//!< Smoothing mask
	physx::PxU32	extraDataIndex;		//!< Index of extra data

	/**
	\brief Returns an unnormalized normal, in general
	*/
	physx::PxVec3	calculateNormal() const
	{
		return (vertices[1].position - vertices[0].position).cross(vertices[2].position - vertices[0].position);
	}
};



/**
\brief Descriptor for creating a rendering mesh part
*/
struct NxRenderMeshPartData
{
	NxRenderMeshPartData() : triangleCount(0), userData(NULL) {}

	/**
	\brief Constructor
	*/
	NxRenderMeshPartData(physx::PxU32 _triCount, void* _data) : triangleCount(_triCount), userData(_data) {}

	physx::PxU32	triangleCount;	//!< Number of triangles in this mesh part
	void*			userData;		//!< User definable pointer to part data, passed back to createTriangles
};


/**
\brief Authoring interface for an NxRenderMeshAsset
*/
class NxRenderMeshAssetAuthoring : public NxApexAssetAuthoring
{
public:
	/** \brief Vertex buffer class used for mesh creation */
	class VertexBuffer : public NxApexRenderBufferData<NxRenderVertexSemantic, NxRenderVertexSemantic::Enum> {};

	/** \brief How the geometry is stored.  Currently only supporting triangles. */
	struct Primitive
	{
		/**
		\brief Enum of geometry stored types.
		*/
		enum Enum
		{
			TRIANGLE_LIST,
			//		TRIANGLE_STRIP, // Not supported for now
			//		TRIANGLE_FAN,	// Not supported for now

			COUNT
		};
	};

	/** What kind of integer is used for indices. */
	struct IndexType
	{
		/**
		\brief Enum of integers types using for indices.
		*/
		enum Enum
		{
			UINT,
			USHORT,

			COUNT
		};
	};

	/** Description of one submesh, corresponding to one material.  The vertex buffer format contains bone indices, so these do not need
	    to be described here.  The submesh's partitioning into parts is described here. */
	class SubmeshDesc
	{
	public:
		/** Name of material associated with this geometry. */
		const char*				m_materialName;

		/** Vertex buffers for this submesh. One may pass in the same buffers for each submesh. */
		const VertexBuffer*		m_vertexBuffers;

		/** Number of vertex buffers in m_VertexBuffers array. */
		physx::PxU32			m_numVertexBuffers;

		/** Number of vertices.  Each vertex buffer in m_VertexBuffers must have this many vertices. */
		physx::PxU32			m_numVertices;

		/** How the geometry is represented.  See the Primitive enum. */
		Primitive::Enum			m_primitive;

		/** Type of the indices used in m_VertexIndices. See the IndexType enum. */
		IndexType::Enum			m_indexType;

		/** Buffer of vertex indices, stored as described by primitive and indexSize. If NULL, m_vertexIndices = {0,1,2,...} is implied. */
		const void*				m_vertexIndices;

		/** Size (in indices) of m_VertexIndices. */
		physx::PxU32			m_numIndices;

		/**
			Smoothing groups associated with each triangle.  The size of this array (if not NULL) must be appropriate for the m_primitive type.
			Since only triangle lists are currently supported, the size of this array (if not NULL) must currently be m_numIndices/3.
		*/
		physx::PxU32*			m_smoothingGroups;

		/** Vertex index offset. */
		physx::PxU32			m_firstVertex;

		/** If not NULL, an array (of m_IndexType-sized indices) into m_VertexIndices, at the start of each part. */
		const void*				m_partIndices;

		/** If m_PartIndices is not NULL, the number of parts. */
		physx::PxU32			m_numParts;

		/** Winding order of the submesh */
		NxRenderCullMode::Enum	m_cullMode;

		/** Constructor sets default values. */
		SubmeshDesc()
		{
			memset(this, 0, sizeof(SubmeshDesc));
		}

		/** Validity check, returns true if this descriptor contains valid fields. */
		bool	isValid() const
		{
			return	m_materialName != NULL &&
			        m_vertexBuffers != NULL &&	// BRG - todo: check the vertex buffers for validity
			        m_numVertexBuffers > 0 &&
			        m_numVertices > 0 &&
			        m_primitive >= (Primitive::Enum)0 && m_primitive < Primitive::COUNT &&
			        m_indexType >= (IndexType::Enum)0 && m_indexType < IndexType::COUNT &&
			        m_numIndices > 0 &&
			        (m_partIndices == NULL || m_numParts > 0) &&
			        (m_cullMode == NxRenderCullMode::CLOCKWISE || m_cullMode == NxRenderCullMode::COUNTER_CLOCKWISE || m_cullMode == NxRenderCullMode::NONE);
		}
	};

	/** Description of a mesh, used for authoring an NxApexRender mesh.  It contains a number of vertex buffers and submeshes. */
	class MeshDesc
	{
	public:
		/** Array of descriptors for the submeshes in this mesh. */
		const SubmeshDesc*		m_submeshes;

		/** The number of elements in m_submeshes. */
		physx::PxU32			m_numSubmeshes;

		/** Texture UV direction. */
		NxTextureUVOrigin::Enum	m_uvOrigin;



		/** Constructor sets default values. */
		MeshDesc() : m_submeshes(NULL), m_numSubmeshes(0), m_uvOrigin(NxTextureUVOrigin::ORIGIN_TOP_LEFT) {}

		/** Validity check, returns true if this descriptor contains valid fields. */
		bool	isValid() const
		{
			return	m_submeshes != NULL &&
			        m_numSubmeshes > 0;
		}
	};


	/**
	\brief Mesh-building function.
	\param [in] meshDesc					contains the setup for all vertex buffers
	\param [in] createMappingInformation	A vertex buffer with remapping indices will be generated. The name of the buffer is VERTEX_ORIGINAL_INDEX
	*/
	virtual void			createRenderMesh(const MeshDesc& meshDesc, bool createMappingInformation) = 0;


	/**
	Utility to reduce a vertex buffer of explicit vertices (NxVertex).
	The parameters 'map' and 'vertices' must point to arrays of size vertexCount.
	The parameter 'smoothingGroups' must point to an array of size vertexCount, or be NULL.  If not NULL, only vertices with equal smoothing groups will be merged.
	Upon return, the map array will be filled in with remapped vertex positions for a new vertex buffer.
	The return value is the number of vertices in the reduced buffer.
	Note: this function does NOT actually create the new vertex buffer.
	*/
	virtual physx::PxU32	createReductionMap(PxU32* map, const NxVertex* vertices, const PxU32* smoothingGroups, PxU32 vertexCount,
	        const physx::PxVec3& positionTolerance, physx::PxF32 normalTolerance, physx::PxF32 UVTolerance) = 0;


	/**
	If set, static data buffers will be deleted after they are used in createRenderResources.
	*/
	virtual void			deleteStaticBuffersAfterUse(bool set)	= 0;

	/**
	Old mesh-building interface follows (DEPRECATED, to be removed by beta release):
	*/

	/* Public access to NxRenderMeshAsset get methods */

	/// \brief Return the number of submeshes
	virtual physx::PxU32			getSubmeshCount() const = 0;
	/// \brief Return the number of mesh parts
	virtual physx::PxU32			getPartCount() const = 0;
	/// \brief Return the name of a submesh
	virtual const char*				getMaterialName(physx::PxU32 submeshIndex) const = 0;
	/// \brief Set the name of a submesh
	virtual void					setMaterialName(physx::PxU32 submeshIndex, const char* name) = 0;
	/// \brief Set the winding order of a submesh
	virtual void					setWindingOrder(physx::PxU32 submeshIndex, NxRenderCullMode::Enum winding) = 0;
	/// \brief Return the winding order of a submesh
	virtual NxRenderCullMode::Enum	getWindingOrder(physx::PxU32 submeshIndex) const = 0;
	/// \brief Return a submesh
	virtual const NxRenderSubmesh&	getSubmesh(physx::PxU32 submeshIndex) const = 0;
	/// \brief Return a mutable submesh
	virtual NxRenderSubmesh&		getSubmeshWritable(physx::PxU32 submeshIndex) = 0;
	/// \brief Return the bounds of a mesh part
	virtual const physx::PxBounds3&	getBounds(physx::PxU32 partIndex = 0) const = 0;
	/// \brief Get the asset statistics
	virtual void					getStats(NxRenderMeshAssetStats& stats) const = 0;
};


/**
\brief Rendering mesh (data) class.

To render a mesh asset, you must create an instance
*/
class NxRenderMeshAsset : public NxApexAsset
{
public:

	/**
	\brief Instance this asset, return the created NxRenderMeshActor.

	See NxRenderMeshActor
	*/
	virtual NxRenderMeshActor*		createActor(const NxRenderMeshActorDesc& desc) = 0;

	/**
	\brief Releases an NxRenderMeshActor instanced by this asset.
	*/
	virtual void					releaseActor(NxRenderMeshActor&) = 0;

	/**
	\brief Number of submeshes.

	Each part effectively has the same number of submeshes, even if some are empty.
	*/
	virtual physx::PxU32			getSubmeshCount() const = 0;

	/**
	\brief Number of parts.

	These act as separate meshes, but they share submesh data (like materials).
	*/
	virtual physx::PxU32			getPartCount() const = 0;

	/**
	\brief Returns an array of length submeshCount()
	*/
	virtual const char*				getMaterialName(physx::PxU32 submeshIndex) const = 0;

	/**
	\brief Returns an submesh

	A submesh contains all the triangles in all parts with the same material
	(indexed by submeshIndex)
	*/
	virtual const NxRenderSubmesh&	getSubmesh(physx::PxU32 submeshIndex) const = 0;

	/**
	\brief Returns the axis-aligned bounding box of the vertices for the given part.

	Valid range of partIndex is {0..partCount()-1}
	*/
	virtual const physx::PxBounds3&	getBounds(physx::PxU32 partIndex = 0) const = 0;

	/**
	\brief Returns stats (sizes, counts) for the asset.

	See NxRenderMeshAssetStats.
	*/
	virtual void					getStats(NxRenderMeshAssetStats& stats) const = 0;

protected:
	virtual							~NxRenderMeshAsset() {}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_RENDER_MESH_ASSET_H
