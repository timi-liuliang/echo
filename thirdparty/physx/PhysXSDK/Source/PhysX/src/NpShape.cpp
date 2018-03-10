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


#include "NpShape.h"
#include "NpRigidDynamic.h"
#include "NpRigidStatic.h"
#include "NpArticulationLink.h"
#include "GuConvexMesh.h"
#include "GuTriangleMesh.h"
#include "GuHeightFieldUtil.h"
#include "SqUtilities.h"
#include "ScbNpDeps.h"

using namespace physx;

NpShape::NpShape(const PxGeometry& geometry,
				 PxShapeFlags shapeFlags,
				 const PxU16* materialIndices,
				 PxU16 materialCount,
				 bool isExclusive) 
:	PxShape				(PxConcreteType::eSHAPE, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
,	mActor				(NULL)
,	mShape				(geometry, shapeFlags, materialIndices, materialCount, isExclusive)
,	mName				(NULL)
,	mInternalShapeFlags (PxU8(isExclusive ? NpInternalShapeFlag::eEXCLUSIVE : 0))
{
	PX_ASSERT(mShape.getScShape().getPxShape() == static_cast<PxShape*>(this));

	PxShape::userData = NULL;

	incMeshRefCount();
}


NpShape::~NpShape()
{
	decMeshRefCount();

	PxU32 nbMaterials = mShape.getNbMaterials();
	for (PxU32 i=0; i < nbMaterials; i++)
	{
		NpMaterial* mat = static_cast<NpMaterial*>(mShape.getMaterial(i));
		mat->decRefCount();
	}
}


void NpShape::onRefCountZero()
{
	NpFactory::getInstance().onShapeRelease(this);
	// see NpShape.h for ref counting semantics for shapes
	NpDestroy(getScbShape());
}

// PX_SERIALIZATION
void NpShape::exportExtraData(PxSerializationContext& stream)
{	
	getScbShape().getScShape().exportExtraData(stream);
	stream.writeName(mName);
}

void NpShape::importExtraData(PxDeserializationContext& context)
{
	getScbShape().getScShape().importExtraData(context);
	context.readName(mName);
}

void NpShape::requires(PxProcessPxBaseCallback& c)
{
	//meshes
	PxBase* mesh = NULL;
	switch(mShape.getGeometryType())
	{
	case PxGeometryType::eCONVEXMESH:
		mesh = static_cast<const PxConvexMeshGeometry&>(mShape.getGeometry()).convexMesh;
		break;
	case PxGeometryType::eHEIGHTFIELD:
		mesh = static_cast<const PxHeightFieldGeometry&>(mShape.getGeometry()).heightField;
		break;
	case PxGeometryType::eTRIANGLEMESH:
		mesh = static_cast<const PxTriangleMeshGeometry&>(mShape.getGeometry()).triangleMesh;
		break;
	case PxGeometryType::eSPHERE:
	case PxGeometryType::ePLANE:
	case PxGeometryType::eCAPSULE:
	case PxGeometryType::eBOX:
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		break;
	}
	
	if(mesh)
		c.process(*mesh);

	//material
	PxU32 nbMaterials = mShape.getNbMaterials();
	for (PxU32 i=0; i < nbMaterials; i++)
	{
		NpMaterial* mat = static_cast<NpMaterial*>(mShape.getMaterial(i));
		c.process(*mat);
	}
}

void NpShape::resolveReferences(PxDeserializationContext& context)
{	
	// getMaterials() only works after material indices have been patched. 
	// in order to get to the new material indices, we need access to the new materials.
	// this only leaves us with the option of acquiring the material through the context given an old material index (we do have the mapping)
	{
		PxU32 nbIndices = mShape.getScShape().getNbMaterialIndices();
		const PxU16* indices = mShape.getScShape().getMaterialIndices();

		for (PxU32 i=0; i < nbIndices; i++)
		{
			PxBase* base = context.resolveReference(PX_SERIAL_REF_KIND_MATERIAL_IDX, size_t(indices[i]));
			PX_ASSERT(base && base->is<PxMaterial>());

			NpMaterial& material = *static_cast<NpMaterial*>(base);
			getScbShape().getScShape().resolveMaterialReference(i, PxU16(material.getHandle()));
		}
	}

	context.translatePxBase(mActor);

	getScbShape().getScShape().resolveReferences(context);	


	incMeshRefCount();

	// Increment materials' refcounts in a second pass. Works better in case of failure above.
	PxU32 nbMaterials = mShape.getNbMaterials();
	for (PxU32 i=0; i < nbMaterials; i++)
	{
		NpMaterial* mat = static_cast<NpMaterial*>(mShape.getMaterial(i));
		mat->incRefCount();
	}	
}

NpShape* NpShape::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpShape* obj = new (address) NpShape(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpShape);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION


void NpShape::release()
{
	NP_WRITE_CHECK(getOwnerScene());

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeRelease, static_cast<PxShape *>(this), (PxU8)1);

	bool userRefReleased = !(mBaseFlags & PxBaseFlag::eIS_RELEASABLE);
	if(isExclusive() && userRefReleased)
	{
		physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxShape::release: use of this method to remove a shape from an actor is deprecated, use PxRigidActor::detachShape");
		NpActor::getShapeManager(*mActor).detachShape(*this, *mActor, true);
		return;
	}
	else if(userRefReleased)
	{
		physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxShape::release: user reference has already been released");
		return;
	}
	
	releaseInternal();
}

void NpShape::releaseInternal()
{
	NpPhysics::getInstance().notifyDeletionListenersUserRelease(this, userData);

	mBaseFlags &= ~PxBaseFlag::eIS_RELEASABLE;
	decRefCount();
}

Sc::RigidCore& NpShape::getScRigidObjectExclusive() const
{
	const PxType actorType = mActor->getConcreteType();

	if (actorType == PxConcreteType::eRIGID_DYNAMIC)
		return static_cast<NpRigidDynamic&>(*mActor).getScbBodyFast().getScBody();
	else if (actorType == PxConcreteType::eARTICULATION_LINK)
		return static_cast<NpArticulationLink&>(*mActor).getScbBodyFast().getScBody();
	else
		return static_cast<NpRigidStatic&>(*mActor).getScbRigidStaticFast().getScStatic();
}

PxGeometryType::Enum NpShape::getGeometryType() const
{
	NP_READ_CHECK(getOwnerScene());

	return mShape.getGeometryType();
}


void NpShape::setGeometry(const PxGeometry& g)
{
	NP_WRITE_CHECK(getOwnerScene());
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setGeometry: shared shapes attached to actors are not writable.");

	// PT: fixes US2117
	if(g.getType() != getGeometryTypeFast())
	{
		getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxShape::setGeometry(): Invalid geometry type. Changing the type of the shape is not supported.");
		return;
	}

#ifdef PX_CHECKED
	bool isValid = false;
	switch(g.getType())
	{
		case PxGeometryType::eSPHERE:
			isValid = static_cast<const PxSphereGeometry&>(g).isValid();
		break;

		case PxGeometryType::ePLANE:
			isValid = static_cast<const PxPlaneGeometry&>(g).isValid();
		break;

		case PxGeometryType::eCAPSULE:
			isValid = static_cast<const PxCapsuleGeometry&>(g).isValid();
		break;

		case PxGeometryType::eBOX:
			isValid = static_cast<const PxBoxGeometry&>(g).isValid();
		break;

		case PxGeometryType::eCONVEXMESH:
			isValid = static_cast<const PxConvexMeshGeometry&>(g).isValid();
		break;

		case PxGeometryType::eTRIANGLEMESH:
			isValid = static_cast<const PxTriangleMeshGeometry&>(g).isValid();
		break;

		case PxGeometryType::eHEIGHTFIELD:
			isValid = static_cast<const PxHeightFieldGeometry&>(g).isValid();
		break;
		
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default:
			break;
	}

	if(!isValid)
	{
		getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxShape::setGeometry(): Invalid geometry!");
		return;
	}
#endif

	decMeshRefCount();

	mShape.setGeometry(g);

	incMeshRefCount();

	if((mShape.getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE) && mActor)
	{
		PX_ASSERT(mActor);

		NpScene* scene = NpActor::getOwnerScene(*mActor);
		if(scene)
		{
			Sq::ActorShape* sqData = NpActor::getShapeManager(*mActor).findSceneQueryData(*this);
			PX_ASSERT(sqData);
			scene->getSceneQueryManagerFast().markForUpdate(sqData);
		}
	}

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetGeometry, static_cast<PxShape *>(this), g);
}


PxGeometryHolder NpShape::getGeometry() const
{
	PX_COMPILE_TIME_ASSERT(sizeof(Gu::GeometryUnion)>=sizeof(PxGeometryHolder));
	return reinterpret_cast<const PxGeometryHolder&>(mShape.getGeometry());
}


bool NpShape::getBoxGeometry(PxBoxGeometry& g) const
{
	NP_READ_CHECK(getOwnerScene());

	if (getGeometryTypeFast() != PxGeometryType::eBOX)
		return false;

	g = static_cast<const PxBoxGeometry &>(mShape.getGeometry());
	return true;
}


bool NpShape::getSphereGeometry(PxSphereGeometry& g) const
{
	NP_READ_CHECK(getOwnerScene());

	if (getGeometryTypeFast() != PxGeometryType::eSPHERE)
		return false;

	g = static_cast<const PxSphereGeometry &>(mShape.getGeometry());
	return true;
}


bool NpShape::getCapsuleGeometry(PxCapsuleGeometry& g) const
{
	NP_READ_CHECK(getOwnerScene());

	if (getGeometryTypeFast() != PxGeometryType::eCAPSULE)
		return false;

	g = static_cast<const PxCapsuleGeometry &>(mShape.getGeometry());
	return true;
}


bool NpShape::getPlaneGeometry(PxPlaneGeometry& g) const
{
	NP_READ_CHECK(getOwnerScene());

	if (getGeometryTypeFast() != PxGeometryType::ePLANE)
		return false;

	g = static_cast<const PxPlaneGeometry &>(mShape.getGeometry());
	return true;
}


bool NpShape::getConvexMeshGeometry(PxConvexMeshGeometry& g) const
{
	NP_READ_CHECK(getOwnerScene());

	if (getGeometryTypeFast() != PxGeometryType::eCONVEXMESH)
		return false;

	g = static_cast<const PxConvexMeshGeometry &>(mShape.getGeometry());
	return true;
}


bool NpShape::getTriangleMeshGeometry(PxTriangleMeshGeometry& g) const
{
	NP_READ_CHECK(getOwnerScene());

	if (getGeometryTypeFast() != PxGeometryType::eTRIANGLEMESH)
		return false;

	g = static_cast<const PxTriangleMeshGeometry &>(mShape.getGeometry());
	return true;
}


bool NpShape::getHeightFieldGeometry(PxHeightFieldGeometry& g) const
{
	NP_READ_CHECK(getOwnerScene());

	if (getGeometryTypeFast() != PxGeometryType::eHEIGHTFIELD)
		return false;

	g = static_cast<const PxHeightFieldGeometry &>(mShape.getGeometry());
	return true;
}


PxRigidActor* NpShape::getActor() const
{
	NP_READ_CHECK(getOwnerScene());
	return mActor;
}


void NpShape::setLocalPose(const PxTransform& newShape2Actor)
{
	PX_CHECK_AND_RETURN(newShape2Actor.isSane(), "NpShape::setLocalPose: pose is not valid.");
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setLocalPose: shared shapes attached to actors are not writable.");
	NP_WRITE_CHECK(getOwnerScene());

	mShape.setShape2Actor(newShape2Actor.getNormalized());

	if(mShape.getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE && mActor)
	{
		NpScene* scene = NpActor::getAPIScene(*mActor);
		if(scene)
		{
			Sq::ActorShape* sqData = NpActor::getShapeManager(*mActor).findSceneQueryData(*this);
			PX_ASSERT(sqData);
			scene->getSceneQueryManagerFast().markForUpdate(sqData);
		}
	}

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetLocalPose, static_cast<PxShape *>(this), newShape2Actor);
}


PxTransform NpShape::getLocalPose() const
{
	NP_READ_CHECK(getOwnerScene());

	return mShape.getShape2Actor();
}

///////////////////////////////////////////////////////////////////////////////

void NpShape::setSimulationFilterData(const PxFilterData& data)
{
	NP_WRITE_CHECK(getOwnerScene());
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setSimulationFilterData: shared shapes attached to actors are not writable.");
	mShape.setSimulationFilterData(data);

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetSimulationFilterData, static_cast<PxShape *>(this), data);
}

PxFilterData NpShape::getSimulationFilterData() const
{
	NP_READ_CHECK(getOwnerScene());
	return mShape.getSimulationFilterData();
}

void NpShape::resetFiltering()
{
	physx::shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__, "NpShape::resetFiltering: This method has been deprecated!");

	if (mActor)
	{
		NpScene* scene = NpActor::getOwnerScene(*mActor);
		if (scene)
		{
			PxShape* shape = this;
			scene->resetFiltering(*mActor, &shape, 1);
		}
	}
}

void NpShape::setQueryFilterData(const PxFilterData& data)
{
	NP_WRITE_CHECK(getOwnerScene());
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setQueryFilterData: shared shapes attached to actors are not writable.");

	mShape.getScShape().setQueryFilterData(data);	// PT: this one doesn't need double-buffering

#if PX_SUPPORT_VISUAL_DEBUGGER
	Scb::Scene* scbScene = mShape.getScbSceneForAPI();
	if(scbScene && scbScene->getSceneVisualDebugger().isConnected(true))
		scbScene->getSceneVisualDebugger().updatePvdProperties(&mShape);
#endif

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetQueryFilterData, static_cast<PxShape *>(this), data);
}

PxFilterData NpShape::getQueryFilterData() const
{
	NP_READ_CHECK(getOwnerScene());

	return getQueryFilterDataFast();
}

///////////////////////////////////////////////////////////////////////////////

void NpShape::setMaterials(PxMaterial*const* materials, PxU16 materialCount)
{
	NP_WRITE_CHECK(getOwnerScene());
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setMaterials: shared shapes attached to actors are not writable.");

#ifdef PX_CHECKED
	if (!NpShape::checkMaterialSetup(mShape.getGeometry(), "Shape::setMaterials()", materials,  materialCount))
		return;
#endif

	PxU32 oldMaterialCount = mShape.getNbMaterials();
	PX_ALLOCA(oldMaterials, PxMaterial*, oldMaterialCount);
	PxU32 tmp = mShape.getMaterials(oldMaterials, oldMaterialCount);
	PX_ASSERT(tmp == oldMaterialCount);
	PX_UNUSED(tmp);

	if (mShape.setMaterials(materials, materialCount))
	{
		for(PxU32 i=0; i < materialCount; i++)
			static_cast<NpMaterial*>(materials[i])->incRefCount();

		for(PxU32 i=0; i < oldMaterialCount; i++)
			static_cast<NpMaterial*>(oldMaterials[i])->decRefCount();
	}

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetMaterials, static_cast<PxShape *>(this), materials, materialCount);
}


PxU16 NpShape::getNbMaterials() const
{
	NP_READ_CHECK(getOwnerScene());

	return mShape.getNbMaterials();
}


PxU32 NpShape::getMaterials(PxMaterial** userBuffer, PxU32 bufferSize) const
{
	NP_READ_CHECK(getOwnerScene());

	return mShape.getMaterials(userBuffer, bufferSize);
}

PxMaterial* NpShape::getMaterialFromInternalFaceIndex(PxU32 faceIndex) const
{
	NP_READ_CHECK(getOwnerScene());

	bool isHf = (getGeometryType() == PxGeometryType::eHEIGHTFIELD);
	bool isMesh = (getGeometryType() == PxGeometryType::eTRIANGLEMESH);
	if( faceIndex == 0xFFFFffff && (isHf || isMesh) )
	{
		Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__,
			"PxShape::getMaterialFromInternalFaceIndex received 0xFFFFffff as input - returning NULL.");
		return NULL;
	}

	PxMaterialTableIndex hitMatTableId = 0;

	if(isHf)
	{
		PxHeightFieldGeometry hfGeom;
		getHeightFieldGeometry(hfGeom);

		hitMatTableId = hfGeom.heightField->getTriangleMaterialIndex(faceIndex);
	}
	else if(isMesh)
	{
		PxTriangleMeshGeometry triGeo;
		getTriangleMeshGeometry(triGeo);

		Gu::TriangleMesh* tm = static_cast<Gu::TriangleMesh*>(triGeo.triangleMesh);
		if(tm->hasPerTriangleMaterials())
			hitMatTableId = triGeo.triangleMesh->getTriangleMaterialIndex(faceIndex);
	}

	return getMaterial(hitMatTableId);
}

void NpShape::setContactOffset(PxReal contactOffset)
{
	NP_WRITE_CHECK(getOwnerScene());

	PX_CHECK_AND_RETURN(PxIsFinite(contactOffset), "NpShape::setContactOffset: invalid float");
	PX_CHECK_AND_RETURN((contactOffset >= 0.0f && contactOffset > mShape.getRestOffset()), "NpShape::setContactOffset: contactOffset should be positive, and greater than restOffset!");
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setContactOffset: shared shapes attached to actors are not writable.");

	mShape.setContactOffset(contactOffset);

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetContactOffset, static_cast<PxShape *>(this), contactOffset);
}


PxReal NpShape::getContactOffset() const
{
	NP_READ_CHECK(getOwnerScene());

	return mShape.getContactOffset();
}


void NpShape::setRestOffset(PxReal restOffset)
{
	NP_WRITE_CHECK(getOwnerScene());
	PX_CHECK_AND_RETURN(PxIsFinite(restOffset), "NpShape::setRestOffset: invalid float");
	PX_CHECK_AND_RETURN((restOffset < mShape.getContactOffset()), "NpShape::setRestOffset: restOffset should be less than contactOffset!");
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setRestOffset: shared shapes attached to actors are not writable.");

	mShape.setRestOffset(restOffset);

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetRestOffset, static_cast<PxShape *>(this), restOffset);
}


PxReal NpShape::getRestOffset() const
{
	NP_READ_CHECK(getOwnerScene());

	return mShape.getRestOffset();
}


void NpShape::setActor(PxRigidActor* actor)
{
	PX_ASSERT(isExclusive());
	mActor = actor;
}

void NpShape::setFlagsInternal(PxShapeFlags inFlags)
{
	const bool hasMeshTypeGeom = mShape.getGeometryType() == PxGeometryType::eTRIANGLEMESH || mShape.getGeometryType() == PxGeometryType::eHEIGHTFIELD;

	if(hasMeshTypeGeom && (inFlags & PxShapeFlag::eTRIGGER_SHAPE))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
			"NpShape::setFlag(s): triangle mesh and heightfield triggers are not supported!");
		return;
	}

	if((inFlags & PxShapeFlag::eSIMULATION_SHAPE) && (inFlags & PxShapeFlag::eTRIGGER_SHAPE))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
			"NpShape::setFlag(s): shapes cannot simultaneously be trigger shapes and simulation shapes.");
		return;
	}

	const PxShapeFlags oldFlags = mShape.getFlags();

	const bool oldIsSimShape = oldFlags & PxShapeFlag::eSIMULATION_SHAPE;
	const bool isSimShape = inFlags & PxShapeFlag::eSIMULATION_SHAPE;

	if(mActor)
	{
		const PxType type = mActor->getConcreteType();

		// PT: US5732 - support kinematic meshes
		bool isKinematic = false;
		if(type==PxConcreteType::eRIGID_DYNAMIC)
		{
			PxRigidDynamic* rigidDynamic = static_cast<PxRigidDynamic*>(mActor);
			isKinematic = rigidDynamic->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC;
		}

		if((type != PxConcreteType::eRIGID_STATIC) && !isKinematic && isSimShape && !oldIsSimShape && (hasMeshTypeGeom || mShape.getGeometryType() == PxGeometryType::ePLANE))
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"NpShape::setFlag(s): triangle mesh, heightfield and plane shapes can only be simulation shapes if part of a PxRigidStatic!");
			return;
		}
	}

	const bool oldHasSceneQuery = oldFlags & PxShapeFlag::eSCENE_QUERY_SHAPE;
	const bool hasSceneQuery = inFlags & PxShapeFlag::eSCENE_QUERY_SHAPE;

	mShape.setFlags(inFlags);

	if(oldHasSceneQuery != hasSceneQuery && mActor)
	{
		NpScene* npScene = getAPIScene();
		if(npScene)
		{
			if(hasSceneQuery)
				NpActor::getShapeManager(*mActor).setupSceneQuery(npScene->getSceneQueryManagerFast(), *mActor, *this);
			else
				NpActor::getShapeManager(*mActor).teardownSceneQuery(npScene->getSceneQueryManagerFast(), *this);
		}
	}
}


void NpShape::setFlag(PxShapeFlag::Enum flag, bool value)
{
	NP_WRITE_CHECK(getOwnerScene());

	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setFlag: shared shapes attached to actors are not writable.");

	PxShapeFlags shapeFlags = mShape.getFlags();
	shapeFlags = value ? shapeFlags | flag : shapeFlags & ~flag;
	
	setFlagsInternal(shapeFlags);

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetFlags, static_cast<PxShape *>(this), (PxU32)shapeFlags);
}

void NpShape::setFlags( PxShapeFlags inFlags )
{
	NP_WRITE_CHECK(getOwnerScene());

	setFlagsInternal(inFlags);

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetFlags, static_cast<PxShape *>(this), (PxU32)inFlags);
}


PxShapeFlags NpShape::getFlags() const
{
	NP_READ_CHECK(getOwnerScene());
	return mShape.getFlags();
}


bool NpShape::isExclusive() const
{
	NP_READ_CHECK(getOwnerScene());
	return mInternalShapeFlags & NpInternalShapeFlag::eEXCLUSIVE;
}


void NpShape::setName(const char* debugName)		
{
	NP_WRITE_CHECK(getOwnerScene());
	PX_CHECK_AND_RETURN(isWritable(), "NpShape::setName: shared shapes attached to actors are not writable.");

	mName = debugName;

	GRB_EVENT(getOwnerScene(), GrbInteropEvent3, GrbInteropEvent3::PxShapeSetName, static_cast<PxShape *>(this), debugName);

#if PX_SUPPORT_VISUAL_DEBUGGER
	Scb::Scene* scbScene = mShape.getScbSceneForAPI();
	if(scbScene && scbScene->getSceneVisualDebugger().isConnected(true) )
		scbScene->getSceneVisualDebugger().updatePvdProperties(&mShape);
#endif
}


const char* NpShape::getName() const
{
	NP_READ_CHECK(getOwnerScene());

	return mName;
}


NpScene* NpShape::getOwnerScene()	const 
{	
	return mActor ? NpActor::getOwnerScene(*mActor) : NULL; 
}



NpScene* NpShape::getAPIScene()	const 
{	
	// gets called when we update SQ structures due to a write - in which case there must be an actor 
	PX_ASSERT(mActor);
	return NpActor::getAPIScene(*mActor);
}

///////////////////////////////////////////////////////////////////////////////

namespace physx
{
Sc::RigidCore* NpShapeGetScRigidObjectFromScbSLOW(const Scb::Shape &scb)
{
	
	const size_t offset = size_t(&(reinterpret_cast<NpShape*>(0)->getScbShape()));
	const NpShape* np = reinterpret_cast<const NpShape*>(reinterpret_cast<const char*>(&scb)-offset);
	
	return np->NpShape::getActor() ? &np->getScRigidObjectExclusive() : NULL;
}

size_t NpShapeGetScPtrOffset()
{
	const size_t offset = size_t(&(reinterpret_cast<NpShape*>(0)->getScbShape().getScShape()));	
	return offset;
}

void NpShapeIncRefCount(Scb::Shape& scb)
{
	const size_t offset = size_t(&(reinterpret_cast<NpShape*>(0)->getScbShape()));
	NpShape* np = reinterpret_cast<NpShape*>(reinterpret_cast<char*>(&scb)-offset);
	np->incRefCount();
}

void NpShapeDecRefCount(Scb::Shape& scb)
{
	const size_t offset = size_t(&(reinterpret_cast<NpShape*>(0)->getScbShape()));
	NpShape* np = reinterpret_cast<NpShape*>(reinterpret_cast<char*>(&scb)-offset);
	np->decRefCount();
}
}


// see NpConvexMesh.h, NpHeightField.h, NpTriangleMesh.h for details on how ref counting works
// for meshes
Cm::RefCountable* NpShape::getMeshRefCountable()
{
	switch(mShape.getGeometryType())
	{
		case PxGeometryType::eCONVEXMESH:
			return static_cast<Gu::ConvexMesh*>(
				static_cast<const PxConvexMeshGeometry&>(mShape.getGeometry()).convexMesh);

		case PxGeometryType::eHEIGHTFIELD:
			return static_cast<Gu::HeightField*>(
				static_cast<const PxHeightFieldGeometry&>(mShape.getGeometry()).heightField);

		case PxGeometryType::eTRIANGLEMESH:
			return static_cast<Gu::TriangleMesh*>(
				static_cast<const PxTriangleMeshGeometry&>(mShape.getGeometry()).triangleMesh);
		
		case PxGeometryType::eSPHERE:
		case PxGeometryType::ePLANE:
		case PxGeometryType::eCAPSULE:
		case PxGeometryType::eBOX:
		case PxGeometryType::eGEOMETRY_COUNT:
		case PxGeometryType::eINVALID:
		default:
			break;
	}
	return NULL;
}

bool NpShape::isWritable()
{
	// a shape is writable if it's exclusive, or it's not connected to any actors (which is true if the ref count is 1 and the user ref is not released.)
	return isExclusive() || (getRefCount()==1 && (mBaseFlags & PxBaseFlag::eIS_RELEASABLE));
}


void NpShape::incMeshRefCount()
{
	Cm::RefCountable* npMesh = getMeshRefCountable();
	if(npMesh)
		npMesh->incRefCount();
}

void NpShape::decMeshRefCount()
{
	Cm::RefCountable* npMesh = getMeshRefCountable();
	if(npMesh)
		npMesh->decRefCount();
}


bool NpShape::checkMaterialSetup(const PxGeometry& geom, const char* errorMsgPrefix, PxMaterial*const* materials, PxU16 materialCount)
{
	for(PxU32 i=0; i<materialCount; ++i)
	{
		if(!materials[i])
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
					"material pointer %d is NULL!", i);
			return false;
		}
	}

	// check that simple shapes don't get assigned multiple materials
	if (materialCount > 1 && (geom.getType() != PxGeometryType::eHEIGHTFIELD) && (geom.getType() != PxGeometryType::eTRIANGLEMESH))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
			"%s: multiple materials defined for single material geometry!", errorMsgPrefix);
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////


#if PX_ENABLE_DEBUG_VISUALIZATION
#include "GuDebug.h"

static Cm::Matrix34 getScaledMatrix(const PxTransform& absPose, const PxMeshScale& scale)
{
	const PxMat33 rotQ = PxMat33(absPose.q) * scale.toMat33();
	return Cm::Matrix34(rotQ, absPose.p);
}

void NpShape::visualize(Cm::RenderOutput& out, const PxRigidActor& actor)
{
	NpScene* npScene = NpActor::getOwnerScene(actor);
	PX_ASSERT(npScene);

	const PxReal scale = npScene->getVisualizationParameter(PxVisualizationParameter::eSCALE);
	if(!scale) return;

	const PxTransform absPose = actor.getGlobalPose() * mShape.getShape2Actor();

	if(npScene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS))
		out << PxU32(PxDebugColor::eARGB_YELLOW) << PxMat44(PxIdentity) << Cm::DebugBox(mShape.getGeometryUnion().computeBounds(absPose));

	const PxReal collisionAxes = scale * npScene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_AXES);
	if(collisionAxes != 0.0f)
		out << Gu::Debug::convertToPxMat44(absPose) << Cm::DebugBasis(PxVec3(collisionAxes), 0xcf0000, 0x00cf00, 0x0000cf);

	if(	npScene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES)		||
		npScene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS)	||
		npScene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_EDGES))
	{
		const PxBounds3& cullbox = npScene->getVisualizationCullingBox();

		const PxReal fscale = scale * npScene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_FNORMALS);

		// Pack bool params into bit mask.
		PxU64 mask = 0;

	#define SET_MASK(mask, param) (mask |= ((PxU64)!!npScene->getVisualizationParameter(param)) << param)

		SET_MASK(mask, PxVisualizationParameter::eCULL_BOX);
		SET_MASK(mask, PxVisualizationParameter::eCOLLISION_FNORMALS);
		SET_MASK(mask, PxVisualizationParameter::eCOLLISION_EDGES);
		SET_MASK(mask, PxVisualizationParameter::eCOLLISION_SHAPES);

		const Cm::Matrix34 pose34(absPose);

		Sc::ShapeCore& shape =  mShape.getScShape();
		const PxU32 numMaterials = shape.getNbMaterialIndices();

		const PxGeometryType::Enum type = getGeometryType();
		switch(type)
		{
			case PxGeometryType::eSPHERE:
			{
				PxSphereGeometry tmp;
				getSphereGeometry(tmp);
				Gu::Debug::visualize(tmp, out, pose34, cullbox, mask, fscale, numMaterials);
				break;
			}
			case PxGeometryType::ePLANE:
			{
				PxPlaneGeometry tmp;
				getPlaneGeometry(tmp);
				Gu::Debug::visualize(tmp, out, pose34, cullbox, mask, fscale, numMaterials);
				break;
			}
			case PxGeometryType::eCAPSULE:
			{
				PxCapsuleGeometry tmp;
				getCapsuleGeometry(tmp);
				Gu::Debug::visualize(tmp, out, pose34, cullbox, mask, fscale, numMaterials);
				break;
			}
			case PxGeometryType::eBOX:
			{
				PxBoxGeometry tmp;
				getBoxGeometry(tmp);
				Gu::Debug::visualize(tmp, out, pose34, cullbox, mask, fscale, numMaterials);
				break;
			}
			case PxGeometryType::eCONVEXMESH:
			{
				
				PxConvexMeshGeometry tmp;
				getConvexMeshGeometry(tmp);
				Gu::Debug::visualize(tmp, out, getScaledMatrix(absPose, tmp.scale), cullbox, mask, fscale, numMaterials);
				break;
			}
			case PxGeometryType::eTRIANGLEMESH :
			{
				PxTriangleMeshGeometry tmp;
				getTriangleMeshGeometry(tmp);
				Gu::Debug::visualize(tmp, out, getScaledMatrix(absPose, tmp.scale), cullbox, mask, fscale, numMaterials);
				break;
			}
			case PxGeometryType::eHEIGHTFIELD :
			{
				PxHeightFieldGeometry tmp;
				getHeightFieldGeometry(tmp);
				Gu::Debug::visualize(tmp, out, pose34, cullbox, mask, fscale, numMaterials);
				break;
			}
			case PxGeometryType::eGEOMETRY_COUNT:
			case PxGeometryType::eINVALID:
			default:
				break;
		}
	}
}

#endif  // PX_ENABLE_DEBUG_VISUALIZATION
