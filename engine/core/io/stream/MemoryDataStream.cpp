#include "MemoryDataStream.h"
#include "engine/core/base/echo_def.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/util/AssertX.h"
#include <engine/core/log/Log.h>
#include "engine/core/io/IO.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

namespace Echo
{
	MemoryDataStream::MemoryDataStream(void* pMem, size_t size, bool freeOnClose, bool readOnly)
		: DataStream(static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		mData = mPos = static_cast<ui8*>(pMem);
		m_size = size;
		mEnd = mData + m_size;
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(const String& name, void* pMem, size_t size, 
		bool freeOnClose, bool readOnly)
		: DataStream(name, static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		mData = mPos = static_cast<ui8*>(pMem);
		m_size = size;
		mEnd = mData + m_size;
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(size_t size, bool freeOnClose, bool readOnly)
		: DataStream(static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		m_size = size;
		mFreeOnClose = freeOnClose;
		mData = (ui8*)EchoMalloc(m_size);
		mPos = mData;
		mEnd = mData + m_size;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(const String& name, size_t size, 
		bool freeOnClose, bool readOnly)
		: DataStream(name, static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		m_size = size;
		mFreeOnClose = freeOnClose;
		mData = (ui8*)EchoMalloc(m_size);
		mPos = mData;
		mEnd = mData + m_size;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::~MemoryDataStream()
	{
		close();
	}

	size_t MemoryDataStream::read(void* buf, size_t count)
	{
		size_t cnt = count;
		// Read over end of memory?
		if (mPos + cnt > mEnd)
			cnt = mEnd - mPos;
		if (cnt == 0)
			return 0;

		EchoAssert(cnt <= count);

		memcpy(buf, mPos, cnt);
		mPos += cnt;
		return cnt;
	}

	size_t MemoryDataStream::write(const void* buf, size_t count)
	{
		size_t written = 0;
		if (isWriteable())
		{
			written = count;
			// we only allow writing within the extents of allocated memory
			// check for buffer overrun & disallow
			if (mPos + written > mEnd)
				written = mEnd - mPos;
			if (written == 0)
				return 0;

			memcpy(mPos, buf, written);
			mPos += written;
		}
		return written;
	}

	size_t MemoryDataStream::readLine(char* buf, size_t maxCount, 
		const String& delim)
	{
		// Deal with both Unix & Windows LFs
		bool trimCR = false;
		if (delim.find_first_of('\n') != String::npos)
		{
			trimCR = true;
		}

		size_t pos = 0;

		// Make sure pos can never go past the end of the data 
		while (pos < maxCount && mPos < mEnd)
		{
			if (delim.find(*mPos) != String::npos)
			{
				// Trim off trailing CR if this was a CR/LF entry
				if (trimCR && pos && buf[pos-1] == '\r')
				{
					// terminate 1 character early
					--pos;
				}

				// Found terminator, skip and break out
				++mPos;
				break;
			}

			buf[pos++] = *mPos++;
		}

		// terminate
		buf[pos] = '\0';

		return pos;
	}

	size_t MemoryDataStream::skipLine(const String& delim)
	{
		size_t pos = 0;

		// Make sure pos can never go past the end of the data 
		while (mPos < mEnd)
		{
			++pos;
			if (delim.find(*mPos++) != String::npos)
			{
				// Found terminator, break out
				break;
			}
		}

		return pos;

	}

	void MemoryDataStream::skip(long count)
	{
		size_t newpos = (size_t)( ( mPos - mData ) + count );
		EchoAssert(mData + newpos <= mEnd);

		mPos = mData + newpos;
	}

	void MemoryDataStream::seek(size_t pos, int origin)
	{
		if (origin == SEEK_END)
		{
			EchoAssert(mEnd + pos <= mEnd);
			mPos = mEnd + pos;
		}
		else
		{
			EchoAssert(mData + pos <= mEnd);
			mPos = mData + pos;
		}
	}

	size_t MemoryDataStream::tell(void) const
	{
		//mData is start, mPos is current location
		return mPos - mData;
	}

	bool MemoryDataStream::eof(void) const
	{
		return mPos >= mEnd;
	}

	void MemoryDataStream::close(void)    
	{
		if (mFreeOnClose && mData)
		{
			EchoSafeFree(mData);
		}
	}
}
