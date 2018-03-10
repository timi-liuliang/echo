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
#include "GuVecBox.h"
#include "GuVecCapsule.h"
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

static bool fullContactsGenerationCapsuleBox(const Gu::CapsuleV& capsule, const Gu::BoxV& box, const PxVec3 halfExtents,  const Ps::aos::PsMatTransformV& aToB, const Ps::aos::PsTransformV& transf0,const Ps::aos::PsTransformV& transf1,
								Gu::PersistentContact* manifoldContacts, PxU32& numContacts, Gu::ContactBuffer& contactBuffer, Gu::PersistentContactManifold& manifold, Ps::aos::Vec3VArg normal, const Ps::aos::FloatVArg contactDist, const bool doOverlapTest)
{

	using namespace Ps::aos;
	Gu::PolygonalData polyData;
	PCMPolygonalBox polyBox(halfExtents);
	polyBox.getPolygonalData(&polyData);

	Mat33V identity = M33Identity();
	SupportLocalImpl<BoxV> map(box, transf1, identity, identity);
	
	PxU32 origContacts = numContacts;
	if(generateCapsuleBoxFullContactManifold(capsule, polyData, &map, aToB,  manifoldContacts, numContacts, contactDist, normal, doOverlapTest))
	{
		//EPA has contacts and we have new contacts, we discard the EPA contacts
		if(origContacts != 0 && numContacts != origContacts)
		{
			numContacts--;
			manifoldContacts++;
		}

		manifold.addBatchManifoldContacts2(manifoldContacts, numContacts);
		
		normal = transf1.rotate(normal);
		
		manifold.addManifoldContactsToContactBuffer(contactBuffer, normal, transf0, capsule.radius);
	
		return true;
		
	}

	return false;

}


bool pcmContactCapsuleBox(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;

	Gu::PersistentContactManifold& manifold = cache.getManifold();
	Ps::prefetchLine(&manifold, 256);
	const PxCapsuleGeometry& shapeCapsule = shape0.get<const PxCapsuleGeometry>();
	const PxBoxGeometry& shapeBox = shape1.get<const PxBoxGeometry>();

	PX_ASSERT(transform1.q.isSane());
	PX_ASSERT(transform0.q.isSane());  

	const Vec3V zeroV = V3Zero();
	const Vec3V boxExtents = V3LoadU(shapeBox.halfExtents);

	const FloatV contactDist = FLoad(contactDistance);

	const PsTransformV transf0 = loadTransformA(transform0);
	const PsTransformV transf1 = loadTransformA(transform1);

	const PsTransformV curRTrans = transf1.transformInv(transf0);
	const PsMatTransformV aToB_(curRTrans);

	const FloatV capsuleRadius = FLoad(shapeCapsule.radius);
	const FloatV capsuleHalfHeight = FLoad(shapeCapsule.halfHeight);

	const PxU32 initialContacts = manifold.mNumContacts;

	const FloatV boxMargin = Gu::CalculatePCMBoxMargin(boxExtents);
	
	const FloatV minMargin = FMin(boxMargin, capsuleRadius);

	const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.8f));
	
	//manifold.refreshContactPoints(curRTrans, projectBreakingThreshold, contactDist);
	const FloatV refreshDist = FAdd(contactDist, capsuleRadius);
	manifold.refreshContactPoints(aToB_, projectBreakingThreshold, refreshDist);

	const PxU32 newContacts = manifold.mNumContacts;
	const bool bLostContacts = (newContacts != initialContacts);//((initialContacts == 0) || (newContacts != initialContacts));


	PX_UNUSED(bLostContacts);
	if(bLostContacts || manifold.invalidate_SphereCapsule(curRTrans, minMargin))	
	{

		PxGJKStatus status = PxGJKStatus(manifold.mNumContacts > 0 ? GJK_UNDEFINED : GJK_NON_INTERSECT);

		Vec3V closestA(zeroV), closestB(zeroV);
		Vec3V normal(zeroV); // from a to b
		const FloatV zero = FZero();
		FloatV penDep = zero;

		manifold.setRelativeTransform(curRTrans);
		const PsMatTransformV aToB(curRTrans);
		
		const bool takeCoreShape = true;
		Gu::BoxV box(transf1.p, boxExtents);
		box.setMargin(zero);
		
		//transform capsule into the local space of box
		Gu::CapsuleV capsule(aToB.p, aToB.rotate(V3Scale(V3UnitX(), capsuleHalfHeight)), capsuleRadius);
		
		status =  Gu::GJKLocalPenetration(capsule, box, contactDist, closestA, closestB, normal, penDep, manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, takeCoreShape);

		Gu::PersistentContact* manifoldContacts = PX_CP_TO_PCP(contactBuffer.contacts);
		PxU32 numContacts = 0;
		bool doOverlapTest = false;
		if(status == GJK_NON_INTERSECT)
		{
			return false;
		}
		else if(status == GJK_DEGENERATE)
		{
			return fullContactsGenerationCapsuleBox(capsule, box, shapeBox.halfExtents,  aToB, transf0, transf1, manifoldContacts, numContacts, contactBuffer, 
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
				status= Gu::EPALocalPenetration(capsule, box, closestA, closestB, normal, penDep,
				manifold.mAIndice, manifold.mBIndice, manifold.mNumWarmStartPoints, takeCoreShape);
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
				return fullContactsGenerationCapsuleBox(capsule, box, shapeBox.halfExtents,  aToB, transf0, transf1, manifoldContacts, numContacts, contactBuffer, 
					manifold, normal, contactDist, doOverlapTest);
			}
			else
			{
				
				//The contacts is either come from GJK or EPA
				const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.1f));
				const Vec4V localNormalPen = V4SetW(Vec4V_From_Vec3V(normal), penDep);
				manifold.addManifoldPoint2(curRTrans.transformInv(closestA), closestB, localNormalPen, replaceBreakingThreshold);
				
				normal = transf1.rotate(normal);
				manifold.addManifoldContactsToContactBuffer(contactBuffer, normal, transf0, capsuleRadius);
			
				return true;
			}
		}	
	}
	else if(manifold.getNumContacts() > 0)
	{
		const Vec3V worldNormal = manifold.getWorldNormal(transf1);
		manifold.addManifoldContactsToContactBuffer(contactBuffer, worldNormal, transf0, capsuleRadius);
		return true;
	}

	return false;

}
}
}
