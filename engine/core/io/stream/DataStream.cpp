#include "DataStream.h"
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
		char* pBuf = (char*)EchoMalloc(m_size+1);
		// Ensure read from begin of stream
		seek(0);
		read(pBuf, m_size);
		pBuf[m_size] = '\0';
		String str;
		str.insert(0, pBuf, m_size);
		EchoSafeFree(pBuf);
		return str;
	}

	const void* DataStream::getBuffer()
	{
		if (!m_buffer)
		{
			m_buffer = (char*)EchoMalloc(m_size+1);
			read(m_buffer, m_size);
			m_buffer[m_size] = '\0';
		}

		return m_buffer;
	}

    void DataStream::readAll( void* buffer )
    {
        seek(0);
        read(buffer, m_size);
    }
}
