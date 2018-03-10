/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef FRACTURING_H

#define FRACTURING_H

#include "NxApex.h"
#include "PsShare.h"
#include "foundation/PxPlane.h"
//#include "ApexSharedSerialization.h"
#include "FractureTools.h"
#include "ApexString.h"
#include "NxExplicitHierarchicalMesh.h"
#include "authoring/ApexCSG.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace physx
{
namespace apex
{

using namespace FractureTools;


struct IntersectMesh
{
	enum GridPattern
	{
		None,	// An infinite plane
		Equilateral,
		Right
	};

	physx::PxF32 getSide(const physx::PxVec3& v)
	{
		if (m_pattern == None)
		{
			return m_plane.distance(v);
		}
		physx::PxVec3 vLocal;
		m_tm.multiplyByInverseRT(v, vLocal);
		physx::PxF32 x = vLocal.x - m_cornerX;
		physx::PxF32 y = vLocal.y - m_cornerY;
		if (y < 0)
		{
			return 0;
		}
		physx::PxF32 scaledY = y / m_ySpacing;
		physx::PxU32 gridY = (physx::PxU32)scaledY;
		if (gridY >= m_numY)
		{
			return 0;
		}
		scaledY -= (physx::PxF32)gridY;
		physx::PxU32 yParity = gridY & 1;
		if (yParity != 0)
		{
			scaledY = 1.0f - scaledY;
		}
		if (m_pattern == Equilateral)
		{
			x += 0.5f * m_xSpacing * scaledY;
		}
		if (x < 0)
		{
			return 0;
		}
		physx::PxF32 scaledX = x / m_xSpacing;
		physx::PxU32 gridX = (physx::PxU32)scaledX;
		if (gridX >= m_numX)
		{
			return 0;
		}
		scaledX -= (physx::PxF32)gridX;
		physx::PxU32 xParity = (physx::PxU32)(scaledX >= scaledY);
		physx::PxU32 triangleNum = 2 * (gridY * m_numX + gridX) + xParity;
		PX_ASSERT(triangleNum < m_triangles.size());
		physx::NxExplicitRenderTriangle& triangle = m_triangles[triangleNum];
		physx::PxVec3& v0 = triangle.vertices[0].position;
		physx::PxVec3& v1 = triangle.vertices[1].position;
		physx::PxVec3& v2 = triangle.vertices[2].position;
		return ((v1 - v0).cross(v2 - v0)).dot(v - v0);
	}

	void clear()
	{
		m_pattern = None;
		m_plane = physx::PxPlane(0, 0, 1, 0);
		m_vertices.reset();
		m_triangles.reset();
	}

	void build(const physx::PxPlane& plane)
	{
		clear();
		m_plane = plane;
	}

	void build(GridPattern pattern, const physx::PxPlane& plane,
	           physx::PxF32 cornerX, physx::PxF32 cornerY, physx::PxF32 xSpacing, physx::PxF32 ySpacing, physx::PxU32 numX, physx::PxU32 numY,
	           const physx::PxMat34Legacy& tm, float noiseAmplitude, float relativeFrequency, float xPeriod, float yPeriod,
	           int noiseType, int noiseDir, physx::PxU32 submeshIndex, physx::PxU32 frameIndex, const TriangleFrame& triangleFrame, bool forceGrid);

	GridPattern									m_pattern;

	physx::PxMat34Legacy						m_tm;
	physx::PxPlane								m_plane;
	physx::Array<physx::NxVertex>				m_vertices;
	physx::Array<physx::NxExplicitRenderTriangle> m_triangles;

	physx::PxU32								m_numX;
	physx::PxF32								m_cornerX;
	physx::PxF32								m_xSpacing;
	physx::PxU32								m_numY;
	physx::PxF32								m_cornerY;
	physx::PxF32								m_ySpacing;
};

struct DisplacementMapVolume : public NxDisplacementMapVolume
{
	DisplacementMapVolume();

	void init(const NxFractureSliceDesc& desc);

	void getData(physx::PxU32& width, physx::PxU32& height, physx::PxU32& depth, physx::PxU32& size, unsigned char const** ppData) const;

private:

	void buildData(const physx::PxVec3 scale = physx::PxVec3(1)) const;

	// Data creation is lazy, and does not effect externally visible state
	//    Note: At some point, we will want to switch to floating point displacements
	mutable physx::Array<unsigned char> data;

	physx::PxU32 width;
	physx::PxU32 height;
	physx::PxU32 depth;

};

// CutoutSet

struct PolyVert
{
	physx::PxU16 index;
	physx::PxU16 flags;
};

struct ConvexLoop
{
	physx::Array<PolyVert> polyVerts;
};

struct Cutout
{
	physx::Array<physx::PxVec3> vertices;
	physx::Array<ConvexLoop> convexLoops;
};

struct CutoutSet : public NxCutoutSet
{
	CutoutSet() : periodic(false), dimensions(0.0f)
	{
	}

	enum Version
	{
		First = 0,
		// New versions must be put here.  There is no need to explicitly number them.  The
		// numbers above were put there to conform to the old DestructionToolStreamVersion enum.

		Count,
		Current = Count - 1
	};

	physx::PxU32			getCutoutCount() const
	{
		return cutouts.size();
	}

	physx::PxU32			getCutoutVertexCount(physx::PxU32 cutoutIndex) const
	{
		return cutouts[cutoutIndex].vertices.size();
	}
	physx::PxU32			getCutoutLoopCount(physx::PxU32 cutoutIndex) const
	{
		return cutouts[cutoutIndex].convexLoops.size();
	}

	const physx::PxVec3&	getCutoutVertex(physx::PxU32 cutoutIndex, physx::PxU32 vertexIndex) const
	{
		return cutouts[cutoutIndex].vertices[vertexIndex];
	}

	physx::PxU32			getCutoutLoopSize(physx::PxU32 cutoutIndex, physx::PxU32 loopIndex) const
	{
		return cutouts[cutoutIndex].convexLoops[loopIndex].polyVerts.size();
	}

	physx::PxU32			getCutoutLoopVertexIndex(physx::PxU32 cutoutIndex, physx::PxU32 loopIndex, physx::PxU32 vertexNum) const
	{
		return cutouts[cutoutIndex].convexLoops[loopIndex].polyVerts[vertexNum].index;
	}
	physx::PxU32			getCutoutLoopVertexFlags(physx::PxU32 cutoutIndex, physx::PxU32 loopIndex, physx::PxU32 vertexNum) const
	{
		return cutouts[cutoutIndex].convexLoops[loopIndex].polyVerts[vertexNum].flags;
	}
	bool					isPeriodic() const
	{
		return periodic;
	}
	const physx::PxVec2&	getDimensions() const
	{
		return dimensions;
	}

	void					serialize(physx::PxFileBuf& stream) const;
	void					deserialize(physx::PxFileBuf& stream);

	void					release()
	{
		delete this;
	}

	physx::Array<Cutout>	cutouts;
	bool					periodic;
	physx::PxVec2			dimensions;
};

class PartConvexHullProxy : public NxExplicitHierarchicalMesh::NxConvexHull, public physx::UserAllocated
{
public:
	physx::ConvexHull	impl;

	PartConvexHullProxy()
	{
		impl.init();
	}

	PartConvexHullProxy(const PartConvexHullProxy& hull)
	{
		*this = hull;
	}

	PartConvexHullProxy&			operator = (const PartConvexHullProxy& hull)
	{
		impl.init();
		if (hull.impl.mParams)
		{
			impl.mParams->copy(*hull.impl.mParams);
		}
		return *this;
	}

	virtual void					buildFromPoints(const void* points, physx::PxU32 numPoints, physx::PxU32 pointStrideBytes)
	{
		impl.buildFromPoints(points, numPoints, pointStrideBytes);
	}

	virtual const physx::PxBounds3&	getBounds() const
	{
		return impl.getBounds();
	}

	virtual physx::PxF32			getVolume() const
	{
		return impl.getVolume();
	}

	virtual physx::PxU32			getVertexCount() const
	{
		return impl.getVertexCount();
	}

	virtual physx::PxVec3			getVertex(physx::PxU32 vertexIndex) const
	{
		if (vertexIndex < impl.getVertexCount())
		{
			return impl.getVertex(vertexIndex);
		}
		return physx::PxVec3(0.0f);
	}

	virtual physx::PxU32			getEdgeCount() const
	{
		return impl.getEdgeCount();
	}

	virtual physx::PxVec3			getEdgeEndpoint(physx::PxU32 edgeIndex, physx::PxU32 whichEndpoint) const
	{
		if (edgeIndex < impl.getEdgeCount())
		{
			return impl.getVertex(impl.getEdgeEndpointIndex(edgeIndex, whichEndpoint));
		}
		return physx::PxVec3(0.0f);
	}

	/**
		This is the number of planes which bound the convex hull.
	*/
	virtual physx::PxU32			getPlaneCount() const
	{
		return impl.getPlaneCount();
	}

	/**
		This is the plane indexed by planeIndex, which must in
		the range [0, getPlaneCount()-1].
	*/
	virtual physx::PxPlane			getPlane(physx::PxU32 planeIndex) const
	{
		if (planeIndex < impl.getPlaneCount())
		{
			return impl.getPlane(planeIndex);
		}
		return physx::PxPlane(physx::PxVec3(0.0f), 0.0f);
	}

	virtual bool					rayCast(physx::PxF32& in, physx::PxF32& out, const physx::PxVec3& orig, const physx::PxVec3& dir,
	        const physx::PxMat44& localToWorldRT, const physx::PxVec3& scale, physx::PxVec3* normal = NULL) const
	{
		return impl.rayCast(in, out, orig, dir, localToWorldRT, scale, normal);
	}

	virtual bool					reduceHull(physx::PxU32 maxVertexCount, physx::PxU32 maxEdgeCount, physx::PxU32 maxFaceCount, bool inflated)
	{
		return impl.reduceHull(maxVertexCount, maxEdgeCount, maxFaceCount, inflated);
	}

	virtual void					release()
	{
		delete this;
	}
};

PX_INLINE void	resizeCollision(physx::Array<physx::PartConvexHullProxy*>& collision, physx::PxU32 hullCount)
{
	const physx::PxU32 oldHullCount = collision.size();
	for (physx::PxU32 i = hullCount; i < oldHullCount; ++i)
	{
		collision[i]->release();
	}
	collision.resize(hullCount);
	for (physx::PxU32 i = oldHullCount; i < hullCount; ++i)
	{
		collision[i] = PX_NEW(physx::PartConvexHullProxy)();
	}
}

void buildCollisionGeometry(physx::Array<physx::PartConvexHullProxy*>& volumes, const NxCollisionVolumeDesc& desc,
							const physx::PxVec3* vertices, physx::PxU32 vertexCount, physx::PxU32 vertexByteStride,
							const physx::PxU32* indices, physx::PxU32 indexCount);


// ExplicitHierarchicalMesh

static physx::PxU64	sNextChunkEUID = 0;	// Execution-unique identifier for chunks

class ExplicitHierarchicalMesh : public NxExplicitHierarchicalMesh, public physx::UserAllocated
{
public:

	// This has been copied from DestructionToolStreamVersion, at ToolStreamVersion_RemovedExplicitHMesh_mMaxDepth.
	enum Version
	{
		First = 0,
		AddedMaterialFramesToHMesh_and_NoiseType_and_GridSize_to_Cleavage = 7,
		IncludingVertexFormatInSubmeshData = 12,
		AddedMaterialLibraryToMesh = 14,
		AddedCacheChunkSurfaceTracesAndInteriorSubmeshIndex = 32,
		RemovedExplicitHMesh_mMaxDepth = 38,
		UsingExplicitPartContainers,
		SerializingMeshBSP,
		SerializingMeshBounds,
		AddedFlagsFieldToPart,
		PerPartMeshBSPs,
		StoringRootSubmeshCount,
		MultipleConvexHullsPerChunk,
		InstancingData,
		UVInstancingData,
		DisplacementData,
		ChangedMaterialFrameToIncludeFracturingMethodContext,
		RemovedInteriorSubmeshIndex,
		AddedSliceDepthToMaterialFrame,
		RemovedNxChunkAuthoringFlag,
		ReaddedFlagsToPart,
		IntroducingChunkPrivateFlags,
		// New versions must be put here.  There is no need to explicitly number them.  The
		// numbers above were put there to conform to the old DestructionToolStreamVersion enum.

		Count,
		Current = Count - 1
	};

	struct Part : public physx::UserAllocated
	{
		Part() : mMeshBSP(NULL), mSurfaceNormal(0.0f), mFlags(0)
		{
			mBounds.setEmpty();
		}

		~Part()
		{
			if (mMeshBSP != NULL)
			{
				mMeshBSP->release();
				mMeshBSP = NULL;
			}
			resizeCollision(mCollision, 0);
		}

		enum Flags
		{
			MeshOpen =	(1<<0),
		};

		physx::PxBounds3								mBounds;
		physx::Array<physx::NxExplicitRenderTriangle>	mMesh;
		ApexCSG::IApexBSP*								mMeshBSP;
		physx::Array<physx::PartConvexHullProxy*>		mCollision;
		physx::PxVec3									mSurfaceNormal;	// used to kick chunk out if desired
		physx::PxU32									mFlags;	// See Flags
	};

	struct Chunk : public physx::UserAllocated
	{
		Chunk() : mParentIndex(-1), mFlags(0), mPartIndex(-1), mInstancedPositionOffset(physx::PxVec3(0.0f)), mInstancedUVOffset(physx::PxVec2(0.0f)), mPrivateFlags(0)
		{
			mEUID = sNextChunkEUID++;
		}

		enum Flags
		{
			Root		=	(1<<0),
			RootLeaf	=	(1<<1),
		};

		bool	isRootChunk() const
		{
			return (mPrivateFlags & Root) != 0;
		}

		bool	isRootLeafChunk() const	// This means that the chunk is a root chunk and has no children that are root chunks
		{
			return (mPrivateFlags & RootLeaf) != 0;
		}

		PX_INLINE physx::PxU64	getEUID() const
		{
			return mEUID;
		}

		physx::PxI32							mParentIndex;
		physx::PxU32							mFlags;	// See NxDestructibleAsset::ChunkFlags
		physx::PxI32							mPartIndex;
		physx::PxVec3							mInstancedPositionOffset;	// if instanced, the offsetPosition
		physx::PxVec2							mInstancedUVOffset;	// if instanced, the offset UV
		physx::PxU32							mPrivateFlags;	// Things that don't make it to the NxDestructibleAsset; authoring only.  See ExplicitHierarchicalMesh::Chunk::Flags

	private:
		physx::PxU64							mEUID;	// A unique identifier during the application execution.  Not to be serialized.
	};

	physx::Array<Part*>					mParts;
	physx::Array<Chunk*>				mChunks;
	physx::Array<NxExplicitSubmeshData>	mSubmeshData;
	physx::Array<physx::NxMaterialFrame>	mMaterialFrames;
	physx::PxU32						mRootSubmeshCount;	// How many submeshes came with the root mesh

	ApexCSG::IApexBSPMemCache*			mBSPMemCache;

	DisplacementMapVolume               mDisplacementMapVolume;

	ExplicitHierarchicalMesh();
	~ExplicitHierarchicalMesh();

	// Sorts chunks in parent-sorted order (stable)
	void sortChunks(physx::Array<physx::PxU32>* indexRemap = NULL);

	// Generate part surface normals, if possible
	void createPartSurfaceNormals();

	// NxExplicitHierarchicalMesh implementation:

	physx::PxU32 addPart();
	bool removePart(physx::PxU32 index);
	physx::PxU32 addChunk();
	bool removeChunk(physx::PxU32 index);
	void serialize(physx::PxFileBuf& stream, NxEmbedding& embedding) const;
	void deserialize(physx::PxFileBuf& stream, NxEmbedding& embedding);
	physx::PxI32 maxDepth() const;
	physx::PxU32 partCount() const;
	physx::PxU32 chunkCount() const;
	physx::PxU32 depth(physx::PxU32 chunkIndex) const;
	physx::PxI32* parentIndex(physx::PxU32 chunkIndex);
	physx::PxU64 chunkUniqueID(physx::PxU32 chunkIndex);
	physx::PxI32* partIndex(physx::PxU32 chunkIndex);
	physx::PxVec3* instancedPositionOffset(physx::PxU32 chunkIndex);
	physx::PxVec2* instancedUVOffset(physx::PxU32 chunkIndex);
	physx::PxU32 meshTriangleCount(physx::PxU32 partIndex) const;
	physx::NxExplicitRenderTriangle* meshTriangles(physx::PxU32 partIndex);
	physx::PxBounds3 meshBounds(physx::PxU32 partIndex) const;
	physx::PxBounds3 chunkBounds(physx::PxU32 chunkIndex) const;
	physx::PxU32* chunkFlags(physx::PxU32 chunkIndex) const;
	physx::PxU32 convexHullCount(physx::PxU32 partIndex) const;
	const NxExplicitHierarchicalMesh::NxConvexHull** convexHulls(physx::PxU32 partIndex) const;
	physx::PxVec3* surfaceNormal(physx::PxU32 partIndex);
	const NxDisplacementMapVolume& displacementMapVolume() const;
	physx::PxU32 submeshCount() const;
	NxExplicitSubmeshData* submeshData(physx::PxU32 submeshIndex);
	physx::PxU32 addSubmesh(const NxExplicitSubmeshData& submeshData);
	physx::PxU32 getMaterialFrameCount() const;
	physx::NxMaterialFrame getMaterialFrame(physx::PxU32 index) const;
	void setMaterialFrame(physx::PxU32 index, const physx::NxMaterialFrame& materialFrame);
	physx::PxU32 addMaterialFrame();
	void clear(bool keepRoot = false);
	void set(const NxExplicitHierarchicalMesh& mesh);
	bool calculatePartBSP(physx::PxU32 partIndex, physx::PxU32 randomSeed, physx::PxU32 microgridSize, NxBSPOpenMode::Enum meshMode, IProgressListener* progressListener = NULL, volatile bool* cancel = NULL);
	void calculateMeshBSP(physx::PxU32 randomSeed, IProgressListener* progressListener = NULL, const physx::PxU32* microgridSize = NULL, NxBSPOpenMode::Enum meshMode = physx::NxBSPOpenMode::Automatic);
	void replaceInteriorSubmeshes(physx::PxU32 partIndex, physx::PxU32 frameCount, physx::PxU32* frameIndices, physx::PxU32 submeshIndex);
	void visualize(NxApexRenderDebug& debugRender, physx::PxU32 flags, physx::PxU32 index = 0) const;
	void release();
	void buildMeshBounds(physx::PxU32 partIndex);
	void buildCollisionGeometryForPart(physx::PxU32 partIndex, const NxCollisionVolumeDesc& desc);
	void buildCollisionGeometryForRootChunkParts(const physx::NxCollisionDesc& desc, bool aggregateRootChunkParentCollision = true);
	void initializeDisplacementMapVolume(const physx::NxFractureSliceDesc& desc);
	void reduceHulls(const physx::NxCollisionDesc& desc, bool inflated);
	void aggregateCollisionHullsFromRootChildren(physx::PxU32 chunkIndex);
};

}
} // end namespace physx::apex

#endif

#endif
