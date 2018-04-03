#pragma once

#include "scl/alloc_pool.h"
#include "scl/thread.h"

#include <memory.h>

namespace scl {

//多线程分配器
template <typename T>
class thread_allocator
{
public:
	thread_allocator()
	{
		memset(m_maxAllocCount, 0, sizeof(m_maxAllocCount));
	}

	static T* alloc()
	{
		int thread_index = thread::self_index();
		return alloc(thread_index);
	}

	static T* alloc(const int thread_index)
	{
		const int self_index = thread::self_index();
		assert(thread_index == self_index);
		assert(thread_index >= 0 && thread_index < MAX_THREAD_COUNT);
		if (!m_threads[thread_index].has_init())
			m_threads[thread_index].init(m_maxAllocCount[thread_index]);
		return m_threads[thread_index].alloc();
	}

	static bool has_init()
	{
		int thread_index = thread::self_index();
		return has_init(thread_index);
	}

	static bool has_init(const int thread_index)
	{
		assert(thread_index >= 0 && thread_index < MAX_THREAD_COUNT);
		return m_threads[thread_index].has_init();
	}

	static void init(const int maxAllocCount)
	{
		for (int i = 0; i < MAX_THREAD_COUNT; ++i)
			init(maxAllocCount, i);
	}

	static void init(const int maxAllocCount, const int thread_index)
	{
		assert(thread_index >= 0 && thread_index < MAX_THREAD_COUNT);
		m_maxAllocCount[thread_index] = maxAllocCount;
	}
	
	static void free(T* p)
	{
		int thread_index = thread::self_index();
		thread_allocator::free(p, thread_index);
	}

	static void free(T* p, const int thread_index)
	{
		assert(thread_index >= 0 && thread_index < MAX_THREAD_COUNT);
		m_threads[thread_index].free(p);
	}

	static void release()
	{
		for (int i = 0; i < MAX_THREAD_COUNT; ++i)
			m_threads[i].release();
	}

	static int thread_count() 
	{
		int c = 0;
		for (int i = 0; i < MAX_THREAD_COUNT; ++i)
		{
			if (m_threads[i].has_init())
				++c;
		}
		return c;
	}

	static int	page_size	(const int threadIndex) { return m_threads[threadIndex].page_size();		}
	static int	page_count	(const int threadIndex) { return m_threads[threadIndex].page_count();		}
	static int	total_memory(const int threadIndex) { return m_threads[threadIndex].total_memory();	}
	static int	max_thread_count() { return MAX_THREAD_COUNT; }

private:
	static grow_pool<T>		m_threads		[MAX_THREAD_COUNT];	//每个线程使用一个alloc_pool
	static int				m_maxAllocCount	[MAX_THREAD_COUNT]; //每个线程分配器的大小
};

template<typename T>
grow_pool<T> thread_allocator<T>::m_threads[MAX_THREAD_COUNT];

template<typename T>
int thread_allocator<T>::m_maxAllocCount[MAX_THREAD_COUNT];


} //namespace scl

