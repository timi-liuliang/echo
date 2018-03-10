#include "engine/core/Util/Singleton.h"
#include "Engine/core/main/OpenMPTaskMgr.h"
#include "Engine/core/main/EngineSettings.h"
#include "Animation.h"
#include "AnimState.h"
#include "Skeleton.h"
#include "AnimManager.h"
#include "AnimKeyFrame.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/util/Exception.h"
#include "Bone.h"
#include "Solver/AnimSolver.h"
#include "engine/core/Math/Quaternion.h"
#include "AnimBlender.h"

namespace Echo
{
	void SkipOptimizeBone(array<bool, 256>& effectiveBones, Skeleton* skeleton, const char* boneName)
	{
		Bone* bone = skeleton->getBone(boneName);
		if (bone)
		{
			effectiveBones[bone->getId()] = true;
		}
	}

	// 构造函数
	AnimBlender::AnimBlender(Skeleton* pSkel)
		: m_skeleton(pSkel)
		, m_mode(BM_CROSSFADE)
		, m_animState(NULL)
		, m_targetAnimState(NULL)
		, m_blendTime(0)
		, m_leftBlendTime(0)
		, m_KeyOrientation(NULL)
		, m_KeyPosition(NULL)
		, mbNeedMerge(false)
		, mbIsLoopSkill(false)
	{
		size_t boneCount = pSkel->getBoneCount();
		m_AnimMatrix.resize(boneCount, Matrix4::IDENTITY);
		m_KeyOrientation.resize(boneCount, Quaternion::IDENTITY);
		m_KeyPosition.resize(boneCount, Vector3::ZERO);
		m_KeyScale.resize(boneCount, 1.0f);
		m_effectiveBones.fill(true);

		
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_Spine1");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_Spine2");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_L_ForeTwist");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_R_ForeTwist");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_L_UpperArm");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_R_UpperArm");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_L_Forearm");
		SkipOptimizeBone(m_skipOptimizeBones, m_skeleton, "Bip01_R_Forearm");

		// 以1开始计数
		static int identifier = 1;
		m_identifier = identifier++;
	}

	// 析构函数
	AnimBlender::~AnimBlender()
	{
		removeSkelController();
	}

	bool AnimBlender::boneAnimOptimized(size_t i)
	{
		if (!EngineSettingsMgr::instance()->isEnableAnimOptimize())
		{
			return false;
		}

		if (m_effectiveBones[i])
		{
			return false;
		}

		return true;
	}

	// 混合
	bool AnimBlender::blend(AnimState* pAnimState, ui32 blendTime)
	{
		// 是否为部分动画
		if (pAnimState->isPartition())
		{
			pAnimState->setBlendTime(blendTime);
			m_partitionAnimStates[pAnimState->getRootBone()] = pAnimState;
		}
		else
		{
			//如果动画执行顺序为A-B-A，直接抛弃B，返回;如果为A-A，直接返回
			if (pAnimState == m_animState)
			{
				if (m_targetAnimState)
				{
					m_targetAnimState = NULL;
				}
				return true;
			}

			//如果第一次执行动画，不需要融合
			if (NULL == m_animState)
			{
				AnimKeyFrame* pSrcKeyFrame = pAnimState->getCurrentAnimKeyFrame();
				for (size_t i = 0; i < m_skeleton->getBoneCount(); ++i)
				{
					pSrcKeyFrame->getAnimQuaternion(i).toMat4(m_AnimMatrix[i]);

					const Vector4& translate = pSrcKeyFrame->getAnimTranslate(i);
					m_AnimMatrix[i].translate((Vector3)translate);
				}//for
				m_animState = pAnimState;

				m_targetAnimState = NULL;
				return true;
			}

			switch (m_mode)
			{
				case BM_REPLACE:
				{
					m_animState->enable(false);
					m_animState = pAnimState;
					m_blendTime = blendTime;
					m_leftBlendTime = 0;
				}
				break;
				case BM_CROSSFADE:
				{
					m_targetAnimState = pAnimState;
					m_blendTime = blendTime;
					m_leftBlendTime = blendTime;
				}
				break;
				default: break;
			}

		}

		return true;
	}

	// 添加动画控制器
	void AnimBlender::addSkelController(ISkeletonController* skelController)
	{
		m_skelControllers.push_back(skelController);
	}

	void AnimBlender::removeSkelController(ISkeletonController* skelController)
	{
		for (size_t i = 0; i < m_skelControllers.size(); ++i)
		{
			ISkeletonController* temp = m_skelControllers[i];
			if (temp == skelController)
			{
				m_skelControllers[i] = m_skelControllers[m_skelControllers.size() - 1];
				m_skelControllers.pop_back();
				break;
			}
		}
	}

	void AnimBlender::removeSkelController()
	{
		for (size_t i = 0; i < m_skelControllers.size(); ++i)
		{
			ISkeletonController* skeletonCtrl = m_skelControllers[i];
			EchoSafeDelete(skeletonCtrl, ISkeletonController);
		}
		m_skelControllers.clear();
	}

	// 设置混合模式
	void AnimBlender::setBlendMode(BlendMode mode)
	{
		m_mode = mode;
	}

	Skeleton* AnimBlender::getSkeleton() const
	{
		return m_skeleton;
	}

	// 是否处于混合状态
	bool AnimBlender::isBlending() const
	{
		return (m_leftBlendTime > 0);
	}

	AnimState* AnimBlender::getAnimState() const
	{
		return m_animState;
	}

	const Matrix4& AnimBlender::getAnimMatrix(ui32 idx) const
	{
		return m_AnimMatrix[idx];
	}

	const Matrix4* AnimBlender::getAnimMatrixPtr() const
	{
		return m_AnimMatrix.data();
	}

	// 任务
	struct AnimBlenderTask : public CpuThreadPool::Job
	{
		int			 m_blenderID;		// 动画混合器
		i32			 m_deltaTime;

		// 执行任务
		virtual bool process()
		{
			AnimBlender* blender = AnimManager::instance()->getAnimBlender(m_blenderID);
			if (blender)
				blender->frameMoveInteral(m_deltaTime);

			return true;
		}

		// 获取任务类型
		virtual int getType()
		{
			return OpenMPTaskMgr::TT_AnimationUpdate;
		}
	};

	// 更新
	void AnimBlender::frameMove(i32 deltaTime, bool isSync)
	{
#ifdef ECHO_EDITOR_MODE
		//if (deltaTime)
		frameMoveInteral(deltaTime);
#else
		// 在引擎获取整个更新函数的控制之前。异步更新会有一些小BUG。
		isSync = true;

		if (isSync)
		{
			frameMoveInteral(deltaTime);
		}
		else
		{
			AnimBlenderTask* task = EchoNew(AnimBlenderTask);
			task->m_blenderID = m_identifier;
			task->m_deltaTime = deltaTime;
			OpenMPTaskMgr::instance()->addTask(OpenMPTaskMgr::TT_AnimationUpdate, task);
		}
#endif
	}

	// 停止所有部分动画
	void AnimBlender::stopAllPartitionAinm()
	{
		m_partitionAnimStates.clear();
	}

	// 计算骨架位置
	bool AnimBlender::calcSkeletonPose(SkeletonPose& oSkeletonPos, AnimState* animState, i32 deltaTime)
	{
		for (size_t i = 0; i < m_skeleton->getBoneCount(); ++i)
		{
			if (boneAnimOptimized(i))
			{
				continue;
			}

			Bone* pBone = m_skeleton->getBoneById(i);
			if (animState->isEffectBone(pBone))
			{
				// 当前动画数据
				Quaternion curOrientation;
				Vector4	   curTranslation;
				if (animState->calcBonePos(curOrientation, curTranslation, pBone, deltaTime))
				{
					oSkeletonPos.m_rotations[i] = curOrientation;
					oSkeletonPos.m_translations[i] = curTranslation;
				}
			}
		}

		return true;
	}

	// 计算部分动画对骨架影响
	bool AnimBlender::calcSkeletonPoseByPartitionAnimState(SkeletonPose& skeletonPose, i32 deltaTime)
	{
		if (m_partitionAnimStates.empty())
		{
			return true;
		}

		// 更新partitionAnimState
		for (map<int, AnimState*>::type::iterator it = m_partitionAnimStates.begin(); it != m_partitionAnimStates.end();)
		{
			AnimState* animState = it->second;
			animState->frameMove(deltaTime);
			if (!animState->isEnable())
			{
				m_partitionAnimStates.erase(it++);
			}
			else
			{
				it++;
			}
		}

		// 记录原骨骼位置
		SkeletonPose origSkeletonPose = skeletonPose;

		// 应用动画
		for (map<int, AnimState*>::type::iterator it = m_partitionAnimStates.begin(); it != m_partitionAnimStates.end(); it++)
		{
			AnimState* partAnimState = it->second;
			Bone* partRootBone = m_skeleton->getBoneById(partAnimState->getRootBone());
			float partWeight = partAnimState->getBlendWeight();

			// 计算锚点位置
			Vector4 anchorPos; Quaternion anchorRotation;
			anchorRotation = skeletonPose.m_rotations[partRootBone->getId()];
			anchorPos = skeletonPose.m_translations[partRootBone->getId()];

			calcSkeletonPose(skeletonPose, partAnimState, deltaTime);

			// 部分动画影响后锚点位置
			Quaternion partRotation = skeletonPose.m_rotations[partRootBone->getId()];
			Vector4    partPos = skeletonPose.m_translations[partRootBone->getId()];

			skeletonPose.m_rotations[partRootBone->getId()] = anchorRotation;
			skeletonPose.m_translations[partRootBone->getId()] = anchorPos;

			// 偏移计算
			Vector4 offset = anchorPos - partPos;
			offset.w = 0.0f;
			for (size_t i = 0; i < m_skeleton->getBoneCount(); ++i)
			{
				Bone* pBone = m_skeleton->getBoneById(i);
				if (partAnimState->isEffectBone(pBone) && partAnimState->getRootBone() != pBone->getId())
				{
					skeletonPose.m_translations[pBone->getId()] += offset;

					// 混合
					if (partWeight != 1.f)
					{
						Quaternion::Slerp(skeletonPose.m_rotations[i], origSkeletonPose.m_rotations[i], skeletonPose.m_rotations[i], partWeight, true);
						skeletonPose.m_translations[i] = origSkeletonPose.m_translations[i] * (1 - partWeight) + skeletonPose.m_translations[i] * partWeight;
					}
				}
			}
		}

		return true;
	}

	// 计算骨架控制器骨架影响
	bool AnimBlender::calcSkeletonControllers(SkeletonPose& skeletonPose, i32 deltaTime)
	{
		for (size_t i = 0; i < m_skelControllers.size(); i++)
		{
			m_skelControllers[i]->frameMove(deltaTime);
			m_skelControllers[i]->solver(skeletonPose);
		}

		return true;
	}

	void AnimBlender::calcObjectSpacePose(SkeletonPose& skeletonPose)
	{
		if (!m_animState || !m_animState->getAnim()->hasScaling())
			return;

		for (size_t i = 0; i < m_skeleton->getBoneCount(); i++)
		{
			if (boneAnimOptimized(i))
			{
				continue;
			}

			int parentID = m_skeleton->getBoneById(i)->getParentId();

			const float& localScale = skeletonPose.m_translations[i].w;
			const Quaternion& localQuat = skeletonPose.m_rotations[i];
			const Vector3& localPosition = (Vector3)skeletonPose.m_translations[i];

			if (parentID >= 0)
			{
				Bone* bone = m_skeleton->getBoneById(parentID);
				if (bone != NULL && !bone->isDummy())
				{
					const float& parentScale = skeletonPose.m_translations[parentID].w;
					const Quaternion& parentQuat = skeletonPose.m_rotations[parentID];
					const Vector3& parentPosition = skeletonPose.m_translations[parentID];

					float scale = localScale * parentScale;
					Vector3 position = localPosition * parentScale;
					parentQuat.rotateVec3(position, position);
					position += parentPosition;

					skeletonPose.m_rotations[i] = localQuat;
					skeletonPose.m_rotations[i] *= parentQuat;
					skeletonPose.m_translations[i] = Vector4(position.x, position.y, position.z, scale);
				}
			}

			m_KeyScale[i] = skeletonPose.m_translations[i].w;
			m_KeyOrientation[i] = skeletonPose.m_rotations[i];
			m_KeyPosition[i] = skeletonPose.m_translations[i];
		}

	}
	
	void AnimBlender::setEffectiveBones(const array<bool,256>& bones)
	{
		if (!m_skeleton)
		{
			return;
		}

		m_effectiveBones = bones;	
		for (size_t i = 0; i < m_effectiveBones.size(); ++i)
		{
			m_effectiveBones[i] |= m_skipOptimizeBones[i];
		}
	}

	// 融合动画
	void AnimBlender::crossFade(SkeletonPose& oSkeletonPose, AnimState* firstAnimState, AnimState* secondAnimState, float dstWeight, i32 deltaTime) const
	{
		EchoAssert(firstAnimState->getAnim()->getAnimType() == Animation::AT_SKELETON);
		EchoAssert(secondAnimState->getAnim()->getAnimType() == Animation::AT_SKELETON);
		if (firstAnimState->getAnim()->hasScaling() != secondAnimState->getAnim()->hasScaling())
		{
			EchoLogError("property[hasScaling] mismatch between anim file [%s] and [%s] .", firstAnimState->getAnim()->getName().c_str(), secondAnimState->getAnim()->getName().c_str());
		}


		for (size_t i = 0; i < m_skeleton->getBoneCount(); i++)
		{
			Bone* pBone = m_skeleton->getBoneById(i);
			const Vector3& vBone = pBone->getPosition();

			// 获取骨骼位置
			Quaternion ori_0, ori_1;
			Vector4	   pos_0, pos_1;
			firstAnimState->calcBonePos(ori_0, pos_0, pBone, deltaTime);
			secondAnimState->calcBonePos(ori_1, pos_1, pBone, deltaTime);
			if (firstAnimState->isEffectBone(pBone) && secondAnimState->isEffectBone(pBone))
			{
				Quaternion::Slerp(oSkeletonPose.m_rotations[i], ori_0, ori_1, dstWeight, true);
				Vector4 posMerge = pos_0 * (1 - dstWeight) + pos_1 * dstWeight;
				oSkeletonPose.m_translations[i] = posMerge;
			}
			else if (firstAnimState->isEffectBone(pBone))
			{
				oSkeletonPose.m_rotations[i] = ori_0;
				oSkeletonPose.m_translations[i] = pos_0;
			}
			else if (secondAnimState->isEffectBone(pBone))
			{
				oSkeletonPose.m_rotations[i] = ori_1;
				oSkeletonPose.m_translations[i] = pos_1;
			}
			else
			{
				oSkeletonPose.m_rotations[i] = Quaternion::IDENTITY;
				oSkeletonPose.m_translations[i] = Vector4(oSkeletonPose.m_rotations[i] * vBone, 1.0f);
			}
		}
	}

	// 更新
	void AnimBlender::frameMoveInteral(i32 deltaTime)
	{
		if (!m_animState && m_partitionAnimStates.empty())
			return;

		// 1.动画融合
		SkeletonPose skeletonPose(m_skeleton);
		if (m_animState)
		{
			// 状态更新
			m_animState->frameMove(deltaTime);
			if (m_targetAnimState)
			{
				m_targetAnimState->frameMove(deltaTime);
				m_leftBlendTime = m_leftBlendTime - deltaTime;

				if ((m_leftBlendTime <= 0 && deltaTime > 0)
					|| (m_leftBlendTime >= 0 && deltaTime < 0))
				{
					m_leftBlendTime = 0;
					if (m_animState != m_targetAnimState)
					{
						m_animState->enable(false);
					}
					m_animState = m_targetAnimState;
					m_targetAnimState = NULL;
					mbNeedMerge = false;
				}
			}

			if (m_mode == BM_CROSSFADE && m_targetAnimState && m_leftBlendTime > 0.f)
			{
				float dstWeight = 1.0f - (float)m_leftBlendTime / (float)m_blendTime;
				crossFade(skeletonPose, m_animState, m_targetAnimState, dstWeight, deltaTime);
			}
			else
			{
				calcSkeletonPose(skeletonPose, m_animState, deltaTime);
			}
		}

		// 2.处理部分动画
		calcSkeletonPoseByPartitionAnimState(skeletonPose, deltaTime);

		// 3.骨架控制器
		calcSkeletonControllers(skeletonPose, deltaTime);

		// 4.计算物体空间方位
		calcObjectSpacePose(skeletonPose);

		// 5.计录最终结果
		if (m_animState && m_animState->getAnim()->hasScaling())
		{
			for (size_t i = 0; i < m_skeleton->getBoneCount(); i++)
			{
				if (boneAnimOptimized(i))
				{
					continue;
				}

				const Vector3& postion = (Vector3)skeletonPose.m_translations[i];
				const float& scale = skeletonPose.m_translations[i].w;

				if (skeletonPose.m_lookAtRotations.find(i) != skeletonPose.m_lookAtRotations.end())
					skeletonPose.m_rotations[i] = skeletonPose.m_lookAtRotations[i];

				skeletonPose.m_rotations[i].toMat4(m_AnimMatrix[i]);
				m_AnimMatrix[i].scale(scale, scale, scale);
				m_AnimMatrix[i].translate(postion);

				Bone* bone = m_skeleton->getBoneById(i);
				m_AnimMatrix[i] = bone->getInverseMatrix() * m_AnimMatrix[i];
			}
		}
		else
		{
			for (size_t i = 0; i < m_skeleton->getBoneCount(); i++)
			{
				Vector3 pos = (Vector3)skeletonPose.m_translations[i] - skeletonPose.m_rotations[i] * m_skeleton->getBoneById(i)->getPosition();

				if (skeletonPose.m_lookAtRotations.find(i) != skeletonPose.m_lookAtRotations.end())
					skeletonPose.m_rotations[i] = skeletonPose.m_lookAtRotations[i];

				skeletonPose.m_rotations[i].toMat4(m_AnimMatrix[i]);
				m_AnimMatrix[i].scale(1.0f, 1.0f, 1.0f);
				m_AnimMatrix[i].translate(pos);

				m_KeyOrientation[i] = skeletonPose.m_rotations[i] * m_skeleton->getBoneById(i)->getRotation();
				m_KeyPosition[i] = skeletonPose.m_translations[i];
				m_KeyScale[i] = 1.0f;
			}
		}
	}

	const Quaternion& AnimBlender::getAnimOrientation(ui32 idx) const
	{
		return m_KeyOrientation[idx];
	}

	const Quaternion* AnimBlender::getAnimOrientationPtr() const
	{
		return m_KeyOrientation.data();
	}

	const Vector3& AnimBlender::getAnimPosition(ui32 idx) const
	{
		return m_KeyPosition[idx];
	}

	const Vector3* AnimBlender::getAnimPositionPtr() const
	{
		return m_KeyPosition.data();
	}

	float AnimBlender::getAnimScale(ui32 idx) const
	{
		return m_KeyScale[idx];
	}

	void AnimBlender::setIsLoopSkill(bool isLoopSkill)
	{
		mbIsLoopSkill = isLoopSkill;
	}
}
