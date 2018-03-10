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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "PsIntrinsics.h"
#include "PsUserAllocated.h"
#include "GuMeshInterface.h"

using namespace physx;
using namespace Gu;

MeshInterface::MeshInterface() :
	mNbTris			(0),
	mNbVerts		(0),
	mTris			(NULL),
	mVerts			(NULL)
{
}

MeshInterface::~MeshInterface()
{
}

bool MeshInterface::IsValid() const
{
	if(!mNbTris || !mNbVerts)	return false;
	if(!mTris || !mVerts)		return false;
	return true;
}

PxU32 MeshInterface::CheckTopology()	const
{
	// Check topology. If the model contains degenerate faces, collision report can be wrong in some cases.
	// e.g. it happens with the standard MAX teapot. So clean your meshes first... If you don't have a mesh cleaner
	// you can try this: www.codercorner.com/Consolidation.zip

	PxU32 NbDegenerate = 0;

	VertexPointers VP;

	// Using callbacks, we don't have access to vertex indices. Nevertheless we still can check for
	// redundant vertex pointers, which cover all possibilities (callbacks/pointers/strides).
	for(PxU32 i=0;i<mNbTris;i++)
	{
		GetTriangle(VP, i);

		if(		(VP.vertex[0]==VP.vertex[1])
			||	(VP.vertex[1]==VP.vertex[2])
			||	(VP.vertex[2]==VP.vertex[0]))	NbDegenerate++;
	}

	return NbDegenerate;
}

bool MeshInterface::SetPointers(const void* tris, bool has16BitIndices, const PxVec3* verts)
{
	if(!tris || !verts)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "MeshInterface::SetPointers: pointer is NULL");
		return false;
	}

	mTris	= tris;
	mVerts	= verts;
	mHas16BitIndices = (PxU32)has16BitIndices;
	return true;
}
