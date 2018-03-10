/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "GuHeightField.h"
#include "GuConvexMeshData.h"
#include "GuBigConvexData2.h"
#include "GuConvexMesh.h"
#include "GuMeshInterface.h"
#include "GuHybridModel.h"
#include "GuTriangleMesh.h"
#include "GuGeometryUnion.h"
#include "PsIntrinsics.h"
#include "CmPhysXCommon.h"
#include "PxMetaData.h"

#include "PxIO.h"

using namespace physx;
using namespace Ps;
using namespace Cm;
using namespace Gu;

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_Valency(PxOutputStream& stream)
{
// 4 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,	Valency)
	PX_DEF_BIN_METADATA_ITEM(stream,	Valency, PxU16,	mCount,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Valency, PxU16,	mOffset,	0)
}

static void getBinaryMetaData_BigConvexRawData(PxOutputStream& stream)
{
// 24 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,	BigConvexRawData)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexRawData, PxU16,	mSubdiv,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexRawData, PxU16,	mNbSamples,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexRawData, PxU8,		mSamples,		PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexRawData, PxU32,	mNbVerts,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexRawData, PxU32,	mNbAdjVerts,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexRawData, Valency,	mValencies,		PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexRawData, PxU8,		mAdjacentVerts,	PxMetaDataFlag::ePTR)
}

void BigConvexData::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_Valency(stream);
	getBinaryMetaData_BigConvexRawData(stream);

// 28 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,	BigConvexData)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexData, BigConvexRawData,	mData,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	BigConvexData, void,				mVBuffer,	PxMetaDataFlag::ePTR)

	//------ Extra-data ------

	// mData.mSamples
	// PT: can't use one array of PxU16 since we don't want to flip those bytes during conversion.
	// PT: We only align the first array for DE1340, but the second one shouldn't be aligned since
	// both are written as one unique block of memory.
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	BigConvexData, PxU8, mData.mNbSamples, PX_SERIAL_ALIGN, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	BigConvexData, PxU8, mData.mNbSamples, 0, 0)

	// mData.mValencies
	// PT: same here, we must only align the first array
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	BigConvexData, Valency,	mData.mNbVerts, PX_SERIAL_ALIGN, 0)
	PX_DEF_BIN_METADATA_EXTRA_ALIGN(stream,	BigConvexData, PX_SERIAL_ALIGN)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	BigConvexData, PxU8,	mData.mNbAdjVerts, 0, 0)
}

static void getBinaryMetaData_InternalObjectsData(PxOutputStream& stream)
{
// 16 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,		InternalObjectsData)
	PX_DEF_BIN_METADATA_ITEM(stream,		InternalObjectsData,	PxReal,	mRadius,	0)
	PX_DEF_BIN_METADATA_ITEMS_AUTO(stream,	InternalObjectsData,	PxReal,	mExtents,	0)
}

static void getBinaryMetaData_HullPolygonData(PxOutputStream& stream)
{
// 20 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,		HullPolygonData)
	PX_DEF_BIN_METADATA_ITEMS_AUTO(stream,	HullPolygonData,	PxReal,	mPlane,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		HullPolygonData,	PxU16,	mVRef8,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		HullPolygonData,	PxU8,	mNbVerts,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		HullPolygonData,	PxU8,	mMinIndex,	0)
}

static void getBinaryMetaData_ConvexHullData(PxOutputStream& stream)
{
// 64 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,	ConvexHullData)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, PxBounds3,				mAABB,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, PxVec3,					mCenterOfMass,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, HullPolygonData,		mPolygons,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, BigConvexRawData,		mBigConvexRawData,	PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, PxU16,					mNbEdges,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, PxU8,					mNbHullVertices,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, PxU8,					mNbPolygons,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexHullData, InternalObjectsData,	mInternal,			0)
}

void Gu::ConvexMesh::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_InternalObjectsData(stream);
	getBinaryMetaData_HullPolygonData(stream);
	getBinaryMetaData_ConvexHullData(stream);
	BigConvexData::getBinaryMetaData(stream);

// 136 bytes
	PX_DEF_BIN_METADATA_VCLASS(stream,ConvexMesh)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,ConvexMesh, PxBase)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,ConvexMesh, RefCountable)

	//
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexMesh, ConvexHullData,	mHullData,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexMesh, PxU32,			mNb,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexMesh, BigConvexData,	mBigConvexData,	PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexMesh, PxReal,			mMass,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexMesh, PxMat33,		mInertia,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	ConvexMesh, GuMeshFactory,	mMeshFactory,	PxMetaDataFlag::ePTR)

	//------ Extra-data ------

	// mHullData.mPolygons (Gu::HullPolygonData, PxVec3, PxU8*2, PxU8)
	// PT: we only align the first array since the other ones are contained within it
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, HullPolygonData,	mHullData.mNbPolygons,		PX_SERIAL_ALIGN, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, PxVec3,				mHullData.mNbHullVertices,	0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, PxU8,				mHullData.mNbEdges,			0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, PxU8,			    mHullData.mNbEdges,			0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, PxU8,			    mHullData.mNbHullVertices,	0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, PxU8,			    mHullData.mNbHullVertices,	0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, PxU8,			    mHullData.mNbHullVertices,	0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	Gu::ConvexMesh, PxU8,			    mNb,						0, PxMetaDataFlag::eCOUNT_MASK_MSB)
	PX_DEF_BIN_METADATA_EXTRA_ALIGN(stream,	ConvexMesh, 4)
	// mBigConvexData
	PX_DEF_BIN_METADATA_EXTRA_ITEM(stream, Gu::ConvexMesh, BigConvexData, mBigConvexData, PX_SERIAL_ALIGN)
}

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_PxHeightFieldSample(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	PxHeightFieldSample)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxHeightFieldSample,	PxI16,		    height,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxHeightFieldSample,	PxBitAndByte,	materialIndex0,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxHeightFieldSample,	PxBitAndByte,	materialIndex1,	0)
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxBitAndByte,			PxU8)
}

static void getBinaryMetaData_HeightFieldData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxHeightFieldFlags, PxU16)
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	PxHeightFieldFormat::Enum, PxU32)

	PX_DEF_BIN_METADATA_CLASS(stream,	HeightFieldData)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxU32,					rows,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxU32,					columns,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxReal,				rowLimit,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxReal,				colLimit,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxReal,				nbColumns,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxHeightFieldSample,	samples,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxReal,				thickness,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxReal,				convexEdgeThreshold,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxHeightFieldFlags,	flags,					0)
#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxU16,					paddAfterFlags,			PxMetaDataFlag::ePADDING)
#endif
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxHeightFieldFormat::Enum,	format,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxBounds3,					mAABB,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxU32,						rowsPadded,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxU32,						columnsPadded,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxU32,						tilesU,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightFieldData, PxU32,						tilesV,					0)
}

void Gu::HeightField::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_PxHeightFieldSample(stream);
	getBinaryMetaData_HeightFieldData(stream);

	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxMaterialTableIndex, PxU16)

	PX_DEF_BIN_METADATA_VCLASS(stream,		HeightField)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	HeightField, PxBase)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	HeightField, RefCountable)

	PX_DEF_BIN_METADATA_ITEM(stream,	HeightField, HeightFieldData,	mData,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightField, PxU32,				mSampleStride,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightField, PxU32,				mNbSamples,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightField, PxReal,			mMinHeight,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightField, PxReal,			mMaxHeight,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	HeightField, GuMeshFactory,		mMeshFactory,	PxMetaDataFlag::ePTR)

	//------ Extra-data ------

	// mData.samples
#if HF_TILED_MEMORY_LAYOUT
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	HeightField, PxHeightFieldSample, mNbSamples, 16, 0)	// PT: ### try to remove mNbSamples later
#else
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	HeightField, PxHeightFieldSample, mNbSamples, PX_SERIAL_ALIGN, 0)	// PT: ### try to remove mNbSamples later
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_LeafTriangles(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	LeafTriangles)
	PX_DEF_BIN_METADATA_ITEM(stream,	LeafTriangles, PxU32,	Data,	0)
}

void MeshInterface::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,MeshInterface)

	PX_DEF_BIN_METADATA_ITEM(stream,	MeshInterface, PxU32,	mNbTris,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	MeshInterface, PxU32,	mNbVerts,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	MeshInterface, void,	mTris,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	MeshInterface, PxVec3,	mVerts,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	MeshInterface, PxU32,	mHas16BitIndices,	0)
}

void RTreeMidphase::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_LeafTriangles(stream);

// 128 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,RTreeMidphase)

	PX_DEF_BIN_METADATA_ITEM(stream,RTreeMidphase, MeshInterface,	mIMesh,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,RTreeMidphase, PxReal,			mGeomEpsilon,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,RTreeMidphase, RTree,			mRTree,			0)
}

static void getBinaryMetaData_RTreePage(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	RTreePage)
	PX_DEF_BIN_METADATA_ITEMS(stream,	RTreePage, PxReal,	minx,	0, RTREE_PAGE_SIZE)
	PX_DEF_BIN_METADATA_ITEMS(stream,	RTreePage, PxReal,	miny,	0, RTREE_PAGE_SIZE)
	PX_DEF_BIN_METADATA_ITEMS(stream,	RTreePage, PxReal,	minz,	0, RTREE_PAGE_SIZE)
	PX_DEF_BIN_METADATA_ITEMS(stream,	RTreePage, PxReal,	maxx,	0, RTREE_PAGE_SIZE)
	PX_DEF_BIN_METADATA_ITEMS(stream,	RTreePage, PxReal,	maxy,	0, RTREE_PAGE_SIZE)
	PX_DEF_BIN_METADATA_ITEMS(stream,	RTreePage, PxReal,	maxz,	0, RTREE_PAGE_SIZE)
	PX_DEF_BIN_METADATA_ITEMS(stream,	RTreePage, PxU32,	ptrs,	0, RTREE_PAGE_SIZE)
}

void RTree::getBinaryMetaData(PxOutputStream& stream)
{
	getBinaryMetaData_RTreePage(stream);

// 96 bytes
	PX_DEF_BIN_METADATA_CLASS(stream, RTree)

	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxVec4,		mBoundsMin,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxVec4,		mBoundsMax,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxVec4,		mInvDiagonal,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxVec4,		mDiagonalScaler,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxU32,		mPageSize,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxU32,		mNumRootPages,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxU32,		mNumLevels,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxU32,		mTotalNodes,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxU32,		mTotalPages,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxU32,		mUnused,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, PxU32,		mFlags,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,	RTree, RTreePage,	mPages,				PxMetaDataFlag::ePTR)

	//------ Extra-data ------

	// mPages
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,RTree, RTreePage, mTotalPages, 128, 0)
}

///////////////////////////////////////////////////////////////////////////////

static void getBinaryMetaData_InternalTriangleMeshData(PxOutputStream& stream)
{
// 224 => 208 => 192 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,	InternalTriangleMeshData)

	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, PxU32,			mNumVertices,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, PxU32,			mNumTriangles,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, PxVec3,			mVertices,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, void,				mTriangles,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, PxU8,				mExtraTrigData,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, RTreeMidphase,	mCollisionModel,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, PxBounds3,		mAABB,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMeshData, PxU8,				mFlags,					0)	
	#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEMS(stream,	InternalTriangleMeshData, bool,				mPaddingFromBool,		PxMetaDataFlag::ePADDING, 3)
	#endif
	//------ Extra-data ------

	// mVertices
	PX_DEF_BIN_METADATA_EXTRA_ITEMS(stream, InternalTriangleMeshData, PxVec3, mVertices, mNumVertices, 0, PX_SERIAL_ALIGN)

	// mTriangles
	// PT: quite tricky here: we exported either an array of PxU16s or an array of PxU32s. We trick the converter by
	// pretending we exported both, with the same control variable (m16BitIndices) but opposed control flags. Also there's
	// no way to capture "mNumTriangles*3" using the macros, so we just pretend we exported 3 buffers instead of 1.
	// But since in reality it's all the same buffer, only the first one is declared as aligned.

	PX_DEF_BIN_METADATA_EXTRA_ITEMS_MASKED_CONTROL(stream,	InternalTriangleMeshData, PxU16, mFlags, PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES, mNumTriangles, 0, PX_SERIAL_ALIGN)
	PX_DEF_BIN_METADATA_EXTRA_ITEMS_MASKED_CONTROL(stream,	InternalTriangleMeshData, PxU16, mFlags, PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES, mNumTriangles, 0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ITEMS_MASKED_CONTROL(stream,	InternalTriangleMeshData, PxU16, mFlags, PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES, mNumTriangles, 0, 0)
	PX_DEF_BIN_METADATA_EXTRA_ITEMS_MASKED_CONTROL(stream,	InternalTriangleMeshData, PxU32, mFlags, PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES, mNumTriangles, PxMetaDataFlag::eCONTROL_FLIP, PX_SERIAL_ALIGN)
	PX_DEF_BIN_METADATA_EXTRA_ITEMS_MASKED_CONTROL(stream,	InternalTriangleMeshData, PxU32, mFlags, PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES, mNumTriangles, PxMetaDataFlag::eCONTROL_FLIP, 0)
	PX_DEF_BIN_METADATA_EXTRA_ITEMS_MASKED_CONTROL(stream,	InternalTriangleMeshData, PxU32, mFlags, PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES, mNumTriangles, PxMetaDataFlag::eCONTROL_FLIP, 0)

	// mExtraTrigData
	PX_DEF_BIN_METADATA_EXTRA_ITEMS(stream, InternalTriangleMeshData, PxU8, mExtraTrigData, mNumTriangles, 0, PX_SERIAL_ALIGN)
}

void InternalTriangleMesh::getBinaryMetaData(PxOutputStream& stream)
{
	MeshInterface::getBinaryMetaData(stream);
	RTreeMidphase::getBinaryMetaData(stream);
	RTree::getBinaryMetaData(stream);

	getBinaryMetaData_InternalTriangleMeshData(stream);

// 256 => 240 bytes
	PX_DEF_BIN_METADATA_CLASS(stream, InternalTriangleMesh)

	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, InternalTriangleMeshData,	mData,					0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, PxU16,					mMaterialIndices,		PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, PxU32,					mFaceRemap,				PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, PxU32,					mAdjacencies,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, PxU32,					mNumAdjacencies,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, PxReal,					mConvexEdgeThreshold,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, PxU32,					mOwnsMemory,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,	InternalTriangleMesh, MeshInterface,			mMeshInterface,			0)

	//------ Extra-data ------

	// mMaterialIndices
	PX_DEF_BIN_METADATA_EXTRA_ITEMS(stream, InternalTriangleMesh, PxU16, mMaterialIndices,	mData.mNumTriangles,	0, PX_SERIAL_ALIGN)

	// mFaceRemap
	PX_DEF_BIN_METADATA_EXTRA_ITEMS(stream, InternalTriangleMesh, PxU32, mFaceRemap,		mData.mNumTriangles,	0, PX_SERIAL_ALIGN)

	// mAdjacencies
	PX_DEF_BIN_METADATA_EXTRA_ITEMS(stream, InternalTriangleMesh, PxU32, mAdjacencies,		mNumAdjacencies,		0, PX_SERIAL_ALIGN)
}

void Gu::TriangleMesh::getBinaryMetaData(PxOutputStream& stream)
{
	InternalTriangleMesh::getBinaryMetaData(stream);

// 320 => 304 => 272 => 256 bytes
	PX_DEF_BIN_METADATA_VCLASS(stream,		TriangleMesh)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	TriangleMesh, PxBase)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	TriangleMesh, RefCountable)

	PX_DEF_BIN_METADATA_ITEM(stream,		TriangleMesh, InternalTriangleMesh,	mMesh,						0)
	PX_DEF_BIN_METADATA_ITEM(stream,		TriangleMesh, GuMeshFactory,		mMeshFactory,				PxMetaDataFlag::ePTR)
#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEMS_AUTO(stream,	TriangleMesh, PxU32,				mPaddingFromInternalMesh,	PxMetaDataFlag::ePADDING)
#endif
}

///////////////////////////////////////////////////////////////////////////////

void MaterialIndicesStruct::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	MaterialIndicesStruct)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialIndicesStruct, PxU16,	indices,	PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialIndicesStruct, PxU16,	numIndices,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialIndicesStruct, PxU16,	pad,		PxMetaDataFlag::ePADDING)
#if defined(PX_P64)
	PX_DEF_BIN_METADATA_ITEM(stream,	MaterialIndicesStruct, PxU32,	pad64,		PxMetaDataFlag::ePADDING)
#endif

	//------ Extra-data ------
	// indices
	PX_DEF_BIN_METADATA_EXTRA_ITEMS(stream, MaterialIndicesStruct, PxU16, indices, numIndices, 0, PX_SERIAL_ALIGN)
}

///////////////////////////////////////////////////////////////////////////////

void Gu::GeometryUnion::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxGeometryType::Enum, PxU32)

	// The various PxGeometry classes are all public, so I can't really put the meta-data function in there. And then
	// I can't access their protected members. So we use the same trick as for the ShapeContainer
	class ShadowConvexMeshGeometry : public PxConvexMeshGeometryLL
	{
	public:
		static void getBinaryMetaData(PxOutputStream& stream_)
		{
			PX_DEF_BIN_METADATA_CLASS(stream_,	ShadowConvexMeshGeometry)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowConvexMeshGeometry, PxGeometryType::Enum,	mType,		0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowConvexMeshGeometry, PxMeshScale,			scale,		0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowConvexMeshGeometry, PxConvexMesh,			convexMesh,	PxMetaDataFlag::ePTR)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowConvexMeshGeometry, ConvexHullData,		hullData,	PxMetaDataFlag::ePTR)
		}
	};
	ShadowConvexMeshGeometry::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxConvexMeshGeometryLL, ShadowConvexMeshGeometry)

	/////////////////

	class ShadowTriangleMeshGeometry : public PxTriangleMeshGeometryLL
	{
	public:
		static void getBinaryMetaData(PxOutputStream& stream_)
		{
			PX_DEF_BIN_METADATA_TYPEDEF(stream_, PxMeshGeometryFlags, PxU8)

			PX_DEF_BIN_METADATA_CLASS(stream_,	ShadowTriangleMeshGeometry)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowTriangleMeshGeometry, PxGeometryType::Enum,		mType,				0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowTriangleMeshGeometry, PxMeshScale,				scale,				0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowTriangleMeshGeometry, PxMeshGeometryFlags,		meshFlags,			0)
			PX_DEF_BIN_METADATA_ITEMS(stream_,	ShadowTriangleMeshGeometry,	PxU8,						paddingFromFlags,	PxMetaDataFlag::ePADDING, 3)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowTriangleMeshGeometry, PxTriangleMesh,				triangleMesh,		PxMetaDataFlag::ePTR)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowTriangleMeshGeometry, InternalTriangleMeshData,	meshData,			PxMetaDataFlag::ePTR)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowTriangleMeshGeometry, PxU16,						materialIndices,	PxMetaDataFlag::ePTR)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowTriangleMeshGeometry, MaterialIndicesStruct,		materials,			0)
		}
	};
	ShadowTriangleMeshGeometry::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream,PxTriangleMeshGeometryLL, ShadowTriangleMeshGeometry)

	/////////////////

	class ShadowHeightFieldGeometry : public PxHeightFieldGeometryLL
	{
	public:
		static void getBinaryMetaData(PxOutputStream& stream_)
		{
			PX_DEF_BIN_METADATA_CLASS(stream_,		ShadowHeightFieldGeometry)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, PxGeometryType::Enum,	mType,					0)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, PxHeightField,		    heightField,			PxMetaDataFlag::ePTR)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, PxReal,					heightScale,			0)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, PxReal,					rowScale,				0)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, PxReal,					columnScale,			0)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, PxMeshGeometryFlags,		heightFieldFlags,		0)
			PX_DEF_BIN_METADATA_ITEMS_AUTO(stream_,	ShadowHeightFieldGeometry, PxU8,					paddingFromFlags,		PxMetaDataFlag::ePADDING)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, HeightField,				heightFieldData,		PxMetaDataFlag::ePTR)
			PX_DEF_BIN_METADATA_ITEM(stream_,		ShadowHeightFieldGeometry, MaterialIndicesStruct,	materials,				0)
		}
	};
	ShadowHeightFieldGeometry::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream,PxHeightFieldGeometryLL, ShadowHeightFieldGeometry)

	/////////////////

	class ShadowPlaneGeometry : public PxPlaneGeometry
	{
	public:
		static void getBinaryMetaData(PxOutputStream& stream_)
		{
			PX_DEF_BIN_METADATA_CLASS(stream_,	ShadowPlaneGeometry)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowPlaneGeometry, PxGeometryType::Enum,	mType,		0)
		}
	};
	ShadowPlaneGeometry::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream,PxPlaneGeometry, ShadowPlaneGeometry)

	/////////////////

	class ShadowSphereGeometry : public PxSphereGeometry
	{
	public:
		static void getBinaryMetaData(PxOutputStream& stream_)
		{
			PX_DEF_BIN_METADATA_CLASS(stream_,	ShadowSphereGeometry)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowSphereGeometry, PxGeometryType::Enum,		mType,		0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowSphereGeometry, PxReal,				    radius,		0)
		}
	};
	ShadowSphereGeometry::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxSphereGeometry, ShadowSphereGeometry)

	/////////////////

	class ShadowCapsuleGeometry : public PxCapsuleGeometry
	{
	public:
		static void getBinaryMetaData(PxOutputStream& stream_)
		{
			PX_DEF_BIN_METADATA_CLASS(stream_,	ShadowCapsuleGeometry)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowCapsuleGeometry, PxGeometryType::Enum,	mType,		0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowCapsuleGeometry, PxReal,					radius,		0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowCapsuleGeometry, PxReal,					halfHeight,	0)
		}
	};
	ShadowCapsuleGeometry::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxCapsuleGeometry, ShadowCapsuleGeometry)

	/////////////////

	class ShadowBoxGeometry : public PxBoxGeometry
	{
	public:
		static void getBinaryMetaData(PxOutputStream& stream_)
		{
			PX_DEF_BIN_METADATA_CLASS(stream_,	ShadowBoxGeometry)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowBoxGeometry, PxGeometryType::Enum,	mType,		0)
			PX_DEF_BIN_METADATA_ITEM(stream_,	ShadowBoxGeometry, PxVec3,					halfExtents,0)
		}
	};
	ShadowBoxGeometry::getBinaryMetaData(stream);
	PX_DEF_BIN_METADATA_TYPEDEF(stream, PxBoxGeometry, ShadowBoxGeometry)

	/*
	- geom union offset & size
	- control type offset & size
	- type-to-class mapping
	*/

// 44 bytes
	PX_DEF_BIN_METADATA_CLASS(stream, Gu::GeometryUnion)

	PX_DEF_BIN_METADATA_UNION(stream,		Gu::GeometryUnion, geometry)
	PX_DEF_BIN_METADATA_UNION_TYPE(stream,	Gu::GeometryUnion, PxSphereGeometry,		PxGeometryType::eSPHERE)
	PX_DEF_BIN_METADATA_UNION_TYPE(stream, 	Gu::GeometryUnion, PxPlaneGeometry,			PxGeometryType::ePLANE)
	PX_DEF_BIN_METADATA_UNION_TYPE(stream, 	Gu::GeometryUnion, PxCapsuleGeometry,		PxGeometryType::eCAPSULE)
	PX_DEF_BIN_METADATA_UNION_TYPE(stream, 	Gu::GeometryUnion, PxBoxGeometry,			PxGeometryType::eBOX)
	PX_DEF_BIN_METADATA_UNION_TYPE(stream, 	Gu::GeometryUnion, PxConvexMeshGeometryLL,	PxGeometryType::eCONVEXMESH)
	PX_DEF_BIN_METADATA_UNION_TYPE(stream, 	Gu::GeometryUnion, PxTriangleMeshGeometryLL,PxGeometryType::eTRIANGLEMESH)
	PX_DEF_BIN_METADATA_UNION_TYPE(stream, 	Gu::GeometryUnion, PxHeightFieldGeometryLL,	PxGeometryType::eHEIGHTFIELD)
}
