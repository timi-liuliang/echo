////////////////////////////////////////////////////////////////////////////////
//	thread.h
//	Thread类
//	2010.09.11 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/ptr.h"

namespace scl {

const int INVALID_THREAD_ID = -1;

#ifdef SCL_WIN
const int MAX_THREAD_COUNT = 64; // 最大线程数量
#endif

#ifdef SCL_LINUX
const int MAX_THREAD_COUNT = 256; // 最大线程数量
#endif

#ifdef SCL_APPLE
const int MAX_THREAD_COUNT = 64; // 最大线程数量
#endif

#ifdef SCL_ANDROID
const int MAX_THREAD_COUNT = 64; // 最大线程数量
#endif

#ifdef SCL_HTML5
const int MAX_THREAD_COUNT = 64; // 最大线程数量
#endif


class thread_info;

////////////////////////////////////////////////////////////////////////////////
//	class thread
////////////////////////////////////////////////////////////////////////////////
class thread
{
public:
	typedef void* (*ThreadFunction)(void*, int*);

public:
	thread();
	~thread();

	int				start				(ThreadFunction function, void* param = NULL, bool start_at_once = true, bool auto_detach = false);
	void			resume				();
	void			set_affinity_mask	(int cpuid = 0);
	bool			is_running			() const;
	bool			is_main_thread		() const;

	void			send_stop_signal	();
	bool			wait				(const int timeout = -1, bool check_thread = true);  // wait for thread close, timeout is millionseconds, if timeout == -1 wait until thread exits.
	bool			force_kill			(); //强制杀死线程，非常不安全

	//static members
	static	int		self				();			//线程id
	static	int		self_process		(); //进程id
	static	int		self_index			();	//线程在g_thread_infos数组中的index
	static	bool	exists				(const int thread_id);

	static	int		main_thread_id		()	{ return m_main_thread_id; }


public:
	enum SIGNAL
	{
		SIGNAL_NORMAL	= 0, //正常运行
		SIGNAL_STOP		= 1, //停止
		SIGNAL_HUNG		= 2, //挂起
	};

	class info
	{
	public:
		void*					handle;				//线程在操作系统中的句柄

		uint					process_id;			//进程id
		int						id;					//线程id
		int						index;				//线程在g_threads中的index

		uint64					pthread_id;			//pthread线程id  仅在linux下有效

		int						parent_thread_id;	//父线程的id

		thread::ThreadFunction	function;			//线程的执行函数
		void*					param;				//线程执行函数的参数
		int						signal;				//线程信号，其他线程通过设置这个信号和线程函数通信

		bool					auto_detach;		//线程函数推出后，线程资源是否自动释放，注意，如果该值为false，那么创建线程（thread::start）和释放线程(thread::start)的线程必须是同一个线程
		void*					exit_code;			//线程函数退出代码
		volatile bool			is_running;			//线程函数是否正在运行

		void clear()
		{
			handle		= NULL;

			process_id	= -1;
			id			= -1;
			index		= -1;
			pthread_id	= -1;

			function	= NULL; 
			param		= NULL; 
			signal		= 0; 

			exit_code	= NULL;
			auto_detach = false;
			is_running	= false;
		}
	};

private:
	static int __init_main_thread_info();

private:
	info		m_info;
	static int	m_main_thread_id;
};

template <typename T>
class class_thread : public thread
{
public:
	struct class_thread_info
	{
		class_function	function;
		void*			ptr;

		class_thread_info() :
			function	(NULL), 
			ptr			(NULL)	{}
	};

	typedef void* (T::*functionT)(int*);
	int start(functionT func, T* param = NULL, bool startAtOnce = true)
	{
		m_class_info.function	= reinterpret_cast<class_function>(func);
		m_class_info.ptr		= static_cast<void*>(param);
		return thread::start(_class_thread_function_dispatcher, this, startAtOnce);
	}

private:
	static void* _class_thread_function_dispatcher(void* param, int* singal);
	class_thread_info	m_class_info;
};


template <typename T>
void* scl::class_thread<T>::_class_thread_function_dispatcher(void* param, int* signal)
{
	class_thread*	t		= static_cast<class_thread*>(param);
	T*				c		= static_cast<T*>(t->m_class_info.ptr);
	functionT		func	= reinterpret_cast<functionT>(t->m_class_info.function);

	return (c->*func)(signal);
}

class mutex
{
public:
	mutex		();
	~mutex		();
	void create	();
	void lock	();
	void unlock	();
private:
	void*		m_handle;
};

class mutex_lock
{
public:
	mutex_lock	(mutex* m);
	~mutex_lock	();
	void unlock	();
private:
	mutex*		m_mutex;
};


//信号量
class semaphore
{
public:
	semaphore();
	~semaphore();
	bool create(const int count = 0);
	void post();
	bool wait();

private:
	void* m_handle;
};



bool compare_and_swap(volatile uint* i,		uint oldval,	uint newval);
bool compare_and_swap(volatile int* i,		int oldval,		int newval);
bool compare_and_swap(volatile ptr_t* i,	ptr_t oldval,	ptr_t newval);

int atomic_inc(volatile int* i);
int atomic_dec(volatile int* i);

extern mutex __inner_error_log_mutex;

} //namespace scl

