#include "engine/core/Util/AssertX.h"
#include "engine/core/log/LogManager.h"
#include "CpuThreadPool.h"
#include "engine/core/Thread/Threading.h"

namespace Echo
{
	// 构造函数
	CpuThreadPool::CpuThreadPool(const CpuThreadPool::Cinfo& info, CpuThreadPool::StartThreadsMode mode)
	{
	//	m_numOfJobsProcessed.assign(0);

	//	// 信号量初始化
	//	for (scl::semaphore& sem : m_semaphores)
	//	{
	//		sem.create();
	//	}
	//		
	//	// 开启工作线程
	//	if (mode == STM_OnConstruction)
	//	{
	//		startThreads( info);
	//	}
	}

	// 析构函数
	CpuThreadPool::~CpuThreadPool()
	{

	}

	// 启动线程,只有当构造时采用STM_Manually的情况下才可以调用此函数
	void CpuThreadPool::startThreads(const Cinfo& info)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else

		//EchoAssert(!m_info.m_numThreads && info.m_numThreads>0);

		//// 确定线程数量
		//m_info = info;
		//if (m_info.m_numThreads > m_workerThreads.size() || m_info.m_numThreads<1)
		//{
		//	EchoLogWarning( "You requested more or less threads than the CpuThreadPool supports - see MAX_NUM_THREADS");
		//	m_info.m_numThreads = std::max<int>(m_info.m_numThreads, 1);
		//	m_info.m_numThreads = std::min<int>(m_info.m_numThreads, m_workerThreads.size());
		//}

		//// 创建线程
		//for (ui32 i = 0; i < m_info.m_numThreads; i++)
		//{
		//	ThreadData& threadData = m_workerThreads[i];
		//	threadData.m_threadPool = this;
		//	threadData.m_killThread = false;

 	//		// Worker thread IDs start from 1, 0 is reserved for the main thread
 	//		threadData.m_threadId = i + 1;
  //          auto localThread = std::thread(CpuThreadPool::threadMainForwarder, std::ref(threadData));
 	//		threadData.m_thread.swap(localThread);
 	//		threadData.m_buffer.alloc(1024 * 32);
		//}
#endif
	}

	// 线程入口函数
	void CpuThreadPool::threadMainForwarder(CpuThreadPool::ThreadData& threadData)
	{
		threadData.m_threadPool->threadMain(threadData.m_threadId - 1);
	}

	// 工作线程主函数
	void CpuThreadPool::threadMain(int threadIndex)
	{
	//	ThreadData& threadData = m_workerThreads[threadIndex];

	//	// 主循环
	//	while (true)
	//	{
	//		if (threadData.m_buffer.used()>=sizeof(JobInfo))
	//		{
	//			JobInfo jobInfo;
	//			threadData.m_buffer.peek(&jobInfo, sizeof(JobInfo));
	//			jobInfo.m_job->process();

	//			threadData.m_buffer.drop(sizeof(JobInfo));

	//			// 唤醒主线程
	//			if (m_info.m_isBlocking)
	//				m_semaphores[jobInfo.m_job->getType()].post();
	//		}
	//		else
	//		{
	//			// 终止线程
	//			if (threadData.m_killThread)
	//				return;

	//			// 线程挂起
	//			threadData.m_semaphore.wait();
	//		}
	//	}
	}

	// 处理工作队列
	void CpuThreadPool::processJobs(CpuThreadPool::Job** jobs, int numOfJobs)
	{
#ifdef ECHO_PLATFORM_HTML5
		for( int i=0; i<numOfJobs; i++)
		{
			jobs[i]->process();
		}
#else
		//for (int i = 0; i < numOfJobs; i++)
		//{
		//	// 获取最空闲线程索引
		//	int taskNum = 65535;
		//	int threadIdx = 0;
		//	ThreadData& threadData = m_workerThreads[threadIdx];

		//	JobInfo jobInfo;
		//	jobInfo.m_job = jobs[i];

		//	const int freelen = threadData.m_buffer.free();
		//	if (freelen > sizeof(JobInfo))
		//	{
		//		threadData.m_buffer.write(&jobInfo, sizeof(JobInfo));
		//		m_numOfJobsProcessed[jobInfo.m_job->getType()]++;

		//		// 唤醒对应工作线程
		//		threadData.m_semaphore.post();
		//	}
		//	else
		//	{
		//		EchoLogWarning("CpuThreadPool::processJobs. RingBuffer don't have enough memory space.");
		//	}
		//}
#endif
	}

	// 阻塞等待任务完成
	void CpuThreadPool::waitForComplete(int type)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		//EchoAssert(m_info.m_isBlocking);

		//// 阻塞主线程
		//if (m_info.m_isBlocking)
		//{
		//	for (int i = 0; i < m_numOfJobsProcessed[type]; i++)
		//		m_semaphores[type].wait();

		//	m_numOfJobsProcessed[type] = 0;
		//}
#endif
	}

	// 获取使用的线程数量
	int CpuThreadPool::getNumThreads() const
	{
		return m_info.m_numThreads;
	}

	// 终止
	void CpuThreadPool::stop()
	{
		//// 执行退出
		//for (int i = 0; i < getNumThreads(); i++)
		//{
		//	// 获取线程索引
		//	ThreadData& threadData = m_workerThreads[i];

		//	// 唤醒对应工作线程
		//	threadData.m_killThread = true;
		//	threadData.m_semaphore.post();

		//	// 等待结束
		//	threadData.m_thread.join();
		//}
	}
}