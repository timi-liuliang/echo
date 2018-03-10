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

#include "PsVecMath.h"
#include "PsVecTransform.h"
#include "GuVecTriangle.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuTriangleMeshData.h"
#include "PxTriangleMesh.h"
#include "GuContactBuffer.h"
#include "GuHeightField.h"
#include "GuPCMContactConvexCommon.h"
#include "GuSegment.h"
#include "GuGeomUtilsInternal.h"
#include "GuPCMContactMeshCallback.h"

using namespace physx;
using namespace Gu;
using namespace Ps::aos;

#if defined(__SPU__)
#include "CmMemFetch.h"
extern unsigned char HeightFieldBuffer[sizeof(physx::Gu::HeightField)+16];
#endif




namespace physx
{

struct PCMCapsuleVsHeightfieldContactGenerationCallback :  PCMHeightfieldContactGenerationCallback<PCMCapsuleVsHeightfieldContactGenerationCallback>
{
	PCMCapsuleVsHeightfieldContactGenerationCallback& operator=(const PCMCapsuleVsHeightfieldContactGenerationCallback&);

public:
	PCMCapsuleVsMeshContactGeneration		mGeneration;

	PCMCapsuleVsHeightfieldContactGenerationCallback(
		const Gu::CapsuleV&			capsule,
		const Ps::aos::FloatVArg	contactDistance,
		const Ps::aos::FloatVArg	replaceBreakingThreshold,
	
		const PsTransformV& capsuleTransform, 
		const PsTransformV& heightfieldTransform,
		const PxTransform&	heightfieldTransform1,
		Gu::MultiplePersistentContactManifold& multiManifold,
		Gu::ContactBuffer& contactBuffer,
		Gu::HeightFieldUtil& hfUtil 
		
		
	) :
		PCMHeightfieldContactGenerationCallback<PCMCapsuleVsHeightfieldContactGenerationCallback>(hfUtil, heightfieldTransform1),
		mGeneration(capsule, contactDistance, replaceBreakingThreshold, capsuleTransform, heightfieldTransform, multiManifold, contactBuffer)
	{
	}

	template<PxU32 CacheSize>
	void processTriangleCache(Gu::TriangleCache<CacheSize>& cache)
	{
		mGeneration.processTriangleCache<CacheSize, PCMCapsuleVsMeshContactGeneration>(cache);
	}
	
};

bool Gu::pcmContactCapsuleHeightField(GU_CONTACT_METHOD_ARGS)
{
	const PxCapsuleGeometry& shapeCapsule = shape0.get<const PxCapsuleGeometry>();
	const PxHeightFieldGeometryLL& shapeHeight = shape1.get<const PxHeightFieldGeometryLL>();

	Gu::MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();

	const FloatV capsuleRadius = FLoad(shapeCapsule.radius);
	const FloatV contactDist = FLoad(contactDistance);

	const PsTransformV capsuleTransform = loadTransformA(transform0);//capsule transform
	const PsTransformV heightfieldTransform = loadTransformA(transform1);//height feild

	const PsTransformV curTransform = heightfieldTransform.transformInv(capsuleTransform);
	const PsMatTransformV aToB(curTransform);

	// We must be in local space to use the cache
	const FloatV replaceBreakingThreshold = FMul(capsuleRadius, FLoad(0.001f));
	const FloatV projectBreakingThreshold = FMul(capsuleRadius, FLoad(0.05f));
	const PxU32 previousTotalContacts = multiManifold.mNumTotalContacts;
	const FloatV refereshDistance = FAdd(capsuleRadius, contactDist);
	multiManifold.refreshManifold(aToB, projectBreakingThreshold, refereshDistance);
	const bool bLostContacts = (multiManifold.mNumTotalContacts != previousTotalContacts);

	if(bLostContacts || multiManifold.invalidate(curTransform, capsuleRadius, FLoad(0.02f)))
	{

		multiManifold.mNumManifolds = 0;
		multiManifold.setRelativeTransform(curTransform); 
#ifdef __SPU__
		const Gu::HeightField& hf = *Cm::memFetchAsync<const Gu::HeightField>(HeightFieldBuffer, Cm::MemFetchPtr(static_cast<Gu::HeightField*>(shapeHeight.heightField)), sizeof(Gu::HeightField), 1);
		Cm::memFetchWait(1);
#if HF_TILED_MEMORY_LAYOUT
		g_sampleCache.init((uintptr_t)(hf.getData().samples), hf.getData().tilesU);
#endif
#else
		const Gu::HeightField& hf = *static_cast<Gu::HeightField*>(shapeHeight.heightField);
#endif

		Gu::HeightFieldUtil hfUtil(shapeHeight, hf);

		const PxVec3 tmp = transform0.q.getBasisVector0() * shapeCapsule.halfHeight;
		
		const PxReal inflatedRadius = shapeCapsule.radius + contactDistance;

		const PxVec3 capsuleCenterInMesh = transform1.transformInv(transform0.p);
		const PxVec3 capsuleDirInMesh = transform1.rotateInv(tmp);
		const Gu::CapsuleV capsule(V3LoadU(capsuleCenterInMesh), V3LoadU(capsuleDirInMesh), capsuleRadius);


		PCMCapsuleVsHeightfieldContactGenerationCallback callback(
			capsule,
			contactDist,
			replaceBreakingThreshold,
			capsuleTransform,
			heightfieldTransform,
			transform1,
			multiManifold,
			contactBuffer,
			hfUtil
		);

		PxBounds3 bounds;
		bounds.maximum = PxVec3(shapeCapsule.halfHeight + inflatedRadius, inflatedRadius, inflatedRadius);
		bounds.minimum = -bounds.maximum;

		bounds = PxBounds3::transformFast(transform1.transformInv(transform0), bounds);

		MPT_SET_CONTEXT("coch", transform1, PxMeshScale());
		hfUtil.overlapAABBTriangles(transform1, bounds, 0, &callback);

		callback.mGeneration.processContacts(GU_CAPSULE_MANIFOLD_CACHE_SIZE, false);
	}
	return multiManifold.addManifoldContactsToContactBuffer(contactBuffer, capsuleTransform, heightfieldTransform, capsuleRadius);
}


}
