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

#include "PxPhysXConfig.h"

#if PX_USE_CLOTH_API

#include "NpClothFabric.h"
#include "NpFactory.h"
#include "NpPhysics.h"

#include "GuSerialize.h"
#include "CmPhysXCommon.h"
#include "PxAssert.h"
#include "CmUtils.h"

using namespace physx;

void NpClothFabric::exportExtraData(PxSerializationContext& stream)
{
	mFabric.exportExtraData(stream);
}


void NpClothFabric::importExtraData(PxDeserializationContext& context)
{
	mFabric.importExtraData(context);	
}

NpClothFabric* NpClothFabric::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpClothFabric* obj = new (address) NpClothFabric(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpClothFabric);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION


NpClothFabric::NpClothFabric()
: PxClothFabric(PxConcreteType::eCLOTH_FABRIC, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
{
}


NpClothFabric::~NpClothFabric()
{
}


void NpClothFabric::release()
{
	NpPhysics::getInstance().notifyDeletionListenersUserRelease(this, NULL);

	mBaseFlags &= ~PxBaseFlag::eIS_RELEASABLE;
	decRefCount();
}


void NpClothFabric::onRefCountZero()
{
	if(NpFactory::getInstance().removeClothFabric(*this))
	{
		if(getBaseFlags() & PxBaseFlag::eOWNS_MEMORY)
		   NpFactory::getInstance().releaseClothFabricToPool(*this);
	    else
		    this->~NpClothFabric();
		NpPhysics::getInstance().notifyDeletionListenersMemRelease(this, NULL);
		return;
	}

	// PT: if we reach this point, we didn't find the cloth fabric in the Physics object => don't delete!
	// This prevents deleting the object twice.
	Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "NpClothFabric: double deletion detected!");
}


PxU32 NpClothFabric::getReferenceCount() const
{
	return getRefCount();
}


/**
   Load cloth fabric data from the given stream.

   @param[in] stream input stream to load fabric data from
   @return true if loading was successful

   @sa For cooker implementation, see ClothFabricBuilder.cpp in PhysXCooking/src
 */
bool NpClothFabric::load(PxInputStream& stream)
{
    return mFabric.load(stream);
}

bool NpClothFabric::load(const PxClothFabricDesc& desc)
{
	return mFabric.load(desc);
}

PxU32 NpClothFabric::getNbParticles() const
{
	return getScClothFabric().getNbParticles();
}

PxU32 NpClothFabric::getNbPhases() const
{
	return getScClothFabric().getNbPhases();
}

PxU32 NpClothFabric::getNbSets() const
{
    return getScClothFabric().getNbSets();
}

PxU32 NpClothFabric::getNbParticleIndices() const
{
    return getScClothFabric().getNbParticleIndices();
}

PxU32 NpClothFabric::getNbRestvalues() const
{
    return getScClothFabric().getNbRestvalues();
}

PxU32 NpClothFabric::getNbTethers() const
{
	return getScClothFabric().getNbTethers();
}

PxU32 NpClothFabric::getPhases(PxClothFabricPhase* userPhaseIndexBuffer, PxU32 bufferSize) const
{
	return getScClothFabric().getPhases(userPhaseIndexBuffer, bufferSize);
}

PxU32 NpClothFabric::getSets(PxU32* userSetBuffer, PxU32 bufferSize) const
{
    return getScClothFabric().getSets(userSetBuffer, bufferSize);
}

PxU32 NpClothFabric::getParticleIndices(PxU32* userParticleIndexBuffer, PxU32 bufferSize) const
{
    return getScClothFabric().getParticleIndices(userParticleIndexBuffer, bufferSize);
}

PxU32 NpClothFabric::getRestvalues(PxReal* userRestvalueBuffer, PxU32 bufferSize) const
{
    return getScClothFabric().getRestvalues(userRestvalueBuffer, bufferSize);
}

PxU32 NpClothFabric::getTetherAnchors(PxU32* userAnchorBuffer, PxU32 bufferSize) const
{
	return getScClothFabric().getTetherAnchors(userAnchorBuffer, bufferSize);
}

PxU32 NpClothFabric::getTetherLengths(PxReal* userLengthBuffer, PxU32 bufferSize) const
{
	return getScClothFabric().getTetherLengths(userLengthBuffer, bufferSize);
}

PxClothFabricPhaseType::Enum NpClothFabric::getPhaseType(PxU32 phaseIndex) const
{
    return getScClothFabric().getPhaseType(phaseIndex);
}

void NpClothFabric::scaleRestlengths(PxReal scale)
{
	mFabric.scaleRestlengths(scale);
}

#endif // PX_USE_CLOTH_API

