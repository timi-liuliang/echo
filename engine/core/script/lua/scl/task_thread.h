#include "scl/thread.h"
#include "scl/ring_queue.h"


namespace scl {

////////////////////////////////////////////////////////////////////////
// class task_thread
//
// 使用一个thread来处理一个task队列
// 内部使用一个等待处理的环形队列来保存未处理的task，外部产生task的线程（务必是单线程）来add多个task到这个队列，task_thread会在另外的工作线程中来逐个处理这些task
// 使用一个处理后的task组成的环形队列来保存处理后的信息，方便外界处理。处理task的线程（务必是单线程）调用process_task_thread_result来处理结果。
//
// 具体使用示例：
//
//		#include "scl/task_thread.h"
//
//		using scl::task_thread;
//		using scl::task;
//
//		struct task_param { int i; };
//		struct task_result { int r;};
//
//		bool task_func(void* param, void* result)
//		{
//			task_param* p	= static_cast<task_param*>(param);
//			task_result* r	= static_cast<task_result*>(result);
//			return true;  //这里返回true会导致task被添加到result队列中。注意！无论返回true还是false，task都会被从待处理队列中移除。
//		}
//
//		bool task_result_func(void* result)
//		{
//			task_result* r = static_cast<task_result*>(result);
//			return true;  //这里返回true会导致task被从result队列中移除，返回false则不会被移除，下次调用process_task仍然会处理。
//		}
//
//		int main()
//		{
//			task_thread* pool = new task_thread[8];
//			int freeIndex = scl::find_free_task_thread(pool, 8);
//			task t(task_func, new task_param, new task_result);
//			if (freeIndex >= 0 && freeIndex < 8) pool[freeIndex]->add(t);
//			while (scl::process_task_thread_result(pool, 8, task_result_func) == 0);
//			delete t.param; delete t.result; delete[] pool;
//			return 0;
//		}
////////////////////////////////////////////////////////////////////////

class task_thread;

class task
{
public:
	typedef void (*funcT)(void*, void*, task_thread*);

	funcT	func;
	void*	param;
	void*	result;

	task() : func(NULL), param(NULL), result(NULL) {}
	task(funcT f, void* p = NULL, void* r= NULL) : func(f), param(p), result(r) {}
};

class task_thread
{
public:
#ifdef SCL_WIN
	static const int						MAX_TASK_COUNT = 256;
#endif
#ifdef SCL_LINUX
	static const int						MAX_TASK_COUNT = 10000;
#endif
#ifdef SCL_APPLE
	static const int						MAX_TASK_COUNT = 1024;
#endif
#ifdef SCL_ANDROID
	static const int						MAX_TASK_COUNT = 1024;
#endif
#ifdef SCL_HTML5
	static const int						MAX_TASK_COUNT = 256;
#endif

public:
	task_thread() : m_resultThreadID(-1), m_userData(NULL), m_processedCount(0), m_processedTime(0), m_index(-1), m_ignoreResult(false) {}
	~task_thread();

	void			start				();
	bool			add					(const task& t); 
	int				free_count			() const			{ return m_tasks.free() + m_results.free(); }
	bool			is_running			() const			{ return m_thread.is_running(); }
	bool			process_result		(task::funcT func, bool check_thread = false); 
	int				process_all_results	(task::funcT func, int maxProcessCount = -1); //maxProcessCount是最大可处理的数量，如果为-1，表示要处理完所有请求
	void			set_ignore_result	(bool v) { m_ignoreResult = v; }
	void			stop				();
	bool			has_result			() const { return m_results.used() > 0; }
	bool			has_task			() const { return m_tasks.used() > 0; }
	void			set_log_buffer_size	(const int v) { m_logBufferSize = v ;}

	//任务统计属性
	int				index				() const		{ return m_index;			}
	int				processed_count		() const		{ return m_processedCount;	}
	uint64			processed_time		() const		{ return m_processedTime;	}
	void			set_user_data		(void* data)	{ m_userData = data;		}
	void			set_index			(const int i)	{ m_index = i;				}
	int				task_count			()	const		{ return m_tasks.used();	}
	int				result_count		()	const		{ return m_results.used();	}
	
private:
	static void*	thread_func			(void* param, int* signal);

private:
	scl::thread								m_thread;
	scl::ring_queue<task, MAX_TASK_COUNT>	m_tasks;			//未处理任务队列
	scl::ring_queue<task, MAX_TASK_COUNT*2>	m_results;			//已处理任务结果的队列
	int										m_resultThreadID;	//结果处理线程的thread id，用于检查结果处理是否都在同一个线程中执行
	volatile	void*						m_userData;			//用户自定义数据
	volatile	int							m_processedCount;	//已处理数量
	volatile	uint64						m_processedTime;	//处理第一个任务时的时间
	int										m_index;			//线程的index
	bool									m_ignoreResult;		//忽略处理结果
	int										m_logBufferSize;	//线程用到的日志缓冲区大小
};

//从一个task_thread数组中找到空闲队列最大的task_thread的下标索引
int				find_free_task_thread		(const task_thread* task_threads, const int max_thread_count);

//主线程处理结果的函数
int				process_task_thread_result	(task_thread* task_threads, const int max_thread_count, task::funcT func);
int				process_task_thread_all_results(task_thread* task_threads, const int max_thread_count, task::funcT func, int maxProcessCount);

} //namespace scl

