#pragma once

////////////////////////////////////////////////////////////////////////
//	任务线程池
//		用于管理一个task_thread数组
//
//	示例：
//	task_thread_pool pool(THREAD_COUNT);
//	while (true)
//	{
//		while (!pool[id].add(task(task_func))) sleep(1);
//		pool.process_result(pool, thread_count, print_result_func);
//	}
////////////////////////////////////////////////////////////////////////

#include "scl/task_thread.h"
#include "scl/varray.h"

namespace scl {

class task_thread_pool
{
public:
	task_thread_pool() : m_counter(0) {}
	task_thread_pool(const int thread_count)	{ set_thread_count(thread_count); }
	~task_thread_pool();

	void	set_thread_count(const int size);
	int		add				(const task& t);	//添加一个任务，如果线程池已满，则返回线程池的poolIndex，注意！这个值和threadIndex没有关系！
	bool	add				(const task& t, const int poolIndex);	//向指定的线程池添加一个任务，如果线程池已满，则返回false
	void	process_result	(task::funcT func);
	void	process_all_results(task::funcT func, const int maxProcessCount);
	void	stop			();
	int		thread_count	() const { return m_threads.size(); }
	//bool	has_result		() const; //是否有尚未处理的result
	//bool	has_task		() const; //是否有尚未处理的task
	bool	has_task		() const { return m_counter != 0; }

	//属性
	bool	has_free		()					{ return -1 != find_free_task_thread(m_threads.c_array(), m_threads.size()); }
	int		processed_count	(const int index)	const { return m_threads[index].processed_count(); }
	uint64	processed_time	(const int index)	const { return m_threads[index].processed_time(); }
	int		task_count		(const int index)	const { return m_threads[index].task_count(); }
	int		result_count	(const int index)	const { return m_threads[index].result_count(); }
	int		task_count		()	const			{ return m_counter; }
	void	set_log_buffer_size(const int v);

private:
	void	_add_counter(const int c = 1);
	void	_dec_counter(const int c = 1);

private:
	varray<task_thread> m_threads;
	int		m_counter;	//任务计数器，add时候+1，处理完result之后-1
};

} //namespace scl

