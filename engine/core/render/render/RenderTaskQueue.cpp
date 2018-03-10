#include "RenderTaskQueue.h"

//#include "scl/memory_barrier.h"

using namespace std;

namespace Echo
{

#ifndef ECHO_PLATFORM_HTML5
	RenderTaskQueue::RenderTaskQueue()
		: m_tail(0)
		, m_head(0)
	{

	}

	RenderTaskQueue::~RenderTaskQueue()
	{

	}

	void RenderTaskQueue::push(RenderTask* task)
	{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
		const auto current_tail = m_tail;
		const auto next_tail = increment(current_tail);

		while (next_tail == m_head)
		{
		};
        
        //scl_memory_barrier();
        
		m_queue[current_tail] = task;
        
        //scl_memory_barrier();
        
		m_tail = next_tail;
#else
		const auto current_tail = m_tail.load(memory_order_seq_cst);
		const auto next_tail = increment(current_tail);

		while (next_tail == m_head.load(memory_order_seq_cst))
		{
		};

		m_queue[current_tail] = task;
		m_tail.store(next_tail, memory_order_seq_cst);
#endif
	}

	RenderTask* RenderTaskQueue::pop()
	{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
		const auto current_head = m_head;
		if (current_head == m_tail)
		{
			return nullptr;
		}
        //scl_memory_barrier();

		RenderTask* task = m_queue[current_head];
        
        //scl_memory_barrier();
        
		m_head = increment(current_head);
#else
        const auto current_head = m_head.load(memory_order_seq_cst);
		if (current_head == m_tail.load(memory_order_seq_cst))
		{
			return nullptr;
		}

		RenderTask* task = m_queue[current_head];
		m_head.store(increment(current_head), memory_order_seq_cst);
#endif
		return task;
	}

	bool RenderTaskQueue::isEmpty()
	{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
        return (m_head == m_tail);
#else
		return (m_head.load(memory_order_seq_cst) == m_tail.load(memory_order_seq_cst));
#endif
	}

	bool RenderTaskQueue::isFull()
	{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
		const auto next_tail = increment(m_tail);
		return (next_tail == m_head);
#else
		const auto next_tail = increment(m_tail.load(memory_order_seq_cst));
		return (next_tail == m_head.load(memory_order_seq_cst));
#endif
	}

	bool RenderTaskQueue::isLockFree()
	{
#ifdef ECHO_RENDER_THREAD_LOCK_FREE
        return true;
#else
		return (m_tail.is_lock_free() && m_head.is_lock_free());
#endif
	}

	size_t RenderTaskQueue::increment(size_t idx)
	{
		return (idx + 1) % QUEUE_SIZE;
	}
    
#endif //#ifndef ECHO_PLATFORM_HTML5

} // namespace Echo

