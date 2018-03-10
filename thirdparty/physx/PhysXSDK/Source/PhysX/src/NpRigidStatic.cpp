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


#include "NpRigidStatic.h"
#include "NpPhysics.h"
#include "ScbNpDeps.h"
#include "NpScene.h"

using namespace physx;

NpRigidStatic::NpRigidStatic(const PxTransform& pose)
: NpRigidStaticT(PxConcreteType::eRIGID_STATIC, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
, mRigidStatic(pose)
{
}

NpRigidStatic::~NpRigidStatic()
{
}

// PX_SERIALIZATION
void NpRigidStatic::requires(PxProcessPxBaseCallback& c)
{
	NpRigidStaticT::requires(c);	
}

NpRigidStatic* NpRigidStatic::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpRigidStatic* obj = new (address) NpRigidStatic(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpRigidStatic);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

void NpRigidStatic::release()
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	NpPhysics::getInstance().notifyDeletionListenersUserRelease(this, userData);

	Scb::Scene* s = mRigidStatic.getScbSceneForAPI();

	bool noSim = mRigidStatic.getScRigidCore().getActorFlags().isSet(PxActorFlag::eDISABLE_SIMULATION);
	// important to check the non-buffered flag because it tells what the current internal state of the object is
	// (someone might switch to non-simulation and release all while the sim is running). Reading is fine even if 
	// the sim is running because actor flags are read-only internally.
	if (s && noSim)
	{
		// need to do it here because the Np-shape buffer will not be valid anymore after the release below
		// and unlike simulation objects, there is no shape buffer in the simulation controller
		getShapeManager().clearShapesOnRelease(*s, *this);
	}

	NpRigidStaticT::release();

	if (s)
	{
		s->removeRigidStatic(mRigidStatic, true, noSim);
		static_cast<NpScene*>(s->getPxScene())->removeFromRigidActorList(mIndex);
	}

	mRigidStatic.destroy();
}

void NpRigidStatic::setGlobalPose(const PxTransform& pose, bool /*wake*/)
{
	PX_CHECK_AND_RETURN(pose.isSane(), "NpRigidStatic::setGlobalPose: pose is not valid.");

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	NpScene* npScene = NpActor::getAPIScene(*this);
#ifdef PX_CHECKED
	if(npScene)
		npScene->checkPositionSanity(*this, pose, "PxRigidStatic::setGlobalPose");
#endif

	mRigidStatic.setActor2World(pose.getNormalized());

	if(npScene)
	{
		mShapeManager.markAllSceneQueryForUpdate(npScene->getSceneQueryManagerFast());
		npScene->getSceneQueryManagerFast().invalidateStaticTimestamp();
	}

#if PX_SUPPORT_VISUAL_DEBUGGER
	// have to do this here since this call gets not forwarded to Scb::RigidStatic
	Scb::Scene* scbScene = NpActor::getScbFromPxActor(*this).getScbSceneForAPI();
	if(scbScene && scbScene->getSceneVisualDebugger().isConnected(true))
		scbScene->getSceneVisualDebugger().updatePvdProperties(&mRigidStatic);
#endif

	updateShaderComs();
}

PxTransform NpRigidStatic::getGlobalPose() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return mRigidStatic.getActor2World();
}

PxShape* NpRigidStatic::createShape(const PxGeometry& geometry, PxMaterial*const* materials, PxU16 materialCount, PxShapeFlags shapeFlags)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	PX_CHECK_AND_RETURN_NULL(materials, "createShape: material pointer is NULL");
	PX_CHECK_AND_RETURN_NULL(materialCount>0, "createShape: material count is zero");

	NpShape* shape = static_cast<NpShape*>(NpPhysics::getInstance().createShape(geometry, materials, materialCount, true, shapeFlags));

	if ( shape != NULL )
	{
		mShapeManager.attachShape(*shape, *this);
		GRB_EVENT(getScene(), GrbInteropEvent3, GrbInteropEvent3::PxActorCreateShape, static_cast<PxActor *>(this), static_cast<PxShape *>(shape), 1);
		shape->releaseInternal();
	}
	return shape;
}

PxU32 physx::NpRigidStaticGetShapes(Scb::RigidStatic& rigid, void* const *&shapes)
{
	NpRigidStatic* a = static_cast<NpRigidStatic*>(rigid.getScRigidCore().getPxActor());
	NpShapeManager& sm = a->getShapeManager();
	shapes = reinterpret_cast<void *const *>(sm.getShapes());
	return sm.getNbShapes();
}


void NpRigidStatic::switchToNoSim()
{
	getScbRigidStaticFast().switchToNoSim(false);
}


void NpRigidStatic::switchFromNoSim()
{
	getScbRigidStaticFast().switchFromNoSim(false);
}

#ifdef PX_CHECKED
bool NpRigidStatic::checkConstraintValidity() const
{
	// Perhaps NpConnectorConstIterator would be worth it...
	NpConnectorIterator iter = (const_cast<NpRigidStatic*>(this))->getConnectorIterator(NpConnectorType::eConstraint); 
	while (PxBase* ser = iter.getNext())
	{
		NpConstraint* c = static_cast<NpConstraint*>(ser);
		if(!c->NpConstraint::isValid())
			return false;
	}
	return true;
}
#endif

#if PX_ENABLE_DEBUG_VISUALIZATION
#include "GuDebug.h"
void NpRigidStatic::visualize(Cm::RenderOutput& out, NpScene* scene)
{
	NpRigidStaticT::visualize(out, scene);

	if (getScbRigidStaticFast().getActorFlags() & PxActorFlag::eVISUALIZATION)
	{
		Scb::Scene& scbScene = scene->getScene();
		PxReal scale = scbScene.getVisualizationParameter(PxVisualizationParameter::eSCALE);

		//visualize actor frames
		PxReal actorAxes = scale * scbScene.getVisualizationParameter(PxVisualizationParameter::eACTOR_AXES);
		if (actorAxes != 0)
			out << Gu::Debug::convertToPxMat44(getGlobalPose()) << Cm::DebugBasis(PxVec3(actorAxes));
	}
}
#endif

