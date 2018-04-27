#ifndef __ECHO_ANIMSTATE_H__
#define __ECHO_ANIMSTATE_H__

#include "Animation.h"

namespace Echo
{
	class Animation;
	class Quaternion;
	class Vector4;
	class AnimKeyFrame;


	class AnimState
	{
	public:
		AnimState(Animation* pAnim);
		virtual ~AnimState();

		// 重置
		void reset();

		virtual void enable(bool bEnable);
		virtual void enableLoop(bool bEnable);

		// 设置循环次数
		void setLoopTime(int time) { m_loopTime = time; }

		virtual void setLength(ui32 length);

		// 设置动画播放位置
		void setTimePosition(ui32 timePos);

		// 获取动画播放位置
		ui32 getTimePosition() const;

		virtual Animation*		getAnim() const;
		virtual bool			isEnable() const;
		virtual bool			isLoop() const;
		virtual ui32			getLength() const;
		virtual ui32			getCurrentFrameIndex() const;
		virtual ui32			getNextFrameIndex() const;
		virtual ui32			getPrevFrameIndex() const;
		virtual AnimKeyFrame*	getCurrentAnimKeyFrame() const;
		virtual AnimKeyFrame*	getAnimKeyFrame(ui32 index) const;

		// 更新
		virtual void frameMove(i32 deltaTime);

		// 计算指定骨骼当前位置旋转
		bool calcBonePos(Quaternion& oRotation, Vector4& oTranslate, const Bone* bone, i32 deltaTime);

		// 设置播放速度
		void setSpeed(float speed) { m_speed = speed; }

		// 获取影响骨骼权重
		virtual real32 getWeight(ui32 index) const;

		// 获取混合权重
		float getBlendWeight() const { return m_blendWeight; }

		// 获取部分主骨骼索引
		int getRootBone() { return m_anim ? m_anim->getPartionRootBone() : -1; }

		// 是否影响指定骨骼
		bool isEffectBone(const Bone* bone);

		// 是否是局部动画
		bool isPartition() const;

		// 设置动画融合时间(毫秒)
		void setBlendTime(const ui32 time) { m_blendTime = time; }

		// 当前播放是否已经到最后
		bool isEnd() { return m_bIsEnd; }

	protected:
		Animation*			m_anim;		// 所控制动画
		bool				m_bEnable;
		ui32				m_loopTime;		// 循环次数
		ui32				m_length;		// 动画长度
		i32					m_timePos;		// 当前时间刻度
		ui32				m_nFrame;
		float				m_speed;		// 控制动画播放速度
		int					m_layer;		// 所属动画层级
		ui32				m_blendTime;	// 混合时长
		float				m_blendWeight;	// 当前混合权重
		bool				m_bIsEnd;		// 当次动画是否播完
	};
}

#endif