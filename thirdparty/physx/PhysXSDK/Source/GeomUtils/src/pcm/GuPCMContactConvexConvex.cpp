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

#include "GuGJKPenetrationWrapper.h"
#include "GuEPAPenetrationWrapper.h"
#include "GuGJKPenetration.h"
#include "GuEPA.h"
#include "GuVecConvexHull.h"
#include "GuVecShrunkConvexHull.h"
#include "GuVecShrunkConvexHullNoScale.h"
#include "GuVecConvexHullNoScale.h"
#include "GuGeometryUnion.h"   

#include "GuContactMethodImpl.h"
#include "GuPCMShapeConvex.h"
#include "GuPCMContactGen.h"
#include "GuContactBuffer.h"

using namespace physx;
using namespace Gu;

#ifdef	PCM_LOW_LEVEL_DEBUG
#include "CmRenderOutput.h"
extern physx::Cm::RenderOutput* gRenderOutPut;
#endif

namespace physx
{
namespace Gu
{


static bool fullContactsGenerationConvexConvex(const Gu::ConvexHullV& convexHull0, Gu::ConvexHullV& convexHull1, const Ps::aos::PsTransformV& transf0, const Ps::aos::PsTransformV& transf1, 
											   const bool idtScale0, const bool idtScale1, Gu::PersistentContact* manifoldContacts, PxU32& numContacts, Gu::ContactBuffer& contactBuffer, Gu::PersistentContactManifold& manifold, Ps::aos::Vec3VArg normal, 
											   const Ps::aos::FloatVArg contactDist,  const bool doOverlapTest)
{
	using namespace Ps::aos;
	Gu::PolygonalData polyData0, polyData1;
	getPCMConvexData(convexHull0, idtScale0, polyData0);
	getPCMConvexData(convexHull1, idtScale1, polyData1);

	PxU8 buff0[sizeof(SupportLocalImpl<ConvexHullV>)];
	PxU8 buff1[sizeof(SupportLocalImpl<ConvexHullV>)];

	SupportLocal* map0 = (idtScale0 ? (SupportLocal*)PX_PLACEMENT_NEW(buff0, SupportLocalImpl<ConvexHullNoScaleV>)((ConvexHullNoScaleV&)convexHull0, transf0, convexHull0.vertex2Shape, convexHull0.shape2Vertex, idtScale0) : 
		(SupportLocal*)PX_PLACEMENT_NEW(buff0, SupportLocalImpl<ConvexHullV>)(convexHull0, transf0, convexHull0.vertex2Shape, convexHull0.shape2Vertex, idtScale0));

	SupportLocal* map1 = (idtScale1 ? (SupportLocal*)PX_PLACEMENT_NEW(buff1, SupportLocalImpl<ConvexHullNoScaleV>)((ConvexHullNoScaleV&)convexHull1, transf1, convexHull1.vertex2Shape, convexHull1.shape2Vertex, idtScale1) : 
		(SupportLocal*)PX_PLACEMENT_NEW(buff1, SupportLocalImpl<ConvexHullV>)(convexHull1, transf1, convexHull1.vertex2Shape, convexHull1.shape2Vertex, idtScale1));


	//If the origContacts == 1, which means GJK generate  one contact
	PxU32 origContacts = numContacts;

	if(generateFullContactManifold(polyData0, polyData1, map0, map1, manifoldContacts, numContacts, contactDist, normal, doOverlapTest))
	{
		//if we are using gjk direction to search for the faces but the 2D projection fail to generate contacts, we should try to use the
		//sperating axis test to get the minimum sperating axis to calculate the faces to improve stability. However, this should be a 
		//rare case.
		if(numContacts == origContacts && doOverlapTest == false)
		{
			//use SAT to try to generate contacts
			generateFullContactManifold(polyData0, polyData1, map0, map1, manifoldContacts, numContacts, contactDist, normal, true);
		}

		//if we already have a gjk contacts, but if the full manifold generate contacts, we need to drop the gjk contact, because the normal
		//will be different
		if(numContacts != origContacts && origContacts != 0)
		{
			numContacts--;
			manifoldContacts++;
		}
	
		//reduce contacts
		manifold.addBatchManifoldContacts(manifoldContacts, numContacts);

		const Vec3V worldNormal =  manifold.getWorldNormal(transf1);
		
		//add the manifold contacts;
		manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1);


#ifdef	PCM_LOW_LEVEL_DEBUG
		manifold.drawManifold(*gRenderOutPut, transf0, transf1);
#endif
		return true;
		
	}

	return false;

}

static PxGJKStatus convexHullNoScale0(Gu::ShrunkConvexHullV& convexHull0, Gu::ShrunkConvexHullV& convexHull1, const bool idtScale1, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& closestA,
	Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penDep, PersistentContactManifold& manifold)
{
	if(idtScale1)
	{
		return gjkRelativePenetration((Gu::ShrunkConvexHullNoScaleV&)convexHull0, (Gu::ShrunkConvexHullNoScaleV&)convexHull1, aToB, contactDist, closestA, closestB, normal, penDep,
						manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints);
	}
	else
	{
		return gjkRelativePenetration((Gu::ShrunkConvexHullNoScaleV&)convexHull0, convexHull1, aToB, contactDist, closestA, closestB, normal, penDep,
					manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints);
	}
}

static PxGJKStatus convexHullHasScale0(Gu::ShrunkConvexHullV& convexHull0, Gu::ShrunkConvexHullV& convexHull1, const bool idtScale1, const Ps::aos::PsMatTransformV& aToB, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& closestA,
	Ps::aos::Vec3V& closestB, Ps::aos::Vec3V& normal, Ps::aos::FloatV& penDep, PersistentContactManifold& manifold)
{
	if(idtScale1)
	{
		return Gu::gjkRelativePenetration(convexHull0, (Gu::ShrunkConvexHullNoScaleV&)convexHull1, aToB, contactDist, closestA, closestB, normal, penDep,
						manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints);
	}
	else
	{
		return Gu::gjkRelativePenetration(convexHull0, convexHull1, aToB, contactDist, closestA, closestB, normal, penDep,
					manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints);
	}
}

bool pcmContactConvexConvex(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;
	const PxConvexMeshGeometryLL& shapeConvex0 = shape0.get<const PxConvexMeshGeometryLL>();
	const PxConvexMeshGeometryLL& shapeConvex1 = shape1.get<const PxConvexMeshGeometryLL>();
	Gu::PersistentContactManifold& manifold = cache.getManifold();

	Ps::prefetchLine(shapeConvex0.hullData);
	Ps::prefetchLine(shapeConvex1.hullData);


	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());

	const FloatV zero = FZero();
	const Vec3V zeroV = V3Zero();

	const Vec3V vScale0 = V3LoadU(shapeConvex0.scale.scale);
	const Vec3V vScale1 = V3LoadU(shapeConvex1.scale.scale);
	const FloatV contactDist = FLoad(contactDistance);

	//Transfer A into the local space of B
	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);
	const PsTransformV curRTrans(transf1.transformInv(transf0));
	const PsMatTransformV aToB(curRTrans);
	
	const Gu::ConvexHullData* hullData0 = shapeConvex0.hullData;
	const Gu::ConvexHullData* hullData1 = shapeConvex1.hullData;

	const FloatV convexMargin0 = Gu::CalculatePCMConvexMargin(hullData0, vScale0);
	const FloatV convexMargin1 = Gu::CalculatePCMConvexMargin(hullData1, vScale1);
	
	const PxU32 initialContacts = manifold.mNumContacts;
	
	const FloatV minMargin = FMin(convexMargin0, convexMargin1);
	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.8f));
	
	manifold.refreshContactPoints(aToB, projectBreakingThreshold, contactDist);
	const PxU32 newContacts = manifold.mNumContacts;
	const bool bLostContacts = (newContacts != initialContacts);

#ifdef	PCM_LOW_LEVEL_DEBUG
	gRenderOutPut = cache.mRenderOutput;
#endif

	//manifold.mNumContacts = 0;
	PX_UNUSED(bLostContacts);
	if(bLostContacts || manifold.invalidate_BoxConvex(curRTrans, minMargin))
	{

		const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.05f));

		PxGJKStatus status = PxGJKStatus(manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT);

		const bool idtScale0 = shapeConvex0.scale.isIdentity();
		const bool idtScale1 = shapeConvex1.scale.isIdentity();
		const QuatV vQuat0 = QuatVLoadU(&shapeConvex0.scale.rotation.x);
		const QuatV vQuat1 = QuatVLoadU(&shapeConvex1.scale.rotation.x);
		Gu::ShrunkConvexHullV convexHull0(hullData0, zeroV, vScale0, vQuat0);
		Gu::ShrunkConvexHullV convexHull1(hullData1, zeroV, vScale1, vQuat1);

		Vec3V closestA(zeroV), closestB(zeroV), normal(zeroV); // from a to b
		FloatV penDep = zero;
		
		if(idtScale0)
		{

			status = convexHullNoScale0(convexHull0, convexHull1, idtScale1, aToB, contactDist, closestA, closestB, normal, penDep, manifold);
		}
		else
		{
			status = convexHullHasScale0(convexHull0, convexHull1, idtScale1, aToB, contactDist, closestA, closestB, normal, penDep, manifold);

		}

		manifold.setRelativeTransform(curRTrans);

		Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
		PxU32 numContacts = 0;
		if(status == GJK_DEGENERATE)
		{
			return fullContactsGenerationConvexConvex(convexHull0, convexHull1, transf0, transf1, idtScale0, idtScale1, manifoldContacts, numContacts, contactBuffer, 
				manifold, normal, contactDist, true);
		}
		else if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else
		{
			bool doOverlapTest  = false;
			if(manifold.mNumContacts > 0)
			{
				if(status == GJK_CONTACT)
				{
			
					const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);

					//Add contact to contact stream
					manifoldContacts[numContacts].mLocalPointA = localPointA;
					manifoldContacts[numContacts].mLocalPointB = closestB;
					manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;

					//Add contact to manifold
					manifold.addManifoldPoint(localPointA, closestB, localNormalPen, replaceBreakingThreshold);
				}
				else
				{
					PX_ASSERT(status == EPA_CONTACT);
				
					EPASupportMapPairRelativeImpl<Gu::ConvexHullV, Gu::ConvexHullV> supportMap((Gu::ConvexHullV&)convexHull0, (Gu::ConvexHullV&)convexHull1, aToB);
					status= epaPenetration((Gu::ConvexHullV&)convexHull0, (Gu::ConvexHullV&)convexHull1, &supportMap, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, 
						closestA, closestB, normal, penDep); 


					if(status == EPA_CONTACT)
					{
						
						const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
						const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
						
						//Add contact to contact stream
						manifoldContacts[numContacts].mLocalPointA = localPointA;
						manifoldContacts[numContacts].mLocalPointB = closestB;
						manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;

						//Add contact to manifold
						manifold.addManifoldPoint(localPointA, closestB, localNormalPen, replaceBreakingThreshold);
					}
					else
					{
						if(status == EPA_FAIL)
							doOverlapTest = true;
					}
				}

				if(manifold.mNumContacts < initialContacts || doOverlapTest)
				{
					return fullContactsGenerationConvexConvex(convexHull0, convexHull1, transf0, transf1, idtScale0, idtScale1, manifoldContacts, numContacts, contactBuffer, manifold, normal, contactDist, doOverlapTest);
				}
				else
				{
					const Vec3V worldNormal = manifold.getWorldNormal(transf1);
					manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1);
					return true;
				}
			}
			else
			{
				//a pair of object in contact the first time
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
				
					EPASupportMapPairRelativeImpl<Gu::ConvexHullV, Gu::ConvexHullV> supportMap((Gu::ConvexHullV&)convexHull0, (Gu::ConvexHullV&)convexHull1, aToB);
					status= epaPenetration((Gu::ConvexHullV&)convexHull0, (Gu::ConvexHullV&)convexHull1, &supportMap, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, 
						closestA, closestB, normal, penDep); 

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
						if(status == EPA_FAIL)
							doOverlapTest = true;
					}
				}

				return fullContactsGenerationConvexConvex(convexHull0, convexHull1, transf0, transf1, idtScale0, idtScale1, manifoldContacts, numContacts, 
					contactBuffer, manifold, normal, contactDist, doOverlapTest);
			}
		}
	}
	else if(manifold.getNumContacts()> 0)
	{
		const Vec3V worldNormal =  manifold.getWorldNormal(transf1);
		manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1);
		return true;
	}

	return false;
	
}

}
}
