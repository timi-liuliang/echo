#include "MemManager.h"
#include "engine/core/Util/AssertX.h"
#include "MemDefaultAlloc.h"
#include "MemBinnedAlloc.h"

namespace Echo
{
	static MemoryManager* g_inst = nullptr;

	// 构造函数
	MemoryManager::MemoryManager()
		: m_malloc(nullptr) 
	{
	}

	// 获取实例
	MemoryManager* MemoryManager::instance()
	{
		if (!g_inst)
		{
			g_inst = new MemoryManager;

#if ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_BINNED
			g_inst->m_malloc = Echo::MallocBinnedMgr::CreateInstance();
#endif
		}

		return g_inst;
	}

	// 替换实例
	void MemoryManager::replaceInstance(MemoryManager* inst)
	{
		EchoAssert(!g_inst);
		g_inst = inst;
		g_inst->m_malloc = inst->m_malloc;

#if ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_BINNED
		Echo::MallocBinnedMgr::ReplaceInstance((MallocInterface*)(inst->m_malloc));
#endif
	}

	// 销毁实例
	void MemoryManager::destroyInstance()
	{
		if (g_inst)
		{
			delete g_inst;
			g_inst = nullptr;
		}
	}

	// 释放内存泄漏日志
	void MemoryManager::outputMemLeakInfo()
	{
#if ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_BINNED
	#ifdef ECHO_DEBUG
		Echo::MallocBinnedMgr::ReleaseInstance();
		Echo::release_memx();
	#endif
#endif
	}
}