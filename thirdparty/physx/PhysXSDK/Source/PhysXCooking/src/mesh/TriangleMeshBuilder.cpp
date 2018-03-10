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


#include "GuGeomUtilsInternal.h"
#include "TriangleMeshBuilder.h"
#include "PxTriangleMeshDesc.h"
#include "InternalTriangleMeshBuilder.h"
#include "EdgeList.h"
#include "PsFastMemory.h"
#include "GuConvexEdgeFlags.h"
#include "GuSerialize.h"

using namespace physx;
using namespace Gu;

//
// When suppressTriangleMeshRemapTable is true, the face remap table is not created. This saves a significant amount of memory,
// but the SDK will not be able to provide information about which mesh triangle is hit in collisions, sweeps or raycasts hits.
//
bool TriangleMeshBuilder::loadFromDesc(const PxTriangleMeshDesc& _desc, const PxCookingParams& params, bool validateMesh)
{
	if(!_desc.isValid())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "TriangleMesh::loadFromDesc: desc.isValid() failed!");
		return false;
	}

	// Create a local copy that we can modify
	PxTriangleMeshDesc desc = _desc;

	// Save simple params
	mMesh.mConvexEdgeThreshold	= _desc.convexEdgeThreshold;
	{
		// Handle implicit topology
		PxU32* topology = NULL;
		if(!desc.triangles.data)
		{
			// We'll create 32-bit indices
			desc.flags &= ~PxMeshFlag::e16_BIT_INDICES;
			desc.triangles.stride = sizeof(PxU32)*3;

			{
			// Non-indexed mesh => create implicit topology
			desc.triangles.count = desc.points.count/3;
			// Create default implicit topology
			topology = PX_NEW_TEMP(PxU32)[desc.points.count];
			for(PxU32 i=0;i<desc.points.count;i++)
				topology[i] = i;
			desc.triangles.data = topology;
			}
		}
		// Continue as usual using our new descriptor

		// Convert and clean the input mesh
		if (!importMesh(desc, params,validateMesh))
			return false;

		// Cleanup if needed
		PX_DELETE_POD(topology);
	}

	// The midphase tree has to be created *after* the topology has (possibly) been remapped
	if (params.meshSizePerformanceTradeOff < 0.0f || params.meshSizePerformanceTradeOff > 1.0f)
		Ps::getFoundation().error(
			PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__,
			"PxCookingParams::meshSizePerformanceTradeOff clamped to [0,1] range (value=%.4f)", params.meshSizePerformanceTradeOff);
	InternalTriangleMeshBuilder builder(&mMesh, params);
	builder.createRTree();

	// Compute local bounds
	computeLocalBounds(); // AP scaffold: local bounds are computed in builder.createRTree efficiently with SIMD

	builder.createSharedEdgeData(params.buildTriangleAdjacencies, !(params.meshPreprocessParams & PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TriangleMeshBuilder::save(PxOutputStream& stream, bool platformMismatch, const PxCookingParams& params) const
{
	// Export header
	if(!writeHeader('M', 'E', 'S', 'H', PX_MESH_VERSION, platformMismatch, stream))
		return false;

	// Export serialization flags
	PxU32 serialFlags = 0;
	if(mMesh.getMaterials())	serialFlags |= Gu::IMSF_MATERIALS;
	if(mMesh.getFaceRemap())	serialFlags |= Gu::IMSF_FACE_REMAP;
	if(mMesh.getAdjacencies())	serialFlags |= Gu::IMSF_ADJACENCIES;
	// Compute serialization flags for indices
	PxU32 maxIndex=0;
	const Gu::TriangleT<PxU32>* tris = reinterpret_cast<const Gu::TriangleT<PxU32>*>(mMesh.getTriangles());
	for(PxU32 i=0;i<mMesh.getNumTriangles();i++)
	{
		if(tris[i].v[0]>maxIndex)	maxIndex = tris[i].v[0];
		if(tris[i].v[1]>maxIndex)	maxIndex = tris[i].v[1];
		if(tris[i].v[2]>maxIndex)	maxIndex = tris[i].v[2];
	}

	bool force32 = (params.meshPreprocessParams & PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES);
	if (maxIndex <= 0xFFFF && !force32)
		serialFlags |= (maxIndex <= 0xFF ? Gu::IMSF_8BIT_INDICES : Gu::IMSF_16BIT_INDICES);
	writeDword(serialFlags, platformMismatch, stream);

	// Export misc values
	writeFloat(mMesh.mConvexEdgeThreshold, platformMismatch, stream);

	// Export mesh
	writeDword(mMesh.getNumVertices(), platformMismatch, stream);
	writeDword(mMesh.getNumTriangles(), platformMismatch, stream);
	writeFloatBuffer(&mMesh.getVertices()->x, mMesh.getNumVertices()*3, platformMismatch, stream);
	if(serialFlags & Gu::IMSF_8BIT_INDICES)
	{
		const PxU32* indices = tris->v;
		for(PxU32 i=0;i<mMesh.getNumTriangles()*3;i++)
		{
			PxI8 data = (PxI8)indices[i];		
			stream.write(&data, sizeof(PxU8));	
		}
	}
	else if(serialFlags & Gu::IMSF_16BIT_INDICES)
	{
		const PxU32* indices = tris->v;
		for(PxU32 i=0;i<mMesh.getNumTriangles()*3;i++)
			writeWord(Ps::to16(indices[i]), platformMismatch, stream);
	}
	else
		writeIntBuffer(tris->v, mMesh.getNumTriangles()*3, platformMismatch, stream);

	if(mMesh.getMaterials())
		writeWordBuffer(mMesh.getMaterials(), mMesh.getNumTriangles(), platformMismatch, stream);

	if(mMesh.getFaceRemap())
	{
		PxU32 maxId = computeMaxIndex(mMesh.getFaceRemap(), mMesh.getNumTriangles());
		writeDword(maxId, platformMismatch, stream);
		storeIndices(maxId, mMesh.getNumTriangles(), mMesh.getFaceRemap(), stream, platformMismatch);
//		writeIntBuffer(mMesh.getFaceRemap(), mMesh.getNumTriangles(), platformMismatch, stream);
	}

	if(mMesh.getAdjacencies())
		writeIntBuffer(mMesh.getAdjacencies(), mMesh.getNumTriangles()*3, platformMismatch, stream);

	// Export RTree
	mMesh.getCollisionModel().mRTree.save(stream);

	// Export local bounds
	writeFloat(mMesh.mData.mCollisionModel.mGeomEpsilon, platformMismatch, stream);

	writeFloat(mMesh.mData.mAABB.minimum.x, platformMismatch, stream);
	writeFloat(mMesh.mData.mAABB.minimum.y, platformMismatch, stream);
	writeFloat(mMesh.mData.mAABB.minimum.z, platformMismatch, stream);
	writeFloat(mMesh.mData.mAABB.maximum.x, platformMismatch, stream);
	writeFloat(mMesh.mData.mAABB.maximum.y, platformMismatch, stream);
	writeFloat(mMesh.mData.mAABB.maximum.z, platformMismatch, stream);

	if(mMesh.mData.mExtraTrigData)
	{
		writeDword(mMesh.getNumTriangles(), platformMismatch, stream);
		// No need to convert those bytes
		stream.write(mMesh.mData.mExtraTrigData, mMesh.getNumTriangles()*sizeof(PxU8));
	}
	else
		writeDword(0, platformMismatch, stream);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable:4996)	// permitting use of gatherStrided until we have a replacement.

bool TriangleMeshBuilder::importMesh(const PxTriangleMeshDesc& desc,const PxCookingParams& params, bool validate)
{
	//convert and clean the input mesh
	//this is where the mesh data gets copied from user mem to our mem

	mMesh.release();
	PxVec3* verts = mMesh.allocateVertices(desc.points.count);

	Gu::TriangleT<PxU32>* tris = reinterpret_cast<Gu::TriangleT<PxU32>*>(mMesh.allocateTriangles(desc.triangles.count, true));

	//copy, and compact to get rid of strides:
	Ps::gatherStrided(desc.points.data, verts, mMesh.getNumVertices(), sizeof(PxVec3), desc.points.stride);

#ifdef PX_CHECKED
	// PT: check all input vertices are valid
	for(PxU32 i=0;i<desc.points.count;i++)
	{
		const PxVec3& p = verts[i];
		if(!PxIsFinite(p.x) || !PxIsFinite(p.y) || !PxIsFinite(p.z))
		{
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "input mesh contains corrupted vertex data");
			return false;
		}
	}
#endif

	//for trigs index stride conversion and eventual reordering is also needed, I don't think flexicopy can do that for us.

	Gu::TriangleT<PxU32>* dest = tris;
	const Gu::TriangleT<PxU32>* pastLastDest = tris + mMesh.getNumTriangles();
	const PxU8* source = (const PxU8 *)desc.triangles.data;

	//4 combos of 16 vs 32 and flip vs no flip
	PxU32 c = (desc.flags & PxMeshFlag::eFLIPNORMALS)?(PxU32)1:0;
	if (desc.flags & PxMeshFlag::e16_BIT_INDICES)
	{
		//index stride conversion is also needed, I don't think flexicopy can do that for us.
		while (dest < pastLastDest)
		{
			const PxU16 * trig16 = (const PxU16 *)source;
			dest->v[0] = trig16[0];
			dest->v[1] = trig16[1+c];
			dest->v[2] = trig16[2-c];
			dest ++;
			source += desc.triangles.stride;
		}
	}
	else
	{
		while (dest < pastLastDest)
		{
			const PxU32 * trig32 = (const PxU32 *)source;
			dest->v[0] = trig32[0];
			dest->v[1] = trig32[1+c];
			dest->v[2] = trig32[2-c];
			dest ++;
			source += desc.triangles.stride;
		}
	}

	//copy the material index list if any:
	if(desc.materialIndices.data)
	{
		PxMaterialTableIndex* materials = mMesh.allocateMaterials();
		Ps::gatherStrided(desc.materialIndices.data, materials, mMesh.getNumTriangles(), sizeof(PxMaterialTableIndex), desc.materialIndices.stride);

		// Check material indices
		for(PxU32 i=0;i<mMesh.getNumTriangles();i++)	PX_ASSERT(materials[i]!=0xffff);
	}

	// Clean the mesh using ICE's MeshBuilder
	// This fixes the bug in ConvexTest06 where the inertia tensor computation fails for a mesh => it works with a clean mesh

	// TODO: PT: MAKE SURE RETURNED INDICES FROM RAYCASTS ARE OK !
	if (!(params.meshPreprocessParams & PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH) || validate)
	{
		InternalTriangleMeshBuilder builder(&mMesh, params);
		if(!builder.cleanMesh(validate))
		{
			if(!validate)
				Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "cleaning the mesh failed");
			return false;
		}
	}
	else
	{
		// we need to fill the remap table if no cleaning was done
		if(params.suppressTriangleMeshRemapTable == false)
		{
			InternalTriangleMeshBuilder builder(&mMesh, params);
			builder.fillRemapTable();
		}
	}
	return true;
}

#pragma warning(pop)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define PROFILE_BOUNDS
#ifdef PROFILE_BOUNDS
	#include <windows.h>
	#pragma comment(lib, "winmm.lib")
#endif

void TriangleMeshBuilder::computeLocalBounds()
{
#ifdef PROFILE_BOUNDS
	int time = timeGetTime();
#endif

	PxBounds3& localBounds = mMesh.mData.mAABB;
	computeBoundsAroundVertices(localBounds, mMesh.getNumVertices(), mMesh.getVertices());

	// Derive a good geometric epsilon from local bounds. We must do this before bounds extrusion for heightfields.
	//
	// From Charles Bloom:
	// "Epsilon must be big enough so that the consistency condition abs(D(Hit))
	// <= Epsilon is satisfied for all queries. You want the smallest epsilon
	// you can have that meets that constraint. Normal floats have a 24 bit
	// mantissa. When you do any float addition, you may have round-off error
	// that makes the result off by roughly 2^-24 * result. Our result is
	// scaled by the position values. If our world is strictly required to be
	// in a box of world size W (each coordinate in -W to W), then the maximum
	// error is 2^-24 * W. Thus Epsilon must be at least >= 2^-24 * W. If
	// you're doing coordinate transforms, that may scale your error up by some
	// amount, so you'll need a bigger epsilon. In general something like
	// 2^-22*W is reasonable. If you allow scaled transforms, it needs to be
	// something like 2^-22*W*MAX_SCALE."
	// PT: TODO: runtime checkings for this
	PxReal geomEpsilon = 0.0f;
	for (PxU32 i = 0; i < 3; i++)
		geomEpsilon = PxMax(geomEpsilon, PxMax(PxAbs(localBounds.maximum[i]), PxAbs(localBounds.minimum[i])));
	geomEpsilon *= powf(2.0f, -22.0f);
	mMesh.mData.mCollisionModel.mGeomEpsilon = geomEpsilon;

#ifdef PROFILE_BOUNDS
	int deltaTime = timeGetTime() - time;
	printf("Bounds time: %f\n", float(deltaTime)*0.001f);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
