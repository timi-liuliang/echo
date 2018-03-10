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
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuTriangleMeshData.h"
#include "GuConvexUtilsInternal.h"
#include "PxTriangleMesh.h"
#include "GuContactBuffer.h"
#include "GuMidphase.h"
#include "GuTriangleCache.h"
#include "GuPCMContactConvexCommon.h"
#include "GuHeightFieldUtil.h"
#include "GuPCMContactMeshCallback.h"

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

struct PCMSphereVsMeshContactGenerationCallback : PCMMeshContactGenerationCallback< PCMSphereVsMeshContactGenerationCallback >
{

public:
	PCMSphereVsMeshContactGeneration		mGeneration;
	

	PCMSphereVsMeshContactGenerationCallback(
		const Ps::aos::Vec3VArg		sphereCenter,
		const Ps::aos::FloatVArg	sphereRadius,
		const Ps::aos::FloatVArg	contactDist,
		const Ps::aos::FloatVArg	replaceBreakingThreshold,
		const PsTransformV& sphereTransform,
		const PsTransformV& meshTransform,
		Gu::MultiplePersistentContactManifold& multiManifold,
		Gu::ContactBuffer& contactBuffer,
		const PxU8*		extraTriData,
		const Cm::FastVertex2ShapeScaling& meshScaling,
		bool idtMeshScale
	) :
		PCMMeshContactGenerationCallback<PCMSphereVsMeshContactGenerationCallback>(meshScaling, extraTriData, idtMeshScale), 
		mGeneration(sphereCenter, sphereRadius, contactDist, replaceBreakingThreshold, sphereTransform, meshTransform, multiManifold, contactBuffer)
	{
	}

	PX_FORCE_INLINE bool doTest(const PxVec3&, const PxVec3&, const PxVec3&) { return true; }

	template<PxU32 CacheSize>
	void processTriangleCache(Gu::TriangleCache<CacheSize>& cache)
	{
		mGeneration.processTriangleCache<CacheSize, PCMSphereVsMeshContactGeneration>(cache);
	}
	
};


bool Gu::pcmContactSphereMesh(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;
	Gu::MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	const PxTriangleMeshGeometryLL& shapeMesh = shape1.get<const PxTriangleMeshGeometryLL>();

	//gRenderOutPut = cache.mRenderOutput;

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&transform0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&transform1.p.x);

	const FloatV sphereRadius = FLoad(shapeSphere.radius);
	const FloatV contactDist = FLoad(contactDistance);
	
	const PsTransformV sphereTransform(p0, q0);//sphere transform
	const PsTransformV meshTransform(p1, q1);//triangleMesh  
	const PsTransformV curTransform = meshTransform.transformInv(sphereTransform);
	const PsMatTransformV aToB(curTransform);

	// We must be in local space to use the cache
	const FloatV replaceBreakingThreshold = FMul(sphereRadius, FLoad(0.001f));
	const FloatV projectBreakingThreshold = FMul(sphereRadius, FLoad(0.05f));
	const PxU32 previousTotalContacts = multiManifold.mNumTotalContacts;
	const FloatV refereshDistance = FAdd(sphereRadius, contactDist);
	multiManifold.refreshManifold(aToB, projectBreakingThreshold, refereshDistance);
	const bool bLostContacts = (multiManifold.mNumTotalContacts != previousTotalContacts);

	if(bLostContacts || multiManifold.invalidate(curTransform, sphereRadius, FLoad(0.02f)))
	{
		const PxVec3 sphereCenterShape1Space = transform1.transformInv(transform0.p);
		PxReal inflatedRadius = shapeSphere.radius + contactDistance;

		const Vec3V sphereCenter = V3LoadU(sphereCenterShape1Space);

#ifdef __SPU__
	// PT: TODO: cache this one
	// fetch meshData to temp buffer
	PX_ALIGN_PREFIX(16) char meshDataBuf[sizeof(InternalTriangleMeshData)] PX_ALIGN_SUFFIX(16);
	Cm::memFetchAlignedAsync(PxU64(meshDataBuf), PxU64(shapeMesh.meshData), sizeof(InternalTriangleMeshData), 5);
#endif

	Cm::FastVertex2ShapeScaling meshScaling;	// PT: TODO: get rid of default ctor :(
	const bool idtMeshScale = shapeMesh.scale.isIdentity();
	if(!idtMeshScale)
		meshScaling.init(shapeMesh.scale);

#ifdef __SPU__
		Cm::memFetchWait(5);
		const InternalTriangleMeshData* meshData = reinterpret_cast<const InternalTriangleMeshData*>(meshDataBuf);
#else
		const InternalTriangleMeshData* meshData = shapeMesh.meshData;
#endif
		multiManifold.mNumManifolds = 0;
		multiManifold.setRelativeTransform(curTransform); 

		const PxU8* PX_RESTRICT extraData = meshData->mExtraTrigData;
		// mesh scale is not baked into cached verts
		PCMSphereVsMeshContactGenerationCallback callback(
			sphereCenter,
			sphereRadius,
			contactDist,
			replaceBreakingThreshold,
			sphereTransform,
			meshTransform,
			multiManifold,
			contactBuffer,
			extraData,
			meshScaling,
			idtMeshScale);

		Gu::RTreeMidphaseData hmd;
		meshData->mCollisionModel.getRTreeMidphaseData(hmd);
		
		PxVec3 obbCenter = sphereCenterShape1Space;
		PxVec3 obbExtents = PxVec3(inflatedRadius);
		PxMat33 obbRot(PxIdentity);
		if(!idtMeshScale)
			meshScaling.transformQueryBounds(obbCenter, obbExtents, obbRot);
		const Gu::Box obb(obbCenter, obbExtents, obbRot);

		MPT_SET_CONTEXT("pcsm", transform1, shapeMesh.scale);
		Gu::MeshRayCollider::collideOBB(obb, true, hmd, callback);

		callback.flushCache();

		callback.mGeneration.processContacts(GU_SPHERE_MANIFOLD_CACHE_SIZE, false);
	}
	
	//multiManifold.drawManifold(*gRenderOutPut, sphereTransform, meshTransform);
	return multiManifold.addManifoldContactsToContactBuffer(contactBuffer, sphereTransform, meshTransform, sphereRadius);
}

}
