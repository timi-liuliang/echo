/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_IMPACT_EMITTER_ASSET_H
#define NX_IMPACT_EMITTER_ASSET_H

#include "NxApex.h"

namespace physx
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

#define NX_IMPACT_EMITTER_AUTHORING_TYPE_NAME "ImpactEmitterAsset"

class NxImpactEmitterActor;
class NxEmitterLodParamDesc;

///Impact emitter asset class
class NxImpactEmitterAsset : public NxApexAsset
{
protected:
	PX_INLINE NxImpactEmitterAsset() {}
	virtual ~NxImpactEmitterAsset() {}

public:
	/// Get the setID that corresponds to the set name \sa NxImpactEmitterActor::registerImpact()
	virtual physx::PxU32			querySetID(const char* setName) = 0;

	/**
	\brief Get a list of the event set names.  The name index in the outSetNames list is the setID.
	\param [in,out] inOutSetNames An array of const char * with at least nameCount
	\param [in,out] nameCount The size of setNames as input and output, set to -1 if setIDs > nameCount
	*/
	virtual void					getSetNames(const char** inOutSetNames, physx::PxU32& nameCount) const = 0;
};

///Impact emitter asset authoring. Used to create Impact Emitter assets
class NxImpactEmitterAssetAuthoring : public NxApexAssetAuthoring
{
protected:
	virtual ~NxImpactEmitterAssetAuthoring() {}

public:
};


PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_IMPACT_EMITTER_ASSET_H
