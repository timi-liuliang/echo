#pragma once

#include "scl/type.h"

#ifdef SCL_WIN32
__forceinline void scl_memory_barrier(void) { long Barrier; __asm { xchg Barrier, eax } }
#endif

#ifdef SCL_WIN64
__forceinline void scl_memory_barrier(void) {  }
#endif

#if defined(SCL_LINUX) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#define scl_memory_barrier __sync_synchronize
#endif

#ifdef SCL_APPLE 
#include <libkern/OSAtomic.h>
#define scl_memory_barrier OSMemoryBarrier
//#include <atomic>
//#define scl_memory_barrier std::atomic_thread_fence
#endif
