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


#include "GuVecTriangle.h"
#include "GuVecCapsule.h"
#include "GuGeometryUnion.h"
#include "PsVecMath.h"
#include "PsVecTransform.h"

#include "GuContactMethodImpl.h"
#include "GuTriangleMeshData.h"
#include "PxTriangleMesh.h"
#include "GuContactBuffer.h"
#include "GuMidphase.h"
#include "GuPCMContactConvexCommon.h"
#include "GuSegment.h"
#include "GuVecCapsule.h"
#include "GuGeomUtilsInternal.h"
#include "GuPCMContactMeshCallback.h"
#include "GuConvexEdgeFlags.h"

using namespace physx;
using namespace Gu;
using namespace physx::shdfnd::aos;

#ifdef	PCM_LOW_LEVEL_DEBUG
#include "CmRenderOutput.h"
extern physx::Cm::RenderOutput* gRenderOutPut;
#endif

#if defined(__SPU__)
#include "CmMemFetch.h"
#endif


namespace physx
{

struct PCMCapsuleVsMeshContactGenerationCallback : PCMMeshContactGenerationCallback< PCMCapsuleVsMeshContactGenerationCallback >
{
	PCMCapsuleVsMeshContactGenerationCallback& operator=(const PCMCapsuleVsMeshContactGenerationCallback&);
public:
	PCMCapsuleVsMeshContactGeneration		mGeneration;

	PCMCapsuleVsMeshContactGenerationCallback(
		const Gu::CapsuleV&			capsule,
		const Ps::aos::FloatVArg	contactDist,
		const Ps::aos::FloatVArg	replaceBreakingThreshold,
		const PsTransformV&			sphereTransform,
		const PsTransformV&			meshTransform,
		Gu::MultiplePersistentContactManifold& multiManifold,
		Gu::ContactBuffer&			contactBuffer,
		const PxU8*					extraTriData,
		const Cm::FastVertex2ShapeScaling& meshScaling,
		bool						idtMeshScale
	) :
		PCMMeshContactGenerationCallback<PCMCapsuleVsMeshContactGenerationCallback>(meshScaling, extraTriData, idtMeshScale),
		mGeneration(capsule, contactDist, replaceBreakingThreshold, sphereTransform, meshTransform, multiManifold, contactBuffer)
	{
	}

	PX_FORCE_INLINE bool doTest(const PxVec3&, const PxVec3&, const PxVec3&) { return true; }

	template<PxU32 CacheSize>
	void processTriangleCache(Gu::TriangleCache<CacheSize>& cache)
	{
		mGeneration.processTriangleCache<CacheSize, PCMCapsuleVsMeshContactGeneration>(cache);
	}
	
};

bool Gu::pcmContactCapsuleMesh(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;
	Gu::MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();
	const PxCapsuleGeometry& shapeCapsule= shape0.get<const PxCapsuleGeometry>();
	const PxTriangleMeshGeometryLL& shapeMesh = shape1.get<const PxTriangleMeshGeometryLL>();

	//gRenderOutPut = cache.mRenderOutput;
	const FloatV capsuleRadius = FLoad(shapeCapsule.radius);
	const FloatV contactDist = FLoad(contactDistance);

	const PsTransformV capsuleTransform = loadTransformA(transform0);//capsule transform
	const PsTransformV meshTransform = loadTransformA(transform1);//triangleMesh  

	const PsTransformV curTransform = meshTransform.transformInv(capsuleTransform);
	const PsMatTransformV aToB(curTransform);

	// We must be in local space to use the cache
	const FloatV replaceBreakingThreshold = FMul(capsuleRadius, FLoad(0.001f));
	const FloatV projectBreakingThreshold = FMul(capsuleRadius, FLoad(0.05f));
	const PxU32 previousTotalContacts = multiManifold.mNumTotalContacts;
	const FloatV refereshDistance = FAdd(capsuleRadius, contactDist);
	//multiManifold.refreshManifold(aToB, projectBreakingThreshold, contactDist);
	multiManifold.refreshManifold(aToB, projectBreakingThreshold, refereshDistance);
	const bool bLostContacts = (multiManifold.mNumTotalContacts != previousTotalContacts);

	if(bLostContacts || multiManifold.invalidate(curTransform, capsuleRadius, FLoad(0.02f)))
	{
		//const FloatV capsuleHalfHeight = FloatV_From_F32(shapeCapsule.halfHeight);
		Cm::FastVertex2ShapeScaling meshScaling;
		const bool idtMeshScale = shapeMesh.scale.isIdentity();
		if(!idtMeshScale)
			meshScaling.init(shapeMesh.scale);

		// Capsule data
		Gu::Segment worldCapsule;
		const PxVec3 tmp = transform0.q.getBasisVector0() * shapeCapsule.halfHeight;
		worldCapsule.p0 = transform0.p + tmp;
		worldCapsule.p1 = transform0.p - tmp;

		
		const Gu::Segment meshCapsule(	// Capsule in mesh space
			transform1.transformInv(worldCapsule.p0),
			transform1.transformInv(worldCapsule.p1));

		const PxReal inflatedRadius = shapeCapsule.radius + contactDistance;

		const PxVec3 capsuleCenterInMesh = transform1.transformInv(transform0.p);
		const PxVec3 capsuleDirInMesh = transform1.rotateInv(tmp);
		const Gu::CapsuleV capsule(V3LoadU(capsuleCenterInMesh), V3LoadU(capsuleDirInMesh), capsuleRadius);

		// We must be in local space to use the cache
		const Gu::Capsule queryCapsule(meshCapsule, inflatedRadius);

	
		const Gu::InternalTriangleMeshData* meshData = shapeMesh.meshData;
#ifdef __SPU__
		// fetch meshData to temp storage
		PX_ALIGN_PREFIX(16) char meshDataBuf[sizeof(Gu::InternalTriangleMeshData)] PX_ALIGN_SUFFIX(16);
		Cm::memFetchAlignedAsync(Cm::MemFetchPtr(meshDataBuf), Cm::MemFetchPtr(shapeMesh.meshData), sizeof(Gu::InternalTriangleMeshData), 5);
		Cm::memFetchWait(5);
		meshData = reinterpret_cast<const Gu::InternalTriangleMeshData*>(meshDataBuf);
#endif

		multiManifold.mNumManifolds = 0;
		multiManifold.setRelativeTransform(curTransform); 

		const PxU8* PX_RESTRICT extraData = meshData->mExtraTrigData;
		// mesh scale is not baked into cached verts
		PCMCapsuleVsMeshContactGenerationCallback callback(
			capsule,
			contactDist,
			replaceBreakingThreshold,
			capsuleTransform,
			meshTransform,
			multiManifold,
			contactBuffer,
			extraData,
			meshScaling,
			idtMeshScale);

		//bound the capsule in shape space by an OBB:
		Gu::Box queryBox;
		queryBox.create(queryCapsule);

		//apply the skew transform to the box:
		if(!idtMeshScale)
			meshScaling.transformQueryBounds(queryBox.center, queryBox.extents, queryBox.rot);

		Gu::RTreeMidphaseData hmd;
		meshData->mCollisionModel.getRTreeMidphaseData(hmd);

		MPT_SET_CONTEXT("pccm", transform1, shapeMesh.scale);
		MeshRayCollider::collideOBB(queryBox, true, hmd, callback);

		callback.flushCache();

		callback.mGeneration.processContacts(GU_CAPSULE_MANIFOLD_CACHE_SIZE, false);
	}

	
	//multiManifold.drawManifold(*gRenderOutPut, capsuleTransform, meshTransform);
	return multiManifold.addManifoldContactsToContactBuffer(contactBuffer, capsuleTransform, meshTransform, capsuleRadius);
}

}
