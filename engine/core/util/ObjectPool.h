#pragma once

#include "engine/core/Base/EchoDef.h"

namespace Echo
{
	/**
	 * 对象池
	 */
	template<typename T>
	class ObjectPool
	{
	public:
		// 构造函数
		ObjectPool()
		{
			Init();
		}

		// 构造函数
		ObjectPool(ui32 capacity)
		{
			Init();

			Reserve(capacity);
		}

		// 析构函数
		~ObjectPool()
		{
			Clear();
		}

		// 新建
		T* New()
		{
			if (m_minFreeIdx < m_capacity)
			{
				int idx = m_minFreeIdx;
				m_minFreeIdx = -1;

				for (ui32 i = idx + 1; i < m_capacity; i++)
				{
					if (!m_flags[i])
					{
						m_minFreeIdx = i;
						break;
					}
				}

				m_flags[idx] = true;
				m_size++;
				return &m_objs[idx];
			}

			return NULL;
		}

		// 析构
		void Delete(T* obj)
		{
			ui32 idx = obj - m_objs;
			EchoAssert(idx < m_capacity);

			m_flags[idx] = false;
			obj = NULL;
			m_size--;

			if (idx < m_minFreeIdx)
				m_minFreeIdx = idx;
		}

		// 重置容量
		void Reserve(ui32 capacity)
		{
			EchoAssert(capacity < 65535);

			Clear();

			m_capacity = capacity;
			m_size = 0;
			m_minFreeIdx = 0;

			m_objs = EchoNewArray(T, m_capacity);
			m_flags = EchoAlloc(bool, m_capacity);
			memset(m_flags, false, sizeof(bool)*m_capacity);
		}

		// 清空
		void Clear()
		{
			EchoAssert(m_size == 0 && m_minFreeIdx == 0);

			EchoSafeDeleteArray(m_objs, T, m_capacity);
			EchoSafeFree(m_flags);
		}

		// 获取容量
		ui32 getCapacity() const { return m_capacity; }

		// 获取大小
		ui32 getSize() const { return m_size; }

	private:
		// 初始化
		void Init()
		{
			m_capacity = 0;
			m_size = 0;
			m_minFreeIdx = 0;
			m_objs = NULL;
			m_flags = NULL;
		}

	private:
		T*		m_objs;			// 已分配对象
		bool*	m_flags;		// 标记是使用状态
		ui32	m_capacity;		// 对象数量
		ui32	m_size;			// 当前使用数量
		ui32	m_minFreeIdx;	// 当前空闲对像下标(最小)
	};
}