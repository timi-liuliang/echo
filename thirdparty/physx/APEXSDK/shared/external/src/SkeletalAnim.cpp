/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <PsShare.h>
#include <PsMathUtils.h>
#include <PxFileBuffer.h>
#include <MeshImport.h>
#include <AutoGeometry.h>
#include <PsFile.h>
#include <PsString.h>

#include "SkeletalAnim.h"
#include "TriangleMesh.h"
#include "NxFromPx.h"
#include <NxApexRenderDebug.h>
#include <algorithm>	// for std::max

namespace Samples
{

// -------------------------------------------------------------------
void SkeletalBone::clear()
{
	name = "";
	id = -1;
	pose.id();
	bindWorldPose.id();
	invBindWorldPose.id();
	currentWorldPose.id();

	scale = physx::PxVec3(1.0f, 1.0f, 1.0f);
	parent = -1;
	firstChild = -1;
	numChildren = 0;
	firstVertex = -1;
	boneOption = 0;
	inflateConvex = 0.0f;
	minimalBoneWeight = 0.4f;
	numShapes = 0;
	selected = false;
	allowPrimitives = true;
	dirtyParams = false;
	manualShapes = false;
	isRoot = false;
	isRootLock = false;
}

// -------------------------------------------------------------------
void BoneKeyFrame::clear()
{
	relPose.id();
	time = 0.0f;
	scale = physx::PxVec3(1.0f, 1.0f, 1.0f);
}

// -------------------------------------------------------------------
void BoneTrack::clear()
{
	firstFrame = -1;
	numFrames = 0;
}

// -------------------------------------------------------------------
void SkeletalAnimation::clear()
{
	name = "";
	mBoneTracks.clear();
	minTime = 0.0f;
	maxTime = 0.0f;
};

// -------------------------------------------------------------------
SkeletalAnim::SkeletalAnim()
{
	clear();
}

// -------------------------------------------------------------------
SkeletalAnim::~SkeletalAnim()
{
	clear();
}

// -------------------------------------------------------------------
void SkeletalAnim::clear()
{
	ragdollMode = false;

	mBones.clear();
	mBones.resize(0);

	for (physx::PxU32 i = 0; i < mAnimations.size(); i++)
	{
		delete mAnimations[i];
	}

	mAnimations.clear();
	mAnimations.resize(0);

	mKeyFrames.clear();
	mKeyFrames.resize(0);

	mSkinningMatrices.clear();
	mSkinningMatrices.resize(0);
	mSkinningMatricesWorld.clear();
	mSkinningMatricesWorld.resize(0);

	mParent = NULL;
}

// -------------------------------------------------------------------
int SkeletalAnim::findBone(const std::string& name)
{
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		if (mBones[i].name == name)
		{
			return (int)i;
		}
	}
	return -1;
}

// -------------------------------------------------------------------
void SkeletalAnim::interpolateBonePose(int animNr, int boneNr, float time, physx::PxMat34Legacy& pose, physx::PxVec3& scale)
{
	// the default
	pose.id();
	scale = physx::PxVec3(1.0f, 1.0f, 1.0f);

	const std::vector<SkeletalAnimation*>& animations = mParent == NULL ? mAnimations : mParent->mAnimations;
	const std::vector<BoneKeyFrame>& keyFrames = mParent == NULL ? mKeyFrames : mParent->mKeyFrames;

	if (animNr < 0 || animNr >= (int)animations.size())
	{
		return;
	}
	if (boneNr < 0 || boneNr >= (int)animations[(physx::PxU32)animNr]->mBoneTracks.size())
	{
		return;
	}

	BoneTrack& t = animations[(physx::PxU32)animNr]->mBoneTracks[(physx::PxU32)boneNr];
	if (t.numFrames == 0)
	{
		return;
	}

	// special cases
	int frameNr = -1;
	if (t.numFrames == 1)
	{
		frameNr = t.firstFrame;
	}
	else if (time <= keyFrames[(physx::PxU32)t.firstFrame].time)
	{
		frameNr = t.firstFrame;
	}
	else if (time >= keyFrames[physx::PxU32(t.firstFrame + t.numFrames - 1)].time)
	{
		frameNr = t.firstFrame + t.numFrames - 1;
	}

	if (frameNr >= 0)
	{
		pose = keyFrames[(physx::PxU32)frameNr].relPose;
		scale = keyFrames[(physx::PxU32)frameNr].scale;
		return;
	}
	// binary search
	physx::PxU32 l = (physx::PxU32)t.firstFrame;
	physx::PxU32 r = physx::PxU32(t.firstFrame + t.numFrames - 1);
	while (r > l + 1)
	{
		physx::PxU32 m = (l + r) / 2;
		if (keyFrames[m].time == time)
		{
			pose = keyFrames[m].relPose;
			scale = keyFrames[m].scale;
			return;
		}
		else if (keyFrames[m].time > time)
		{
			r = m;
		}
		else
		{
			l = m;
		}
	}
	float dt = keyFrames[r].time - keyFrames[l].time;
	// avoid singular case
	if (dt == 0.0f)
	{
		pose = keyFrames[l].relPose;
		scale = keyFrames[l].scale;
	}

	// interpolation
	float sr = (time - keyFrames[l].time) / dt;
	float sl = 1.0f - sr;

	scale = keyFrames[l].scale * sl + keyFrames[r].scale * sr;
	pose.t = keyFrames[l].relPose.t * sl + keyFrames[r].relPose.t * sr;
	physx::PxQuat ql = physx::PxQuat(keyFrames[l].relPose.M);
	physx::PxQuat qr = physx::PxQuat(keyFrames[r].relPose.M);
	physx::PxQuat q = physx::slerp(sr, ql, qr);
	pose.M.fromQuat(q);
}

// -------------------------------------------------------------------
void SkeletalAnim::setBindPose()
{
	PX_ASSERT(mBones.size() == mSkinningMatrices.size());
	PX_ASSERT(mBones.size() == mSkinningMatricesWorld.size());
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		mSkinningMatrices[i] = physx::PxMat44::createIdentity();
		mBones[i].currentWorldPose = mBones[i].bindWorldPose;
		mSkinningMatricesWorld[i] = mBones[i].currentWorldPose;
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::setAnimPose(int animNr, float time, bool lockRootbone /* = false */)
{
	if (animNr >= 0)
	{
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			if (mBones[i].parent < 0)
			{
				setAnimPoseRec(animNr, (int)i, time, lockRootbone);
			}
		}

		PX_ASSERT(mBones.size() == mSkinningMatrices.size());
		PX_ASSERT(mBones.size() == mSkinningMatricesWorld.size());
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			SkeletalBone& b = mBones[i];
			mSkinningMatrices[i] = b.currentWorldPose * b.invBindWorldPose;
			mSkinningMatricesWorld[i] = b.currentWorldPose;
		}
	}
	else
	{
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			mSkinningMatrices[i] = physx::PxMat44::createIdentity();
			mSkinningMatricesWorld[i] = mBones[i].bindWorldPose;
		}
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::setBoneCollision(physx::PxU32 boneNr, int option)
{
	if (mBones[boneNr].boneOption != option)
	{
		if (mBones[boneNr].boneOption == HACD::BO_COLLAPSE || option == HACD::BO_COLLAPSE)
		{
			// PH: Follow up the hierarchy until something is not set to collapse and mark all dirty
			int current = mBones[boneNr].parent;
			while (current != -1)
			{
				mBones[(physx::PxU32)current].dirtyParams = true;
				if (mBones[(physx::PxU32)current].boneOption != HACD::BO_COLLAPSE)
				{
					break;
				}

				if (mBones[(physx::PxU32)current].parent == current)
				{
					break;
				}

				current = mBones[(physx::PxU32)current].parent;
			}
		}
		mBones[boneNr].dirtyParams = true;

		// Find all children that collapse into this bone and mark them dirty
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			// See whether boneNr is one of its parents
			bool found = false;
			physx::PxU32 current = i;
			while (current != (physx::PxU32)-1 && !found)
			{
				if (current == boneNr)
				{
					found = true;
				}

				if (mBones[current].boneOption != HACD::BO_COLLAPSE)
				{
					break;
				}

				if ((int)current == mBones[current].parent)
				{
					break;
				}

				current = (physx::PxU32)mBones[current].parent;
			}
			if (found)
			{
				mBones[i].dirtyParams = true;
			}
		}
	}

	mBones[boneNr].boneOption = option;
}

// -------------------------------------------------------------------
void SkeletalAnim::setAnimPoseRec(int animNr, int boneNr, float time, bool lockBoneTranslation)
{
	SkeletalBone& b = mBones[(physx::PxU32)boneNr];

	{
		physx::PxMat34Legacy keyPose;
		physx::PxVec3 keyScale;

		// query the first frame instead of the current one if you want to lock this bone
		float myTime = (lockBoneTranslation && b.isRootLock) ? 0.0f : time;
		interpolateBonePose(animNr, boneNr, myTime, keyPose, keyScale);

		// todo: consider scale
		physx::PxMat34Legacy combinedPose;
		combinedPose.t = b.pose.t + keyPose.t;
		combinedPose.M = b.pose.M * keyPose.M;

		if (b.parent < 0)
		{
			b.currentWorldPose = combinedPose;
		}
		else
		{
			b.currentWorldPose = mBones[(physx::PxU32)b.parent].currentWorldPose * combinedPose;
		}
	}

	const int* children = mParent == NULL ? &mChildren[0] : &mParent->mChildren[0];
	for (int i = b.firstChild; i < b.firstChild + b.numChildren; i++)
	{
		setAnimPoseRec(animNr, children[i], time, lockBoneTranslation);
	}
}

// -------------------------------------------------------------------
bool SkeletalAnim::loadFromMeshImport(mimp::MeshSystemContainer* msc, std::string& error, bool onlyAddAnimation)
{
	bool ret = false;

	if (!onlyAddAnimation)
	{
		mBones.clear();
		mSkinningMatrices.clear();
		mSkinningMatricesWorld.clear();

		for (unsigned int i = 0; i < mAnimations.size(); i++)
		{
			SkeletalAnimation* a = mAnimations[i];
			delete a;
		}
		mAnimations.clear();
	}

	bool addAnimation = true;

	if (msc)
	{
		mimp::MeshSystem* ms = mimp::gMeshImport->getMeshSystem(msc);
		if (onlyAddAnimation && ms->mSkeletonCount > 0)
		{
			std::vector<int> overwriteBindPose(ms->mSkeletons[0]->mBoneCount, -1);
			// figure out how those bones map to each other.
			int numNotEqual = 0;
			int numNonZeroMatches = 0;
			for (int i = 0; i < ms->mSkeletons[0]->mBoneCount; i++)
			{
				mimp::MeshBone& bone = ms->mSkeletons[0]->mBones[i];
				for (size_t j = 0; j < mBones.size(); j++)
				{
					if (mBones[j].name.compare(ms->mSkeletons[0]->mBones[i].mName) == 0)
					{
						// found one, but let's see if the bind pose also matches more or less
						physx::PxTransform inputPose;
						inputPose.p = *(physx::PxVec3*)bone.mPosition;
						inputPose.q = *(physx::PxQuat*)bone.mOrientation;


						physx::general_shared3::PxMat34Legacy pose(inputPose);

						physx::general_shared3::PxMat34Legacy poseOld(mBones[j].pose);

						bool equal = (pose.t - poseOld.t).magnitude() <= ((0.5f * pose.t + 0.5f * poseOld.t).magnitude() * 0.01f);

						if (equal && !pose.t.isZero())
						{
							numNonZeroMatches++;
						}

						if (!equal && (inputPose.p.isZero() || numNonZeroMatches > 0))
						{
							// identity, skip the new bone's bind pose
							continue;
						}

						if (!equal)
						{
							char buf[128];
							physx::string::sprintf_s(buf, 128, "Bone %d (%s) does not match bind pose\n", (int)i, ms->mSkeletons[0]->mBones[i].mName);
							error.append(buf);

							numNotEqual++;
						}

						overwriteBindPose[(physx::PxU32)i] = (int)j;
						break;
					}
				}
			}

			if (numNotEqual > 0)
			{
				error = std::string("Failed to load animation:\n") + error;
				addAnimation = false;
			}
			else
			{
				// reset all bind poses exactly, now that we know they match pretty well
				for (physx::PxU32 i = 0; i < (physx::PxU32)ms->mSkeletons[0]->mBoneCount; i++)
				{
					mimp::MeshBone& bone = ms->mSkeletons[0]->mBones[i];

					if (overwriteBindPose[i] != -1)
					{
						physx::PxTransform inputPose;
						inputPose.p = *(physx::PxVec3*)bone.mPosition;
						inputPose.q = *(physx::PxQuat*)bone.mOrientation;
						physx::general_shared3::PxMat34Legacy pose(inputPose);

						mBones[(physx::PxU32)overwriteBindPose[i]].pose = pose;
					}
				}
			}
		}
		else if (ms->mSkeletonCount)
		{
			mimp::MeshSkeleton* sk = ms->mSkeletons[0];

			for (int i = 0; i < sk->mBoneCount; i++)
			{
				mimp::MeshBone& b = sk->mBones[i];
				SkeletalBone sb;
				sb.clear();
				sb.name = b.mName;
				sb.id   = i;
				sb.parent = b.mParentIndex;
				sb.firstChild = 0;
				sb.numChildren = 0;
				sb.isRootLock = b.mParentIndex == 0; // lock the second bone in the hierarchy, first one is the scene root, not the anim root (for fbx files)

				physx::PxQuat q;
				physx::PxQuatFromArray(q, b.mOrientation);
				sb.pose.M.fromQuat(q);
				physx::PxVec3FromArray(sb.pose.t, b.mPosition);
				physx::PxVec3FromArray(sb.scale, b.mScale);

				for (physx::PxU32 bi = 0; bi < mBones.size(); bi++)
				{
					if (mBones[bi].name == b.mName)
					{
						if (error.empty())
						{
							error = "Duplicated Bone Names, rename one:\n";
						}

						error.append(b.mName);
						error.append("\n");
					}
				}

				mBones.push_back(sb);
			}

			ret = true; // allow loading a skeleton without animation
		}

		if (ms->mAnimationCount && addAnimation)
		{
			for (unsigned int i = 0; i < ms->mAnimationCount; i++)
			{
				mimp::MeshAnimation* animation = ms->mAnimations[i];
				SkeletalAnimation* anim = new SkeletalAnimation;
				anim->clear();
				if (ms->mAnimationCount == 1 && ms->mAssetName != NULL)
				{
					const char* lastDir = std::max(strrchr(ms->mAssetName, '/'), strrchr(ms->mAssetName, '\\'));
					anim->name = lastDir != NULL ? lastDir + 1 : ms->mAssetName;
				}
				else
				{
					anim->name = animation->mName;
				}

				size_t numBones = mBones.size();
				anim->mBoneTracks.resize(numBones);
				for (physx::PxU32 j = 0; j < numBones; j++)
				{
					anim->mBoneTracks[j].clear();
				}

				for (physx::PxU32 j = 0; j < (physx::PxU32)animation->mTrackCount; j++)
				{
					mimp::MeshAnimTrack* track = animation->mTracks[j];
					std::string boneName = track->mName;
					int boneNr = findBone(boneName);
					if (boneNr >= 0 && boneNr < (physx::PxI32)numBones)
					{
						anim->mBoneTracks[(physx::PxU32)boneNr].firstFrame = (int)mKeyFrames.size();
						anim->mBoneTracks[(physx::PxU32)boneNr].numFrames  = track->mFrameCount;

						physx::PxMat34Legacy parent = mBones[(physx::PxU32)boneNr].pose;
						physx::PxMat33Legacy pinverse;
						parent.M.getInverse(pinverse);

						float ftime = 0;

						for (physx::PxU32 k = 0; k < (physx::PxU32)track->mFrameCount; k++)
						{
							mimp::MeshAnimPose& pose = track->mPose[k];
							BoneKeyFrame frame;
							frame.clear();

							physx::PxMat34Legacy mat;

							physx::PxQuat q;
							physx::PxQuatFromArray(q, pose.mQuat);

							mat.M.fromQuat(q);
							physx::PxVec3FromArray(mat.t, pose.mPos);

							frame.time = ftime;
							physx::PxVec3FromArray(frame.scale, pose.mScale);

							frame.relPose.t = mat.t - parent.t;
							frame.relPose.M.multiply(pinverse, mat.M);

							mKeyFrames.push_back(frame);

							// eazymesh samples at 60 Hz, not 1s
							ftime += track->mDtime / 200.f;
						}
					}
					else if (!onlyAddAnimation)
					{
						// if onlyAddAnimation is set, the bone count does not have to match up, additional bones are just ignored
						PX_ASSERT(0);
					}

				}

				mAnimations.push_back(anim);
			}

			ret = true;
		}

		physx::PxMat34Legacy matId;
		matId.id();
		mSkinningMatrices.resize((physx::PxU32)mBones.size(), matId);
		mSkinningMatricesWorld.resize((physx::PxU32)mBones.size(), matId);
		init(!onlyAddAnimation);
	}

	return ret;
}

// -------------------------------------------------------------------
bool SkeletalAnim::saveToMeshImport(mimp::MeshSystemContainer* msc)
{
#ifndef PX_WINDOWS
	PX_UNUSED(msc);
	return false;
#else

	if (msc == NULL)
	{
		return false;
	}

	mimp::MeshSystem* ms = mimp::gMeshImport->getMeshSystem(msc);
	if (ms == NULL)
	{
		return false;
	}

	ms->mSkeletonCount = 1;
	ms->mSkeletons = (mimp::MeshSkeleton**)::malloc(sizeof(mimp::MeshSkeleton*));
	ms->mSkeletons[0] = new mimp::MeshSkeleton;

	ms->mSkeletons[0]->mBoneCount = (int)mBones.size();
	ms->mSkeletons[0]->mBones = (mimp::MeshBone*)::malloc(sizeof(mimp::MeshBone) * mBones.size());
	for (size_t i = 0; i < mBones.size(); i++)
	{
		mimp::MeshBone& bone = ms->mSkeletons[0]->mBones[i];

		size_t nameLen = mBones[i].name.length() + 1;
		bone.mName = (char*)::malloc(sizeof(char) * nameLen);
		strcpy_s((char*)bone.mName, nameLen, mBones[i].name.c_str());

		(physx::PxQuat&)bone.mOrientation = mBones[i].pose.M.toQuat();
		
		bone.mParentIndex = mBones[i].parent;

		(physx::PxVec3&)bone.mPosition = mBones[i].pose.t;

		(physx::PxVec3&)bone.mScale = mBones[i].scale;
	}

	ms->mAnimationCount = (unsigned int)mAnimations.size();
	ms->mAnimations = (mimp::MeshAnimation**)::malloc(sizeof(mimp::MeshAnimation*) * mAnimations.size());
	for (unsigned int a = 0; a < ms->mAnimationCount; a++)
	{
		ms->mAnimations[a] = new mimp::MeshAnimation;

		PX_ASSERT(mAnimations[a] != NULL);
		size_t nameLen = mAnimations[a]->name.length() + 1;
		ms->mAnimations[a]->mName = (char*)::malloc(sizeof(char) * nameLen);
		strcpy_s((char*)ms->mAnimations[a]->mName, nameLen, mAnimations[a]->name.c_str());

		unsigned int trackCount = 0;
		for (size_t i = 0; i < mBones.size(); i++)
		{
			trackCount += mAnimations[a]->mBoneTracks[i].numFrames > 0 ? 1 : 0;
		}

		ms->mAnimations[a]->mTrackCount = (physx::PxI32)trackCount;
		ms->mAnimations[a]->mTracks = (mimp::MeshAnimTrack**)::malloc(sizeof(mimp::MeshAnimTrack*) * trackCount);
		ms->mAnimations[a]->mDuration = 0.0f;
		ms->mAnimations[a]->mDtime = 0.0f;

		unsigned int curTrack = 0;
		for (size_t t = 0; t < mBones.size(); t++)
		{
			if (mAnimations[a]->mBoneTracks[t].numFrames <= 0)
			{
				continue;
			}

			mimp::MeshAnimTrack* track = ms->mAnimations[a]->mTracks[curTrack++] = new mimp::MeshAnimTrack;

			track->mName = ms->mSkeletons[0]->mBones[t].mName; // just use the same name as the bone array already does
			const unsigned int firstFrame = (physx::PxU32)mAnimations[a]->mBoneTracks[t].firstFrame;
			track->mFrameCount = mAnimations[a]->mBoneTracks[t].numFrames;
			ms->mAnimations[a]->mFrameCount = std::max(ms->mAnimations[a]->mFrameCount, track->mFrameCount);

			track->mPose = (mimp::MeshAnimPose*)::malloc(sizeof(mimp::MeshAnimPose) * track->mFrameCount);

			track->mDuration = 0.0f;

			for (int f  = 0; f < track->mFrameCount; f++)
			{
				mimp::MeshAnimPose& pose = track->mPose[f];
				BoneKeyFrame& frame = mKeyFrames[firstFrame + f];

				physx::PxMat34Legacy mat;

				mat.M.multiply(frame.relPose.M, mBones[t].pose.M);
				mat.t = frame.relPose.t + mBones[t].pose.t;

				(physx::PxVec3&)pose.mScale = frame.scale;
				(physx::PxVec3&)pose.mPos = mat.t;

				(physx::PxQuat&)pose.mQuat = mat.M.toQuat();

				track->mDuration = std::max(track->mDuration, frame.time);
			}

			track->mDtime = track->mDuration / (float)track->mFrameCount * 200.0f;

			ms->mAnimations[a]->mDuration = std::max(ms->mAnimations[a]->mDuration, track->mDuration);
			ms->mAnimations[a]->mDtime = std::max(ms->mAnimations[a]->mDtime, track->mDtime);
		}
	}

	return true;
#endif
}

// -------------------------------------------------------------------
bool SkeletalAnim::initFrom(physx::apex::NxRenderMeshAssetAuthoring& mesh)
{
	PX_ASSERT(mesh.getPartCount() == 1);

	physx::PxU32 numBones = 0;
	for (physx::PxU32 submeshIndex = 0; submeshIndex < mesh.getSubmeshCount(); submeshIndex++)
	{
		const physx::NxVertexBuffer& vb = mesh.getSubmesh(submeshIndex).getVertexBuffer();
		const physx::NxVertexFormat& vf = vb.getFormat();
		physx::PxU32 bufferIndex = (physx::PxU32)vf.getBufferIndexFromID(vf.getSemanticID(physx::apex::NxRenderVertexSemantic::BONE_INDEX));

		physx::apex::NxRenderDataFormat::Enum format;
		const physx::PxU16* boneIndices = (const physx::PxU16*)vb.getBufferAndFormat(format, bufferIndex);

		unsigned int numBonesPerVertex = 0;
		switch (format)
		{
		case physx::apex::NxRenderDataFormat::USHORT1:
			numBonesPerVertex = 1;
			break;
		case physx::apex::NxRenderDataFormat::USHORT2:
			numBonesPerVertex = 2;
			break;
		case physx::apex::NxRenderDataFormat::USHORT3:
			numBonesPerVertex = 3;
			break;
		case physx::apex::NxRenderDataFormat::USHORT4:
			numBonesPerVertex = 4;
			break;
		default:
			break;
		}

		if (boneIndices == NULL || numBonesPerVertex == 0)
		{
			return false;
		}

		const unsigned int numElements = numBonesPerVertex * vb.getVertexCount();

		for (unsigned int i = 0; i < numElements; i++)
		{
			numBones = std::max(numBones, boneIndices[i] + 1u);
		}
	}

	SkeletalBone initBone;
	initBone.clear();
	mBones.resize(numBones, initBone);

	for (unsigned int i = 0; i < numBones; i++)
	{
		mBones[i].id = (physx::PxI32)i;
	}

	mSkinningMatrices.resize(numBones);
	mSkinningMatricesWorld.resize(numBones);

	init(true);

	return numBones > 0;
}

// -------------------------------------------------------------------
bool SkeletalAnim::loadFromXML(const std::string& xmlFile, std::string& error)
{
	clear();

	physx::PxFileBuffer fb(xmlFile.c_str(), physx::PxFileBuf::OPEN_READ_ONLY);
	if (!fb.isOpen())
	{
		return false;
	}

	FAST_XML::FastXml* fastXml = FAST_XML::createFastXml(this);
	fastXml->processXml(fb);

	int errorLineNumber = -1;
	const char* xmlError = fastXml->getError(errorLineNumber);
	if (xmlError != NULL)
	{
		char temp[1024];
		physx::string::sprintf_s(temp, 1024, "Xml parse error in %s on line %d:\n\n%s", xmlFile.c_str(), errorLineNumber, xmlError);
		error = temp;
		return false;
	}

	fastXml->release();

	physx::PxMat34Legacy matId;
	matId.id();
	mSkinningMatrices.resize((physx::PxU32)mBones.size(), matId);
	mSkinningMatricesWorld.resize((physx::PxU32)mBones.size(), matId);

	init(true);
	return true;
}

// -------------------------------------------------------------------
bool SkeletalAnim::loadFromParent(const SkeletalAnim* parent)
{
	if (parent == NULL)
	{
		return false;
	}

	mParent = parent;

	mBones.resize(mParent->mBones.size());
	physx::PxMat34Legacy matId;
	matId.id();
	mSkinningMatrices.resize((physx::PxU32)mBones.size(), matId);
	mSkinningMatricesWorld.resize((physx::PxU32)mBones.size(), matId);
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		mBones[i] = mParent->mBones[i];
	}

	return true;
}

// -------------------------------------------------------------------
bool SkeletalAnim::saveToXML(const std::string& xmlFile) const
{
	FILE* f = 0;
	if (physx::fopen_s(&f, xmlFile.c_str(), "w") != 0)
	{
		return false;
	}

	fprintf(f, "<skeleton>\n\n");

	fprintf(f, "  <bones>\n");
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		const SkeletalBone& bone = mBones[i];

		physx::PxQuat q(bone.pose.M);
		float angle;
		physx::PxVec3 axis;
		q.toRadiansAndUnitAxis(angle, axis);
		angle = q.getAngle();

		fprintf(f, "    <bone id = \"%i\" name = \"%s\">\n", bone.id, bone.name.c_str());
		fprintf(f, "      <position x=\"%f\" y=\"%f\" z=\"%f\" />\n", bone.pose.t.x, bone.pose.t.y, bone.pose.t.z);
		fprintf(f, "      <rotation angle=\"%f\">\n", angle);
		fprintf(f, "        <axis x=\"%f\" y=\"%f\" z=\"%f\" />\n", axis.x, axis.y, axis.z);
		fprintf(f, "      </rotation>\n");
		fprintf(f, "      <scale x=\"%f\" y=\"%f\" z=\"%f\" />\n", 1.0f, 1.0f, 1.0f);
//		dont' use bone.scale.x, bone.scale.y, bone.scale.z because the length is baked into the bones
		fprintf(f, "    </bone>\n");
	}
	fprintf(f, "  </bones>\n\n");

	fprintf(f, "  <bonehierarchy>\n");
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		const SkeletalBone& bone = mBones[i];
		if (bone.parent < 0)
		{
			continue;
		}
		fprintf(f, "    <boneparent bone=\"%s\" parent=\"%s\" />\n", bone.name.c_str(), mBones[(physx::PxU32)bone.parent].name.c_str());
	}
	fprintf(f, "  </bonehierarchy>\n\n");

	fprintf(f, "  <animations>\n");
	for (physx::PxU32 i = 0; i < mAnimations.size(); i++)
	{
		const SkeletalAnimation* anim = mAnimations[i];

		fprintf(f, "    <animation name = \"%s\" length=\"%f\">\n", anim->name.c_str(), anim->maxTime);
		fprintf(f, "      <tracks>\n");

		for (physx::PxU32 j = 0; j < anim->mBoneTracks.size(); j++)
		{
			const BoneTrack& track = anim->mBoneTracks[j];
			if (track.numFrames == 0)
			{
				continue;
			}

			fprintf(f, "        <track bone = \"%s\">\n", mBones[j].name.c_str());
			fprintf(f, "          <keyframes>\n");

			for (int k = track.firstFrame; k < track.firstFrame + track.numFrames; k++)
			{
				const BoneKeyFrame& frame = mKeyFrames[(physx::PxU32)k];
				physx::PxQuat q(frame.relPose.M);
				float angle;
				physx::PxVec3 axis;
				q.toRadiansAndUnitAxis(angle, axis);
				angle = q.getAngle();

				fprintf(f, "            <keyframe time = \"%f\">\n", frame.time);
				fprintf(f, "              <translate x=\"%f\" y=\"%f\" z=\"%f\" />\n", frame.relPose.t.x, frame.relPose.t.y, frame.relPose.t.z);
				fprintf(f, "              <rotate angle=\"%f\">\n", angle);
				fprintf(f, "                <axis x=\"%f\" y=\"%f\" z=\"%f\" />\n", axis.x, axis.y, axis.z);
				fprintf(f, "              </rotate>\n");
				fprintf(f, "              <scale x=\"%f\" y=\"%f\" z=\"%f\" />\n", frame.scale.x, frame.scale.y, frame.scale.z);
				fprintf(f, "            </keyframe>\n");
			}
			fprintf(f, "          </keyframes>\n");
			fprintf(f, "        </track>\n");
		}
		fprintf(f, "      </tracks>\n");
		fprintf(f, "    </animation>\n");
	}
	fprintf(f, "  </animations>\n");
	fprintf(f, "</skeleton>\n\n");

	fclose(f);


	return true;
}

// -------------------------------------------------------------------
void SkeletalAnim::init(bool firstTime)
{
	if (firstTime)
	{
		setupConnectivity();

		// init bind poses
		physx::PxVec3 oneOneOne(1.0f, 1.0f, 1.0f);
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			if (mBones[i].parent < 0)
			{
				initBindPoses((physx::PxI32)i, oneOneOne);
			}

			// collapse finger and toes
			if (
				mBones[i].name.find("finger") != std::string::npos ||
				mBones[i].name.find("Finger") != std::string::npos ||
				mBones[i].name.find("FINGER") != std::string::npos ||
				mBones[i].name.find("toe") != std::string::npos ||
				mBones[i].name.find("Toe") != std::string::npos ||
				mBones[i].name.find("TOE") != std::string::npos)
			{
				mBones[i].boneOption = 2; // this is collapse
			}
		}
	}

	PX_ASSERT(mBones.size() == mSkinningMatrices.size());
	PX_ASSERT(mBones.size() == mSkinningMatricesWorld.size());
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		SkeletalBone& b = mBones[i];
		b.bindWorldPose.getInverseRT(b.invBindWorldPose);
		b.currentWorldPose = mBones[i].bindWorldPose;
		mSkinningMatrices[i] = physx::PxMat44::createIdentity();
		mSkinningMatricesWorld[i] = b.currentWorldPose;
	}

	// init time interval of animations
	for (physx::PxU32 i = 0; i < mAnimations.size(); i++)
	{
		SkeletalAnimation* a = mAnimations[i];
		bool first = true;
		for (physx::PxU32 j = 0; j < a->mBoneTracks.size(); j++)
		{
			BoneTrack& b = a->mBoneTracks[j];
			for (int k = b.firstFrame; k < b.firstFrame + b.numFrames; k++)
			{
				float time = mKeyFrames[(physx::PxU32)k].time;
				if (first)
				{
					a->minTime = time;
					a->maxTime = time;
					first = false;
				}
				else
				{
					if (time < a->minTime)
					{
						a->minTime = time;
					}
					if (time > a->maxTime)
					{
						a->maxTime = time;
					}
				}
			}
		}
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::initBindPoses(int boneNr, const physx::PxVec3& scale)
{
	SkeletalBone& b = mBones[(physx::PxU32)boneNr];
	b.pose.t = b.pose.t.multiply(scale);

	physx::PxVec3 newScale = scale.multiply(b.scale);

	if (b.parent < 0)
	{
		b.bindWorldPose = b.pose;
	}
	else
	{
		b.bindWorldPose = mBones[(physx::PxU32)b.parent].bindWorldPose * b.pose;
	}

	for (int i = b.firstChild; i < b.firstChild + b.numChildren; i++)
	{
		initBindPoses(mChildren[(physx::PxU32)i], newScale);
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::setupConnectivity()
{
	size_t i;
	size_t numBones = mBones.size();
	for (i = 0; i < numBones; i++)
	{
		SkeletalBone& b = mBones[i];
		if (b.parent >= 0)
		{
			mBones[(physx::PxU32)b.parent].numChildren++;
		}
	}
	int first = 0;
	for (i = 0; i < numBones; i++)
	{
		mBones[i].firstChild = first;
		first += mBones[i].numChildren;
	}
	mChildren.resize((physx::PxU32)first);
	for (i = 0; i < numBones; i++)
	{
		if (mBones[i].parent < 0)
		{
			continue;
		}
		SkeletalBone& p = mBones[(physx::PxU32)mBones[i].parent];
		mChildren[(physx::PxU32)p.firstChild++] = (int)i;
	}
	for (i = 0; i < numBones; i++)
	{
		mBones[i].firstChild -= mBones[i].numChildren;
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::draw(physx::NxApexRenderDebug* batcher)
{
	PX_ASSERT(batcher != NULL);
	if (batcher == NULL)
	{
		return;
	}

	const physx::PxU32 colorWhite = batcher->getDebugColor(physx::DebugColors::White);
	const physx::PxU32 colorBlack = batcher->getDebugColor(physx::DebugColors::Black);
	for (physx::PxU32 i = 0; i < mBones.size(); i++)
	{
		SkeletalBone& bone = mBones[i];

		physx::PxU32 color = bone.selected ? colorWhite : colorBlack;
		batcher->setCurrentColor(color);

		if (bone.parent >= 0 /*&& mBones[bone.parent].parent >= 0*/)
		{
			SkeletalBone& parent = mBones[(physx::PxU32)bone.parent];

			batcher->debugLine(bone.currentWorldPose.t, parent.currentWorldPose.t);
		}
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::copyFrom(const SkeletalAnim& anim)
{
	clear();

	mBones.resize(anim.mBones.size());
	for (physx::PxU32 i = 0; i < anim.mBones.size(); i++)
	{
		mBones[i] = anim.mBones[i];
	}

	mSkinningMatrices.resize(anim.mSkinningMatrices.size());
	for (physx::PxU32 i = 0; i < anim.mSkinningMatrices.size(); i++)
	{
		mSkinningMatrices[i] = anim.mSkinningMatrices[i];
	}

	mSkinningMatricesWorld.resize(anim.mSkinningMatricesWorld.size());
	for (physx::PxU32 i = 0; i < anim.mSkinningMatricesWorld.size(); i++)
	{
		mSkinningMatricesWorld[i] = anim.mSkinningMatricesWorld[i];
	}

	mChildren.resize(anim.mChildren.size());
	for (physx::PxU32 i = 0; i < anim.mChildren.size(); i++)
	{
		mChildren[i] = anim.mChildren[i];
	}

	for (physx::PxU32 i = 0; i < anim.mAnimations.size(); i++)
	{
		SkeletalAnimation* a = anim.mAnimations[i];
		SkeletalAnimation* na = new SkeletalAnimation();
		na->minTime = a->minTime;
		na->maxTime = a->maxTime;
		na->name = a->name;
		na->mBoneTracks.resize(a->mBoneTracks.size());
		for (physx::PxU32 j = 0; j < a->mBoneTracks.size(); j++)
		{
			na->mBoneTracks[j] = a->mBoneTracks[j];
		}
		mAnimations.push_back(na);
	}

	mKeyFrames.resize(anim.mKeyFrames.size());
	for (physx::PxU32 i = 0; i < anim.mKeyFrames.size(); i++)
	{
		mKeyFrames[i] = anim.mKeyFrames[i];
	}
}


// -------------------------------------------------------------------
void SkeletalAnim::clearShapeCount(int boneIndex)
{
	if (boneIndex < 0)
	{
		for (physx::PxU32 i = 0; i < mBones.size(); i++)
		{
			mBones[i].numShapes = 0;
		}
	}
	else
	{
		PX_ASSERT((physx::PxU32)boneIndex < mBones.size());
		mBones[(physx::PxU32)boneIndex].numShapes = 0;
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::incShapeCount(int boneIndex)
{
	if (boneIndex >= 0 && (physx::PxU32)boneIndex < mBones.size())
	{
		mBones[(physx::PxU32)boneIndex].numShapes++;
	}
}

// -------------------------------------------------------------------
void SkeletalAnim::decShapeCount(int boneIndex)
{
	if (boneIndex >= 0 && (physx::PxU32)boneIndex < mBones.size())
	{
		PX_ASSERT(mBones[(physx::PxU32)boneIndex].numShapes > 0);
		mBones[(physx::PxU32)boneIndex].numShapes--;
	}
}
// -------------------------------------------------------------------
bool SkeletalAnim::processElement(const char* elementName, int argc, const char** argv, const char* /*elementData*/, int /*lineno*/)
{
	PX_UNUSED(argc);
	static int activeBoneTrack = -1;
	static BoneKeyFrame* activeKeyFrame;
	static bool isAnimation = false;

	if (strcmp(elementName, "skeleton") == 0)
	{
		// ok, a start
	}
	else if (strcmp(elementName, "bones") == 0)
	{
		// the list of bones
	}
	else if (strcmp(elementName, "bone") == 0)
	{
		PX_ASSERT(argc == 4);
		PX_ASSERT(strcmp(argv[0], "id") == 0);
		PX_ASSERT(strcmp(argv[2], "name") == 0);
		SkeletalBone bone;
		bone.clear();
		bone.id = atoi(argv[1]);
		bone.name = argv[3];
		mBones.push_back(bone);
	}
	else if (strcmp(elementName, "position") == 0)
	{
		PX_ASSERT(argc == 6);
		PX_ASSERT(strcmp(argv[0], "x") == 0);
		PX_ASSERT(strcmp(argv[2], "y") == 0);
		PX_ASSERT(strcmp(argv[4], "z") == 0);
		physx::PxVec3 pos;
		pos.x = (physx::PxF32)atof(argv[1]);
		pos.y = (physx::PxF32)atof(argv[3]);
		pos.z = (physx::PxF32)atof(argv[5]);
		mBones.back().pose.t = pos;
	}
	else if (strcmp(elementName, "rotation") == 0)
	{
		PX_ASSERT(argc == 2);
		PX_ASSERT(strcmp(argv[0], "angle") == 0);
		mBones.back().pose.M(0, 0) = (physx::PxF32)atof(argv[1]);
		isAnimation = false;
	}
	else if (strcmp(elementName, "axis") == 0 && !isAnimation)
	{
		PX_ASSERT(argc == 6);
		PX_ASSERT(strcmp(argv[0], "x") == 0);
		PX_ASSERT(strcmp(argv[2], "y") == 0);
		PX_ASSERT(strcmp(argv[4], "z") == 0);
		physx::PxVec3 axis;
		axis.x = (physx::PxF32)atof(argv[1]);
		axis.y = (physx::PxF32)atof(argv[3]);
		axis.z = (physx::PxF32)atof(argv[5]);
		physx::PxF32 angle = mBones.back().pose.M(0, 0);
		physx::PxQuat quat(angle, axis);
		mBones.back().pose.M.fromQuat(quat);
	}
	else if (strcmp(elementName, "scale") == 0)
	{
		PX_ASSERT(argc == 6);
		PX_ASSERT(strcmp(argv[0], "x") == 0);
		PX_ASSERT(strcmp(argv[2], "y") == 0);
		PX_ASSERT(strcmp(argv[4], "z") == 0);
		physx::PxVec3 scale;
		scale.x = (physx::PxF32)atof(argv[1]);
		scale.y = (physx::PxF32)atof(argv[3]);
		scale.z = (physx::PxF32)atof(argv[5]);
		mBones.back().scale = scale;
	}
	else if (strcmp(elementName, "bonehierarchy") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "boneparent") == 0)
	{
		PX_ASSERT(argc == 4);
		PX_ASSERT(strcmp(argv[0], "bone") == 0);
		PX_ASSERT(strcmp(argv[2], "parent") == 0);
		int child = findBone(argv[1]);
		int parent = findBone(argv[3]);
		if (child >= 0 && child < (int)mBones.size() && parent >= 0 && parent < (int)mBones.size())
		{
			mBones[(physx::PxU32)child].parent = parent;
		}
	}
	else if (strcmp(elementName, "animations") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "animation") == 0)
	{
		PX_ASSERT(argc == 4);
		PX_ASSERT(strcmp(argv[0], "name") == 0);

		SkeletalAnimation* anim = new SkeletalAnimation;
		anim->clear();
		anim->name = argv[1];
		anim->mBoneTracks.resize((physx::PxU32)mBones.size());

		mAnimations.push_back(anim);
	}
	else if (strcmp(elementName, "tracks") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "track") == 0)
	{
		PX_ASSERT(argc == 2);
		PX_ASSERT(strcmp(argv[0], "bone") == 0);
		activeBoneTrack = findBone(argv[1]);
		if (activeBoneTrack >= 0 && activeBoneTrack < (int)mBones.size())
		{
			mAnimations.back()->mBoneTracks[(physx::PxU32)activeBoneTrack].firstFrame = (int)(mKeyFrames.size());
			mAnimations.back()->mBoneTracks[(physx::PxU32)activeBoneTrack].numFrames = 0;
		}
	}
	else if (strcmp(elementName, "keyframes") == 0)
	{
		// ok
	}
	else if (strcmp(elementName, "keyframe") == 0)
	{
		PX_ASSERT(argc == 2);
		PX_ASSERT(strcmp(argv[0], "time") == 0);

		mAnimations.back()->mBoneTracks[(physx::PxU32)activeBoneTrack].numFrames++;

		mKeyFrames.push_back(BoneKeyFrame());
		activeKeyFrame = &mKeyFrames.back();
		activeKeyFrame->clear();
		activeKeyFrame->time = (float)atof(argv[1]);
	}
	else if (strcmp(elementName, "translate") == 0)
	{
		PX_ASSERT(argc == 6);
		PX_ASSERT(strcmp(argv[0], "x") == 0);
		PX_ASSERT(strcmp(argv[2], "y") == 0);
		PX_ASSERT(strcmp(argv[4], "z") == 0);
		activeKeyFrame->relPose.t.x = (physx::PxF32)atof(argv[1]);
		activeKeyFrame->relPose.t.y = (physx::PxF32)atof(argv[3]);
		activeKeyFrame->relPose.t.z = (physx::PxF32)atof(argv[5]);
	}
	else if (strcmp(elementName, "rotate") == 0)
	{
		PX_ASSERT(argc == 2);
		PX_ASSERT(strcmp(argv[0], "angle") == 0);
		activeKeyFrame->relPose.M(0, 0) = (physx::PxF32)atof(argv[1]);
		isAnimation = true;
	}
	else if (strcmp(elementName, "axis") == 0 && isAnimation)
	{
		PX_ASSERT(argc == 6);
		PX_ASSERT(strcmp(argv[0], "x") == 0);
		PX_ASSERT(strcmp(argv[2], "y") == 0);
		PX_ASSERT(strcmp(argv[4], "z") == 0);
		physx::PxVec3 axis;
		axis.x = (physx::PxF32)atof(argv[1]);
		axis.y = (physx::PxF32)atof(argv[3]);
		axis.z = (physx::PxF32)atof(argv[5]);
		axis.normalize();
		physx::PxF32 angle = activeKeyFrame->relPose.M(0, 0);
		physx::PxQuat quat(angle, axis);
		activeKeyFrame->relPose.M.fromQuat(quat);
	}
	else
	{
		// always break here, at least in debug mode
		PX_ALWAYS_ASSERT();
	}

	return true;
}

} // namespace Samples
