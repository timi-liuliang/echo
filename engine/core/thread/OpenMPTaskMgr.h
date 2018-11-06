#pragma once

#include <engine/core/Thread/Pool/CpuThreadPool.h>
#include "engine/core/Base/echo_def.h"

namespace Echo
{
	/**
	 * 多线程任务处理管理器 2014-10-27 
	 * 参考 http://www.openmp.org
	 */
	class OpenMPTaskMgr
	{
	public:
		// 任务类型
		enum TaskType
		{
			TT_AnimationUpdate = 0,		// 动画更新
			TT_EffectSystem,			// 更新特效
		};

	public:
		~OpenMPTaskMgr();

		// instance
		static OpenMPTaskMgr* instance();

		// 添加任务
		void addTask(TaskType type, CpuThreadPool::Job* task);

		// 执行任务(相关逻辑会有一帧延迟,并无大碍)
		void execTasks(TaskType type);

		// 等待动画更新完成
		void waitForAnimationUpdateComplete();

		// 等待特效更新完成
		void waitForEffectSystemUpdateComplete();

	private:
		OpenMPTaskMgr();

	private:
		vector<CpuThreadPool::Job*>::type	m_animationUpdateTasks;				// 动画更新任务
		vector<CpuThreadPool::Job*>::type	m_animationUpdateTasksFinished;		// 动画更新任务已完成
		vector<CpuThreadPool::Job*>::type	m_effectSystemUpdateTasks;			// 特效更新任务
		vector<CpuThreadPool::Job*>::type	m_effectSystemUpdateTasksFinished;	// 特效更新任务
		CpuThreadPool*						m_threadPool;						// 线程池
	};
}