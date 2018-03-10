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


#ifndef PX_PHYSICS_SCP_MATERIAL_CORE
#define PX_PHYSICS_SCP_MATERIAL_CORE

#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PxMaterial.h"
#include "PxVec3.h"
#include "PxsMaterialCore.h"

namespace physx
{

class PxMaterial;

namespace Sc
{

typedef	PxsMaterialData	MaterialData;

class MaterialCore : public PxsMaterialCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
											MaterialCore(const MaterialData& desc);
											MaterialCore(const PxEMPTY&) : PxsMaterialCore(PxEmpty) {}
											MaterialCore(){}
											~MaterialCore();
	static			void					getBinaryMetaData(PxOutputStream& stream);

	PX_FORCE_INLINE	void					save(MaterialData& data)		const			{ data = *this;			}
	PX_FORCE_INLINE	void					load(const MaterialData& data)					{ static_cast<MaterialData&>(*this) = data; }	// To make synchronization between master material and scene material table less painful

};

} // namespace Sc

}

#endif
