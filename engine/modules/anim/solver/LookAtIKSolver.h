#pragma once
#include "AnimSolver.h"

namespace Echo
{
	class Bone;
	class Skeleton;
	struct SkeletonPose;


	class SkeletonControllerLookAt : public ISkeletonController
	{
	public:
		SkeletonControllerLookAt();
		~SkeletonControllerLookAt();

		// 设置作用骨骼点
		void attachBone(Skeleton* skeleton, const char* boneName);

		// 设置旋转值
		void setRotation(const Quaternion& rotation);

		// 更新
		virtual void frameMove(i32 deltaTime);

		// 执行
		virtual void solver(SkeletonPose& skeletonPose);

		// 获取作用骨骼点
		virtual const Bone* getBone() { return m_bone; }

	protected:
		Skeleton*		m_skeleton;			// 对应骨架
		Bone*			m_bone;				// 被操作骨骼名称
		Quaternion		m_rotation;			// 骨骼旋转
	};
}