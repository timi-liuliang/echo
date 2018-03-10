/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef RT_RENDERABLE_H
#define RT_RENDERABLE_H

#include "PsArray.h"
#include "PsUserAllocated.h"
#include "PxMat34Legacy.h"
#include "PxVec2.h"

namespace physx
{
namespace apex
{
	class NxUserRenderer;
	class NxUserRenderVertexBuffer;
	class NxUserRenderIndexBuffer;
	class NxUserRenderBoneBuffer;
	class NxUserRenderResource;
}
using namespace shdfnd;
using namespace apex;
namespace fracture
{

class Actor;
class Convex;

class Renderable : public ::physx::shdfnd::UserAllocated
{
public:
	Renderable();
	~Renderable();

	// Called by rendering thread
	void updateRenderResources(bool rewriteBuffers, void* userRenderData);
	void dispatchRenderResources(NxUserRenderer& api);

	// Per tick bone update, unless Actor is dirty
	void updateRenderCache(Actor* actor);

	// Returns the bounds of all of the convexes
	PxBounds3	getBounds() const;

private:
	// Called by actor after a patternFracture (On Game Thread)
	void updateRenderCacheFull(Actor* actor);

	// To Handle Multiple Materials
	struct SubMesh
	{
		SubMesh(): renderResource(NULL) {}

		Array<PxU32>			mIndexCache;
		NxUserRenderResource*	renderResource;
	};
	// To Handle Bone Limit
	struct ConvexGroup
	{
		Array<SubMesh>			mSubMeshes;
		Array<Convex*>			mConvexCache;
		Array<PxVec3>			mVertexCache;
		Array<PxVec3>			mNormalCache;
		Array<PxVec2>			mTexcoordCache;
		Array<PxU16>			mBoneIndexCache;
		Array<PxMat34Legacy>	mBoneCache;
	};
	// Shared by SubMeshes
	struct MaterialInfo
	{
		MaterialInfo(): mMaxBones(0), mMaterialID(0) {}

		PxU32		mMaxBones;
		NxResID		mMaterialID;
	};
	//
	Array<ConvexGroup>	mConvexGroups;
	Array<MaterialInfo> mMaterialInfo;

	NxUserRenderVertexBuffer*	mVertexBuffer;
	NxUserRenderIndexBuffer*	mIndexBuffer;
	NxUserRenderBoneBuffer*		mBoneBuffer;
	PxU32						mVertexBufferSize;
	PxU32						mIndexBufferSize;
	PxU32						mBoneBufferSize;
	PxU32						mVertexBufferSizeLast;
	PxU32						mIndexBufferSizeLast;
	PxU32						mBoneBufferSizeLast;
	bool						mFullBufferDirty;
	bool valid;
};

}
}

#endif
#endif