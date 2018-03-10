#include "engine/core/Base/TypeDef.h"
#include "engine/core/Math/Quaternion.h"
#include "engine/core/Util/LogManager.h"
#include "AnimSolver.h"
#include "Engine/modules/Anim/Bone.h"
#include "Engine/modules/Anim/Skeleton.h"


namespace Echo
{
	// 构造函数
	SkeletonControllerBoneRotation::SkeletonControllerBoneRotation()
	{

	}

	// 析构函数
	SkeletonControllerBoneRotation::~SkeletonControllerBoneRotation()
	{

	}

	// 设置作用骨骼点
	void SkeletonControllerBoneRotation::attachBone( Skeleton* skeleton, const char* boneName)
	{
		m_skeleton = skeleton;
		m_bone     = skeleton->getBone( boneName);
	}

	// 设置旋转值
	void SkeletonControllerBoneRotation::setRotation( const Quaternion& rotation)
	{
		m_rotation = rotation;
	}

	// 更新
	void SkeletonControllerBoneRotation::frameMove( i32 deltaTime)
	{
	}

	// 执行
	void SkeletonControllerBoneRotation::solver( SkeletonPose& skeletonPose)
	{
#ifdef ECHO_PLATFORM_ANDROID
		if (m_bone->getId() >= skeletonPose.m_rotations.size())
		{
			EchoLogError("[BoneRotation::solver:%d]:: this == [0x%08x]", __LINE__, this);
			EchoLogError("[BoneRotation::solver:%d]:: skeletonPose == [0x%08x]", __LINE__, &skeletonPose);
			EchoLogError("[BoneRotation::solver:%d]:: bone == [0x%08x]", __LINE__, m_bone); 
			EchoLogError("[BoneRotation::solver:%d]:: boneid == [%d]", __LINE__, m_bone->getId());
			EchoLogError("[BoneRotation::solver:%d]:: m_rotations.size == [%d]", __LINE__, skeletonPose.m_rotations.size());

			return; 
		}
#endif
		skeletonPose.m_rotations[m_bone->getId()] *= m_rotation;
	}
}