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

#include "GuVecBox.h"
#include "GuVecShrunkBox.h"
#include "GuVecConvexHull.h"
#include "GuVecConvexHullNoScale.h"
#include "GuVecTriangle.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuPCMShapeConvex.h"
#include "GuTriangleMeshData.h"
#include "PxTriangleMesh.h"
#include "GuContactBuffer.h"
#include "GuHeightField.h"
#include "GuHeightFieldUtil.h"
#include "GuPCMContactConvexCommon.h"
#include "GuPCMContactMeshCallback.h"

#include "PsVecMath.h"


using namespace physx;
using namespace Gu;
using namespace physx::shdfnd::aos;

#if defined(__SPU__)
#include "CmMemFetch.h"
extern unsigned char HeightFieldBuffer[sizeof(Gu::HeightField)+16];
#endif

#ifdef PX_WIIU  
#pragma ghs nowarning 1656 //within a function using alloca or VLAs, alignment of local variables
#endif

namespace physx
{

struct PCMConvexVsHeightfieldContactGenerationCallback
	: PCMHeightfieldContactGenerationCallback< PCMConvexVsHeightfieldContactGenerationCallback >
{
	PCMConvexVsHeightfieldContactGenerationCallback& operator=(const PCMConvexVsHeightfieldContactGenerationCallback&);
public:
	PCMConvexVsMeshContactGeneration		mGeneration;

	PCMConvexVsHeightfieldContactGenerationCallback(
		const Ps::aos::FloatVArg				contactDistance,
		const Ps::aos::FloatVArg				replaceBreakingThreshold,
		const Gu::PolygonalData&				polyData,
		SupportLocal*							polyMap,
		const Cm::FastVertex2ShapeScaling&		convexScaling,
		bool									idtConvexScale,
		const PsTransformV&						convexTransform, 
		const PsTransformV&						heightfieldTransform,
		const PxTransform&						heightfieldTransform1,
		Gu::MultiplePersistentContactManifold&	multiManifold,
		Gu::ContactBuffer&						contactBuffer,
		Gu::HeightFieldUtil&					hfUtil,
		Gu::Container&							delayedContacts
		
	) :
		PCMHeightfieldContactGenerationCallback< PCMConvexVsHeightfieldContactGenerationCallback >(hfUtil, heightfieldTransform1),
		mGeneration(contactDistance, replaceBreakingThreshold, convexTransform, heightfieldTransform,  multiManifold,
			contactBuffer, polyData, polyMap, delayedContacts, convexScaling, idtConvexScale)
	{
	}

	template<PxU32 CacheSize>
	void processTriangleCache(Gu::TriangleCache<CacheSize>& cache)
	{
		mGeneration.processTriangleCache<CacheSize, PCMConvexVsMeshContactGeneration>(cache);
	}
	
};

bool Gu::PCMContactConvexHeightfield(
	const Gu::PolygonalData& polyData, Gu::SupportLocal* polyMap, const Ps::aos::FloatVArg minMargin,
	const PxBounds3& hullAABB, const PxHeightFieldGeometry& shapeHeightfield,
	const PxTransform& transform0, const PxTransform& transform1,
	PxReal contactDistance, Gu::ContactBuffer& contactBuffer,
	const Cm::FastVertex2ShapeScaling& convexScaling, bool idtConvexScale,Gu::MultiplePersistentContactManifold& multiManifold)

{

	using namespace Ps::aos;
	using namespace Gu;

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&transform0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&transform1.p.x);

	const FloatV contactDist = FLoad(contactDistance);
	//Transfer A into the local space of B
	const PsTransformV convexTransform(p0, q0);//box
	const PsTransformV heightfieldTransform(p1, q1);//heightfield  
	const PsTransformV curTransform = heightfieldTransform.transformInv(convexTransform);
	const PsMatTransformV aToB(curTransform);

	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.6f));
	const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.05f));

	const PxU32 previousTotalContacts = multiManifold.mNumTotalContacts;
	multiManifold.refreshManifold(aToB, projectBreakingThreshold, contactDist);

	const bool bLostContacts = (multiManifold.mNumTotalContacts != previousTotalContacts);

	if(bLostContacts || multiManifold.invalidate(curTransform, minMargin))
	{
		multiManifold.mNumManifolds = 0;
		multiManifold.setRelativeTransform(curTransform); 

	
	////////////////////

		const PxTransform t0to1 = transform1.transformInv(transform0);
		

#ifdef __SPU__
		const Gu::HeightField& hf = *Cm::memFetchAsync<const Gu::HeightField>(HeightFieldBuffer, Cm::MemFetchPtr(static_cast<Gu::HeightField*>(shapeHeightfield.heightField)), sizeof(Gu::HeightField), 1);
		Cm::memFetchWait(1);
#if HF_TILED_MEMORY_LAYOUT
		g_sampleCache.init((uintptr_t)(hf.getData().samples), hf.getData().tilesU);
#endif
#else
	const Gu::HeightField& hf = *static_cast<Gu::HeightField*>(shapeHeightfield.heightField);
#endif
	Gu::HeightFieldUtil hfUtil(shapeHeightfield, hf);

	//Gu::HeightFieldUtil hfUtil(shapeHeightfield);

		////////////////////

		/*const Cm::Matrix34 world0(transform0);
		const Cm::Matrix34 world1(transform1);

		const PxU8* PX_RESTRICT extraData = meshData->mExtraTrigData;*/

		LocalContainer(delayedContacts, PCM_LOCAL_CONTACTS_SIZE);
		
		PCMConvexVsHeightfieldContactGenerationCallback blockCallback(
			contactDist,
			replaceBreakingThreshold,
			polyData,
			polyMap, 
			convexScaling, 
			idtConvexScale,
			convexTransform, 
			heightfieldTransform,
			transform1,
			multiManifold,
			contactBuffer,
			hfUtil,
			delayedContacts
		);

		MPT_SET_CONTEXT("coxh", transform1, PxMeshScale());
		hfUtil.overlapAABBTriangles(transform1, PxBounds3::transformFast(t0to1, hullAABB), 0, &blockCallback);

		PX_ASSERT(multiManifold.mNumManifolds <= GU_MAX_MANIFOLD_SIZE);
		blockCallback.mGeneration.generateLastContacts();
		blockCallback.mGeneration.processContacts(GU_SINGLE_MANIFOLD_CACHE_SIZE, false);
	}

	//multiManifold.drawManifold(*gRenderOutPut, convexTransform, meshTransform);
	return multiManifold.addManifoldContactsToContactBuffer(contactBuffer, heightfieldTransform);

}


bool Gu::pcmContactConvexHeightField(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;

	const PxConvexMeshGeometryLL& shapeConvex = shape0.get<const PxConvexMeshGeometryLL>();
	const physx::PxHeightFieldGeometryLL& shapHeightField = shape1.get<const PxHeightFieldGeometryLL>();

	const Gu::ConvexHullData* hullData = shapeConvex.hullData;
	Gu::MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();

	//gRenderOutPut = cache.mRenderOutput;

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&transform0.p.x);

	const PsTransformV convexTransform(p0, q0);

	//const bool idtScaleMesh = shapeMesh.scale.isIdentity();

	//Cm::FastVertex2ShapeScaling meshScaling;
	//if(!idtScaleMesh)
	//	meshScaling.init(shapeMesh.scale);

	Cm::FastVertex2ShapeScaling convexScaling;
	PxBounds3 hullAABB;
	PolygonalData polyData;
	const bool idtScaleConvex = getPCMConvexData(shape0, convexScaling, hullAABB, polyData);

	const Vec3V vScale = V3LoadU(shapeConvex.scale.scale);
	const FloatV convexMargin = Gu::CalculatePCMConvexMargin(hullData, vScale);
	const QuatV vQuat = QuatVLoadU(&shapeConvex.scale.rotation.x);
	Gu::ConvexHullV convexHull(hullData, V3Zero(), vScale, vQuat);

	if(idtScaleConvex)
	{
		SupportLocalShrunkImpl<Gu::ConvexHullNoScaleV, Gu::ShrunkConvexHullNoScaleV> convexMap((ConvexHullNoScaleV&)convexHull, convexTransform, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScaleConvex);
		return Gu::PCMContactConvexHeightfield(polyData, &convexMap, convexMargin, hullAABB, shapHeightField,transform0,transform1, contactDistance, contactBuffer, convexScaling, idtScaleConvex, multiManifold);
	}
	else
	{
		SupportLocalShrunkImpl<Gu::ConvexHullV, Gu::ShrunkConvexHullV> convexMap(convexHull, convexTransform, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScaleConvex);
		return Gu::PCMContactConvexHeightfield(polyData, &convexMap, convexMargin, hullAABB, shapHeightField,transform0,transform1, contactDistance, contactBuffer, convexScaling, idtScaleConvex, multiManifold);
	}
}  

bool Gu::pcmContactBoxHeightField(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;

	MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();

	const PxBoxGeometry& shapeBox = shape0.get<const PxBoxGeometry>();
	const physx::PxHeightFieldGeometryLL& shapHeightField = shape1.get<const PxHeightFieldGeometryLL>();;

	//gRenderOutPut = cache.mRenderOutput;


	PxVec3 ext = shapeBox.halfExtents + PxVec3(contactDistance);
	const PxBounds3 hullAABB(-ext, ext);

	Cm::FastVertex2ShapeScaling idtScaling;

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&transform0.p.x);

	const Vec3V boxExtents = V3LoadU(shapeBox.halfExtents);
	const FloatV minMargin = Gu::CalculatePCMBoxMargin(boxExtents);

	Gu::BoxV boxV(V3Zero(), boxExtents);

	const PsTransformV boxTransform(p0, q0);//box

	Gu::PolygonalData polyData;
	Gu::PCMPolygonalBox polyBox(shapeBox.halfExtents);
	polyBox.getPolygonalData(&polyData);

	Mat33V identity =  M33Identity();
	//SupportLocalImpl<Gu::BoxV> boxMap(boxV, boxTransform, identity, identity);
	SupportLocalShrunkImpl<Gu::BoxV, Gu::ShrunkBoxV> boxMap(boxV, boxTransform, identity, identity, true);

	return Gu::PCMContactConvexHeightfield(polyData, &boxMap, minMargin, hullAABB, shapHeightField,transform0,transform1, contactDistance, contactBuffer, idtScaling, true, multiManifold);
}
}
