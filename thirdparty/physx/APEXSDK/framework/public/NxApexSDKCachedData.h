/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_SDKCACHED_DATA_H
#define NX_APEX_SDKCACHED_DATA_H

/*!
\file
\brief classes NxApexModuleCachedData, NxApexSDKCachedData
*/

#include "NxApexSDK.h"

namespace NxParameterized
{
class Interface;
};

namespace physx
{
namespace apex
{


/**
\brief Cached data is stored per-module.
*/
class NxApexModuleCachedData
{
public:
	/**
	 * \brief Retreives the cached data for the asset, if it exists.
	 *
	 * Otherwise returns NULL.
	 */
	virtual ::NxParameterized::Interface*	getCachedDataForAssetAtScale(NxApexAsset& asset, const physx::PxVec3& scale) = 0;

	/**	
	 *	\brief Serializes the cooked data for a single asset into a stream.
	 */
	virtual physx::general_PxIOStream2::PxFileBuf& serializeSingleAsset(NxApexAsset& asset, physx::general_PxIOStream2::PxFileBuf& stream) = 0;

	/**	
	 *	\brief Deserializes the cooked data for a single asset from a stream.
	 */
	virtual physx::general_PxIOStream2::PxFileBuf& deserializeSingleAsset(NxApexAsset& asset, physx::general_PxIOStream2::PxFileBuf& stream) = 0;
};

/**
\brief A method for storing actor data in a scene
*/
class NxApexSDKCachedData
{
public:
	/**
	 * \brief Retreives the scene cached data for the actor, if it exists.
	 *
	 * Otherwise returns NULL.
	 */
	virtual NxApexModuleCachedData*	getCacheForModule(NxAuthObjTypeID moduleID) = 0;

	/**
	 * \brief Save cache configuration to a stream
	 */
	virtual physx::general_PxIOStream2::PxFileBuf&  serialize(physx::general_PxIOStream2::PxFileBuf&) const = 0;

	/**
	 * \brief Load cache configuration from a stream
	 */
	virtual physx::general_PxIOStream2::PxFileBuf&  deserialize(physx::general_PxIOStream2::PxFileBuf&) = 0;

	/**
	 * \brief Clear data
	 */
	virtual void					clear(bool force = true) = 0;

protected:
	NxApexSDKCachedData() {}
	virtual							~NxApexSDKCachedData() {}
};

}
} // end namespace physx::apex


#endif // NX_APEX_SDKCACHED_DATA_H
