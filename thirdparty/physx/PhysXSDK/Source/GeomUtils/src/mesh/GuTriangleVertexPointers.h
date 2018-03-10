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

#ifndef GU_TRIANGLE_VERTEX_POINTERS_H
#define GU_TRIANGLE_VERTEX_POINTERS_H

#include "PxTriangleMesh.h"
#include "GuTriangleMeshData.h"
#include "GuTriangle32.h"
#include "CmMemFetch.h"

namespace physx
{
using namespace Cm;
namespace Gu
{
	struct IndexTriple16 { PxU16 p0, p1, p2; };
	struct IndexTriple32 { PxU32 p0, p1, p2; };
	class TriangleVertexPointers
	{
	public:
		PX_FORCE_INLINE const PxVec3& operator[](PxU32 x) const
		{
			return *v[x];
		}

		TriangleVertexPointers()
		{
		}

		PX_FORCE_INLINE TriangleVertexPointers(const Gu::InternalTriangleMeshData& mesh, PxU32 triangleIndex)
		{
			set(mesh, triangleIndex);
		}

		static PX_FORCE_INLINE bool has16BitIndices(const Gu::InternalTriangleMeshData& mesh)	{ return mesh.mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES ? true : false; }

		PX_FORCE_INLINE void set(const Gu::InternalTriangleMeshData& mesh, PxU32 triangleIndex)
		{
			if (has16BitIndices(mesh))
			{
				const Gu::TriangleT<PxU16>& indices = (reinterpret_cast<const Gu::TriangleT<PxU16>*>(mesh.mTriangles))[triangleIndex];
				v[0] = &(mesh.mVertices[indices[0]]);
				v[1] = &(mesh.mVertices[indices[1]]);
				v[2] = &(mesh.mVertices[indices[2]]);
			}
			else
			{
				const Gu::TriangleT<PxU32>& indices = (reinterpret_cast<const Gu::TriangleT<PxU32>*>(mesh.mTriangles))[triangleIndex];
				v[0] = &(mesh.mVertices[indices[0]]);
				v[1] = &(mesh.mVertices[indices[1]]);
				v[2] = &(mesh.mVertices[indices[2]]);
			}
		}

		// AP: refactor needed, see US14379
		static void PX_FORCE_INLINE getTriangleVerts(
			const Gu::InternalTriangleMeshData* meshDataLocalStorage,
			PxU32 TriangleIndex, PxVec3& v0, PxVec3& v1, PxVec3& v2)
		{
			MemFetchPtr mTris = MemFetchPtr(meshDataLocalStorage->mTriangles);
			MemFetchPtr mVerts = MemFetchPtr(meshDataLocalStorage->mVertices);
			MemFetchSmallBuffer buf0, buf1, buf2;
			PxU32 i0, i1, i2;
			if (meshDataLocalStorage->mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES)
			{
				IndexTriple16* inds = memFetchAsync<IndexTriple16>(mTris+(sizeof(PxU16)*TriangleIndex*3), 5, buf0);
				memFetchWait(5);
				i0 = inds->p0; i1 = inds->p1; i2 = inds->p2;
			} 
			else 
			{ 
				IndexTriple32* inds = memFetchAsync<IndexTriple32>(mTris+(sizeof(PxU32)*TriangleIndex*3), 5, buf0);
				memFetchWait(5);
				i0 = inds->p0; i1 = inds->p1; i2 = inds->p2;
			} 

			PxVec3* vertex[3];
			vertex[0] = memFetchAsync<PxVec3>(mVerts+i0*12+0, 5, buf0);
			vertex[1] = memFetchAsync<PxVec3>(mVerts+i1*12+0, 5, buf1);
			vertex[2] = memFetchAsync<PxVec3>(mVerts+i2*12+0, 5, buf2);
			memFetchWait(5);
			v0 = *vertex[0]; v1 = *vertex[1]; v2 = *vertex[2];
		}

		template<int N> static void PX_FORCE_INLINE getTriangleVertsN(
			const Gu::InternalTriangleMeshData* meshDataLocalStorage,
			const PxU32* PX_RESTRICT triIndices, PxU32 indexCount, PxVec3 output[N][3])
		{
			MemFetchPtr mTris = MemFetchPtr(meshDataLocalStorage->mTriangles);
			MemFetchPtr mVerts = MemFetchPtr(meshDataLocalStorage->mVertices);
			MemFetchSmallBuffer buf0[N], buf1[N], buf2[N], buf3[N];

			PxVec3* v[N][3];
			PX_ASSERT(indexCount <= N);
			if (meshDataLocalStorage->mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES)
			{
				IndexTriple16* inds[N];
				for (PxU32 i = 0; i < indexCount; i++)
					inds[i] = memFetchAsync<IndexTriple16>(mTris+(sizeof(PxU16)*triIndices[i]*3), i/*dma tag*/, buf3[i]);

				for (PxU32 i = 0; i < indexCount; i++)
				{
					memFetchWait(i);
					v[i][0] = memFetchAsync<PxVec3>(mVerts+inds[i]->p0*12+0, i, buf0[i]);
					v[i][1] = memFetchAsync<PxVec3>(mVerts+inds[i]->p1*12+0, i, buf1[i]);
					v[i][2] = memFetchAsync<PxVec3>(mVerts+inds[i]->p2*12+0, i, buf2[i]);
				}
			} 
			else 
			{ 
				IndexTriple32* inds[N];
				for (PxU32 i = 0; i < indexCount; i++)
					inds[i] = memFetchAsync<IndexTriple32>(mTris+(sizeof(PxU32)*triIndices[i]*3), i/*dma tag*/, buf3[i]);

				for (PxU32 i = 0; i < indexCount; i++)
				{
					memFetchWait(i);
					v[i][0] = memFetchAsync<PxVec3>(mVerts+inds[i]->p0*12+0, i, buf0[i]);
					v[i][1] = memFetchAsync<PxVec3>(mVerts+inds[i]->p1*12+0, i, buf1[i]);
					v[i][2] = memFetchAsync<PxVec3>(mVerts+inds[i]->p2*12+0, i, buf2[i]);
				}
			} 

			for (PxU32 i = 0; i < indexCount; i++)
			{
				memFetchWait(i);
				output[i][0] = *v[i][0];
				output[i][1] = *v[i][1];
				output[i][2] = *v[i][2];
			}
		}
	private:
		const PxVec3* v[3];
	};
}
//#endif
}

#endif
