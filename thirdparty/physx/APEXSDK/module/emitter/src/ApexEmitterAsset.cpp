/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApex.h"
#include "ModuleEmitter.h"
#include "ApexEmitterAsset.h"
#include "ApexEmitterActor.h"
//#include "ApexSharedSerialization.h"

#include "EmitterGeomBox.h"
#include "EmitterGeomSphere.h"
#include "EmitterGeomSphereShell.h"
#include "EmitterGeomCylinder.h"
#include "EmitterGeomExplicit.h"
#include "ApexEmitterAssetPreview.h"

#include "EmitterGeomBoxParams.h"
#include "EmitterGeomSphereParams.h"
#include "EmitterGeomSphereShellParams.h"
#include "EmitterGeomCylinderParams.h"
#include "EmitterGeomExplicitParams.h"

#include "NxIofxAsset.h"

namespace physx
{
namespace apex
{
namespace emitter
{


struct ApexEmitterGeomTypes
{
	enum Enum
	{
		GEOM_GROUND = 0,
		GEOM_BOX,
		GEOM_SPHERE,
		GEOM_SPHERE_SHELL,
		GEOM_EXLICIT,
	};
};

void ApexEmitterAsset::copyLodDesc2(NxEmitterLodParamDesc& dst, const ApexEmitterAssetParametersNS::emitterLodParamDesc_Type& src)
{
	PX_ASSERT(src.version == dst.current);

	dst.bias			= src.bias;
	dst.distanceWeight	= src.distanceWeight;
	dst.lifeWeight		= src.lifeWeight;
	dst.maxDistance		= src.maxDistance;
	dst.separationWeight = src.separationWeight;
	dst.speedWeight		= src.speedWeight;
}

void ApexEmitterAsset::copyLodDesc2(ApexEmitterAssetParametersNS::emitterLodParamDesc_Type& dst, const NxEmitterLodParamDesc& src)
{
	dst.version			= src.current;
	dst.bias			= src.bias;
	dst.distanceWeight	= src.distanceWeight;
	dst.lifeWeight		= src.lifeWeight;
	dst.maxDistance		= src.maxDistance;
	dst.separationWeight = src.separationWeight;
	dst.speedWeight		= src.speedWeight;
}



void ApexEmitterAsset::postDeserialize(void* userData)
{
	PX_UNUSED(userData);

	ApexSimpleString tmpStr;
	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr;

	if (mParams->iofxAssetName == NULL)
	{
		NxParameterized::Handle h(mParams);
		h.getParameter("iofxAssetName");
		h.initParamRef(h.parameterDefinition()->refVariantVal(0), true);
	}

	if (mParams->iosAssetName == NULL)
	{
		NxParameterized::Handle h(mParams);
		h.getParameter("iosAssetName");
		h.initParamRef(h.parameterDefinition()->refVariantVal(0), true);
	}

	if (mParams->geometryType == NULL)
	{
		NxParameterized::Handle h(mParams);
		h.getParameter("geometryType");
		h.initParamRef(h.parameterDefinition()->refVariantVal(0), true);
	}


	ApexSimpleString iofxName(mParams->iofxAssetName->name());
	ApexSimpleString iosName(mParams->iosAssetName->name());
	ApexSimpleString iosTypeName(mParams->iosAssetName->className());

	if (!iofxName.len() || !iosName.len() || !iosTypeName.len())
	{
		APEX_INTERNAL_ERROR("IOFX, IOS, or IOS type not initialized");
		return;
	}

	if (mGeom)
	{
		mGeom->destroy();
	}

	mParams->getParameterHandle("geometryType", h);
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (!refPtr)
	{
		APEX_INTERNAL_ERROR("No emitter geometry specified");
		return;
	}

	tmpStr = refPtr->className();

	if (tmpStr == EmitterGeomBoxParams::staticClassName())
	{
		EmitterGeomBox* exp = PX_NEW(EmitterGeomBox)(refPtr);
		mGeom = exp;
	}
	else if (tmpStr == EmitterGeomSphereParams::staticClassName())
	{
		EmitterGeomSphere* exp = PX_NEW(EmitterGeomSphere)(refPtr);
		mGeom = exp;
	}
	else if (tmpStr == EmitterGeomSphereShellParams::staticClassName())
	{
		EmitterGeomSphereShell* exp = PX_NEW(EmitterGeomSphereShell)(refPtr);
		mGeom = exp;
	}
	else if (tmpStr == EmitterGeomCylinderParams::staticClassName())
	{
		EmitterGeomCylinder* exp = PX_NEW(EmitterGeomCylinder)(refPtr);
		mGeom = exp;
	}
	else if (tmpStr == EmitterGeomExplicitParams::staticClassName())
	{
		EmitterGeomExplicit* exp = PX_NEW(EmitterGeomExplicit)(refPtr);
		mGeom = exp;
	}
	else
	{
		PX_ASSERT(0 && "Invalid geometry type for APEX emitter");
		return;
	}

	copyLodDesc2(mLodDesc, mParams->lodParamDesc);

	initializeAssetNameTable();
}


void ApexEmitterAsset::initializeAssetNameTable()
{
	// clean up asset tracker list
	mIosAssetTracker.removeAllAssetNames();
	mIofxAssetTracker.removeAllAssetNames();

	mIosAssetTracker.addAssetName(mParams->iosAssetName->className(),
	                              mParams->iosAssetName->name());

	mIofxAssetTracker.addAssetName(mParams->iofxAssetName->name(), false);

	NxParamArray<NxVec2R> cp(mParams, "rateVsTimeCurvePoints", (NxParamDynamicArrayStruct*)&mParams->rateVsTimeCurvePoints);
	for (physx::PxU32 i = 0; i < cp.size(); i++)
	{
		mRateVsTimeCurve.addControlPoint(cp[i]);
	}
}

ApexEmitterAsset::ApexEmitterAsset(ModuleEmitter* module, NxResourceList& list, const char* name):
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL),
	mModule(module),
	mName(name),
	mGeom(NULL),
	mIofxAssetTracker(module->mSdk, NX_IOFX_AUTHORING_TYPE_NAME),
	mIosAssetTracker(module->mSdk)
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = static_cast<ApexEmitterAssetParameters*>(traits->createNxParameterized(ApexEmitterAssetParameters::staticClassName()));

	PX_ASSERT(mParams);

	mParams->setSerializationCallback(this);
	list.add(*this);
}

ApexEmitterAsset::ApexEmitterAsset(ModuleEmitter* module,
                                   NxResourceList& list,
                                   NxParameterized::Interface* params,
                                   const char* name) :
	mParams(static_cast<ApexEmitterAssetParameters*>(params)),
	mDefaultActorParams(NULL),
	mDefaultPreviewParams(NULL),
	mModule(module),
	mName(name),
	mGeom(NULL),
	mIofxAssetTracker(module->mSdk, NX_IOFX_AUTHORING_TYPE_NAME),
	mIosAssetTracker(module->mSdk)
{
	// this may no longer make any sense
	mParams->setSerializationCallback(this);

	// call this now to "initialize" the asset
	postDeserialize();

	list.add(*this);
}

ApexEmitterAsset::~ApexEmitterAsset()
{
}

void ApexEmitterAsset::release()
{
	mModule->mSdk->releaseAsset(*this);
}

void ApexEmitterAsset::destroy()
{
	/* Assets that were forceloaded or loaded by actors will be automatically
	 * released by the ApexAssetTracker member destructors.
	 */

	if (mGeom)
	{
		mGeom->destroy();
	}
	mGeom = NULL;

	if (mDefaultActorParams)
	{
		mDefaultActorParams->destroy();
		mDefaultActorParams = 0;
	}

	if (mDefaultPreviewParams)
	{
		mDefaultPreviewParams->destroy();
		mDefaultPreviewParams = 0;
	}

	if (mParams)
	{
		mParams->destroy();
		mParams = NULL;
	}

	/* Actors are automatically cleaned up on deletion by NxResourceList dtor */
	delete this;
}

NxEmitterExplicitGeom* ApexEmitterAsset::isExplicitGeom()
{
	NX_READ_ZONE();
	return const_cast<NxEmitterExplicitGeom*>(mGeom->getNxEmitterGeom()->isExplicitGeom());
}

const NxEmitterExplicitGeom* ApexEmitterAsset::isExplicitGeom() const
{
	return mGeom->getNxEmitterGeom()->isExplicitGeom();
}

NxParameterized::Interface* ApexEmitterAsset::getDefaultActorDesc()
{
	NX_WRITE_ZONE();
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultActorParams)
	{
		const char* className = ApexEmitterActorParameters::staticClassName();
		NxParameterized::Interface* param = traits->createNxParameterized(className);
		mDefaultActorParams = static_cast<ApexEmitterActorParameters*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	if (mDefaultActorParams)
	{
		mDefaultActorParams->emitterDuration = mParams->emitterDuration;
	}

	return mDefaultActorParams;
}

NxParameterized::Interface* ApexEmitterAsset::getDefaultAssetPreviewDesc()
{
	NX_WRITE_ZONE();
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	PX_ASSERT(traits);
	if (!traits)
	{
		return NULL;
	}

	// create if not yet created
	if (!mDefaultPreviewParams)
	{
		const char* className = EmitterAssetPreviewParameters::staticClassName();
		NxParameterized::Interface* param = traits->createNxParameterized(className);
		mDefaultPreviewParams = static_cast<EmitterAssetPreviewParameters*>(param);

		PX_ASSERT(param);
		if (!param)
		{
			return NULL;
		}
	}

	return mDefaultPreviewParams;
}

NxApexActor* ApexEmitterAsset::createApexActor(const NxParameterized::Interface& parms, NxApexScene& apexScene)
{
	NX_WRITE_ZONE();
	if (!isValidForActorCreation(parms, apexScene))
	{
		return NULL;
	}

	NxApexActor* ret = 0;

	const char* className = parms.className();
	if (strcmp(className, ApexEmitterActorParameters::staticClassName()) == 0)
	{
		NxApexEmitterActorDesc desc;
		const ApexEmitterActorParameters* pDesc = static_cast<const ApexEmitterActorParameters*>(&parms);

		desc.attachRelativePose	= pDesc->attachRelativePose;
		desc.initialPose		= pDesc->initialPose;
		desc.emitAssetParticles	= pDesc->emitAssetParticles;
		desc.emitterDuration	= pDesc->emitterDuration;
		desc.initialScale		= pDesc->initialScale;

		/* Resolve the authored collision group mask name into the actual ID */
		if (pDesc->overlapTestGroupMaskName != NULL &&
		        pDesc->overlapTestGroupMaskName[0] != 0)
		{
			NiResourceProvider* nrp = mModule->mSdk->getInternalResourceProvider();
			NxResID cgmns = mModule->mSdk->getCollisionGroupMaskNameSpace();
			NxResID cgresid = nrp->createResource(cgmns, pDesc->overlapTestGroupMaskName);
			desc.overlapTestCollisionGroups = (physx::PxU32)(size_t) nrp->getResource(cgresid);
		}

		ret = createEmitterActor(desc, apexScene);

#if NX_SDK_VERSION_MAJOR == 2
		/* Resolve the authored collision group name into the actual ID */
		NxGroupsMask* raycastGroupsMask = 0;
		if (pDesc->overlapTestGroupsMask128Name != NULL &&
		        pDesc->overlapTestGroupsMask128Name[0] != 0)
		{
			NiResourceProvider* nrp = mModule->mSdk->getInternalResourceProvider();
			NxResID cgmns = mModule->mSdk->getCollisionGroup128NameSpace();
			NxResID cgresid = nrp->createResource(cgmns, pDesc->overlapTestGroupsMask128Name);
			raycastGroupsMask = static_cast<NxGroupsMask*>(nrp->getResource(cgresid));
		}

		// check the NxGroupsMask specified in parms, set in actor if diff than default
		if (raycastGroupsMask)
		{
			NxApexEmitterActor* ea = static_cast<NxApexEmitterActor*>(ret);
			ea->setOverlapTestCollisionGroupsMask(raycastGroupsMask);
		}
#endif
	}

	return ret;
}



NxApexAssetPreview* ApexEmitterAsset::createApexAssetPreview(const NxParameterized::Interface& parms, NxApexAssetPreviewScene* previewScene)
{
	NX_WRITE_ZONE();
	NxApexAssetPreview* ret = 0;

	const char* className = parms.className();
	if (strcmp(className, EmitterAssetPreviewParameters::staticClassName()) == 0)
	{
		NxApexEmitterPreviewDesc desc;
		const EmitterAssetPreviewParameters* pDesc = static_cast<const EmitterAssetPreviewParameters*>(&parms);

		desc.mPose	= pDesc->pose;
		desc.mScale = pDesc->scale;

		ret = createEmitterPreview(desc, previewScene);
	}

	return ret;
}


NxApexEmitterActor* ApexEmitterAsset::createEmitterActor(const NxApexEmitterActorDesc& desc, const NxApexScene& scene)
{
	if (!desc.isValid())
	{
		return NULL;
	}

	EmitterScene* es = mModule->getEmitterScene(scene);
	ApexEmitterActor* actor = PX_NEW(ApexEmitterActor)(desc, *this, mEmitterActors, *es);
	if (!actor->isValid())
	{
		actor->destroy();
		return NULL;
	}
	return actor;
}

void ApexEmitterAsset::releaseEmitterActor(NxApexEmitterActor& nxactor)
{
	ApexEmitterActor* actor = DYNAMIC_CAST(ApexEmitterActor*)(&nxactor);
	actor->destroy();
}

NxApexEmitterPreview* ApexEmitterAsset::createEmitterPreview(const NxApexEmitterPreviewDesc& desc, NxApexAssetPreviewScene* previewScene)
{
	if (!desc.isValid())
	{
		return NULL;
	}

	ApexEmitterAssetPreview* p = PX_NEW(ApexEmitterAssetPreview)(desc, *this, previewScene, NxGetApexSDK());
	if (p && !p->isValid())
	{
		p->destroy();
		p = NULL;
	}
	return p;
}

void ApexEmitterAsset::releaseEmitterPreview(NxApexEmitterPreview& nxpreview)
{
	ApexEmitterAssetPreview* preview = DYNAMIC_CAST(ApexEmitterAssetPreview*)(&nxpreview);
	preview->destroy();
}


physx::PxU32 ApexEmitterAsset::forceLoadAssets()
{
	physx::PxU32 assetLoadedCount = 0;

	assetLoadedCount += mIofxAssetTracker.forceLoadAssets();
	assetLoadedCount += mIosAssetTracker.forceLoadAssets();

	return assetLoadedCount;
}

bool ApexEmitterAsset::isValidForActorCreation(const ::NxParameterized::Interface& /*actorParams*/, NxApexScene& /*apexScene*/) const
{
	NX_READ_ZONE();
	if (!mGeom)
	{
		return false;
	}

	const NxEmitterExplicitGeom* explicitGeom = isExplicitGeom();
	if (explicitGeom)
	{
		// Velocity array must be same size as positions or 0

		const NxEmitterExplicitGeom::PointParams* points;
		const physx::PxVec3* velocities;
		physx::PxU32 numPoints, numVelocities;
		explicitGeom->getParticleList(points, numPoints, velocities, numVelocities);
		if (!(numPoints == numVelocities || 0 == numVelocities))
		{
			return false;
		}

		const NxEmitterExplicitGeom::SphereParams* spheres;
		physx::PxU32 numSpheres;
		explicitGeom->getSphereList(spheres, numSpheres, velocities, numVelocities);
		if (!(numSpheres == numVelocities || 0 == numVelocities))
		{
			return false;
		}

		const NxEmitterExplicitGeom::EllipsoidParams* ellipsoids;
		physx::PxU32 numEllipsoids;
		explicitGeom->getEllipsoidList(ellipsoids, numEllipsoids, velocities, numVelocities);
		if (!(numEllipsoids == numVelocities || 0 == numVelocities))
		{
			return false;
		}

		// Radiuses are > 0

		for (physx::PxU32 i = 0; i < numSpheres; ++i)
			if (explicitGeom->getSphereRadius(i) <= 0)
			{
				return false;
			}

		for (physx::PxU32 i = 0; i < numEllipsoids; ++i)
			if (explicitGeom->getEllipsoidRadius(i) <= 0)
			{
				return false;
			}

		/*		// Normals are normalized

				for(physx::PxU32 i = 0; i < numEllipsoids; ++i)
					if( !explicitGeom->getEllipsoidNormal(i).isNormalized() )
						return false;
		*/
		// Distance >= 0

		if (explicitGeom->getDistance() < 0)
		{
			return false;
		}

		// Distance > 0 if we have shapes

		if ((numSpheres || numEllipsoids) && !explicitGeom->getDistance())
		{
			return false;
		}
	}

	return true;
}

/*===============  Asset Authoring =================*/
#ifndef WITHOUT_APEX_AUTHORING
void ApexEmitterAssetAuthoring::setInstancedObjectEffectsAssetName(const char* iofxname)
{
	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr;

	mParams->getParameterHandle("iofxAssetName", h);
	mParams->initParamRef(h, NULL, true);
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (refPtr)
	{
		refPtr->setName(iofxname);
	}

}


void ApexEmitterAssetAuthoring::setInstancedObjectSimulatorAssetName(const char* iosname)
{
	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr;

	mParams->getParameterHandle("iosAssetName", h);
	mParams->initParamRef(h, NULL, true);
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (refPtr)
	{
		refPtr->setName(iosname);
	}
}


void ApexEmitterAssetAuthoring::setInstancedObjectSimulatorTypeName(const char* iostname)
{
	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr;

	mParams->getParameterHandle("iosAssetName", h);
	mParams->initParamRef(h, iostname, true);
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (refPtr)
	{
		refPtr->setClassName(iostname);
	}
}


void ApexEmitterAssetAuthoring::release()
{
	mModule->mSdk->releaseAssetAuthoring(*this);
}


NxEmitterBoxGeom* ApexEmitterAssetAuthoring::setBoxGeom()
{
	if (mGeom)
	{
		mGeom->destroy();
		if (mParams->geometryType)
		{
			mParams->geometryType->destroy();
			mParams->geometryType = NULL;
		}
	}

	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr = 0;
	mParams->getParameterHandle("geometryType", h);

	mParams->getParamRef(h, refPtr);
	if (refPtr)
	{
		refPtr->destroy();
		mParams->setParamRef(h, 0);
	}

	mParams->initParamRef(h, EmitterGeomBoxParams::staticClassName(), true);

	refPtr = 0;
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (!refPtr)
	{
		return NULL;
	}

	EmitterGeomBox* box = PX_NEW(EmitterGeomBox)(refPtr);
	mGeom = box;
	return box;
}

NxEmitterSphereGeom* ApexEmitterAssetAuthoring::setSphereGeom()
{
	if (mGeom)
	{
		mGeom->destroy();
		if (mParams->geometryType)
		{
			mParams->geometryType->destroy();
			mParams->geometryType = NULL;
		}
	}

	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr = 0;
	mParams->getParameterHandle("geometryType", h);

	mParams->getParamRef(h, refPtr);
	if (refPtr)
	{
		refPtr->destroy();
		mParams->setParamRef(h, 0);
	}

	mParams->initParamRef(h, EmitterGeomSphereParams::staticClassName(), true);

	refPtr = 0;
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (!refPtr)
	{
		return NULL;
	}

	EmitterGeomSphere* sphere = PX_NEW(EmitterGeomSphere)(refPtr);
	mGeom = sphere;
	return sphere;
}

NxEmitterSphereShellGeom* 	ApexEmitterAssetAuthoring::setSphereShellGeom()
{
	if (mGeom)
	{
		mGeom->destroy();
		if (mParams->geometryType)
		{
			mParams->geometryType->destroy();
			mParams->geometryType = NULL;
		}
	}

	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr = 0;
	mParams->getParameterHandle("geometryType", h);

	mParams->getParamRef(h, refPtr);
	if (refPtr)
	{
		refPtr->destroy();
		mParams->setParamRef(h, 0);
	}

	mParams->initParamRef(h, EmitterGeomSphereShellParams::staticClassName(), true);

	refPtr = 0;
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (!refPtr)
	{
		return NULL;
	}

	EmitterGeomSphereShell* sphereshell = PX_NEW(EmitterGeomSphereShell)(refPtr);
	mGeom = sphereshell;
	return sphereshell;
}

NxEmitterExplicitGeom* 	ApexEmitterAssetAuthoring::setExplicitGeom()
{
	if (mGeom)
	{
		mGeom->destroy();
		if (mParams->geometryType)
		{
			mParams->geometryType->destroy();
			mParams->geometryType = NULL;
		}
	}

	NxParameterized::Handle h(*mParams);
	NxParameterized::Interface* refPtr = 0;
	mParams->getParameterHandle("geometryType", h);

	mParams->getParamRef(h, refPtr);
	if (refPtr)
	{
		refPtr->destroy();
		mParams->setParamRef(h, 0);
	}

	mParams->initParamRef(h, EmitterGeomExplicitParams::staticClassName(), true);

	refPtr = 0;
	mParams->getParamRef(h, refPtr);
	PX_ASSERT(refPtr);
	if (!refPtr)
	{
		return NULL;
	}

	EmitterGeomExplicit* exp = PX_NEW(EmitterGeomExplicit)(refPtr);
	mGeom = exp;
	return exp;
}
#endif

}
}
} // namespace physx::apex
