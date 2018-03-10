#pragma once

#include "MemAllocDef.h"

namespace Echo
{
	/**
	 * 内存管理器(进程唯一)
	 */
	class MemoryManager
	{
	public:
		// 获取实例
		static MemoryManager* instance();

		// 替换实例
		static void replaceInstance(MemoryManager* inst);

		// 销毁实例
		static void destroyInstance();

		// 释放内存泄漏日志
		static void outputMemLeakInfo();

	private:
		MemoryManager();
		~MemoryManager() {}

	private:
		void*	m_malloc;			// 分配器
	};
}