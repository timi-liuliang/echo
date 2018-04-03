////////////////////////////////////////////////////////////////////////////////
//	thread.cpp
//	Thread类
//	2010.09.11 caolei
////////////////////////////////////////////////////////////////////////////////


#include "scl/thread.h"
#include "scl/assert.h"
#include "scl/time.h"
#include "scl/array.h"

#ifdef SCL_WIN
#include <windows.h>
#include <process.h>
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#endif

#if defined(SCL_LINUX) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <semaphore.h>
#endif

#if defined(SCL_APPLE)
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <mach/error.h>
#include <dispatch/dispatch.h>
#endif

namespace scl {

mutex __inner_error_log_mutex;

#if defined(SCL_LINUX)

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/syscall.h>

inline pid_t gettid() 
{ 
	return syscall(SYS_gettid);
}

#endif

#if defined(SCL_APPLE)
inline mach_port_t gettid()
{
    return pthread_mach_thread_np(pthread_self());
}
#endif

#if defined(SCL_HTML5)
inline pid_t gettid()
{
	return 0;
}
#endif

#ifdef SCL_ANDROID
#include <unistd.h>
#endif

//scl::array<thread*, MAX_THREAD_COUNT>	g_thread_infos;

volatile thread* g_threads[MAX_THREAD_COUNT] = { NULL };

int __g_threads_getFreeSlot(thread* t)
{
	int global_index = -1;
	for (int i = 0; i < MAX_THREAD_COUNT; ++i)
	{
		if (compare_and_swap((void**)(&(g_threads[i])), NULL, t))
		{
			global_index = i;
			break;
		}
	}
	if (global_index == -1)
	{
		//线程数组已满
		assert(false);
		return -1;
	}
	return global_index;
}

int thread::__init_main_thread_info()
{
#ifdef SCL_HTML5
	return 0;
#endif

	static thread main_thread;
	int main_thread_id = thread::self();

	int index = __g_threads_getFreeSlot(&main_thread);

	info& _i = main_thread.m_info;
	_i.handle		= NULL;
	_i.process_id	= thread::self_process();
	_i.id			= main_thread_id;
	_i.index		= index;

#if defined(SCL_LINUX) || defined(SCL_APPLE)
	_i.pthread_id	= reinterpret_cast<uint64>(pthread_self());
#elif defined(SCL_ANDROID)
	_i.pthread_id	= pthread_self();
#else
	_i.pthread_id	= 0;
#endif

	return main_thread_id;
}
int thread::m_main_thread_id = thread::__init_main_thread_info();

#ifdef SCL_WIN
uint __stdcall PlatformThreadFunction(void* p)
{
	if (NULL == p)
		return 0;

	thread::info* pinfo = static_cast<thread::info*>(p);
	pinfo->id			= ::GetCurrentThreadId();
	pinfo->process_id	= ::GetCurrentProcessId();

	//开始执行
	pinfo->is_running 	= true;
	assert(pinfo->function);
	pinfo->exit_code 	= pinfo->function(pinfo->param, &(pinfo->signal));
	assert(pinfo);
	pinfo->is_running 	= false;

	assert(pinfo->index >= 0 && pinfo->index < MAX_THREAD_COUNT);
	g_threads[pinfo->index] = NULL;

	return 0;
}
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
void* PlatformThreadFunction(void* p)
{
	if (NULL == p)
		return NULL;

	thread::info* pinfo = static_cast<thread::info*>(p);
	pinfo->process_id	= getpid();
	pinfo->id			= gettid();
#ifdef SCL_ANDROID
	pinfo->pthread_id	= pthread_self();
#else
	pinfo->pthread_id	= reinterpret_cast<uint64>(pthread_self());
#endif

	while (pinfo->signal == thread::SIGNAL_HUNG)
		scl::sleep(1);

	//开始执行
	pinfo->is_running = true;
	assert(pinfo->function);
	pinfo->exit_code = pinfo->function(pinfo->param, &(pinfo->signal));
	assert(pinfo);
	pinfo->is_running = false;

	assert(pinfo->index >= 0 && pinfo->index < MAX_THREAD_COUNT);
	g_threads[pinfo->index] = NULL;
	
	return NULL;
}
#endif


thread::thread()
{
	m_info.clear();
}


thread::~thread() 
{
	//在线程关闭前，就析构了thread对象，这会导致g_threads数组中存在野指针
	if (m_info.is_running)
	{
		assert(false);
		wait();
	}

#ifdef SCL_WIN
	CloseHandle(m_info.handle);
	m_info.handle = NULL;
#endif
}

int thread::start(ThreadFunction function, void* param, bool start_at_once, bool auto_detach)
{
	assert(m_info.id == -1 && m_info.handle == NULL);

	m_info.clear();

	//找到一个空闲的g_threads的位置
	m_info.index = __g_threads_getFreeSlot(this);
	m_info.function			= function;
	m_info.param			= param;
	m_info.signal			= SIGNAL_NORMAL;
	m_info.parent_thread_id = thread::self();
	m_info.auto_detach		= auto_detach;

#ifdef SCL_WIN
	m_info.handle = reinterpret_cast<void*>(_beginthreadex(NULL, 0, PlatformThreadFunction, &m_info, CREATE_SUSPENDED, NULL));
	if (0 == m_info.handle)
	{
		assertf(0, "create thread failed! errno[%d]", GetLastError());
		return -1;
	}

	if (start_at_once)
		resume();

#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	if (!start_at_once)
		m_info.signal = SIGNAL_HUNG;

	int r = 0;
	pthread_attr_t attr;
	r = pthread_attr_init(&attr);
	if (r != 0)
	{
		assertf(0, "pthread_attr_init failed! errno[%d]", errno);
		return -1;
	}
	if (auto_detach)
	{
		r = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (r != 0)
		{
			assertf(0, "pthread_attr_setdetachstate failed! errno[%d]", errno);
			return -1;
		}
	}

	r = pthread_create(
			(pthread_t*)&(m_info.handle),
			&attr,
			PlatformThreadFunction,
			&m_info);
	if (r != 0)
	{
		assertf(0, "create thread failed! errno[%d]", errno);
		return -1;
	}
#endif

	//这里必须等待线程启动并设置is_running后才能返回
	if (start_at_once) while(!m_info.is_running);
	
	return 0;
}

void thread::resume()
{
	if (m_info.is_running)
	{
		assert(0);
		return;
	}

#ifdef SCL_WIN
	ResumeThread(m_info.handle);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	m_info.signal = SIGNAL_NORMAL;
#endif

	m_info.is_running = true;
}

void thread::set_affinity_mask(int cpuid)
{
	if (m_info.is_running)
	{
		assert(0);
		return;
	}
	
#ifdef SCL_WIN
	uint mask = (1 << cpuid);
 	int result = SetThreadAffinityMask(m_info.handle, mask);
	if (0 == result)
		assert(0);
#endif
    
#ifdef SCL_APPLE
	thread_affinity_policy_data_t policy = { cpuid };
	thread_port_t mach_thread = pthread_mach_thread_np((pthread_t)m_info.handle);
	kern_return_t ret = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
	if (ret != KERN_SUCCESS)
	{
		assert(0);
	}

#endif

#ifdef SCL_ANDROID
    assert(0);
#endif

#ifdef SCL_LINUX
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpuid, &mask);
	if(sched_setaffinity(0, sizeof(mask), &mask) == -1)
		assert(false);
#endif
}

bool thread::wait(const int timeout, bool check_thread)
{
	bool thread_exited = false;
	if (check_thread && m_info.parent_thread_id != thread::self())
	{
		assertf(false, "thread::start and thread::wait are not in the same thread!");
		return false;
	}

#ifdef SCL_WIN
	const DWORD r = ::WaitForSingleObject(m_info.handle, timeout);
	thread_exited = (r == WAIT_OBJECT_0);
#endif

#ifdef SCL_APPLE
    assert(0);
#endif

#if defined(SCL_ANDROID)
    assert(0);
#endif
 
#if defined(SCL_LINUX)
	int r = -1;
	if (timeout == -1)
		r = pthread_join((pthread_t)(m_info.handle), NULL);
	else
	{
		struct timespec wait_time;
		wait_time.tv_sec = timeout / 1000;
		wait_time.tv_nsec = (timeout % 1000) * 1000000;
		r = pthread_timedjoin_np((pthread_t)(m_info.handle), NULL, &wait_time);
	}
	thread_exited = (r == 0);
#endif

	if (thread_exited)
	{
		for (int i = 0; i < MAX_THREAD_COUNT; ++i)
		{
			volatile thread* t = g_threads[i];
			if (NULL == t)
			{
				//assert(false);
				continue;
			}
			if (t->m_info.id == m_info.id)
			{	
				g_threads[i] = NULL; //TODO 线程安全？
				break;
			}
		}
	}

	m_info.clear();
	return thread_exited;
}

int thread::self()
{
#ifdef SCL_WIN
	return ::GetCurrentThreadId();
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	return gettid();
#endif
}


int thread::self_process()
{
#ifdef SCL_WIN
	return ::GetCurrentProcessId();
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return getpid();
#endif
}

int thread::self_index()
{
	int id = self();
	for (int i = 0; i < MAX_THREAD_COUNT; ++i)
	{
		if (NULL != g_threads[i] && g_threads[i]->m_info.id == id)
			return i;
	}
	return -1;
}

bool thread::exists(const int thread_id)
{
	for (int i = 0; i < MAX_THREAD_COUNT; ++i)
	{
		if (NULL != g_threads[i] && 
			g_threads[i]->m_info.id == thread_id && 
			g_threads[i]->m_info.is_running)
			return true;	
	}
	return false;
}

bool thread::is_running() const
{
	return m_info.is_running;
}


bool thread::is_main_thread() const
{
	assert(m_main_thread_id > 0);
	return m_info.id == m_main_thread_id;
}

void thread::send_stop_signal()
{
	m_info.signal = SIGNAL_STOP;
}

bool thread::force_kill()
{
	bool result = false;

	assert(m_info.handle);
#ifdef SCL_WIN
	const DWORD r = ::TerminateThread(m_info.handle, 0);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
	//linux下没有强制终止线程的方法，所以这里使用send_stop_signal通知手动检查signal的点，并调用pthread_cancel来中断阻塞的系统调用
	send_stop_signal();
	int r = pthread_cancel(reinterpret_cast<pthread_t>(m_info.handle));
#endif

#if defined(SCL_ANDROID) || defined(SCL_HTML5)
	send_stop_signal();
	//TODO android没实现
	//android下考虑使用pthread_kill(id, SIGUSR1，但是线程函数必须要处理SIGUSR1信号
	int r = -1;
	assert(0);
#endif
	
	result = (r != 0);

	if (result)
		result = wait();

	if (result)
		m_info.is_running = false;
	return result;
}

mutex::mutex() : m_handle(NULL)
{
	create();
}

mutex::~mutex()
{
#ifdef SCL_WIN
	CRITICAL_SECTION* pcs = static_cast<CRITICAL_SECTION*>(m_handle);
	if (NULL != pcs)
	{
		DeleteCriticalSection(pcs);
		free(pcs);
		m_handle = NULL;
	}
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	pthread_mutex_t* pmutex = static_cast<pthread_mutex_t*>(m_handle);
	if (NULL != pmutex)
	{
		pthread_mutex_destroy(pmutex);
		free(pmutex);
		m_handle = NULL;
	}
#endif
}

void mutex::lock()
{
	if (NULL == m_handle)
		create();
#ifdef SCL_WIN
	CRITICAL_SECTION* pcs = static_cast<CRITICAL_SECTION*>(m_handle);
	EnterCriticalSection(pcs);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	pthread_mutex_t* pmutex = static_cast<pthread_mutex_t*>(m_handle);
	pthread_mutex_lock(pmutex);
#endif
}

void mutex::unlock()
{
#ifdef SCL_WIN
	CRITICAL_SECTION* pcs = static_cast<CRITICAL_SECTION*>(m_handle);
	LeaveCriticalSection(pcs);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	pthread_mutex_t* pmutex = static_cast<pthread_mutex_t*>(m_handle);
	pthread_mutex_unlock(pmutex);
#endif
}

void mutex::create()
{
	if (NULL != m_handle)
		return;
#ifdef SCL_WIN
	CRITICAL_SECTION* pcs = static_cast<CRITICAL_SECTION*>(malloc(sizeof(CRITICAL_SECTION)));
	InitializeCriticalSection(pcs);
	m_handle = pcs;
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	pthread_mutex_t* pmutex = static_cast<pthread_mutex_t*>(malloc(sizeof(pthread_mutex_t)));
	pthread_mutex_init(pmutex, NULL);
	m_handle = pmutex;
#endif
}


mutex_lock::mutex_lock(mutex* m)
{
	if (NULL == m)
		return;
	m_mutex = m; 
	m_mutex->lock();
}

mutex_lock::~mutex_lock()
{
	if (NULL != m_mutex)
		m_mutex->unlock();
}

void mutex_lock::unlock()
{
	assert(m_mutex);
	if (NULL != m_mutex)
	{
		m_mutex->unlock();
		m_mutex = NULL;
	}
}


semaphore::semaphore() : m_handle(NULL) 
{
}

semaphore::~semaphore()
{
	
#ifdef SCL_WIN
	if (NULL != m_handle)
		CloseHandle(m_handle);
#endif
	
#if defined(SCL_LINUX) || defined(SCL_ANDROID)
	if (NULL != m_handle)
	{
		if (0 != sem_destroy(static_cast<sem_t*>(m_handle)))
		{
			assert(0);
		}
		delete static_cast<sem_t*>(m_handle);
	}		
#endif
	
}


bool semaphore::create(const int count)
{
	
#ifdef SCL_WIN
	m_handle = CreateSemaphore(NULL, count, 0x7FFFFFFF, NULL);
	assert(NULL != m_handle);
	return NULL != m_handle;
#endif
	
#if defined(SCL_LINUX) || defined(SCL_ANDROID)
	sem_t* s = new sem_t;
	int r = sem_init(s, 0, count);
	if (r != 0)
	{
		delete s;
		m_handle = NULL;
	}
	else
		m_handle = s;
	assert(r == 0);
	return r == 0;
#endif
	
#ifdef SCL_APPLE
	dispatch_semaphore_t s = dispatch_semaphore_create(count);
	m_handle = s;
	assert(NULL != m_handle);
	return NULL != m_handle;
#endif
	
#ifdef SCL_HTML5
	return NULL;
#endif
}

void semaphore::post()
{
	assert(NULL != m_handle);
	
#ifdef SCL_WIN
	BOOL r = ReleaseSemaphore(m_handle, 1, NULL);
	assert(r);
#endif
	
#if defined(SCL_LINUX) || defined(SCL_ANDROID)
	int r = sem_post(static_cast<sem_t*>(m_handle));
	assert(r == 0);
#endif
	
#ifdef SCL_APPLE
	dispatch_semaphore_signal(static_cast<dispatch_semaphore_t>(m_handle));
#endif
	
}

bool semaphore::wait()
{
	assert(NULL != m_handle);
	
#ifdef SCL_WIN
	DWORD r = WaitForSingleObject(m_handle, INFINITE);
	assert(r == WAIT_OBJECT_0);
	return r == WAIT_OBJECT_0;
#endif
	
#if defined(SCL_LINUX) || defined(SCL_ANDROID)
	int r = sem_wait(static_cast<sem_t*>(m_handle));
	assert(r == 0);
	return r == 0;
#endif
	
#ifdef SCL_APPLE
	long r = dispatch_semaphore_wait(static_cast<dispatch_semaphore_t>(m_handle), DISPATCH_TIME_FOREVER);
	assert(r == 0);
	return r == 0;
#endif
	
}


bool compare_and_swap(volatile uint* pval, uint oldval, uint newval)
{
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return __sync_bool_compare_and_swap(pval, oldval, newval);
#endif

#ifdef SCL_WIN
	return InterlockedCompareExchange(pval, newval, oldval) == oldval;
#endif
}


bool compare_and_swap(volatile int* pval, int oldval, int newval)
{
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return __sync_bool_compare_and_swap(pval, oldval, newval);
#endif

#ifdef SCL_WIN
	return InterlockedCompareExchange((volatile long*)pval, (long)newval, (long)oldval) == oldval;
#endif
}

bool compare_and_swap(volatile ptr_t* pval, ptr_t oldval, ptr_t newval)
{
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return __sync_bool_compare_and_swap(pval, oldval, newval);
#endif

#ifdef SCL_WIN
	return InterlockedCompareExchangePointer(pval, newval, oldval) == oldval;
#endif
}

int atomic_inc(volatile int* pval)
{
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return __sync_add_and_fetch(pval, 1);
#endif

#ifdef SCL_WIN
	return InterlockedIncrement(reinterpret_cast<volatile LONG*>(pval));
#endif
}

int atomic_dec(volatile int* pval)
{
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return __sync_sub_and_fetch(pval, 1);
#endif

#ifdef SCL_WIN
	return InterlockedDecrement(reinterpret_cast<volatile LONG*>(pval));
#endif
}

}//namespace scl

