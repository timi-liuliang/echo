#pragma once

#include "Skeleton.h"
#include "Engine/core/Resource/ResourceManager.h"

namespace Echo
{
	class Bone;
	class AnimBlender;
	class DataStream;
	class AnimState;
	class Animation;
	class AnimSystem : public Resource
	{
	public:
		AnimSystem(const String& name, bool isManual);
		virtual~AnimSystem();

		// 更新
		void tick(i32 elapsedTime, bool isSync=false);

		// 播放动画
		void playAnimation(AnimState* animState, ui32 blendTime = 200);

		// 停止所有动画
		void stopAllAnimation() {}

		// 获取指定骨骼当前位置
		Vector3 getBonePos(const Bone* bone);

		// 是否暂停
		bool isPause() const { return m_isPause; }

		// 暂停
		void setPause(bool isPause) { m_isPause = isPause; }

	public:
		// 设置骨架
		void setSkeleton(const char* skeleton, AnimBlender* externalBlender = NULL);

		// 获取骨架
		Skeleton* getSkeleton() { return m_skeleton; }

		// 获取混合器
		AnimBlender* getBlender() { return m_blenderExternal ? m_blenderExternal : m_blender; }

		// 添加动画
		void addAnimation(const char* animation);

		// 获取动画状态
		AnimState* getAnimState(const char* animName);

		// 获取动画状态但不创建
		AnimState* getAnimStateNotCreate(const char* animName);

		// 获取动画
		const vector<Animation*>::type& getAnimations() { return m_animations; }

		// 判断动画是否存在
		bool isAnimationExist(const char* fileName);

		// 清空动画
		void cleanAnimations();

		// 保存
		void save(const char* filePath);

	protected:
		// 资源相关接口实现
		virtual size_t calculateSize() const;
		virtual bool prepareImpl(Echo::DataStream* stream) { return true; }
		virtual bool loadImpl();
		virtual void unloadImpl();

		// 检测合法性
		bool checkup();

	protected:
		Skeleton*					m_skeleton;			// 对应骨架
		vector<Animation*>::type	m_animations;		// 可播放动画
		vector<AnimState*>::type	m_animStates;		// 动画状态
		AnimBlender*				m_blender;			// 动画混合器
		AnimBlender*				m_blenderExternal;	// 外部动画混合器
		bool						m_isPause;			// 暂停
	};


	/**
	* 动画系统工厂
	*/
	class AnimSystemManager : public ResourceManager
	{
		__DeclareSingleton(AnimSystemManager);

	public:
		AnimSystemManager() 
		{
			__ConstructSingleton;
		}
		virtual ~AnimSystemManager() 
		{
			__DestructSingleton;
		}

		// 创建动画系统
		AnimSystem* createAnimSystem(const String& name, bool isManual = false);

	protected:
		// 实现
		virtual Resource* createImpl(const String& name, bool isManual);
	};
}