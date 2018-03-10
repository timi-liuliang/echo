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


#include "GuGJKPenetration.h"
#include "GuEPA.h"
#include "GuVecCapsule.h"
#include "GuVecConvexHull.h"
#include "GuVecConvexHullNoScale.h"
#include "GuVecShrunkConvexHull.h"
#include "GuVecShrunkConvexHullNoScale.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"
#include "GuPCMContactGen.h"
#include "GuPCMShapeConvex.h"

#ifdef	PCM_LOW_LEVEL_DEBUG
#include "CmRenderOutput.h"
extern physx::Cm::RenderOutput* gRenderOutPut;
#endif


namespace physx
{
namespace Gu
{

static void addToContactBuffer(Gu::ContactBuffer& contactBuffer, const Ps::aos::Vec3VArg worldNormal, const Ps::aos::Vec3VArg worldPoint, const Ps::aos::FloatVArg penDep)
{
	using namespace Ps::aos;
	Gu::ContactPoint& contact = contactBuffer.contacts[contactBuffer.count++];
	V4StoreA(Vec4V_From_Vec3V(worldNormal), (PxF32*)&contact.normal.x);
	V4StoreA(Vec4V_From_Vec3V(worldPoint), (PxF32*)&contact.point.x);
	FStore(penDep, &contact.separation);

	contact.internalFaceIndex0 = PXC_CONTACT_NO_FACE_INDEX;
	contact.internalFaceIndex1 = PXC_CONTACT_NO_FACE_INDEX;

}

static bool fullContactsGenerationSphereConvex(const Gu::CapsuleV& capsule, const Gu::ConvexHullV& convexHull, const Ps::aos::PsTransformV& transf0,const Ps::aos::PsTransformV& transf1,
								Gu::PersistentContact* manifoldContacts, PxU32& numContacts, Gu::ContactBuffer& contactBuffer, const bool idtScale, Gu::PersistentContactManifold& manifold, Ps::aos::Vec3VArg normal, const Ps::aos::FloatVArg contactDist, bool doOverlapTest)
{
	using namespace Ps::aos;
	Gu::PolygonalData polyData;
	getPCMConvexData(convexHull,idtScale, polyData);

	PxU8 buff[sizeof(SupportLocalImpl<ConvexHullV>)];
	SupportLocal* map = (idtScale ? (SupportLocal*)PX_PLACEMENT_NEW(buff, SupportLocalImpl<ConvexHullNoScaleV>)((ConvexHullNoScaleV&)convexHull, transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale) : 
	(SupportLocal*)PX_PLACEMENT_NEW(buff, SupportLocalImpl<ConvexHullV>)(convexHull, transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale));

	if(generateSphereFullContactManifold(capsule, polyData, map, manifoldContacts, numContacts, contactDist, normal, doOverlapTest))
	{

		if(numContacts > 0)
		{
			
			Gu::PersistentContact& p = manifold.getContactPoint(0);

			p.mLocalPointA = manifoldContacts[0].mLocalPointA;
			p.mLocalPointB = manifoldContacts[0].mLocalPointB;
			p.mLocalNormalPen = manifoldContacts[0].mLocalNormalPen;
			manifold.mNumContacts =1;

			//transform normal to world space
			const Vec3V worldNormal = transf1.rotate(normal);
			const Vec3V worldP = V3NegScaleSub(worldNormal, capsule.radius, transf0.p);
			const FloatV penDep = FSub(V4GetW(manifoldContacts[0].mLocalNormalPen), capsule.radius);

	#ifdef	PCM_LOW_LEVEL_DEBUG
			manifold.drawManifold(*gRenderOutPut, transf0, transf1, capsule.radius);
	#endif

			addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);

			return true;
		}
		
	}

	return false;
}

bool pcmContactSphereConvex(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;

	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());
	
	
	const PxConvexMeshGeometryLL& shapeConvex = shape1.get<const PxConvexMeshGeometryLL>();
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();

	Gu::PersistentContactManifold& manifold = cache.getManifold();

	const Vec3V zeroV = V3Zero();

	Ps::prefetchLine(shapeConvex.hullData);
	const Vec3V vScale = V3LoadU(shapeConvex.scale.scale);
	const FloatV sphereRadius = FLoad(shapeSphere.radius);
	const FloatV contactDist = FLoad(contactDistance);
	const Gu::ConvexHullData* hullData = shapeConvex.hullData;
	
	//Transfer A into the local space of B
	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);
	const PsTransformV curRTrans(transf1.transformInv(transf0));
	const PsMatTransformV aToB(curRTrans);
	
	
	const FloatV convexMargin = Gu::CalculatePCMConvexMargin(hullData, vScale);

	const PxU32 initialContacts = manifold.mNumContacts;
	const FloatV minMargin = FMin(convexMargin, sphereRadius);
	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.05f));
	
	const FloatV refreshDistance = FAdd(sphereRadius, contactDist);
	manifold.refreshContactPoints(aToB, projectBreakingThreshold, refreshDistance);
	
	const PxU32 newContacts = manifold.mNumContacts;

	const bool bLostContacts = (newContacts != initialContacts);

	PxGJKStatus status = PxGJKStatus(manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT);

		
	Vec3V closestA(zeroV), closestB(zeroV);
	Vec3V normal(zeroV); // from a to b
	const FloatV zero = FZero();
	FloatV penDep = zero;

#ifdef	PCM_LOW_LEVEL_DEBUG
	gRenderOutPut = cache.mRenderOutput;
#endif

	PX_UNUSED(bLostContacts);


	if(bLostContacts || manifold.invalidate_SphereCapsule(curRTrans, minMargin))
	{

		manifold.setRelativeTransform(curRTrans);
		
		const QuatV vQuat = QuatVLoadU(&shapeConvex.scale.rotation.x);  
	
		//use the original shape
		Gu::ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);
		convexHull.setMargin(zero);
		//transform capsule into the local space of convexHull
		const bool takeCoreShape = true;
		Gu::CapsuleV capsule(aToB.p, sphereRadius);

		const bool idtScale = shapeConvex.scale.isIdentity();
		if(idtScale)
		{
			  
			status = Gu::gjkLocalPenetration(capsule, *PX_CONVEX_TO_NOSCALECONVEX(&convexHull), contactDist, closestA, closestB, normal, penDep, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, takeCoreShape);
		}
		else
		{
			status = Gu::gjkLocalPenetration(capsule, convexHull, contactDist, closestA, closestB, normal, penDep, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, takeCoreShape);

		}

		if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else if(status == GJK_CONTACT)
		{
			Gu::PersistentContact& p = manifold.getContactPoint(0);
			p.mLocalPointA = zeroV;//sphere center
			p.mLocalPointB = closestB;
			p.mLocalNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
			manifold.mNumContacts =1;

#ifdef	PCM_LOW_LEVEL_DEBUG
			manifold.drawManifold(*gRenderOutPut, transf0, transf1, capsule.radius);
#endif
			
			//transform normal to world space
			const Vec3V worldNormal = transf1.rotate(normal);
			const Vec3V worldP = V3NegScaleSub(worldNormal, sphereRadius, transf0.p);
			penDep = FSub(penDep, sphereRadius);
			addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);
			return true;

		}
		else if(status == GJK_DEGENERATE)
		{
			Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
			PxU32 numContacts = 0;
			return fullContactsGenerationSphereConvex(capsule, convexHull, transf0, transf1, manifoldContacts, numContacts, contactBuffer, idtScale, manifold, normal, contactDist, true);
		}
		else if(status == EPA_CONTACT)
		{
			if(idtScale)
			{
				Gu::ConvexHullNoScaleV& noScaleConvexHull = *PX_CONVEX_TO_NOSCALECONVEX(&convexHull);
				EPASupportMapPairLocalImpl<Gu::CapsuleV, Gu::ConvexHullNoScaleV> supportMap(capsule, noScaleConvexHull);
				status= Gu::epaPenetration(capsule, noScaleConvexHull, &supportMap, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,
						closestA, closestB, normal, penDep, takeCoreShape);
				
			}
			else
			{
				EPASupportMapPairLocalImpl<Gu::CapsuleV, Gu::ConvexHullV> supportMap(capsule, convexHull);
				status= Gu::epaPenetration(capsule, convexHull, &supportMap, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,
						closestA, closestB, normal, penDep, takeCoreShape);

			}

			if(status == EPA_CONTACT)
			{
				Gu::PersistentContact& p = manifold.getContactPoint(0);
				p.mLocalPointA = zeroV;//sphere center
				p.mLocalPointB = closestB;
				p.mLocalNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
				manifold.mNumContacts =1;

#ifdef	PCM_LOW_LEVEL_DEBUG
				manifold.drawManifold(*gRenderOutPut, transf0, transf1, capsule.radius);
#endif
				
				//transform normal to world space
				const Vec3V worldNormal = transf1.rotate(normal);
				const Vec3V worldP = V3NegScaleSub(worldNormal, sphereRadius, transf0.p);
				penDep = FSub(penDep, sphereRadius);

				addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);
				return true;
			}
			else
			{

				Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
				PxU32 numContacts = 0;
				return fullContactsGenerationSphereConvex(capsule, convexHull, transf0, transf1, manifoldContacts, numContacts, contactBuffer, idtScale, manifold, normal, contactDist, true);

			}
			
		}
	}
	else if(manifold.mNumContacts > 0)
	{
		//ML:: the manifold originally has contacts
		Gu::PersistentContact& p = manifold.getContactPoint(0);
		const Vec3V worldNormal = transf1.rotate(Vec3V_From_Vec4V(p.mLocalNormalPen));
		const Vec3V worldP = V3NegScaleSub(worldNormal, sphereRadius, transf0.p);
		penDep = FSub(V4GetW(p.mLocalNormalPen), sphereRadius);

#ifdef	PCM_LOW_LEVEL_DEBUG
		manifold.drawManifold(*gRenderOutPut, transf0, transf1, sphereRadius);
#endif
	
		addToContactBuffer(contactBuffer, worldNormal, worldP, penDep);
		return true;
	}

	return false;

}  
}//Gu
}//phyxs

