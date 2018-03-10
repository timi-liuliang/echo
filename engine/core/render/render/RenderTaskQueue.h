#pragma once

#include "engine/core/Memory/MemManager.h"
#include <array>

#ifndef ECHO_PLATFORM_HTML5
	#include <atomic>
#endif

namespace Echo
{
	class RenderTask;

#ifndef ECHO_PLATFORM_HTML5
	class RenderTaskQueue final
	{
		static const size_t QUEUE_SIZE = 1024 * 20;
	public:
		RenderTaskQueue();
		~RenderTaskQueue();

		void push(RenderTask* task);
		RenderTask* pop();
		bool isEmpty();
		bool isFull();
		bool isLockFree();

	private:
		size_t increment(size_t idx);

		std::array<RenderTask*, QUEUE_SIZE> m_queue;

#ifdef ECHO_RENDER_THREAD_LOCK_FREE
		volatile size_t m_tail;
		volatile size_t m_head;
#else
		std::atomic<size_t> m_tail;
		std::atomic<size_t> m_head;
#endif
	};
    
#else
	class RenderTaskQueue final : public ObjectAlloc
	{
	public:
		void push(RenderTask* task) {}
		RenderTask* pop() { return nullptr; }
		bool isEmpty() { return true; }
		bool isFull() { return true; }
		bool isLockFree() { return true; }
	};
#endif
}
