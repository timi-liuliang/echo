
#include "task_thread_pool.h"

namespace scl {

void task_thread_pool::set_thread_count(const int size)
{
	if (m_threads.capacity() > 0)
	{
		assert(false);
		return;
	}
	m_threads.reserve(size);
	m_threads.resize(size);
	for (int i = 0; i < m_threads.size(); ++i)
		m_threads[i].set_index(i);
}

int task_thread_pool::add(const task& t)
{
	int freeIndex = find_free_task_thread(m_threads.c_array(), m_threads.size());
	if (freeIndex < 0 || freeIndex >= m_threads.size())
		return -1;
	if (m_threads[freeIndex].add(t))
		_add_counter();
	return freeIndex;
}

bool task_thread_pool::add(const task& t, const int poolIndex)
{
	int index = poolIndex;
	if (index < 0 || index >= m_threads.size())
		return false;
	bool r = m_threads[index].add(t);
	if (r)
		_add_counter();
	return r;
}

task_thread_pool::~task_thread_pool()
{
	stop();
}

void task_thread_pool::stop()
{
	for (int i = 0; i < m_threads.size(); ++i)
		m_threads[i].stop();
}
//
//bool task_thread_pool::has_result() const
//{
//	for (int i = 0; i < m_threads.size(); ++i)
//	{
//		if (m_threads[i].has_result())
//			return true;
//	}
//	return false;
//}
//
//bool task_thread_pool::has_task() const
//{
//	for (int i = 0; i < m_threads.size(); ++i)
//	{
//		if (m_threads[i].has_task())
//			return true;
//	}
//	return false;
//}
//
void task_thread_pool::set_log_buffer_size(const int v)
{
	for (int i = 0; i < m_threads.size(); ++i)
		m_threads[i].set_log_buffer_size(v);
}

void task_thread_pool::process_result(task::funcT func)
{
	const int processed = process_task_thread_result(m_threads.c_array(), m_threads.size(), func);
	if (processed > 0)
		_dec_counter(processed);
}

void task_thread_pool::process_all_results(task::funcT func, const int maxProcessCount)
{
	const int processed = process_task_thread_all_results(m_threads.c_array(), m_threads.size(), func, maxProcessCount);
	if (processed > 0)
		_dec_counter(processed);
}

void task_thread_pool::_add_counter(const int c)
{
	m_counter += c;
}

void task_thread_pool::_dec_counter(const int c)
{
	m_counter -= c;
}


} //namespace scl
