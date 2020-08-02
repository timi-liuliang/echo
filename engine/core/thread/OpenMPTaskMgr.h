#pragma once

#include <engine/core/thread/pool/CpuThreadPool.h>
#include "engine/core/base/echo_def.h"

namespace Echo
{
	/**
	 * OpenMPTaskManager 2014-10-27
	 * http://www.openmp.org
	 */
	class OpenMPTaskMgr
	{
	public:
		// Task type
		enum TaskType
		{
			TT_AnimationUpdate = 0,
			TT_EffectSystem,
		};

	public:
		~OpenMPTaskMgr();

		// instance
		static OpenMPTaskMgr* instance();

		// add
		void addTask(TaskType type, CpuThreadPool::Job* task);

		// execs
		void execTasks(TaskType type);

		// wait for finished
		void waitForAnimationUpdateComplete();

		// wait finished
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
