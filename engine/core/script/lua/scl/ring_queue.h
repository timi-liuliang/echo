////////////////////////////////////////////////////////////////////////////////
//	ring_queue
//  2010.05.27 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/assert.h"

#define SCL_RING_QUEUE_USE_MEMORY_BARRIER

#ifdef SCL_RING_QUEUE_USE_MEMORY_BARRIER
#include "scl/memory_barrier.h"


#ifdef SCL_APPLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#endif

namespace scl {

////////////////////////////////////////////////////////////////////////////////
//	class ring_queue
//	注意MAX_COUNT是数组的大小，由于m_tail占一位，所以实际容量大小是MAX_COUNT - 1
////////////////////////////////////////////////////////////////////////////////
template<typename T, int MAX_COUNT = 32>
class ring_queue
{
public:
	ring_queue();
	~ring_queue();

	//下面这些push_back函数必须在同一个线程中调用
	void	push_back		(const T& elem);
	// push_back_fast is not thread safe! because after all push_back_fast, the return T& object may be modified both by current thread and other thread.
	T&		push_back_fast_unsafe();
	T&		push_back_begin	(const int offset = 0);
	void	push_back_end	(const int count = 1);

	//下面这两个函数必须在同一个线程中调用
	void	pop_front		(T& elem);
	T&		peek_front		();
	void	drop			(const int count = 1);

	int		used			() const;
	int		free			() const;
	int		capacity		() const { return MAX_COUNT; }
	void	clear_unsafe	();

private:
	T		m_queue[MAX_COUNT];
	int		m_head;
	int		m_tail;
};

template<typename T, int MAX_COUNT>
ring_queue<T, MAX_COUNT>::ring_queue()
{
	//初始化成员变量
	m_head		= 0;
	m_tail		= 0;
}

template<typename T, int MAX_COUNT>
ring_queue<T, MAX_COUNT>::~ring_queue()
{
}

template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::push_back(const T& elem)
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

template<typename T, int MAX_COUNT>
T& ring_queue<T, MAX_COUNT>::push_back_fast_unsafe()
{
	T& r = push_back_begin();
	push_back_end();
	return r;
}


template<typename T, int MAX_COUNT>
T& ring_queue<T, MAX_COUNT>::push_back_begin(const int offset)
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

template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::push_back_end(int count)
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


template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::pop_front(T& elem)
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

template<typename T, int MAX_COUNT>
T& ring_queue<T, MAX_COUNT>::peek_front()
{
	if (used() <= 0)
	{
		assert(false);
		static T empty;
		return empty;
	}
	return m_queue[m_head];
}

template<typename T, int MAX_COUNT>
void ring_queue<T, MAX_COUNT>::drop(const int count)
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


template<typename T, int MAX_COUNT>
int scl::ring_queue<T, MAX_COUNT>::used() const
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

template<typename T, int MAX_COUNT>
int scl::ring_queue<T, MAX_COUNT>::free() const
{
	return MAX_COUNT - used() - 1; //减1是因为m_tail占了一位
}

template<typename T, int MAX_COUNT>
void scl::ring_queue<T, MAX_COUNT>::clear_unsafe() 
{
	while (used())
		drop();
}



} //namespace scl

#ifdef SCL_APPLE
#pragma GCC diagnostic pop
#endif


