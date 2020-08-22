#include "FileHandleDataStream.h"
#include "engine/core/base/echo_def.h"
#include <engine/core/log/Log.h>
#include "engine/core/io/IO.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

namespace Echo
{
	FileHandleDataStream::FileHandleDataStream(const String& name, ui32 accessMode)
		: DataStream(name, accessMode)
	{
		String mode = "rb";
		if (accessMode == READ)
			mode = "rb";
		else if (accessMode == WRITE)
			mode = "wb";
		else if (accessMode == (READ | WRITE))
			mode = "wb+";

        m_fileHandle = fopen(name.c_str(), mode.c_str());
		if (m_fileHandle)
		{
			// Determine size
			fseek(m_fileHandle, 0, SEEK_END);
			m_size = ftell(m_fileHandle);
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
