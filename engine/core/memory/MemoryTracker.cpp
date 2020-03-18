#include "engine/core/util/AssertX.h"
#include "engine/core/memory/MemoryTracker.h"
#include "engine/core/util/StackTrace.h"
#include <iostream>
#include <fstream>

#ifdef ECHO_PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	define _CRT_SECURE_NO_WARNINGS
#	include <Windows.h>
#	include <Windowsx.h>
#	define EchoOutputCString(str) ::OutputDebugStringA(str)
#else
#	define EchoOutputCString(str) std::cerr << str
#endif

namespace Echo
{
#if ECHO_MEMORY_TRACKER
    MemoryTracker::Alloc::Alloc()
        : m_bytes(0)
        , m_line(0)
    {
    }
    
    MemoryTracker::Alloc::Alloc(size_t sz, unsigned int p, const char *file, size_t ln, const char *func)
        : m_bytes(sz)
        , m_pool(p)
        , m_line(ln)
    {
        if(file)    m_filename = file;
        if(func)    m_function = func;
        
        m_stackDepth = StackTrace( m_callstack, 128);
    }
    
    // stack to string
    std::string MemoryTracker::Alloc::getStackDesc()
    {
        return StackTraceDesc(m_callstack, 128, m_stackDepth, 6);
    }
    
    MemoryTracker::MemoryTracker()
        : m_leakFileName(ECHO_MEMORY_LEAKS_FILENAME)
        , m_totalAllocations(0)
    {}
    
	MemoryTracker& MemoryTracker::get()
	{
		static MemoryTracker tracker;
		return tracker;
	}

	void MemoryTracker::recordAlloc(void* ptr, size_t sz, unsigned int pool, const char* file, size_t ln, const char* func)
	{
		EE_LOCK_MUTEX(m_mutex)
		const char* msg = "Double allocation with same address - this probably means you have a mismatched allocation / deallocation style.";
		EchoAssertX(m_allocations.find(ptr) == m_allocations.end(), msg);

		const char* filewithoutPath = file;
		if(filewithoutPath)
		{
			size_t len = strlen(file);
			filewithoutPath += len;
			while(true)
			{
				filewithoutPath--;
				if(*filewithoutPath == '\\' || *filewithoutPath == '/')
				{
					filewithoutPath++;
					break;
				}
				else if(filewithoutPath == file)
					break;
			}
		}

		m_allocations[ptr] = Alloc(sz, pool, filewithoutPath, ln, func);
		if(pool >= m_allocationsByPool.size())
			m_allocationsByPool.resize(pool+1, 0);
		m_allocationsByPool[pool] += sz;
		m_totalAllocations += sz;
	}

	void MemoryTracker::recordDealloc(void* ptr)
	{
		if (ptr)
        {
            EE_LOCK_MUTEX(m_mutex)
            AllocationMap::iterator it = m_allocations.find(ptr);
            if(it!=m_allocations.end())
            {
                m_allocationsByPool[it->second.m_pool] -= it->second.m_bytes;
                m_totalAllocations -= it->second.m_bytes;
                m_allocations.erase(it);
            }
            else
            {
                std::cout << "Unable to locate allocation unit - this probably means you have a mismatched allocation / deallocation style.";
            }
        }
	}

	size_t MemoryTracker::getTotalMemoryAllocated() const
	{
		return m_totalAllocations;
	}

	size_t MemoryTracker::getMemoryAllocatedForPool(unsigned int pool) const
	{
		return m_allocationsByPool[pool];
	}

	void MemoryTracker::reportLeaks()
	{
		std::stringstream os;

		if (m_allocations.empty())
		{
			os << "Echo Memory: No memory leaks." << std::endl;
		}
		else
		{			
			os << "Echo Memory: Detected memory leaks !!! " << std::endl;
			os << "Echo Memory: (" << m_allocations.size() << ") Allocation(s) with total " << m_totalAllocations << " bytes." << std::endl;
			os << "Echo Memory: Dumping allocations -> " << std::endl;

            for (auto it : m_allocations)
			{
                Alloc& alloc = it.second;

                os << (!alloc.m_filename.empty() ? alloc.m_filename : "(unknown source):");
                os << "(" << alloc.m_line << ") : {" << alloc.m_bytes << " bytes}" << " function: " << alloc.m_function;
                os << "\n" << alloc.getStackDesc() << std::endl;
			}
            
			os << std::endl;			
		}

		if (m_isDumpToStdOut)		
			std::cout << os.str();		

#ifdef ECHO_PLATFORM_ANDROID
		FILE* fp = fopen(("sdcard/" + m_leakFileName).c_str(),"a+");
		if(fp)
		{
            fwrite(os.str().c_str(), os.str().length(),1,fp);
            fclose(fp);
		}
#else
		std::ofstream of;
		std::locale loc = std::locale::global(std::locale(""));
		of.open(m_leakFileName.c_str());
		std::locale::global(loc);
		of << os.str();
		of.close();
		EchoOutputCString(os.str().c_str());
#endif
	}
#endif
}
