#pragma once

#include <memory.h>
#include "engine/core/math/Def.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	template <typename T, i32 N>
	class GrowableStack
	{
	public:
		GrowableStack()
		{
			m_stack = m_array;
			m_count = 0;
			m_capacity = N;
		}

		~GrowableStack()
		{
			if (m_stack != m_array)
			{
				EchoSafeFree(m_stack);
				m_stack = NULL;
			}
		}

		void push(const T& element)
		{
			if (m_count == m_capacity)
			{
				T* old = m_stack;
				m_capacity *= 2;
				m_stack = (T*)EchoMalloc(m_capacity * sizeof(T));
				memcpy(m_stack, old, m_count * sizeof(T));
				if (old != m_array)
				{
					EchoSafeFree(old);
				}
			}

			m_stack[m_count] = element;
			++m_count;
		}

		T pop()
		{
			assert(m_count > 0);
			--m_count;
			return m_stack[m_count];
		}

		i32 getCount()
		{
			return m_count;
		}

	private:
		T*	m_stack;
		T	m_array[N];
		i32 m_count;
		i32 m_capacity;
	};
}
