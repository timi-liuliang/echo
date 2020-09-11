#include "OpenMPTaskMgr.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	OpenMPTaskMgr::OpenMPTaskMgr()
	{
		Echo::CpuThreadPool::Cinfo info;
		info.m_numThreads = std::thread::hardware_concurrency();
		info.m_isBlocking = true;

		m_threadPool = EchoNew(Echo::CpuThreadPool(info));
	}

	OpenMPTaskMgr::~OpenMPTaskMgr()
	{
		size_t totalNums = m_animationUpdateTasks.size();
		for (size_t i = 0; i < totalNums; ++i)
		{
			EchoSafeDelete(m_animationUpdateTasks[i], Job);
		}

		EchoSafeDeleteContainer(m_effectSystemUpdateTasks, Job);

		m_threadPool->stop();
		EchoSafeDelete(m_threadPool, CpuThreadPool);
	}

	OpenMPTaskMgr* OpenMPTaskMgr::instance()
	{
		static OpenMPTaskMgr* inst = EchoNew(OpenMPTaskMgr);
		return inst;
	}

	void OpenMPTaskMgr::addTask(TaskType type, CpuThreadPool::Job* task)
	{
		switch (type)
		{
			case TT_AnimationUpdate: m_animationUpdateTasks.emplace_back(task);			break;
			case TT_EffectSystem:	 m_effectSystemUpdateTasks.emplace_back(task);		break;
			default:				 EchoLogError("OpenMPTaskMgr::Unknown task type");	break;
		}
	}

	void OpenMPTaskMgr::execTasks(TaskType type)
	{
		switch (type)
		{
			case TT_AnimationUpdate:
			{
				if (!m_animationUpdateTasks.empty())
				{
					m_animationUpdateTasksFinished = m_animationUpdateTasks;
					m_animationUpdateTasks.clear();
					m_threadPool->processJobs(m_animationUpdateTasksFinished.data(), int(m_animationUpdateTasksFinished.size()));
				}
			}
			break;

			case TT_EffectSystem:
			{
				if (!m_effectSystemUpdateTasks.empty())
				{
					m_effectSystemUpdateTasksFinished = m_effectSystemUpdateTasks;
					m_effectSystemUpdateTasks.clear();
					m_threadPool->processJobs(m_effectSystemUpdateTasksFinished.data(), int(m_effectSystemUpdateTasksFinished.size()));
				}
			}
			break;
		}
	}

	void OpenMPTaskMgr::waitForAnimationUpdateComplete()
	{
		m_threadPool->waitForComplete(TT_AnimationUpdate);
		EchoSafeDeleteContainer(m_animationUpdateTasksFinished, Job);
	}

	void OpenMPTaskMgr::waitForEffectSystemUpdateComplete()
	{
		m_threadPool->waitForComplete(TT_EffectSystem);

		for (CpuThreadPool::Job* job : m_effectSystemUpdateTasksFinished)
		{
			job->onFinished();
		}

		EchoSafeDeleteContainer(m_effectSystemUpdateTasksFinished, Job);
	}
}
