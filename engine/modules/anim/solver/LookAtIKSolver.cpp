#include "engine/core/Math/Quaternion.h"
#include "Engine/modules/Anim/Bone.h"
#include "Engine/modules/Anim/Skeleton.h"
#include "LookAtIKSolver.h"

namespace Echo
{
	// 构造函数
	SkeletonControllerLookAt::SkeletonControllerLookAt()
	{

	}

	// 析构函数
	SkeletonControllerLookAt::~SkeletonControllerLookAt()
	{

	}

	// 设置作用骨骼点
	void SkeletonControllerLookAt::attachBone(Skeleton* skeleton, const char* boneName)
	{
		m_skeleton = skeleton;
		m_bone = skeleton->getBone(boneName);
	}

	// 设置旋转值
	void SkeletonControllerLookAt::setRotation(const Quaternion& rotation)
	{
		m_rotation = rotation;
	}

	// 更新
	void SkeletonControllerLookAt::frameMove(i32 deltaTime)
	{
	}

	// 执行
	void SkeletonControllerLookAt::solver(SkeletonPose& skeletonPose)
	{
		skeletonPose.m_lookAtRotations[m_bone->getId()] = m_bone->getRotation() * m_rotation;
	}
}