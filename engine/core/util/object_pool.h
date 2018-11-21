#pragma once

#include "engine/core/Base/echo_def.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	template<typename T>
	class ObjectPool
	{
	public:
		ObjectPool(i32 capacity)
		{
			init( capacity);
		}

		~ObjectPool()
		{
			clear();
		}

		T* newObj()
		{
			if (!m_frees.empty())
			{
				ui32 idx = m_frees.back();
				m_frees.pop_back();

				return &m_objs[idx];
			}

			return nullptr;
		}

		void deleteObj(T* obj)
		{
			i32 idx = static_cast<i32>(obj - m_objs);
			m_frees.push_back(idx);
		}

		void clear()
		{
			EchoSafeDeleteArray(m_objs, T, m_capacity);
		}

	private:
		void init(i32 capcity)
		{
			m_capacity = capcity;
			m_objs = EchoNewArray(T, m_capacity);

			m_frees.reserve(capcity);
			for (i32 i = capcity-1; i >=0; i--)
				m_frees.push_back(i);
		}

	private:
		ui32				m_capacity;		// capacity
		T*					m_objs;			// all objs
		std::vector<ui32>	m_frees;		// free objs
	};
}