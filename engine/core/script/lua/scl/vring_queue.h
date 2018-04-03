////////////////////////////////////////////////////////////////////////////////
//	vring_queue
//  2015.04.08 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/assert.h"

#define SCL_RING_QUEUE_USE_MEMORY_BARRIER

#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
#include "scl/memory_barrier.h"
#endif

namespace scl {

////////////////////////////////////////////////////////////////////////////////
//	class vring_queue
//	注意maxCount，由于m_tail占一位，所以实际容量大小是maxCount-1
////////////////////////////////////////////////////////////////////////////////
template<typename T>
class vring_queue
{
public:
	vring_queue();
	~vring_queue();

	void alloc(const int maxCount)
	{
		assert(NULL == m_queue);
		m_queue = new T[maxCount];
		MAX_COUNT = maxCount;
	}

	//下面这些push_back函数必须在同一个线程中调用
	void	push_back		(const T& elem);
	T&		push_back_fast	();
	T&		push_back_begin	(const int offset = 0);
	void	push_back_end	(const int count = 1);

	//下面这两个函数必须在同一个线程中调用
	void	pop_front		(T& elem);
	T&		peek_front		();
	void	drop			(const int count = 1);

	int		used			() const;
	int		free			() const;
	int		capacity		() const { return MAX_COUNT; }

private:
	T*		m_queue;
	int		m_head;
	int		m_tail;
	int		MAX_COUNT;
};

template<typename T>
vring_queue<T>::vring_queue()
{
	//初始化成员变量
	m_queue		= NULL;
	m_head		= 0;
	m_tail		= 0;
	MAX_COUNT	= 0;
}

template<typename T>
vring_queue<T>::~vring_queue()
{
	if (NULL != m_queue)
	{
		delete[] m_queue;
		m_queue = NULL;
	}
}

template<typename T>
void vring_queue<T>::push_back(const T& elem)
{
	if (free() <= 0)
	{
		assert(false);
		return;
	}
	int tail = m_tail;

	m_queue[tail] = elem;

	//注意对m_tail的修改必须是原子的，这样才能保证线程安全
	tail++;
	if (tail >= MAX_COUNT)
	{
		tail = tail % MAX_COUNT;
	}

#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_tail = tail;
}

template<typename T>
T& vring_queue<T>::push_back_fast()
{
	T& r = push_back_begin();
	push_back_end();
	return r;
}


template<typename T>
T& vring_queue<T>::push_back_begin(const int offset)
{
	if (free() <= offset)
	{
		assert(false);
		throw 1;
	}
	int tail = m_tail + offset;
	if (tail >= MAX_COUNT)
		tail = tail % MAX_COUNT;
	return m_queue[tail];
}

template<typename T>
void vring_queue<T>::push_back_end(int count)
{
	//注意对m_tail的修改必须是原子的，这样才能保证线程安全
	int tail = m_tail;
	tail += count;
	if (tail >= MAX_COUNT)
	{
		tail = tail % MAX_COUNT;
	}
#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_tail = tail;
}


template<typename T>
void vring_queue<T>::pop_front(T& elem)
{
	if (used() <= 0)
	{
		assert(false);
		return;
	}
	int head = m_head;

	elem = m_queue[head];

	//注意,只允许在这里对m_head进行修改，
	//但是m_head的修改必须一次完成最终值，而不能出现中间值，
	//否则会被其他线程读取到中间值
	head++;
	if (head >= MAX_COUNT)
	{
		head = head % MAX_COUNT;
	}
#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_head = head;
}

template<typename T>
T& vring_queue<T>::peek_front()
{
	if (used() <= 0)
	{
		assert(false);
		static T empty;
		return empty;
	}
	return m_queue[m_head];
}

template<typename T>
void vring_queue<T>::drop(const int count)
{
	if (used() <= 0)
	{
		assert(false);
		return;
	}
	int head = m_head;

	//注意,只允许在这里对m_head进行修改，
	//但是m_head的修改必须一次完成最终值，而不能出现中间值，
	//否则会被其他线程读取到中间值
	head += count;
	if (head >= MAX_COUNT)
	{
		head = head % MAX_COUNT;
	}
#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
	scl_memory_barrier();
#endif

	m_head = head;
}


template<typename T>
int scl::vring_queue<T>::used() const
{
	const int currentHead = m_head;
	const int currentTail = m_tail;
	int usedLength = 0;
	if (currentHead > currentTail) // is wrapped
	{
		//m_head到buffer尾部的长度 + 头部到m_tail的长度
		usedLength = (MAX_COUNT - currentHead) + currentTail;
	}
	else
	{
		usedLength = currentTail - currentHead;
	}
	return usedLength;
}

template<typename T>
int scl::vring_queue<T>::free() const
{
	return MAX_COUNT - used() - 1; //减1是因为m_tail占了一位
}



} //namespace scl
