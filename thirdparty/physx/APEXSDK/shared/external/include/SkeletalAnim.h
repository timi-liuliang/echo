/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SKELETAL_ANIM
#define SKELETAL_ANIM

#include <FastXml.h>
#include <vector>
#include "TriangleMesh.h"

#include "NxApexDefs.h"
#if NX_SDK_VERSION_MAJOR == 2
class NxActor;
class NxScene;
#endif

namespace MESHIMPORT
{
};

namespace physx
{
namespace apex
{
class NxApexRenderDebug;
class NxRenderMeshAssetAuthoring;
}
}

namespace mimp
{
	class MeshSystemContainer;
};

namespace Samples
{

class TriangleMesh;

// ---------------------------------------------------------------------------
struct SkeletalBone
{
	void clear();

	std::string name;
	int id;
	physx::PxMat34Legacy pose;
	physx::PxVec3 scale;
	int parent;
	int firstChild;
	int numChildren;
	int firstVertex;

	physx::PxMat34Legacy bindWorldPose;
	physx::PxMat34Legacy invBindWorldPose;
	physx::PxMat34Legacy currentWorldPose;
	int boneOption;
	float inflateConvex;
	float minimalBoneWeight;
	int numShapes;
	bool selected;
	bool isRoot; // this is used for localspace sim
	bool isRootLock; // this is used to lock the translation of the rootbone
	bool allowPrimitives;
	bool dirtyParams;
	bool manualShapes;
};

struct BoneKeyFrame
{
	void clear();
	physx::PxMat34Legacy relPose;
	float time;
	physx::PxVec3 scale;
};

struct BoneTrack
{
	void clear();
	int firstFrame;
	int numFrames;
};

struct SkeletalAnimation
{
	void clear();
	std::string name;
	std::vector<BoneTrack> mBoneTracks;
	float minTime;
	float maxTime;
};

// ---------------------------------------------------------------------------
class SkeletalAnim : public FAST_XML::FastXml::Callback
{
public:
	SkeletalAnim();
	virtual ~SkeletalAnim();

	void clear();
	void copyFrom(const SkeletalAnim& anim);

	bool loadFromXML(const std::string& xmlFile, std::string& error);
	bool saveToXML(const std::string& xmlFile) const;
	bool loadFromParent(const SkeletalAnim* parent);
	bool loadFromMeshImport(mimp::MeshSystemContainer* msc, std::string& error, bool onlyAddAnimation);
	bool saveToMeshImport(mimp::MeshSystemContainer* msc);
	bool initFrom(physx::apex::NxRenderMeshAssetAuthoring& rma);

	void setBindPose();
	void setAnimPose(int animNr, float time, bool lockRootbone = false);
	const std::vector<SkeletalBone> &getBones() const
	{
		return mBones;
	}
	void setBoneCollision(physx::PxU32 boneNr, int option);
	void setBoneSelected(physx::PxU32 boneNr, bool selected)
	{
		mBones[boneNr].selected = selected;
	}
	void setBoneRoot(physx::PxU32 boneNr, bool isRoot)
	{
		mBones[boneNr].isRoot = isRoot;
	}
	void setBoneAllowPrimitives(physx::PxU32 boneNr, bool on)
	{
		mBones[boneNr].dirtyParams |= mBones[boneNr].allowPrimitives != on;
		mBones[boneNr].allowPrimitives = on;
	}
	void setBoneInflation(physx::PxU32 boneNr, float value)
	{
		mBones[boneNr].inflateConvex = value;
	}
	void setBoneMinimalWeight(physx::PxU32 boneNr, float value)
	{
		mBones[boneNr].dirtyParams |= mBones[boneNr].minimalBoneWeight != value;
		mBones[boneNr].minimalBoneWeight = value;
	}
	void setBoneDirty(physx::PxU32 boneNr, bool on)
	{
		mBones[boneNr].dirtyParams = on;
	}
	void setBoneManualShapes(physx::PxU32 boneNr, bool on)
	{
		mBones[boneNr].manualShapes = on;
	}
	const std::vector<int> &getChildren() const
	{
		return mChildren;
	}
	const std::vector<physx::PxMat44>& getSkinningMatrices() const
	{
		return mSkinningMatrices;
	}
	const std::vector<physx::PxMat44>& getSkinningMatricesWorld() const
	{
		return mSkinningMatricesWorld;
	}
	const std::vector<SkeletalAnimation*> &getAnimations() const
	{
		if (mParent != NULL)
		{
			return mParent->getAnimations();
		}
		return mAnimations;
	}

	void draw(physx::NxApexRenderDebug* batcher);

	void clearShapeCount(int boneIndex = -1);
	void incShapeCount(int boneIndex);
	void decShapeCount(int boneIndex);

	void setRagdoll(bool on);

	virtual bool processElement(const char* elementName, int argc, const char** argv, const char* elementData, int lineno);
	virtual bool processComment(const char* comment) // encountered a comment in the XML
	{
		PX_UNUSED(comment);
		return true;
	}

	virtual bool processClose(const char* element, physx::PxU32 depth, bool& isError)	 // process the 'close' indicator for a previously encountered element
	{
		PX_UNUSED(element);
		PX_UNUSED(depth);
		isError = false;
		return true;
	}

	virtual void*   fastxml_malloc(physx::PxU32 size)
	{
		return ::malloc(size);
	}
	virtual void	fastxml_free(void* mem)
	{
		::free(mem);
	}

private:
	void init(bool firstTime);
	void initBindPoses(int boneNr, const physx::PxVec3& scale);
	void setAnimPoseRec(int animNr, int boneNr, float time, bool lockBoneTranslation);

	void interpolateBonePose(int animNr, int boneNr, float time, physx::PxMat34Legacy& pose, physx::PxVec3& scale);
	int  findBone(const std::string& name);
	void setupConnectivity();

	// skeleton
	std::vector<SkeletalBone> mBones;
	std::vector<physx::PxMat44> mSkinningMatrices;
	std::vector<physx::PxMat44> mSkinningMatricesWorld;
	std::vector<int> mChildren;

	// animation
	std::vector<SkeletalAnimation*> mAnimations;
	std::vector<BoneKeyFrame> mKeyFrames;

	const SkeletalAnim* mParent;

	bool ragdollMode;
};

} // namespace Samples

#endif
