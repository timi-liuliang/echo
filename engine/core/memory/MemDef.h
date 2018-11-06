#pragma once

#include "engine/core/base/echo_def.h"

// Memory Allocator
#define ECHO_MEMORY_ALLOCATOR_DEFAULT       4
#define ECHO_MEMORY_ALLOCATOR_BINNED        5

// STL Memory Allocator
#define ECHO_STL_MEMORY_ALLOCATOR_STANDARD	1
#define ECHO_STL_MEMORY_ALLOCATOR_CUSTOM	2

// Echo Engine Memory Managed
#ifdef ECHO_DEBUG
	#define ECHO_MEMORY_TRACKER 0
#else
	#define ECHO_MEMORY_TRACKER 0
#endif

#ifdef ECHO_DEBUG
#	define ECHO_MEMORY_ALLOCATOR    ECHO_MEMORY_ALLOCATOR_DEFAULT
#else
#	define ECHO_MEMORY_ALLOCATOR	ECHO_MEMORY_ALLOCATOR_DEFAULT
#endif

// Echo STL memory allocator
#define ECHO_STL_MEMORY_ALLOCATOR	ECHO_STL_MEMORY_ALLOCATOR_STANDARD

#define ECHO_MEMORY_LEAKS_FILENAME	"MemoryLeaks.log"
