#include "engine/core/util/AssertX.h"
#include "engine/core/memory/MemTracker.h"
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

	MemTracker& MemTracker::get()
	{
		static MemTracker tracker;
		return tracker;
	}

	void MemTracker::_recordAlloc(void* ptr, size_t sz, unsigned int pool, 
		const char* file, size_t ln, const char* func)
	{
		EE_LOCK_AUTO_MUTEX
		const char* msg = "Double allocation with same address - this probably means you have a mismatched allocation / deallocation style.";
		EchoAssertX(mAllocations.find(ptr) == mAllocations.end(), msg);

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

		mAllocations[ptr] = Alloc(sz, pool, filewithoutPath, ln, func);
		if(pool >= mAllocationsByPool.size())
			mAllocationsByPool.resize(pool+1, 0);
		mAllocationsByPool[pool] += sz;
		mTotalAllocations += sz;
	}

	void MemTracker::_recordDealloc(void* ptr)
	{
		// deal cleanly with null pointers
		if (!ptr)
			return;

		EE_LOCK_AUTO_MUTEX
		AllocationMap::iterator i = mAllocations.find(ptr);

		const char* msg = "Unable to locate allocation unit - this probably means you have a mismatched allocation / deallocation style.";
		EchoAssertX(i != mAllocations.end(), msg);
		// update category stats
		mAllocationsByPool[i->second.pool] -= i->second.bytes;
		// global stats
		mTotalAllocations -= i->second.bytes;
		mAllocations.erase(i);
	}

	size_t MemTracker::getTotalMemoryAllocated() const
	{
		return mTotalAllocations;
	}

	size_t MemTracker::getMemoryAllocatedForPool(unsigned int pool) const
	{
		return mAllocationsByPool[pool];
	}

	void MemTracker::reportLeaks()
	{
		std::stringstream os;

		if (mAllocations.empty())
		{
			os << "Echo Memory: No memory leaks." << std::endl;
		}
		else
		{			
			os << "Echo Memory: Detected memory leaks !!! " << std::endl;
			os << "Echo Memory: (" << mAllocations.size() << ") Allocation(s) with total " << mTotalAllocations << " bytes." << std::endl;
			os << "Echo Memory: Dumping allocations -> " << std::endl;


			for (AllocationMap::const_iterator i = mAllocations.begin(); i != mAllocations.end(); ++i)
			{
				const Alloc& alloc = i->second;
				if (!alloc.filename.empty())				
					os << alloc.filename;
				else
					os << "(unknown source):";

				os << "(" << alloc.line << ") : {" << alloc.bytes << " bytes}" << " function: " << alloc.function << std::endl; 				

			}			
			os << std::endl;			
		}

		if (mDumpToStdOut)		
			std::cout << os.str();		

#ifdef ECHO_PLATFORM_ANDROID
		FILE* fp = NULL;
		fp = fopen("sdcard/MemoryLeak.txt","a+");
		if(!fp)
		{
			return;
		}
		//Echo::Log("find SdCard");
		fwrite(os.str().c_str(), os.str().length(),1,fp);
		fclose(fp);
#else
		std::ofstream of;
		std::locale loc = std::locale::global(std::locale(""));
		of.open(mLeakFileName.c_str());
		std::locale::global(loc);
		of << os.str();
		of.close();
		EchoOutputCString(os.str().c_str());
#endif
	}
#endif // ECHO_DEBUG_MODE
}
