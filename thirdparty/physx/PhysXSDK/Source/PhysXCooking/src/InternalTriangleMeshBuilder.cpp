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

#include "RTreeCooking.h"
#include "InternalTriangleMeshBuilder.h"
#include "EdgeList.h"
#include "MeshCleaner.h"
#include "GuConvexEdgeFlags.h"

using namespace physx;
using namespace Gu;

InternalTriangleMeshBuilder::InternalTriangleMeshBuilder(InternalTriangleMesh* m, const PxCookingParams& params) :
	mesh(m),
	edgeList(NULL),
	mParams(params)
{
}

InternalTriangleMeshBuilder::~InternalTriangleMeshBuilder()
{
	releaseEdgeList();
}

void InternalTriangleMeshBuilder::remapTopology(const PxU32* order)
{
	PX_ASSERT(mesh);

	Gu::InternalTriangleMeshData& data = mesh->mData;

	if(!data.mNumTriangles)
		return;

	// Remap one array at a time to limit memory usage

	Gu::TriangleT<PxU32>* newTopo = (Gu::TriangleT<PxU32>*)PX_ALLOC(data.mNumTriangles * sizeof(Gu::TriangleT<PxU32>), PX_DEBUG_EXP("Gu::TriangleT<PxU32>"));
	for(PxU32 i=0;i<data.mNumTriangles;i++)
		newTopo[i]	= ((Gu::TriangleT<PxU32>*)data.mTriangles)[order[i]];
	PX_FREE_AND_RESET(data.mTriangles);
	data.mTriangles = newTopo;

	if(mesh->mMaterialIndices)
	{
		PxMaterialTableIndex* newMat = PX_NEW(PxMaterialTableIndex)[data.mNumTriangles];
		for(PxU32 i=0;i<data.mNumTriangles;i++)
			newMat[i] = mesh->mMaterialIndices[order[i]];
		PX_DELETE_POD(mesh->mMaterialIndices);
		mesh->mMaterialIndices = newMat;
	}

	if(!mParams.suppressTriangleMeshRemapTable)
	{
		PxU32* newMap = PX_NEW(PxU32)[data.mNumTriangles];
		for(PxU32 i=0;i<data.mNumTriangles;i++)
		{
//			PxU32 index = order[i];
//			newMap[index] = mesh->mFaceRemap ? mesh->mFaceRemap[i] : i;
			newMap[i] = mesh->mFaceRemap ? mesh->mFaceRemap[order[i]] : order[i];	// PT: fixed August, 15, 2004
		}
		PX_DELETE_POD(mesh->mFaceRemap);
		mesh->mFaceRemap = newMap;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RTreeCookerRemap : RTreeCooker::RemapCallback 
{
	PxU32 mNumTris;
	RTreeCookerRemap(PxU32 numTris) : mNumTris(numTris)
	{
	}

	virtual void remap(PxU32* val, PxU32 start, PxU32 leafCount)
	{
		PX_ASSERT(leafCount > 0);
		PX_ASSERT(leafCount <= 16); // sanity check
		PX_ASSERT(start < mNumTris);
		PX_ASSERT(start+leafCount <= mNumTris);
		PX_ASSERT(val);
		LeafTriangles lt;
		// here we remap from ordered leaf index in the rtree to index in post-remap in triangles
		// this post-remap will happen later
		lt.SetData(leafCount, start);
		*val = lt.Data;
	}
};

bool InternalTriangleMeshBuilder::createRTree()
{
	mesh->setupMeshInterface();

	Gu::InternalTriangleMeshData& triData = mesh->mData;

	Array<PxU32> resultPermute;
	Gu::RTree& rtree = const_cast<Gu::RTree&>(mesh->getCollisionModel().mRTree);

	RTreeCookerRemap rc(triData.mNumTriangles);
	RTreeCooker::buildFromTriangles(
		rtree,
		triData.mVertices, triData.mNumVertices,
		(triData.mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES) ? reinterpret_cast<PxU16*>(triData.mTriangles) : NULL,
		!(triData.mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES) ? reinterpret_cast<PxU32*>(triData.mTriangles) : NULL,
		triData.mNumTriangles, resultPermute, &rc, mParams.meshSizePerformanceTradeOff, mParams.meshCookingHint);

	PX_ASSERT(resultPermute.size() == triData.mNumTriangles);

	remapTopology(resultPermute.begin());

	return true;
}

void InternalTriangleMeshBuilder::fillRemapTable()
{
	PX_DELETE_POD(mesh->mFaceRemap); 

	Gu::InternalTriangleMeshData& data = mesh->mData;

	mesh->mFaceRemap = PX_NEW(PxU32)[data.mNumTriangles];
	for (PxU32 i = 0; i < data.mNumTriangles; i++)
	{
		mesh->mFaceRemap[i] = i;
	}
}

bool InternalTriangleMeshBuilder::cleanMesh(bool validate)
{
	PX_ASSERT(mesh);

	Gu::InternalTriangleMeshData& data = mesh->mData;

	{
		PxF32 meshWeldTolerance = 0.0f;
		if(mParams.meshPreprocessParams & PxMeshPreprocessingFlag::eWELD_VERTICES)
		{
			if(mParams.meshWeldTolerance == 0.f)
			{
				Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "TriangleMesh: Enable mesh welding with 0 weld tolerance!");
			}
			else
			{
				meshWeldTolerance = mParams.meshWeldTolerance;
			}
		}
		MeshCleaner cleaner(data.mNumVertices, data.mVertices, data.mNumTriangles, (const PxU32*)data.mTriangles, meshWeldTolerance);
		if(!cleaner.mNbTris)
			return false;

		if(validate)
		{
			// if we do only validate, we check if cleaning did not remove any verts or triangles. 
			// such a mesh can be then directly used for cooking without clean flag
			if((cleaner.mNbVerts != data.mNumVertices) || (cleaner.mNbTris != data.mNumTriangles))
			{
				return false;
			}
		}

		// PT: deal with the remap table
		{
			PX_DELETE_POD(mesh->mFaceRemap); 

			// PT: TODO: optimize this
			if(cleaner.mRemap)
			{
				const PxU32 newNbTris = cleaner.mNbTris;

				// Remap material array
				if(mesh->mMaterialIndices)
				{
					PxMaterialTableIndex* tmp = PX_NEW(PxMaterialTableIndex)[newNbTris];
					for(PxU32 i=0;i<newNbTris;i++)
						tmp[i] = mesh->mMaterialIndices[cleaner.mRemap[i]];

					PX_DELETE_POD(mesh->mMaterialIndices);
					mesh->mMaterialIndices = tmp;
				}

				if(!mParams.suppressTriangleMeshRemapTable)
				{
					mesh->mFaceRemap = PX_NEW(PxU32)[newNbTris];
					PxMemCopy(mesh->mFaceRemap, cleaner.mRemap, newNbTris*sizeof(PxU32));
				}
			}
		}

		// PT: deal with geometry
		{
			if(data.mNumVertices!=cleaner.mNbVerts)
			{
				PX_FREE_AND_RESET(data.mVertices);
				mesh->allocateVertices(cleaner.mNbVerts);
			}
			PxMemCopy(data.mVertices, cleaner.mVerts, data.mNumVertices*sizeof(PxVec3));
		}

		// PT: deal with topology
		{
			PX_ASSERT(!(data.mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES));
			if(data.mNumTriangles!=cleaner.mNbTris)
			{
				PX_FREE_AND_RESET(data.mTriangles);
				mesh->allocateTriangles(cleaner.mNbTris, true);
			}

			const float testLength = 500.0f*500.0f*mParams.scale.length*mParams.scale.length;
			bool bigTriangle = false;
			const PxVec3* v = data.mVertices;
			for(PxU32 i=0;i<data.mNumTriangles;i++)
			{
				const PxU32 vref0 = cleaner.mIndices[i*3+0];
				const PxU32 vref1 = cleaner.mIndices[i*3+1];
				const PxU32 vref2 = cleaner.mIndices[i*3+2];
				PX_ASSERT(vref0!=vref1 && vref0!=vref2 && vref1!=vref2);

				reinterpret_cast<Gu::TriangleT<PxU32>*>(data.mTriangles)[i].v[0] = vref0;
				reinterpret_cast<Gu::TriangleT<PxU32>*>(data.mTriangles)[i].v[1] = vref1;
				reinterpret_cast<Gu::TriangleT<PxU32>*>(data.mTriangles)[i].v[2] = vref2;

				if(		(v[vref0] - v[vref1]).magnitudeSquared() >= testLength
					||	(v[vref1] - v[vref2]).magnitudeSquared() >= testLength
					||	(v[vref2] - v[vref0]).magnitudeSquared() >= testLength
					)
					bigTriangle = true;
			}
			if(bigTriangle)
				Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "TriangleMesh: triangles are too big, reduce their size to increase simulation stability!");
		}
	}
	return true;
}

void InternalTriangleMeshBuilder::createSharedEdgeData(bool buildAdjacencies, bool buildActiveEdges)
{
	PX_ASSERT(mesh);

	if (buildAdjacencies) // building edges is required if buildAdjacencies is requested
		buildActiveEdges = true;

	PX_DELETE_POD(mesh->mData.mExtraTrigData); 
	PX_DELETE_POD(mesh->mAdjacencies);

	PxU32 nTrigs = mesh->getNumTriangles();

	mesh->mData.mExtraTrigData = PX_NEW(PxU8)[nTrigs];
	if (!buildActiveEdges)
		memset(mesh->mData.mExtraTrigData, 8+16+32, sizeof(PxU8)*nTrigs); // all edges are convex by default (see ETD_CONVEX_EDGE)
	else
		memset(mesh->mData.mExtraTrigData, 0, sizeof(PxU8)*nTrigs); 

	if (!buildActiveEdges)
		return;
	
	const Gu::TriangleT<PxU32>* trigs = reinterpret_cast<const Gu::TriangleT<PxU32>*>(mesh->getTriangles());
	if(0x40000000 <= nTrigs)
	{
		//mesh is too big for this algo, need to be able to express trig indices in 30 bits, and still have an index reserved for "unused":
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "TriangleMesh: mesh is too big for this algo!");
		return;
	}
	
	createEdgeList();
	if(edgeList)
	{
		PX_ASSERT(edgeList->GetNbFaces()==mesh->getNumTriangles());
		if(edgeList->GetNbFaces()==mesh->getNumTriangles())
		{
			for(PxU32 i=0;i<edgeList->GetNbFaces();i++)
			{
				const Gu::EdgeTriangleData& ET = edgeList->GetEdgeTriangle(i);
				// Replicate flags
				if(Gu::EdgeTriangleAC::HasActiveEdge01(ET))	mesh->mData.mExtraTrigData[i] |= Gu::ETD_CONVEX_EDGE_01;
				if(Gu::EdgeTriangleAC::HasActiveEdge12(ET))	mesh->mData.mExtraTrigData[i] |= Gu::ETD_CONVEX_EDGE_12;
				if(Gu::EdgeTriangleAC::HasActiveEdge20(ET))	mesh->mData.mExtraTrigData[i] |= Gu::ETD_CONVEX_EDGE_20;
			}
		}
	}

	// fill the adjacencies
	if(buildAdjacencies)
	{
		mesh->mAdjacencies = PX_NEW(PxU32)[nTrigs*3];
		memset(mesh->mAdjacencies, 0xFFFFffff, sizeof(PxU32)*nTrigs*3);	
		mesh->mNumAdjacencies = nTrigs*3;

		PxU32 NbEdges = edgeList->GetNbEdges();
		const Gu::EdgeDescData* ED = edgeList->GetEdgeToTriangles();
		const Gu::EdgeData* Edges = edgeList->GetEdges();
		const PxU32* FBE = edgeList->GetFacesByEdges();

		while(NbEdges--)
		{
			// Get number of triangles sharing current edge
			PxU32 Count = ED->Count;
			
			if(Count > 1)
			{
				PxU32 FaceIndex0 = FBE[ED->Offset+0];
				PxU32 FaceIndex1 = FBE[ED->Offset+1];

				const Gu::EdgeData& edgeData = *Edges;
				const Gu::TriangleT<PxU32>& T0 = trigs[FaceIndex0];
				const Gu::TriangleT<PxU32>& T1 = trigs[FaceIndex1];

				PxU32 offset0 = T0.findEdgeCCW(edgeData.Ref0,edgeData.Ref1);				
				PxU32 offset1 = T1.findEdgeCCW(edgeData.Ref0,edgeData.Ref1);

				mesh->setTriangleAdjacency(FaceIndex0, FaceIndex1, offset0);
				mesh->setTriangleAdjacency(FaceIndex1, FaceIndex0, offset1);
			}
			ED++;
			Edges++;
		}
	}

#ifdef PX_DEBUG
	for(PxU32 i=0;i<mesh->getNumTriangles();i++)
	{
		const Gu::TriangleT<PxU32>& T = trigs[i];
		PX_UNUSED(T);
		const Gu::EdgeTriangleData& ET = edgeList->GetEdgeTriangle(i);
		PX_ASSERT((Gu::EdgeTriangleAC::HasActiveEdge01(ET) && (mesh->mData.mExtraTrigData[i] & Gu::ETD_CONVEX_EDGE_01)) || (!Gu::EdgeTriangleAC::HasActiveEdge01(ET) && !(mesh->mData.mExtraTrigData[i] & Gu::ETD_CONVEX_EDGE_01)));
		PX_ASSERT((Gu::EdgeTriangleAC::HasActiveEdge12(ET) && (mesh->mData.mExtraTrigData[i] & Gu::ETD_CONVEX_EDGE_12)) || (!Gu::EdgeTriangleAC::HasActiveEdge12(ET) && !(mesh->mData.mExtraTrigData[i] & Gu::ETD_CONVEX_EDGE_12)));
		PX_ASSERT((Gu::EdgeTriangleAC::HasActiveEdge20(ET) && (mesh->mData.mExtraTrigData[i] & Gu::ETD_CONVEX_EDGE_20)) || (!Gu::EdgeTriangleAC::HasActiveEdge20(ET) && !(mesh->mData.mExtraTrigData[i] & Gu::ETD_CONVEX_EDGE_20)));
	}
#endif
	return;
}

void InternalTriangleMeshBuilder::createEdgeList()
{
	Gu::EDGELISTCREATE create;
	create.NbFaces		= mesh->getNumTriangles();
	if(mesh->has16BitIndices())
	{
		create.DFaces		= NULL;
		create.WFaces		= (PxU16*)mesh->getTriangles();
	}
	else
	{
		create.DFaces		= (PxU32*)mesh->getTriangles();
		create.WFaces		= NULL;
	}
	create.FacesToEdges	= true;
	create.EdgesToFaces	= true;
	create.Verts		= mesh->getVertices();
	//create.Epsilon = 0.1f;
	//	create.Epsilon		= convexEdgeThreshold;
	edgeList = PX_NEW(Gu::EdgeListBuilder);
	if(!edgeList->Init(create))
	{
		PX_DELETE(edgeList);
		edgeList = 0;
	}
}

void InternalTriangleMeshBuilder::releaseEdgeList()
{
	PX_DELETE(edgeList);
	edgeList = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

