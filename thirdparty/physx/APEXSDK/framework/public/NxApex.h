/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_H
#define NX_APEX_H

/**
\file
\brief The top level include file for all of the APEX API.

Include this whenever you want to use anything from the APEX API
in a source file.
*/

#include "foundation/Px.h"

#include "NxApexUsingNamespace.h"


namespace NxParameterized
{
class Traits;
class Interface;
class Serializer;
};


#include "foundation/PxPreprocessor.h"
#include "foundation/PxSimpleTypes.h"
#include "foundation/PxAssert.h"
#include "PxFileBuf.h"
#include "foundation/PxBounds3.h"
#include "foundation/PxVec2.h"
#include "foundation/PxVec3.h"


// APEX public API:
// In general, APEX public headers will not be included 'alone', so they
// should not include their prerequisites.

#include "NxApexDefs.h"
#include "NxApexDesc.h"
#include "NxApexInterface.h"
#include "NxApexSDK.h"

#include "NxApexActor.h"
#include "NxApexContext.h"
#include "NxApexNameSpace.h"
#include "NxApexPhysXObjectDesc.h"
#include "NxApexRenderDataProvider.h"
#include "NxApexRenderable.h"
#include "NxApexAssetPreview.h"
#include "NxApexAsset.h"
#include "NxApexRenderContext.h"
#include "NxApexScene.h"
#include "NxApexSDKCachedData.h"
#include "NxApexUserProgress.h"
#include "NxModule.h"
#include "NxInstancedObjectSimulationAsset.h"

#include "NxApexRenderDataFormat.h"
#include "NxApexRenderBufferData.h"
#include "NxUserRenderResourceManager.h"
#include "NxUserRenderVertexBufferDesc.h"
#include "NxUserRenderInstanceBufferDesc.h"
#include "NxUserRenderSpriteBufferDesc.h"
#include "NxUserRenderIndexBufferDesc.h"
#include "NxUserRenderBoneBufferDesc.h"
#include "NxUserRenderResourceDesc.h"
#include "NxUserRenderSurfaceBufferDesc.h"
#include "NxUserRenderSurfaceBuffer.h"
#include "NxUserRenderResource.h"
#include "NxUserRenderVertexBuffer.h"
#include "NxUserRenderInstanceBuffer.h"
#include "NxUserRenderSpriteBuffer.h"
#include "NxUserRenderIndexBuffer.h"
#include "NxUserRenderBoneBuffer.h"
#include "NxUserRenderer.h"

#include "NxVertexFormat.h"
#include "NxRenderMesh.h"
#include "NxRenderMeshActorDesc.h"
#include "NxRenderMeshActor.h"
#include "NxRenderMeshAsset.h"
#include "NxResourceCallback.h"
#include "NxResourceProvider.h"

#endif // NX_APEX_H
