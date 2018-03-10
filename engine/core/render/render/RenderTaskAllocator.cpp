#include "engine/core/Base/TypeDef.h"
#include "RenderTaskAllocator.h"
#include "engine/core/Memory/MemManager.h"
#include "Render/RenderThread.h"
#include "RenderTask.h"

namespace Echo
{
	const size_t SmallTaskAllocator::BUDDLE_TASK_COUNT = 1024 * 8;

	SmallTaskAllocator::SmallTaskAllocator()
		: m_index(0)
		, m_buddle_index(0)
	{
		void* buddle = EchoMalloc(RenderTask::SMALL_TASK_SIZE * BUDDLE_TASK_COUNT);
		m_buddles.push_back(buddle);
	}

	SmallTaskAllocator::~SmallTaskAllocator()
	{
        for (ui32 i=0; i<m_buddles.size(); ++i)
		{
			EchoSafeFree(m_buddles[i]);
		}
	}

	void* SmallTaskAllocator::alloc()
	{
		if (++m_index >= BUDDLE_TASK_COUNT)
		{
			if (++m_buddle_index >= m_buddles.size())
			{
				void* buddle = EchoMalloc(RenderTask::SMALL_TASK_SIZE * BUDDLE_TASK_COUNT);
				m_buddles.push_back(buddle);
			}

			m_index = 0;
		}

		void* mem = m_buddles[m_buddle_index];
		void* ret_mem = (Byte*)mem + m_index*RenderTask::SMALL_TASK_SIZE;
		return ret_mem;
	}

	void SmallTaskAllocator::free(void* mem)
	{
		// dummy
	}

	void SmallTaskAllocator::reset()
	{
		m_index = 0;
		m_buddle_index = 0;
	}

}