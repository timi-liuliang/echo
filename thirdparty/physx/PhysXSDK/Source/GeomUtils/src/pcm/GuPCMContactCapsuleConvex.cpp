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
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuContactBuffer.h"
#include "GuPCMContactGen.h"
#include "GuPCMShapeConvex.h"

#ifdef	PCM_LOW_LEVEL_DEBUG
#include "CmRenderOutput.h"
extern physx::Cm::RenderOutput* gRenderOutPut;
#endif

using namespace physx;
using namespace Gu;

namespace physx
{
namespace Gu
{

static bool fullContactsGenerationCapsuleConvex(const Gu::CapsuleV& capsule, const Gu::ConvexHullV& convexHull,  const Ps::aos::PsMatTransformV& aToB, const Ps::aos::PsTransformV& transf0,const Ps::aos::PsTransformV& transf1,
								Gu::PersistentContact* manifoldContacts, PxU32& numContacts, Gu::ContactBuffer& contactBuffer, const bool idtScale, Gu::PersistentContactManifold& manifold, Ps::aos::Vec3VArg normal, const Ps::aos::FloatVArg contactDist, const bool doOverlapTest)
{

	using namespace Ps::aos;
	Gu::PolygonalData polyData;
	getPCMConvexData(convexHull,idtScale, polyData);

	PxU8 buff[sizeof(SupportLocalImpl<ConvexHullV>)];
	SupportLocal* map = (idtScale ? (SupportLocal*)PX_PLACEMENT_NEW(buff, SupportLocalImpl<ConvexHullNoScaleV>)((ConvexHullNoScaleV&)convexHull, transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale) : 
	(SupportLocal*)PX_PLACEMENT_NEW(buff, SupportLocalImpl<ConvexHullV>)(convexHull, transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale));

	PxU32 origContacts = numContacts;
	if(generateFullContactManifold(capsule, polyData, map, aToB, manifoldContacts, numContacts, contactDist, normal, doOverlapTest))
	{
		//if we already have a gjk contacts, but if the full manifold generate contacts, we need to drop the gjk contact, because the normal
		//will be different
		if(numContacts != origContacts && origContacts != 0)
		{
			numContacts--;
			manifoldContacts++;
		}

		manifold.addBatchManifoldContacts2(manifoldContacts, numContacts);
		//transform normal into the world space
		normal = transf1.rotate(normal);
		manifold.addManifoldContactsToContactBuffer(contactBuffer, normal, transf0, capsule.radius);

		return true;
		
	}
	return false;

}

bool pcmContactCapsuleConvex(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;

	const PxConvexMeshGeometryLL& shapeConvex = shape1.get<const PxConvexMeshGeometryLL>();
	const PxCapsuleGeometry& shapeCapsule = shape0.get<const PxCapsuleGeometry>();

	Gu::PersistentContactManifold& manifold = cache.getManifold();

	Ps::prefetchLine(shapeConvex.hullData);

		
	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());

	const Vec3V zeroV = V3Zero();

	const Vec3V vScale = V3LoadU(shapeConvex.scale.scale);

	const FloatV contactDist = FLoad(contactDistance);
	const FloatV capsuleHalfHeight = FLoad(shapeCapsule.halfHeight);
	const FloatV capsuleRadius = FLoad(shapeCapsule.radius);
	const Gu::ConvexHullData* hullData =shapeConvex.hullData;
	
	//Transfer A into the local space of B
	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);
	const PsTransformV curRTrans(transf1.transformInv(transf0));
	const PsMatTransformV aToB(curRTrans);
	

	const FloatV convexMargin = Gu::CalculatePCMConvexMargin(hullData, vScale);
	const FloatV capsuleMinMargin = Gu::CalculateCapsuleMinMargin(capsuleRadius);
	const FloatV minMargin = FMin(convexMargin, capsuleMinMargin);
	
	const PxU32 initialContacts = manifold.mNumContacts;
	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(1.25f));
	const FloatV refreshDist = FAdd(contactDist, capsuleRadius);

	manifold.refreshContactPoints(aToB,  projectBreakingThreshold, refreshDist);

	const PxU32 newContacts = manifold.mNumContacts;
	const bool bLostContacts = (newContacts != initialContacts);

	PxGJKStatus status = PxGJKStatus(manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT);

	Vec3V closestA(zeroV), closestB(zeroV), normal(zeroV); // from a to b
	const FloatV zero = FZero();
	FloatV penDep = zero;

#ifdef	PCM_LOW_LEVEL_DEBUG
	gRenderOutPut = cache.mRenderOutput;
#endif
	
	PX_UNUSED(bLostContacts);
	if(bLostContacts || manifold.invalidate_SphereCapsule(curRTrans, minMargin))
	{
		bool takeCoreShape = true;
		manifold.setRelativeTransform(curRTrans);
		const QuatV vQuat = QuatVLoadU(&shapeConvex.scale.rotation.x);  
		Gu::ConvexHullV convexHull(hullData, zeroV, vScale, vQuat);
		convexHull.setMargin(zero);
	
		//transform capsule(a) into the local space of convexHull(b)
		Gu::CapsuleV capsule(aToB.p, aToB.rotate(V3Scale(V3UnitX(), capsuleHalfHeight)), capsuleRadius);
	

		const bool idtScale = shapeConvex.scale.isIdentity();
		if(idtScale)
		{
			
			status = Gu::gjkLocalPenetration(capsule, *PX_CONVEX_TO_NOSCALECONVEX(&convexHull), contactDist, closestA, closestB, normal, penDep, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, takeCoreShape);
		}
		else
		{
			status = Gu::gjkLocalPenetration(capsule, convexHull, contactDist, closestA, closestB, normal, penDep, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, takeCoreShape);

		}     

		Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
		PxU32 numContacts = 0;
		bool doOverlapTest = false;
		if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else if(status == GJK_DEGENERATE)
		{
			return fullContactsGenerationCapsuleConvex(capsule, convexHull, aToB, transf0, transf1, manifoldContacts, numContacts, contactBuffer, idtScale, 
				manifold, normal, contactDist, true);
		}
		else 
		{
			if(status == GJK_CONTACT)
			{
				const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
				const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
				//Add contact to contact stream
				manifoldContacts[numContacts].mLocalPointA = localPointA;
				manifoldContacts[numContacts].mLocalPointB = closestB;
				manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;
			}
			else
			{
				PX_ASSERT(status == EPA_CONTACT);
				EPASupportMapPairLocalImpl<Gu::CapsuleV, Gu::ConvexHullV> supportMap(capsule, (Gu::ConvexHullV&)convexHull);
				status= Gu::epaPenetration(capsule, (Gu::ConvexHullV&)convexHull, &supportMap, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints,
					closestA, closestB, normal, penDep, takeCoreShape);
				
				if(status == EPA_CONTACT)
				{
					const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
					//Add contact to contact stream
					manifoldContacts[numContacts].mLocalPointA = localPointA;
					manifoldContacts[numContacts].mLocalPointB = closestB;
					manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;

				}
				else
				{
					doOverlapTest = true;   
				}
			}

			
		
			if(initialContacts == 0 || bLostContacts || doOverlapTest)
			{
				return fullContactsGenerationCapsuleConvex(capsule, convexHull, aToB, transf0, transf1, manifoldContacts, numContacts, contactBuffer, idtScale, manifold, normal, contactDist, doOverlapTest);
			}
			else
			{
				//This contact is either come from GJK or EPA
				const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.05f));
				const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
				manifold.addManifoldPoint2(aToB.transformInv(closestA), closestB, localNormalPen, replaceBreakingThreshold);  
				normal = transf1.rotate(normal);
				manifold.addManifoldContactsToContactBuffer(contactBuffer, normal, transf0, capsuleRadius);

				return true;
			}
		}	
	}
	else if (manifold.getNumContacts() > 0)
	{
		normal = manifold.getWorldNormal(transf1);
		manifold.addManifoldContactsToContactBuffer(contactBuffer, normal, transf0, capsuleRadius);
		return true;
	}
	return false;
}

}//Gu
}//physx
