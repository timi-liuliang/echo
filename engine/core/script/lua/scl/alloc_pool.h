#pragma once
////////////////////////////////////////////////////////////////////////////////
//	alloc_pool	
//	使用池来分配内存
////////////////////////////////////////////////////////////////////////////////

#include "scl/stack.h"
#include <new>	//placement new
#include <stdlib.h> //malloc()
#include <memory.h> //memset

template<typename T>
class alloc_pool
{
public:
	alloc_pool() : m_memory(NULL), m_flags(NULL), m_allocCount(0), m_hasInit(false), m_maxSize(0) {}

	inline void			init			(int maxSize);
	inline void			release			();
	bool				is_full			() const { return m_allocCount == 0 && m_free.size() == 0; }
	bool				has_init		() const { return m_hasInit; }
	int					free_size		() const { return m_allocCount + m_free.size(); }
	//void				set_thread_id	(const int threadID) { m_threadID = id; }
	//int					thread_id		() const { return m_threadID; }
	bool				in_pool			(T* elem) const;

	inline T*			alloc			();
	inline void			free			(T* const pElem);

private:
	T*					m_memory;
	char*				m_flags;
	scl::vstack<int>	m_free;
	int					m_allocCount;
	bool				m_hasInit;
	int					m_maxSize; //等于m_free.capacity() 但是效率更高
	//int					m_threadID;
};

template<typename T>
inline void alloc_pool<T>::init(const int maxSize)
{
	assert(!m_hasInit);
	assert(maxSize > 0);

	//为了规避init时对T的构造函数的调用，这里使用malloc
	//后来为了方便new_tracer能够记录alloc_pool分配的内存，这里又改成了new byte[]
	m_memory = static_cast<T*>(static_cast<void*>(new byte[maxSize * sizeof(T)]));
	if (NULL == m_memory)
	{
		assert(0);
		return;
	}
	m_free.reserve(maxSize);
	m_allocCount = maxSize; 
	m_flags = new char[maxSize];
	memset(m_flags, 0, sizeof(m_flags[0]) * maxSize);
	m_hasInit = true;
	m_maxSize = maxSize;
}

template<typename T>
inline void alloc_pool<T>::release()
{
	//检测内存泄漏
	//m_free.size()是被释放的所有元素总和
	//m_allocCount是最初m_memory中还未被分配出去的数量
	//这两部分总和在release的时候必须和分配器的总数量相同，才能保证所有alloc的内存都被free了
	assert(m_free.size() + m_allocCount == m_free.capacity());

	m_hasInit = false;
	delete[] (byte*)m_memory;

	delete[] m_flags;
	//::free(m_memory);
}

template<typename T>
inline bool alloc_pool<T>::in_pool(T* elem) const
{
	return elem >= m_memory && elem < m_memory + m_maxSize; 
}

template<typename T>
inline T* alloc_pool<T>::alloc()
{
	if (!m_hasInit)
	{
		assert(0);
		return NULL;
	}
	if (m_allocCount > 0)
	{
		--m_allocCount;
		T* p = new (&m_memory[m_allocCount]) T;
		assert(m_flags[m_allocCount] == 0);
		m_flags[m_allocCount] = 1;
		return p;
	}
	else if (m_free.size() > 0)
	{
		int index = m_free.pop();
 		T* p = new (&m_memory[index]) T;
		assert(m_flags[index] == 0);
		m_flags[index] = 1;
 		return p;
	}
	else
	{
		//throw(1);	//TODO 定义对应异常
		assert(0);
		return NULL;
	}
}

template<typename T>
inline void alloc_pool<T>::free(T* const pElem)
{
	if (!m_hasInit)
	{
		assert(0);
		return;
	}
	if (NULL != pElem)
	{	
		uint64 index = pElem - &(m_memory[0]); //TODO 这里太混乱了！应该定义一个类似于ULONG_PTR的东西
		int32 index32 = static_cast<int32>(index);

		//检查指针是否在范围内，如果不在分配器内部，可能是其他线程分配的内存，需要检查上层的线程逻辑
		if (index32 < 0 || index32 >= m_free.capacity())
		{
			assertf(false, "index = %d", index32);
			return;
		}

		//析构
		pElem->~T();
		
		//置flag
		int i32 = static_cast<int>(index);
		assertf(m_flags[i32] == 1, "flag[%d] = %d", i32, m_flags[i32]);
		m_flags[i32] = 0;

		//添加到可用堆栈中
		m_free.push(i32);
	}
}
