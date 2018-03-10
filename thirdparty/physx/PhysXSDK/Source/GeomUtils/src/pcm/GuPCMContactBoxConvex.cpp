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
#include "GuVecBox.h"
#include "GuVecShrunkBox.h"
#include "GuVecShrunkConvexHull.h"
#include "GuVecShrunkConvexHullNoScale.h"
#include "GuVecConvexHull.h"
#include "GuVecConvexHullNoScale.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuPCMContactGen.h"
#include "GuPCMShapeConvex.h"
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

static bool fullContactsGenerationBoxConvex(const PxVec3& halfExtents, const Gu::BoxV& box, Gu::ConvexHullV& convexHull, const Ps::aos::PsTransformV& transf0, const Ps::aos::PsTransformV& transf1, 
									Gu::PersistentContact* manifoldContacts, PxU32& numContacts, Gu::ContactBuffer& contactBuffer, Gu::PersistentContactManifold& manifold, Ps::aos::Vec3VArg normal, const Ps::aos::FloatVArg contactDist, const bool idtScale, const bool doOverlapTest)
{
	using namespace Ps::aos;
	Gu::PolygonalData polyData0;
	PCMPolygonalBox polyBox0(halfExtents);
	polyBox0.getPolygonalData(&polyData0);
	polyData0.mPolygonVertexRefs = gPCMBoxPolygonData;
	
	Gu::PolygonalData polyData1;
	getPCMConvexData(convexHull, idtScale, polyData1);
	
	Mat33V identity =  M33Identity();
	SupportLocalImpl<BoxV> map0(box, transf0, identity, identity, true);

	PxU8 buff1[sizeof(SupportLocalImpl<ConvexHullV>)];

	SupportLocal* map1 = (idtScale ? (SupportLocal*)PX_PLACEMENT_NEW(buff1, SupportLocalImpl<ConvexHullNoScaleV>)((ConvexHullNoScaleV&)convexHull, transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale) : 
		(SupportLocal*)PX_PLACEMENT_NEW(buff1, SupportLocalImpl<ConvexHullV>)(convexHull, transf1, convexHull.vertex2Shape, convexHull.shape2Vertex, idtScale));

	//If the origContacts == 1, which means GJK generate  one contact
	PxU32 origContacts = numContacts;
	if(generateFullContactManifold(polyData0, polyData1, &map0, map1, manifoldContacts, numContacts, contactDist, normal, doOverlapTest))
	{
		//if we already have a gjk contacts, but the full manifold generate contacts, we need to drop the gjk contact, because the normal
		//will be different
		if(numContacts != origContacts && origContacts != 0)
		{
			numContacts--;
			manifoldContacts++;
		}
		//reduce contacts
		manifold.addBatchManifoldContacts(manifoldContacts, numContacts);
		
		const Vec3V worldNormal =  manifold.getWorldNormal(transf1);

		manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1);
		
		return true;
	}

	return false;

}


bool pcmContactBoxConvex(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;

	const PxConvexMeshGeometryLL& shapeConvex = shape1.get<const PxConvexMeshGeometryLL>();
	const PxBoxGeometry& shapeBox = shape0.get<const PxBoxGeometry>();
	
	Gu::PersistentContactManifold& manifold = cache.getManifold();
	Ps::prefetchLine(shapeConvex.hullData);
	
	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());

	const Vec3V zeroV = V3Zero();


	const FloatV contactDist = FLoad(contactDistance);
	const Vec3V boxExtents = V3LoadU(shapeBox.halfExtents);

	const Vec3V vScale = V3LoadU(shapeConvex.scale.scale);
	
	//Transfer A into the local space of B
	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);
	const PsTransformV curRTrans(transf1.transformInv(transf0));
	const PsMatTransformV aToB(curRTrans);

	const Gu::ConvexHullData* hullData = shapeConvex.hullData;
	const FloatV convexMargin = Gu::CalculatePCMConvexMargin(hullData, vScale);
	const FloatV boxMargin = Gu::CalculatePCMBoxMargin(boxExtents);

	const FloatV minMargin = FMin(convexMargin, boxMargin);//FMin(boxMargin, convexMargin);
	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.8f));
	const PxU32 initialContacts = manifold.mNumContacts;

	manifold.refreshContactPoints(aToB, projectBreakingThreshold, contactDist);  
	
	//After the refresh contact points, the numcontacts in the manifold will be changed
	const PxU32 newContacts = manifold.mNumContacts;

	//const bool bLostContacts = ((initialContacts > 0) & (newContacts != initialContacts));
	const bool bLostContacts = (newContacts != initialContacts);

	PX_UNUSED(bLostContacts);

	if(bLostContacts || manifold.invalidate_BoxConvex(curRTrans, minMargin))	
	{
	
		const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.05f));
		PxGJKStatus status = PxGJKStatus(manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT);

		Vec3V closestA(zeroV), closestB(zeroV), normal(zeroV); // from a to b
		FloatV penDep = FZero(); 
		
		const QuatV vQuat = QuatVLoadU(&shapeConvex.scale.rotation.x);
		Gu::ShrunkConvexHullV convexHull(hullData, zeroV, vScale, vQuat);
		Gu::ShrunkBoxV box(zeroV, boxExtents);
		const bool idtScale = shapeConvex.scale.isIdentity();

		if(idtScale)
		{
			status = Gu::gjkRelativePenetration(box, *PX_SCONVEX_TO_NOSCALECONVEX(&convexHull), aToB, contactDist, closestA, closestB, normal, penDep,
				manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints);
		}
		else
		{
			status = Gu::gjkRelativePenetration(box, convexHull, aToB, contactDist, closestA, closestB, normal, penDep,
				manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints);

		}  

		manifold.setRelativeTransform(curRTrans); 

		Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
		PxU32 numContacts = 0;
		if(status == GJK_DEGENERATE)
		{
			return fullContactsGenerationBoxConvex(shapeBox.halfExtents, box, convexHull, transf0, transf1, manifoldContacts, numContacts, contactBuffer, 
				manifold, normal, contactDist, idtScale, true);
		}
		else if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else
		{
			if(manifold.mNumContacts > 0)
			{
				bool doOverlapTest  = false;
				if(status == GJK_CONTACT)
				{
					const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);

					//Add contact to contact stream
					manifoldContacts[numContacts].mLocalPointA = localPointA;
					manifoldContacts[numContacts].mLocalPointB = closestB;
					manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;

					//Add contact to manifold
					manifold.addManifoldPoint(aToB.transformInv(closestA), closestB, localNormalPen, replaceBreakingThreshold);

				}
				else 
				{
					PX_ASSERT(status == EPA_CONTACT);
				
					EPASupportMapPairRelativeImpl<Gu::BoxV, Gu::ConvexHullV> supportMap((Gu::BoxV&)box, (Gu::ConvexHullV&)convexHull, aToB);
					status= epaPenetration((Gu::BoxV&)box, (Gu::ConvexHullV&)convexHull, &supportMap, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, 
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
						doOverlapTest = true;
					}
				}

					
				if(manifold.mNumContacts < initialContacts || doOverlapTest)//bLostContacts || doOverlapTest)
				{
					return fullContactsGenerationBoxConvex(shapeBox.halfExtents, box, convexHull, transf0, transf1, manifoldContacts, numContacts,  contactBuffer, manifold, normal, contactDist, idtScale, doOverlapTest);
				}
				else
				{
					const Vec3V worldNormal = transf1.rotate(normal);
					manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1);

					return true;
				}
			}
			else
			{
				bool doOverlapTest = false;
				//generate full manifold
				if(status == GJK_CONTACT)
				{
					const Vec3V localPointA = aToB.transformInv(closestA);//curRTrans.transformInv(closestA);
					const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);

					//Add contact to contact stream
					manifoldContacts[numContacts].mLocalPointA = localPointA;
					manifoldContacts[numContacts].mLocalPointB = closestB;
					manifoldContacts[numContacts++].mLocalNormalPen = localNormalPen;

				}
				else if(status == EPA_CONTACT)
				{
					EPASupportMapPairRelativeImpl<Gu::BoxV, Gu::ConvexHullV> supportMap((Gu::BoxV&)box, (Gu::ConvexHullV&)convexHull, aToB);
					status= epaPenetration((Gu::BoxV&)box, (Gu::ConvexHullV&)convexHull, &supportMap, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, 
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
						doOverlapTest = true;
					}

				}
				return fullContactsGenerationBoxConvex(shapeBox.halfExtents, box, convexHull, transf0, transf1, manifoldContacts, numContacts,  contactBuffer, manifold, normal, contactDist, idtScale, doOverlapTest);
			}
			
		} 
	}
	else if(manifold.getNumContacts()>0)
	{
		const Vec3V worldNormal =  manifold.getWorldNormal(transf1);
		manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf1);
		return true;
	}

	return false;

}

}//Gu
}//physx
