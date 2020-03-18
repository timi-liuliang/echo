#pragma once

#include "engine/core/memory/MemDef.h"
#include "engine/core/thread/Threading.h"
#include <string>
#include <vector>

#if ECHO_MEMORY_TRACKER

namespace Echo
{
	class MemoryTracker
	{    
	protected:
		// Allocation record
		struct Alloc
		{
			size_t          m_bytes;
			unsigned int    m_pool;
			std::string     m_filename;
			size_t          m_line;
			std::string     m_function;
            void*           m_callstack[128];
            i32             m_stackDepth = 0;

            Alloc();
            Alloc(size_t sz, unsigned int p, const char *file, size_t ln, const char *func);
            
            // stack to string
            std::string getStackDesc();
		};
		typedef EchoHashMap<void*, Alloc> AllocationMap;
		typedef std::vector<size_t> AllocationsByPool;
        
	public:
        // Static utility method to get the memory tracker instance
        static MemoryTracker& get();
        
        // report leaks
		void reportLeaks();

		/** Set the name of the report file that will be produced on exit. */
		void setReportFileName(const std::string& name) { m_leakFileName = name; }
        
		// Return the name of the file which will contain the report at exit
		const std::string& getReportFileName() const { return m_leakFileName; }
        
		// whether the memory report should be sent to stdout
		void setReportToStdOut(bool rep) { m_isDumpToStdOut = rep; }
		bool getReportToStdOut() const { return m_isDumpToStdOut; }

		// Get the total amount of memory allocated currently.
		size_t getTotalMemoryAllocated() const;
        
		// Get the amount of memory allocated in a given pool
		size_t getMemoryAllocatedForPool(unsigned int pool) const;

		// Record an allocation that has been made. Only to be called by
		void recordAlloc(void* ptr, size_t sz, unsigned int pool = 0, const char* file = NULL, size_t ln = 0, const char* func = NULL);
		
        // Record the deallocation of memory.
		void recordDealloc(void* ptr);
        
    protected:
        MemoryTracker();
        ~MemoryTracker() { reportLeaks(); }
        
    protected:
        std::string         m_leakFileName;
        bool                m_isDumpToStdOut = true;
        AllocationMap       m_allocations;
        size_t              m_totalAllocations;
        AllocationsByPool   m_allocationsByPool;
		EE_MUTEX			(m_mutex);
	};
}

#endif
