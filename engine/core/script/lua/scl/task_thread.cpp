
#include "task_thread.h"

#include "scl/time.h"
#include "scl/log.h"
#include "scl/assert.h"
#include "scl/backtrace.h"
#include "scl/frame_lock.h"

#include <stdio.h>

#ifdef SCL_WIN
#include <Windows.h>
#endif

namespace scl {

void* task_thread::thread_func(void* param, int* signal)
{
	task_thread* tp = static_cast<task_thread*>(param);
	log::init_in_thread(tp->m_logBufferSize);
	scl::frame_lock fl;
	while (1)
	{
SCL_ASSERT_TRY
{
		fl.start();
		//检查信号量
		if (NULL != signal 
			&& *signal == thread::SIGNAL_STOP 
			&& tp->m_tasks.used() == 0)
		{
			break;
		}

		if (tp->m_tasks.used() > 0)
		{
			task t;
			tp->m_tasks.pop_front(t);
			t.func(t.param, t.result, tp);
			if (tp->m_processedTime == 0)
				tp->m_processedTime = SCL_TICK;
			if (!tp->m_ignoreResult)
				tp->m_results.push_back(t);
			++tp->m_processedCount;
		}
		else
		{
			fl.wait();
		}
		scl::sleep(0);

}
SCL_ASSERT_CATCH
{
		log_warn << "get a catch in task_thread::thread_func !" << scl::endl;
		continue;
}

	}
	return NULL;
}

void task_thread::start()
{
	if (m_thread.is_running())
		return;
	m_thread.start(thread_func, this);
}

bool task_thread::process_result(task::funcT func, bool check_thread)
{
	if (check_thread)
	{
		if (-1 == m_resultThreadID)
			m_resultThreadID = scl::thread::self();

		if (scl::thread::self() != m_resultThreadID)
		{
			assert(false); //所有对process_result的调用必须在同一个线程中完成
			return false;
		}
	}

	if (m_results.used() <= 0)
		return false;

	task t;
	m_results.pop_front(t);

	func(t.param, t.result, this);

	return true;
}


int task_thread::process_all_results(task::funcT func, int maxProcessCount)
{
	if (maxProcessCount == -1)
		maxProcessCount = m_results.used();



	int c = 0;
	int processed = 0;
	while (c < maxProcessCount)
	{
		if (process_result(func, c == 0))
		{
			++processed;
		}
		++c;
	}
	return processed;
}

bool task_thread::add(const task& t)
{
	if (!is_running())
		start();

	if (m_tasks.free() <= 0)
		return false;
	m_tasks.push_back(t);
	return true;
}

task_thread::~task_thread()
{
	stop();
}

void task_thread::stop()
{
	if (!m_thread.is_running())
		return;
	
	m_thread.send_stop_signal();
	if (!m_thread.wait())
	{
		assert(0);
	}
}

int find_free_task_thread(const task_thread* task_threads, const int max_thread_count)
{
	int maxFreeIndex = -1;
	int maxFreeValue = 0;
	for (int i = 0; i < max_thread_count; ++i)
	{
		const int freeCount = task_threads[i].free_count();
		if (freeCount > maxFreeValue)
		{
			maxFreeIndex = i;
			maxFreeValue = freeCount;
		}
	}
	return maxFreeIndex;
}

int process_task_thread_result(task_thread* task_threads, const int max_thread_count, task::funcT func)
{
	int processedCount = 0;
	for (int i = 0; i < max_thread_count; ++i)
	{
		bool r = task_threads[i].process_result(func);
		if (r) 
			++processedCount;
	}
	return processedCount;
}

int process_task_thread_all_results(task_thread* task_threads, const int max_thread_count, task::funcT func, int maxProcessCount)
{
	int processedCount = 0;
	for (int i = 0; i < max_thread_count; ++i)
	{
		processedCount += task_threads[i].process_all_results(func, maxProcessCount);			
	}
	return processedCount;
}


} //namespace scl

