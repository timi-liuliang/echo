#pragma once
#include <vector>

namespace Echo
{
	class SmallTaskAllocator final
	{
	public:
		static const size_t BUDDLE_TASK_COUNT;

		SmallTaskAllocator();
		~SmallTaskAllocator();

		void* alloc();
		void free(void* mem);
		void reset();

	private:
		ui32 m_index;
		ui32 m_buddle_index;
		std::vector<void*> m_buddles;
	};
}