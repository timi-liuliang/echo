#pragma once

#include <thread>
#include <engine/core/util/Array.hpp>

namespace Echo
{
	/**
	 * 基于CPU的线程池实现
	 */
	class CpuThreadPool
	{
	public:
		// 构造信息
		struct Cinfo
		{
			ui32		m_numThreads;		// 创建线程数量
			bool		m_isBlocking;		// 执行工作时是否阻塞主线程

			// 构造函数
			Cinfo()
			{
				m_numThreads = 0;
				m_isBlocking = true;
			}
		};

		// 线程池开始状态
		enum StartThreadsMode
		{
			STM_OnConstruction,		// 构造后立即启动
			STM_Manually,			// 手动调用startThreads后启动
		};

		/**
		* Job Base type for all jobs
		*/
		class Job
		{
		public:
			Job(){}
			virtual ~Job(){}

			// 处理任务
			virtual bool process() = 0;

			// 任务完成后调用(主线程)
			virtual bool onFinished() { return true; }

			// 获取任务类型
			virtual int getType() = 0;
		};


		// 工作信息
		struct JobInfo
		{
			Job*	m_job;
		};

		// 工作线程状态
		struct ThreadData
		{
			CpuThreadPool*			m_threadPool;		// 对应的线程池
			int						m_threadId;			// 线程Id (0-N)
			std::thread				m_thread;			// 线程
			//scl::semaphore			m_semaphore;		// 信号量，用于暂停工作线程
			//scl::ring_buffer		m_buffer;			// 工作
			bool					m_killThread;		// 标记位，当线程被要求关闭时设置为true

			// 构造函数
			ThreadData()
				: m_threadPool( NULL)
			{
				//m_semaphore.create();
			}
		};

	public:
		CpuThreadPool(const Cinfo& info, StartThreadsMode mode=STM_OnConstruction);
		virtual ~CpuThreadPool();

		// 启动线程,只有当构造时采用STM_Manually的情况下才可以调用此函数
		void startThreads(const Cinfo& info);

		// 处理工作队列(非阻塞)
		void processJobs(Job** jobs, int numOfJobs);

		// 阻塞等待任务完成(要求当线线程为阻塞模式)
		void waitForComplete( int type);

		// 获取使用的线程数量
		int getNumThreads() const;

		// 终止
		void stop();

	protected:
		// 线程入口函数
		static void threadMainForwarder(ThreadData& threadData);

		// 工作线程主函数
		void threadMain(int threadIndex);

	private:
		Cinfo						m_info;								// 当前线程数量
		array<ThreadData, 32>		m_workerThreads;					// 工作线程数据
		//array<scl::semaphore, 24>	m_semaphores;						// 信号量，用于主线程等待
		//array<int, 24>				m_numOfJobsProcessed;				// 工作线程处理的任务数量，对应工作类型
	};
}
