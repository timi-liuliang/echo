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


#include "GuConvexMeshData.h"
#include "EdgeList.h"
#include "PxCooking.h"
#include "BigConvexDataBuilder.h"
#include "GuBigConvexData2.h"
#include "PsUserAllocated.h"
#include "GuIntersectionRayPlane.h"
#include "PsMathUtils.h"
#include "GuSerialize.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace physx;
using namespace Gu;

static const PxU32 gSupportVersion = 0;
static const PxU32 gVersion = 0;

BigConvexDataBuilder::BigConvexDataBuilder(const Gu::ConvexHullData* hull, BigConvexData* gm, const PxVec3* hullVerts) : mHullVerts(hullVerts)
{
	mSVM = gm;
	mHull = hull;
}

BigConvexDataBuilder::~BigConvexDataBuilder()
{
}

bool BigConvexDataBuilder::Initialize()
{
	mSVM->mData.mSamples = PX_NEW(PxU8)[(unsigned int)(mSVM->mData.mNbSamples*2)];

#ifdef PX_DEBUG
//	printf("SVM: %d bytes\n", mNbSamples*sizeof(PxU8)*2);
#endif

	return true;
}

bool BigConvexDataBuilder::PrecomputeSample(PxU32 offset, const PxVec3& dir)
{
	PxU32 NbVerts = mHull->mNbHullVertices;
//	const PxVec3* Verts = mSVM->mHull->GetVerts();
	const PxVec3* Verts = mHullVerts;

	float minimum = PX_MAX_F32;
	float min2 = PX_MAX_F32;
	PxU32 MinID = 0;
	PxU32 MinID2 = 0;

	for(PxU32 i=0;i<NbVerts;i++)
	{
		const float dp = Verts[i].dot(dir);

		if(dp < minimum)
		{
			minimum = dp;
			MinID = i;
		}

		if((-dp) < min2)
		{
			min2 = -dp;
			MinID2 = i;
		}
	}

	mSVM->mData.mSamples[offset] = (PxU8)MinID;
	mSVM->mData.mSamples[offset + mSVM->mData.mNbSamples] = (PxU8)MinID2;

	return true;
}

bool BigConvexDataBuilder::Save(PxOutputStream& stream, bool platformMismatch, const PxU32 nbFaces, const HullTriangleData* faces) const
{
	// Export header
	if(!WriteHeader('S', 'U', 'P', 'M', gSupportVersion, platformMismatch, stream))
		return false;

	// Save base gaussmap
//	if(!GaussMapBuilder::Save(stream, platformMismatch))	return false;
		// Export header
		if(!WriteHeader('G', 'A', 'U', 'S', gVersion, platformMismatch, stream))
			return false;

		// Export basic info
	//	stream.StoreDword(mSubdiv);
		writeDword(mSVM->mData.mSubdiv, platformMismatch, stream);		// PT: could now write Word here
	//	stream.StoreDword(mNbSamples);
		writeDword(mSVM->mData.mNbSamples, platformMismatch, stream);	// PT: could now write Word here

	// Save map data
	// It's an array of bytes so we don't care about 'PlatformMismatch'
	stream.write(mSVM->mData.mSamples, sizeof(PxU8)*mSVM->mData.mNbSamples*2);

	// Export valencies
	// TODO: allow lazy-evaluation
	ComputeValencies(nbFaces, faces);

	if(!Save(stream, platformMismatch))	return false;

	return true;
}


bool BigConvexDataBuilder::ComputeValencies(const PxU32 nbFaces, const HullTriangleData* faces) const
{
	// Create valencies
	VALENCIESCREATE vc;
	vc.NbVerts		= mHull->mNbHullVertices;
	vc.NbFaces		= nbFaces;

	vc.dFaces		= (const PxU32*)faces;
	vc.AdjacentList	= true;

	if(!Compute(vc))	
		return false;	

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BigConvexDataBuilder::Precompute(PxU32 subdiv)
{
	const PxU32 Subdiv = subdiv;
	const PxU32 NbSamples = 6*subdiv*subdiv;
	mSVM->mData.mSubdiv = Ps::to16(Subdiv);
	mSVM->mData.mNbSamples = Ps::to16(NbSamples);

	if(!Initialize())	return false;

	const float HalfSubdiv = float(subdiv-1) * 0.5f;
	for(PxU32 Face=0;Face<6;Face++)
	{
		for(PxU32 j=0;j<subdiv;j++)
		{
			for(PxU32 i=0;i<subdiv;i++)
			{
				const PxU32 Offset = j+i*subdiv+Face*subdiv*subdiv;
				PX_ASSERT(Offset<mSVM->mData.mNbSamples);

				PxVec3 Dir;

				switch(Face)
				{
					case 0:
					case 1:
						Dir.x = Face==0 ? -1.0f : 1.0f;
						Dir.y = 1.0f - i / HalfSubdiv;
						Dir.z = 1.0f - j / HalfSubdiv;
					break;

					case 2:
					case 3:
						Dir.y = Face==2 ? -1.0f : 1.0f;
						Dir.z = 1.0f - i / HalfSubdiv;
						Dir.x = 1.0f - j / HalfSubdiv;
					break;

					case 4:
					case 5:
						Dir.z = Face==4 ? -1.0f : 1.0f;
						Dir.x = 1.0f - i / HalfSubdiv;
						Dir.y = 1.0f - j / HalfSubdiv;
					break;
				}

				Dir.normalize();
				if(!PrecomputeSample(Offset, Dir))	return false;
			}
		}
	}
	return true;
}




static const PxU32 gValencyVersion = 2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes valencies and adjacent vertices.
 *	After the call, get results with the appropriate accessors.
 *
 *	\param		vc		[in] creation structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool BigConvexDataBuilder::Compute(const VALENCIESCREATE& vc) const
{
	mSVM->mData.mNbVerts	= vc.NbVerts;

	// Get ram for valencies
	mSVM->mData.mValencies = PX_NEW(Gu::Valency)[mSVM->mData.mNbVerts];
	PxMemZero(mSVM->mData.mValencies, mSVM->mData.mNbVerts*sizeof(Gu::Valency));

	// Compute valencies
	Gu::EdgeListBuilder EL;
	{
		Gu::EDGELISTCREATE ELC;
		ELC.NbFaces			= vc.NbFaces;
		ELC.DFaces			= vc.dFaces;
		ELC.WFaces			= vc.wFaces;
		ELC.FacesToEdges	= true;

		// Create edge info
		if(!EL.Init(ELC))	return false;
//		bool Status = EL.CreateFacesToEdges(vc.NbFaces, vc.dFaces, vc.wFaces);
//		if(!Status) return false;

		// Compute valencies
		for(PxU32 i=0;i<EL.GetNbEdges();i++)
		{
//			mValencies[EL.GetEdge(i).mRef0]++;
//			mValencies[EL.GetEdge(i).mRef1]++;
			mSVM->mData.mValencies[EL.GetEdge(i).Ref0].mCount++;
			mSVM->mData.mValencies[EL.GetEdge(i).Ref1].mCount++;
			PX_ASSERT(mSVM->mData.mValencies[EL.GetEdge(i).Ref0].mCount!=0xffff);
			PX_ASSERT(mSVM->mData.mValencies[EL.GetEdge(i).Ref1].mCount!=0xffff);
		}
	}

	// Should I then compute adjacent vertices for each vertex ?
	if(vc.AdjacentList)
	{
		// Get ram for offsets
//		mOffsets = (PxU32*)ICE_ALLOC_MEM(sizeof(PxU32)*mNbVerts,Valency_Offsets);

		// Create offsets
		mSVM->CreateOffsets();

//		mNbAdjVerts = mOffsets[mNbVerts-1] + mValencies[mNbVerts-1];
		mSVM->mData.mNbAdjVerts = PxU32(mSVM->mData.mValencies[mSVM->mData.mNbVerts-1].mOffset + mSVM->mData.mValencies[mSVM->mData.mNbVerts-1].mCount);
		PX_ASSERT(mSVM->mData.mNbAdjVerts==EL.GetNbEdges()*2);

		// Get ram for adjacent vertices references
		mSVM->mData.mAdjacentVerts = PX_NEW(PxU8)[mSVM->mData.mNbAdjVerts];

		// Create adjacent vertices
		for(PxU32 i=0;i<EL.GetNbEdges();i++)
		{
			PxU32 Ref0 = EL.GetEdge(i).Ref0;
			PxU32 Ref1 = EL.GetEdge(i).Ref1;
			PX_ASSERT(Ref0<256);
			PX_ASSERT(Ref1<256);
			mSVM->mData.mAdjacentVerts[mSVM->mData.mValencies[Ref0].mOffset++] = (PxU8)Ref1;
			mSVM->mData.mAdjacentVerts[mSVM->mData.mValencies[Ref1].mOffset++] = (PxU8)Ref0;
		}

		// Recreate offsets
		mSVM->CreateOffsets();
	}

	return true;
}

bool BigConvexDataBuilder::Save(PxOutputStream& stream, bool platformMismatch) const
{
	// Export header
	if(!WriteHeader('V', 'A', 'L', 'E', gValencyVersion, platformMismatch, stream))
		return false;

	writeDword(mSVM->mData.mNbVerts, platformMismatch, stream);
	writeDword(mSVM->mData.mNbAdjVerts, platformMismatch, stream);

	{
		PxU16* temp = PX_NEW_TEMP(PxU16)[mSVM->mData.mNbVerts];
		for(PxU32 i=0;i<mSVM->mData.mNbVerts;i++)
			temp[i] = mSVM->mData.mValencies[i].mCount;

		const PxU32 maxIndex = computeMaxIndex(temp, mSVM->mData.mNbVerts);
		writeDword(maxIndex, platformMismatch, stream);
		StoreIndices(Ps::to16(maxIndex), mSVM->mData.mNbVerts, temp, stream, platformMismatch);

		PX_DELETE_POD(temp);
	}
	stream.write(mSVM->mData.mAdjacentVerts, mSVM->mData.mNbAdjVerts);

/*	{
		PxU32 maxIndex = ComputeMaxIndex(mValencies, mNbVerts);
		WriteDword(maxIndex, PlatformMismatch, stream);
		StoreIndices(maxIndex, mNbVerts, mValencies, stream, PlatformMismatch);
	}

	{
		PxU32 maxIndex = ComputeMaxIndex(mAdjacentVerts, mNbAdjVerts);
		WriteDword(maxIndex, PlatformMismatch, stream);
		StoreIndices(maxIndex, mNbAdjVerts, mAdjacentVerts, stream, PlatformMismatch);
	}*/

//	WriteDwordBuffer(mValencies, mNbVerts, PlatformMismatch, stream);
//	WriteDwordBuffer(mAdjacentVerts, mNbAdjVerts, PlatformMismatch, stream);
	// Offset will be recreated

	return true;
}
