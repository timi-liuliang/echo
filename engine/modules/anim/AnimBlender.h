#ifndef __ECHO_ANIMBLENDER_H__
#define __ECHO_ANIMBLENDER_H__

#include "engine/core/Resource/EchoThread.h"
#include "engine/core/Util/PtrMonitor.h"
#include "engine/core/Util/Array.hpp"
//#include "scl/bitset.h"

namespace Echo
{
	class Skeleton;
	class AnimState;
	class BlenderEvent;
	class ISkeletonController;
	struct SkeletonPose;
	class AnimBlender
	{
		friend class BlenderEvent;
		friend class AnimManager;
		typedef vector<ISkeletonController*>::type SkeletonControllerList;
	public:
		// 混合模式
		enum BlendMode
		{
			BM_REPLACE,		// stop src anim and start dest anim
			BM_CROSSFADE,	// blend src animation out while blending dest animation in
		};

	public:	
		// 获取唯一标识符
		ui32 getIdentifier() const { return m_identifier; }

		// 播放动画
		virtual bool blend(AnimState* pAnimState, ui32 blendTime);

		virtual void					setBlendMode(BlendMode mode);

		virtual Skeleton*				getSkeleton() const;
		virtual bool					isBlending() const;
		virtual AnimState*				getAnimState() const;
		virtual const Matrix4&			getAnimMatrix(ui32 idx) const;
		const Matrix4*					getAnimMatrixPtr() const;
		virtual const Quaternion&		getAnimOrientation(ui32 idx) const;
		const Quaternion*				getAnimOrientationPtr() const;
		virtual const Vector3&			getAnimPosition(ui32 idx) const;
		const Vector3*					getAnimPositionPtr() const;
		float							getAnimScale(ui32 idx) const;
		virtual void					setBlendAnimState(AnimState* state){ /*m_pBlendAnimState = state;*/ };

		// 每帧更新
		virtual void					frameMove(i32 deltaTime, bool isSync=false);
		virtual void					setIsLoopSkill(bool isLoopSkill);

		// 添加动画控制器
		void addSkelController(ISkeletonController* skelController);

		// 删除动画控制器
		void removeSkelController(ISkeletonController* skelController);

		// 刪除所有动画控制器
		void removeSkelController();

		// 停止所有部分动画
		void stopAllPartitionAinm();

	public:
		// 更新
		void frameMoveInteral(i32 deltaTime);

		// 融合动画
		void crossFade(SkeletonPose& oSkeletonPose, AnimState* firstAnimState, AnimState* secondAnimState, float dstWeight, i32 deltaTime) const;

		// 计算骨架位置
		bool calcSkeletonPose(SkeletonPose& oSkeletonPos, AnimState* animState, i32 deltaTime);

		// 计算部分动画对骨架影响
		bool calcSkeletonPoseByPartitionAnimState(SkeletonPose& skeletonPose, i32 deltaTime);

		// 计算骨架控制器骨架影响
		bool calcSkeletonControllers(SkeletonPose& skeletonPose, i32 deltaTime);

		void calcObjectSpacePose(SkeletonPose& skeletonPose);

		void setEffectiveBones(const array<bool,256>& bones);

	private:
		AnimBlender(Skeleton* pSkel);
		virtual ~AnimBlender();

		bool boneAnimOptimized(size_t i);

	protected:
		ui32						m_identifier;			// 唯一标识符
		Skeleton*					m_skeleton;
		BlendMode					m_mode;
		AnimState*					m_animState;
		AnimState*					m_targetAnimState;
		map<int, AnimState*>::type	m_partitionAnimStates;	// 部分动画
		SkeletonControllerList      m_skelControllers;		// 骨架控制器 
		ui32						m_blendTime;
		int							m_leftBlendTime;
		vector<Matrix4>::type		m_AnimMatrix;
		vector<Quaternion>::type	m_KeyOrientation;
		vector<Vector3>::type		m_KeyPosition;
		vector<float>::type			m_KeyScale;
		bool						mbNeedMerge;
		bool						mbIsLoopSkill;
		bool						m_enableAnimOptimize;
		array<bool,256>				m_effectiveBones;
		array<bool,256>				m_skipOptimizeBones;
	};
}

#endif