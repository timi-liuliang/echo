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


#include "CookingUtils.h"
#include "PxMath.h"
#include "GuRevisitedRadixBuffered.h"

using namespace physx;
using namespace Gu;

ReducedVertexCloud::ReducedVertexCloud(const PxVec3* verts, PxU32 nb_verts) : mNbRVerts(0), mRVerts(NULL), mXRef(NULL)
{
	mVerts		= verts;
	mNbVerts	= nb_verts;
}

ReducedVertexCloud::~ReducedVertexCloud()
{
	Clean();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Frees used ram.
*	\return		Self-reference
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ReducedVertexCloud& ReducedVertexCloud::Clean()
{
	PX_DELETE_POD(mXRef);
	PX_FREE_AND_RESET(mRVerts);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Reduction method. Use this to create a minimal vertex cloud.
*	\param		rc		[out] result structure
*	\return		true if success
*	\warning	This is not about welding nearby vertices, here we look for real redundant ones.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ReducedVertexCloud::Reduce(REDUCEDCLOUD* rc)
{
	Clean();

	mXRef = PX_NEW(PxU32)[mNbVerts];

	float* f = PX_NEW_TEMP(float)[mNbVerts];

	for(PxU32 i=0;i<mNbVerts;i++)
		f[i] = mVerts[i].x;

	RadixSortBuffered Radix;
	Radix.Sort((const PxU32*)f, mNbVerts, RADIX_UNSIGNED);

	for(PxU32 i=0;i<mNbVerts;i++)
		f[i] = mVerts[i].y;
	Radix.Sort((const PxU32*)f, mNbVerts, RADIX_UNSIGNED);

	for(PxU32 i=0;i<mNbVerts;i++)
		f[i] = mVerts[i].z;
	const PxU32* Sorted = Radix.Sort((const PxU32*)f, mNbVerts, RADIX_UNSIGNED).GetRanks();

	PX_DELETE_POD(f);

	mNbRVerts = 0;
	PxU32 Junk[] = {PX_INVALID_U32, PX_INVALID_U32, PX_INVALID_U32};
	const PxVec3* Previous = (const PxVec3*)Junk;
	mRVerts = (PxVec3*)PX_ALLOC(sizeof(PxVec3) * mNbVerts, PX_DEBUG_EXP("PxVec3"));
	PxU32 Nb = mNbVerts;
	while(Nb--)
	{
		const PxU32 Vertex = *Sorted++;	// Vertex number

		if(		PX_IR(mVerts[Vertex].x)!=PX_IR(Previous->x)
			||	PX_IR(mVerts[Vertex].y)!=PX_IR(Previous->y)
			||	PX_IR(mVerts[Vertex].z)!=PX_IR(Previous->z))
		{
			mRVerts[mNbRVerts++] = mVerts[Vertex];
		}
		Previous = &mVerts[Vertex];

		mXRef[Vertex] = mNbRVerts-1;
	}

	if(rc)
	{
		rc->CrossRef	= mXRef;
		rc->NbRVerts	= mNbRVerts;
		rc->RVerts		= mRVerts;
	}
	return true;
}
