#include "DataStream.h"
#include "engine/core/base/echo_def.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/util/AssertX.h"
#include <engine/core/log/Log.h>
#include "IO.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

namespace Echo
{
	DataStream::~DataStream()
	{
		EchoSafeFree(m_buffer);
	}

	template <typename T> DataStream& DataStream::operator >>(T& val)
	{
		read(static_cast<void*>(&val), sizeof(T));
		return *this;
	}

    void DataStream::bindMethods()
    {
        CLASS_BIND_METHOD(DataStream, test, "test");
    }

    void DataStream::test(const char* msg)
    {
        EchoLogError("Data Stream test succeed");
    }

	size_t DataStream::write(const void* buf, size_t count)
	{
		(void)buf;
		(void)count;
		return 0;
	}

	String DataStream::getLine(bool trimAfter)
	{
		char tmpBuf[ECHO_STREAM_TEMP_SIZE];
		String retString;
		size_t readCount;
		// Keep looping while not hitting delimiter
		while ((readCount = read(tmpBuf, ECHO_STREAM_TEMP_SIZE-1)) != 0)
		{
			// Terminate string
			tmpBuf[readCount] = '\0';

			char* p = strchr(tmpBuf, '\n');
			if (p != 0)
			{
				// Reposition backwards
				skip((long)(p + 1 - tmpBuf - readCount));
				*p = '\0';
			}

			retString += tmpBuf;

			if (p != 0)
			{
				// Trim off trailing CR if this was a CR/LF entry
				if (retString.length() && retString[retString.length()-1] == '\r')
				{
					retString.erase(retString.length()-1, 1);
				}

				// Found terminator, break out
				break;
			}
		}

		if (trimAfter)
		{
			StringUtil::Trim(retString);
		}

		return retString;
	}

	size_t DataStream::readLine(char* buf, size_t maxCount, const String& delim)
	{
		// Deal with both Unix & Windows LFs
		bool trimCR = false;
		if (delim.find_first_of('\n') != String::npos)
		{
			trimCR = true;
		}

		char tmpBuf[ECHO_STREAM_TEMP_SIZE];
		size_t chunkSize = std::min<size_t>(maxCount, (size_t)ECHO_STREAM_TEMP_SIZE-1);
		size_t totalCount = 0;
		size_t readCount; 
		while (chunkSize && (readCount = read(tmpBuf, chunkSize)) != 0)
		{
			// Terminate
			tmpBuf[readCount] = '\0';

			// Find first delimiter
			size_t pos = strcspn(tmpBuf, delim.c_str());

			if (pos < readCount)
			{
				// Found terminator, reposition backwards
				skip((long)(pos + 1 - readCount));
			}

			// Are we genuinely copying?
			if (buf)
			{
				memcpy(buf+totalCount, tmpBuf, pos);
			}
			totalCount += pos;

			if (pos < readCount)
			{
				// Trim off trailing CR if this was a CR/LF entry
				if (trimCR && totalCount && buf[totalCount-1] == '\r')
				{
					--totalCount;
				}

				// Found terminator, break out
				break;
			}

			// Adjust chunkSize for next time
			chunkSize = std::min<size_t>(maxCount-totalCount, (size_t)ECHO_STREAM_TEMP_SIZE-1);
		}

		// Terminate
		buf[totalCount] = '\0';

		return totalCount;
	}

	size_t DataStream::skipLine(const String& delim)
	{
		char tmpBuf[ECHO_STREAM_TEMP_SIZE];
		size_t total = 0;
		size_t readCount;
		// Keep looping while not hitting delimiter
		while ((readCount = read(tmpBuf, ECHO_STREAM_TEMP_SIZE-1)) != 0)
		{
			// Terminate string
			tmpBuf[readCount] = '\0';

			// Find first delimiter
			size_t pos = strcspn(tmpBuf, delim.c_str());

			if (pos < readCount)
			{
				// Found terminator, reposition backwards
				skip((long)(pos + 1 - readCount));

				total += pos + 1;

				// break out
				break;
			}

			total += readCount;
		}

		return total;
	}

	String DataStream::getAsString(void)
	{
		// Read the entire buffer
		char* pBuf = (char*)EchoMalloc(mSize+1);
		// Ensure read from begin of stream
		seek(0);
		read(pBuf, mSize);
		pBuf[mSize] = '\0';
		String str;
		str.insert(0, pBuf, mSize);
		EchoSafeFree(pBuf);
		return str;
	}

	const void* DataStream::getBuffer()
	{
		if (!m_buffer)
		{
			m_buffer = (char*)EchoMalloc(mSize+1);
			read(m_buffer, mSize);
			m_buffer[mSize] = '\0';
		}

		return m_buffer;
	}

    void DataStream::readAll( void* buffer )
    {
        seek(0);
        read(buffer, mSize);
    }

	MemoryDataStream::MemoryDataStream(void* pMem, size_t size, bool freeOnClose, bool readOnly)
		: DataStream(static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		mData = mPos = static_cast<ui8*>(pMem);
		mSize = size;
		mEnd = mData + mSize;
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(const String& name, void* pMem, size_t size, 
		bool freeOnClose, bool readOnly)
		: DataStream(name, static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		mData = mPos = static_cast<ui8*>(pMem);
		mSize = size;
		mEnd = mData + mSize;
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(DataStream& sourceStream, 
		bool freeOnClose, bool readOnly)
		: DataStream(static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		// Copy data from incoming stream
		mSize = sourceStream.size();
		mData = (ui8*)EchoMalloc(mSize);
		mPos = mData;
		mEnd = mData + sourceStream.read(mData, mSize);
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(DataStream* sourceStream, 
		bool freeOnClose, bool readOnly)
		: DataStream(static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		// Copy data from incoming stream
		mSize = sourceStream->size();
		mData = (ui8*)EchoMalloc(mSize);
		mPos = mData;
		mEnd = mData + sourceStream->read(mData, mSize);
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(const String& name, DataStream& sourceStream, 
		bool freeOnClose, bool readOnly)
		: DataStream(name, static_cast<ui8>(readOnly ? READ : (READ | WRITE)))
	{
		// Copy data from incoming stream
		mSize = sourceStream.size();
		mData = (ui8*)EchoMalloc(mSize);
		mPos = mData;
		mEnd = mData + sourceStream.read(mData, mSize);
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(const String& name, DataStream* sourceStream, 
		bool freeOnClose, bool readOnly)
		: DataStream(name, static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		// Copy data from incoming stream
		mSize = sourceStream->size();
		mData = (ui8*)EchoMalloc(mSize);
		mPos = mData;
		mEnd = mData + sourceStream->read(mData, mSize);
		mFreeOnClose = freeOnClose;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(size_t size, bool freeOnClose, bool readOnly)
		: DataStream(static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		mSize = size;
		mFreeOnClose = freeOnClose;
		mData = (ui8*)EchoMalloc(mSize);
		mPos = mData;
		mEnd = mData + mSize;
		EchoAssert(mEnd >= mPos);
	}

	MemoryDataStream::MemoryDataStream(const String& name, size_t size, 
		bool freeOnClose, bool readOnly)
		: DataStream(name, static_cast<ui16>(readOnly ? READ : (READ | WRITE)))
	{
		mSize = size;
		mFreeOnClose = freeOnClose;
		mData = (ui8*)EchoMalloc(mSize);
		mPos = mData;
		mEnd = mData + mSize;
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

	FileHandleDataStream::FileHandleDataStream(const String& name, ui32 accessMode)
		: DataStream(name, accessMode)
	{
		String mode = "rb";
		if (accessMode == READ)
			mode = "rb";
		else if (accessMode == WRITE)
			mode = "wb";
		else if (accessMode == (READ | WRITE))
			mode = "rb+";

        m_fileHandle = fopen(name.c_str(), mode.c_str());
		if (m_fileHandle)
		{
			// Determine size
			fseek(m_fileHandle, 0, SEEK_END);
			mSize = ftell(m_fileHandle);
			fseek(m_fileHandle, 0, SEEK_SET);
		}
		else
		{
			EchoLogError( "FileHandleDataStream fopen [%s] failed", name.c_str());
		}
	}

	FileHandleDataStream::~FileHandleDataStream()
	{
		close();
	}

	size_t FileHandleDataStream::read(void* buf, size_t count)
	{
		return fread(buf, 1, count, m_fileHandle);
	}

	size_t FileHandleDataStream::write(const void* buf, size_t count)
	{
		if (!isWriteable())
			return 0;
		else
			return fwrite(buf, 1, count, m_fileHandle);
	}

	void FileHandleDataStream::skip(long count)
	{
		fseek(m_fileHandle, count, SEEK_CUR);
	}

	void FileHandleDataStream::seek(size_t pos, int origin)
	{
		fseek(m_fileHandle, static_cast<long>(pos), origin);
	}

	size_t FileHandleDataStream::tell(void) const
	{
		return ftell(m_fileHandle);
	}

	bool FileHandleDataStream::eof(void) const
	{
		return feof(m_fileHandle) != 0;
	}

	void FileHandleDataStream::close(void)
	{
		if (m_fileHandle != 0)
		{
			fflush(m_fileHandle);
			fclose(m_fileHandle);
            m_fileHandle = 0;
		}
	}
}
