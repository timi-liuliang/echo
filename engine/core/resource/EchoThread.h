#pragma once

#include "engine/core/Thread/Threading.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	/**
	 * 线程对象
	 */
	class Event;
	class ThreadLoopObject
	{
	public:
		// 线程状态
		enum ThreadState
		{
			TS_Running,		// 运行中
			TS_ShutingDown,	// 已关闭
		};

	public:
		ThreadLoopObject();
		virtual ~ThreadLoopObject();

		// 启动
		virtual void start();

		// 关闭
		virtual void shutdown();

	public:
		// 活跃中
		virtual bool IsRunning();

		// 是否挂起
		virtual bool IsPause();

		// 线程处理循环
		virtual void processLoop()=0;
	protected:
		// 暂停线程
		void pause();

		// 恢复线程
		void resume();

#ifdef ECHO_PLATFORM_WINDOWS
		// 线程函数
		static unsigned __stdcall ThreadCallBackProc(void *pvArg);
#else
		// 线程函数
		static void* ThreadCallBackProc(void *pvArg);
#endif

	protected:
		void*			m_hThread;					// 线程句柄
		Event*			m_threadEvent;				// 线程事件对象
		ThreadState		m_state;					// 线程状态
		bool			mIsPause;					// 是否是挂起状态
	};

	/**
	 * 资源加载线程
	 */
	class StreamThread : public ThreadLoopObject
	{
	public:
		/** 任务优化级 */
		enum TaskLevel
		{
			TL_High,
			TL_Normal,
			TL_Low
		};

		/** 任务 */
		class Task
		{
		public:
			Task(){}
			virtual ~Task(){}

			// 处理任务
			virtual bool process() = 0;

			// 任务完成响应
			virtual bool finished() = 0;

			// 返回该事件的优先级别
			virtual TaskLevel GetLevel() { return TL_Normal; }

			// 是否重复通知
			virtual bool isRepeatNotifyIfFail()=0;

			// 是否自动释放
			virtual bool isAutoDestroy()=0;
		};
		typedef multimap<TaskLevel,Task*>::type TaskMap;
		typedef vector<Task*>::type	TaskArray;

	public:
		virtual ~StreamThread();

		// instance
		static StreamThread* instance();

		// 添加任务
		virtual ui32 addTask( Task* pThreadEvent);

		// 关闭
		virtual void shutdown();

		// 通知任务完成
		virtual void notifyTaskFinished();

		// 获取当前任务数量
		virtual int getTaskCount();

		// 获取已完成任务数量
		virtual int getTaskFinishedCount();

		// 线程处理循环
		virtual void processLoop();

	private:
		StreamThread();

		// 获取任务
		Task* getNextTask();

	protected:
		TaskMap			m_taskQueue;				// 任务队列
		TaskArray		m_taskFinished;				// 已完成任务
		EE_MUTEX(		m_mutexTask);				// 请求队列互斥量
		EE_MUTEX(		m_mutexTaskFinished);		// 完成队列互斥量
		ui32			m_totalTaskCount;			// 任务计数
	};
}
