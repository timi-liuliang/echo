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

#ifndef GU_MIDPHASE_TRACE_H
#define GU_MIDPHASE_TRACE_H

#include "PxTransform.h"
#include "PxMeshScale.h"
#include "CmMidphaseTrace.h"

#if MIDPHASE_TRACE

#pragma warning(disable:4350)
#pragma warning(disable:4530)


#include <stdio.h>
#include <map>
#include <vector>
#include "PsAtomic.h"

FILE* sqTrace = NULL;
std::vector<PxU32> inds32;
std::vector<PxVec3> testVerts;
volatile PxI32 sqTraceLocked = 0;
PxU32 meshId = 0;


namespace physx { namespace Gu {

void closeTrace()
{
	fclose(sqTrace);
	sqTrace = NULL;
}

PX_PHYSX_COMMON_API PxU32 getMeshId()
{
	PxU32 tmp = meshId;
	meshId++;
	return tmp;
}

PX_PHYSX_COMMON_API void writeTrace(
	const char* tag, PxTransform& pose, PxMeshScale& scale, const Ps::Array<PxU32>& touchedLeaves,
	bool writeMesh, PxU32 meshId, const PxVec3* verts, PxU32 nbVerts, const void* inds, PxU32 nbInds, PxU32 has16bitIndices,
	const PxVec3* orig, const PxVec3* dir, const PxReal* maxT, PxU32 bothSides, const PxVec3* inflate, const Gu::Box* obb,
	const PxHeightFieldGeometry* hfGeom)
{
	PX_UNUSED(tag); PX_UNUSED(pose); PX_UNUSED(scale); PX_UNUSED(touchedLeaves); PX_UNUSED(writeMesh); PX_UNUSED(meshId); PX_UNUSED(verts);
	PX_UNUSED(nbVerts); PX_UNUSED(inds); PX_UNUSED(nbInds); PX_UNUSED(has16bitIndices); PX_UNUSED(orig); PX_UNUSED(dir);
	PX_UNUSED(maxT); PX_UNUSED(bothSides); PX_UNUSED(inflate); PX_UNUSED(obb); PX_UNUSED(hfGeom);
#if 0
	if (writeMesh)
	{
		PX_CHECK(nbInds %3 == 0);
		PX_CHECK(nbInds >= 3);
		PX_CHECK(nbVerts > 0);
		if (nbInds %3 != 0 || nbInds < 3 || nbVerts == 0) {
			int a = 1; PX_UNUSED(a); }
	}
	PX_UNUSED(hfGeom); PX_UNUSED(touchedLeaves);
	// obtain the lock
	while (Ps::atomicCompareExchange(&sqTraceLocked, 1, 0) == 1)
		PX_CHECK(false);

	// open the trace file if not open yet
	if (sqTrace == NULL)
	{
		sqTrace = fopen("c:/1/sqTrace.bin", "wb");
		atexit(closeTrace);
	}

	PxU32 tmp = 0;
	// check if this is the first time we see this mesh, if it is, write out the mesh data and save the mesh in meshMap
	if (writeMesh)
	{
		// write code=0 for 'add new mesh' code
		tmp = 0;
		fwrite(&tmp, 4, 1, sqTrace);

		// add new mesh id to the meshMap, write it out and increment the meshId
		//meshMap1[computeFullHash(verts, nbVerts, inds, nbInds, has16bitIndices)] = meshId;
		//meshMap[hash] = meshId;
		fwrite(&meshId, 4, 1, sqTrace);

		// convert 16-bit to 32-bit indices if needed
		inds32.resize(nbInds);
		for (PxU32 i = 0; i < nbInds; i++)
			inds32[i] = has16bitIndices ? ((PxU16*)inds)[i] : ((PxU32*)inds)[i];

		// write out vertex and index data
		fwrite(&nbVerts, sizeof(nbVerts), 1, sqTrace); // 4 bytes
		fwrite(verts, sizeof(verts[0]), nbVerts, sqTrace); // 12 bytes * nbVerts
		fwrite(&nbInds, sizeof(nbInds), 1, sqTrace); // 4 bytes
		fwrite(inds32.data(), 4, nbInds, sqTrace); // 4 bytes*nbInds
	}

	PX_ASSERT(orig || obb);

	// write 1 for raycast, 2 for inflated raycast, 3 for obb query
	tmp = obb ? PxU32(3) : (inflate ? PxU32(2) : PxU32(1));
	fwrite(&tmp, 4, 1, sqTrace);


	fwrite(tag, 4, 1, sqTrace);
	tmp = meshId;
	fwrite(&tmp, 4, 1, sqTrace);
	fwrite(&pose.p, sizeof(pose.p), 1, sqTrace);
	fwrite(&pose.q, sizeof(pose.q), 1, sqTrace);
	fwrite(&scale.scale, sizeof(scale.scale), 1, sqTrace);
	fwrite(&scale.rotation, sizeof(scale.rotation), 1, sqTrace);
	if (obb)
	{
		// code=3
		fwrite(&(obb->rot.column0), sizeof(obb->rot.column0), 1, sqTrace); // 12 bytes
		fwrite(&(obb->rot.column1), sizeof(obb->rot.column1), 1, sqTrace); // 12 bytes
		fwrite(&(obb->rot.column2), sizeof(obb->rot.column2), 1, sqTrace); // 12 bytes
		fwrite(&(obb->center), sizeof(obb->center), 1, sqTrace); // 12 bytes
		fwrite(&(obb->extents), sizeof(obb->extents), 1, sqTrace); // 12 bytes
	} else
	{
		if (inflate) // code=2, otherwise code=1
			fwrite(inflate, sizeof(*inflate), 1, sqTrace);
		fwrite(orig, sizeof(*orig), 1, sqTrace);
		fwrite(dir, sizeof(*dir), 1, sqTrace);
		fwrite(maxT, sizeof(*maxT), 1, sqTrace);
		fwrite(&bothSides, sizeof(bothSides), 1, sqTrace);
	}
	tmp = touchedLeaves.size();
	fwrite(&tmp, 4, 1, sqTrace);
	fwrite(touchedLeaves.begin(), 4, tmp, sqTrace);
	tmp = 0xAB5C155A;
	fwrite(&tmp, 4, 1, sqTrace);
	fflush(sqTrace);
	Ps::atomicExchange(&sqTraceLocked, 0);
#endif
}

}}

#endif // MIDPHASE_TRACE

#endif // header guard
